#include "bilibili.h"
#include "network.h"
#include "bilibili_gui.h"
#include "ArduinoJson.h"


#define FANS_API    "https://api.bilibili.com/x/relation/stat?vmid=487263224"
#define OTHER_API   "https://api.bilibili.com/x/space/upstat?mid="

struct BilibiliAppRunData
{
    unsigned int fans_num;
    unsigned int follow_num;
    unsigned int refresh_status;
    unsigned long refresh_time_millis;
};

static BilibiliAppRunData *run_data = NULL;


void bilibili_init(void)
{
    bilibili_gui_init();
    // 初始化运行时参数
    run_data = (BilibiliAppRunData *)malloc(sizeof(BilibiliAppRunData));
    run_data->fans_num = 0;
    run_data->follow_num = 0;
    run_data->refresh_status = 0;
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
            run_data->fans_num = payload.substring(startIndex_1, endIndex_1).toInt();
            Serial.printf("fans_num=%d",run_data->fans_num); 
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
    http.end();
}

