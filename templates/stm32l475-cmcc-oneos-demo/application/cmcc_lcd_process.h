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
 * @file        cmcc_lcd_process.h
 *
 * @brief       This file provides LCD function declaration and event definition.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMCC_LCD_PROCESS_H__
#define __CMCC_LCD_PROCESS_H__
#include <os_task.h>
#include <stdint.h>

#define CMCC_LCD_MENU_MAX 14

#define CMCC_LCD_HOME_ROW_SP 100
#define CMCC_LCD_HOME_ROW_H 32
#define CMCC_LCD_HOME_LCOLUMN_SP 35
#define CMCC_LCD_HOME_RCOLUMN_SP 110

#define CMCC_LCD_SENSOR_ROW_SP 65
#define CMCC_LCD_SENSOR_ROW_H 40
#define CMCC_LCD_SENSOR_LCOLUMN_SP 25
#define CMCC_LCD_SENSOR_RCOLUMN_SP 180

#define CMCC_LCD_TIME_ROW_SP 65
#define CMCC_LCD_TIME_ROW_H 50
#define CMCC_LCD_TIME_LCOLUMN_SP 65
#define CMCC_LCD_TIME_RCOLUMN_SP 110

#define CMCC_LCD_INFO_ROW_SP 65
#define CMCC_LCD_INFO_ROW_H 50
#define CMCC_LCD_INFO_LCOLUMN_SP 65
#define CMCC_LCD_INFO_RCOLUMN_SP 110

#define SHOW_COLOR_RED 0xFA27
#define SHOW_COLOR_GREEN 0x1546
#define SHOW_COLOR_BLUE 0x32B9
#define SHOW_COLOR_YELLOW 0xFD20


typedef void (*cmcc_lcd_handle)(void*);

typedef enum
{
    LCD_EVENT_INDEX               = (1 << 0),
    LCD_EVENT_SENSOR              = (1 << 1),
    LCD_EVENT_TIME                = (1 << 2),
    LCD_EVENT_EXIT                = (1 << 3),
    LCD_EVENT_INFO                = (1 << 4),
} cmcc_lcd_event_t;

typedef enum
{
    CMCC_LCD_CONTENT_NONE = 0,
    CMCC_LCD_STATIC_CONTENT,
    CMCC_LCD_DYNAMIC_CONTENT
} cmcc_lcd_content_type_t;

void cmcc_lcd_start(void);
void cmcc_lcd_show_startup_page(void);
void cmcc_lcd_event_put(cmcc_lcd_event_t event);

#endif
