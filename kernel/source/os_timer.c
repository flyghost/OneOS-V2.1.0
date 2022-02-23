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
 * @file        os_timer.c
 *
 * @brief       This file implements the timer functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_errno.h>
#include <os_timer.h>
#include <os_task.h>
#include <os_assert.h>
#include <os_clock.h>
#include <string.h>
#include <os_spinlock.h>
#include <arch_misc.h>
#include "os_kernel_internal.h"

#ifdef OS_USING_TIMER

#define OS_TIMER_POWER                       (4U)
#define OS_TIMER_LOW_MASK                    ((1U << OS_TIMER_POWER) - 1U)
#define OS_TIMER_MOD(tick)                   ((tick) & OS_TIMER_LOW_MASK)
#define OS_TIMER_ROUND(tick)                 ((tick) & (~OS_TIMER_LOW_MASK))
#define OS_TIMER_LIST_ENTRIES                (1U << OS_TIMER_POWER)
#define OS_TIMER_IS_HEAD(node)               ((node)->info & 2U)
#define OS_TIMER_GET_HANDLE_FLAG_INDEX(node) ((node)->info & 1U)

/* Invisible to user */
#define OS_TIMER_FLAG_DEACTIVATED 0x0U   /* Timer is deactive. */
#define OS_TIMER_FLAG_ACTIVATED   0x2U   /* Timer is active. */
#define OS_TIMER_FLAG_STATIC      0x0U
#define OS_TIMER_FLAG_DYNAMIC     0x4U
#define OS_TIMER_FLAG_NOT_INITED  0x0U
#define OS_TIMER_FLAG_INITED      0x08U

#define OS_TIMER_INDEX_INVALID    OS_UINT32_MAX
#define TIMER_TAG                 "TIMER"

#define OS_TIMER_TASK_PRIO        0

#if OS_TIMER_LIST_ENTRIES > 32
#error  "OS_TIMER_LIST_ENTRIES can not be greater than 32 !!!"
#endif

struct os_timer_active_list_info
{
    os_timer_active_node_t active_node;
};

typedef struct os_timer_active_list_info os_timer_active_list_info_t;

OS_ALIGN(OS_ALIGN_SIZE)
static os_uint8_t     gs_timer_task_stack[OS_TIMER_TASK_STACK_SIZE];
static os_task_t      gs_os_timer_task;
static os_bool_t      gs_os_timer_need_handle = OS_FALSE;
static os_list_node_t gs_os_timer_list = OS_LIST_INIT(gs_os_timer_list);
static OS_DEFINE_SPINLOCK(gs_os_timer_list_lock);

static os_timer_active_list_info_t *gs_os_timer_list_start;
static os_timer_active_list_info_t *gs_os_timer_list_end;
static os_timer_active_list_info_t *gs_os_timer_list_current;
static os_timer_active_list_info_t  gs_os_timer_active_list_info[OS_TIMER_LIST_ENTRIES];
static os_uint32_t                  gs_os_timer_active_bit_mask;

/* Indicates whether the timer is being handled by the timer task. */
static os_uint8_t                   gs_os_timer_handle_flag[OS_TIMER_LIST_ENTRIES][2];

OS_INLINE void _k_timer_get_active_list(os_timer_active_list_info_t **current,
                                        os_timer_active_list_info_t **start,
                                        os_timer_active_list_info_t **end)
{
    if (current)
    {
        *current = gs_os_timer_list_current;
    }

    if (start)
    {
        *start   = gs_os_timer_list_start;
    }

    if (end)
    {
        *end     = gs_os_timer_list_end;
    }
}

OS_INLINE void _k_timer_del_from_list(os_timer_t *timer)
{
    os_list_del(&timer->list);
}

OS_INLINE void _k_timer_add_to_list(os_timer_t *timer)
{
    os_list_add_tail(&gs_os_timer_list, &timer->list);
}

OS_INLINE void _k_timer_set_handle_flag_index(os_timer_active_node_t *node, os_uint8_t index)
{
    if (0 == index)
    {
        node->info &= (~0x1);
    }
    else
    {
        node->info |= 0x1;
    }
}

OS_INLINE void _k_timer_update_handle_flag(os_uint32_t current_index)
{
    os_uint8_t handle_flag_index;
    os_uint8_t next_handle_flag_index;

    handle_flag_index      = OS_TIMER_GET_HANDLE_FLAG_INDEX(&gs_os_timer_list_start[current_index].active_node);
    next_handle_flag_index = (0 == handle_flag_index) ? 1 : 0;

    gs_os_timer_handle_flag[current_index][handle_flag_index]      = 1;
    gs_os_timer_handle_flag[current_index][next_handle_flag_index] = 0;

    _k_timer_set_handle_flag_index(&gs_os_timer_list_start[current_index].active_node, next_handle_flag_index);
}

#ifdef OS_TIMER_SORT
OS_INLINE void _k_timer_set_head(os_timer_active_node_t *node)
{
    node->info |= 0x2; 
}

OS_INLINE void _k_timer_clear_head(os_timer_active_node_t *node)
{
    node->info &= (~0x2);
}
#endif

static void _k_timer_do_init(os_timer_t *timer,
                             const char *name,
                             void      (*function)(void *parameter),
                             void       *parameter,
                             os_uint32_t timeout,
                             os_uint8_t  flag)
{
    timer->active_node.flag    = (flag & (~OS_TIMER_FLAG_ACTIVATED)) | OS_TIMER_FLAG_INITED;
    timer->timeout_func        = function;
    timer->parameter           = parameter;
    timer->init_ticks          = ((0 == timeout) ? 1 : timeout);
    timer->round_ticks         = timer->init_ticks;
    timer->index               = OS_TIMER_INDEX_INVALID;

    if (OS_NULL != name)
    {
        strncpy(&timer->name[0], name, OS_NAME_MAX);
        timer->name[OS_NAME_MAX] = '\0';
    }
    else
    {
        timer->name[0] = '\0';
    }

    os_list_init(&timer->active_node.active_list);

#ifdef OS_TIMER_SORT
    _k_timer_clear_head(&timer->active_node);
#endif
}

#ifdef OS_TIMER_SORT
OS_INLINE void _k_timer_add_to_sort_list(os_timer_active_node_t *active_list_head,
                                                   os_timer_t             *add_timer)
{
    os_list_node_t *pos;
    os_timer_t     *tmp_timer;

    /*
     * If enable OS_TIMER_SORT,the actual round ticks of the timer is the
     * sum of its round ticks plus all the round ticks of its previous timers
     * which are located in the same active list.
     */

    os_list_for_each(pos, &active_list_head->active_list)
    {
        tmp_timer = os_list_entry(pos, os_timer_t, active_node.active_list);

        if (add_timer->round_ticks >= tmp_timer->round_ticks)
        {
            /* Round ticks of added timer should subtract the round ticks of previous timers. */
            add_timer->round_ticks -= tmp_timer->round_ticks;
        }
        else
        {
            /*
             * Round ticks of next timer which is located after the added timer should 
             * subtract the round ticks of the added timer.
             */
            tmp_timer->round_ticks -= add_timer->round_ticks;
            break;
        }
    }

    /* Add timer before pos. */
    os_list_add_tail(pos, &add_timer->active_node.active_list);
}

OS_INLINE void _k_timer_del_from_sort_list(os_timer_t            *del_timer)
{
    os_timer_t             *next_timer;
    os_timer_active_node_t *next_node;

    next_node = os_list_entry(del_timer->active_node.active_list.next, os_timer_active_node_t, active_list);
    
    if (!OS_TIMER_IS_HEAD(next_node))
    {
        /* Next timer exist. The next timer's round ticks adds round ticks of the deleted timer. */

        next_timer = os_list_entry(del_timer->active_node.active_list.next, os_timer_t, active_node.active_list);

        next_timer->round_ticks += del_timer->round_ticks;
    }

    os_list_del(&del_timer->active_node.active_list);
}

/**
 ***********************************************************************************************************************
 * @brief           This function gets the number of remaining ticks for next active timer.
 *
 * @param           None
 *
 * @return          Number of ticks.
 ***********************************************************************************************************************
 */
os_tick_t k_timer_get_next_remain_ticks(void)
{
    os_timer_active_list_info_t *iter_current;
    os_timer_active_list_info_t *current;
    os_timer_active_list_info_t *start;
    os_timer_active_list_info_t *end;
    os_timer_t                  *timer;
    os_tick_t                    min_timeout_tick = OS_UINT32_MAX;
    os_tick_t                    tmp_timeout_tick;
    os_uint32_t                  i;
 
     /* 
     * If timer list bit mask has been set, prove there is timer need to schedule 
     * and the remain time must be zero.
     */
    if(0 != gs_os_timer_active_bit_mask)
    {
        return 0;
    }

    _k_timer_get_active_list(&current, &start, &end);

    iter_current = current;

    /* 
     * If enable OS_TIMER_SORT, each active list is sorted. So the first timer of 
     * each active list has the least expiration time.In the worst case,it needs to 
     * traverse the first timer of all active lists.
     */
    for (i = 0; i < OS_TIMER_LIST_ENTRIES; i++)
    {
        if (!os_list_empty(&iter_current->active_node.active_list))
        {
            timer = os_list_entry(iter_current->active_node.active_list.next, os_timer_t, active_node.active_list);

            if (0 == timer->round_ticks)
            {
                /*
                 * If the round ticks of the first timer in the active list is 0,
                 * there is no need to continue traversing.Because it's the next timer
                 * that's about to expire.
                 */
                min_timeout_tick = i;
                break;
            }
            else
            {
                /* Calculate the expire time and update the minimal timeout ticks. */

                tmp_timeout_tick =  timer->round_ticks + i;

                if (min_timeout_tick > tmp_timeout_tick)
                {
                    min_timeout_tick = tmp_timeout_tick;               
                }
            }
        }

        /* Increase iter_current. If it goes beyond the end,wrap around. */
        if (++iter_current > end)
        {
            iter_current = start;
        }
    }

    return min_timeout_tick;
}

/**
 ***********************************************************************************************************************
 * @brief           This function update timer active list after low power wakes up.
 *
 * @param[in]       ticks       Number of ticks that need to be updated.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void k_timer_update_active_list(os_tick_t ticks)
{
    os_timer_active_list_info_t *iter_current;
    os_timer_active_list_info_t *current;
    os_timer_active_list_info_t *start;
    os_timer_active_list_info_t *end;
    os_timer_t                  *timer;
    os_uint32_t                  round_ticks;
    os_uint32_t                  i;

    if (ticks > 0)
    {
        _k_timer_get_active_list(&current, &start, &end);

        iter_current = current;
        round_ticks  = OS_TIMER_ROUND(ticks - 1);

        /* 
         * If ticks greater than OS_TIMER_LIST_ENTRIES, it needs to traverse OS_TIMER_LIST_ENTRIES 
         * active lists. Otherwise just traverse ticks active lists.
         */
        for (i = 0; (i < OS_TIMER_LIST_ENTRIES) && (i < ticks); i++)
        {
            if (!os_list_empty(&iter_current->active_node.active_list))
            {
                timer = os_list_entry(iter_current->active_node.active_list.next, os_timer_t, active_node.active_list);

                if (timer->round_ticks >= round_ticks)
                {
                    timer->round_ticks -= round_ticks;
                }

                if ((i <= OS_TIMER_MOD(ticks - 1)) && (timer->round_ticks >= OS_TIMER_LIST_ENTRIES))
                {
                    timer->round_ticks -= OS_TIMER_LIST_ENTRIES;
                }
            }

            /* Increase iter_current. If it goes beyond the end,wrap around. */
            if (++iter_current > end)
            {
                iter_current = start;
            }
        }

        i = OS_TIMER_MOD((current - start) + ticks);

        gs_os_timer_list_current = &start[i];
    }

    return;
}

#endif

static void _k_timer_activate(os_timer_t *timer)
{
    os_timer_active_list_info_t *current;
    os_timer_active_list_info_t *start;
    os_uint32_t                  dec_ticks;
    os_uint32_t                  index;
    os_uint8_t                   handle_flag_index;

    OS_ASSERT(timer->init_ticks > 0);

    if (os_list_empty(&timer->active_node.active_list))
    {
        /* Timer is inactive,activate it. */

        _k_timer_get_active_list(&current, &start, OS_NULL);

        dec_ticks = timer->init_ticks - 1;

        /* 
         * Calculate index of acitve list array to insert.
         * Do not use "index = OS_TIMER_MOD((current - start) + dec_ticks);",
         * because it maybe overflow.
         */
        index              = OS_TIMER_MOD(dec_ticks);
        index              = OS_TIMER_MOD((current - start) + index);
        timer->round_ticks = OS_TIMER_ROUND(dec_ticks);

#ifdef OS_TIMER_SORT
        _k_timer_add_to_sort_list(&start[index].active_node, timer);
#else
        os_list_add_tail(&start[index].active_node.active_list , &timer->active_node.active_list);
#endif

        timer->active_node.flag |= OS_TIMER_FLAG_ACTIVATED;
        timer->index             = index;
        
        handle_flag_index = OS_TIMER_GET_HANDLE_FLAG_INDEX(&start[index].active_node);
        _k_timer_set_handle_flag_index(&timer->active_node, handle_flag_index);
    }
}

static void _k_timer_deactivate(os_timer_t *timer)
{
    if (timer->active_node.flag & OS_TIMER_FLAG_ACTIVATED)
    {
        timer->active_node.flag &= ~OS_TIMER_FLAG_ACTIVATED;
        timer->index             = OS_TIMER_INDEX_INVALID;

        if (!os_list_empty(&timer->active_node.active_list))
        {
#ifdef OS_TIMER_SORT
            _k_timer_del_from_sort_list(timer);
#else
            os_list_del(&timer->active_node.active_list);
#endif
        }
    }
}

static void _k_timer_remove(os_timer_t *timer)
{
    _k_timer_del_from_list(timer);
    _k_timer_deactivate(timer);
    timer->active_node.flag &= ~OS_TIMER_FLAG_INITED;
}

static os_tick_t _k_timer_calc_remain_ticks(os_timer_t *timer)
{
    os_timer_active_list_info_t *current;
    os_timer_active_list_info_t *start;
    os_timer_active_list_info_t *end;
    os_tick_t                    remain_tick;
    os_tick_t                    diff;
    os_uint32_t                  current_index;
    os_uint8_t                   handle_flag_index;
#ifdef OS_TIMER_SORT
    os_timer_t                  *tmp_timer;
    os_list_node_t              *pos;
    os_timer_active_node_t      *active_node;
#endif

    if (timer->active_node.flag & OS_TIMER_FLAG_ACTIVATED)
    {
        OS_ASSERT(OS_TIMER_INDEX_INVALID != timer->index);

        _k_timer_get_active_list(&current, &start, &end);

        /* Calculate the index of list pointed by gs_os_timer_list_current. */
        current_index = current - start;

        if (timer->index >= current_index)
        {
            /* 
             * The index of the active list where the timer is located is greater
             * than index of list pointed by gs_os_timer_list_current.
             */
            diff = timer->index - current_index + 1;
        }
        else
        {
            /* 
             * The index of the active list where the timer is located is less
             * than index of list pointed by gs_os_timer_list_current. The pointer
             * needs to wrap around to reach the list where the timer is located. 
             */
            diff = end - current + 1;
            diff += timer->index + 1;
        }

        remain_tick = 0;

#ifdef OS_TIMER_SORT
        /*
         * If OS_TIMER_SORT enable,the actual round ticks of the timer is the
         * sum of its round ticks plus all the round ticks of its previous timers
         * which are located in the same active list. 
         */
        for (pos = &timer->active_node.active_list; ;pos = pos->prev)
        {
            active_node = os_list_entry(pos, os_timer_active_node_t, active_list);
            
            if (OS_TIMER_IS_HEAD(active_node))
            {
                break;
            }

            tmp_timer   = os_list_entry(pos, os_timer_t, active_node.active_list);
            remain_tick += tmp_timer->round_ticks;
        }

        remain_tick += diff;
#else
        remain_tick = timer->round_ticks + diff;
#endif

        handle_flag_index = OS_TIMER_GET_HANDLE_FLAG_INDEX(&timer->active_node);
        if (1 == gs_os_timer_handle_flag[timer->index][handle_flag_index])
        {
            /* Timer is being handled by timer task. */
            if (remain_tick <= OS_TIMER_LIST_ENTRIES)
            {
                 remain_tick = 0;
            }
            else
            {
                 remain_tick -= OS_TIMER_LIST_ENTRIES;
            }
        }
    }
    else
    {
        remain_tick = 0;
    }

    return remain_tick;
}

static void _k_timer_task_entry(void *parameter)
{
    os_timer_active_node_t       expired_timer_head;
    os_timer_active_node_t       restart_timer_head;
    os_list_node_t              *current_pos;
    os_list_node_t              *restart_pos;
    os_timer_t                  *restart_timer;
    os_timer_t                  *current_timer;
    os_uint8_t                   handle_flag_index;
    void                       (*timeout_func)(void *timeout_param);
    void                        *timeout_param;
    os_task_t                   *current_task;
    os_uint16_t                  current_list_idx;
    os_timer_active_list_info_t *iter_current;

    OS_KERNEL_INIT();

    OS_UNREFERENCE(parameter);

#ifdef OS_TIMER_SORT
    _k_timer_set_head(&expired_timer_head);
    _k_timer_set_head(&restart_timer_head);
#endif

    while (OS_TRUE)
    {
        OS_KERNEL_ENTER();
    
        if (!gs_os_timer_need_handle)
        {
            /* No timer needs handle,suspend myself. */

            current_task = k_task_self();
            k_readyq_remove(current_task);
            current_task->state &= ~OS_TASK_STATE_READY;
            current_task->state |= OS_TASK_STATE_SUSPEND;
    
            OS_KERNEL_EXIT_SCHED();
        }
        else
        {
            /* 
             * There are timers need handle.
             * 1.Move the current active list to expire list.
             * 2.Handle timers of expire list one by one.
             * 3.If the timer does not expire or timer is periodic,add the timer to restart list.
             * 4.Call timeout function of expired timer.
             * 5.Add the timer of restart list to active list.
             */

            /* Move the current active list pointed by gs_os_timer_list_current to expire list.
             *   -----      -----   add   -----       -----      -----   del   -----     -----      -----
             *  | cur |<-->| node|  ----> | cur |<-->| exp |<-->| node| ----> | cur |   | exp |<-->| node|
             *  | head|<-->|     |        | head|<-->| head|<-->|     |       | head|   | head|<-->|     |
             *   -----      -----          -----      -----      -----         -----     -----      -----
             */
            
            /* 
            * Clear flag. The timer tick handler will set the flag when
            * it find list pointed by gs_os_timer_list_current is not empty.
            */
            gs_os_timer_need_handle = OS_FALSE;

            while (0 != gs_os_timer_active_bit_mask)
            {
                /* Get the timer's current list idx,which is needed to handle. */
                current_list_idx = os_ffs(gs_os_timer_active_bit_mask) - 1;
                iter_current = &gs_os_timer_active_list_info[current_list_idx];
                /* Move the current active list pointed to expire list. */
                os_list_add(&iter_current->active_node.active_list, &expired_timer_head.active_list);
                os_list_del(&iter_current->active_node.active_list);

                _k_timer_update_handle_flag((os_uint32_t)current_list_idx);

                /* Clear timer's list bit mask. */
                OS_CLEAR_BIT(gs_os_timer_active_bit_mask, current_list_idx);

                /* Handle timers of expire list one by one. */
                while (OS_NULL != (current_pos = os_list_first(&expired_timer_head.active_list)))
                {
                    current_timer = os_list_entry(current_pos, os_timer_t, active_node.active_list);

                    /* Delete current timer from expire list. */
#ifdef OS_TIMER_SORT
                    _k_timer_del_from_sort_list(current_timer);
#else
                    os_list_del(current_pos);
#endif

                    os_list_init(&restart_timer_head.active_list);

                    OS_ASSERT(0 == OS_TIMER_MOD(current_timer->round_ticks));

                    /* 
                    * Get timeout funciton and parameter. If the timer does not expire or timer is periodic,
                    * add the timer to restart list in order to activate it again after call timeout funcion.
                    */
                    if (0 == current_timer->round_ticks)
                    {
                        /* Timer expire. */

                        if (current_timer->active_node.flag & OS_TIMER_FLAG_PERIODIC)
                        {
                            /* Add current timer to restart timer list in order to activate it again afterwards. */
                            os_list_add(&restart_timer_head.active_list, current_pos);
                        }

                        /* Get timeout function and parameter. The timeout funciton will be called afterwards. */
                        timeout_func = current_timer->timeout_func;
                        timeout_param = current_timer->parameter;
                    }
                    else
                    {
                        /* Timer do not expire */ 

                        /* Add current timer to restart timer list. */
                        os_list_add(&restart_timer_head.active_list, current_pos);

                        timeout_func = OS_NULL;
                    }

                    /* The timer is oneshot, deactivate it. */
                    if (os_list_empty(&restart_timer_head.active_list))
                    {
                        _k_timer_deactivate(current_timer);
                    }

                    OS_KERNEL_EXIT();

                    /* Call timeout function if it is expired. */
                    if (OS_NULL != timeout_func)
                    {
                        timeout_func(timeout_param);
                    }

                    OS_KERNEL_ENTER();

                    /* If the timer does not expire or timer is periodic, add timer to active list. */
                    if (OS_NULL != (restart_pos = os_list_first(&restart_timer_head.active_list)))
                    {
                        /* Remove timer from restart list. */
                        os_list_del(restart_pos);

                        restart_timer = os_list_entry(restart_pos, os_timer_t, active_node.active_list);

                        if (0 == restart_timer->round_ticks)
                        {
                            /* Timer expire, reactivate timer. */
                            _k_timer_activate(restart_timer);
                        }
                        else
                        {
                            /* Timer does not expire, round_ticks subtract active list entries. */
                            restart_timer->round_ticks -= OS_TIMER_LIST_ENTRIES;
                        
                            /* 
                            * There is no need to recalculate active list's index where the 
                            * unexpired timer adds. Just add timer to its original active list. 
                            */
#ifdef OS_TIMER_SORT
                            _k_timer_add_to_sort_list(&gs_os_timer_list_start[restart_timer->index].active_node,
                                                        restart_timer);
#else
                            os_list_add_tail(&(gs_os_timer_list_start[restart_timer->index].active_node.active_list),
                                            restart_pos);
#endif
                        
                            handle_flag_index = OS_TIMER_GET_HANDLE_FLAG_INDEX(&gs_os_timer_list_start[restart_timer->index].active_node);
                            _k_timer_set_handle_flag_index(&restart_timer->active_node, handle_flag_index);
                        }
                    }
                }
            }
            OS_KERNEL_EXIT();
        }
    }
}

void k_move_timer_list_one_step(void)
{
    if (++gs_os_timer_list_current > gs_os_timer_list_end)
    {
        gs_os_timer_list_current = gs_os_timer_list_start;
    }
}

os_bool_t k_timer_need_handle(void)
{
    os_bool_t need_handle;

    /* 
     * If the list pointed by gs_os_timer_list_current is not expty,set flag gs_os_timer_need_handle 
     * and resume timer task to handle the timer list pointed by gs_os_timer_list_current.
     */
    if (!os_list_empty(&gs_os_timer_list_current->active_node.active_list))
    {
#ifdef OS_TIMER_SORT
        os_timer_t *timer;

        timer = os_list_entry(gs_os_timer_list_current->active_node.active_list.next, os_timer_t, active_node.active_list);
        if (timer->round_ticks >= OS_TIMER_LIST_ENTRIES)
        {
            /* There is no timer timeout, do not need wake up timer task. */
            timer->round_ticks -= OS_TIMER_LIST_ENTRIES;
        }
        else
#endif
        {
            /* List is not empty,set flag and set the list bit mask.*/
            gs_os_timer_need_handle = OS_TRUE;
            OS_SET_BIT(gs_os_timer_active_bit_mask, (gs_os_timer_list_current - gs_os_timer_list_start));
        }
    }

    need_handle = OS_FALSE;

    /* List pointed by gs_os_timer_list_current is not empty,resume timer task to handle the list. */
    if (gs_os_timer_need_handle && (OS_TASK_STATE_SUSPEND == gs_os_timer_task.state))
    {
        gs_os_timer_task.state &= ~OS_TASK_STATE_SUSPEND;
        gs_os_timer_task.state |= OS_TASK_STATE_READY;
        k_readyq_put(&gs_os_timer_task);
        need_handle = OS_TRUE;
    }

    return need_handle;
}

void k_timer_module_init(void)
{
    os_int32_t i;
    os_err_t ret;

    for (i = 0; i < OS_TIMER_LIST_ENTRIES; i++)
    {
        os_list_init(&gs_os_timer_active_list_info[i].active_node.active_list);

        _k_timer_set_handle_flag_index(&gs_os_timer_active_list_info[i].active_node, 0);

#ifdef OS_TIMER_SORT
        _k_timer_set_head(&gs_os_timer_active_list_info[i].active_node);
#endif
    }

    gs_os_timer_list_start   = &gs_os_timer_active_list_info[0];
    gs_os_timer_list_current = &gs_os_timer_active_list_info[0];
    gs_os_timer_list_end     = &gs_os_timer_active_list_info[OS_TIMER_LIST_ENTRIES-1];

    memset(gs_os_timer_handle_flag, 0, sizeof(gs_os_timer_handle_flag));

    /* Create and start timer task. */
    ret = os_task_init(&gs_os_timer_task,
                       "timer", 
                       _k_timer_task_entry, 
                       OS_NULL,
                       &gs_timer_task_stack[0],
                       sizeof(gs_timer_task_stack),
                       OS_TIMER_TASK_PRIO);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why initialize timer task failed?");
    }

    ret = os_task_startup(&gs_os_timer_task);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why startup timer task failed?");
    }

    return;
}

os_err_t k_list_timer(os_int32_t argc, char **argv)
{
    os_timer_t     *timer;
    os_list_node_t *next;
    os_list_node_t *pos;
    os_tick_t       remain_ticks;
    const char     *item_title = "timer";

    OS_KERNEL_INIT();

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s interval     remain                flag\r\n", OS_NAME_MAX, item_title);
    os_kprintf("------------ ------------ ------------ -----------------------------\r\n");

    OS_KERNEL_ENTER();

    os_list_for_each_safe(pos, next, &gs_os_timer_list)
    {
        timer = os_list_entry(pos, os_timer_t, list);

        /* Calculate remaining ticks. */
        remain_ticks = _k_timer_calc_remain_ticks(timer);

        os_kprintf("%-*.*s %-12lu %-12lu ",
                   OS_NAME_MAX,
                   OS_NAME_MAX,
                   timer->name,
                   timer->init_ticks,
                   remain_ticks);

        if (timer->active_node.flag & OS_TIMER_FLAG_ACTIVATED)
        {
            os_kprintf("%-*s", strlen("inactive,"), "active,");
        }
        else
        {
            os_kprintf("%-*s", strlen("inactive,"), "inactive,");
        }

        if (timer->active_node.flag & OS_TIMER_FLAG_PERIODIC)
        {
            os_kprintf("%-*s", strlen("periodic,"), "periodic,");
        }
        else
        {
            os_kprintf("%-*s", strlen("periodic,"), "oneshot,");
        }

        if (timer->active_node.flag & OS_TIMER_FLAG_DYNAMIC)
        {
            os_kprintf("%-*s\r\n", strlen("dynamic"), "dynamic");
        }
        else
        {
            os_kprintf("%-*s\r\n", strlen("dynamic"), "static");
        }
    }

    os_kprintf("current tick:0x%08lx \r\n", os_tick_get());

    OS_KERNEL_EXIT();

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function initialize a timer and places it on the list of timer object.
 *
 * @param[in]       timer           The descriptor of timer control block.
 * @param[in]       name            Pointer to timer name string.
 * @param[in]       timeout         Entry function of timeout.
 * @param[in]       parameter       Parameter of entry function.
 * @param[in]       timeout         Number of ticks that need to be delayed.
 * @param[in]       flag            Flag of timer.
 *
 * @return          None
 ***********************************************************************************************************************
 */
os_err_t os_timer_init(os_timer_t *timer,
                   const char *name,
                   void      (*function)(void *parameter),
                   void       *parameter,
                   os_tick_t  timeout,
                   os_uint8_t flag)
{
    os_err_t        ret; 
    os_timer_t       *iter_timer;

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(OS_NULL != function);

    ret = OS_EOK;

    os_spin_lock(&gs_os_timer_list_lock);
    os_list_for_each_entry(iter_timer,&gs_os_timer_list, os_timer_t, list)
    {
        if (iter_timer == timer)
        {
            OS_KERN_LOG(KERN_ERROR, TIMER_TAG, "The timer(addr: %p, name: %s) has been exist", iter_timer, iter_timer->name);
            ret = OS_EINVAL;
            break;
        }
    }
    if (OS_EOK == ret)
    {
        os_spin_unlock(&gs_os_timer_list_lock);

        _k_timer_do_init(timer, name, function, parameter, timeout, flag & (~OS_TIMER_FLAG_DYNAMIC));

        os_spin_lock(&gs_os_timer_list_lock);
        _k_timer_add_to_list(timer);
    }
    os_spin_unlock(&gs_os_timer_list_lock);

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will deinitializate the specified timer by timer descriptor.
 *
 * @attention       It corresponds to os_timer_init.
 *
 * @param[in]       timer            The descriptor of timer control block
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
void os_timer_deinit(os_timer_t *timer)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 == (timer->active_node.flag & OS_TIMER_FLAG_DYNAMIC));
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    _k_timer_remove(timer);
    OS_KERNEL_EXIT();
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function create a timer and places it on the list of timer object.
 *
 * @details         This function create a timer and places it on the list of timer object.The memory space of the 
 *                  timer control block descriptor is applied from the heap.
 *
 * @param[in]       name            Pointer to timer name string.
 * @param[in]       timeout         Entry function of timeout.
 * @param[in]       parameter       Parameter of entry function.
 * @param[in]       timeout         Number of ticks that need to be delayed.
 * @param[in]       flag            Flag of timer.
 * 
 * @return          On success, return a timer control block descriptor; on error, OS_NULL is returned.
 * @retval          not OS_NULL     Return a timer control block descriptor.
 * @retval          OS_NULL         Call os_timer_create failed.
 ***********************************************************************************************************************
 */
os_timer_t *os_timer_create(const char *name,
                            void      (*function)(void *parameter),
                            void       *parameter,
                            os_tick_t   timeout,
                            os_uint8_t  flag)
{
    os_timer_t *timer;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    timer = (os_timer_t *)OS_KERNEL_MALLOC(sizeof(os_timer_t));
    if (OS_NULL == timer)
    {
        OS_KERN_LOG(KERN_ERROR, TIMER_TAG, "malloc timer fail");
    }
    else
    {
        _k_timer_do_init(timer, name, function, parameter, timeout, flag | OS_TIMER_FLAG_DYNAMIC);

        OS_KERNEL_ENTER();
        _k_timer_add_to_list(timer);
        OS_KERNEL_EXIT();
    }

    return timer;
}

/**
***********************************************************************************************************************
* @brief          This function will destroy the specified timer by timer descriptor.
*
* @attention      It corresponds to os_timer_create.
*
* @param[in]      timer           The descriptor of timer control block
*
* @return         Will only return OS_EOK
***********************************************************************************************************************
*/
os_err_t os_timer_destroy(os_timer_t *timer)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_DYNAMIC));
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    OS_KERNEL_ENTER();
    _k_timer_remove(timer);
    OS_KERNEL_EXIT();

    OS_KERNEL_FREE(timer);

    return OS_EOK;
}
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function will starup the specified timer by timer descriptor.
 *
 * @param[in]       timer           The descriptor of timer control block
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t os_timer_start(os_timer_t *timer)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();

    /* First deactivate timer, then start it. */
    _k_timer_deactivate(timer);
    _k_timer_activate(timer);

    OS_KERNEL_EXIT();

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will stop the specified timer by timer descriptor.
 *
 * @param[in]       timer           The descriptor of timer control block
 *
 * @return          Stop success or fail.
 * @retval          OS_EOK          success.
 * @retval          OS_ERROR        fail.
 ***********************************************************************************************************************
 */
os_err_t os_timer_stop(os_timer_t *timer)
{
    os_err_t ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    if (!(timer->active_node.flag & OS_TIMER_FLAG_ACTIVATED))
    {
        ret = OS_ERROR;
    }
    else
    {
        OS_KERNEL_ENTER();
        _k_timer_deactivate(timer);
        OS_KERNEL_EXIT();

        ret = OS_EOK;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function set delay time of timer.
 *
 * @param           timer           The descriptor of timer control block.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t os_timer_set_timeout_ticks(os_timer_t *timer, os_tick_t timeout)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    timer->init_ticks = timeout;
    OS_KERNEL_EXIT();

    return OS_EOK;
}


/**
 ***********************************************************************************************************************
 * @brief           This function gets the initial ticks that timer needs to delay.
 *
 * @param           timer           The descriptor of timer control block.
 *
 * @return          Number of ticks.
 ***********************************************************************************************************************
 */
os_tick_t os_timer_get_timeout_ticks(os_timer_t *timer)
{
    os_tick_t init_ticks;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    init_ticks = timer->init_ticks;
    OS_KERNEL_EXIT();

    return init_ticks;
}

/**
 ***********************************************************************************************************************
 * @brief           This function gets the remaining ticks before expiration.
 *
 * @param           timer           The descriptor of timer control block.
 *
 * @return          Number of ticks.
 ***********************************************************************************************************************
 */
os_tick_t os_timer_get_remain_ticks(os_timer_t *timer)
{
    os_tick_t remain_ticks;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    remain_ticks = _k_timer_calc_remain_ticks(timer);
    OS_KERNEL_EXIT();

    return remain_ticks;
}

/**
 ***********************************************************************************************************************
 * @brief           This function return the timer is active or inactive.
 *
 * @param           timer           The descriptor of timer control block.
 *
 * @return          Timer is active or inactive.
 * @retval          OS_FASLE        Timer is inactive.
 * @retval          OS_TRUE         Timer is active.
 ***********************************************************************************************************************
 */
os_bool_t os_timer_is_active(os_timer_t *timer)
{
    os_bool_t is_active;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    is_active = (timer->active_node.flag & OS_TIMER_FLAG_ACTIVATED) ? OS_TRUE : OS_FALSE;
    OS_KERNEL_EXIT();

    return is_active;
}

/**
 ***********************************************************************************************************************
 * @brief           This function set the timer to be oneshot.
 *
 * @param           timer           The descriptor of timer control block.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t os_timer_set_oneshot(os_timer_t *timer)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    timer->active_node.flag &= ~OS_TIMER_FLAG_PERIODIC;
    OS_KERNEL_EXIT();

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function set the timer to be periodic.
 *
 * @param           timer           The descriptor of timer control block.
 *
 * @return          Will only return OS_EOK
 ***********************************************************************************************************************
 */
os_err_t os_timer_set_periodic(os_timer_t *timer)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    timer->active_node.flag |= OS_TIMER_FLAG_PERIODIC;
    OS_KERNEL_EXIT();

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function return the timer is one shot or periodic.
 *
 * @param           timer           The descriptor of timer control block.
 *
 * @return          Timer is one shot or periodic.
 * @retval          OS_FASLE        Timer is one shot.
 * @retval          OS_TRUE         Timer is periodic.
 ***********************************************************************************************************************
 */
os_bool_t os_timer_is_periodic(os_timer_t *timer)
{
    os_bool_t is_periodic;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != timer);
    OS_ASSERT(0 != (timer->active_node.flag & OS_TIMER_FLAG_INITED));

    OS_KERNEL_ENTER();
    is_periodic = (timer->active_node.flag & OS_TIMER_FLAG_PERIODIC) ? OS_TRUE : OS_FALSE;
    OS_KERNEL_EXIT();

    return is_periodic;
}

#endif /* End of OS_USING_TIMER */

