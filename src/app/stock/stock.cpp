#include "app/stock/stock.h"
#include "network.h"
#include "ArduinoJson.h"
#include  "lvgl.h"


// need you change your apikey key = your api key code = stock number
#define STOCK_API    "http://api.tianapi.com/finance/index?key=72f5b2592703a78caf052d420ca68127&code=sh000001&list=0"


StockAppRunData *stock_run_data = NULL;

static lv_obj_t *stockImg0 = NULL, *stockImg1 = NULL;
static lv_obj_t *stocklabel = NULL;
static lv_style_t   stock_style;

LV_FONT_DECLARE(font_24);
LV_IMG_DECLARE(xiadie);
LV_IMG_DECLARE(tongji);
LV_IMG_DECLARE(shangzhang);

void stock_init(void)
{
    // 初始化运行时参数
    stock_run_data = (StockAppRunData *)malloc(sizeof(StockAppRunData));
    stock_run_data->open_price = 0;
    stock_run_data->cur_price = 0;
    stock_run_data->end_price = 0;
    stock_gui_init();
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
            stock_run_data->open_price = sk["newslist"][2];
            stock_run_data->cur_price = sk["newslist"][4];
            Serial.printf("open_price=%d",stock_run_data->open_price); 
        }
    }
    else
    {
        Serial.println("[HTTP] ERROR");
    }
    http.end();
}

void stock_gui_init(void)
{
    lv_style_init(&stock_style);
    lv_style_set_text_font(&stock_style,&font_24);

    stockImg0 = lv_img_create(lv_scr_act());
    lv_obj_align(stockImg0, LV_ALIGN_RIGHT_MID, -50,100);

    stockImg1 = lv_img_create(lv_scr_act());
    lv_obj_align(stockImg1, LV_ALIGN_RIGHT_MID, -5,100);    


    stocklabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(stocklabel, &stock_style,0);
    lv_label_set_text(stocklabel, " ");
    lv_obj_align(stocklabel, LV_ALIGN_BOTTOM_RIGHT, -15,0);    
}


void display_stock(void)
{

    lv_img_set_src(stockImg0, &tongji);
    lv_label_set_text_fmt(stocklabel, "%d", stock_run_data->cur_price);

    if(stock_run_data->cur_price>stock_run_data->open_price)
    {
        lv_img_set_src(stockImg1, &shangzhang);
    }
    else lv_img_set_src(stockImg1, &xiadie);
}