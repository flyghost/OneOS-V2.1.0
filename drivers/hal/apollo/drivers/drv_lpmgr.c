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
 * \@file        drv_lpmgr.c
 *
 * \@brief       This file implements low power manager for stm32.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>
#include <drv_lptim.h>
#include <am_mcu_apollo.h>
#include <os_memory.h>
#include <string.h>

static void uart_console_reconfig(void)
{
#ifdef OS_USING_SHELL
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    os_device_control(os_console_get_device(), OS_DEVICE_CTRL_CONFIG, &config);
#endif
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
static int lpm_sleep(uint8_t mode)
{
    switch (mode)
    {
    case SYS_SLEEP_MODE_NONE:
        break;

    case SYS_SLEEP_MODE_IDLE:
        //os_kprintf("SYS_SLEEP_MODE_IDLE\n");
        // __WFI();
        break;

    case SYS_SLEEP_MODE_LIGHT:
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
        break;

    case SYS_SLEEP_MODE_DEEP:
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
        uart_console_reconfig();
        break;

    case SYS_SLEEP_MODE_STANDBY:
        break;

    case SYS_SLEEP_MODE_SHUTDOWN:
        break;

    default:
        os_kprintf("invalid mode: %d\n", mode);
        break;
    }

    return OS_EOK;
}

extern void SysTick_Configuration(void);


/**
 ***********************************************************************************************************************
 * @brief           Caculate the PM tick from OS tick.
 *
 * @param[in]       tick            OS tick.
 *
 * @return          PM tick.
 ***********************************************************************************************************************
 */
static os_tick_t lpm_tick_from_os_tick(os_tick_t tick)
{
    os_uint32_t freq = lptim_get_countfreq();

    return (freq * tick / OS_TICK_PER_SECOND);
}

/**
 ***********************************************************************************************************************
 * @brief           Caculate the OS tick from PM tick.
 *
 * @param[in]       tick            PM tick.
 *
 * @return          OS tick.
 ***********************************************************************************************************************
 */
static os_tick_t os_tick_from_lpm_tick(os_uint32_t tick)
{
    static os_uint32_t os_tick_remain = 0;
    os_uint32_t        ret, freq;

    freq = lptim_get_countfreq();
    ret  = (tick * OS_TICK_PER_SECOND + os_tick_remain) / freq;

//    os_tick_remain += (tick * OS_TICK_PER_SECOND);
//    os_tick_remain %= freq;

    return ret;
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
    //if (timeout != OS_TICK_MAX)
    {
        /* Convert OS Tick to pmtimer timeout value */
        timeout = lpm_tick_from_os_tick(timeout);
        if (timeout > lptim_get_tick_max())
        {
            timeout = lptim_get_tick_max();
        }
        /* Enter LPM_TIMER_MODE */
        lptim_start(timeout);
    }
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
    /* Reset pmtimer status */
    lptim_stop();
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
    os_uint32_t timer_tick;

    timer_tick = lptim_get_current_tick();

    return os_tick_from_lpm_tick(timer_tick);
 
}


static const struct os_lpmgr_ops lpmgr_ops = {
    .sleep = lpm_sleep,
    .sleeptick_start = lpm_timer_start,
    .sleeptick_stop = lpm_timer_stop,
    .sleeptick_gettick = lpm_timer_get_tick,
};


/**
***********************************************************************************************************************
* @brief           Initialise low power manager.
*
* @param[in]       None.
*
* @return          0.
***********************************************************************************************************************
*/
int drv_lpmgr_hw_init(void)
{
    os_uint8_t sleep_mask = 0;
    struct os_lpmgr_dev  *lpmgr;

    lpmgr = (struct os_lpmgr_dev  *)os_calloc(1, sizeof(struct os_lpmgr_dev));
    OS_ASSERT(lpmgr != NULL);
    memset(lpmgr, 0, sizeof(struct os_lpmgr_dev));
    
    
    lpmgr->ops = &lpmgr_ops;

    lpmgr->min_tick = 2;
    lpmgr->max_tick = 100*2000;  /*  */

    /* Initialize timer mask */
    sleep_mask = (1UL << SYS_SLEEP_MODE_DEEP) | (1UL << SYS_SLEEP_MODE_LIGHT);

    /* Initialize system lpmgr module */
    os_lpmgr_register(lpmgr, sleep_mask, OS_NULL);

    return 0;
}

OS_DEVICE_INIT(drv_lpmgr_hw_init, OS_INIT_SUBLEVEL_HIGH);

