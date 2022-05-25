#include "weather.h"
#include "weather_gui.h"
#include "ESP32Time.h"
#include "network.h"
#include "ArduinoJson.h"
#include <esp32-hal-timer.h>
#include <map>

#define WEATHER_APP_NAME        "Weather"
#define WEATHER_NOW_API         "https://www.yiketianqi.com/free/day?appid=%s&appsecret=%s&unescape=1" 
#define WEATHER_DALIY_API       "https://www.yiketianqi.com/free/week?unescape=1&appid=%s&appsecret=%s" 
#define TIME_API                "http://api.m.taobao.com/rest/api3.do?api=mtop.common.gettimestamp" 
#define WEATHER_PAGE_SIZE       2
#define UPDATE_WEATHER          0x01       // 更新天气
#define UPDATE_DALIY_WEATHER    0x02 // 更新每天天气
#define UPDATE_TIME             0x04          // 更新时间


struct WT_Config
{
    String tianqi_appid;                 // tianqiapid 的 appid
    String tianqi_appsecret;             // tianqiapid 的 appsecret
    String tianqi_addr;                  // tianqiapid 的地址（填中文）
    unsigned long weatherUpdataInterval; // 天气更新的时间间隔(s)
    unsigned long timeUpdataInterval;    // 日期时钟更新的时间间隔(s)
};

struct WeatherAppRunData
{
    unsigned long preWeatherMillis; // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;    // 更新时间计数器
    long long preNetTimestamp;      // 上一次的网络时间戳
    long long errorNetTimestamp;    // 网络到显示过程中的时间误差
    long long preLocalTimestamp;    // 上一次的本地机器时间戳
    unsigned int coactusUpdateFlag; // 强制更新标志
    int clock_page;
    unsigned int update_type; // 更新类型的标志位

    BaseType_t xReturned_task_task_update; // 更新数据的异步任务
    TaskHandle_t xHandle_task_task_update; // 更新数据的异步任务

    ESP32Time g_rtc; // 用于时间解码
    Weather wea;     // 保存天气状况
};

static WT_Config cfg_data;
static WeatherAppRunData *run_data = NULL;

enum wea_event_Id
{
    UPDATE_NOW,
    UPDATE_NTP,
    UPDATE_DAILY
};

std::map<String, int> weatherMap = {{"qing", 0}, {"yin", 1}, {"yu", 2}, {"yun", 3}, {"bingbao", 4}, {"wu", 5}, {"shachen", 6}, {"lei", 7}, {"xue", 8}};

static void task_update(void *parameter); // 异步更新任务

static int windLevelAnalyse(String str)
{
    int ret = 0;
    for (char ch : str)
    {
        if (ch >= '0' && ch <= '9')
        {
            ret = ret * 10 + (ch - '0');
        }
    }
    return ret;
}

void get_weather(void)
{
    if (WL_CONNECTED != WiFi.status())
        return;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = {0};
    snprintf(api, 128, WEATHER_NOW_API, "appid", "appsecret");

    Serial.print("API = ");
    Serial.println(api);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            // Serial.println(payload);
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            strcpy(run_data->wea.cityname, sk["city"].as<String>().c_str());
            run_data->wea.weather_code = weatherMap[sk["wea_img"].as<String>()];
            run_data->wea.temperature = sk["tem"].as<int>();

            // 获取湿度
            run_data->wea.humidity = 50;
            char humidity[8] = {0};
            strncpy(humidity, sk["humidity"].as<String>().c_str(), 8);
            humidity[strlen(humidity) - 1] = 0; // 去除尾部的 % 号
            run_data->wea.humidity = atoi(humidity);

            run_data->wea.maxTemp = sk["tem1"].as<int>();
            run_data->wea.minTemp = sk["tem2"].as<int>();
            strcpy(run_data->wea.windDir, sk["win"].as<String>().c_str());
            run_data->wea.windLevel = windLevelAnalyse(sk["win_speed"].as<String>());
            run_data->wea.airQulity = airQulityLevel(sk["air"].as<int>());
            Serial.printf("weathercode=%d",run_data->wea.weather_code);
            // Serial.printf("temp=%d,humid=%d,win=%d,air=%d",run_data->wea.temperature,run_data->wea.humidity,run_data->wea.windLevel,run_data->wea.airQulity);
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

static long long get_timestamp()
{
    // 使用本地的机器时钟
    run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
    run_data->preLocalTimestamp = millis();
    return run_data->preNetTimestamp;
}

static long long get_timestamp(String url)
{
    if (WL_CONNECTED != WiFi.status())
        return 0;

    String time = "";
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println(payload);
            int time_index = (payload.indexOf("data")) + 12;
            time = payload.substring(time_index, payload.length() - 3);
            // 以网络时间戳为准
            run_data->preNetTimestamp = atoll(time.c_str()) + run_data->errorNetTimestamp + TIMEZERO_OFFSIZE;
            run_data->preLocalTimestamp = millis();
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        run_data->preNetTimestamp = run_data->preNetTimestamp + (millis() - run_data->preLocalTimestamp);
        run_data->preLocalTimestamp = millis();
    }
    http.end();

    return run_data->preNetTimestamp;
}

static void get_daliyWeather(short maxT[], short minT[])
{
    if (WL_CONNECTED != WiFi.status())
        return;

    HTTPClient http;
    http.setTimeout(1000);
    char api[128] = {0};
    snprintf(api, 128, WEATHER_DALIY_API, "74432881", "AmYzA12F");
    Serial.print("API = ");
    Serial.println(api);
    http.begin(api);

    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            Serial.println(payload);
            DynamicJsonDocument doc(2048);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            for (int gDW_i = 0; gDW_i < 7; ++gDW_i)
            {
                maxT[gDW_i] = sk["data"][gDW_i]["tem_day"].as<int>();
                minT[gDW_i] = sk["data"][gDW_i]["tem_night"].as<int>();
            }
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
}

static void UpdateTime_RTC(long long timestamp)
{
    struct TimeStr t;
    run_data->g_rtc.setTime(timestamp / 1000);
    t.month = run_data->g_rtc.getMonth() + 1;
    t.day = run_data->g_rtc.getDay();
    t.hour = run_data->g_rtc.getHour(true);
    t.minute = run_data->g_rtc.getMinute();
    t.second = run_data->g_rtc.getSecond();
    t.weekday = run_data->g_rtc.getDayofWeek();
    // Serial.printf("time : %d-%d-%d\n",t.hour, t.minute, t.second);
    display_time(t, LV_SCR_LOAD_ANIM_NONE);
}

void weather_init(void)
{
    // 初始化运行时参数
    run_data = (WeatherAppRunData *)calloc(1, sizeof(WeatherAppRunData));
    memset((char *)&run_data->wea, 0, sizeof(Weather));
    run_data->preNetTimestamp = 1577808000000; // 上一次的网络时间戳 初始化为2020-01-01 00:00:00
    run_data->errorNetTimestamp = 2;
    run_data->preLocalTimestamp = millis(); // 上一次的本地机器时间戳
    run_data->clock_page = 0;
    run_data->preWeatherMillis = 0;
    run_data->preTimeMillis = 0;
    // 强制更新
    run_data->coactusUpdateFlag = 0x01;
    run_data->update_type = 0x00; // 表示什么也不需要更新
    Serial.println("weather init ok!");
}



// static void weather_message_handle(void)
// {
//     switch (type)
//     {
//     case APP_MESSAGE_WIFI_CONN:
//     {
//         Serial.println(F("----->weather_event_notification"));
//         int event_id = (int)message;
//         switch (event_id)
//         {
//         case UPDATE_NOW:
//         {
//             Serial.print(F("weather update.\n"));
//             run_data->update_type |= UPDATE_WEATHER;

//             get_weather();
//             if (run_data->clock_page == 0)
//             {
//                 display_weather(run_data->wea, LV_SCR_LOAD_ANIM_NONE);
//             }
//         };
//         break;
//         case UPDATE_NTP:
//         {
//             Serial.print(F("ntp update.\n"));
//             run_data->update_type |= UPDATE_TIME;

//             long long timestamp = get_timestamp(TIME_API); // nowapi时间API
//             if (run_data->clock_page == 0)
//             {
//                 UpdateTime_RTC(timestamp);
//             }
//         };
//         break;
//         case UPDATE_DAILY:
//         {
//             Serial.print(F("daliy update.\n"));
//             run_data->update_type |= UPDATE_DALIY_WEATHER;

//             get_daliyWeather(run_data->wea.daily_max, run_data->wea.daily_min);
//             if (run_data->clock_page == 1)
//             {
//                 display_curve(run_data->wea.daily_max, run_data->wea.daily_min, LV_SCR_LOAD_ANIM_NONE);
//             }
//         };
//         break;
//         default:
//             break;
//         }
//     }
//     break;
//     case APP_MESSAGE_GET_PARAM:
//     {
//         char *param_key = (char *)message;
//         if (!strcmp(param_key, "tianqi_appid"))
//         {
//             snprintf((char *)ext_info, 32, "%s", cfg_data.tianqi_appid.c_str());
//         }
//         else if (!strcmp(param_key, "tianqi_appsecret"))
//         {
//             snprintf((char *)ext_info, 32, "%s", cfg_data.tianqi_appsecret.c_str());
//         }
//         else if (!strcmp(param_key, "tianqi_addr"))
//         {
//             snprintf((char *)ext_info, 32, "%s", cfg_data.tianqi_addr.c_str());
//         }
//         else if (!strcmp(param_key, "weatherUpdataInterval"))
//         {
//             snprintf((char *)ext_info, 32, "%u", cfg_data.weatherUpdataInterval);
//         }
//         else if (!strcmp(param_key, "timeUpdataInterval"))
//         {
//             snprintf((char *)ext_info, 32, "%u", cfg_data.timeUpdataInterval);
//         }
//         else
//         {
//             snprintf((char *)ext_info, 32, "%s", "NULL");
//         }
//     }
//     break;
//     case APP_MESSAGE_SET_PARAM:
//     {
//         char *param_key = (char *)message;
//         char *param_val = (char *)ext_info;
//         if (!strcmp(param_key, "tianqi_appid"))
//         {
//             cfg_data.tianqi_appid = param_val;
//         }
//         else if (!strcmp(param_key, "tianqi_appsecret"))
//         {
//             cfg_data.tianqi_appsecret = param_val;
//         }
//         else if (!strcmp(param_key, "tianqi_addr"))
//         {
//             cfg_data.tianqi_addr = param_val;
//         }
//         else if (!strcmp(param_key, "weatherUpdataInterval"))
//         {
//             cfg_data.weatherUpdataInterval = atol(param_val);
//         }
//         else if (!strcmp(param_key, "timeUpdataInterval"))
//         {
//             cfg_data.timeUpdataInterval = atol(param_val);
//         }
//     }
//     break;
//     default:
//         break;
//     }
// }

