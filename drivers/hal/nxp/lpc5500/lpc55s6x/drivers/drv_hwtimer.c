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
 * @brief       This file implements hwtimer driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_task.h>

#define LOG_TAG             "drv.hwtimer"
#include <drv_log.h>

#include <device.h>
#include <os_memory.h>
#include "drv_hwtimer.h"
#include "fsl_ctimer.h"

typedef struct lpc_ctimer
{
    os_clockevent_t ce;
    struct lpc_ctimer_info *info;
    os_list_node_t list;
}lpc_ctimer_t;

static os_list_node_t lpc_ctimer_list = OS_LIST_INIT(lpc_ctimer_list);

static void ctimer_irq_callback(struct lpc_ctimer *lpc_ctimer)
{
    CTIMER_Type *base = lpc_ctimer->info->ctimer_base;

    uint32_t int_stat;
    
    int_stat = CTIMER_GetStatusFlags(base);
    
    CTIMER_ClearStatusFlags(base, int_stat);
    
    os_clockevent_isr((os_clockevent_t *)lpc_ctimer);
}

#define CTIMER_IRQHandler_DEFINE(__index)                                           \
void CTIMER##__index##_IRQHandler(void)                                             \
{                                                                                   \
    struct lpc_ctimer *lpc_ctimer;                                                  \
                                                                                    \
    os_list_for_each_entry(lpc_ctimer, &lpc_ctimer_list, struct lpc_ctimer, list)   \
    {                                                                               \
        if (lpc_ctimer->info->ctimer_base == CTIMER##__index)                       \
        {                                                                           \
            break;                                                                  \
        }                                                                           \
    }                                                                               \
                                                                                    \
    if (lpc_ctimer->info->ctimer_base == CTIMER##__index)                           \
        ctimer_irq_callback(lpc_ctimer);                                            \
}

CTIMER_IRQHandler_DEFINE(0);
CTIMER_IRQHandler_DEFINE(1);
CTIMER_IRQHandler_DEFINE(2);
CTIMER_IRQHandler_DEFINE(3);
CTIMER_IRQHandler_DEFINE(4);

static os_uint64_t lpc_timer_read(void *clock)
{
    struct lpc_ctimer *timer;

    timer = (struct lpc_ctimer *)clock;

    return timer->info->ctimer_base->TC;
}

static void lpc_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct lpc_ctimer *timer;
    CTIMER_Type *ctimer_base;

    OS_ASSERT(ce != OS_NULL);
    OS_ASSERT(prescaler == 0);
    OS_ASSERT(count != 0);

    timer = (struct lpc_ctimer *)ce;
    
    ctimer_base = timer->info->ctimer_base;

    /* Match Configuration for Channel 0 */
    ctimer_match_config_t matchCfg;

    /* Configuration*/
    matchCfg.enableCounterReset = true;
    matchCfg.enableCounterStop  = false;    /* period */
    matchCfg.matchValue         = count;
    matchCfg.outControl         = kCTIMER_Output_NoAction;
    matchCfg.outPinInitState    = false;
    matchCfg.enableInterrupt    = true;
    
    CTIMER_SetupMatch(ctimer_base, kCTIMER_Match_0, &matchCfg);
    CTIMER_StartTimer(ctimer_base);
}

static void lpc_timer_stop(os_clockevent_t *ce)
{
    struct lpc_ctimer *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct lpc_ctimer *)ce;

    CTIMER_StopTimer(timer->info->ctimer_base);
    CTIMER_Reset(timer->info->ctimer_base);
}

static const struct os_clockevent_ops lpc_tim_ops =
{
    .start = lpc_timer_start,
    .stop  = lpc_timer_stop,
    .read  = lpc_timer_read,
};

static os_uint32_t lpc_clock_freq(struct lpc_ctimer_info *info)
{
    CTIMER_Type *ctimer_base = info->ctimer_base;

#ifdef CTIMER0_PERIPHERAL
    if (ctimer_base == CTIMER0_PERIPHERAL)
        return CTIMER0_TICK_FREQ;
#endif

#ifdef CTIMER1_PERIPHERAL
    if (ctimer_base == CTIMER1_PERIPHERAL)
        return CTIMER1_TICK_FREQ;
#endif

#ifdef CTIMER2_PERIPHERAL
    if (ctimer_base == CTIMER2_PERIPHERAL)
        return CTIMER2_TICK_FREQ;
#endif

#ifdef CTIMER3_PERIPHERAL
    if (ctimer_base == CTIMER3_PERIPHERAL)
        return CTIMER3_TICK_FREQ;
#endif

#ifdef CTIMER4_PERIPHERAL
    if (ctimer_base == CTIMER4_PERIPHERAL)
        return CTIMER4_TICK_FREQ;
#endif

    return 0;
}

static int lpc_ctimer_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct lpc_ctimer_info *info = (struct lpc_ctimer_info *)dev->info;
    struct lpc_ctimer *lpc_ctimer = os_calloc(1, sizeof(struct lpc_ctimer));

    OS_ASSERT(lpc_ctimer);

    lpc_ctimer->info = info;

    os_clockevent_t *ce = &lpc_ctimer->ce;

    ce->rating  = 320;
    ce->freq    = lpc_clock_freq(info);
    ce->mask    = 0xffffffffull;

    ce->prescaler_mask = 0;
    ce->prescaler_bits = 0;

    ce->count_mask  = 0xfffffffful;
    ce->count_bits  = 32;

    ce->feature     = OS_CLOCKEVENT_FEATURE_PERIOD;

    ce->min_nsec    = NSEC_PER_SEC / ce->freq;

    ce->ops         = &lpc_tim_ops;
    os_clockevent_register(dev->name, ce);

    level = os_irq_lock();
    os_list_add_tail(&lpc_ctimer_list, &lpc_ctimer->list);
    os_irq_unlock(level);

    return result;
}

OS_DRIVER_INFO lpc_ctimer_driver = {
    .name   = "CTIMER_Type",
    .probe  = lpc_ctimer_probe,
};

OS_DRIVER_DEFINE(lpc_ctimer_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);
