#include "weather_gui.h"
#include "weather.h"
#include "weather_image.h"

#include "lvgl.h"
#include <esp32-hal.h>



LV_FONT_DECLARE(font_88);
LV_FONT_DECLARE(font_32);
LV_FONT_DECLARE(font_24);

static lv_style_t   timer_style;
static lv_style_t   weather_style;
static lv_style_t   daylabel_style;

static lv_obj_t *weatherImg = NULL, *weatherdayImg=NULL, *weatherlabel=NULL;
static lv_obj_t *cityLabel = NULL;
static lv_obj_t *clockLabel = NULL, *daylabel = NULL ,*weeklabel = NULL;
static lv_obj_t *tempImg = NULL, *tempLabel = NULL;
static lv_obj_t *humiImg = NULL, *humiLabel = NULL;
static lv_obj_t *airImg = NULL, *airImg2 = NULL;
// static lv_chart_series_t *ser1, *ser2;

// 天气图标路径的映射关系
const void *weaImage_map[] = {&qingtian, &duoyun, &dayu, &qingzhuanduoyun, &bingbao,
                              &wumai, &shachenbao, &leizhenyu, &baoxue};


void weather_gui_init(void)
{
    lv_style_init(&timer_style);
    lv_style_set_text_font(&timer_style,&font_88);

    lv_style_init(&weather_style);
    lv_style_set_text_font(&weather_style,&font_24);

    lv_style_init(&daylabel_style);
    lv_style_set_text_font(&daylabel_style, &font_32);

    weatherImg = lv_img_create(lv_scr_act());
    // lv_img_set_src(weatherImg, weaImage_map[0]);
    lv_obj_align(weatherImg, LV_ALIGN_LEFT_MID, 50,15);

    tempImg = lv_img_create(lv_scr_act());
    lv_img_set_src(tempImg, &temp);
    lv_obj_align(tempImg, LV_ALIGN_LEFT_MID, 165,-25);

    tempLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(tempLabel, &weather_style,0);
    lv_label_set_text(tempLabel, " ");
    lv_obj_align(tempLabel, LV_ALIGN_LEFT_MID, 210,-25);    

    humiImg = lv_img_create(lv_scr_act());
    lv_img_set_src(humiImg, &hum);
    lv_obj_align(humiImg, LV_ALIGN_LEFT_MID, 165,25);

    humiLabel = lv_label_create(lv_scr_act());
    lv_obj_add_style(humiLabel, &weather_style,0);
    lv_label_set_text(humiLabel, " ");
    lv_obj_align(humiLabel, LV_ALIGN_LEFT_MID, 210,25);    

    airImg = lv_img_create(lv_scr_act());
    lv_img_set_src(airImg, &air);
    lv_obj_align(airImg, LV_ALIGN_LEFT_MID, 165,75);

    airImg2 = lv_img_create(lv_scr_act());
    // lv_img_set_src(airImg2, &weixiao);
    lv_obj_align(airImg2, LV_ALIGN_LEFT_MID, 210,75);  

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
    lv_obj_align(weeklabel,LV_ALIGN_BOTTOM_LEFT,180,0);    
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
    lv_img_set_src(weatherImg, weaImage_map[weather_run_data->wea.weather_code]);
    lv_img_set_src(tempImg, &temp);
    lv_img_set_src(humiImg, &hum);
    lv_img_set_src(airImg, &air);
    lv_label_set_text_fmt(tempLabel, "%d", weather_run_data->wea.temperature);
    lv_label_set_text_fmt(humiLabel, "%d", weather_run_data->wea.humidity);
    switch (weather_run_data->wea.airQulity)
    {
        case 0:
            lv_img_set_src(airImg2, &weixiao);
            break;
        case 1:
            lv_img_set_src(airImg2, &yiban);
            break;
        case 2:
            lv_img_set_src(airImg2, &bukaixin);
            break;                    
        default:
            break;
    }
}


int airQulityLevel(int q)
{
    if (q < 160)
    {
        return 0;
    }
    else if (q < 320)
    {
        return 1;
    }
    return 2;
}