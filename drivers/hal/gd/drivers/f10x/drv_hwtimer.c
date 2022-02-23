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
 * @brief       This file implements hwtimer driver for gd32.
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

struct gd32_timer
{
    os_clockevent_t ce;
    struct gd32_timer_info *info;
    os_list_node_t list;
};

static os_list_node_t gd32_timer_list = OS_LIST_INIT(gd32_timer_list);

static void timer_irq_callback(struct gd32_timer *timer)
{
    uint32_t timer_periph = timer->info->timer_periph;

    if (timer_interrupt_flag_get(timer_periph, TIMER_INT_FLAG_UP) == SET)
    {
        timer_interrupt_flag_clear(timer_periph, TIMER_INT_FLAG_UP);
        os_clockevent_isr((os_clockevent_t *)timer);
    }
}

#define TIMER_IRQHandler_DEFINE(__uart_index)                                   \
void TIMER##__uart_index##_IRQHandler(void)                                     \
{                                                                               \
    struct gd32_timer *timer;                                                   \
                                                                                \
    os_list_for_each_entry(timer, &gd32_timer_list, struct gd32_timer, list)    \
    {                                                                           \
        if (timer->info->timer_periph == TIMER##__uart_index)                   \
        {                                                                       \
            timer_irq_callback(timer);                                          \
            return;                                                             \
        }                                                                       \
    }                                                                           \
}

TIMER_IRQHandler_DEFINE(1);
TIMER_IRQHandler_DEFINE(2);
TIMER_IRQHandler_DEFINE(3);
TIMER_IRQHandler_DEFINE(4);

static os_uint64_t gd32_timer_read(void *clock)
{
    struct gd32_timer *timer;

    timer = (struct gd32_timer *)clock;

    return timer_counter_read(timer->info->timer_periph);
}

static void gd32_timer_start(os_clockevent_t *ce, os_uint32_t prescaler, os_uint64_t count)
{
    struct gd32_timer *timer;
    uint32_t timer_periph;
    
    timer_parameter_struct timer_initpara = 
    {
        .prescaler         = prescaler,
        .alignedmode       = TIMER_COUNTER_EDGE,
        .counterdirection  = TIMER_COUNTER_UP,
        .period            = count,
        .clockdivision     = TIMER_CKDIV_DIV1,
        .repetitioncounter = 0,
    };

    OS_ASSERT(ce != OS_NULL);
    OS_ASSERT(prescaler != 0);
    OS_ASSERT(count != 0);

    timer = (struct gd32_timer *)ce;
    
    timer_periph = timer->info->timer_periph;

    timer_deinit(timer_periph);
    timer_init(timer_periph, &timer_initpara);
    timer_interrupt_flag_clear(timer_periph, TIMER_INT_FLAG_UP);

    timer_enable(timer_periph);
    timer_update_event_enable(timer_periph);
    timer_interrupt_enable(timer_periph, TIMER_INT_UP);
}

static void gd32_timer_stop(os_clockevent_t *ce)
{
    struct gd32_timer *timer;
    uint32_t timer_periph;

    OS_ASSERT(ce != OS_NULL);

    timer = (struct gd32_timer *)ce;

    timer_periph = timer->info->timer_periph;

    timer_interrupt_disable(timer_periph, TIMER_INT_UP);
    timer_disable(timer_periph);
}

static const struct os_clockevent_ops gd32_tim_ops =
{
    .start = gd32_timer_start,
    .stop  = gd32_timer_stop,
    .read  = gd32_timer_read,
};

static os_uint32_t gd32_clock_freq(struct gd32_timer_info *info)
{
    return SystemCoreClock / 2;
}

static int gd32_timer_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    os_err_t    result  = 0;

    struct gd32_timer_info *info = (struct gd32_timer_info *)dev->info;
    struct gd32_timer *gd32_timer = os_calloc(1, sizeof(struct gd32_timer));

    OS_ASSERT(gd32_timer);

    gd32_timer->info = info;

    os_clockevent_t *ce = &gd32_timer->ce;
   
    ce->rating  = 160;
    ce->freq    = gd32_clock_freq(info);
    ce->mask    = 0xffffffffull;
    
    ce->prescaler_mask = 0xfffful;
    ce->prescaler_bits = 16;

    ce->count_mask = 0xfffful;
    ce->count_bits = 16;

    ce->feature    = OS_CLOCKEVENT_FEATURE_PERIOD;

    ce->min_nsec = NSEC_PER_SEC / ce->freq;
    
    ce->ops     = &gd32_tim_ops;
    os_clockevent_register(dev->name, ce);

    level = os_irq_lock();
    os_list_add_tail(&gd32_timer_list, &gd32_timer->list);
    os_irq_unlock(level);

    rcu_periph_clock_enable(gd32_timer->info->periph);    
    nvic_irq_enable(gd32_timer->info->nvic_irq, 0, 2);
    
    return result;
}

OS_DRIVER_INFO gd32_timer_driver = {
    .name   = "TIMER_Type",
    .probe  = gd32_timer_probe,
};

OS_DRIVER_DEFINE(gd32_timer_driver, PREV, OS_INIT_SUBLEVEL_MIDDLE);

