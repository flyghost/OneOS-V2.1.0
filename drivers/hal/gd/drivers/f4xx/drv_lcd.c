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
 * @file        drv_lcd_mipi.c
 *
 * @brief       This file implements lcd mipi driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <string.h>
#include <os_sem.h>
#include <os_memory.h>
#include <os_irq.h>
#include <graphic/graphic.h>
#include <graphic/dsi.h>
#include <lcd_port.h>
#include <shell.h>
#include "drv_gpio.h"

#define LOG_TAG "drv.lcd"
#include <drv_log.h>

struct stm32_lcd {
    os_device_graphic_t graphic;

    LTDC_HandleTypeDef *hltdc;

#ifdef OS_USING_DSI
    os_device_dsi_t    *dsi;
#endif
};

static os_err_t ltdc_init(struct stm32_lcd *lcd)
{
    OS_ASSERT(lcd != OS_NULL);

#ifdef OS_USING_DSI

    lcd->dsi = (os_device_dsi_t *)os_device_find("dsi");

    if (lcd->dsi != OS_NULL)
    {
        os_dsi_start(lcd->dsi);
    }

#endif

    return OS_EOK;
}

static void ltdc_layer_init(struct stm32_lcd *lcd)
{
    int i, layer_nr;

    LTDC_LayerCfgTypeDef layer_cfg;

    layer_nr = ARRAY_SIZE(lcd->hltdc->LayerCfg);

    for (i = 0; i < layer_nr; i++)
    {
        layer_cfg = lcd->hltdc->LayerCfg[i];

        if (layer_cfg.ImageWidth == 0 || layer_cfg.ImageHeight == 0)
            continue;

        layer_cfg.FBStartAdress = (uint32_t)lcd->graphic.info.framebuffer;

        HAL_LTDC_ConfigLayer(lcd->hltdc, &layer_cfg, i);
    }
}

static os_err_t stm32_lcd_init(struct stm32_lcd *lcd)
{
    lcd->graphic.info.width            = LCD_WIDTH;
    lcd->graphic.info.height           = LCD_HEIGHT;
    lcd->graphic.info.pixel_format     = LCD_PIXEL_FORMAT;
    lcd->graphic.info.bits_per_pixel   = LCD_BITS_PER_PIXEL;
    lcd->graphic.info.framebuffer_size = LCD_WIDTH * LCD_HEIGHT * LCD_BITS_PER_PIXEL / 8;
    lcd->graphic.info.framebuffer      = (void *)os_malloc_align(LCD_WIDTH * LCD_HEIGHT * (LCD_BITS_PER_PIXEL / 8), 32);

    OS_ASSERT(lcd->graphic.info.framebuffer);
    
    memset(lcd->graphic.info.framebuffer, 0, LCD_WIDTH * LCD_HEIGHT * (LCD_BITS_PER_PIXEL / 8));
    
    ltdc_init(lcd);
    ltdc_layer_init(lcd);

    return OS_EOK;
}

struct os_device_graphic_ops ops =
{
    .set_pixel  = OS_NULL,
    .get_pixel  = OS_NULL,

    .draw_hline = OS_NULL,
    .draw_vline = OS_NULL,

    .blit_line  = OS_NULL,

    .display_on = OS_NULL,
    .update     = OS_NULL,
};

static int stm32_lcd_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    struct stm32_lcd *lcd;

    lcd = os_calloc(1, sizeof(struct stm32_lcd));

    OS_ASSERT(lcd);

    lcd->hltdc = (LTDC_HandleTypeDef *)dev->info;

    stm32_lcd_init(lcd);

    lcd->graphic.ops = &ops;

    os_graphic_register("lcd", &lcd->graphic);

    os_kprintf("stm32 lcd found.\r\n");

    return OS_EOK;
}

OS_DRIVER_INFO stm32_lcd_driver = {
    .name   = "LTDC_HandleTypeDef",
    .probe  = stm32_lcd_probe,
};

OS_DRIVER_DEFINE(stm32_lcd_driver, "3");

