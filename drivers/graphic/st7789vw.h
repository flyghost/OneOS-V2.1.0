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
 * @file        st7789vw.h
 *
 * @brief       This file provides st7789vw marco definition and function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_LCD_H__
#define __DRV_LCD_H__

#include <os_task.h>

#define LCD_W 240
#define LCD_H 240

/* POINT_COLOR */
#define WHITE   0xFFFF
#define BLACK   0x0000
#define BLUE    0x001F
#define BRED    0XF81F
#define GRED    0XFFE0
#define GBLUE   0X07FF
#define RED     0xF800
#define MAGENTA 0xF81F
#define GREEN   0x07E0
#define CYAN    0x7FFF
#define YELLOW  0xFFE0
#define BROWN   0XBC40
#define BRRED   0XFC07
#define GRAY    0X8430
#define GRAY175 0XAD75
#define GRAY151 0X94B2
#define GRAY187 0XBDD7
#define GRAY240 0XF79E

void lcd_clear(os_uint16_t color);
void lcd_address_set(os_uint16_t x1, os_uint16_t y1, os_uint16_t x2, os_uint16_t y2);
void lcd_set_color(os_uint16_t back, os_uint16_t fore);

void lcd_draw_point(os_uint16_t x, os_uint16_t y);
void lcd_draw_circle(os_uint16_t x0, os_uint16_t y0, os_uint8_t r);
void lcd_draw_line(os_uint16_t x1, os_uint16_t y1, os_uint16_t x2, os_uint16_t y2);
void lcd_draw_rectangle(os_uint16_t x1, os_uint16_t y1, os_uint16_t x2, os_uint16_t y2);
void lcd_fill(os_uint16_t x_start, os_uint16_t y_start, os_uint16_t x_end, os_uint16_t y_end, os_uint16_t color);

void     lcd_show_num(os_uint16_t x, os_uint16_t y, os_uint32_t num, os_uint8_t len, os_uint32_t size);
os_err_t lcd_show_string(os_uint16_t x, os_uint16_t y, os_uint32_t size, const char *fmt, ...);
os_err_t lcd_show_image(os_uint16_t x, os_uint16_t y, os_uint16_t length, os_uint16_t wide, const os_uint8_t *p);

void lcd_enter_sleep(void);
void lcd_exit_sleep(void);
void lcd_display_on(void);
void lcd_display_off(void);

#endif
