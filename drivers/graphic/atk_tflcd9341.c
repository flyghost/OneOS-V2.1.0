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
 * @file        atk_tflcd9341.c
 *
 * @brief       This file provides atk_tflcd9341 driver functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_memory.h>
#include <graphic/graphic.h>
#include <shell.h>
#include <string.h>
#include <clocksource.h>

#include <dlog.h>

#include "atk_tflcd9341.h"
#include "atk_tflcd_font.h"

#define DBG_TAG "LCD"

struct atk_lcd
{
    os_device_graphic_t graphic;
};

struct atk_lcd_info lcd_dev =
{
    .dir = 0,
    .width  = OS_GRAPHIC_WIDTH,
    .height = OS_GRAPHIC_HEIGHT,
    .wramcmd = 0x2C,
    .setxcmd = 0x2A,
    .setycmd = 0x2B,
};

os_uint32_t FORE_COLOR = 0xFF000000;
os_uint32_t BACK_COLOR = 0xFFFFFFFF;
struct atk_lcd_mem *lcd = (struct atk_lcd_mem *) LCD_BASE;

static void __LCD_WR_REG(os_uint16_t regval)
{
    os_clocksource_ndelay(1000);
    lcd->reg = regval;
}

static void __LCD_WR_DATA(os_uint16_t data)
{
    os_clocksource_ndelay(1000);
    lcd->ram = data;
}

static os_uint16_t __LCD_RD_DATA(void)
{
    os_uint16_t ram;
    ram = lcd->ram;
    return ram;
}

static void __LCD_WriteReg(os_uint16_t LCD_Reg, os_uint16_t LCD_RegValue)
{
    lcd->reg = LCD_Reg;
    lcd->ram = LCD_RegValue;
}

static void __LCD_WriteRAM_Prepare(void)
{
    lcd->reg = lcd_dev.wramcmd;
}

static void lcd_set_cursor(os_uint16_t Xpos, os_uint16_t Ypos)
{

    __LCD_WR_REG(lcd_dev.setxcmd);
    __LCD_WR_DATA(Xpos >> 8);
    __LCD_WR_DATA(Xpos & 0xFF);
    __LCD_WR_REG(lcd_dev.setycmd);
    __LCD_WR_DATA(Ypos >> 8);
    __LCD_WR_DATA(Ypos & 0xFF);
}
static void lcd_clear(os_uint32_t color)
{
    os_uint32_t index = 0;
    os_uint32_t totalpoint = lcd_dev.width;

    totalpoint *= lcd_dev.height;
    lcd_set_cursor(0x00, 0x0000);
    __LCD_WriteRAM_Prepare();
    for (index = 0; index < totalpoint; index++)
    {
        lcd->ram = color;
    }
}

static void lcd_fill(os_uint16_t sx, os_uint16_t sy, os_uint16_t ex, os_uint16_t ey, os_uint32_t color)
{
    os_uint16_t i, j;
    os_uint16_t xlen = 0;
    xlen = ex - sx + 1;
    for (i = sy; i <= ey; i++)
    {
        lcd_set_cursor(sx, i);
        __LCD_WriteRAM_Prepare();
        for (j = 0; j < xlen; j++)
        {
            lcd->ram = color;
        }
    }
}

static void lcd_read_id(void)
{
    os_uint8_t id[2];

    __LCD_WR_REG(0xD3);

    /*dummy read*/
    __LCD_RD_DATA();
    __LCD_RD_DATA();

    id[0] = __LCD_RD_DATA();
    id[1] = __LCD_RD_DATA();

    lcd_dev.id = (id[0] << 8) | id[1];
    LOG_I(DBG_TAG, "lcd id:0x%x", lcd_dev.id);
    FORE_COLOR = RED;
}

static void lcd_scan_direction(os_uint8_t dir)
{
    os_uint16_t regval = 0;
    os_uint16_t dirreg = 0;
    os_uint16_t temp;

    switch (dir)
    {
    case L2R_U2D:
        regval |= (0 << 7) | (0 << 6) | (0 << 5);
        break;
    case L2R_D2U:
        regval |= (1 << 7) | (0 << 6) | (0 << 5);
        break;
    case R2L_U2D:
        regval |= (0 << 7) | (1 << 6) | (0 << 5);
        break;
    case R2L_D2U:
        regval |= (1 << 7) | (1 << 6) | (0 << 5);
        break;
    case U2D_L2R:
        regval |= (0 << 7) | (0 << 6) | (1 << 5);
        break;
    case U2D_R2L:
        regval |= (0 << 7) | (1 << 6) | (1 << 5);
        break;
    case D2U_L2R:
        regval |= (1 << 7) | (0 << 6) | (1 << 5);
        break;
    case D2U_R2L:
        regval |= (1 << 7) | (1 << 6) | (1 << 5);
        break;
    }
    dirreg = 0x36;
    regval |= 0x08;

    __LCD_WriteReg(dirreg, regval);

    if (regval & 0x20)
    {
        if (lcd_dev.width < lcd_dev.height)
        {
            temp = lcd_dev.width;
            lcd_dev.width = lcd_dev.height;
            lcd_dev.height = temp;
        }
    }
    else
    {
        if (lcd_dev.width > lcd_dev.height)
        {
            temp = lcd_dev.width;
            lcd_dev.width = lcd_dev.height;
            lcd_dev.height = temp;
        }
    }

    __LCD_WR_REG(lcd_dev.setxcmd);
    __LCD_WR_DATA(0);
    __LCD_WR_DATA(0);
    __LCD_WR_DATA((lcd_dev.width - 1) >> 8);
    __LCD_WR_DATA((lcd_dev.width - 1) & 0xFF);
    __LCD_WR_REG(lcd_dev.setycmd);
    __LCD_WR_DATA(0);
    __LCD_WR_DATA(0);
    __LCD_WR_DATA((lcd_dev.height - 1) >> 8);
    __LCD_WR_DATA((lcd_dev.height - 1) & 0xFF);
}

static void lcd_display_direction(os_uint8_t dir)
{
    lcd_dev.dir = dir;
    if (dir == 0)
    {
        lcd_dev.width = 240;
        lcd_dev.height = 320;
    }
    else
    {
        lcd_dev.width = 320;
        lcd_dev.height = 240;
    }
    lcd_scan_direction(DFT_SCAN_DIR);
}

static void lcd_init(void)
{

    __LCD_WR_REG(0xCF);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0xC1);
    __LCD_WR_DATA(0x30);
    __LCD_WR_REG(0xED);
    __LCD_WR_DATA(0x64);
    __LCD_WR_DATA(0x03);
    __LCD_WR_DATA(0x12);
    __LCD_WR_DATA(0x81);
    __LCD_WR_REG(0xE8);
    __LCD_WR_DATA(0x85);
    __LCD_WR_DATA(0x10);
    __LCD_WR_DATA(0x7A);
    __LCD_WR_REG(0xCB);
    __LCD_WR_DATA(0x39);
    __LCD_WR_DATA(0x2C);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x34);
    __LCD_WR_DATA(0x02);
    __LCD_WR_REG(0xF7);
    __LCD_WR_DATA(0x20);
    __LCD_WR_REG(0xEA);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x00);
    /*Power control*/
    __LCD_WR_REG(0xC0);
    /*VRH[5:0]*/
    __LCD_WR_DATA(0x1B);
    __LCD_WR_REG(0xC1);
    __LCD_WR_DATA(0x01);
    __LCD_WR_REG(0xC5);
    __LCD_WR_DATA(0x30);
    __LCD_WR_DATA(0x30);
    __LCD_WR_REG(0xC7);
    __LCD_WR_DATA(0xB7);
    __LCD_WR_REG(0x36);
    __LCD_WR_DATA(0x48);
    __LCD_WR_REG(0x3A);
    __LCD_WR_DATA(0x55);
    __LCD_WR_REG(0xB1);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x1A);
    __LCD_WR_REG(0xB6);
    __LCD_WR_DATA(0x0A);
    __LCD_WR_DATA(0xA2);
    __LCD_WR_REG(0xF2);
    __LCD_WR_DATA(0x00);
    __LCD_WR_REG(0x26);
    __LCD_WR_DATA(0x01);
    __LCD_WR_REG(0xE0);
    __LCD_WR_DATA(0x0F);
    __LCD_WR_DATA(0x2A);
    __LCD_WR_DATA(0x28);
    __LCD_WR_DATA(0x08);
    __LCD_WR_DATA(0x0E);
    __LCD_WR_DATA(0x08);
    __LCD_WR_DATA(0x54);
    __LCD_WR_DATA(0xA9);
    __LCD_WR_DATA(0x43);
    __LCD_WR_DATA(0x0A);
    __LCD_WR_DATA(0x0F);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x00);
    __LCD_WR_REG(0xE1);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x15);
    __LCD_WR_DATA(0x17);
    __LCD_WR_DATA(0x07);
    __LCD_WR_DATA(0x11);
    __LCD_WR_DATA(0x06);
    __LCD_WR_DATA(0x2B);
    __LCD_WR_DATA(0x56);
    __LCD_WR_DATA(0x3C);
    __LCD_WR_DATA(0x05);
    __LCD_WR_DATA(0x10);
    __LCD_WR_DATA(0x0F);
    __LCD_WR_DATA(0x3F);
    __LCD_WR_DATA(0x3F);
    __LCD_WR_DATA(0x0F);
    __LCD_WR_REG(0x2B);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x01);
    __LCD_WR_DATA(0x3f);
    __LCD_WR_REG(0x2A);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0x00);
    __LCD_WR_DATA(0xef);
    __LCD_WR_REG(0x11);

    os_task_msleep(120);
    __LCD_WR_REG(0x29);
    lcd_display_direction(0);

    lcd_clear(WHITE);
}

static void lcd_back_light_set(os_uint8_t on)
{
    os_pin_mode(BSP_ATK9341_BL_PIN, PIN_MODE_OUTPUT);

    if (on)
        os_pin_write(BSP_ATK9341_BL_PIN, 1);
    else
        os_pin_write(BSP_ATK9341_BL_PIN, 0);
}

void lcd_set_color(os_uint16_t back, os_uint16_t fore)
{
    BACK_COLOR = back;
    FORE_COLOR = fore;
}

static void atk_tflcd9341_fill(struct os_device *dev, struct os_device_rect_info *rect)
{
    os_int32_t x_start = rect->x;
    os_int32_t y_start = rect->y;
    os_int32_t x_end   = rect->x + rect->width - 1;
    os_int32_t y_end   = rect->y + rect->height - 1;
    const char *color  = rect->color;

    os_uint16_t color16 = *color | (*(color + 1) << 8);
    lcd_fill(x_start, y_start, x_end, y_end, color16);

}

const static struct os_device_graphic_ops ops =
{
    .set_pixel  = OS_NULL,
    .get_pixel  = OS_NULL,

    .draw_hline = OS_NULL,
    .draw_vline = OS_NULL,

    .blit_line  = OS_NULL,

    .display_on = OS_NULL,
    .update     = OS_NULL,

    .fill       = atk_tflcd9341_fill,
};
static void __os_hw_lcd_init(void)
{
    lcd_back_light_set(1);

    lcd_read_id();

    lcd_init();
    
    lcd_set_color(WHITE, BLACK);
}
static int os_hw_lcd_init(void)
{
    struct atk_lcd *lcd;

    lcd = os_calloc(1, sizeof(struct atk_lcd));

    OS_ASSERT(lcd);

    __os_hw_lcd_init();

    lcd->graphic.info.width            = lcd_dev.width;
    lcd->graphic.info.height           = lcd_dev.height;
    lcd->graphic.info.pixel_format     = OS_GRAPHIC_PIXEL_FORMAT_RGB565;
    lcd->graphic.info.bits_per_pixel   = 16;

    lcd->graphic.ops = &ops;

    os_graphic_register("lcd", &lcd->graphic);

    LOG_I(DBG_TAG, "atk tflcd9341 found.");

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_lcd_init, OS_INIT_SUBLEVEL_LOW);
