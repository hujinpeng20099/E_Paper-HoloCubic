#include "app/stock/stock.h"
#include "network.h"
#include "ArduinoJson.h"

//key = your api key code = stock number
#define STOCK_API    "http://api.tianapi.com/finance/index?key=72f5b2592703a78caf052d420ca68127&code=sh603986&list=0"


struct StockAppRunData
{
    unsigned int stock_code;
    unsigned int open_price;
    unsigned int cur_price;
    unsigned int end_price;
    unsigned int turnover;
    unsigned long trading_volume;
    unsigned char flag;
    unsigned char up_date; 
};

static StockAppRunData *run_data = NULL;


void stock_init(void)
{
    // 初始化运行时参数
    run_data = (StockAppRunData *)malloc(sizeof(StockAppRunData));
    run_data->open_price = 0;
    run_data->cur_price = 0;
    run_data->end_price = 0;
    Serial.println("stock_init ok!");
}

void update_stock(void)
{
    HTTPClient http;
    http.setTimeout(1000);
    http.begin(STOCK_API);
    int httpCode = http.GET();
    if (httpCode > 0)
    {
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
        {
            String payload = http.getString();
            DynamicJsonDocument doc(1000);
            deserializeJson(doc, payload);
            JsonObject sk = doc.as<JsonObject>();
            run_data->open_price = sk["newslist"][1];
            Serial.printf("open_price=%d",run_data->open_price); 
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
    http.end();
}


