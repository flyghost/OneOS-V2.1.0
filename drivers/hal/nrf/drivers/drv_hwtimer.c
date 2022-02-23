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
 * @brief       This file implements hwtimer driver for nrf5.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_task.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.hwtimer"
#include <drv_log.h>

#include <os_memory.h>
#include "drv_hwtimer.h"

struct nrf5_timer
{
    #if defined(OS_USING_CLOCKSOURCE) || defined(OS_USING_CLOCKEVENT)
    union _clock {
        #ifdef OS_USING_CLOCKSOURCE
        os_clocksource_t   cs;
        #endif
        #ifdef OS_USING_CLOCKEVENT
        os_clockevent_t    ce;
        #endif
    } clock;
    #endif

    os_uint32_t freq;
    struct nrf5_timer_info *info;
    os_list_node_t list;
};

static os_list_node_t nrf5_timer_list = OS_LIST_INIT(nrf5_timer_list);

extern void set_hwtimer_int_event_flag(void);
/**
 * @brief Handler for timer events.
 */
void timer_irq_callback(nrf_timer_event_t event_type, void* p_context)
{
    struct nrf5_timer *timer;

    os_list_for_each_entry(timer, &nrf5_timer_list, struct nrf5_timer, list)
    {
        if(&timer->info->timer_periph == (nrfx_timer_t *)p_context){
            #ifdef OS_USING_LPMGR
            set_hwtimer_int_event_flag();
            #endif
            os_clockevent_isr((os_clockevent_t *)timer);
            return;
        }
    }
}

static os_uint64_t nrf5_timer_read(void *clock)
{
    uint64_t ret;
    struct nrf5_timer *timer;

    timer = (struct nrf5_timer *)clock;

    ret = nrfx_timer_capture(&timer->info->timer_periph, NRF_TIMER_CC_CHANNEL3);
    return ret;
}

void nrf5_timer_init(struct nrf5_timer_info *info)
{
    nrfx_timer_config_t timer_cfg = NRFX_TIMER_DEFAULT_CONFIG;
    nrf_timer_frequency_t fre_num = NRF_TIMER_FREQ_1MHz;
    timer_cfg.interrupt_priority = 1;
    timer_cfg.frequency = fre_num;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    timer_cfg.p_context = &info->timer_periph;
    
    nrfx_timer_init(&info->timer_periph, &timer_cfg, timer_irq_callback);
}

static void nrf5_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct nrf5_timer *timer;
    
    OS_ASSERT(ce != OS_NULL);
    OS_ASSERT(count != 0);

    timer = (struct nrf5_timer *)ce;

    nrf5_timer_init(timer->info);

    nrfx_timer_extended_compare(&timer->info->timer_periph, timer->info->channel, count,
                                    timer->info->short_mask, true);

    nrfx_timer_enable(&timer->info->timer_periph);
}

static void nrf5_timer_stop(os_clockevent_t *ce)
{
    struct nrf5_timer *timer;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct nrf5_timer *)ce;

    nrfx_timer_uninit(&timer->info->timer_periph);
}

static const struct os_clockevent_ops nrf5_tim_ops =
{
    .start = nrf5_timer_start,
    .stop  = nrf5_timer_stop,
    .read  = nrf5_timer_read,
};

static os_uint32_t nrf5_clock_freq(struct nrf5_timer_info *info)
{
    return 1000000;//NRF_TIMER_FREQ_1MHz
}

static int nrf5_timer_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct nrf5_timer_info *info = (struct nrf5_timer_info *)dev->info;
    struct nrf5_timer *nrf5_timer = os_calloc(1, sizeof(struct nrf5_timer));

    OS_ASSERT(nrf5_timer);

    nrf5_timer->info = info;
    

#ifdef OS_USING_CLOCKSOURCE
    if (os_clocksource_best() == OS_NULL)
    {

        nrf5_timer->freq = nrf5_clock_freq(info);

        nrf5_timer->clock.cs.rating    = 320;
        nrf5_timer->clock.cs.freq    = nrf5_clock_freq(info);
        nrf5_timer->clock.cs.mask    = 0xfffffffful;
        nrf5_timer->clock.cs.read    = nrf5_timer_read;

        nrf5_timer_init(nrf5_timer->info);
        nrfx_timer_extended_compare(&nrf5_timer->info->timer_periph, nrf5_timer->info->channel, nrf5_timer->clock.cs.mask,
                                    nrf5_timer->info->short_mask, false);
        nrfx_timer_enable(&nrf5_timer->info->timer_periph);
        
        os_clocksource_register(dev->name, &nrf5_timer->clock.cs);
    }
    else
#endif

#ifdef OS_USING_CLOCKEVENT
    {
        nrf5_timer->clock.ce.rating  = 320;
        nrf5_timer->clock.ce.freq    = nrf5_clock_freq(info);
        nrf5_timer->clock.ce.mask    = 0xfffffffful;
        
        nrf5_timer->clock.ce.prescaler_mask = 0xfffffffful;
        nrf5_timer->clock.ce.prescaler_bits = 32;

        nrf5_timer->clock.ce.count_mask = 0xfffffffful; 
        nrf5_timer->clock.ce.count_bits = 32;

        nrf5_timer->clock.ce.feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

        nrf5_timer->clock.ce.min_nsec = NSEC_PER_SEC / nrf5_timer->clock.ce.freq;
        
        nrf5_timer->clock.ce.ops     = &nrf5_tim_ops;
        os_clockevent_register(dev->name, &nrf5_timer->clock.ce);
    }
#endif
    
    level = os_irq_lock();
    os_list_add_tail(&nrf5_timer_list, &nrf5_timer->list);
    os_irq_unlock(level);

    return result;
}

OS_DRIVER_INFO nrf5_timer_driver = {
    .name   = "TIMER_Type",
    .probe  = nrf5_timer_probe,
};

OS_DRIVER_DEFINE(nrf5_timer_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

