#include <lvgl.h>
#include <oc_storage.h>
#include "smartlock_ui.h"

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

#define LOCK_STATUS "Locked"
#define UNLOCK_STATUS "Unlocked"

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
}

void show_smartlock(void)
{
    char lockstatus[65] = {0};
    char password[65] = {0};
    oc_storage_read("lockstatus", (uint8_t*)lockstatus, sizeof(lockstatus));
    oc_storage_read("password", (uint8_t*)password, sizeof(password));

    config_label_style();
    config_area_style();

    back_ground = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_clean_style_list(back_ground, LV_OBJ_PART_MAIN);
    lv_obj_set_style_local_bg_color(back_ground, LV_OBJ_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_obj_set_size(back_ground, 800, 480);

    title = lv_label_create(back_ground, NULL);
    lv_obj_set_pos(title,75, 30);
    lv_obj_add_style(title, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text(title, "ONEOS智能门锁");

    area1 = lv_obj_create(back_ground, NULL);
    lv_obj_set_pos(area1, 0, 110);
    lv_obj_set_size(area1, 240, 162);
    lv_obj_add_style(area1, LV_LABEL_PART_MAIN, &area_style);
    area2 = lv_obj_create(back_ground, NULL);
    lv_obj_set_pos(area2, 240, 110);
    lv_obj_set_size(area2, 240, 162);
    lv_obj_add_style(area2, LV_LABEL_PART_MAIN, &area_style);

    label1 = lv_label_create(area1, NULL);
    lv_obj_set_pos(label1, 50, 60);
    lv_obj_add_style(label1, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);
    lv_label_set_text_fmt(label1, "%s", password);
    
    label2 = lv_label_create(area2, NULL);
    lv_obj_set_pos(label2, 50, 60);
    lv_obj_add_style(label2, LV_LABEL_PART_MAIN, &label_style);
    lv_label_set_text_fmt(label2, "%s", (strcmp(lockstatus, UNLOCK_STATUS))?"已上锁":"未上锁");
}

void ui_set_lock_passwd(char *passwd)
{
    if(NULL != label2 && NULL != passwd)
    {
        lv_label_set_text_fmt(label1, "%s", passwd);
    }
}

void ui_set_lock_status(char *status)
{
    if(NULL != label2 && NULL != status)
    {
        lv_label_set_text_fmt(label2, "%s", (strcmp(status, UNLOCK_STATUS))?"已上锁":"未上锁");
    }
}
