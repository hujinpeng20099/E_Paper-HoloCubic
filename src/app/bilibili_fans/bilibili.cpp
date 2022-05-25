#include "bilibili.h"
#include "network.h"

#define FANS_API "https://api.bilibili.com/x/relation/stat?vmid="
#define OTHER_API "https://api.bilibili.com/x/space/upstat?mid="


struct B_Config
{
    String bili_uid;              // bilibili的uid
    unsigned long updataInterval; // 更新的时间间隔(s)
};


struct BilibiliAppRunData
{
    unsigned int fans_num;
    unsigned int follow_num;
    unsigned int refresh_status;
    unsigned long refresh_time_millis;
};

struct MyHttpResult
{
    int httpCode = 0;
    String httpResponse = "";
};

static B_Config cfg_data;
static BilibiliAppRunData *run_data = NULL;

static MyHttpResult http_request(String uid)
{
    // String url = "http://www.dtmb.top/api/fans/index?id=" + uid;
    MyHttpResult result;
    String url = FANS_API + uid;
    HTTPClient *httpClient = new HTTPClient();
    httpClient->setTimeout(1000);
    bool status = httpClient->begin(url);
    if (status == false)
    {
        result.httpCode = -1;
        return result;
    }
    int httpCode = httpClient->GET();
    String httpResponse = httpClient->getString();
    httpClient->end();
    result.httpCode = httpCode;
    result.httpResponse = httpResponse;
    return result;
}

void bilibili_init(void)
{
    // bilibili_gui_init();
    // 初始化运行时参数
    cfg_data.bili_uid="487263224";
    cfg_data.updataInterval=86400;//一天更新一次
    run_data = (BilibiliAppRunData *)malloc(sizeof(BilibiliAppRunData));
    run_data->fans_num = 0;
    run_data->follow_num = 0;
    run_data->refresh_status = 0;
}

void update_fans_num(void)
{
    MyHttpResult result = http_request(cfg_data.bili_uid);
    if (-1 == result.httpCode)
    {
        Serial.println("[HTTP] Http request failed.");
    }
    if (result.httpCode > 0)
    {
        if (result.httpCode == HTTP_CODE_OK || result.httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = result.httpResponse;
            // Serial.println("[HTTP] OK");
            // Serial.println(payload);
            int startIndex_1 = payload.indexOf("follower") + 10;
            int endIndex_1 = payload.indexOf('}', startIndex_1);
            int startIndex_2 = payload.indexOf("following") + 11;
            int endIndex_2 = payload.indexOf(',', startIndex_2);
            String res = payload.substring(startIndex_1, endIndex_1);
            run_data->fans_num = payload.substring(startIndex_1, endIndex_1).toInt();
            run_data->follow_num = payload.substring(startIndex_2, endIndex_2).toInt();
            run_data->refresh_status = 1;
            Serial.printf("fans_num=%d",run_data->fans_num);
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
}

