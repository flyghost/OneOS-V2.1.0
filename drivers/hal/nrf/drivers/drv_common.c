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

//#include <dma/dma.h>
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_memory.h>
#include "drv_common.h"
#include "drv_gpio.h"
#include "drv_pwm.h"
#include "board.h"
#include "drv_uart.h"
#include "nrf_systick.h"
#include "nrf_rtc.h"
#include "nrf_drv_clock.h"

#ifdef OS_USING_CLOCKSOURCE
#include "clocksource_cortexm.h"
#endif

#ifdef OS_USING_CLOCKEVENT
#include "clocksource.h"
#endif

static volatile os_bool_t hardware_init_done = OS_FALSE;

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

#define OSTICK_CLOCK_HZ  ( 32768UL )
#define COUNTER_MAX 0x00ffffff
#define CYC_PER_TICK (OSTICK_CLOCK_HZ / OS_TICK_PER_SECOND)
#define MAX_TICKS ((COUNTER_MAX - CYC_PER_TICK) / CYC_PER_TICK)
#define MIN_DELAY 32

#define TICK_RATE_HZ  OS_TICK_PER_SECOND

#define ROUNDED_DIV(A, B)      (((A) + ((B) / 2)) / (B))
#define portNRF_RTC_PRESCALER  ( (os_uint32_t) (ROUNDED_DIV(OSTICK_CLOCK_HZ, OS_TICK_PER_SECOND) - 1) )

#define NRF_RTC_REG        NRF_RTC1
/* IRQn used by the selected RTC */
#define NRF_RTC_IRQn       RTC1_IRQn

extern void set_rtc1_int_event_flag(void);
void RTC1_IRQHandler(void)
{
    if (nrf_rtc_int_is_enabled(NRF_RTC_REG, NRF_RTC_INT_TICK_MASK) &&
        nrf_rtc_event_pending(NRF_RTC_REG, NRF_RTC_EVENT_TICK)) 
    {
        nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_TICK);
        os_tick_increase();
    }
    if (nrf_rtc_int_is_enabled(NRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK) &&
        nrf_rtc_event_pending(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0)) 
    {
        nrf_rtc_event_disable(NRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);
        nrf_rtc_int_disable(NRF_RTC_REG, NRF_RTC_INT_COMPARE0_MASK);
        nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_COMPARE_0);

        #ifdef OS_USING_LPMGR
        set_rtc1_int_event_flag();
        #endif
    }
}

void os_rtc_tick_init(void)
{
    nrf_clock_lf_src_set((nrf_clock_lfclk_t)NRFX_CLOCK_CONFIG_LF_SRC);
    nrfx_clock_lfclk_start();

    nrf_rtc_prescaler_set(NRF_RTC_REG, portNRF_RTC_PRESCALER);

    NVIC_SetPriority(NRF_RTC_IRQn, 6);
    NVIC_EnableIRQ(NRF_RTC_IRQn);

    /* Clear the event flag and possible pending interrupt */
    nrf_rtc_event_clear(NRF_RTC_REG, NRF_RTC_EVENT_TICK);
    nrf_rtc_event_enable(NRF_RTC_REG, NRF_RTC_INT_TICK_MASK);
    nrf_rtc_int_enable(NRF_RTC_REG, NRF_RTC_INT_TICK_MASK);

    NVIC_ClearPendingIRQ(NRF_RTC_IRQn);


    nrf_rtc_task_trigger(NRF_RTC_REG, NRF_RTC_TASK_CLEAR);
    nrf_rtc_task_trigger(NRF_RTC_REG, NRF_RTC_TASK_START);
}

static os_err_t os_hw_board_init(void)
{
    os_irq_enable();
    nrf_drv_clock_init();

#if defined(OS_USING_PWM) && defined(BSP_USING_PWM)
    nrf52_pwm_init();
#endif

    /* Heap initialization */
#if defined(OS_USING_SYS_HEAP)
    if((os_size_t)HEAP_END > (os_size_t)HEAP_BEGIN)
    {
        os_sys_heap_init();
        os_sys_heap_add((void *)HEAP_BEGIN, (os_size_t)HEAP_END - (os_size_t)HEAP_BEGIN, OS_MEM_ALG_DEFAULT);
    }
#endif

#ifdef OS_USING_DMA
    os_dma_mem_init();
#endif

    return OS_EOK;
}
OS_CORE_INIT(os_hw_board_init, OS_INIT_SUBLEVEL_MIDDLE);

static os_err_t board_post_init(void)
{
#ifdef OS_USING_PIN
    os_hw_pin_init();
#endif

#if defined(OS_USING_DWT_FOR_CLOCKSOURCE) && defined(DWT)
    cortexm_dwt_init();
#endif

    //calc_mult_shift(&mult_systick2msec, &shift_systick2msec, OS_TICK_PER_SECOND, 1000, 1);

    os_rtc_tick_init();

    hardware_init_done = OS_TRUE;

    return OS_EOK;
}
OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_LOW);

void os_hw_cpu_reset(void)
{
    NVIC_SystemReset();
}
