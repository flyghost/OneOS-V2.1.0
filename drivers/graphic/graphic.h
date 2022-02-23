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

#ifndef _GRAPHIC_H_
#define _GRAPHIC_H_

#include <board.h>
#include <drv_cfg.h>

#define OS_GRAPHIC_CTRL_RECT_UPDATE      0  /* update graphic device rect area */
#define OS_GRAPHIC_CTRL_POWERON          1  /* power on graphic device */
#define OS_GRAPHIC_CTRL_POWEROFF         2  /* power off graphic device */
#define OS_GRAPHIC_CTRL_GET_INFO         3  /* get graphic device info */
#define OS_GRAPHIC_CTRL_SET_MODE         4  /* set graphic device mode */
#define OS_GRAPHIC_CTRL_GET_EXT          5  /* get graphic device extern info */
#define OS_GRAPHIC_CTRL_SET_PIXEL        6  /* set graphic device pixel */
#define OS_GRAPHIC_CTRL_FILL             7  /* fill graphic device */

enum
{
    OS_GRAPHIC_PIXEL_FORMAT_INVALID = 0,
    OS_GRAPHIC_PIXEL_FORMAT_MONO,
    OS_GRAPHIC_PIXEL_FORMAT_GRAY4,
    OS_GRAPHIC_PIXEL_FORMAT_GRAY16,
    OS_GRAPHIC_PIXEL_FORMAT_RGB332,
    OS_GRAPHIC_PIXEL_FORMAT_RGB444,
    OS_GRAPHIC_PIXEL_FORMAT_RGB565,
    OS_GRAPHIC_PIXEL_FORMAT_RGB565P,
    OS_GRAPHIC_PIXEL_FORMAT_BGR565 = OS_GRAPHIC_PIXEL_FORMAT_RGB565P,
    OS_GRAPHIC_PIXEL_FORMAT_RGB666,
    OS_GRAPHIC_PIXEL_FORMAT_RGB888,
    OS_GRAPHIC_PIXEL_FORMAT_ARGB888,
    OS_GRAPHIC_PIXEL_FORMAT_ABGR888,
    OS_GRAPHIC_PIXEL_FORMAT_ARGB565,
    OS_GRAPHIC_PIXEL_FORMAT_ALPHA,
    OS_GRAPHIC_PIXEL_FORMAT_COLOR,
};

#define OS_GRAPHIC_PIXEL_POSITION(x, y)  ((x << 16) | y)

struct os_device_graphic_info
{
    os_uint8_t  pixel_format;       /* Graphic format. */
    os_uint8_t  bits_per_pixel;     /* Bits per pixel. */
    os_uint16_t reserved;           /* Reserved field. */

    os_uint16_t width;              /* Width of graphic device. */
    os_uint16_t height;             /* Height of graphic device. */

    os_uint8_t *framebuffer;        /* Frame buffer pointer */
    os_uint8_t  framebuffer_avail;  /* Frame buffer is available for gui task or not */
    os_uint8_t  framebuffer_num;    /* Frame buffer number */
    os_uint32_t framebuffer_size;   /* Size of each frame buffer */
};

struct os_device_rect_info
{
    os_uint16_t x;                  /* x coordinate. */
    os_uint16_t y;                  /* y coordinate. */
    os_uint16_t width;              /* Width. */
    os_uint16_t height;             /* Height. */
    char *color;
};

struct os_graphic_pixel {
    char *pixel;
    os_int32_t x;
    os_int32_t y;
};

struct os_device_graphic_ops
{
    void (*set_pixel) (struct os_device *dev, const char *pixel, os_int32_t x, os_int32_t y);
    void (*get_pixel) (struct os_device *dev, char *pixel, os_int32_t x, os_int32_t y);

    void (*draw_hline)(struct os_device *dev, const char *pixel, os_int32_t x1, os_int32_t x2, os_int32_t y);
    void (*draw_vline)(struct os_device *dev, const char *pixel, os_int32_t x, os_int32_t y1, os_int32_t y2);

    void (*blit_line) (struct os_device *dev, const char *pixel, os_int32_t x, os_int32_t y, os_size_t size);

    void (*display_on)(struct os_device *dev, os_bool_t on_off);

    void (*update)(struct os_device *dev, struct os_device_rect_info *rect);

    void (*fill)(struct os_device *dev, struct os_device_rect_info *rect);
};

#define os_graphix_ops(device)          ((struct os_device_graphic_ops *)(device->user_data))

typedef struct os_device_graphic {
    os_device_t parent;
    
    struct os_device_graphic_info info;
    const struct os_device_graphic_ops *ops;
} os_device_graphic_t;

void os_graphic_register(const char *name, os_device_graphic_t *graphic);

#endif /* _GRAPHIC_H_ */

