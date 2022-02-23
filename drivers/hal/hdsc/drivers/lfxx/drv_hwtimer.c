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
 * @file        drv_hwtimer.c
 *
 * @brief       This file implements timer driver for hc32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <timer/timer.h>

#ifdef OS_USING_CLOCKSOURCE
#include <timer/clocksource.h>
#endif

#ifdef OS_USING_CLOCKEVENT
#include <timer/clockevent.h>
#endif

#include <drv_hwtimer.h>

#ifdef OS_USING_PWM
#include "drv_pwm.h"
#endif

#ifdef OS_USING_PULSE_ENCODER
#include "drv_pulse_encoder.h"
#endif

#include <drv_log.h>

#define TIMER_MODE_PWM 0x01
#define TIMER_MODE_PULSE_ENCODER 0x02

static os_list_node_t hc32_timer_list = OS_LIST_INIT(hc32_timer_list);

struct hc32_timer {
    union _clock
    {
        os_clocksource_t   cs;
        os_clockevent_t    ce;
    } clock;

    struct hc32_tim_info *info;
    os_uint32_t freq;
    os_list_node_t list;

    void *user_data;
};

void Tim1_IRQHandler(void)
{
    struct hc32_timer *timer;

    if(TRUE == Bt_GetIntFlag(TIM1, BtUevIrq))
    {
        Bt_ClearIntFlag(TIM1, BtUevIrq);

#ifdef OS_USING_CLOCKEVENT
        os_list_for_each_entry(timer, &hc32_timer_list, struct hc32_timer, list)
        {
            if ((timer->info->mode == TIMER_MODE_TIM) && (timer->info->unit == TIM1))
            {
                os_clockevent_isr((os_clockevent_t *)timer);
                break;
            }
        }
#endif
    }
}

static os_uint64_t hc32_timer_read(void *clock)
{
    struct hc32_timer *timer;

    timer = (struct hc32_timer *)clock;
    return Bt_M0_Cnt32Get(timer->info->unit);
}

#ifdef OS_USING_CLOCKEVENT

en_bt_cr_timclkdiv_t hc32_calc_div(os_uint32_t prescaler)
{
    if((1 << BtPCLKDiv1) >= prescaler)
    {
        return BtPCLKDiv1;
    }
    else if((1 << BtPCLKDiv2) >= prescaler)
    {
        return BtPCLKDiv2;
    }

    else if((1 << BtPCLKDiv4) >= prescaler)
    {
        return BtPCLKDiv4;
    }

    else if((1 << BtPCLKDiv8) >= prescaler)
    {
        return BtPCLKDiv8;
    }

    else if((1 << BtPCLKDiv16) >= prescaler)
    {
        return BtPCLKDiv16;
    }

    else if((1 << BtPCLKDiv32) >= prescaler)
    {
        return BtPCLKDiv32;
    }
    else if((1 << BtPCLKDiv64) >= prescaler)
    {
        return BtPCLKDiv64;
    }
    else
    {
        return BtPCLKDiv256;
    }
}

static void hc32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct hc32_timer *timer;
    stc_bt_mode0_cfg_t	   stcBtBaseCfg;

    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer = (struct hc32_timer *)ce;

    DDL_ZERO_STRUCT(stcBtBaseCfg);
    stcBtBaseCfg.enWorkMode = BtWorkMode0;
    stcBtBaseCfg.enCT		= BtTimer;
    stcBtBaseCfg.enPRS		= hc32_calc_div(prescaler);
    stcBtBaseCfg.enCntMode	= Bt32bitFreeMode;
    stcBtBaseCfg.bEnTog 	= FALSE;
    stcBtBaseCfg.bEnGate	= FALSE;
    stcBtBaseCfg.enGateP	= BtGatePositive;

    Bt_Mode0_Init(timer->info->unit, &stcBtBaseCfg);

    Bt_M0_ARRSet(timer->info->unit, timer->clock.ce.count_mask + 1 - (count & timer->clock.ce.count_mask));
    Bt_M0_Cnt32Set(timer->info->unit, timer->clock.ce.count_mask + 1 - (count & timer->clock.ce.count_mask));

    Bt_ClearIntFlag(timer->info->unit, BtUevIrq);
    Bt_Mode0_EnableIrq(timer->info->unit);
    EnableNvic(timer->info->irqn, IrqLevel3, TRUE);
    Bt_M0_Run(timer->info->unit);
}

static void hc32_timer_stop(os_clockevent_t *ce)
{
    struct hc32_timer *timer;

    OS_ASSERT(ce != OS_NULL);
    timer = (struct hc32_timer *)ce;

    Bt_Mode0_DisableIrq(timer->info->unit);
    Bt_M0_Stop(timer->info->unit);
}

static const struct os_clockevent_ops hc32_tim_ops =
{
    .start = hc32_timer_start,
    .stop  = hc32_timer_stop,
    .read  = hc32_timer_read,
};
#endif

static int hc32_tim_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_uint32_t freq;
    struct hc32_timer *timer;
    stc_bt_mode0_cfg_t	   stcBtBaseCfg;

    Sysctrl_SetPeripheralGate(SysctrlPeripheralBaseTim, TRUE);

    timer = os_calloc(1, sizeof(struct hc32_timer));
    OS_ASSERT(timer);

    timer->info = (struct hc32_tim_info *)dev->info;

    freq = Sysctrl_GetPClkFreq();

    if (timer->info->mode == TIMER_MODE_TIM)
    {
#ifdef OS_USING_CLOCKSOURCE
        if (os_clocksource_best() == OS_NULL)
        {
            DDL_ZERO_STRUCT(stcBtBaseCfg);

            stcBtBaseCfg.enWorkMode = BtWorkMode0;
            stcBtBaseCfg.enCT		= BtTimer;
            stcBtBaseCfg.enPRS		= BtPCLKDiv1;
            stcBtBaseCfg.enCntMode	= Bt32bitFreeMode;
            stcBtBaseCfg.bEnTog 	= FALSE;
            stcBtBaseCfg.bEnGate	= FALSE;
            stcBtBaseCfg.enGateP	= BtGatePositive;
            Bt_Mode0_Init(timer->info->unit, &stcBtBaseCfg);

            Bt_M0_Cnt32Set(timer->info->unit, 0);
            Bt_M0_Run(timer->info->unit);

            timer->freq = freq / (1 << stcBtBaseCfg.enPRS);

            timer->clock.cs.rating	= 320;
            timer->clock.cs.freq	= timer->freq;
            timer->clock.cs.mask	= 0xffffffffull;
            timer->clock.cs.read	= hc32_timer_read;

            os_clocksource_register(dev->name, &timer->clock.cs);
        }
        else
#endif
        {
#ifdef OS_USING_CLOCKEVENT
            stcBtBaseCfg.enWorkMode = BtWorkMode0;
            stcBtBaseCfg.enCT		= BtTimer;
            stcBtBaseCfg.enPRS		= BtPCLKDiv1;
            stcBtBaseCfg.enCntMode	= Bt32bitFreeMode;
            stcBtBaseCfg.bEnTog 	= FALSE;
            stcBtBaseCfg.bEnGate	= FALSE;
            stcBtBaseCfg.enGateP	= BtGatePositive;
            Bt_Mode0_Init(timer->info->unit, &stcBtBaseCfg);

            timer->freq = freq / (1 << stcBtBaseCfg.enPRS);

            timer->clock.ce.rating	= (stcBtBaseCfg.enCntMode == Bt32bitFreeMode) ? 320 : 160;
            timer->clock.ce.freq    = timer->freq;
            timer->clock.ce.mask	= 0xffffffffull;
            timer->clock.ce.prescaler_mask = 0xfffful;
            timer->clock.ce.prescaler_bits = 16;
            timer->clock.ce.count_mask = (stcBtBaseCfg.enCntMode == Bt32bitFreeMode) ? 0xfffffffful : 0xfffful;
            timer->clock.ce.count_bits = (stcBtBaseCfg.enCntMode == Bt32bitFreeMode) ? 32 : 16;
            timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_ONESHOT;
            timer->clock.ce.min_nsec = NSEC_PER_SEC / timer->clock.ce.freq;
            timer->clock.ce.ops 	= &hc32_tim_ops;

            os_clockevent_register(dev->name, &timer->clock.ce);
#endif
        }
    }

    os_list_add(&hc32_timer_list, &timer->list);

    return OS_EOK;
}

OS_DRIVER_INFO hc32_tim_driver = {
    .name   = "TIMER_Type",
    .probe  = hc32_tim_probe,
};

OS_DRIVER_DEFINE(hc32_tim_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);
