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
#include <stdint.h>
#include <os_task.h>
#include <device.h>
#include <gd32vf103.h>
#include <gd32vf103_eclic.h>
#include <riscv_encoding.h>
#include <os_memory.h>
#include <os_clock.h>
#include <drv_gpio.h>
#include <drv_usart.h>
#include <oneos_config.h>
#include <board.h>

/* Updates the variable SystemCoreClock and must be called
 * whenever the core clock is changed during program execution.*/
extern void riscv_clock_init(void);
/* Holds the system core clock, which is the system clock frequency
 * supplied to the SysTick timer and the processor core clock. */
static void ostick_config(os_uint32_t ticks)
{
    *(os_uint64_t *)(TIMER_CTRL_ADDR + TIMER_MTIMECMP) = ticks;
    eclic_irq_enable(CLIC_INT_TMR, 0, 0);
    *(os_uint64_t *)(TIMER_CTRL_ADDR + TIMER_MTIME) = 0;
}
/* void eclic_mtip_handler */

void eclic_mtip_handler(void)
{

    *(os_uint64_t *)(TIMER_CTRL_ADDR + TIMER_MTIME) = 0;
    os_tick_increase();
}

#if defined(OS_USING_USER_MAIN) && defined(OS_USING_HEAP)
#define OS_HEAP_SIZE 6144
static uint32_t os_heap[OS_HEAP_SIZE]; /* heap default size: 4K(1024 * 4) */
OS_WEAK void *  os_heap_begin_get(void)
{
    return os_heap;
}

OS_WEAK void *os_heap_end_get(void)
{
    return os_heap + OS_HEAP_SIZE;
}
#endif

/**
 ***********************************************************************************************************************
 * @brief       Initializes the CPU, System clocks, and Peripheral device
 *
 * @param[in]       none
 *
 * @return          none
 ***********************************************************************************************************************
 */

static os_err_t os_hw_board_init(void)
{
    
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern int __Vectors;
    interrupt_stack_addr = *(os_ubase_t *)&__Vectors;
#elif __ICCARM__
#error:not support iccarm
#else
    extern int _sp;
    interrupt_stack_addr = (os_ubase_t)&_sp;
#endif

    riscv_clock_init();
    ostick_config(TIMER_FREQ / OS_TICK_PER_SECOND);

#if defined(OS_USING_SYS_HEAP)
    os_sys_heap_init();
    os_sys_heap_add((void *)HEAP_BEGIN, (os_size_t)HEAP_END - (os_size_t)HEAP_BEGIN, OS_MEM_ALG_DEFAULT);
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

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_LOW);



