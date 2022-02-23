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
 * @file        os_clock.c
 *
 * @brief       This file implements the the operating system tick handling.
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_task.h>
#include <arch_interrupt.h>

#include "os_kernel_internal.h"

#define TICK_Q_BUCKETS      8
static os_list_node_t gs_os_tickq_bucket[TICK_Q_BUCKETS];   /* Doubly linked bucket list head */
static os_tick_t gs_os_tick = 0;    /*Operating system tick*/

/**
 ***********************************************************************************************************************
 * @brief           This function initialize tick queue.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_tickq_init(void)
{
    os_size_t i;

    for (i = 0; i < TICK_Q_BUCKETS; i++ ) 
    {
        os_list_init(&gs_os_tickq_bucket[i]);
    }

    return;
}

OS_INLINE void _k_tickq_delta_insert(os_list_node_t *head, struct os_task *task)
{
    os_list_node_t *pos;
    struct os_task *task_iter;

    os_list_for_each(pos, head)
    {
        task_iter = os_list_entry(pos, os_task_t, tick_node);
        /*To avoid wrapping, use delta sorting*/
        if (task->tick_timeout < (task_iter->tick_absolute - gs_os_tick)) 
        {
            /* Insert this tick_node before the task_iter. */
            os_list_add_tail(&task_iter->tick_node, &task->tick_node);
            break;
        }
    }

    if (pos == head)
    {
        os_list_add_tail(head, &task->tick_node);
    }
    
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function put a task on the tick queue.
 *                 
 * @param[in]       task        Task control block.
 * @param[in]       timeout     Task timeout time(unit:tick).
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_tickq_put(struct os_task *task, os_tick_t timeout)
{
    os_list_node_t *tickq_head;
    os_int32_t      bidx;

    task->tick_timeout  = timeout;
    task->tick_absolute = gs_os_tick + timeout;

    bidx       = task->tick_absolute & (TICK_Q_BUCKETS - 1);
    tickq_head = &gs_os_tickq_bucket[bidx];

    _k_tickq_delta_insert(tickq_head, task);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function remove a task from the tick queue.
 *                 
 * @param[in]       task    Task control block.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_tickq_remove(struct os_task *task)
{
    os_list_del(&task->tick_node);
    
    return;
}

void os_tick_increase(void)
{
    os_int32_t      bidx;
    os_list_node_t *tickq_head;
    struct os_task *iter_task;
    os_bool_t       is_sched;

    OS_KERNEL_INIT();

    /*The operating system is not running*/
    if (OS_NULL == g_os_current_task)
    {
        return;
    }

    is_sched = OS_FALSE;

    OS_KERNEL_ENTER();

    gs_os_tick++;

    /* Process tick queue */
    bidx        = gs_os_tick & (TICK_Q_BUCKETS - 1);
    tickq_head  = &gs_os_tickq_bucket[bidx];

    while (1)
    {
        if (os_list_empty(tickq_head))
        {   
            break;
        }

        iter_task = os_list_first_entry(tickq_head, os_task_t, tick_node);

        /* No sleeping task need to be waked */
        if ((os_base_t)(iter_task->tick_absolute - gs_os_tick) > 0)
        {
            break;
        }

        k_tickq_remove(iter_task);
        iter_task->state &= ~OS_TASK_STATE_SLEEP;

        if (iter_task->state & OS_TASK_STATE_BLOCK)
        {
            os_list_del(&iter_task->task_node);
            iter_task->state        &= ~OS_TASK_STATE_BLOCK;
            iter_task->switch_retval = OS_ETIMEOUT;   
        }

        /* The task state may be suspend, empty or running */
        if (OS_TASK_STATE_SUSPEND != iter_task->state)
        {
            iter_task->state |= OS_TASK_STATE_READY;
            k_readyq_put(iter_task);

            is_sched = OS_TRUE;
        }

        OS_KERNEL_EXIT();

        OS_KERNEL_ENTER();
    }

    /* Process time slice */
    g_os_current_task->remaining_time_slice--;
    if (!g_os_current_task->remaining_time_slice)
    {
        g_os_current_task->remaining_time_slice = g_os_current_task->time_slice;

        if ((g_os_current_task->state & OS_TASK_STATE_READY)
            && (g_os_current_task->task_node.next != g_os_current_task->task_node.prev))
        {
            k_readyq_move_tail(g_os_current_task);
            is_sched = OS_TRUE;
        }
    }

#ifdef OS_USING_TIMER
    /* Is need timer handle.*/
    if (k_timer_need_handle())
    {
        is_sched = OS_TRUE;
    }
    /* Move gs_os_timer_list_current forward one step. */
    k_move_timer_list_one_step();
#endif

    if (is_sched)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }
    
    return;    
}

/**
 ***********************************************************************************************************************
 * @brief           This function get os tick.
 *                  
 * @param[in]       None.
 *
 * @return          System global tick.
 ***********************************************************************************************************************
 */ 
os_tick_t os_tick_get(void)
{
    return gs_os_tick;
}

/**
 ***********************************************************************************************************************
 * @brief           This function set os tick..
 * 
 * @param[in]       The tick you want to set.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void os_tick_set(os_tick_t tick)
{
    OS_KERNEL_INIT();

    OS_KERNEL_ENTER();

    gs_os_tick = tick;

    OS_KERNEL_EXIT();

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function convert milliseconds to ticks.
 *               
 * @param[in]       The number of milliseconds to convert.
 *
 * @return          The converted ticks.
 ***********************************************************************************************************************
 */ 
os_tick_t os_tick_from_ms(os_uint32_t ms)
{
    os_tick_t tick;

    tick  = OS_TICK_PER_SECOND * (ms / 1000);
    tick += (OS_TICK_PER_SECOND * (ms % 1000) + 999) / 1000;

    return tick;
}

#ifdef OS_USING_TICKLESS_LPMGR

static os_uint32_t _k_task_is_need_schedule(os_uint32_t idx)
{
    os_list_node_t *tickq_head;
    os_bool_t       is_sched;
    struct os_task *iter_task;
    tickq_head  = &gs_os_tickq_bucket[idx];
    is_sched = OS_FALSE;
    while (1)
    {
        if (os_list_empty(tickq_head))
        {   
            break;
        }
    
        iter_task = os_list_first_entry(tickq_head, os_task_t, tick_node);

        /* No sleeping task need to be waked */
        if ((os_base_t)(iter_task->tick_absolute - gs_os_tick) > 0)
        {
            break;
        }

        k_tickq_remove(iter_task);
        iter_task->state &= ~OS_TASK_STATE_SLEEP;

        if (iter_task->state & OS_TASK_STATE_BLOCK)
        {
            os_list_del(&iter_task->task_node);
            iter_task->state        &= ~OS_TASK_STATE_BLOCK;
            iter_task->switch_retval = OS_ETIMEOUT;   
        }

        /* The task state may be suspend, empty or running */
        if (OS_TASK_STATE_SUSPEND != iter_task->state)
        {
            iter_task->state |= OS_TASK_STATE_READY;
            k_readyq_put(iter_task);

            is_sched = OS_TRUE;
        }

    }

    return is_sched;
}

static os_uint32_t _k_wake_up_sleep_task(void)
{
    os_uint32_t need_sched = OS_FALSE;
    os_int32_t      bidx   = 0;
  
    for (bidx = 0; bidx < TICK_Q_BUCKETS; bidx++)
    {
        if (OS_TRUE == _k_task_is_need_schedule(bidx))
        {
            need_sched = OS_TRUE;
        }
    }

    return need_sched;
}

OS_INLINE os_tick_t _k_tickq_get_next_ticks(void)
{
    os_size_t       i;
    struct os_task *task;
    os_list_node_t *tickq_head;
    os_tick_t       min_ticks = OS_TICK_MAX;
    os_tick_t       diff_ticks;

    for (i = 0; i < TICK_Q_BUCKETS; i++) 
    {
        tickq_head = &gs_os_tickq_bucket[i];

        if (!os_list_empty(tickq_head))
        {
            task = os_list_first_entry(tickq_head, os_task_t, tick_node);

            diff_ticks = task->tick_absolute - gs_os_tick;

            if ((os_base_t)diff_ticks <= 0)
            {
                min_ticks = 0;
                break;
            }
            else
            {
                min_ticks = (min_ticks > diff_ticks) ? diff_ticks : min_ticks;
            }
        }
    }

    return (0 == min_ticks || OS_TICK_MAX == min_ticks) ? min_ticks : (min_ticks - 1);
}

/**
 ***********************************************************************************************************************
 * @brief           This function gets the smaller remaining ticks of the expiring sleep task and timer.
 *
 * @param           None
 *
 * @return          Return time value in ticks.
 ***********************************************************************************************************************
 */
os_tick_t os_tickless_get_sleep_ticks(void)
{
    os_tick_t min_ticks = OS_TICK_MAX;
    os_tick_t tickq_next_ticks;

    OS_KERNEL_INIT();

    OS_KERNEL_ENTER();

#if defined(OS_USING_TIMER)
 
#if defined(OS_TIMER_SORT)
    min_ticks = k_timer_get_next_remain_ticks();
#else
#error "In low power mode, if a timer is used, OS_TIMER_SORT must be defined"
#endif

#endif

    tickq_next_ticks = _k_tickq_get_next_ticks();
    if (min_ticks > tickq_next_ticks)
    {
        min_ticks = tickq_next_ticks;
    }

    OS_KERNEL_EXIT();

    return min_ticks;
}

/**
 ***********************************************************************************************************************
 * @brief           This function update tick and timer after low power wakes up.
 *
 * @param[in]       sleep_ticks      Number of ticks that need to be updated.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_tickless_update(os_tick_t sleep_ticks)
{
    os_bool_t is_sched = OS_FALSE;

    OS_KERNEL_INIT();

    OS_KERNEL_ENTER();
    
    gs_os_tick += sleep_ticks;
    
    is_sched = _k_wake_up_sleep_task();

#if defined(OS_USING_TIMER)
 
#if defined(OS_TIMER_SORT)
    k_timer_update_active_list(sleep_ticks);
    if (k_timer_need_handle())
    {
        is_sched = OS_TRUE;
    }
    k_move_timer_list_one_step();
#else
#error "In low power mode, if a timer is used, OS_TIMER_SORT must be defined"
#endif

#endif

    if (is_sched)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

}

#endif
