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

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#endif

#include <timer/hrtimer.h>

void systick_init(void)
{
    /* systick for kernel tick */ 
    SysTick->LOAD  = SystemCoreClock / OS_TICK_PER_SECOND;      /* set reload register */
    SysTick->VAL   = 0UL;                                       /* Load the systick Counter Value */
    SysTick->CTRL |= 3;
}

void dma_Init(void)
{
#ifdef OS_USING_DMA
    FL_RCC_EnableGroup2BusClock(FL_RCC_GROUP2_BUSCLK_DMA);

    NVIC_DisableIRQ(DMA_IRQn);
    NVIC_SetPriority(DMA_IRQn, 2);
    NVIC_EnableIRQ(DMA_IRQn);

    FL_DMA_Enable(DMA);
#endif
}

void os_tick_handler(void)
{
    os_tick_increase();

#ifdef OS_USING_CLOCKSOURCE
    os_clocksource_update();
#endif
}

void SysTick_Handler(void)
{
    os_tick_handler();  
}

void _Error_Handler(char *s, int num)
{
    volatile int loop = 1;
    while (loop);
}

void SystemClock_Config(void)
{
    extern void SelRCHFToPLL(uint32_t rchf, uint32_t clock);
    extern void SystemCoreClockUpdate(void);
    SelRCHFToPLL(FL_RCC_RCHF_FREQUENCY_8MHZ,48 - 1);
    SystemCoreClockUpdate();
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initial FM33 board.
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */
static os_err_t os_hw_board_init()
{
    SystemClock_Config();

    dma_Init();

    systick_init();

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
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

    return OS_EOK;
}

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_HIGH);
