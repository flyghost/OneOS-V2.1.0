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
 * @brief       This file implements timer driver for cm32.
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

#define DBG_TAG "drv.hwtimer"
#include <dlog.h>

#define TIMER_MODE_TIM 0x00

static os_list_node_t cm32_timer_list = OS_LIST_INIT(cm32_timer_list);

struct cm32_timer timers[] = {
    {
        .mode = TIMER_MODE_TIM,
        .name = "tim5",
        .handle = TIM5,
        .irqn = TIM5_IRQn,
    },
    {
        .mode = TIMER_MODE_TIM,
        .name = "tim6",
        .handle = TIM6,
        .irqn = TIM6_IRQn,
    },
    {
        .mode = TIMER_MODE_TIM,
        .name = "tim7",
        .handle = TIM7,
        .irqn = TIM7_IRQn,
    },
};

void TIM5_IRQHandler(void)
{
    if (TIM_GetIntStatus(TIM5, TIM_INT_UPDATE) != RESET)
    {
#ifdef OS_USING_CLOCKEVENT
        if (timers[1].mode == TIMER_MODE_TIM)
        {
            os_clockevent_isr((os_clockevent_t *)&timers[1]);
        }
#endif
        TIM_ClrIntPendingBit(TIM5, TIM_INT_UPDATE);
    }
}

void TIM6_IRQHandler(void)
{
    if (TIM_GetIntStatus(TIM6, TIM_INT_UPDATE) != RESET)
    {
        TIM_ClrIntPendingBit(TIM6, TIM_INT_UPDATE);
    }
}

void TIM7_IRQHandler(void)
{
    if (TIM_GetIntStatus(TIM7, TIM_INT_UPDATE) != RESET)
    {
        TIM_ClrIntPendingBit(TIM7, TIM_INT_UPDATE);
    }
}

static os_uint64_t cm32_timer_read(void *clock)
{
/*
    struct cm32_timer *timer;

    timer = (struct cm32_timer *)clock;
    return Bt_M0_Cnt32Get(timer->unit);
*/

    return 10;
}

#ifdef OS_USING_CLOCKEVENT

/*
en_bt_cr_timclkdiv_t cm32_calc_div(os_uint32_t prescaler)
{

    if((1 << BtPCLKDiv1) >= prescaler)
    {
        return BtPCLKDiv1;
    }
    else if((1 << BtPCLKDiv2) >= prescaler)
    {
        return BtPCLKDiv2;
    }

    else if((1 << BtPCLKDiv4) >= prescaler)
    {
        return BtPCLKDiv4;
    }

    else if((1 << BtPCLKDiv8) >= prescaler)
    {
        return BtPCLKDiv8;
    }

    else if((1 << BtPCLKDiv16) >= prescaler)
    {
        return BtPCLKDiv16;
    }

    else if((1 << BtPCLKDiv32) >= prescaler)
    {
        return BtPCLKDiv32;
    }
    else if((1 << BtPCLKDiv64) >= prescaler)
    {
        return BtPCLKDiv64;
    }
    else
    {
        return BtPCLKDiv256;
    }
}
*/

static void cm32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct cm32_timer *timer;

    TIM_TimeBaseInitType TIM_TimeBaseStructure;
    uint16_t PrescalerValue = 0;

    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer = (struct cm32_timer *)ce;

    /* Compute the prescaler value */
    PrescalerValue = 0; //(uint16_t) (SystemCoreClock / 12000000) - 1;

    /* Time base configuration */
    TIM_TimeBaseStructure.Period    = count & timer->clock.ce.count_mask;//65535;
    TIM_TimeBaseStructure.Prescaler = 0;
    TIM_TimeBaseStructure.ClkDiv    = 0;
    TIM_TimeBaseStructure.CntMode   = TIM_CNT_MODE_UP;

    TIM_InitTimeBase(timer->handle, &TIM_TimeBaseStructure);

    /* Prescaler configuration */
    TIM_ConfigPrescaler(timer->handle, PrescalerValue, TIM_PSC_RELOAD_MODE_IMMEDIATE);

    /* TIMx enable update irq */
    TIM_ConfigInt(timer->handle, TIM_INT_UPDATE, ENABLE);

    /* TIMx enable counter */
    TIM_Enable(timer->handle, ENABLE);
}

static void cm32_timer_stop(os_clockevent_t *ce)
{
    struct cm32_timer *timer;

    OS_ASSERT(ce != OS_NULL);
    timer = (struct cm32_timer *)ce;

    TIM_ConfigInt(timer->handle, TIM_INT_UPDATE, DISABLE);
    TIM_Enable(timer->handle, DISABLE);
}

static const struct os_clockevent_ops cm32_tim_ops =
{
    .start = cm32_timer_start,
    .stop  = cm32_timer_stop,
    .read  = cm32_timer_read,
};
#endif

static void __os_hw_tim_init(struct cm32_timer *tim_drv)
{
    NVIC_InitType NVIC_InitStructure;

    /* PCLK1 = HCLK/4 */
    RCC_ConfigPclk1(RCC_HCLK_DIV1);

    /* TIMx clock enable */
    if (tim_drv->handle == TIM5)
    {
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM5, ENABLE);
    }
    else if (tim_drv->handle == TIM5)
    {
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM6, ENABLE);
    }
    else if (tim_drv->handle == TIM5)
    {
        RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM7, ENABLE);
    }
    else
    {
        LOG_E(DBG_TAG, "The timer is not supported!\n");        
    }

    /* Enable the TIMx global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = tim_drv->irqn;//TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;

    NVIC_Init(&NVIC_InitStructure);
}

/**
 ***********************************************************************************************************************
 * @brief           os_hw_tim_init:init timer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int os_hw_tim_init(void)
{
    os_uint32_t idx = 0;
    os_uint32_t freq;

    for(idx = 0; idx < (sizeof(timers) / sizeof(timers[0])); idx++)
    {
        __os_hw_tim_init(&(timers[idx]));

        freq = 40000000;//Sysctrl_GetPClkFreq();

        if (timers[idx].mode == TIMER_MODE_TIM)
        {
#ifdef OS_USING_CLOCKSOURCE
            if (os_clocksource_best() == OS_NULL)
            {
                timers[idx].freq = freq;//timers[idx].freq = freq / (1 << stcBtBaseCfg.enPRS);

                timers[idx].clock.cs.rating	= 320;
                timers[idx].clock.cs.freq	= timers[idx].freq;
                timers[idx].clock.cs.mask	= 0xffffffffull;
                timers[idx].clock.cs.read	= cm32_timer_read;

                //os_clocksource_register(timers[idx].name, &timers[idx].clock.cs);
            }
            else
#endif
            {
#ifdef OS_USING_CLOCKEVENT
                timers[idx].freq = freq;//timers[idx].freq = freq / (1 << stcBtBaseCfg.enPRS);

                timers[idx].clock.ce.rating	= 160;
                timers[idx].clock.ce.freq    = timers[idx].freq;
                timers[idx].clock.ce.mask	= 0xffffffffull;
                timers[idx].clock.ce.prescaler_mask = 0xfffful;
                timers[idx].clock.ce.prescaler_bits = 16;
                timers[idx].clock.ce.count_mask = 0xfffful;
                timers[idx].clock.ce.count_bits = 16;
                timers[idx].clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;
                timers[idx].clock.ce.min_nsec = NSEC_PER_SEC / timers[idx].clock.ce.freq;
                timers[idx].clock.ce.ops 	= &cm32_tim_ops;

                os_clockevent_register(timers[idx].name, &timers[idx].clock.ce);
#endif
            }
        }

        os_list_add(&cm32_timer_list, &timers[idx].list);
    }
    return OS_EOK;
}

OS_PREV_INIT(os_hw_tim_init, OS_INIT_SUBLEVEL_MIDDLE);
