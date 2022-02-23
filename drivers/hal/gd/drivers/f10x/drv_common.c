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
#include "drv_usart.h"
#include "drv_gpio.h"
#include "board.h"
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#include <timer/hrtimer.h>

void os_tick_handler(void)
{
    os_tick_increase();
    
#ifdef OS_USING_CLOCKSOURCE
    os_clocksource_update();
#endif
}

void systick_config(void)
{
    if (SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND)){
        /* capture error */
        while (1){
        }
    }
    
    /* configure the systick handler priority */
    NVIC_SetPriority(SysTick_IRQn, 0x00U);
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
    systick_config();

#ifdef HAL_SDRAM_MODULE_ENABLED
    SDRAM_Init();
#endif

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
    
    /* USART driver initialization is open by default */
#ifdef OS_USING_SERIAL
//    os_hw_usart_init();
#endif
#if defined(OS_USING_SYSTICK_FOR_CLOCKSOURCE) && defined(DWT)
    cortexm_dwt_init();
#endif
    return OS_EOK;
}

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_HIGH);


void os_hw_cpu_reset(void)
{
    NVIC_SystemReset();
}
