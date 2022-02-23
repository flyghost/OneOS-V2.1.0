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
 * @file        drv_common.c
 *
 * @brief       This file provides board init functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_clock.h>
#include <os_memory.h>
#include "drv_common.h"
#include "board.h"
#include "drv_gpio.h"

#ifdef OS_USING_SHELL
#include "shell.h"
#endif

#ifdef OS_USING_ADC
#include "drv_adc.h"
#endif

/* SysTick configuration */
void os_hw_systick_init(void)
{
    SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND);

    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    NVIC_SetPriority(SysTick_IRQn, 0);
}

/**
 * This is the timer interrupt service routine.
 *
 */
void SysTick_Handler(void)
{
    os_tick_increase();
}

void _Error_Handler(char *s, int num)
{
    volatile int loop = 1;
    while (loop);
}

/**
 * This function will delay for some us.
 *
 * @param us the delay time of us
 */
#ifndef OS_USING_CLOCKSOURCE
void os_hw_us_delay(os_uint32_t us)
{
    os_uint32_t start, now, delta, reload, us_tick;
    start = SysTick->VAL;
    reload = SysTick->LOAD;
    us_tick = SystemCoreClock / 1000000UL;
    do {
        now = SysTick->VAL;
        delta = start > now ? start - now : reload + start - now;
    } while(delta < us_tick * us);
}

#endif

/**
 ***********************************************************************************************************************
 * @brief           This function will initial hc32 board.
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
static os_err_t os_hw_board_init(void)
{
    /* System initialization */
    System_cfg();

    os_hw_systick_init();

    /* Heap initialization */
#if defined(OS_USING_SYS_HEAP)
    if((os_size_t)HEAP_END > (os_size_t)HEAP_BEGIN)
    {
        os_sys_heap_init();
        os_sys_heap_add((void *)HEAP_BEGIN, (os_size_t)HEAP_END - (os_size_t)HEAP_BEGIN, OS_MEM_ALG_DEFAULT);
    }
#endif

    return OS_EOK;
}

OS_CORE_INIT(os_hw_board_init, OS_INIT_SUBLEVEL_MIDDLE);

static os_err_t board_post_init(void)
{
#ifdef OS_USING_ADC
    /*For HC32 chip must init before os_hw_systick_init because it use systick for delay*/
    os_hw_adc_init();

    os_hw_systick_init();
#endif

#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

    return OS_EOK;
}

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_HIGH);

void os_hw_cpu_reset(void)
{
    NVIC_SystemReset();
}
