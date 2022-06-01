#ifndef APP_WEATHER_GUI_H
#define APP_WEATHER_GUI_H

#include "lvgl.h"



#ifdef __cplusplus
extern "C"
{
#endif

    int airQulityLevel(int q);
    void weather_gui_init(void);
    void display_timer(void);
    void display_day_weather(void);

#ifdef __cplusplus
} /* extern "C" */
#endif


#endif