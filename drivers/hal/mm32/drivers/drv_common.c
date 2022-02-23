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
 * @brief       This file provides systick time init/IRQ and board init functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mm32_it.h"
#include <string.h>
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <arch_interrupt.h>
#include <os_clock.h>
#include <os_memory.h>
#include "drv_common.h"
#include "board.h"

#ifdef OS_USING_DMA
#include <dma.h>
#endif

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#endif

#include <timer/hrtimer.h>

__IO uint32_t uwTick;

uint32_t HAL_GetTick(void)
{
    return uwTick;
}

void HAL_SuspendTick(void)
{
    /* Disable SysTick Interrupt */
    SysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
}

void _Error_Handler(char *s, int num)
{
    volatile int loop = 1;
    while (loop);
}

static void cortexm_systick_kernel_tick_init(void)
{
    SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND);
    SysTick->CTRL |= 0x00000004UL;
}

void hardware_init(void)
{
    HAL_NVIC_SetPriorityGrouping();
}

void os_hw_cpu_reset(void)
{
    NVIC_SystemReset();
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initial STM32 board.
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
static os_err_t os_hw_board_init(void)
{
#ifdef SCB_EnableICache
    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();
#endif

#ifdef SCB_EnableDCache
    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();
#endif

    hardware_init();
    os_irq_enable();
    HAL_SuspendTick();

#ifdef HAL_SDRAM_MODULE_ENABLED
    SDRAM_Init();
#endif

    /* Heap initialization */
#if defined(OS_USING_SYS_HEAP)
    if((os_size_t)HEAP_END > (os_size_t)HEAP_BEGIN)
    {
        os_sys_heap_init();
        os_sys_heap_add((void *)HEAP_BEGIN, (os_size_t)HEAP_END - (os_size_t)HEAP_BEGIN, OS_MEM_ALG_DEFAULT);
    }
#endif

#ifdef OS_USING_DMA_RAM
    os_dma_mem_init();
#endif

    return OS_EOK;
}
OS_CORE_INIT(os_hw_board_init, OS_INIT_SUBLEVEL_MIDDLE);

void cortexm_systick_init(void)
{
#ifdef OS_USING_SYSTICK_FOR_KERNEL_TICK
    cortexm_systick_kernel_tick_init();
#elif defined(OS_USING_SYSTICK_FOR_CLOCKSOURCE)
    cortexm_systick_clocksource_init();
#elif defined(OS_USING_SYSTICK_FOR_CLOCKEVENT)
    cortexm_systick_clockevent_init();
#endif
}

static os_err_t board_post_init(void)
{
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

#if defined(OS_USING_DWT_FOR_CLOCKSOURCE) && defined(DWT)
    cortexm_dwt_init();
#endif

    //calc_mult_shift(&mult_systick2msec, &shift_systick2msec, OS_TICK_PER_SECOND, 1000, 1);

    cortexm_systick_init();

    return OS_EOK;
}

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_LOW);
