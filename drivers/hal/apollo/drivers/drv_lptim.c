/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        drv_lptim.c
 *
 * \@brief       This file implements low power timer driver for stm32.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_lptim.h>
#include "board.h"
#include <oneos_config.h>
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
//#include <os_irq.h>
#include <os_clock.h>
#include <os_memory.h>

#include "am_mcu_apollo.h"
#include "drv_uart.h"
#include "drv_gpio.h"
#if defined(SOC_APOLLO_XXX)
am_hal_ctimer_config_t g_sTimer1 =
{
    /* Don't link timers. */
    0,

    /* Set up Timer1A. */
    ((AM_REG_CTIMER_CTRL1_TMRA1FN(4)) | AM_REG_CTIMER_CTRL1_TMRA1IE_M | AM_HAL_CTIMER_XT_256HZ),

    /* No configuration for Timer1B. */
    0,
};
#endif

/**
 ***********************************************************************************************************************
 * @brief           Initialise LPTIM.
 *
 * @param[in]       period            period time.
 ***********************************************************************************************************************
 */
static void timerA1_init(uint32_t period)
{
    uint32_t ui32Period;

//    /* Enable the LFRC. */

    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);
    
    /* Set up timer A1 to count 256Hz clocks but don't start it yet */
    am_hal_ctimer_clear(1, AM_HAL_CTIMER_TIMERA);
    #if defined(SOC_APOLLO_XXX)
    am_hal_ctimer_config(1, &g_sTimer1);
    #endif
    #if defined(SOC_APOLLO2_XXX)
    am_hal_ctimer_config_single(1, AM_HAL_CTIMER_TIMERA,
                                   AM_HAL_CTIMER_XT_256HZ |
                                   AM_HAL_CTIMER_FN_CONTINUOUS |
                                   AM_HAL_CTIMER_INT_ENABLE);
	
    #endif
    /* Set up timerA1 to period from LFRC divided to period/12K period. */
    ui32Period = period;
    //os_kprintf("the set time is:%d\r\n",ui32Period);
    am_hal_ctimer_period_set(1, AM_HAL_CTIMER_TIMERA, ui32Period, (ui32Period >> 1));

    /* Clear the timer Interrupt */
    am_hal_ctimer_int_clear(AM_HAL_CTIMER_INT_TIMERA1);
}



/**
 ***********************************************************************************************************************
 * @brief           Start LPTIM with reload value.
 *
 * @param[in]       reload            Reload value.
 *
 * @return          OS_EOK.
 ***********************************************************************************************************************
 */
os_err_t lptim_start(os_uint32_t reload)
{
    if( reload > 0 )
    {
        /* Stop timer A0 and Disable timerA0 Interrupt*/
        am_hal_ctimer_stop(0, AM_HAL_CTIMER_TIMERA);
        am_hal_ctimer_int_disable(AM_HAL_CTIMER_INT_TIMERA0);
       
        /* Enable the timer interrupt in the NVIC. */
        //am_hal_interrupt_enable(AM_HAL_INTERRUPT_CTIMER);
        //am_hal_interrupt_master_enable();
        /* TimerA1 init. */
        timerA1_init(reload);
        
        /* Enable the timerA1 Interrupt. */
        am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA1);
        /* Start timer A1 */
        am_hal_ctimer_start(1, AM_HAL_CTIMER_TIMERA);
        return OS_EOK;
    }
    else
        return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the max count value of LPTIM.
 *
 * @param[in,out]   None.
 *
 * @return          The max count value of LPTIM.
 ***********************************************************************************************************************
 */
os_uint32_t lptim_get_tick_max(void)
{
    return (0xFFFF);
}



/**
***********************************************************************************************************************
* @brief           Stop LPTIM.
*
* @param[in]       None.
*
* @return          None.
***********************************************************************************************************************
*/
void lptim_stop(void)
{
    /* Stop timer A1 and Disable timerA1 Interrupt*/
    am_hal_ctimer_stop(1, AM_HAL_CTIMER_TIMERA);
    am_hal_ctimer_int_disable(AM_HAL_CTIMER_INT_TIMERA1);
   // am_hal_clkgen_osc_stop(AM_HAL_CLKGEN_OSC_XT);
    /* Start timer A0 and enable timerA0 Interrupt*/
    am_hal_ctimer_int_enable(AM_HAL_CTIMER_INT_TIMERA0);
    am_hal_ctimer_start(0, AM_HAL_CTIMER_TIMERA);
}
/**
 ***********************************************************************************************************************
 * @brief           Get current count value of LPTIM.
 *
 * @param[in,out]   None.
 *
 * @return          Current count value of LPTIM.
 ***********************************************************************************************************************
 */
os_uint32_t lptim_get_current_tick(void)
{
    return am_hal_ctimer_read(1, AM_HAL_CTIMER_TIMERA);
}



/**
 ***********************************************************************************************************************
 * @brief           Get the count clock of LPTIM.
 *
 * @param[in]       None.
 *
 * @return          The count clock of LPTIM.
 ***********************************************************************************************************************
 */
os_uint32_t lptim_get_countfreq(void)
{
    return 256;
}

//OS_DEVICE_INIT(hw_lptim_init);
#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(lptim_start, lptim_start, "dump power management status");
SH_CMD_EXPORT(read_tick, lptim_get_current_tick, "dump power management status");
#endif
