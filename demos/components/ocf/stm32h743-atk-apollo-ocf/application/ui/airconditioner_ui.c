#include <stdio.h>
#include <stdlib.h>
#include <lvgl.h>
#include <oc_storage.h>
#include "airconditioner_ui.h"

LV_FONT_DECLARE(lv_font_montserrat_44);
LV_FONT_DECLARE(ocf_font_44);

static lv_style_t label_style;
static lv_style_t area_style;
static lv_obj_t *back_ground = NULL;
static lv_obj_t *area1 = NULL;
static lv_obj_t *area2 = NULL;
static lv_obj_t *title = NULL;
static lv_obj_t *label1 = NULL;
static lv_obj_t *label2 = NULL;

void config_label_style(void)
{
	lv_style_init(&label_style);
	lv_style_set_text_opa(&label_style, LV_STATE_DEFAULT, LV_OPA_COVER);
	lv_style_set_text_color(&label_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
    lv_style_set_text_font(&label_style, LV_STATE_DEFAULT,  &ocf_font_44);
}

void config_area_style(void)
{
    lv_style_init(&area_style);
    lv_style_set_radius(&area_style, LV_STATE_DEFAULT, 0);
    //lv_style_set_bg_color(&label_style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_width(&area_style, LV_STATE_DEFAULT, 1);
    lv_style_set_border_color(&area_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
}

void show_airconditioner(void)
{
    char binaryswitch[65] = {0};
    char temperature[65] = {0};
    oc_storage_read("binaryswitch", (uint8_t*)binaryswitch, sizeof(binaryswitch));
    oc_storage_read("temperature", (uint8_t*)temperature, sizeof(temperature));

    config_label_style();
    config_area_style();

    back_ground = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_clean_style_list(back_ground, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_bg_color(back_ground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(back_ground, 800, 480);

    title = lv_label_create(back_ground, NULL);
    lv_obj_set_pos(title,75, 30);
    lv_obj_add_style(title, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(title, "ONEOS智能空调");

    area1 = lv_obj_create(back_ground, NULL);
    lv_obj_set_pos(area1, 0, 110);
    lv_obj_set_size(area1, 240, 162);
    lv_obj_add_style(area1, LV_LABEL_PART_MAIN, &area_style);
    area2 = lv_obj_create(back_ground, NULL);
    lv_obj_set_pos(area2, 240, 110);
    lv_obj_set_size(area2, 240, 162);
    lv_obj_add_style(area2, LV_LABEL_PART_MAIN, &area_style);

    label1 = lv_label_create(area1, NULL);
    lv_obj_set_pos(label1, 60, 60);
    lv_obj_add_style(label1, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text_fmt(label1, "%4.1f ℃", atof(temperature));
    
    label2 = lv_label_create(area2, NULL);
    lv_obj_set_pos(label2, 100, 60);
    lv_obj_add_style(label2, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text_fmt(label2, "%s", (strcmp(binaryswitch, "0"))?"开":"关");
}

void ui_set_temperature(double temperature)
{
    if(NULL != label1)
    {
        lv_label_set_text_fmt(label1, "%4.1f C", temperature);
    }
}

void ui_set_binary_switch(int binaryswitch)
{
    if(NULL != label2)
    {
        lv_label_set_text_fmt(label2, "%s", (0 == binaryswitch)?"关":"开");
    }
}
