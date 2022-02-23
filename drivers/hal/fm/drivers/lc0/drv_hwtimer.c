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
 * @brief       This file implements timer driver for fm33.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <timer/timer.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#include <drv_hwtimer.h>

#ifdef OS_USING_PWM
#include "drv_pwm.h"
#endif

#ifdef OS_USING_PULSE_ENCODER
#include "drv_pulse_encoder.h"
#endif

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer "
#include <drv_log.h>

static os_list_node_t fm33_timer_list = OS_LIST_INIT(fm33_timer_list);

static void fm33_timer_update_callback(void *htim)
{
    struct fm33_timer *timer;

    os_list_for_each_entry(timer, &fm33_timer_list, struct fm33_timer, list)
    {
        if (timer->info->instance == htim && timer->clock.ce.parent.type == OS_DEVICE_TYPE_CLOCKEVENT)
        {
#ifdef OS_USING_CLOCKEVENT
            if (timer->info->mode == TIMER_MODE_TIM)
            {
                os_clockevent_isr((os_clockevent_t *)timer);
            }
#endif
        }
    }
}

/*timer irq handlers*/
#if defined (BSP_USING_ATIM)
void ATIM_IRQHandler()
{
    if(FL_ATIM_IsEnabledIT_Update(ATIM) && FL_ATIM_IsActiveFlag_Update(ATIM))
    {
        FL_ATIM_ClearFlag_Update(ATIM);
        fm33_timer_update_callback((void *)ATIM);
    }
}
#endif

#if defined (BSP_USING_BSTIM32)
void BSTIM_IRQHandler()
{
    if(FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
    {
        FL_BSTIM32_ClearFlag_Update(BSTIM32);
        fm33_timer_update_callback((void *)BSTIM32);
    }
}
#endif

#if defined (BSP_USING_GPTIM0)
void GPTIM0_IRQHandler()
{
    if(FL_GPTIM_IsEnabledIT_Update(GPTIM0) && FL_GPTIM_IsActiveFlag_Update(GPTIM0))
    {
        FL_GPTIM_ClearFlag_Update(GPTIM0);
        fm33_timer_update_callback((void *)GPTIM0);
    }
}
#endif

#if defined (BSP_USING_GPTIM1)
void GPTIM1_IRQHandler()
{
    if(FL_GPTIM_IsEnabledIT_Update(GPTIM1) && FL_GPTIM_IsActiveFlag_Update(GPTIM1))
    {
        FL_GPTIM_ClearFlag_Update(GPTIM1);
        fm33_timer_update_callback((void *)GPTIM1);
    }
}
#endif

static os_uint32_t fm33_timer_get_freq(struct fm33_timer *timer)
{
    os_uint32_t freq    = 0;
    os_uint32_t clk_div = 0;

    OS_ASSERT(timer != OS_NULL);

    switch (timer->info->type)
    {
#if defined (BSP_USING_ATIM)
        case TYPE_ATIM:
            clk_div = FL_ATIM_GetClockDivision(ATIM);
            freq    = FL_RCC_GetAPB2ClockFreq() >> (clk_div >> ATIM_CR1_CKD_Pos);
            break;
#endif

#if defined (BSP_USING_BSTIM32) || defined(BSP_USING_GPTIM0) || defined(BSP_USING_GPTIM1)
        case TYPE_BTIM:
        case TYPE_GPTIM:
            freq    = FL_RCC_GetAPB1ClockFreq();
            break;
#endif
        default:
            break;
    }

    return freq;
}

static void _timer_start(struct fm33_timer *timer, os_uint32_t prescaler, os_uint64_t count, os_bool_t irq_enable)
{
    OS_ASSERT(count != 0);
    OS_ASSERT(timer != NULL);

    switch(timer->info->type)
    {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
        {
            FL_ATIM_InitTypeDef InitStruct;

            InitStruct.clockSource          = FL_RCC_ATIM_CLK_SOURCE_APB2CLK;
            InitStruct.prescaler            = prescaler - 1;
            InitStruct.counterMode          = FL_ATIM_COUNTER_DIR_UP;
            InitStruct.autoReload           = count;
            InitStruct.autoReloadState      = FL_DISABLE;
            InitStruct.clockDivision        = FL_ATIM_CLK_DIVISION_DIV1;
            InitStruct.repetitionCounter    = 0;

            FL_ATIM_Init(timer->info->instance, &InitStruct);

            if (irq_enable)
            {
                FL_ATIM_EnableIT_Update(timer->info->instance);
                NVIC_EnableIRQ(timer->info->irqn);
            }

            FL_ATIM_Enable(timer->info->instance);
        }
        break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BTIM:
        {
            FL_BSTIM32_InitTypeDef defaultInitStruct; 

            defaultInitStruct.prescaler       = prescaler - 1;
            defaultInitStruct.autoReload      = count;
            defaultInitStruct.autoReloadState = FL_ENABLE;
            defaultInitStruct.clockSource     = FL_RCC_BSTIM32_CLK_SOURCE_APB1CLK;

            FL_BSTIM32_Init(timer->info->instance, &defaultInitStruct );

            if (irq_enable)
            {
                FL_BSTIM32_EnableIT_Update(timer->info->instance);
                NVIC_EnableIRQ(timer->info->irqn);
            }

            FL_BSTIM32_Enable(timer->info->instance);
        }
        break;
#endif

#if defined(BSP_USING_GPTIM0) || defined(BSP_USING_GPTIM1)
        case TYPE_GPTIM:
        {
            FL_GPTIM_InitTypeDef    TimerBaseInitStruct;

            TimerBaseInitStruct.prescaler       = prescaler - 1;
            TimerBaseInitStruct.counterMode     = FL_GPTIM_COUNTER_DIR_UP;
            TimerBaseInitStruct.autoReload      = count;
            TimerBaseInitStruct.autoReloadState = FL_DISABLE;
            TimerBaseInitStruct.clockDivision   = FL_GPTIM_CLK_DIVISION_DIV1;

            FL_GPTIM_Init(timer->info->instance, &TimerBaseInitStruct );

            if (irq_enable)
            {
                FL_GPTIM_EnableIT_Update(timer->info->instance);
                NVIC_EnableIRQ(timer->info->irqn);
            }

            FL_GPTIM_Enable(timer->info->instance);
        }
        break;
#endif
        default:
            break;
    }
}

static os_uint64_t fm33_timer_read(void *clock)
{
    struct fm33_timer *timer         = OS_NULL;
    os_uint64_t        timer_counter = 0;

    timer = (struct fm33_timer *)clock;
    OS_ASSERT(timer != OS_NULL);

    switch(timer->info->type)
    {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
            timer_counter = FL_ATIM_ReadCounter(timer->info->instance);
            break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BTIM:
            timer_counter = FL_BSTIM32_ReadCounter(timer->info->instance);
            break;
#endif

#if defined(BSP_USING_GPTIM0) || defined(BSP_USING_GPTIM1)
        case TYPE_GPTIM:
            timer_counter = FL_GPTIM_ReadCounter(timer->info->instance);
            break;
#endif
        default:
            break;
    }

    return timer_counter;
}

#ifdef OS_USING_CLOCKEVENT
static void fm33_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct fm33_timer *timer = OS_NULL;

    OS_ASSERT(count != 0);

    timer = (struct fm33_timer *)ce;
    OS_ASSERT(timer != NULL);

    OS_ASSERT((timer->info->type != TYPE_LPTIM) && (prescaler != 0));

    _timer_start(timer, prescaler, count, OS_TRUE);
}

static void fm33_timer_stop(os_clockevent_t *ce)
{
    struct fm33_timer *timer = OS_NULL;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct fm33_timer *)ce;
    OS_ASSERT(timer != OS_NULL);

    switch(timer->info->type)
    {
#if defined(BSP_USING_ATIM)
        case TYPE_ATIM:
            FL_ATIM_Disable(timer->info->instance);
            break;
#endif

#if defined(BSP_USING_BSTIM32)
        case TYPE_BTIM:
            FL_BSTIM32_Disable(timer->info->instance);
            break;
#endif

#if defined(BSP_USING_GPTIM0) || defined(BSP_USING_GPTIM1)
        case TYPE_GPTIM:
            FL_GPTIM_Disable(timer->info->instance);
            break;
#endif
        default:
            break;
    }
}

static const struct os_clockevent_ops fm33_tim_ops =
{
    .start = fm33_timer_start,
    .stop  = fm33_timer_stop,
    .read  = fm33_timer_read,
};
#endif


/**
 ***********************************************************************************************************************
 * @brief           fm33_tim_probe:probe timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int fm33_tim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct fm33_timer *timer = OS_NULL;

    timer = os_calloc(1, sizeof(struct fm33_timer));
    OS_ASSERT(timer);

    timer->info = (struct fm33_timer_info*)dev->info;

    timer->freq = fm33_timer_get_freq(timer);

#ifdef OS_USING_PWM
    if (timer->info->mode == TIMER_MODE_PWM)
    {
        fm33_pwm_register(dev->name,timer);
    }
#endif

#ifdef OS_USING_PULSE_ENCODER
    if (timer->info->mode == TIMER_MODE_PULSE_ENCODER)
    {
        fm33_pulse_encoder_register(dev->name,timer);
    }
#endif

    if (timer->info->mode == TIMER_MODE_TIM)
    {
#ifdef OS_USING_CLOCKSOURCE
        if (os_clocksource_best() == OS_NULL && timer->info->bits == 32)
        {
            _timer_start(timer, 1, 0xffffffffull, OS_FALSE);
            
            timer->clock.cs.rating  = 320;
            timer->clock.cs.freq    = timer->freq;
            timer->clock.cs.mask    = 0xffffffffull;
            timer->clock.cs.read    = fm33_timer_read;

            os_clocksource_register(dev->name, &timer->clock.cs);
        }
        else
#endif
        {
#ifdef OS_USING_CLOCKEVENT
            timer->clock.ce.rating  = (timer->info->bits == 32) ? 320 : 160;
            timer->clock.ce.freq    = timer->freq;
            timer->clock.ce.mask    = 0xffffffffull;
            
            timer->clock.ce.prescaler_mask = 0xfffful;
            timer->clock.ce.prescaler_bits = 16;
            
            timer->clock.ce.count_mask = (timer->info->bits == 32) ? 0xfffffffful : 0xfffful;
            timer->clock.ce.count_bits = timer->info->bits;

            timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

            timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
            
            timer->clock.ce.ops     = &fm33_tim_ops;

            os_clockevent_register(dev->name, &timer->clock.ce);
#endif
        }
    }

    os_list_add(&fm33_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO fm33_tim_driver = {
    .name   = "TIM_Type",
    .probe  = fm33_tim_probe,
};

OS_DRIVER_DEFINE(fm33_tim_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);

