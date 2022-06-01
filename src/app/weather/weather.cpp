#include "weather.h"
#include "network.h"
#include "ArduinoJson.h"
#include <esp32-hal-timer.h>
#include <map>
#include "ESP32Time.h"

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

std::map<String, int> weatherMap = {{"qing", 0}, {"yin", 1}, {"yu", 2}, {"yun", 3}, {"bingbao", 4}, {"wu", 5}, {"shachen", 6}, {"lei", 7}, {"xue", 8}};

ESP32Time g_rtc; // 用于时间解码
static WT_Config cfg_data;
wea_run_data *weather_run_data = NULL;

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
    snprintf(api, 128, WEATHER_NOW_API, "74432881", "AmYzA12F");

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
            strcpy(weather_run_data->wea.cityname, sk["city"].as<String>().c_str());
            weather_run_data->wea.weather_code = weatherMap[sk["wea_img"].as<String>()];
            weather_run_data->wea.temperature = sk["tem"].as<int>();

            // 获取湿度
            weather_run_data->wea.humidity = 50;
            char humidity[8] = {0};
            strncpy(humidity, sk["humidity"].as<String>().c_str(), 8);
            humidity[strlen(humidity) - 1] = 0; // 去除尾部的 % 号
            weather_run_data->wea.humidity = atoi(humidity);

            weather_run_data->wea.maxTemp = sk["tem1"].as<int>();
            weather_run_data->wea.minTemp = sk["tem2"].as<int>();
            strcpy(weather_run_data->wea.windDir, sk["win"].as<String>().c_str());
            weather_run_data->wea.windLevel = windLevelAnalyse(sk["win_speed"].as<String>());
            weather_run_data->wea.airQulity = airQulityLevel(sk["air"].as<int>());
            Serial.printf("weathercode=%d",weather_run_data->wea.weather_code);
            // Serial.printf("temp=%d,humid=%d,win=%d,air=%d",weather_run_data->wea.temperature,weather_run_data->wea.humidity,weather_run_data->wea.windLevel,weather_run_data->wea.airQulity);
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
    weather_run_data->preNetTimestamp = weather_run_data->preNetTimestamp + (millis() - weather_run_data->preLocalTimestamp)/1000;//add 1s per min fot timer accuracy
    weather_run_data->preLocalTimestamp = millis();
    return weather_run_data->preNetTimestamp; 
}

static long long get_ntp_timestamp(String url)
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
            weather_run_data->preNetTimestamp = (atoll(time.c_str()) + weather_run_data->errorNetTimestamp + TIMEZERO_OFFSIZE)/1000;
            weather_run_data->preLocalTimestamp = millis();
        }
    }
    else
    {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        // 得不到网络时间戳时
        weather_run_data->preNetTimestamp = weather_run_data->preNetTimestamp + (millis() - weather_run_data->preLocalTimestamp);
        weather_run_data->preLocalTimestamp = millis();
    }
    http.end();

    return weather_run_data->preNetTimestamp;
}

static void get_daliyWeather(wea_run_data *wdata)
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
                wdata->wea.daily_max[gDW_i] = sk["data"][gDW_i]["tem_day"].as<int>();
                wdata->wea.daily_min[gDW_i] = sk["data"][gDW_i]["tem_night"].as<int>();
                wdata->wea.wcode[gDW_i] =  weatherMap[sk["wea_img"].as<String>()];
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
    g_rtc.setTime(timestamp,0);
    weather_run_data->timer.month = g_rtc.getMonth() + 1;
    weather_run_data->timer.day = g_rtc.getDay();
    weather_run_data->timer.hour = g_rtc.getHour(true);
    weather_run_data->timer.minute = g_rtc.getMinute();
    weather_run_data->timer.second = g_rtc.getSecond();
    weather_run_data->timer.weekday = g_rtc.getDayofWeek();

}

void weather_init(void)
{
    // 初始化运行时参数
    weather_run_data = (wea_run_data *)calloc(1, sizeof(wea_run_data));
    memset((char *)&weather_run_data->wea, 0, sizeof(Weather));

    weather_run_data->preNetTimestamp = 1609455565; //// default (1609459200) = 1st Jan 2021
    weather_run_data->errorNetTimestamp = 5;
    weather_run_data->preLocalTimestamp = millis(); // 上一次的本地机器时间戳
    weather_run_data->preTimeMillis = 0;

    UpdateTime_RTC(weather_run_data->preNetTimestamp);
    Serial.println("weather init ok!");
}


void update_ntp_time(void)
{
    UpdateTime_RTC(get_ntp_timestamp(TIME_API));
}

void update_time(void)
{
    UpdateTime_RTC(get_timestamp());
}

void update_weather(void)
{
    get_weather();
    get_daliyWeather(weather_run_data);
}

