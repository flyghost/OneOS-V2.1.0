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
 * @file        drv_lcd.c
 *
 * @brief       This file implements lcd driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <string.h>
#include <os_memory.h>
#include <lcd_port.h>
#include <graphic/graphic.h>
#include <shell.h>
#include "drv_gpio.h"
#define LOG_TAG "drv.lcd"
#include <drv_log.h>

#define LCD_BL_GPIO      GPIO2
#define LCD_BL_GPIO_PIN  31

struct imxrt_lcd {
    os_device_graphic_t graphic;
    struct nxp_lcdif_info *lcdif;
};

static volatile os_uint8_t imxrt_lcd_frame_done;

void LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = ELCDIF_GetInterruptStatus(LCDIF);

    ELCDIF_ClearInterruptStatus(LCDIF, intStatus);

    if (intStatus & kELCDIF_CurFrameDone)
    {
        imxrt_lcd_frame_done = 1;
    }
    
    SDK_ISR_EXIT_BARRIER;
    __DSB();
}

static void imxrt_lcd_fill(struct os_device *dev, struct os_device_rect_info *rect)
{
    os_device_graphic_t *graphic = (os_device_graphic_t *)dev;
    struct os_device_graphic_info *info = &graphic->info;

    DCACHE_CleanInvalidateByRange((uint32_t)(rect->color), 480*272*2);
    ELCDIF_SetNextBufferAddr(LCDIF, (uint32_t)(rect->color));

    imxrt_lcd_frame_done = 0;
    while(!imxrt_lcd_frame_done)
    {
    }
}

void imxrt_lcd_display_on(struct os_device *dev, os_bool_t on_off)
{
    GPIO_PinWrite(LCD_BL_GPIO, LCD_BL_GPIO_PIN, on_off);
}

const static struct os_device_graphic_ops ops =
{
    .set_pixel  = OS_NULL,
    .get_pixel  = OS_NULL,

    .draw_hline = OS_NULL,
    .draw_vline = OS_NULL,

    .blit_line  = OS_NULL,

    .display_on = imxrt_lcd_display_on,
    .update     = OS_NULL,

    .fill       = imxrt_lcd_fill,
};

static int imxrt_lcd_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    struct imxrt_lcd *lcd;

    lcd = os_calloc(1, sizeof(struct imxrt_lcd));
    OS_ASSERT(lcd);

    lcd->lcdif = (struct nxp_lcdif_info *)dev->info;
    lcd->graphic.info.width            = LCD_WIDTH;
    lcd->graphic.info.height           = LCD_HEIGHT;
    lcd->graphic.info.pixel_format     = LCD_PIXEL_FORMAT;
    lcd->graphic.info.bits_per_pixel   = LCD_BITS_PER_PIXEL;
//    lcd->graphic.info.framebuffer      = (void *)lcd->lcdif->config->bufferAddr;
    lcd->graphic.info.framebuffer_avail= 1;
    lcd->graphic.info.framebuffer_num  = 2;
    lcd->graphic.info.framebuffer_size = LCD_WIDTH * LCD_HEIGHT * LCD_BITS_PER_PIXEL / 8;

    //Enable IRQ
    EnableIRQ(LCDIF_IRQn);
    //RGB mode start
    ELCDIF_RgbModeStart(LCDIF_PERIPHERAL);
    //Backlight
    const gpio_pin_config_t config = {
        kGPIO_DigitalOutput,
        1,
        kGPIO_NoIntmode,
    };
    GPIO_PinInit(LCD_BL_GPIO, LCD_BL_GPIO_PIN, &config);
  
    lcd->graphic.ops = &ops;
    os_graphic_register("lcd", &lcd->graphic);

    os_kprintf("imxrt lcd found.\r\n");

    return OS_EOK;
}

OS_DRIVER_INFO imxrt_lcd_driver = {
    .name   = "LCDIF_Type",
    .probe  = imxrt_lcd_probe,
};

OS_DRIVER_DEFINE(imxrt_lcd_driver, DEVICE, OS_INIT_SUBLEVEL_MIDDLE);



