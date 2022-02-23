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
 * @file        os_mutex.c
 *
 * @brief       This file implements the mutex functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-24   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <os_errno.h>
#include <os_mutex.h>
#include <arch_interrupt.h>
#include <string.h>
#include <os_spinlock.h>

#include "os_kernel_internal.h"

#ifdef OS_USING_MUTEX

#define MUTEX_TAG   "MUTEX"

static os_list_node_t gs_os_mutex_resource_list_head = OS_LIST_INIT(gs_os_mutex_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_mutex_resource_list_lock);


OS_INLINE os_bool_t _k_mutex_set_owner_priority(os_mutex_t *mutex, os_uint8_t new_priority)
{
    os_bool_t need_schedule;

    need_schedule = OS_FALSE;
    
    if (mutex->owner->state & OS_TASK_STATE_READY)
    {
        k_readyq_remove(mutex->owner);
        mutex->owner->current_priority = new_priority;
        k_readyq_put(mutex->owner);

        need_schedule = OS_TRUE;
    }
    else
    {
        mutex->owner->current_priority = new_priority;
    }

    return need_schedule;
}

OS_INLINE os_bool_t _k_mutex_restore_priority(os_mutex_t *mutex)
{
    os_task_t  *owner_task;
    os_uint8_t  new_priority;
    os_bool_t   need_schedule;
    os_task_t  *block_task;
    os_mutex_t *iter_mutex;

    owner_task    = mutex->owner;
    need_schedule = OS_FALSE;

    if (owner_task->current_priority != mutex->original_priority)
    {
        new_priority = mutex->original_priority;

        os_list_for_each_entry(iter_mutex, &owner_task->hold_mutex_list_head, os_mutex_t, hold_node)
        {
            if (!os_list_empty(&iter_mutex->task_list_head))
            {
                block_task = os_list_first_entry(&iter_mutex->task_list_head, os_task_t, task_node);
                if (block_task->current_priority < new_priority)
                {
                    new_priority = block_task->current_priority;
                }
            }
        }

        if (new_priority != owner_task->current_priority)
        {
            need_schedule = _k_mutex_set_owner_priority(mutex, new_priority);
        }
    }

    return need_schedule;
}

OS_INLINE void _k_mutex_init(os_mutex_t *mutex, const char *name, os_bool_t recursive, os_uint16_t object_alloc_type)
{
    os_list_init(&mutex->task_list_head);
    
    mutex->owner        = OS_NULL;
    mutex->lock_count   = 0U;
    mutex->is_recursive = recursive;

    mutex->object_alloc_type = object_alloc_type;
    mutex->wake_type         = OS_MUTEX_WAKE_TYPE_PRIO;

    if (OS_NULL != name)
    {
        (void)strncpy(&mutex->name[0], name, OS_NAME_MAX);
        mutex->name[OS_NAME_MAX] = '\0';
    }
    else
    {
        mutex->name[0] = '\0';
    }
    
    mutex->object_inited = OS_KOBJ_INITED;
}

OS_INLINE void _k_mutex_deinit(os_mutex_t *mutex)
{
    OS_KERNEL_INIT();

    OS_KERNEL_ENTER();
    mutex->object_inited = OS_KOBJ_DEINITED;

    if (OS_NULL == mutex->owner)
    {
        OS_KERNEL_EXIT();
    }
    else
    {
        if (mutex->owner->current_priority != mutex->owner->backup_priority)
        {
            if (mutex->owner->state & OS_TASK_STATE_READY)
            {
                k_readyq_remove(mutex->owner);
                mutex->owner->current_priority = mutex->owner->backup_priority;
                k_readyq_put(mutex->owner);
            }
            else
            {
                mutex->owner->current_priority = mutex->owner->backup_priority;
            }
        }
        
        os_list_del(&mutex->hold_node);

        /* Wakeup all suspend tasks */
        k_cancle_all_blocked_task(&mutex->task_list_head);

        OS_KERNEL_EXIT_SCHED();
    }

    os_spin_lock(&gs_os_mutex_resource_list_lock);
    os_list_del(&mutex->resource_node);
    os_spin_unlock(&gs_os_mutex_resource_list_lock);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initlialize a mutex object .
 *
 * @param[in]       mutex           The mutex to initialize.
 * @param[in]       name            The name of mutex.
 * @param[in]       recursive       A recursive mutex or not.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mutex_init(os_mutex_t *mutex, const char *name, os_bool_t recursive)
{
    os_mutex_t     *iter_mutex;
    os_list_node_t *pos;
    os_bool_t       exist;
    os_err_t        ret;

    OS_ASSERT(mutex);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    exist = OS_FALSE;
    ret   = OS_EOK;
    
    os_spin_lock(&gs_os_mutex_resource_list_lock);
    os_list_for_each(pos, &gs_os_mutex_resource_list_head)
    {
        iter_mutex = os_list_entry(pos, os_mutex_t, resource_node);
        if (iter_mutex == mutex)
        {
            OS_KERN_LOG(KERN_ERROR, MUTEX_TAG, "The mutex(addr: %p, name: %s) has been exist",
                        iter_mutex,
                        iter_mutex->name);
                        
            exist = OS_TRUE;
            ret   = OS_EINVAL;
            break;
        }
    }

    if (OS_FALSE == exist)
    {
        os_list_add_tail(&gs_os_mutex_resource_list_head, &mutex->resource_node);
        os_spin_unlock(&gs_os_mutex_resource_list_lock);

        _k_mutex_init(mutex, name, recursive, OS_KOBJ_ALLOC_TYPE_STATIC);
    }
    else
    {
        os_spin_unlock(&gs_os_mutex_resource_list_lock);
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Deinitialize a mutex object.
 *
 * @param[in]       mutex           The mutex to deinitialize.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mutex_deinit(os_mutex_t *mutex)
{
    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT(OS_KOBJ_INITED == mutex->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_STATIC == mutex->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    _k_mutex_deinit(mutex);

    return OS_EOK;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function will create a mutex object from heap.
 *
 * @param[in]       name            The name of mutex.
 * @param[in]       recursive       A recursive mutex or not.
 *
 * @return          The pointer to the created mutex.
 * @retval          pointer         If operation successful.
 * @retval          OS_NULL         Error occurred.
 ***********************************************************************************************************************
 */
os_mutex_t *os_mutex_create(const char *name, os_bool_t recursive)
{
    os_mutex_t *mutex;

    /* Check context. */
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    mutex = (os_mutex_t *)OS_KERNEL_MALLOC(sizeof(os_mutex_t));
    if (OS_NULL == mutex)
    {
        OS_KERN_LOG(KERN_ERROR, MUTEX_TAG, "Malloc mutex memory failed");
    }
    else
    {
        os_spin_lock(&gs_os_mutex_resource_list_lock);
        os_list_add_tail(&gs_os_mutex_resource_list_head, &mutex->resource_node);
        os_spin_unlock(&gs_os_mutex_resource_list_lock);
        
        _k_mutex_init(mutex, name, recursive, OS_KOBJ_ALLOC_TYPE_DYNAMIC);
    }
    
    return mutex;
}

/**
 ***********************************************************************************************************************
 * @brief           Destory a mutex object created from heap.
 *
 * @param[in]       mutex           The mutex to destroy.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mutex_destroy(os_mutex_t *mutex)
{
    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT(OS_KOBJ_INITED == mutex->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_DYNAMIC == mutex->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    _k_mutex_deinit(mutex);

    OS_KERNEL_FREE(mutex);

    return OS_EOK;
}
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           This function locks a mutex. If the mutex is already locked by other task, the calling task will be blocked
 *                  until either the mutex becomes available or waiting time expires. When mutex is locked multiple times
 *                  by the same task, OS_ASSERT() will detect it.
 *
 * @param[in]       mutex           The pointer to a mutex.
 * @param[in]       timeout         Waitting time (in clock ticks).
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mutex_lock(os_mutex_t *mutex, os_tick_t timeout)
{
    os_task_t *current_task;
    os_bool_t  need_schedule;
    os_err_t   ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT(OS_KOBJ_INITED == mutex->object_inited);
    OS_ASSERT(OS_FALSE == mutex->is_recursive);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));
   
    ret           = OS_EOK;
    need_schedule = OS_FALSE;

    current_task = k_task_self();
    OS_ASSERT_EX(mutex->owner != current_task, "Task(%s) use mutex(%s) recursively!",
                 current_task->name,
                 mutex->name);

    OS_KERNEL_ENTER();

    if (mutex->lock_count == 0U)
    {
        mutex->lock_count        = 1U;
        mutex->owner             = current_task;
        mutex->original_priority = current_task->current_priority;

        os_list_add_tail(&current_task->hold_mutex_list_head, &mutex->hold_node);
    }
    else
    {
        if (timeout == OS_NO_WAIT)
        {
            ret = OS_EBUSY;
        }
        else
        {
            /* Priority inherit */
            if ((OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type)
                && (current_task->current_priority < mutex->owner->current_priority))
            {
                (void)_k_mutex_set_owner_priority(mutex, current_task->current_priority);
            }

            if (OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type)
            {
                k_block_task(&mutex->task_list_head, current_task, timeout, OS_TRUE);
            }
            else
            {
                k_block_task(&mutex->task_list_head, current_task, timeout, OS_FALSE);
            }
            
                  
            OS_KERNEL_EXIT_SCHED();

            ret = current_task->switch_retval;

            OS_KERNEL_ENTER();
            if ((OS_EOK != ret) && (OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type))
            {
                need_schedule = _k_mutex_restore_priority(mutex);
            }
        }
    }
        
    if (need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function unlocks a mutex. If there are tasks blocked on the mutex, the highest priority task
 *                  will be woken up and aqcuire the mutex. When mutex is unlocked multiple times by the same task, OS_ASSERT()
 *                  will detect it.
 *
 * @param[in]       mutex           The pointer to a mutex.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mutex_unlock(os_mutex_t *mutex)
{
    os_task_t *current_task;
    os_task_t *block_task;
    os_bool_t  need_schedule;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT(OS_KOBJ_INITED == mutex->object_inited);
    OS_ASSERT(OS_FALSE == mutex->is_recursive);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    need_schedule = OS_FALSE;
    current_task  = k_task_self();

    /* Mutex only can be unlocked by owner */
    OS_ASSERT_EX(mutex->owner == current_task, "The mutex(addr: %p, name: %s, owner: %s) can't unlock by task(%s)", 
                 mutex, 
                 mutex->name,
                 mutex->owner->name,
                 current_task->name);
          
    OS_KERNEL_ENTER();

    os_list_del(&mutex->hold_node);

    /* Restore task priority */
    if (OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type)
    {
        need_schedule = _k_mutex_restore_priority(mutex);
    }
    
    /* Get the new owner, if any */   
    if (!os_list_empty(&mutex->task_list_head))
    {
        block_task = os_list_first_entry(&mutex->task_list_head, os_task_t, task_node);
        k_unblock_task(block_task);

        mutex->owner             = block_task;
        mutex->original_priority = block_task->current_priority;
        os_list_add_tail(&mutex->owner->hold_mutex_list_head, &mutex->hold_node);

        if (block_task->state & OS_TASK_STATE_READY)
        {
            need_schedule = OS_TRUE;
        }
    }
    else
    {           
        mutex->owner      = OS_NULL;
        mutex->lock_count = 0;
    }

    if (os_list_empty(&current_task->hold_mutex_list_head)
        && (current_task->current_priority != current_task->backup_priority))
    {
        OS_KERN_LOG(KERN_INFO, MUTEX_TAG, "New priority(%u) takes effect, old priority(%u)",
                    current_task->backup_priority,
                    current_task->current_priority);
        
        k_readyq_remove(current_task);
        current_task->current_priority = current_task->backup_priority;
        k_readyq_put(current_task);

        need_schedule = OS_TRUE;
    }

    if (need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function recursively locks a mutex and the increments it's nested count. If the mutex is already
 *                  locked by other task, the calling task will block until either the mutex becomes available or waiting
 *                  time expires. In contrast to os_mutex_lock(), it's ok to lock a mutex multiple times.
 *
 * @param[in]       mutex           The pointer to a mutex.
 * @param[in]       timeout         Waitting time (in clock ticks).
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mutex_recursive_lock(os_mutex_t *mutex, os_tick_t timeout)
{
    os_task_t *current_task;
    os_bool_t  need_schedule;
    os_err_t   ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT(OS_KOBJ_INITED == mutex->object_inited);
    OS_ASSERT(OS_TRUE == mutex->is_recursive);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (OS_NO_WAIT == timeout));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    ret           = OS_EOK;
    need_schedule = OS_FALSE;
    current_task  = k_task_self();
    
    OS_KERNEL_ENTER();
    
    if ((0U == mutex->lock_count) || (current_task == mutex->owner))
    {
        if (0U == mutex->lock_count)
        {
            mutex->original_priority = current_task->current_priority;
            mutex->owner             = current_task;
            
            os_list_add_tail(&current_task->hold_mutex_list_head, &mutex->hold_node);
        }

        mutex->lock_count++;
    }
    else
    {
        if (timeout == OS_NO_WAIT)
        {
            ret = OS_EBUSY;
        }
        else
        {
            if ((OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type)
                && (current_task->current_priority < mutex->owner->current_priority))
            {
                (void)_k_mutex_set_owner_priority(mutex, current_task->current_priority);
            }

            if (OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type)
            {
                k_block_task(&mutex->task_list_head, current_task, timeout, OS_TRUE);
            }
            else
            {
                k_block_task(&mutex->task_list_head, current_task, timeout, OS_FALSE);
            }

            OS_KERNEL_EXIT_SCHED();
            
            ret = current_task->switch_retval;
    
            OS_KERNEL_ENTER();
            if ((OS_EOK != ret) && (OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type))
            {            
                need_schedule = _k_mutex_restore_priority(mutex);
            }
        }
    }

    if (need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function recursively unlocks a mutex and decrements it's nested count. If the nested count is 0
 *                  and there are tasks blocked on the mutex, the highest priority task will be woken up and
 *                  aqcuire the mutex. In contrast to os_mutex_unlock(), it's ok to unlock a mutex multiple times.
 *
 * @param[in]       mutex           The pointer to a mutex.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mutex_recursive_unlock(os_mutex_t *mutex)
{
    os_task_t *current_task;
    os_task_t *block_task;
    os_bool_t  need_schedule;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT(OS_KOBJ_INITED == mutex->object_inited);
    OS_ASSERT(OS_TRUE == mutex->is_recursive);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    need_schedule = OS_FALSE;
    current_task  = k_task_self();
    
    /* Mutex only can be unlocked by owner */
    if (current_task != mutex->owner)
    {
        OS_KERN_LOG(KERN_ERROR, MUTEX_TAG, "The mutex(addr: %p, name: %s, owner: %s) can't unlock by task(%s)", 
                    mutex, 
                    mutex->name,
                    mutex->owner->name,
                    current_task->name);
        OS_ASSERT(0);
    }

    OS_KERNEL_ENTER();

    OS_ASSERT(mutex->lock_count > 0U);

    if (mutex->lock_count > 1U)
    {
        mutex->lock_count--;
    }
    else
    {
        os_list_del(&mutex->hold_node);

        /* Restore task priority */
        if (OS_MUTEX_WAKE_TYPE_PRIO == mutex->wake_type)
        {
            need_schedule = _k_mutex_restore_priority(mutex);
        }
        
        /* Get the new owner, if any */
        if (!os_list_empty(&mutex->task_list_head))
        {
            block_task = os_list_first_entry(&mutex->task_list_head, os_task_t, task_node);
            k_unblock_task(block_task);

            mutex->owner             = block_task;
            mutex->original_priority = block_task->current_priority;
            
            os_list_add_tail(&mutex->owner->hold_mutex_list_head, &mutex->hold_node);

            if (block_task->state & OS_TASK_STATE_READY)
            {
                need_schedule = OS_TRUE;
            }
        }
        else
        {
            mutex->owner      = OS_NULL;
            mutex->lock_count = 0U;
        }
    }

    if ((current_task->current_priority != current_task->backup_priority)
        && os_list_empty(&current_task->hold_mutex_list_head))
    {
        OS_KERN_LOG(KERN_INFO, MUTEX_TAG, "New priority(%u) takes effect, old priority(%u)",
                    current_task->backup_priority,
                    current_task->current_priority);
        
        k_readyq_remove(current_task);
        current_task->current_priority = current_task->backup_priority;
        k_readyq_put(current_task);

        need_schedule = OS_TRUE;
    }

    if (need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    return OS_EOK;
}

os_err_t os_mutex_set_wake_type(os_mutex_t *mutex, os_uint8_t wake_type)
{
    os_err_t ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT((OS_MUTEX_WAKE_TYPE_PRIO == wake_type) || (OS_MUTEX_WAKE_TYPE_FIFO == wake_type));

    ret = OS_EBUSY;

    OS_KERNEL_ENTER();

    if (OS_NULL == mutex->owner)
    {
        mutex->wake_type = wake_type;
        ret              = OS_EOK;
    }

    OS_KERNEL_EXIT();

    return ret;
}

os_task_t *os_mutex_get_owner(os_mutex_t *mutex)
{
    OS_ASSERT(OS_NULL != mutex);
    OS_ASSERT(OS_KOBJ_INITED == mutex->object_inited);

    return mutex->owner;
}

#if defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP)
#include <shell.h>

#define SH_SHOW_TASK_CNT_MAX    5

typedef struct
{
    os_task_t   *task;
    os_uint8_t   task_priority;
}sh_block_task_info_t;

typedef struct
{
    os_mutex_t           *mutex;
    os_task_t            *owner;
    os_uint32_t           lock_count;
    
    os_uint8_t            owner_original_priority;
    os_uint8_t            owner_current_priority;
    
    os_uint16_t           block_task_count;
    sh_block_task_info_t  block_task_info[SH_SHOW_TASK_CNT_MAX];
}sh_mutex_info_t;

static void sh_mutex_get_blocked_task(os_list_node_t        *list_head,
                                      sh_block_task_info_t  *block_task_info_buff,
                                      os_uint16_t            buff_cnt_max)
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
    
        block_task_info_buff[task_index].task          = inter_task;
        block_task_info_buff[task_index].task_priority = inter_task->current_priority;
        task_index++;
    }

    return;
}

static void sh_mutex_show_blocked_task(sh_block_task_info_t *block_task_info_buff,
                                       os_uint16_t           buff_cnt_max,
                                       os_uint16_t           block_task_cnt)
{
    os_uint16_t task_index;
    char        name[OS_NAME_MAX + 1];

    for (task_index = 0U; task_index < buff_cnt_max; task_index++)
    {
        if (OS_NULL == block_task_info_buff[task_index].task)
        {
            break;
        }
  
        strncpy(name, block_task_info_buff[task_index].task->name, OS_NAME_MAX);
        name[OS_NAME_MAX] = '\0';
  
        os_kprintf("%s(%d)",
                   (name[0] != '\0') ? name : "-",
                   block_task_info_buff[task_index].task_priority);

        if ((task_index + 1U < buff_cnt_max) && (OS_NULL != block_task_info_buff[task_index + 1U].task))
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

static os_uint16_t sh_copy_mutex_info(sh_mutex_info_t *mutex_info_buff, os_uint16_t buff_cnt)
{
    os_mutex_t  *iter_mutex;
    os_uint16_t  copy_mutex_count;

    OS_KERNEL_INIT();

    os_spin_lock(&gs_os_mutex_resource_list_lock);

    copy_mutex_count = 0;
    os_list_for_each_entry(iter_mutex, &gs_os_mutex_resource_list_head, os_mutex_t, resource_node)
    {
        OS_KERNEL_ENTER();
        
        mutex_info_buff[copy_mutex_count].mutex      = iter_mutex;
        mutex_info_buff[copy_mutex_count].owner      = iter_mutex->owner;
        mutex_info_buff[copy_mutex_count].lock_count = iter_mutex->lock_count;
        
        if (OS_NULL != iter_mutex->owner)
        {
            mutex_info_buff[copy_mutex_count].owner_original_priority = iter_mutex->original_priority;
            mutex_info_buff[copy_mutex_count].owner_current_priority  = iter_mutex->owner->current_priority;
        }
        else
        {
            mutex_info_buff[copy_mutex_count].owner_original_priority = 0U;
            mutex_info_buff[copy_mutex_count].owner_current_priority  = 0U;
        }

        mutex_info_buff[copy_mutex_count].block_task_count = os_list_len(&iter_mutex->task_list_head);
        sh_mutex_get_blocked_task(&iter_mutex->task_list_head,
                                  mutex_info_buff[copy_mutex_count].block_task_info,
                                  SH_SHOW_TASK_CNT_MAX);

        OS_KERNEL_EXIT();

        copy_mutex_count++;
        if (copy_mutex_count == buff_cnt)
        {
            break;
        }
    }

    os_spin_unlock(&gs_os_mutex_resource_list_lock);

    return copy_mutex_count;
}

/**
 ***********************************************************************************************************************
 * @brief           This function prints information about all the mutex and it's corresponding blokced tasks.
 *
 * @param[in]
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t sh_show_mutex_info(os_int32_t argc, char **argv)
{
    sh_mutex_info_t *mutex_info;
    os_uint16_t      mutex_count;
    os_uint16_t      copy_mutex_count;
    os_uint16_t      mutex_index;
    os_uint16_t      len;
    char             mutex_name[OS_NAME_MAX + 1];
    char             owner_name[OS_NAME_MAX + 1];
    char             ori_cur_priority[10];
    os_int32_t       ret;
    
    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s %-*s %-10s %-10s %-9s %-10s\r\n", 
               OS_NAME_MAX, 
               "Mutex", 
               OS_NAME_MAX, 
               "Owner name",
               "Owner priority(ori/cur)",
               "Lock Count", 
               "Recursive",
               "Block Task");

    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    os_kprintf(" ");
    
    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    os_kprintf(" ----------------------- ---------- --------- ----------\r\n");

    os_spin_lock(&gs_os_mutex_resource_list_lock);
    mutex_count = os_list_len(&gs_os_mutex_resource_list_head);
    os_spin_unlock(&gs_os_mutex_resource_list_lock);

    mutex_info = OS_NULL;
    ret        = OS_EOK;

    if (0U != mutex_count)
    {
        mutex_info = (sh_mutex_info_t *)os_malloc(sizeof(sh_mutex_info_t) * mutex_count);
        if (OS_NULL == mutex_info)
        {
            ret = OS_ENOMEM;
        }
    }

    if ((OS_EOK == ret) && (OS_NULL != mutex_info))
    {
        memset((void *)mutex_info, 0, sizeof(sh_mutex_info_t) * mutex_count);
    
        copy_mutex_count = sh_copy_mutex_info(mutex_info, mutex_count);

        for (mutex_index = 0U; mutex_index < copy_mutex_count; mutex_index++)
        {
            if (OS_KOBJ_INITED != mutex_info[mutex_index].mutex->object_inited)
            {
                continue;
            }

            strncpy(mutex_name, mutex_info[mutex_index].mutex->name, OS_NAME_MAX);
            mutex_name[OS_NAME_MAX] = '\0';

            if (OS_NULL != mutex_info[mutex_index].owner)
            {
                strncpy(owner_name, mutex_info[mutex_index].owner->name, OS_NAME_MAX);
                owner_name[OS_NAME_MAX] = '\0';
            }
            else
            {
                owner_name[0] = '\0';
            }

            memset(ori_cur_priority, 0, sizeof(ori_cur_priority));
            if (OS_NULL != mutex_info[mutex_index].owner)
            {
                os_snprintf(ori_cur_priority, sizeof(ori_cur_priority), "%u/%u",
                            mutex_info[mutex_index].owner_original_priority,
                            mutex_info[mutex_index].owner_current_priority);
            }
            else
            {
                strncpy(ori_cur_priority, "-", sizeof(ori_cur_priority));
            }

            os_kprintf("%-*s %-*s %-23s %-10u %-9s %-10u:",
                       OS_NAME_MAX,
                       (mutex_name[0] != '\0') ? mutex_name : "-",
                       OS_NAME_MAX,
                       (owner_name[0] != '\0') ? owner_name : "-",
                       ori_cur_priority,
                       mutex_info[mutex_index].lock_count,
                       mutex_info[mutex_index].mutex->is_recursive ? "Yes" : "No",
                       mutex_info[mutex_index].block_task_count);
                       
            sh_mutex_show_blocked_task(mutex_info[mutex_index].block_task_info,
                                       SH_SHOW_TASK_CNT_MAX,
                                       mutex_info[mutex_index].block_task_count);
            os_kprintf("\r\n");
        }

        os_free(mutex_info);
        mutex_info = OS_NULL;
    }

    return ret;
}
SH_CMD_EXPORT(show_mutex, sh_show_mutex_info, "Show mutex information");

#endif /* defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP) */

#endif /* OS_USING_MUTEX */

