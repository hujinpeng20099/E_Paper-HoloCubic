#ifndef APP_WEATHER_H
#define APP_WEATHER_H

#include "weather_gui.h"

#ifdef __cplusplus
extern "C"
{
#endif
struct Weather
{
    int weather_code; // 天气现象代码
    int temperature;  // 温度
    int humidity;     // 湿度
    int maxTemp;      // 最高气温
    int minTemp;      // 最低气温
    char windDir[20];
    char cityname[10]; // 城市名
    int windLevel;
    int airQulity;

    short daily_max[7];
    short daily_min[7];
    int      wcode[7];
};

struct TimeStr
{
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int weekday;
};


struct wea_run_data
{
    // unsigned long preWeatherMillis; // 上一回更新天气时的毫秒数
    unsigned long preTimeMillis;    // 更新时间计数器
    long long preNetTimestamp;      // 上一次的网络时间戳
    long long errorNetTimestamp;    // 网络到显示过程中的时间误差
    long long preLocalTimestamp;    // 上一次的本地机器时间戳

    struct Weather wea;
    struct TimeStr timer;
};



extern struct wea_run_data *weather_run_data;

void weather_init(void);
void get_weather(void);

void update_ntp_time(void);
void update_time(void);
void update_weather(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif