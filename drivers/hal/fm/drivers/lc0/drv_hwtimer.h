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
 * @file        drv_hwtimer.h
 *
 * @brief       This file provides functions declaration for fm33 timer driver.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_HWTIMER_H__
#define __DRV_HWTIMER_H__

#include <board.h>
#include <os_task.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

typedef enum 
{
    TYPE_ATIM,
    TYPE_BTIM,
    TYPE_GPTIM,
    TYPE_LPTIM,
}hwtimer_type_t;

#define TIMER_MODE_TIM              0x00
#define TIMER_MODE_PWM              0x01
#define TIMER_MODE_PULSE_ENCODER    0x02

struct fm33_timer_info
{
    void          *instance;
    os_uint8_t     index;
    IRQn_Type      irqn;
    hwtimer_type_t type;
    os_uint8_t     mode;
    os_uint8_t     bits;
};

struct fm33_timer 
{
    union _clock 
    {
        os_clocksource_t    cs;
        os_clockevent_t     ce;
    } clock;

    struct fm33_timer_info *info;

    os_uint32_t             freq;

    os_list_node_t          list;
};

#endif
