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
 * @brief       This file provides functions declaration for STM32 timer driver.
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

struct stm32_timer {
    union _clock {
        os_clocksource_t   cs;
        os_clockevent_t    ce;
    } clock;
    
    TIM_HandleTypeDef *handle;
    os_uint32_t freq;
    os_list_node_t list;
    void *user_data;
    os_uint8_t mode;
};

os_bool_t stm32_timer_is_32b(TIM_TypeDef *Instance);

#endif
