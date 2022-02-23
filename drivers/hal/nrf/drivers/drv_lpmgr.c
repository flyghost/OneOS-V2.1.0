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
 * @file        drv_lpmgr.c
 *
 * @brief       This file implements low power manager for nordic.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>
#include "nrf_rtc.h"
#include "nrf_soc.h"

#include <os_memory.h>

volatile unsigned char rtc1_cc_event; //RTC wake up flag
volatile unsigned char gpio_int_event; //gpio interrupt wake up flag
volatile unsigned char hwtimer_int_event; //gpio interrupt wake up flag

void set_rtc1_int_event_flag(void)
{
    rtc1_cc_event = 1;
}

void set_gpio_int_event_flag(void)
{
    gpio_int_event = 1;
}

void set_hwtimer_int_event_flag(void)
{
    gpio_int_event = 1;
}

static void reset_wake_up_flag(void)
{
    //os_enter_critical();
    rtc1_cc_event = 0;
    gpio_int_event = 0;
    hwtimer_int_event = 0;
    //os_exit_critical();
}

static os_bool_t check_sleep_wake_up_flag(void)
{
    os_bool_t ret = OS_FALSE;

    if( rtc1_cc_event == 1 || gpio_int_event == 1 || hwtimer_int_event == 1 )
        ret = OS_TRUE;

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Put device into sleep mode.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       mode            Low power mode.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static int sleep(lpmgr_sleep_mode_e mode)
{
    switch (mode)
    {
    case SYS_SLEEP_MODE_NONE:
        break;

    case SYS_SLEEP_MODE_IDLE:
        __NOP();
        break;

    case SYS_SLEEP_MODE_LIGHT:
        reset_wake_up_flag();
        //clear the event;
        while ( check_sleep_wake_up_flag() == OS_FALSE ) {
            __SEV();
            __WFE();
            __WFE();
        }
        
        break;

    case SYS_SLEEP_MODE_DEEP:
        break;

    case SYS_SLEEP_MODE_STANDBY:
        /* Enter STANDBY mode */
        break;

    case SYS_SLEEP_MODE_SHUTDOWN:
        break;

    default:
        OS_ASSERT(0);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Start the timer of pm.
 *
 * @param[in]       lpm             Low power manager structure.
 * @param[in]       timeout         How many OS ticks that MCU can sleep.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void lpm_timer_start(struct os_lpmgr_dev *lpm, os_tick_t timeout)
{
    OS_ASSERT(lpm != OS_NULL);
    OS_ASSERT(timeout > 0);
    //stop the rtc
    nrf_rtc_task_trigger(NRF_RTC1, NRF_RTC_TASK_STOP);
    nrf_rtc_task_trigger(NRF_RTC1, NRF_RTC_TASK_CLEAR);

    //disable tick
    nrf_rtc_event_clear(NRF_RTC1, NRF_RTC_EVENT_TICK);
    nrf_rtc_event_disable(NRF_RTC1, NRF_RTC_INT_TICK_MASK);
    nrf_rtc_int_disable(NRF_RTC1, NRF_RTC_INT_TICK_MASK);

    //enable the COMPERE
    nrf_rtc_event_disable(NRF_RTC1, RTC_INTENSET_COMPARE0_Msk);
    nrf_rtc_int_disable(NRF_RTC1, RTC_INTENSET_COMPARE0_Msk);
    nrf_rtc_cc_set(NRF_RTC1, 0, timeout);
    nrf_rtc_event_clear(NRF_RTC1, NRF_RTC_EVENT_COMPARE_0);
    nrf_rtc_int_enable(NRF_RTC1, RTC_INTENSET_COMPARE0_Msk);
    nrf_rtc_event_enable(NRF_RTC1, RTC_EVTENSET_COMPARE0_Msk);


    nrf_rtc_task_trigger(NRF_RTC1, NRF_RTC_TASK_START);
}

/**
 ***********************************************************************************************************************
 * @brief           Stop the timer of pm.
 *
 * @param[in]       lpm             Low power manager structure.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void lpm_timer_stop(void)
{
    nrf_rtc_event_disable(NRF_RTC1, RTC_INTENSET_COMPARE0_Msk);
    nrf_rtc_int_disable(NRF_RTC1, RTC_INTENSET_COMPARE0_Msk);

    nrf_rtc_event_clear(NRF_RTC1, NRF_RTC_EVENT_TICK);
    nrf_rtc_event_enable(NRF_RTC1, NRF_RTC_INT_TICK_MASK);
    nrf_rtc_int_enable(NRF_RTC1, NRF_RTC_INT_TICK_MASK);
}
/**
 ***********************************************************************************************************************
 * @brief           Calculate how many OS ticks that MCU has suspended.
 *
 * @param[in]       lpm             Low power manager structure.
 *
 * @return          OS ticks.
 ***********************************************************************************************************************
 */
static os_tick_t lpm_timer_get_tick(void)
{
    unsigned int cnt = 0;

    /* Reset pmtimer status */
    cnt = nrf_rtc_counter_get(NRF_RTC1);

    return cnt;
}

static const struct os_lpmgr_ops lpmgr_ops = {
		sleep,
		lpm_timer_start,
		lpm_timer_stop,
		lpm_timer_get_tick
};

/**
***********************************************************************************************************************
* @brief           Initialise low power manager.
*
* @param[in]		None.
*
* @return          0.
***********************************************************************************************************************
*/
int drv_lpmgr_hw_init(void)
{


    os_uint8_t timer_mask = 0;
    struct os_lpmgr_dev  *lpmgr;

		lpmgr = (struct os_lpmgr_dev  *)os_calloc(1, sizeof(struct os_lpmgr_dev));
	
		lpmgr->ops = &lpmgr_ops;
    lpmgr->min_tick = 1;
    lpmgr->max_tick = 1000;
    /* Initialize timer mask */
    timer_mask = 1UL << SYS_SLEEP_MODE_LIGHT;

    /* Initialize system lpmgr module */
    os_lpmgr_register(lpmgr, timer_mask, OS_NULL);

    return 0;
}

OS_PREV_INIT(drv_lpmgr_hw_init, OS_INIT_SUBLEVEL_MIDDLE);
