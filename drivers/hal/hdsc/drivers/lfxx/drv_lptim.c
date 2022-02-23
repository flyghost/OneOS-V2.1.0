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
 * @file        drv_lptim.c
 *
 * @brief       This file implements low power timer driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_task.h>
#include <os_memory.h>
#include <timer/timer.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>

#include <drv_log.h>

#include <hc_lptim.h>
#include <drv_lptim.h>

static int lptim_status = 0;

static os_list_node_t hc32_lptim_list = OS_LIST_INIT(hc32_lptim_list);

struct hc32_lptimer {
    os_clockevent_t ce;
    struct hc32_lptim_info *info;
    os_list_node_t list;
    os_uint32_t freq;
};

#ifdef  BSP_USING_LPTIM0
void LpTim0_IRQHandler(void)
{
    struct hc32_lptimer *lptimer;

    if (TRUE == Lptim_GetItStatus(M0P_LPTIMER0))
    {
        Lptim_ClrItStatus(M0P_LPTIMER0);

        os_list_for_each_entry(lptimer, &hc32_lptim_list, struct hc32_lptimer, list)
        {
            if (lptimer->info->base == M0P_LPTIMER0)
            {
                os_clockevent_isr((os_clockevent_t *)(&lptimer->ce));
                break;
            }
        }
    }
}
#endif

#ifdef  BSP_USING_LPTIM1
void LpTim1_IRQHandler(void)
{
    struct hc32_lptimer *lptimer;

    if (TRUE == Lptim_GetItStatus(M0P_LPTIMER1))
    {
        Lptim_ClrItStatus(M0P_LPTIMER1);

        os_list_for_each_entry(lptimer, &hc32_lptim_list, struct hc32_lptimer, list)
        {
            if (lptimer->info->base == M0P_LPTIMER1)
            {
                os_clockevent_isr((os_clockevent_t *)(&lptimer->ce));
                break;
            }
        }
    }
}
#endif

static void hc32_lptimer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct hc32_lptimer *timer;
    struct hc32_lptim_info *lptim_info;
    stc_lptim_cfg_t    stcLptCfg;

    OS_ASSERT(prescaler == 1);
    OS_ASSERT(count != 0);

    if (lptim_status == 1)
    {
        Lptim_Cmd(lptim_info->base, FALSE);
        lptim_status = 0;
    }

    timer = (struct hc32_lptimer *)ce;
    lptim_info = (struct hc32_lptim_info *)timer->info;

    DDL_ZERO_STRUCT(stcLptCfg);

    stcLptCfg.enGate   = LptimGateLow;
    stcLptCfg.enGatep  = LptimGatePLow;
    stcLptCfg.enTcksel = LptimRcl;
    stcLptCfg.enTogen  = LptimTogEnLow;
    stcLptCfg.enCt     = LptimTimerFun;
    stcLptCfg.enMd     = LptimMode1;
    stcLptCfg.u16Arr   = timer->ce.count_mask + 1 - (count & timer->ce.count_mask);

    Lptim_Init(lptim_info->base, &stcLptCfg);

    /* prescale */
    lptim_info->base->CR_f.PRS = lptim_info->prescale;

    Lptim_ClrItStatus(lptim_info->base);
    Lptim_ConfIt(lptim_info->base, TRUE);

    Lptim_Cmd(lptim_info->base, TRUE);

    lptim_status = 1;
}

static void hc32_lptimer_stop(os_clockevent_t *ce)
{
    struct hc32_lptimer *lptimer;
    struct hc32_lptim_info *lptim_info;

    lptimer = (struct hc32_lptimer *)ce;

    lptim_info = (struct hc32_lptim_info *)lptimer->info;

    Lptim_Cmd(lptim_info->base, FALSE);

    lptim_status = 0;
}

os_uint64_t hc32_lptimer_read(void *clock)
{
    struct hc32_lptimer *lptimer = OS_NULL;
    struct hc32_lptim_info *lptim_info;
    os_uint32_t cnt = 0, arr = 0;

    lptimer = (struct hc32_lptimer *)clock;

    lptim_info = (struct hc32_lptim_info *)lptimer->info;

    cnt = (os_uint32_t)(lptim_info->base->CNT_f.CNT);
    arr = (os_uint32_t)(lptim_info->base->ARR_f.ARR);

    if (cnt < arr)
        return (os_uint64_t)(lptimer->ce.mask + 1 - arr);
    else
        return (os_uint64_t)(cnt - arr);
}

static const struct os_clockevent_ops hc32_lptim_ops =
{
    .start = hc32_lptimer_start,
    .stop  = hc32_lptimer_stop,
    .read  = hc32_lptimer_read,
};

static void __os_hw_lptim_init(void)
{
#ifdef  BSP_USING_LPTIM0
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim0, TRUE);
#endif
#ifdef  BSP_USING_LPTIM1
    Sysctrl_SetPeripheralGate(SysctrlPeripheralLpTim1, TRUE);
#endif
    Sysctrl_ClkSourceEnable(SysctrlClkRCL, TRUE);
    EnableNvic(LPTIM_0_1_IRQn, IrqLevel3, TRUE);
}

static int hc32_lptim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    struct hc32_lptimer *lptimer;

    lptimer = os_calloc(1, sizeof(struct hc32_lptimer));
    OS_ASSERT(lptimer);

    lptimer->info = (struct hc32_lptim_info *)dev->info;

    level = os_irq_lock();
    os_list_add_tail(&hc32_lptim_list, &lptimer->list);
    os_irq_unlock(level);

    __os_hw_lptim_init();

    lptimer->freq = 32768 / (1 << lptimer->info->prescale);

    lptimer->ce.rating  = 50;
    lptimer->ce.freq    = lptimer->freq;
    lptimer->ce.mask    = 0xfffful;

    lptimer->ce.prescaler_mask = 1;
    lptimer->ce.prescaler_bits = 0;

    lptimer->ce.count_mask = 0xfffful;
    lptimer->ce.count_bits = 16;

    lptimer->ce.feature  = OS_CLOCKEVENT_FEATURE_ONESHOT;

    lptimer->ce.min_nsec = NSEC_PER_SEC / lptimer->ce.freq;

    lptimer->ce.ops     = &hc32_lptim_ops;

    os_clockevent_register(dev->name, &lptimer->ce);

    return OS_EOK;
}

OS_DRIVER_INFO hc32_lptim_driver = {
    .name   = "LPTIMER_Type",
    .probe  = hc32_lptim_probe,
};

OS_DRIVER_DEFINE(hc32_lptim_driver, PREV, OS_INIT_SUBLEVEL_HIGH);
