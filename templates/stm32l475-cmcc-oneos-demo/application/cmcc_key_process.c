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
 * @file        cmcc_key_process.c
 *
 * @brief       Use keys to control the board peripheral devices
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <stdint.h>
#include <device.h>
#include <os_event.h>
#include <stdio.h>

#include <drv_gpio.h>
#include <board.h>
#include <shell.h>


#include "cmcc_key_process.h"
#include "cmcc_lcd_process.h"
#include "cmcc_sensor_process.h"

#undef DBG_SECTION_NAME
#undef DBG_LEVEL
#undef DBG_COLOR
#undef DBG_ENABLE

#define KEY2   GET_PIN(C, 13)
#define KEY1   GET_PIN(D, 10)
#define KEY4   GET_PIN(D, 9)
#define KEY3   GET_PIN(D, 8)
#define BEEP   GET_PIN(B, 2)

static struct os_event beep_event;

static void pin_callback(void *args)
{
    if((int)(unsigned long)args == KEY1)
    {
        cmcc_lcd_event_put(LCD_EVENT_SENSOR);
        os_event_send(&beep_event,EVENT_BEEP);
    }

    if((int)(unsigned long)args == KEY2)
    {
        cmcc_lcd_event_put(LCD_EVENT_INFO);
        os_event_send(&beep_event,EVENT_BEEP);
    }

    if((int)(unsigned long)args == KEY4)
    {
        cmcc_lcd_event_put(LCD_EVENT_INDEX);
        os_event_send(&beep_event,EVENT_BEEP);
    }

    if((int)(unsigned long)args == KEY3)
    {
        cmcc_lcd_event_put(LCD_EVENT_TIME);
        os_event_send(&beep_event,EVENT_BEEP);
    }
}

static void beep_ctrl(void *args)
{
    os_uint32_t event = 0;
    os_err_t rst = OS_EOK;
    while(1)
    {
        rst = os_event_recv(&beep_event,
                            EVENT_BEEP,
                            OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                            OS_WAIT_FOREVER, (os_uint32_t *)&event);

        if (rst == OS_EOK)
        {
            if (event & EVENT_BEEP)
            {
                os_pin_write(BEEP, PIN_HIGH);
                os_task_msleep(200);
                os_pin_write(BEEP, PIN_LOW);
            }
        }
    }
}


void cmcc_key_init(void)
{
    int i;
    os_task_t *cmcc_beep_ctl;

    for (i = 0; i < key_table_size; i++)
    {
        os_pin_mode(key_table[i].pin, key_table[i].mode);
        os_pin_attach_irq(key_table[i].pin, key_table[i].irq_mode, pin_callback, (void *)key_table[i].pin);
        os_pin_irq_enable(key_table[i].pin, PIN_IRQ_ENABLE);
    }

    os_pin_mode(BEEP, PIN_MODE_OUTPUT);

    os_event_init(&beep_event, "beep_event");

    /* create  task 'cmcc_beep_ctl'*/
    cmcc_beep_ctl = os_task_create("beep_task",
                                   beep_ctrl,
                                   OS_NULL,
                                   CMCC_BEEP_TASK_STACK_SIZE, OS_TASK_PRIORITY_MAX / 2 - 4);

    if (cmcc_beep_ctl != OS_NULL)
    {
        os_task_startup(cmcc_beep_ctl);
    }
    
}
