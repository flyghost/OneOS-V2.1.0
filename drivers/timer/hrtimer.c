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
 * @file        cputime.c
 *
 * @brief       This file provides functions for cputime calculation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

/**
 * @file hrtimer.c
 * @author creekwater
 * @brief clockevent的实现
 * @version 0.1
 * @date 2022-03-01
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <arch_interrupt.h>
#include <device.h>
#include <os_task.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_clock.h>
#include <timer/clocksource.h>
#include <timer/clockevent.h>
#include <timer/hrtimer.h>

static os_list_node_t gs_hrtimer_list = OS_LIST_INIT(gs_hrtimer_list);

/**
 * @brief 启动一个定时器
 * 
 * @param next_nsec 定时的目标时间点（单位：纳秒）
 */
static void os_hrtimer_trig_hwtimer(os_uint64_t next_nsec)
{
    os_clockevent_t *ce;

    ce = os_clockevent_best();
    
    os_clockevent_stop(ce);
    
    os_clockevent_start_oneshot(ce, next_nsec - os_clocksource_gettime());
}

/**
 * @brief 在链表中插入一个新的定时事件
 * 
 * @param hrtimer 待插入的定时器
 */
static void os_hrtimer_enqueue(os_hrtimer_t *hrtimer)
{
    os_list_node_t *entry = &gs_hrtimer_list;
    os_hrtimer_t   *tmp;

    os_list_for_each_entry(tmp, &gs_hrtimer_list, os_hrtimer_t, list)
    {
        OS_ASSERT(tmp != hrtimer);
    
        if (tmp->next_nsec > hrtimer->next_nsec)
            break;
        entry = &tmp->list;
    }
    os_list_add(entry, &hrtimer->list);

    hrtimer->state = OS_HRTIMER_STATE_WAIT;
}

/**
 * @brief 定时器回调接口
 * 
 * @param ce 
 */
static void os_hrtimer_callback(os_clockevent_t * ce)
{
    os_hrtimer_t *hrtimer;
    os_hrtimer_t *tmp;

    os_list_for_each_entry_safe(hrtimer, tmp, &gs_hrtimer_list, os_hrtimer_t, list)
    {
        os_uint64_t now = os_clocksource_gettime();
    
        if (hrtimer->next_nsec > now)
            break;

        os_list_del(&hrtimer->list);
        
        hrtimer->state = OS_HRTIMER_STATE_RUN;      // 设置hrtimer为运行态
        hrtimer->timeout_func(hrtimer->parameter);  // 调用超时函数

        if (hrtimer->state != OS_HRTIMER_STATE_RUN)
            continue;

        // hrtimer为周期性定时器，需要在链表中再次插入新的定时器
        if (hrtimer->period_nsec != 0)
        {
            hrtimer->next_nsec = period_calc_next_nsec(hrtimer->next_nsec, now, hrtimer->period_nsec);  // 计算目标时间
            os_hrtimer_enqueue(hrtimer);    // 插入链表
        }
        else
        {
            hrtimer->state = OS_HRTIMER_STATE_NONE; 
        }
    }
    
    hrtimer = os_list_first_entry_or_null(&gs_hrtimer_list, os_hrtimer_t, list);
    if (hrtimer != OS_NULL)
    {
        os_hrtimer_trig_hwtimer(hrtimer->next_nsec);
    }
}

/**
 * @brief 启动定时器
 * 
 * @param hrtimer hrtimer实例
 */
void os_hrtimer_start(os_hrtimer_t *hrtimer)
{
    os_base_t level;

    hrtimer->next_nsec = os_clocksource_gettime() + hrtimer->timeout_nsec;

    level = os_irq_lock();

    os_hrtimer_enqueue(hrtimer);

    if (os_clockevent_best() == OS_NULL)
    {
        os_irq_unlock(level);
        return;
    }

    if (os_list_first_entry_or_null(&gs_hrtimer_list, os_hrtimer_t, list) == hrtimer)
    {
        os_hrtimer_trig_hwtimer(hrtimer->next_nsec);
    }

    os_irq_unlock(level);
}

/**
 * @brief 停止定时器
 * 
 * @param hrtimer 
 */
void os_hrtimer_stop(os_hrtimer_t *hrtimer)
{
    os_base_t     level;
    os_hrtimer_t *tmp;

    level = os_irq_lock();

    hrtimer->period_nsec = 0;

    if (hrtimer->state == OS_HRTIMER_STATE_RUN)
    {
        os_irq_unlock(level);
        return;
    }

    tmp = os_list_first_entry_or_null(&gs_hrtimer_list, os_hrtimer_t, list);

    os_list_del(&hrtimer->list);
    hrtimer->state = OS_HRTIMER_STATE_NONE;

    if (tmp == hrtimer)
    {
        hrtimer = os_list_first_entry_or_null(&gs_hrtimer_list, os_hrtimer_t, list);
        if (hrtimer != OS_NULL)
        {
            os_hrtimer_trig_hwtimer(hrtimer->next_nsec);
        }
    }

    os_irq_unlock(level);
}

/**
 * @brief hrtimer是否已经停止
 * 
 * @param hrtimer 
 * @return os_bool_t 
 */
os_bool_t os_hrtimer_stoped(os_hrtimer_t *hrtimer)
{
    return (hrtimer->state == OS_HRTIMER_STATE_NONE);
}

/**
 * @brief 是否将hrtimer用作系统tikc
 * 
 */
#ifdef OS_USING_HRTIMER_FOR_KERNEL_TICK

void os_tick_handler(void);

static os_hrtimer_t g_hrtimer_systick =
{
    .timeout_func   = (void (*)(void *parameter))os_tick_handler,
    .parameter      = "hrtimer_systick",
    .timeout_nsec   = NSEC_PER_SEC / OS_TICK_PER_SECOND,
    .period_nsec    = NSEC_PER_SEC / OS_TICK_PER_SECOND,
};

#endif

os_err_t os_hrtimer_init(void)
{
    os_clockevent_select_best();

    OS_ASSERT((os_clocksource_best() != OS_NULL && os_clockevent_best() != OS_NULL));

    os_clockevent_register_isr(os_clockevent_best(), os_hrtimer_callback);

    os_hrtimer_t *hrtimer = os_list_first_entry_or_null(&gs_hrtimer_list, os_hrtimer_t, list);

    if (hrtimer != OS_NULL)
    {
        os_hrtimer_trig_hwtimer(hrtimer->next_nsec);
    }

#ifdef OS_USING_HRTIMER_FOR_KERNEL_TICK
    os_hrtimer_start(&g_hrtimer_systick);
#endif

    return OS_EOK;
}

OS_PREV_INIT(os_hrtimer_init, OS_INIT_SUBLEVEL_LOW);

