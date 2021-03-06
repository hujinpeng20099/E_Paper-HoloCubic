#include "bilibili_gui.h"
#include "bilibili.h"



static lv_obj_t *fans_label = NULL;
static lv_obj_t *follow_label = NULL;
static lv_obj_t *logo_image = NULL;

static lv_style_t fans_label_style;

LV_FONT_DECLARE(font_24);
LV_IMG_DECLARE(bilibili_ioc);


void bilibili_gui_init(void)
{
    lv_style_init(&fans_label_style);
    lv_style_set_text_font(&fans_label_style, &font_24);

    // follow_label = lv_label_create(lv_scr_act(), NULL);
    logo_image = lv_img_create(lv_scr_act());
    fans_label = lv_label_create(lv_scr_act()); 
    lv_obj_add_style(fans_label, &fans_label_style,0);
    lv_label_set_text(fans_label, " ");
    lv_obj_align(fans_label,LV_ALIGN_RIGHT_MID,-10,40);

    lv_img_set_src(logo_image, &bilibili_ioc);
    lv_obj_align(logo_image, LV_ALIGN_RIGHT_MID, -10,-10);
    lv_obj_set_size(logo_image, 108, 48);

}


/*
 * 其他函数请根据需要添加
 */
void display_bilibili(void)
{
    lv_label_set_text_fmt(fans_label, "%d", bilibili_run_data->fans_num);
    lv_img_set_src(logo_image, &bilibili_ioc);
}

