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
 * @file        os_ipc.c
 *
 * @brief       This file implements the IPC public functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-11   OneOS team      First Version
 ***********************************************************************************************************************
 */
 
#include <os_errno.h>
#include <arch_interrupt.h>
#include <os_list.h>
#include <os_task.h>
#include <string.h>

#include "os_kernel_internal.h"

/**
 ***********************************************************************************************************************
 * @brief           This function will insert the task to IPC queue. The higher priority task will be inserted before the
 *                  other task.
 *
 * @param[in]       head             The IPC queue.
 * @param[in]       task             The task to be inserted
 *
 * @return          None
 ***********************************************************************************************************************
 */
void k_blockq_insert(os_list_node_t *head, struct os_task *task)
{
    os_list_node_t *pos;
    os_task_t      *task_iter;

    os_list_for_each(pos, head)
    {
        task_iter = os_list_entry(pos, os_task_t, task_node);
        if (task_iter->current_priority > task->current_priority)
        {
            /* Insert this task_node before the task_iter. */
            os_list_add_tail(&task_iter->task_node, &task->task_node);
            break;
        }
    }

    if (pos == head)
    {
        os_list_add_tail(head, &task->task_node);
    }
    
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will block the specific task. and if the task not wait forever it will be insert in tick queue.
 *
 * @param[in]       head            The IPC queue.
 * @param[in]       task            The task wait to block.
 * @param[in]       timeout         Wait time (in clock ticks).
 *
 * @return          None
 ***********************************************************************************************************************
 */
void k_block_task(os_list_node_t *head, os_task_t *task, os_tick_t timeout, os_bool_t is_wake_prio)
{
    k_readyq_remove(task);
    task->state &= ~OS_TASK_STATE_READY;
    
    task->state |= OS_TASK_STATE_BLOCK;
    
    if (OS_TRUE == is_wake_prio)
    {
        k_blockq_insert(head, task);
    }
    else
    {
        os_list_add_tail(head, &task->task_node);
    }
    
    if (OS_WAIT_FOREVER != timeout)
    {
        task->state |= OS_TASK_STATE_SLEEP;
        k_tickq_put(task, timeout);
    }

    task->block_list_head = head;
    task->is_wake_prio    = is_wake_prio;
    task->switch_retval   = OS_EOK;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will unblock the highest priority task. and it will be removed from tick queue if the 
 *                  task wait in the queue.
 *
 * @param[in]       task            The task to unblock.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void k_unblock_task(os_task_t *task)
{
    os_list_del(&task->task_node);
    task->state &= ~OS_TASK_STATE_BLOCK;
    
    if (task->state & OS_TASK_STATE_SLEEP)
    {
        k_tickq_remove(task);
        task->state &= ~OS_TASK_STATE_SLEEP;
    }

    /* The task state may be suspend, empty or running */
    if (OS_TASK_STATE_SUSPEND != task->state)
    {
        task->state |= OS_TASK_STATE_READY;
        k_readyq_put(task);
    }

    task->block_list_head = OS_NULL;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will activate all the task wait in the IPC queue.
 *
 * @param[in]       head            The IPC queue.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void k_cancle_all_blocked_task(os_list_node_t *head)
{
    os_task_t *task;

    while (!os_list_empty(head))
    {
        task = os_list_first_entry(head, os_task_t, task_node);
        k_unblock_task(task);
        
        task->switch_retval = OS_ERROR;
    }

    return;
}

#ifdef OS_USING_SHELL

void k_show_blocked_task(os_task_t **block_task_buff, os_uint16_t buff_cnt_max, os_uint16_t block_task_cnt)
{
    os_uint16_t task_index;
    char        name[OS_NAME_MAX + 1];

    for (task_index = 0U; task_index < buff_cnt_max; task_index++)
    {
        if (OS_NULL == block_task_buff[task_index])
        {
            break;
        }
    
        strncpy(name, block_task_buff[task_index]->name, OS_NAME_MAX);
        name[OS_NAME_MAX] = '\0';
    
        os_kprintf("%s(%d)",
                   (name[0] != '\0') ? name : "-",
                   block_task_buff[task_index]->current_priority);

        if ((task_index + 1U < buff_cnt_max) && (OS_NULL != block_task_buff[task_index + 1U]))
        {
            os_kprintf(" / ");
        }
    }

    if (block_task_cnt > buff_cnt_max)
    {
        os_kprintf(" / ...");
    }

    return;
}

void k_get_blocked_task(os_list_node_t *list_head, os_task_t **block_task_buff, os_uint16_t buff_cnt_max)
{
    os_task_t   *inter_task;
    os_uint16_t  task_index;

    task_index = 0;
    os_list_for_each_entry(inter_task, list_head, os_task_t, task_node)
    {
        if (task_index >= buff_cnt_max)
        {
            break;
        }
    
        block_task_buff[task_index] = inter_task;
        task_index++;
    }

    return;
}

#endif

