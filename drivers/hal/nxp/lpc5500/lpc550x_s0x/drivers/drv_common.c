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
#include <drv_cfg.h>
#include <timer/hrtimer.h>
#include <drv_flash.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#include <timer/clocksource_cortexm.h>
#endif

static volatile os_bool_t hardware_init_done = OS_FALSE;

static os_uint32_t mult_systick2msec = 1;
static os_uint32_t shift_systick2msec = 0;

void os_tick_handler(void)
{   
    os_tick_increase();
    
#ifdef OS_USING_CLOCKSOURCE
    os_clocksource_update();
#endif
}

#ifdef OS_USING_SYSTICK_FOR_KERNEL_TICK
void SysTick_Handler(void)
{ 
    os_tick_handler();
}

static void cortexm_systick_kernel_tick_init(void)
{
    /* init systick  1 systick = 1/(100M / 100) 100¸ösystick = 1s*/
    SysTick_Config(SystemCoreClock / OS_TICK_PER_SECOND);
    /* set pend exception priority */
    NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS) - 1);

}
#endif
#if defined(OS_USING_SYSTICK_FOR_CLOCKEVENT)
void SysTick_Handler(void)
{
    if (hardware_init_done)
    {
        cortexm_systick_clockevent_isr();
    }
    else
    {
        os_tick_handler();
    }
}
#endif

int hardware_init(void)
{
    BOARD_InitBootClocks();
    BOARD_InitBootPins();
    BOARD_InitBootPeripherals();
    CLOCK_EnableClock(kCLOCK_InputMux);
    
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    
    GPIO_PortInit(GPIO, 0);
    GPIO_PortInit(GPIO, 1);
    return 0;
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
static os_err_t os_hw_board_init()
{
    hardware_init_done = OS_FALSE;
    hardware_init();

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

    calc_mult_shift(&mult_systick2msec, &shift_systick2msec, OS_TICK_PER_SECOND, 1000, 1);

    cortexm_systick_init();
    
    hardware_init_done = OS_TRUE;
    
    return OS_EOK;
}

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_HIGH);


