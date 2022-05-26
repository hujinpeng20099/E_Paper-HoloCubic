#include <Arduino.h>
#include "EPaperDrive.h"
#include "lvgl.h"
#include "network.h"
#include "app/stock/stock.h"
#include "app/bilibili_fans/bilibili.h"
#include "app/weather/weather.h"


#define CS  32
#define RST 14
#define DC 27
#define BUSY 26
#define CLK 33
#define MOSI 25

EPaperDrive EPD(1, CS, RST, DC, BUSY); //驱动库的实例化，此处为使用硬件SPI
SPIClass EPDSpi;
Network mynetwork;

static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX*10];

void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    lv_coord_t x=0, y=0;
    for(y=area->y1;y<=area->y2;y++)
    {
        for(x=area->x1;x<=area->x2;x++)
        {
          if(!lv_color_to1(*color_p))EPD.SetPixel(y,x);
          color_p++;
        }
    }   
    lv_disp_flush_ready(disp);
}

void lvgl_task(void * pvParameters)
{
  EPDSpi.begin(CLK,-1,MOSI,-1);
  EPDSpi.beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));
  EPD.SetHardSPI(&EPDSpi);

  EPD.EPD_Set_Model(OPM42);                  //设置屏幕类型，具体型号可以参考文档
  EPD.EPD_init_Full();                       //全刷初始化，使用全刷波形
  EPD.clearbuffer();   

  lv_init();
  // lv_style_init(&label_style);
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = LV_HOR_RES_MAX;
  disp_drv.ver_res = LV_VER_RES_MAX;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;

  // 开启 LV_COLOR_SCREEN_TRANSP 屏幕具有透明和不透明样式
  lv_disp_drv_register(&disp_drv); 
  // label = lv_label_create(lv_scr_act(),NULL);
  // lv_style_set_text_font(&label_style,LV_STATE_DEFAULT, &lv_font_montserrat_48); 
  // lv_obj_add_style(label,LV_LABEL_PART_MAIN,&label_style);  
  // lv_label_set_text(label, "1234567890");
  while(1)
  {
      lv_task_handler();
      vTaskDelay(5);
  }
}

void DataRefresh_task(void * pvParameters) 
{
  uint16_t wt=0,tt=0,bt=0;
  //init weather data and gui
  weather_init();
  //init bilibili data and gui
  bilibili_init();
  //init stock data and gui
  stock_init();
   while(1)
  {
    //refresh weather and stock data 1hour
    if(wt>3600)
    {
      wt=0;

    }
    //refresh timer data 1min
    if(tt>60)
    {
      update_stock();
      update_fans_num();
      get_weather();
      EPD.EPD_Dis_Full((uint8_t *)EPD.EPDbuffer, 1); //将缓存中的图像传给屏幕控制芯片全刷屏幕
      tt=0;
    }
    //refresh bilibili data 12hour
    if(bt>43,200)
    {
      bt=0;
    }    
    vTaskDelay(1000);
    wt++;
    tt++;
    bt++;
  } 
}



void setup() {
  BaseType_t xReturn = pdPASS;
//init serial
  Serial.begin(115200);
//start connect network
  if(mynetwork.start_conn_wifi("ROC","HU515320"))Serial.println("connet wifi success!");
//creat task 
  xReturn = xTaskCreate(lvgl_task,
      "lvgl_task",
      8000,
      NULL,
      tskIDLE_PRIORITY+1,
      NULL);
  xReturn = xTaskCreate(DataRefresh_task,
      "DataRefresh_task",
      10000,
      NULL,
      tskIDLE_PRIORITY+2,
      NULL);    
  if(xReturn==pdPASS)
  {
    Serial.println("task creat success!");
  }
}
HTTPClient http_st;
void loop() {
  vTaskDelay(30000);
}