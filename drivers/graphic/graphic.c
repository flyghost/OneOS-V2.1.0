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
#include <graphic/graphic.h>

static os_err_t os_graphic_control(os_device_t *device, int cmd, void *args)
{
    os_device_graphic_t *graphic;

    OS_ASSERT(device);

    graphic = (os_device_graphic_t *)device;

    switch (cmd)
    {        
    case OS_GRAPHIC_CTRL_RECT_UPDATE:
        if (graphic->ops->update)
        {
            graphic->ops->update(device, (struct os_device_rect_info *)args);
        }
        break;

    case OS_GRAPHIC_CTRL_POWERON:
        if (graphic->ops->display_on)
        {
            graphic->ops->display_on(device, OS_TRUE);
        }
        break;

    case OS_GRAPHIC_CTRL_POWEROFF:
        if (graphic->ops->display_on)
        {
            graphic->ops->display_on(device, OS_FALSE);
        }
        break;

    case OS_GRAPHIC_CTRL_GET_INFO:
        memcpy(args, &graphic->info, sizeof(struct os_device_graphic_info));
        break;

    case OS_GRAPHIC_CTRL_SET_MODE:
        break;

    case OS_GRAPHIC_CTRL_GET_EXT:
        break;

    case OS_GRAPHIC_CTRL_SET_PIXEL:
        if (graphic->ops->set_pixel)
        {
            struct os_graphic_pixel *pixel = (struct os_graphic_pixel *)args;
            graphic->ops->set_pixel(device, pixel->pixel, pixel->x, pixel->y);
        }
        break;

    case OS_GRAPHIC_CTRL_FILL:
        if (graphic->ops->fill)
        {
            graphic->ops->fill(device, (struct os_device_rect_info *)args);
        }
        break;
    }

    return OS_EOK;
}

const static struct os_device_ops graphic_ops = 
{
    .control = os_graphic_control,
};

void os_graphic_register(const char *name, os_device_graphic_t *graphic)
{
    OS_ASSERT(graphic != OS_NULL);
    OS_ASSERT(graphic->ops != OS_NULL);

    graphic->parent.type = OS_DEVICE_TYPE_GRAPHIC;
    graphic->parent.ops  = &graphic_ops;
    os_device_register(&graphic->parent, name);
}

#ifdef OS_USING_SHELL

#include <shell.h>

static int graphic_fill(os_device_graphic_t *graphic, os_int32_t color)
{
    int i, y;
    os_int8_t  *line_buff;
    os_int8_t  *line_buff_8;
    os_int16_t *line_buff_16;
    os_int32_t *line_buff_32;

    line_buff = os_calloc(1, (graphic->info.bits_per_pixel / 8) * graphic->info.width);
    if (line_buff == OS_NULL)
    {
        os_kprintf("graphic test malloc failed\r\n");
        return -1;
    }

    line_buff_8  = (os_int8_t  *)line_buff;
    line_buff_16 = (os_int16_t *)line_buff;
    line_buff_32 = (os_int32_t *)line_buff;

    if (graphic->info.bits_per_pixel == 32)
    {
        for (i = 0; i < graphic->info.width; i++)
            *line_buff_32++ = color;
    }
    else if (graphic->info.bits_per_pixel == 16)
    {
        for (i = 0; i < graphic->info.width; i++)
            *line_buff_16++ = color;
    }
    else if (graphic->info.bits_per_pixel == 8)
    {
        memset(line_buff_8, color, graphic->info.width);
    }
    else
    {
        os_kprintf("graphic test invalid pixel %d.\r\n", graphic->info.bits_per_pixel);
        os_free(line_buff);
        return -1;
    }
        
    for (y = 0; y < graphic->info.height; y++)
    {
        struct os_device_rect_info rect;

        rect.x = 0;
        rect.width = graphic->info.width;
        rect.y = y;
        rect.height = 1;
        rect.color = (char *)line_buff;
                        
        os_device_control(&graphic->parent, OS_GRAPHIC_CTRL_FILL, &rect);
    }

    os_free(line_buff);
    return 0;
}

static int graphic_test_888(os_device_graphic_t *graphic)
{
    /* red */
    os_kprintf("red\r\n");
    graphic_fill(graphic, 0xffff0000);
    os_task_msleep(OS_TICK_PER_SECOND);

    /* green */
    os_kprintf("green\r\n");
    graphic_fill(graphic, 0xff00ff00);
    os_task_tsleep(OS_TICK_PER_SECOND);

    /* blue */
    os_kprintf("blue\r\n");
    graphic_fill(graphic, 0xff0000ff);
    os_task_tsleep(OS_TICK_PER_SECOND);

    /* black */
    os_kprintf("black\r\n");
    graphic_fill(graphic, 0x00000000);
    os_task_tsleep(OS_TICK_PER_SECOND);

    /* white */
    os_kprintf("white\r\n");
    graphic_fill(graphic, 0xffffffff);
    os_task_tsleep(OS_TICK_PER_SECOND);
    
    return 0;
}

static int graphic_test_565(os_device_graphic_t *graphic)
{    
    /* red */
    os_kprintf("red\r\n");
    graphic_fill(graphic, 0xf800);
    os_task_tsleep(OS_TICK_PER_SECOND);

    /* green */
    os_kprintf("green\r\n");
    graphic_fill(graphic, 0x07e0);
    os_task_tsleep(OS_TICK_PER_SECOND);

    /* blue */
    os_kprintf("blue\r\n");
    graphic_fill(graphic, 0x001f);
    os_task_tsleep(OS_TICK_PER_SECOND);

    /* black */
    os_kprintf("black\r\n");
    graphic_fill(graphic, 0x0000);
    os_task_tsleep(OS_TICK_PER_SECOND);

    /* white */
    os_kprintf("white\r\n");
    graphic_fill(graphic, 0xffff);
    os_task_tsleep(OS_TICK_PER_SECOND);
    
    return 0;
}

static os_err_t graphic_test(int argc, char **argv)
{
    if (argc != 2)
    {
        os_kprintf("usage: graphic_test <dev> \r\n");
        os_kprintf("       graphic_test lcd \r\n");
        return -1;
    }

    os_device_graphic_t *graphic;
    graphic = (os_device_graphic_t *)os_device_find(argv[1]);

    if (graphic == NULL || graphic->parent.type != OS_DEVICE_TYPE_GRAPHIC)
    {
        os_kprintf("invalide graphic device [%s].\r\n", argv[1]);
        return OS_EINVAL;
    }

    if (graphic->info.pixel_format == OS_GRAPHIC_PIXEL_FORMAT_ARGB888)
    {
        return graphic_test_888(graphic);
    }

    if (graphic->info.pixel_format == OS_GRAPHIC_PIXEL_FORMAT_RGB565)
    {
        return graphic_test_565(graphic);
    }
    
    return OS_EINVAL;
}

SH_CMD_EXPORT(graphic_test, graphic_test, "graphic_test");

#endif

