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
 * @file        drv_hwtimer.c
 *
 * @brief       This file implements timer driver for mm32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-31   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mm32_hal.h"
#include "mm32_it.h"
#include <drv_common.h>
#include <drv_hwtimer.h>

#include <os_memory.h>
#include <timer/timer.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>

struct mm32_timer {
    union _clock {
        os_clocksource_t   cs;
        os_clockevent_t    ce;
    } clock;

    struct mm32_timer_info *info;

    os_uint32_t  freq;
    os_list_node_t list;
};
static os_list_node_t mm32_timer_list = OS_LIST_INIT(mm32_timer_list);

os_bool_t mm32_timer_is_32b(struct mm32_timer *timer)
{
#ifdef TIM2
    if (timer->info->htim == TIM2)
        return OS_TRUE;
#endif
#ifdef TIM5
    if (timer->info->htim == TIM5)
        return OS_TRUE;
#endif
    else
        return OS_FALSE;
}

static uint32_t mm32_timer_get_freq(struct mm32_timer *timer)
{
    os_uint32_t clk = 0;
    RCC_ClocksTypeDef RCC_Clocks;
    
    OS_ASSERT(timer->info->htim != OS_NULL);
    
    RCC_GetClocksFreq(&RCC_Clocks);
    
    clk = RCC_Clocks.PCLK1_Frequency;
    
#ifdef TIM1
    if (timer->info->htim == TIM1)
        clk = RCC_Clocks.PCLK2_Frequency;
#endif
#ifdef TIM8
    if (timer->info->htim == TIM8)
        clk = RCC_Clocks.PCLK2_Frequency;
#endif
    
    return clk;
}

static void mm32_timer_hard_init(struct mm32_timer *timer)
{
#ifdef TIM1
    if (timer->info->htim == TIM1)
        RCC_APB2PeriphClockCmd(timer->info->tim_clk, ENABLE);
#endif
#ifdef TIM8
    if (timer->info->htim == TIM8)
        RCC_APB2PeriphClockCmd(timer->info->tim_clk, ENABLE);
#endif
    else
        RCC_APB1PeriphClockCmd(timer->info->tim_clk, ENABLE);
}

static void HAL_TIM_PeriodElapsedCallback(struct mm32_timer *timer)
{
    if (TIM_GetITStatus(timer->info->htim, TIM_IT_Update))
    if ((TIM_GetITStatus(timer->info->htim, TIM_IT_Update) != RESET)
        && ((timer->info->htim->DIER & TIM_DIER_UIE) != RESET))
    {
        TIM_ClearITPendingBit(timer->info->htim, TIM_IT_Update);
        os_clockevent_isr((os_clockevent_t *)timer);
    }
}

void HAL_TIM_IRQHandler(TIM_TypeDef *htim)
{
    struct mm32_timer *timer;
    os_list_for_each_entry(timer, &mm32_timer_list, struct mm32_timer, list)
    {
        if (timer->info->htim == htim)
        {
            HAL_TIM_PeriodElapsedCallback(timer);
            return;
        }
    }
}

#if defined(OS_USING_CLOCKSOURCE) || defined(OS_USING_CLOCKEVENT)
static os_uint64_t mm32_timer_read(void *clock)
{
    struct mm32_timer *timer;

    timer = (struct mm32_timer *)clock;

    return TIM_GetCounter(timer->info->htim);
}
#endif

#ifdef OS_USING_CLOCKEVENT
static void mm32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct mm32_timer *timer;

    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer = (struct mm32_timer *)ce;

    TIM_TypeDef *htim = timer->info->htim;

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitdata;
    TIM_TimeBaseInitdata.TIM_Prescaler = prescaler - 1;
    TIM_TimeBaseInitdata.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitdata.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitdata.TIM_Period = count;
    TIM_TimeBaseInitdata.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(htim, &TIM_TimeBaseInitdata);
    TIM_ARRPreloadConfig(htim, ENABLE);

    TIM_ClearITPendingBit(htim, TIM_IT_Update);
    TIM_ITConfig(htim, TIM_IT_Update, ENABLE);
    TIM_Cmd(htim, ENABLE);
}

static void mm32_timer_stop(os_clockevent_t *ce)
{
    struct mm32_timer *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct mm32_timer *)ce;

    TIM_TypeDef *htim = timer->info->htim;

    TIM_ITConfig(htim, TIM_IT_Update, DISABLE);
    TIM_Cmd(htim, DISABLE);
    TIM_ClearITPendingBit(htim, TIM_IT_Update);
}

static const struct os_clockevent_ops mm32_tim_ops =
{
    .start = mm32_timer_start,
    .stop  = mm32_timer_stop,
    .read  = mm32_timer_read,
};
#endif

static int mm32_tim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct mm32_timer *timer;

    timer = os_calloc(1, sizeof(struct mm32_timer));
    OS_ASSERT(timer);

    timer->info = (struct mm32_timer_info *)dev->info;
    
    mm32_timer_hard_init(timer);

    timer->freq = mm32_timer_get_freq(timer);

#ifdef OS_USING_CLOCKSOURCE
    if (os_clocksource_best() == OS_NULL && mm32_timer_is_32b(timer))
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseInitdata;
        TIM_TimeBaseInitdata.TIM_Prescaler = 0x0;
        TIM_TimeBaseInitdata.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseInitdata.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInitdata.TIM_Period = 0xfffful;
        TIM_TimeBaseInitdata.TIM_RepetitionCounter = 0x00;
        TIM_TimeBaseInit(timer->info->htim, &TIM_TimeBaseInitdata);
        TIM_ARRPreloadConfig(timer->info->htim, ENABLE);

        TIM_Cmd(timer->info->htim, ENABLE);
    
        timer->clock.cs.rating  = 320;
        timer->clock.cs.freq    = timer->freq;
        timer->clock.cs.mask    = 0xfffful;
        timer->clock.cs.read    = mm32_timer_read;

        os_clocksource_register(dev->name, &timer->clock.cs);
    }
    else
#endif
    {
#ifdef OS_USING_CLOCKEVENT
        NVIC_Init(&timer->info->NVIC_InitStructure);

        timer->clock.ce.rating  = mm32_timer_is_32b(timer) ? 320 : 160;
        timer->clock.ce.freq    = timer->freq;
        timer->clock.ce.mask    = 0xffffffffull;
        
        timer->clock.ce.prescaler_mask = 0xfffful;
        timer->clock.ce.prescaler_bits = 16;
        
        timer->clock.ce.count_mask = mm32_timer_is_32b(timer) ? 0xfffffffful : 0xfffful;
        timer->clock.ce.count_bits = mm32_timer_is_32b(timer) ? 32 : 16;

        timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

        timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
        
        timer->clock.ce.ops     = &mm32_tim_ops;
        os_clockevent_register(dev->name, &timer->clock.ce);
#endif
    }

    os_list_add(&mm32_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO mm32_tim_driver = {
    .name   = "TIM_TypeDef",
    .probe  = mm32_tim_probe,
};

OS_DRIVER_DEFINE(mm32_tim_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);

