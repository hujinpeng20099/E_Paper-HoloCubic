#ifndef APP_BILIBILI_GUI_H
#define APP_BILIBILI_GUI_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
#define ANIEND                      \
    while (lv_anim_count_running()) \
        lv_task_handler(); //绛夊緟鍔ㄧ敾瀹屾垚

    void bilibili_gui_init(void);
    void display_bilibili(unsigned int fans_num);


#ifdef __cplusplus
} /* extern "C" */
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#include "lvgl.h"
    extern const lv_img_dsc_t app_bilibili;
    extern const lv_img_dsc_t bilibili_logo_ico;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif