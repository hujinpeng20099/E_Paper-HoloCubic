#include "bilibili.h"
#include "network.h"
#include "bilibili_gui.h"
#include "ArduinoJson.h"


#define FANS_API    "https://api.bilibili.com/x/relation/stat?vmid=487263224"


BilibiliAppRunData *bilibili_run_data = NULL;


void bilibili_init(void)
{
    bilibili_gui_init();
    // 初始化运行时参数
    bilibili_run_data = (BilibiliAppRunData *)malloc(sizeof(BilibiliAppRunData));
    bilibili_run_data->fans_num = 0;
    bilibili_run_data->follow_num = 0;
    bilibili_run_data->refresh_status = 0;
    Serial.println("bilibili init ok!");
}

void update_fans_num(void)
{
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(FANS_API);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            Serial.println(payload);
            int startIndex_1 = payload.indexOf("follower") + 10;
            int endIndex_1 = payload.indexOf('}', startIndex_1);
            bilibili_run_data->fans_num = payload.substring(startIndex_1, endIndex_1).toInt();
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
    http.end();
}

