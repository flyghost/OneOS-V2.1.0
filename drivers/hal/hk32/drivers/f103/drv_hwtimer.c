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
 * @brief       This file implements timer driver for hk32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <timer/timer.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#include <drv_common.h>
#include <drv_hwtimer.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>

static os_list_node_t hk32_timer_list = OS_LIST_INIT(hk32_timer_list);

os_bool_t hk32_timer_is_32b(TIM_TypeDef *Instance)
{
    return OS_FALSE;
}

static uint32_t hk32_timer_get_freq(TIM_TypeDef *htim)
{
    OS_ASSERT(htim != OS_NULL);

    return 72 * 1000000;
}

static void HAL_TIM_PeriodElapsedCallback(struct hk32_timer *timer)
{
    if (TIM_GetITStatus(timer->info->htim, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(timer->info->htim, TIM_IT_Update);
        os_clockevent_isr((os_clockevent_t *)timer);
    }
}

static void TIMn_IRQHandler(TIM_TypeDef *htim)
{
    struct hk32_timer *timer;
    os_list_for_each_entry(timer, &hk32_timer_list, struct hk32_timer, list)
    {
        if (timer->info->htim == htim)
        {
            HAL_TIM_PeriodElapsedCallback(timer);
            return;
        }
    }
}

void TIM2_IRQHandler(void)
{
#ifdef TIM2
    TIMn_IRQHandler(TIM2);
#endif
}

void TIM3_IRQHandler(void)
{
#ifdef TIM3
    TIMn_IRQHandler(TIM3);
#endif
}

void TIM4_IRQHandler(void)
{
#ifdef TIM4
    TIMn_IRQHandler(TIM4);
#endif
}

void TIM6_IRQHandler(void)
{
#ifdef TIM6
    TIMn_IRQHandler(TIM6);
#endif
}

#if defined(OS_USING_CLOCKSOURCE) || defined(OS_USING_CLOCKEVENT)
static os_uint64_t hk32_timer_read(void *clock)
{
    struct hk32_timer *timer;

    timer = (struct hk32_timer *)clock;

    return TIM_GetCounter(timer->info->htim);
}

#endif

#ifdef OS_USING_CLOCKEVENT
static void hk32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct hk32_timer *timer;

    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer = (struct hk32_timer *)ce;

    TIM_TypeDef *htim = timer->info->htim;

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitdata;
    TIM_TimeBaseInitdata.TIM_Prescaler = prescaler;
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

static void hk32_timer_stop(os_clockevent_t *ce)
{
    struct hk32_timer *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct hk32_timer *)ce;

    TIM_TypeDef *htim = timer->info->htim;

    TIM_ITConfig(htim, TIM_IT_Update, DISABLE);
    TIM_Cmd(htim, DISABLE);
    TIM_ClearITPendingBit(htim, TIM_IT_Update);
}

static const struct os_clockevent_ops hk32_tim_ops =
{
    .start = hk32_timer_start,
    .stop  = hk32_timer_stop,
    .read  = hk32_timer_read,
};
#endif

static int hk32_tim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct hk32_timer *timer;

    timer = os_calloc(1, sizeof(struct hk32_timer));
    OS_ASSERT(timer);

    timer->info = (const struct hk32_timer_info *)dev->info;
    
    TIM_TypeDef *htim = timer->info->htim;

    timer->freq = hk32_timer_get_freq(htim);

    if (timer->info->rcc_tpye == HK32_RCC_APB1)
    {
        RCC_APB1PeriphClockCmd(timer->info->rcc, ENABLE);
        RCC_APB1PeriphResetCmd(timer->info->rcc, ENABLE);
        RCC_APB1PeriphResetCmd(timer->info->rcc, DISABLE);
    }

    if (timer->info->rcc_tpye == HK32_RCC_APB2)
    {
        RCC_APB2PeriphClockCmd(timer->info->rcc, ENABLE);
        RCC_APB2PeriphResetCmd(timer->info->rcc, ENABLE);
        RCC_APB2PeriphResetCmd(timer->info->rcc, DISABLE);
    }

#ifdef OS_USING_CLOCKSOURCE
    if (os_clocksource_best() == OS_NULL && hk32_timer_is_32b(htim))
    {
        TIM_TimeBaseInitTypeDef TIM_TimeBaseInitdata;
        TIM_TimeBaseInitdata.TIM_Prescaler = 0x0;
        TIM_TimeBaseInitdata.TIM_ClockDivision = TIM_CKD_DIV1;
        TIM_TimeBaseInitdata.TIM_CounterMode = TIM_CounterMode_Up;
        TIM_TimeBaseInitdata.TIM_Period = 0xfffful;
        TIM_TimeBaseInitdata.TIM_RepetitionCounter = 0x00;
        TIM_TimeBaseInit(htim, &TIM_TimeBaseInitdata);
        TIM_ARRPreloadConfig(htim, ENABLE);

        TIM_Cmd(htim, ENABLE);
    
        timer->clock.cs.rating  = 320;
        timer->clock.cs.freq    = timer->freq;
        timer->clock.cs.mask    = 0xfffful;
        timer->clock.cs.read    = hk32_timer_read;

        os_clocksource_register(dev->name, &timer->clock.cs);
    }
    else
#endif
    {
#ifdef OS_USING_CLOCKEVENT
        NVIC_InitTypeDef  NVIC_InitStructure;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannel = timer->info->irq;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);

        timer->clock.ce.rating  = hk32_timer_is_32b(htim) ? 320 : 160;
        timer->clock.ce.freq    = timer->freq;
        timer->clock.ce.mask    = 0xffffffffull;
        
        timer->clock.ce.prescaler_mask = 0xfffful;
        timer->clock.ce.prescaler_bits = 16;
        
        timer->clock.ce.count_mask = hk32_timer_is_32b(htim) ? 0xfffffffful : 0xfffful;
        timer->clock.ce.count_bits = hk32_timer_is_32b(htim) ? 32 : 16;

        timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

        timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
        
        timer->clock.ce.ops     = &hk32_tim_ops;
        os_clockevent_register(dev->name, &timer->clock.ce);
#endif
    }

    os_list_add(&hk32_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO hk32_tim_driver = {
    .name   = "TIM_HandleTypeDef",
    .probe  = hk32_tim_probe,
};

OS_DRIVER_DEFINE(hk32_tim_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);

