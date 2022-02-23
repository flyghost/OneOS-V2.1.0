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
 * @brief       This file implements adc driver for common.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <oneos_config.h>
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_clock.h>
#include <os_memory.h>

#include "am_mcu_apollo.h"
#include "drv_uart.h"
#include "drv_gpio.h"
#include "drv_led.h"
#include "drv_adc.h"
#include "drv_spi.h"
#include "drv_i2c.h"

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define AM_SRAM_BEGIN (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section = "HEAP"
#define AM_SRAM_BEGIN (__segment_end("HEAP"))
#else
extern int __bss_end;
#define AM_SRAM_BEGIN (&__bss_end)
#endif

#define TICK_RATE_HZ     OS_TICK_PER_SECOND
#define SYSTICK_CLOCK_HZ (32768UL)
#define WAKE_INTERVAL    ((uint32_t)((SYSTICK_CLOCK_HZ / TICK_RATE_HZ)))

/*timer clock sourece frequnce depend on ther actual configuration*/
#define SYSTICK_TIM_CLK_HZ 12000
#define TIMER_PERIOD (SYSTICK_TIM_CLK_HZ/OS_TICK_PER_SECOND)

os_uint32_t SystemCoreClock;

static void system_core_clock_init(void)
{
    os_uint32_t ui32ClkFreq;
    os_uint32_t ui32CoreSel = AM_BFR(CLKGEN, CCTRL, CORESEL);

    ui32ClkFreq = AM_HAL_CLKGEN_FREQ_MAX_MHZ >> ui32CoreSel;
    if (ui32CoreSel > 1)
    {
        ui32ClkFreq = AM_HAL_CLKGEN_FREQ_MAX_MHZ / (ui32CoreSel + 1);
    }

    SystemCoreClock = ui32ClkFreq;

}

am_hal_ctimer_config_t g_sTimer0 =
{
    /* Don't link timers. */
    0,

    /* Set up Timer0A. */
    (AM_HAL_CTIMER_FN_REPEAT | AM_HAL_CTIMER_INT_ENABLE | AM_HAL_CTIMER_HFRC_12KHZ),

    /* No configuration for Timer0B. */
    0,
};

void am_ctimer_isr(void)
{
    if(am_hal_ctimer_int_status_get(AM_HAL_CTIMER_INT_TIMERA0))
    {
        am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);

        if (os_task_self() != NULL)
        {
            os_tick_increase();
        }
    }
}

static void timerA0_init(void)
{
    uint32_t ui32Period;
	
#if defined(OS_USING_LPMGR)
	am_hal_ctimer_stop(1, AM_HAL_CTIMER_TIMERA);
  am_hal_ctimer_int_disable(AM_HAL_CTIMER_INT_TIMERA1);
	am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA1);
#endif

    /* Enable the LFRC. */
    //am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_LFRC);

    /* Set up timer A0 to count 3MHz clocks but don't start it yet */
    am_hal_ctimer_clear(0, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_config(0, &g_sTimer0);

    /* Set up timerA0 to 32Hz from LFRC divided to 1 second period. */
    ui32Period = TIMER_PERIOD;
    am_hal_ctimer_period_set(0, AM_HAL_CTIMER_TIMERA, ui32Period, (ui32Period >> 1));

    /* Clear the timer Interrupt */
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA0);
}

void SysTick_Configuration(void)
{
    /* TimerA0 init. */
    timerA0_init();

    /* Enable the timer interrupt in the NVIC. */
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
    am_hal_interrupt_master_enable();

    /* Enable the timer Interrupt. */
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);

    /* Start timer A0 */
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
}

void am_low_power_init(void)
{
    /* Enable internal buck converters. */
	#ifdef SOC_APOLLO2_XXX
		am_hal_pwrctrl_bucks_init();
	#else
    am_hal_mcuctrl_bucks_enable();
	#endif    

    /* Turn off the voltage comparator as this is enabled on reset. */
    am_hal_vcomp_disable();

    /* Run the RTC off the LFRC. */
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);

    /* Stop the XTAL. */
    am_hal_clkgen_osc_stop(AM_HAL_CLKGEN_OSC_XT);

    /* Disable the RTC. */
    am_hal_rtc_osc_disable();

    /* Disable the bandgap. */
	#ifdef SOC_APOLLO2_XXX
	#else
    am_hal_mcuctrl_bandgap_disable();
	#endif
}

void deep_power_save(void)
{
    am_hal_interrupt_master_disable();

    am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

    am_hal_interrupt_master_enable();
}

static os_err_t os_hw_board_init(void)
{
    /* Set the system clock to maximum frequency */
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    /* Set the default cache configuration */
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    /* Configure the board for low power operation */
    am_low_power_init();

    /* Enable the ITM. */
    am_hal_itm_enable();

    /* Config SysTick */
    SysTick_Configuration();

#ifdef RT_USING_IDLE_HOOK
    /* Set sleep deep mode */
    os_thread_idle_sethook(deep_power_save);

#ifndef NO_FPU
    /* Enable the floating point module, and configure the core for lazy stacking */
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);
#else
    am_hal_sysctrl_fpu_disable();
#endif

    /* Turn off unused Flash & SRAM */
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH512K);
    /* am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM32K); */

#endif

    system_core_clock_init();

#if defined(OS_USING_SYS_HEAP)
    if((os_size_t)AM_SRAM_END > (os_size_t)AM_SRAM_BEGIN)
    {
        os_sys_heap_init();
        os_sys_heap_add((void *)AM_SRAM_BEGIN, (os_size_t)AM_SRAM_END - (os_size_t)AM_SRAM_BEGIN, OS_MEM_ALG_DEFAULT);
    }
#endif


#ifdef OS_USING_RTT
    os_hw_rtt_init();
#endif

    return OS_EOK;
}

OS_CORE_INIT(os_hw_board_init,  OS_INIT_SUBLEVEL_MIDDLE);


static os_err_t board_post_init(void)
{
#ifdef OS_USING_PIN
    os_hw_pin_init();
    os_hw_led_init();
#endif

#ifdef OS_USING_SPI
    os_hw_spi_init();
#endif

#ifdef OS_USING_ADC
    os_hw_adc_init();
#endif

#ifdef BSP_USING_I2C
    os_hw_i2c_init();
#endif

    return OS_EOK;
}

OS_POSTCORE_INIT(board_post_init, OS_INIT_SUBLEVEL_HIGH);


