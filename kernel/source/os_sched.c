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
 * @file        os_readyq.c
 *
 * @brief       This file implements the sched functions.
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
#include <os_task.h>
#include <arch_interrupt.h>
#include <string.h>
#include <arch_misc.h>

#include "os_kernel_internal.h"

struct readyq
{
#if OS_TASK_PRIORITY_MAX > 32

#if OS_TASK_PRIORITY_MAX > 256
#error  "OS_TASK_PRIORITY_MAX can not be greater than 256 !!!"
#endif

    /* Maximum priority level, 256 */
    os_uint32_t    priority_group_bmap;	                            /* Lookup table for priority_bmap */
    os_uint8_t     priority_bmap[(OS_TASK_PRIORITY_MAX + 7) / 8];   /* Lookup table for priority_list_array */
#else
    os_uint32_t    priority_bmap;	                                /* Lookup table for priority_list_array */
#endif

    os_list_node_t priority_list_array[OS_TASK_PRIORITY_MAX];	    /* Doubly linked list head */
};

os_task_t    *g_os_current_task = OS_NULL;
os_task_t    *g_os_next_task = OS_NULL;
os_task_t    *g_os_high_task = OS_NULL;                        /*highest priority task */

/* 
 * Define the global lock scheduler variable. If this is non-zero, scheduler is
 * locked. It is used to prevent task schedule.
 */
os_int16_t    g_os_sched_lock_cnt = 0;
static struct readyq gs_os_readyq;  /*Global priority ready queue*/

static void _k_readq_bmap_init(void)
{
    os_uint32_t i;

#if OS_TASK_PRIORITY_MAX > 32
    gs_os_readyq.priority_group_bmap = 0;
    memset(&gs_os_readyq.priority_bmap[0], 0, sizeof(gs_os_readyq.priority_bmap));
#else
    gs_os_readyq.priority_bmap = 0;	/* lookup table for priority_list_array */
#endif

    for (i = 0; i < OS_TASK_PRIORITY_MAX; i++)
    {
        os_list_init(&gs_os_readyq.priority_list_array[i]);
    }
    
    return;
}

OS_INLINE void _k_readq_bmap_set(os_uint8_t current_priority)
{
#if OS_TASK_PRIORITY_MAX > 32
    gs_os_readyq.priority_group_bmap |= (1 << ((os_uint32_t)current_priority >> 3));
    gs_os_readyq.priority_bmap[current_priority >> 3] |= (1 << (current_priority & 0x7));
#else
    gs_os_readyq.priority_bmap |= 1 << current_priority;

#endif

    return;
}

OS_INLINE void _k_readq_bmap_clear(os_uint8_t current_priority)
{
#if OS_TASK_PRIORITY_MAX > 32
    gs_os_readyq.priority_bmap[current_priority >> 3] &= ~(1 << (current_priority & 0x7));

    if (gs_os_readyq.priority_bmap [current_priority >> 3] == 0)
    {
        gs_os_readyq.priority_group_bmap &= ~(1 << ((os_uint32_t)current_priority >> 3));
    }
#else
    gs_os_readyq.priority_bmap &= ~(1 << current_priority);
#endif

    return;
}

OS_INLINE os_uint8_t _k_readyq_highest(void)
{
    os_uint8_t highest_priority;
#if OS_TASK_PRIORITY_MAX > 32  
    os_ubase_t priority_offset;
#endif

#if OS_TASK_PRIORITY_MAX > 32  
    priority_offset  = os_ffs(gs_os_readyq.priority_group_bmap) - 1;
    highest_priority = (priority_offset << 3) + os_ffs(gs_os_readyq.priority_bmap[priority_offset]) - 1;
#else
    highest_priority = os_ffs(gs_os_readyq.priority_bmap) - 1;
#endif
    
    return highest_priority;
}

/**
 ***********************************************************************************************************************
 * @brief           This function put a task on the ready queue.
 *
 * @details         The highest priority task is calculated when the priority ready queue is inserted.
 *                  
 * @param[in]       task    Task control block.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_readyq_put(struct os_task *task)
{
    os_uint8_t priority;

    priority = task->current_priority;

    if ((g_os_high_task == OS_NULL)
        || (priority < g_os_high_task->current_priority))
    {
    	g_os_high_task = task;
    }

    _k_readq_bmap_set(priority);

    os_list_add_tail(&gs_os_readyq.priority_list_array[priority], &task->task_node);

    return;	
}

/**
 ***********************************************************************************************************************
 * @brief           This function remove a task from the ready queue.
 *
 * @details         The highest priority task is calculated when remove a task from the ready queue.
 *                  
 * @param[in]       task    Task control block.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_readyq_remove(struct os_task *task)
{
    os_list_node_t *task_list_head;
    os_uint8_t      priority;
    os_uint8_t      highest_priority;
    
    priority       = task->current_priority;
    task_list_head = &gs_os_readyq.priority_list_array[priority];

    os_list_del(&task->task_node);

    if (os_list_empty(task_list_head))
    {
        /*clear mapbit*/
        _k_readq_bmap_clear(priority);

        if (task == g_os_high_task)
        {
            highest_priority = _k_readyq_highest();
            g_os_high_task  = os_list_entry(gs_os_readyq.priority_list_array[highest_priority].next, os_task_t, task_node);
        }
    }
    else
    {
        if (task == g_os_high_task)
        {
            highest_priority = task->current_priority;
            g_os_high_task  = os_list_entry(gs_os_readyq.priority_list_array[highest_priority].next, os_task_t, task_node);
        }

    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function move a task to the end of the ready queue.
 *
 * @details         The highest priority task is recalculated.
 *                  
 * @param[in]       task    Task control block.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_readyq_move_tail(struct os_task *task)
{
    os_uint8_t      priority;
    os_list_node_t *task_list_head;
     
    priority       = task->current_priority;
    task_list_head = &gs_os_readyq.priority_list_array[priority];

    os_list_move_tail(task_list_head, &task->task_node);

    if (task == g_os_high_task)
    {
        g_os_high_task = os_list_entry(gs_os_readyq.priority_list_array[priority].next, os_task_t, task_node);
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function initialize kernel scheduling .
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_sched_init(void)
{
    _k_readq_bmap_init();

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function start kernel scheduling .
 *
 * @details         Run the first task (highest priority)
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void k_start(void)
{
    g_os_next_task = g_os_high_task;
    
    os_first_task_start();

    /* Never come back. */
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function lock task scheduling.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void os_schedule_lock(void)
{
    OS_KERNEL_INIT();

    OS_KERNEL_ENTER();
    g_os_sched_lock_cnt++;
    OS_KERNEL_EXIT();
    
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function unlock task scheduling.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
void os_schedule_unlock(void)
{
    OS_KERNEL_INIT();

    OS_KERNEL_ENTER();
    g_os_sched_lock_cnt--;
    OS_KERNEL_EXIT();
    
    OS_ASSERT_EX(g_os_sched_lock_cnt >= 0, "Task(%s) use schedule lock incorrectly.", g_os_current_task->name);

    if (0 == g_os_sched_lock_cnt)
    {
        OS_KERNEL_ENTER();
        OS_KERNEL_EXIT_SCHED();
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function unlock task scheduling.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */ 
os_bool_t os_is_schedule_locked(void)
{
    os_bool_t is_lock;

    OS_KERNEL_INIT();

    is_lock = OS_FALSE;

    OS_KERNEL_ENTER();
   
    if (g_os_sched_lock_cnt > 0)
    {
        is_lock = OS_TRUE;
    }
    else if (g_os_sched_lock_cnt == 0)
    {
        ;
    }
    else
    {
        OS_ASSERT_EX(g_os_sched_lock_cnt >= 0, "Task(%s) use schedule lock incorrectly.", g_os_current_task->name);
    }

    OS_KERNEL_EXIT();
    
    return is_lock;
}

