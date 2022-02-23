/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        gui_test.c
 *
 * @brief       The test file for gui.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <string.h>
#include <stdio.h>
#include <shell.h>
#include <lvgl/lvgl.h>
#include <lv_lib_gif/lv_gif.h>

#ifdef OS_USING_GUI_LVGL_GIF
extern const lv_img_dsc_t gif;
static void gif_test(void)
{
    lv_obj_t *scr = lv_scr_act();
    lv_obj_t *img = lv_gif_create_from_data(scr, gif.data);
}
#endif


static void gui_test(void *parameter)
{
#if defined(OS_USING_GUI_LV_EX_GETSTARTED)
    //void lv_ex_get_started_1(void);
    //lv_ex_get_started_1();
        void lv_demo_music(void);
    lv_demo_music();
#elif defined(OS_USING_GUI_LV_EX_MUSIC)
    void lv_demo_music(void);
    lv_demo_music();
#elif defined(OS_USING_GUI_LV_EX_PRINTER)
    void lv_demo_printer(void);
    lv_demo_printer();
#else

#endif

#ifdef OS_USING_GUI_LVGL_GIF
    gif_test();
#endif
}
SH_CMD_EXPORT(gui_test, gui_test, "test gui_test");
