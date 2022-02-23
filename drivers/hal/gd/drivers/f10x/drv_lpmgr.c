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
 * @brief       This file implements low power manager for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>
#include <timer/clockevent.h>

os_clockevent_t *lpce;

static void uart_console_reconfig(void)
{
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    os_device_control(os_console_get_device(), OS_DEVICE_CTRL_CONFIG, &config);
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
static void sleep(struct lpmgr *lpm, uint8_t mode)
{
    switch (mode)
    {
    case SYS_SLEEP_MODE_NONE:
        break;

    case SYS_SLEEP_MODE_IDLE:
        // __WFI();
        break;

    case SYS_SLEEP_MODE_LIGHT:
        if (lpm->run_mode == SYS_RUN_MODE_LOW_SPEED)
        {
            /* Enter LP SLEEP Mode, Enable low-power regulator */
            HAL_PWR_EnterSLEEPMode(PWR_LOWPOWERREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        }
        else
        {
            /* Enter SLEEP Mode, Main regulator is ON */
            HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
        }
        break;

    case SYS_SLEEP_MODE_DEEP:
        /* Enter STOP 2 mode  */
        HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
        /* Re-configure the system clock */
        SystemClock_ReConfig(lpm->run_mode);
        break;

    case SYS_SLEEP_MODE_STANDBY:
        /* Enter STANDBY mode */
        HAL_PWR_EnterSTANDBYMode();
        break;

    case SYS_SLEEP_MODE_SHUTDOWN:
        /* Enter SHUTDOWNN mode */
        HAL_PWREx_EnterSHUTDOWNMode();
        break;

    default:
        OS_ASSERT(0);
        break;
    }
}

static uint8_t run_speed[SYS_RUN_MODE_MAX][2] = {
    {80, 0},
    {80, 1},
    {24, 2},
    {2, 3},
};

static void run(struct lpmgr *lpm, uint8_t mode)
{
    static uint8_t last_mode;
    static char *run_str[] = SYS_RUN_MODE_NAMES;

    if (mode >= SYS_RUN_MODE_MAX)
    {
        os_kprintf("invalid mode: %d\n", mode);
        return;
    }

    if (mode == last_mode)
        return;
    last_mode = mode;

    /* Use MSI as SYSCLK source*/
    SystemClock_MSI_ON();

    /* Set frequency according to mode */
    switch (mode)
    {
    case SYS_RUN_MODE_HIGH_SPEED:
    case SYS_RUN_MODE_NORMAL_SPEED:
        HAL_PWREx_DisableLowPowerRunMode();
        SystemClock_80M();
        /* Configure the main internal regulator output voltage (Range1 by default)*/
        HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
        break;
    case SYS_RUN_MODE_MEDIUM_SPEED:
        HAL_PWREx_DisableLowPowerRunMode();
        SystemClock_24M();
        /* Configure the main internal regulator output voltage */
        HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);
        break;
    case SYS_RUN_MODE_LOW_SPEED:
        SystemClock_2M();
        /* Enter LP RUN mode */
        HAL_PWREx_EnableLowPowerRunMode();
        break;
    default:
        break;
    }

    /* Shut down clock */
    // SystemClock_MSI_OFF();

    /* Re-configure clock for peripheral */
    uart_console_reconfig();
    /* Re-Configure the Systick time */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / OS_TICK_PER_SECOND);
    /* Re-Configure the Systick */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    os_kprintf("switch to %s mode, frequency = %d MHz\n", run_str[mode], run_speed[mode][0]);
}

/**
 ***********************************************************************************************************************
 * @brief           Caculate the PM tick from OS tick.
 *
 * @param[in]       tick            OS tick.
 *
 * @return          PM tick.
 ***********************************************************************************************************************
 */
static os_tick_t stm32l4_lpm_tick_from_os_tick(os_tick_t tick)
{
    os_uint32_t freq = lpce->freq;

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
static os_tick_t stm32l4_os_tick_from_lpm_tick(os_uint32_t tick)
{
    static os_uint32_t os_tick_remain = 0;
    os_uint32_t        ret, freq;

    freq = lpce->freq;
    ret  = (tick * OS_TICK_PER_SECOND + os_tick_remain) / freq;

    os_tick_remain += (tick * OS_TICK_PER_SECOND);
    os_tick_remain %= freq;

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
static void lpm_timer_start(struct lpmgr *lpm, os_uint32_t timeout)
{
    OS_ASSERT(lpm != OS_NULL);
    OS_ASSERT(timeout > 0);

    if (timeout != OS_TICK_MAX)
    {
        /* Convert OS Tick to pmtimer timeout value */
        timeout = stm32l4_lpm_tick_from_os_tick(timeout);
        if (timeout > lpce->mask)
        {
            timeout = lpce->mask;
        }

        /* Enter LPM_TIMER_MODE */
        os_clockevent_start_oneshot(lpce, NSEC_PER_SEC * timeout / lpce->freq);
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
static void lpm_timer_stop(struct lpmgr *lpm)
{
    OS_ASSERT(lpm != OS_NULL);

    /* Reset pmtimer status */
    os_clockevent_stop(lpce);
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
static os_tick_t lpm_timer_get_tick(struct lpmgr *lpm)
{
    os_uint32_t timer_tick;

    OS_ASSERT(lpm != OS_NULL);

    timer_tick = os_clockevent_read(lpce);

    return stm32l4_os_tick_from_lpm_tick(timer_tick);
}

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
    static const struct os_lpmgr_ops s_lpmgr_ops = {
        sleep,
        run,
        lpm_timer_start,
        lpm_timer_stop,
        lpm_timer_get_tick
    };

    os_uint8_t timer_mask = 0;

    lpce = (os_clockevent_t *)os_device_find("lptim1");
    OS_ASSERT(lpce != OS_NULL);
    OS_ASSERT(os_device_open((os_device_t *)lpce, 0) == OS_EOK);

    /* Enable power clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* Initialize timer mask */
    timer_mask = 1UL << SYS_SLEEP_MODE_DEEP;

    /* Initialize system lpmgr module */
    os_lpmgr_init(&s_lpmgr_ops, timer_mask, OS_NULL);

    return 0;
}

OS_PREV_INIT(drv_lpmgr_hw_init);
