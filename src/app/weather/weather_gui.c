#include "weather_gui.h"
#include "weather.h"
#include "weather_image.h"

#include "lvgl.h"
#include <esp32-hal.h>



LV_FONT_DECLARE(font_88);
LV_FONT_DECLARE(font_32);

static lv_style_t   timer_style;
static lv_style_t   weather_style;
static lv_style_t   daylabel_style;

static lv_obj_t *weatherImg = NULL, *weatherdayImg=NULL, *weatherlabel=NULL;
static lv_obj_t *cityLabel = NULL;
static lv_obj_t *clockLabel = NULL, *daylabel = NULL ,*weeklabel = NULL;
static lv_obj_t *tempImg = NULL, *tempBar = NULL, *tempLabel = NULL;
static lv_obj_t *humiImg = NULL, *humiBar = NULL, *humiLabel = NULL;

// static lv_chart_series_t *ser1, *ser2;

// 天气图标路径的映射关系
// const void *weaImage_map[] = {&weather_0, &weather_9, &weather_14, &weather_5, &weather_25,
//                               &weather_30, &weather_26, &weather_11, &weather_23};
const void *weaImage_map[] = {&rain};
static const char weekDayCh[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static const char airQualityCh[6][10] = {"优", "良", "轻度", "中度", "重度", "严重"};

void weather_gui_init(void)
{
    lv_style_init(&timer_style);
    lv_style_set_text_font(&timer_style,&font_88);

    lv_style_init(&weather_style);
    lv_style_set_text_font(&weather_style,&lv_font_montserrat_24);

    lv_style_init(&daylabel_style);
    lv_style_set_text_font(&daylabel_style, &font_32);

    weatherImg = lv_img_create(lv_scr_act());
    lv_img_set_src(weatherImg, weaImage_map[0]);
    lv_obj_align(weatherImg, LV_ALIGN_LEFT_MID, 20,0);


    clockLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(clockLabel, &timer_style,0);
    lv_label_set_text(clockLabel, " ");
    lv_obj_align(clockLabel,LV_ALIGN_TOP_LEFT,10,0);   

    daylabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(daylabel, &daylabel_style,0);
    lv_label_set_text(daylabel, " ");
    lv_obj_align(daylabel,LV_ALIGN_BOTTOM_LEFT,15,0);   

    weeklabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(weeklabel, &daylabel_style,0);
    lv_label_set_text(weeklabel, " ");    
    lv_obj_align(weeklabel,LV_ALIGN_BOTTOM_LEFT,160,0);    
}
/*
更新时间显示
*/
void display_timer(void)
{
    update_time();
    //更新时间显示
    if(weather_run_data->timer.minute<10)
    {
        if(weather_run_data->timer.hour<10)
        {
            lv_label_set_text_fmt(clockLabel, "0%d:0%d", weather_run_data->timer.hour,weather_run_data->timer.minute);
        }
        else
        {
            lv_label_set_text_fmt(clockLabel, "%d:0%d", weather_run_data->timer.hour,weather_run_data->timer.minute);
        } 
    }
    else if(weather_run_data->timer.hour<10)
    {
        if(weather_run_data->timer.minute<10)
        {
            lv_label_set_text_fmt(clockLabel, "0%d:0%d", weather_run_data->timer.hour,weather_run_data->timer.minute);
        }
        else
        {
            lv_label_set_text_fmt(clockLabel, "0%d:%d", weather_run_data->timer.hour,weather_run_data->timer.minute);
        } 
    }
    else
    {
        lv_label_set_text_fmt(clockLabel, "%d:%d", weather_run_data->timer.hour,weather_run_data->timer.minute);
    }
    //更新日期显示
    if(weather_run_data->timer.day<10)
    {
        if(weather_run_data->timer.month<10)
        {
            lv_label_set_text_fmt(daylabel, "0%d-0%d", weather_run_data->timer.month,weather_run_data->timer.day);
        }
        else
        {
            lv_label_set_text_fmt(daylabel, "%d-0%d", weather_run_data->timer.month,weather_run_data->timer.day);
        } 
    }
    else if(weather_run_data->timer.month<10)
    {
        if(weather_run_data->timer.day<10)
        {
            lv_label_set_text_fmt(daylabel, "0%d-0%d", weather_run_data->timer.month,weather_run_data->timer.day);
        }
        else
        {
            lv_label_set_text_fmt(daylabel, "0%d-%d", weather_run_data->timer.month,weather_run_data->timer.day);
        } 
    }
    else
    {
        lv_label_set_text_fmt(daylabel, "%d-%d",weather_run_data->timer.month,weather_run_data->timer.day);
    }
    //更新星期日
    switch (weather_run_data->timer.weekday)
    {
        case 0:
            lv_label_set_text(weeklabel,"Sun");
            break;
        case 1:
            lv_label_set_text(weeklabel,"Mon");
            break;
        case 2:
            lv_label_set_text(weeklabel,"Tue");
            break;
        case 3:
            lv_label_set_text(weeklabel,"Wed");
            break;
        case 4:
            lv_label_set_text(weeklabel,"Thu");
            break;
        case 5:
            lv_label_set_text(weeklabel,"Fri");
            break;
        case 6:
            lv_label_set_text(weeklabel,"Sat");
            break;            
    }
}


void display_day_weather(void)
{

}


int airQulityLevel(int q)
{
    if (q < 50)
    {
        return 0;
    }
    else if (q < 100)
    {
        return 1;
    }
    else if (q < 150)
    {
        return 2;
    }
    else if (q < 200)
    {
        return 3;
    }
    else if (q < 300)
    {
        return 4;
    }
    return 5;
}