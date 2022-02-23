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
 * @brief       This file implements low power manager for cm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <lpmgr.h>
#include <board.h>
#include <timer/clockevent.h>
#include <cm32m101a_pwr.h>
#include <drv_common.h>
#include <os_memory.h>
#include <string.h>

#define OS_NS_TO_TICK(ns) (OS_TICK_PER_SECOND * ns / (1000 * 1000 * 1000))
#define LPMGR_MIN_SLEEPTICK     (2)

os_clockevent_t *lp_ce = NULL;

/*for cm32 chip must enable interrupt while in sleep mode*/
static os_uint32_t os_irq_level = ~0u;

enum
{
    SYSCLK_PLLSRC_HSI,
    SYSCLK_PLLSRC_HSIDIV2,
    SYSCLK_PLLSRC_HSI_PLLDIV2,
    SYSCLK_PLLSRC_HSIDIV2_PLLDIV2,
    SYSCLK_PLLSRC_HSE,
    SYSCLK_PLLSRC_HSEDIV2,
    SYSCLK_PLLSRC_HSE_PLLDIV2,
    SYSCLK_PLLSRC_HSEDIV2_PLLDIV2,
};

static void cortexm_systick_tick_deinit(void)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

static void cortexm_systick_tick_init(void)
{
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    NVIC_SetPriority(SysTick_IRQn, 0);
}

os_uint32_t lpmgr_enter_critical(os_uint8_t sleep_mode)
{
    os_uint32_t level;
    level = os_irq_lock();
    os_irq_level = level;
    return level;
}

void lpmgr_exit_critical(os_uint32_t ctx, os_uint8_t sleep_mode)
{
    if(ctx == ~0u)
        os_irq_unlock(os_irq_level);
    else
        os_irq_unlock(ctx);
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
        break;

    case SYS_SLEEP_MODE_LIGHT:
        cortexm_systick_tick_deinit();
        lpmgr_exit_critical(~0u, mode);

        PWR_EnterSLEEPMode(SLEEP_OFF_EXIT, PWR_SLEEPENTRY_WFI);

        lpmgr_enter_critical(mode);
        cortexm_systick_tick_init();
        break;

    case SYS_SLEEP_MODE_DEEP:
        cortexm_systick_tick_deinit();
        lpmgr_exit_critical(~0u, mode);

        PWR_EnterSTOP2Mode(PWR_STOPENTRY_WFI, PWR_CTRL3_RAM1RET);

        lpmgr_enter_critical(mode);
        cortexm_systick_tick_init();
        break;

    case SYS_SLEEP_MODE_STANDBY:
        break;

    case SYS_SLEEP_MODE_SHUTDOWN:
        break;

    default:
        break;
    }
    return OS_EOK;
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
static os_tick_t lpm_tick_from_os_tick(os_tick_t tick)
{
    os_uint32_t freq = lp_ce->freq;

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

    freq = lp_ce->freq;
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
static void lpm_timer_start(struct os_lpmgr_dev *lpm, os_tick_t timeout)
{
    OS_ASSERT(lpm != OS_NULL);
    OS_ASSERT(timeout > 0);

    if (timeout != OS_TICK_MAX)
    {
        /* Convert OS Tick to pmtimer timeout value */
        timeout = lpm_tick_from_os_tick(timeout);
        if (timeout > lp_ce->mask)
        {
            timeout = lp_ce->mask;
        }

        /* Enter LPM_TIMER_MODE */
        os_clockevent_start_oneshot(lp_ce, NSEC_PER_SEC * timeout / lp_ce->freq);
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
    os_clockevent_stop(lp_ce);
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

    timer_tick = os_clockevent_read(lp_ce);

    return os_tick_from_lpm_tick(timer_tick);
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
    os_uint32_t sleep_mask = 0;
    struct os_lpmgr_dev  *lpmgr;
    os_tick_t lp_min, lp_max;

    lpmgr = (struct os_lpmgr_dev  *)os_calloc(1, sizeof(struct os_lpmgr_dev));
    OS_ASSERT(lpmgr != NULL);
    memset(lpmgr, 0, sizeof(struct os_lpmgr_dev));

    lp_ce = (os_clockevent_t *)os_device_find("lptim");
    OS_ASSERT(lp_ce != OS_NULL);
    OS_ASSERT(os_device_open((os_device_t *)lp_ce) == OS_EOK);

    lpmgr->ops = &lpmgr_ops;

    lp_min = OS_NS_TO_TICK(lp_ce->min_nsec);
    lp_max = OS_NS_TO_TICK(lp_ce->max_nsec);

    if (lp_min < LPMGR_MIN_SLEEPTICK)
    {
        lp_min = LPMGR_MIN_SLEEPTICK;
    }
    
    lpmgr->min_tick = lp_min;
    lpmgr->max_tick = lp_max;

    /* Initialize timer mask */
    sleep_mask = (1UL << SYS_SLEEP_MODE_LIGHT) | (1UL << SYS_SLEEP_MODE_DEEP);

    /* Initialize system lpmgr module */
    os_lpmgr_register(lpmgr, sleep_mask, OS_NULL);

    return 0;
}

OS_PREV_INIT(drv_lpmgr_hw_init, OS_INIT_SUBLEVEL_MIDDLE);
