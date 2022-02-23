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
 * @file        usbd_hid_test.c
 *
 * @brief       The test file for usbd hid.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <shell.h>
#include <stdint.h>

static int x      = 0;
static int y      = 0;
static int left   = 0;
static int right  = 0;
static int scroll = 0;

static void left_callback(void *args)
{
    left = 1;
    os_kprintf("left button click\r\n");
}

static void right_callback(void *args)
{
    right = 1;
    os_kprintf("right button click\r\n");
}

static void update_mouse(os_device_t *device)
{
    uint8_t report[4];

    if ((left | right) != 0)
    {
        report[0] = 0x08 | left | right << 1;
    }
    else
    {
        report[0] = 0;
    }

    report[1] = x;
    report[2] = y;
    report[3] = scroll;

    os_device_write_nonblock(device, HID_REPORT_ID_MOUSE, report, 4);

    left  = 0;
    right = 0;
}

static int usbd_hid_test(int argc, char **argv)
{
    int          i, j;
    os_device_t *device = os_device_find("hidd");
    OS_ASSERT(device != OS_NULL);
    os_device_open(device);

    /* Mouse left button */
    if (key_table_size > 0)
    {
        os_pin_mode(key_table[0].pin, key_table[0].mode);
        os_pin_attach_irq(key_table[0].pin, key_table[0].irq_mode, left_callback, NULL);
        os_pin_irq_enable(key_table[0].pin, PIN_IRQ_ENABLE);
    }

    /* Mouse right button */
    if (key_table_size > 1)
    {
        os_pin_mode(key_table[1].pin, key_table[1].mode);
        os_pin_attach_irq(key_table[1].pin, key_table[1].irq_mode, right_callback, NULL);
        os_pin_irq_enable(key_table[1].pin, PIN_IRQ_ENABLE);
    }

    for (j = 0; j < 5; j++)
    {
        os_kprintf("loop %d/5\r\n", j + 1);

        for (i = 0; i < 20; i++)
        {
            x = 10;
            y = 0;
            update_mouse(device);
            os_task_msleep(100);
        }

        for (i = 0; i < 20; i++)
        {
            x = 0;
            y = 10;
            update_mouse(device);
            os_task_msleep(100);
        }

        for (i = 0; i < 20; i++)
        {
            x = -10;
            y = 0;
            update_mouse(device);
            os_task_msleep(100);
        }

        for (i = 0; i < 20; i++)
        {
            x = 0;
            y = -10;
            update_mouse(device);
            os_task_msleep(100);
        }
    }

    return 0;
}

SH_CMD_EXPORT(usbd_hid_test, usbd_hid_test, "usbd_hid_test");
