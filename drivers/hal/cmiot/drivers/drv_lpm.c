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
 * @file        drv_lpm.c
 *
 * @brief       This file implements low power manager for cm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_clock.h>
#include <timer/clockevent.h>
#include <sys/time.h>
#include <cm32m101a_pwr.h>
#include <lpm.h>
#include <drv_common.h>

os_clockevent_t *lpm_ce = NULL;

static os_uint32_t os_irq_level = ~0u;

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

static os_tick_t lpm_tick_from_os_tick(os_tick_t tick)
{
    os_uint32_t freq = lpm_ce->freq;

    return (freq * tick / OS_TICK_PER_SECOND);
}

os_err_t lpm_timer_start_once(os_uint32_t timeout_ms)
{
    os_uint64_t sleep_ns;
    os_tick_t timeout;

    sleep_ns = NSEC_PER_MSEC * (os_uint64_t)timeout_ms;

    if (lpm_ce->min_nsec > sleep_ns)
    {
        os_kprintf("time too short, timeout_ms[%d ms], time[%d ms, %d ms]\r\n",
                   timeout_ms, lpm_ce->min_nsec / NSEC_PER_MSEC, lpm_ce->max_nsec / NSEC_PER_MSEC);
        return OS_EINVAL;
    }

    if (lpm_ce->max_nsec < sleep_ns)
    {
        os_kprintf("time too long, timeout_ms[%d ms], time[%d ms, %d ms]\r\n",
                   timeout_ms, lpm_ce->min_nsec / NSEC_PER_MSEC, lpm_ce->max_nsec / NSEC_PER_MSEC);
        return OS_EINVAL;
    }

    timeout = lpm_tick_from_os_tick(timeout_ms / (1000 / OS_TICK_PER_SECOND));

    os_clockevent_start_oneshot(lpm_ce, NSEC_PER_SEC * timeout / lpm_ce->freq);

    return OS_EOK;
}

void lpm_enter_sleep(void)
{
    cortexm_systick_tick_deinit();
    lpmgr_exit_critical(~0u, 3);

    PWR_EnterSTOP2Mode(PWR_STOPENTRY_WFI, PWR_CTRL3_RAM1RET);

    lpmgr_enter_critical(3);
    cortexm_systick_tick_init();
}

int lpm_init(void)
{
    lpm_ce = (os_clockevent_t *)os_device_find("lptim");
    OS_ASSERT(lpm_ce != NULL);

    return 0;
}

OS_DEVICE_INIT(lpm_init, OS_INIT_SUBLEVEL_MIDDLE);
