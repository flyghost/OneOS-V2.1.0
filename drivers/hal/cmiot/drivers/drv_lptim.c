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
 * @file        drv_lptim.c
 *
 * @brief       This file implements low power timer driver for cm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_task.h>
#include <os_memory.h>
#include <timer/timer.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

#include <drv_log.h>

#include <cm32m101a_lptim.h>

struct cm32_lptimer {

    os_clockevent_t ce;

    os_uint32_t freq;

    const char *name;

    LPTIM_Module *base;
};

struct cm32_lptimer lptimers[] = {
    {
        .name = "lptim",
        .base = LPTIM,
    },
};

void LPTIM_WKUP_IRQHandler(void)
{
    if (LPTIM_IsActiveFlag_CMPM(LPTIM) != RESET)
    {
        LPTIM_ClearFLAG_CMPM(LPTIM);
        EXTI_ClrITPendBit(EXTI_LINE24);
        os_clockevent_isr((os_clockevent_t *)(&lptimers[0].ce));
    }
}

void LPTIMNVIC_Config(FunctionalState Cmd)
{
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    EXTI_ClrITPendBit(EXTI_LINE24);
    EXTI_InitStructure.EXTI_Line = EXTI_LINE24;
#ifdef __TEST_SEVONPEND_WFE_NVIC_DIS__
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
#else
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
#endif
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /* Enable the RTC Alarm Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = LPTIM_WKUP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = Cmd;
    NVIC_Init(&NVIC_InitStructure);
}

static void cm32_lptimer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct cm32_lptimer *timer;

    timer = (struct cm32_lptimer *)ce;

    LPTIM_EnableIT_CMPM(LPTIM);
    LPTIM_Enable(LPTIM);
    LPTIM_SetAutoReload(LPTIM, count & timer->ce.count_mask);
    LPTIM_SetCompare(LPTIM, 0x0000);
    LPTIM_StartCounter(LPTIM, LPTIM_OPERATING_MODE_ONESHOT);
}

static void cm32_lptimer_stop(os_clockevent_t *ce)
{
    OS_ASSERT(ce != OS_NULL);

    LPTIM_DisableIT_CMPM(LPTIM);
    LPTIM_Disable(LPTIM);
}

os_uint64_t cm32_lptimer_read(void *clock)
{
    os_uint32_t cnt = 0, arr = 0;

    cnt = LPTIM_GetCounter(LPTIM);
    arr = LPTIM_GetAutoReload(LPTIM);

    if(cnt > 0)
    {
        return (os_uint64_t)(cnt);
    }
    else
    {
        return (os_uint64_t)(arr);
    }
}

static const struct os_clockevent_ops cm32_lptim_ops =
{
    .start = cm32_lptimer_start,
    .stop  = cm32_lptimer_stop,
    .read  = cm32_lptimer_read,
};

static void __os_hw_lptim_init(void)
{
    RCC_EnableLsi(ENABLE);
    LPTIMNVIC_Config(ENABLE);
    RCC_ConfigLPTIMClk(RCC_LPTIMCLK_SRC_LSI);
    RCC_EnableRETPeriphClk(RCC_RET_PERIPH_LPTIM, ENABLE);

    LPTIM_SetPrescaler(LPTIM, LPTIM_PRESCALER_DIV32);
}

/**
 ***********************************************************************************************************************
 * @brief           os_hw_lptim_init:init lptimer device.
 *
 * @param[in]       none
 *
 * @return          Return timer probe status.
 * @retval          OS_EOK         timer register success.
 * @retval          OS_ERROR       timer register failed.
 ***********************************************************************************************************************
 */
static int os_hw_lptim_init(void)
{
    os_uint32_t idx = 0;

    __os_hw_lptim_init();

    for(idx = 0; idx < (sizeof(lptimers) / sizeof(lptimers[0])); idx++)
    {
        lptimers[idx].freq = 40000 / 32;

        lptimers[idx].ce.rating  = 50;
        lptimers[idx].ce.freq    = lptimers[idx].freq;
        lptimers[idx].ce.mask    = 0xfffful;

        lptimers[idx].ce.prescaler_mask = 0;
        lptimers[idx].ce.prescaler_bits = 0;

        lptimers[idx].ce.count_mask = 0xfffful;
        lptimers[idx].ce.count_bits = 16;

        lptimers[idx].ce.feature  = OS_CLOCKEVENT_FEATURE_ONESHOT;

        lptimers[idx].ce.min_nsec = NSEC_PER_SEC / lptimers[idx].ce.freq;

        lptimers[idx].ce.ops     = &cm32_lptim_ops;

        os_clockevent_register(lptimers[idx].name, &lptimers[idx].ce);
    }

    return OS_EOK;
}

OS_POSTCORE_INIT(os_hw_lptim_init, OS_INIT_SUBLEVEL_HIGH);
