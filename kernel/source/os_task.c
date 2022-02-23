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
 * @file        os_task.c
 *
 * @brief       This file implements the task functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-06   OneOS team      First Version
 * 2020-11-10   OneOS team      Refactor task implementation.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <arch_interrupt.h>
#include <os_errno.h>
#include <os_clock.h>
#include <string.h>
#include <os_spinlock.h>
#include <arch_misc.h>
#include <arch_exception.h>
#include <os_safety.h>

#include "os_kernel_internal.h"

#define TASK_TAG                        "TASK"

/* The stack and control block of recycle-task */
static OS_TASK_STACK_DEFINE(gs_os_recycle_task_stack, OS_RECYCLE_TASK_STACK_SIZE);
static os_task_t      gs_os_recycle_task;

static os_list_node_t gs_os_task_resource_list_head = OS_LIST_INIT(gs_os_task_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_task_resource_list_lock);

static os_list_node_t gs_os_task_recycle_list_head  = OS_LIST_INIT(gs_os_task_recycle_list_head);

/* When in interrupt context or when scheduler is not started, it is used to record the error code */
static os_err_t gs_os_err_code = OS_EOK;

#ifdef OS_USING_TASK_HOOK
static void (*gs_os_task_switch_hook)(os_task_t *from, os_task_t *to) = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           This function sets a hook function called at the between two task switch.
 *
 * @param[in]       hook            The hook function is complemented by user.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void os_task_switch_set_hook(void (*hook)(os_task_t *from, os_task_t *to))
{
    gs_os_task_switch_hook = hook;
}
#endif /* OS_USING_TASK_HOOK */

#ifdef OS_USING_OVERFLOW_CHECK
static void _k_task_stack_check(os_task_t *from_task, os_task_t *to_task)
{
    os_bool_t is_overflow;

    OS_ASSERT(OS_NULL != from_task);
    OS_ASSERT(OS_NULL != to_task);

    is_overflow = os_task_stack_is_overflow(from_task->stack_top, from_task->stack_begin, from_task->stack_end);
    if (OS_TRUE == is_overflow)
    {
        OS_KERN_LOG(KERN_ERROR, TASK_TAG, "Stack overflow, switch from task(%s), sp is 0x%p.\r\n",
                    os_task_name(from_task),
                    from_task->stack_top);

        os_safety_task_stack_overflow_process();
    }

    is_overflow = os_task_stack_is_overflow(to_task->stack_top, to_task->stack_begin, to_task->stack_end);
    if (OS_TRUE == is_overflow)
    {
        OS_KERN_LOG(KERN_ERROR, TASK_TAG, "Stack overflow, switch to task(%s), sp is 0x%p.\r\n",
                    os_task_name(to_task),
                    to_task->stack_top);
    
        os_safety_task_stack_overflow_process();
    }

    return;
}
#endif /* OS_USING_OVERFLOW_CHECK */

/*
 * If define OS_USING_OVERFLOW_CHECK or OS_USING_TASK_HOOK, OS_TASK_SWITCH_NOTIFY is defined
 * in gcc(-DOS_TASK_SWITCH_NOTIFY).
 */
#ifdef OS_TASK_SWITCH_NOTIFY
/**
 ***********************************************************************************************************************
 * @brief           This function is called at the between two task switch.
 *
 * @param           None.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void os_task_switch_notify(void)
{
#ifdef OS_USING_OVERFLOW_CHECK
    _k_task_stack_check(g_os_current_task, g_os_next_task);
#endif

#ifdef OS_USING_TASK_HOOK
    if (OS_NULL != gs_os_task_switch_hook)
    {
        gs_os_task_switch_hook(g_os_current_task, g_os_next_task);
    }
#endif
}
#endif /* OS_TASK_SWITCH_NOTIFY */

static void _k_recycle_task_entry(void *arg)
{
    os_task_t  *iter_task;
    os_task_t  *current_task;
    os_uint8_t  object_alloc_type;

    OS_KERNEL_INIT();

    OS_UNREFERENCE(arg);
    
    while (1)
    {
        OS_KERNEL_ENTER();

        iter_task = OS_NULL;
        while (1)
        {
            if (os_list_empty(&gs_os_task_recycle_list_head))
            {
                break;
            }
        
            iter_task = os_list_first_entry(&gs_os_task_recycle_list_head, os_task_t, resource_node);
            os_list_del(&iter_task->resource_node);

            OS_KERNEL_EXIT();

            OS_KERN_LOG(KERN_INFO, TASK_TAG, "Recycle task(%s)", iter_task->name);

            /*
             * iter_task memory maybe in user_data, in this case, iter_task memory will be freed at cleanup function.
             * So, object_alloc_type of iter_task is stored at the temporary variable, avoiding illegal memory access.
             * 
             * In addition, if iter_task memory is in user_data, the variabe object_alloc_type of iter_task can't be
             * OS_KOBJ_ALLOC_TYPE_DYNAMIC. Otherwise, there is a bug.
             */
            object_alloc_type = iter_task->object_alloc_type;

            if (OS_NULL != iter_task->cleanup)
            {
                iter_task->cleanup(iter_task->user_data);
            }

            if (object_alloc_type == OS_KOBJ_ALLOC_TYPE_DYNAMIC)
            {
                OS_KERNEL_FREE(iter_task->stack_begin);
                iter_task->stack_top   = OS_NULL;
                iter_task->stack_begin = OS_NULL;
                iter_task->stack_end   = OS_NULL;

                OS_KERNEL_FREE(iter_task);
                iter_task = OS_NULL;
            }

            OS_KERNEL_ENTER();
        }

        /* Suspend myself */
        current_task = k_task_self();
        k_readyq_remove(current_task);
        current_task->state &= ~OS_TASK_STATE_READY;
        current_task->state |= OS_TASK_STATE_SUSPEND;

        OS_KERNEL_EXIT_SCHED();
    }
}

void k_recycle_task_init(void)
{
    os_err_t ret;
    
    ret = os_task_init(&gs_os_recycle_task,
                       OS_RECYCLE_TASK_NAME, 
                       _k_recycle_task_entry,
                       OS_NULL,
                       OS_TASK_STACK_BEGIN_ADDR(gs_os_recycle_task_stack),
                       OS_TASK_STACK_SIZE(gs_os_recycle_task_stack),
                       0U);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why initialize recycle task failed?");
    }

    ret = os_task_startup(&gs_os_recycle_task);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Why startup recycle task failed?");
    }

    return;
}

static void _k_wakeup_recycle_task(void)
{
    if (OS_TASK_STATE_SUSPEND == gs_os_recycle_task.state)
    {
        gs_os_recycle_task.state &= ~OS_TASK_STATE_SUSPEND;
        gs_os_recycle_task.state |= OS_TASK_STATE_READY;
        k_readyq_put(&gs_os_recycle_task);
    }

    return;
}

static void _k_task_exit(void)
{
    os_task_t *current_task;

    OS_KERNEL_INIT();

    current_task = k_task_self();

    os_spin_lock(&gs_os_task_resource_list_lock);
    os_list_del(&current_task->resource_node);
    os_spin_unlock(&gs_os_task_resource_list_lock);

    OS_KERNEL_ENTER();
    
    k_readyq_remove(current_task);
    current_task->state &= ~OS_TASK_STATE_READY;

    current_task->state |= OS_TASK_STATE_CLOSE;
    
    if ((OS_KOBJ_ALLOC_TYPE_DYNAMIC == current_task->object_alloc_type)
        || (OS_NULL != current_task->cleanup))
    {
        os_list_add_tail(&gs_os_task_recycle_list_head, &current_task->resource_node);
        _k_wakeup_recycle_task();
    }
    
    OS_KERNEL_EXIT_SCHED();
    
    return;
}

static void _k_task_init(os_task_t    *task,
                         const char   *name, 
                         void        (*entry)(void *arg),
                         void         *arg,
                         void         *stack_begin,
                         os_uint32_t   stack_size,
                         os_uint8_t    priority,
                         os_uint16_t   object_alloc_type)
{
    if (OS_NULL != name)
    {
        (void)strncpy(task->name, name, OS_NAME_MAX);
        task->name[OS_NAME_MAX] = '\0';
    }
    else
    {
        task->name[0] = '\0';
    }

    task->object_alloc_type    = object_alloc_type;
    task->err_code             = OS_EOK;
    task->switch_retval        = OS_EOK;
    task->backup_priority      = priority;
    task->current_priority     = priority;
    task->time_slice           = OS_SCHEDULE_TIME_SLICE;
    task->remaining_time_slice = OS_SCHEDULE_TIME_SLICE;
    task->user_data            = OS_NULL;
    task->block_list_head      = OS_NULL;

    task->stack_begin = stack_begin;
    task->stack_end   = (void *)((os_uint8_t *)stack_begin + stack_size);
    task->stack_top   = os_hw_stack_init(entry, arg, stack_begin, stack_size, _k_task_exit);

    os_list_init(&task->task_node);
    os_list_init(&task->tick_node);

#if defined(OS_USING_MUTEX)
    os_list_init(&task->hold_mutex_list_head);   
#endif

#if defined(OS_USING_EVENT)
    task->event_set    = 0U;
    task->event_option = 0U;
#endif

    task->swap_data    = 0U;

    task->cleanup      = OS_NULL;
    task->user_data    = OS_NULL;

    return;
}

static void _k_task_deinit(os_task_t *task)
{
    /* Task is at ready queue */
    if (task->state & OS_TASK_STATE_READY)
    {
        k_readyq_remove(task);
        task->state &= ~OS_TASK_STATE_READY;
    }
    /* Task is not at ready queue */
    else
    {
        /* Task is sleep state */
        if (task->state & OS_TASK_STATE_SLEEP)
        {
            k_tickq_remove(task);
            task->state &= ~OS_TASK_STATE_SLEEP;
        }

        /* Task is block state */
        if (task->state & OS_TASK_STATE_BLOCK)
        {
            /* Remove from ipc list head */
            os_list_del(&task->task_node);
            task->state &= ~OS_TASK_STATE_BLOCK;
        }

        if (task->state & OS_TASK_STATE_INIT)
        {
            task->state &= ~OS_TASK_STATE_INIT;
        }

        if (task->state & OS_TASK_STATE_SUSPEND)
        {
            task->state &= ~OS_TASK_STATE_SUSPEND;
        }

        /* Here, if task is running state, do nothing. */
    }

    return;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function creates a task with dynamic memory allocation.
 *
 * @details         Both control block and stack of the task are allocated in memory heap.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       name            Task name.
 * @param[in]       entry           Entry function of the task.
 * @param[in]       arg             Argument of entry function.
 * @param[in]       stack_size      Stack size in bytes.
 * @param[in]       priority        Priority of task.
 * 
 * @return          The address of task's control block.
 * @retval          OS_NULL         Failed to create a task with dynamic memory allocation.
 * @retval          else            Return the address of task control block.
 ***********************************************************************************************************************
 */
os_task_t *os_task_create(const char   *name, 
                          void        (*entry)(void *arg),
                          void         *arg,
                          os_uint32_t   stack_size,
                          os_uint8_t    priority)

{
    os_task_t *task;
    void      *stack_begin;

    OS_ASSERT(OS_NULL != entry);
    OS_ASSERT(stack_size > 0);
    OS_ASSERT(priority < OS_TASK_PRIORITY_MAX);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    stack_size  = OS_ALIGN_UP(stack_size, OS_ARCH_STACK_ALIGN_SIZE);
    stack_begin = OS_KERNEL_MALLOC_ALIGN(OS_ARCH_STACK_ALIGN_SIZE, stack_size);
    task        = (os_task_t *)OS_KERNEL_MALLOC(sizeof(os_task_t));
    
    if ((OS_NULL == stack_begin) || (OS_NULL == task))
    {
        OS_KERN_LOG(KERN_ERROR, TASK_TAG, "Malloc failed, stack_begin(%p), task(%p)", stack_begin, task);

        if (OS_NULL != stack_begin)
        {
            OS_KERNEL_FREE(stack_begin);
            stack_begin = OS_NULL;
        }

        if (OS_NULL != task)
        {
            OS_KERNEL_FREE(task);
            task = OS_NULL;
        }
    }
    else
    {
        _k_task_init(task, name, entry, arg, stack_begin, stack_size, priority, OS_KOBJ_ALLOC_TYPE_DYNAMIC);    

        task->state = OS_TASK_STATE_INIT;

        os_spin_lock(&gs_os_task_resource_list_lock); 
        os_list_add_tail(&gs_os_task_resource_list_head, &task->resource_node);
        os_spin_unlock(&gs_os_task_resource_list_lock);

        task->object_inited = OS_KOBJ_INITED;
    }

    return task;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy a task.
 *
 * @details         If the task to be destroyed is the current task, the resource will be recycled in recycle-task. 
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       task            Task control block.
 *
 * @return          The result of destroying the task.
 * @retval          OS_EOK          Destroy the task successfully.
 * @retval          else            Destroy the task failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_destroy(os_task_t *task)
{
    os_task_t *current_task;
    os_bool_t  need_schedule;
    os_bool_t  task_hold_mutex;
    os_err_t   ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != task);
    OS_ASSERT(OS_KOBJ_INITED == task->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_DYNAMIC == task->object_alloc_type);
    OS_ASSERT((task->state & OS_TASK_STATE_CLOSE) == OS_TASK_STATE_EMPTY);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    need_schedule = OS_FALSE;
    ret           = OS_EOK;

    os_spin_lock(&gs_os_task_resource_list_lock);    
    os_list_del(&task->resource_node);
    os_spin_unlock(&gs_os_task_resource_list_lock);
                              
    OS_KERNEL_ENTER();

#if defined(OS_USING_MUTEX)
    task_hold_mutex = !os_list_empty(&task->hold_mutex_list_head);
#else
    task_hold_mutex = OS_FALSE;
#endif    

    if (OS_TRUE == task_hold_mutex)
    {
        ret = OS_EBUSY;
    }
    else
    {
        task->object_inited = OS_KOBJ_DEINITED;                            
        _k_task_deinit(task); 
        task->state |= OS_TASK_STATE_CLOSE;

        current_task = k_task_self();
        if (task == current_task)
        {
            os_list_add_tail(&gs_os_task_recycle_list_head, &task->resource_node);
            _k_wakeup_recycle_task();

            need_schedule = OS_TRUE;
        }
        else
        {
            OS_KERNEL_EXIT();

            if (OS_NULL != task->cleanup)
            {
                task->cleanup(task->user_data);
            }

            OS_KERNEL_FREE(task->stack_begin);
            task->stack_top   = OS_NULL;
            task->stack_begin = OS_NULL;
            task->stack_end   = OS_NULL;

            OS_KERNEL_FREE(task);
            task = OS_NULL;

            OS_KERNEL_ENTER();
        }
    }

    if (OS_TRUE == need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    if (OS_EOK != ret)
    {
        os_spin_lock(&gs_os_task_resource_list_lock);
        os_list_add_tail(&gs_os_task_resource_list_head, &task->resource_node);
        os_spin_unlock(&gs_os_task_resource_list_lock);   
    }
    
    return ret;
}
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           This function initialize a task with static memory allocation.
 *
 * @attention       This interface is not allowed in interrupt context.
 *
 * @param[in]       task            Task control block.
 * @param[in]       name            Task name.
 * @param[in]       entry           Entry function of the task.
 * @param[in]       arg             Argument of the entry function.
 * @param[in]       stack_begin     The beginning address of stack memory.
 * @param[in]       stack_size      Stack size in bytes.
 * @param[in]       priority        Priority of task.
 *
 * @return          The result of initializing the task.
 * @retval          OS_EOK          Initialize the task successfully. 
 * @retval          else            Initialize the task failed..
 ***********************************************************************************************************************
 */
os_err_t os_task_init(os_task_t    *task,
                      const char   *name, 
                      void        (*entry)(void *arg),
                      void         *arg,
                      void         *stack_begin,
                      os_uint32_t   stack_size,
                      os_uint8_t    priority)
{
    os_task_t      *iter_task;
    os_list_node_t *pos;
    os_bool_t       exist;
    os_err_t        ret;

    OS_ASSERT(OS_NULL != task);
    OS_ASSERT(OS_NULL != entry);
    OS_ASSERT(OS_NULL != stack_begin);
    OS_ASSERT(stack_size > 0);
    OS_ASSERT(priority < OS_TASK_PRIORITY_MAX);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    exist = OS_FALSE;
    ret   = OS_EOK;

    os_spin_lock(&gs_os_task_resource_list_lock);
    os_list_for_each(pos, &gs_os_task_resource_list_head)
    {
        iter_task = os_list_entry(pos, os_task_t, resource_node);
        if (iter_task == task)
        {
            OS_KERN_LOG(KERN_ERROR, TASK_TAG, "The task(addr: 0x%p, name: %s) has been exist",
                        iter_task,
                        iter_task->name);

            exist = OS_TRUE;
            ret   = OS_EINVAL;
            break;
        }
    }

    if (OS_FALSE == exist)
    {
        os_list_add_tail(&gs_os_task_resource_list_head, &task->resource_node);
        os_spin_unlock(&gs_os_task_resource_list_lock);

        _k_task_init(task, name, entry, arg, stack_begin, stack_size, priority, OS_KOBJ_ALLOC_TYPE_STATIC);
    	
        task->state         = OS_TASK_STATE_INIT;
        task->object_inited = OS_KOBJ_INITED;
    }
    else
    {
        os_spin_unlock(&gs_os_task_resource_list_lock);
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Deinitialize a task.
 *
 * @details         If the task to be deinitialized is the current task, the resource will be recycled in recycle-task.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. The task to be deinitialized is the current task and interrupt is disabled.
 *                      3. The task to be deinitialized is the current task and scheduler is locked.
 *
 * @param[in]       task            Task control block.
 *
 * @return          The result of deinitializing the task.
 * @retval          OS_EOK          Deinitialize the task successfully.
 * @retval          else            Deinitialize the task failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_deinit(os_task_t *task)
{
    os_task_t *current_task;
    os_bool_t  need_schedule;
    os_bool_t  task_hold_mutex;
    os_err_t   ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != task);
    OS_ASSERT(OS_KOBJ_INITED == task->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_STATIC == task->object_alloc_type);
    OS_ASSERT((task->state & OS_TASK_STATE_CLOSE) == OS_TASK_STATE_EMPTY);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (task != k_task_self()));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (task != k_task_self()));

    need_schedule = OS_FALSE;
    ret           = OS_EOK;

    os_spin_lock(&gs_os_task_resource_list_lock);
    os_list_del(&task->resource_node);
    os_spin_unlock(&gs_os_task_resource_list_lock);

    OS_KERNEL_ENTER();

#if defined(OS_USING_MUTEX)
    task_hold_mutex = !os_list_empty(&task->hold_mutex_list_head);
#else
    task_hold_mutex = OS_FALSE;
#endif    

    if (OS_TRUE == task_hold_mutex)
    {
        ret = OS_EBUSY;
    }
    else
    {
        task->object_inited = OS_KOBJ_DEINITED;
        _k_task_deinit(task);
        task->state |= OS_TASK_STATE_CLOSE;
        
        current_task = k_task_self();
        if (task == current_task)
        {
            if (OS_NULL != task->cleanup)
            {
                os_list_add_tail(&gs_os_task_recycle_list_head, &task->resource_node);
                _k_wakeup_recycle_task();    
            }

            need_schedule = OS_TRUE;
        }
        else
        {
            if (OS_NULL != task->cleanup)
            {
                OS_KERNEL_EXIT();
                task->cleanup(task->user_data);
                OS_KERNEL_ENTER();
            }
        }
    }

    if (OS_TRUE == need_schedule)
    {
        OS_KERNEL_EXIT_SCHED();
    }
    else
    {
        OS_KERNEL_EXIT();
    }

    if (OS_EOK != ret)
    {
        os_spin_lock(&gs_os_task_resource_list_lock);
        os_list_add_tail(&gs_os_task_resource_list_head, &task->resource_node);
        os_spin_unlock(&gs_os_task_resource_list_lock);   
    }

    return ret;
}

void os_task_set_cleanup_callback(os_task_t *task, void (*cleanup)(void *user_data), void *user_data)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != task);
    OS_ASSERT(OS_NULL != cleanup);

    OS_KERNEL_ENTER();
    task->cleanup   = cleanup;
    task->user_data = user_data;
    OS_KERNEL_EXIT();

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Startup a task.
 *
 * @details         Put the task into the ready queue and trigger a schedule.        
 *
 * @param[in]       task            Task control block.
 *
 * @return          The result of starting the task.
 * @retval          OS_EOK          Startup the task successfully.
 * @retval          else            Startup the task failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_startup(os_task_t *task)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != task);
    OS_ASSERT(OS_KOBJ_INITED == task->object_inited);
    OS_ASSERT(OS_TASK_STATE_INIT == task->state);

    OS_KERNEL_ENTER();

    task->state &= ~OS_TASK_STATE_INIT;
    task->state |= OS_TASK_STATE_READY;
    k_readyq_put(task);

    OS_KERNEL_EXIT_SCHED();
 
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Suspend a task.
 *
 * @param[in]       task            Task control block.
 *
 * @return          The result of suspending the task.
 * @retval          OS_EOK          Suspend the task successfully.
 * @retval          else            Suspend the task failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_suspend(os_task_t *task)
{
    os_task_t *current_task;
    os_err_t   ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != task);
    OS_ASSERT(OS_KOBJ_INITED == task->object_inited);
    OS_ASSERT((OS_FALSE == os_is_irq_disabled()) || (task != k_task_self()));
    OS_ASSERT((OS_FALSE == os_is_schedule_locked()) || (task != k_task_self()));
    
    ret = OS_EOK;

    OS_KERNEL_ENTER();
    if (!(task->state & (OS_TASK_STATE_READY | OS_TASK_STATE_SLEEP | OS_TASK_STATE_BLOCK | OS_TASK_STATE_RUNNING)))
    {
        OS_KERNEL_EXIT();
        OS_KERN_LOG(KERN_ERROR, TASK_TAG, "Incorrect task(%s) state(0x%04X), not allow to suspend.",
                    task->name,
                    task->state);
        
        ret = OS_ERROR;
    }
    else
    {
        /* The task is at ready queue */
        if (task->state & OS_TASK_STATE_READY)
        {
            k_readyq_remove(task);
            task->state &= ~OS_TASK_STATE_READY;
        }

        task->state |= OS_TASK_STATE_SUSPEND;

        current_task = k_task_self();
        if (task != current_task)
        {
            OS_KERNEL_EXIT();
        }
        else
        {
            OS_KERNEL_EXIT_SCHED();
        }
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Resume a task from suspended state.
 *
 * @param[in]       task            Task control block.
 *
 * @return          The result of suspending the task.
 * @retval          OS_EOK          Resume the task successfully.
 * @retval          else            Resume the task failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_resume(os_task_t *task)
{
    os_err_t ret;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != task);
    OS_ASSERT(OS_KOBJ_INITED == task->object_inited);

    ret = OS_EOK;

    OS_KERNEL_ENTER();
    
    if (!(task->state & OS_TASK_STATE_SUSPEND))
    {
        OS_KERNEL_EXIT();
        OS_KERN_LOG(KERN_ERROR, TASK_TAG, "Task(%s) state is not suspend, not allow to resume.", task->name);

        ret = OS_ERROR;    
    }
    else
    {
        task->state &= ~OS_TASK_STATE_SUSPEND;

        if (task->state & (OS_TASK_STATE_SLEEP | OS_TASK_STATE_BLOCK))
        {
            /* 
             * When the task sleep or block, only clear the suspend state.
             * In this case, the resume is also considered successful.
             */
            OS_KERNEL_EXIT();
        }
        else
        {
            task->state |= OS_TASK_STATE_READY;
            k_readyq_put(task);

            OS_KERNEL_EXIT_SCHED();
        }
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Task actively give up cpu to tasks of equal priority.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       task            Task control block.
 *
 * @return          The result of yield the task.
 * @retval          OS_EOK          Yield the task successfully.
 * @retval          else            Yield the task failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_yield(void)
{
    os_task_t *current_task;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    OS_KERNEL_ENTER();
    
    current_task = k_task_self();
    OS_ASSERT(current_task != OS_NULL);

    if (current_task->task_node.next != current_task->task_node.prev)
    {
        k_readyq_move_tail(current_task);
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
 * @brief           Change time slice of task
 *
 * @param[in]       task            Task control block.
 * @param[in]       new_time_slice  New time slice.
 *
 * @return          The result of setting new time slice.
 * @retval          OS_EOK          Set new time slice successfully.
 * @retval          else            Set new time slice failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_set_time_slice(os_task_t *task, os_tick_t new_time_slice)
{
    OS_KERNEL_INIT();

    OS_ASSERT(task != OS_NULL);
    OS_ASSERT(task->object_inited == OS_KOBJ_INITED);
    OS_ASSERT(new_time_slice > 0);

    OS_KERNEL_ENTER();
    
    task->time_slice = new_time_slice;

    if (task->state & OS_TASK_STATE_INIT)
    {
        task->remaining_time_slice = new_time_slice;   
    }
    
    OS_KERNEL_EXIT();
    
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Get time slice of task
 *
 * @param[in]       task            Task control block.
 *
 * @return          Time slice of the task.
 ***********************************************************************************************************************
 */
os_tick_t os_task_get_time_slice(os_task_t *task)
{
    OS_ASSERT(task != OS_NULL);

    return task->time_slice;
}

/**
 ***********************************************************************************************************************
 * @brief           Change priority of task
 *
 * @param[in]       task            Task control block.
 * @param[in]       new_priority    New priority.
 *
 * @return          The result of setting priority.
 * @retval          OS_EOK          Set new priority successfully.
 * @retval          else            Set new priority failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_set_priority(os_task_t *task, os_uint8_t new_priority)
{
    os_bool_t need_schedule;
    os_bool_t task_hold_mutex;

    OS_KERNEL_INIT();

    OS_ASSERT(task != OS_NULL);
    OS_ASSERT(new_priority < OS_TASK_PRIORITY_MAX);
    OS_ASSERT(task->object_inited == OS_KOBJ_INITED);

    need_schedule = OS_FALSE;

    OS_KERNEL_ENTER();

#if defined(OS_USING_MUTEX)
    task_hold_mutex = !os_list_empty(&task->hold_mutex_list_head);
#else
    task_hold_mutex = OS_FALSE;
#endif

    task->backup_priority = new_priority;

    /* Task does not hold mutex */
    if (OS_FALSE == task_hold_mutex)
    {
        if (task->state & OS_TASK_STATE_READY)
        {
            k_readyq_remove(task);
            task->current_priority = new_priority;
            k_readyq_put(task);

            need_schedule = OS_TRUE;
        }
        else if (task->state & OS_TASK_STATE_BLOCK)
        {
            OS_ASSERT(OS_NULL != task->block_list_head);

            if (OS_TRUE == task->is_wake_prio)
            {
                os_list_del(&task->task_node);
                task->current_priority = new_priority;
                k_blockq_insert(task->block_list_head, task);
            }
            else
            {
                task->current_priority = new_priority;
            }
        }
        else
        {
            task->current_priority = new_priority;
        }
    }

    if (OS_TRUE == need_schedule)
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
 * @brief           Get priority of task
 *
 * @param[in]       task            Task control block.
 *
 * @return          Priority of the task.
 ***********************************************************************************************************************
 */
os_uint8_t os_task_get_priority(os_task_t *task)
{
    OS_ASSERT(task != OS_NULL);

    return task->current_priority;
}

/**
 ***********************************************************************************************************************
 * @brief           Get current task control block.
 *
 * @param           None.
 * 
 * @return          The address of current task control block.
 * @retval          OS_NULL         Scheduler is not started.
 * @retval          else            Return the address of current task control block.
 ***********************************************************************************************************************
 */
os_task_t *os_task_self(void)
{
    return g_os_current_task;
}

/**
 ***********************************************************************************************************************
 * @brief           Get task control block by name.
 *
 * @attention       Because the idle-task and recycle-task are built-in tasks, they cannot be found.
 *
 * @param[in]       name            Task name.
 *
 * @return          The address of task control block to be found.
 * @retval          OS_NULL         Task with the specified name was not found.
 * @retval          else            The address of task control block to be found by name.
 ***********************************************************************************************************************
 */
os_task_t *os_task_find(const char *name)
{
    os_list_node_t *pos;
    os_task_t      *iter_task;
    os_task_t      *found_task;

    OS_ASSERT(OS_NULL != name);

    found_task = OS_NULL;

    /* 
     * Because recycle task and idle task are system tasks, 
     * They don't want to be accessed by the application layer.
     */
    if ((name[0] != '\0')
        && strncmp(name, OS_RECYCLE_TASK_NAME, OS_NAME_MAX)
        && strncmp(name, OS_IDLE_TASK_NAME, OS_NAME_MAX))
    {
        os_spin_lock(&gs_os_task_resource_list_lock);
        os_list_for_each(pos, &gs_os_task_resource_list_head)
        {
            iter_task = os_list_entry(pos, os_task_t, resource_node);
            if (!strncmp(name, iter_task->name, OS_NAME_MAX))
            {   
                found_task = iter_task;
                break;
            }
        }
        os_spin_unlock(&gs_os_task_resource_list_lock);
    }
    
    return found_task;
}

/**
 ***********************************************************************************************************************
 * @brief           Query whether the specified task exists.
 *
 * @param[in]       task            Task control block.
 *
 * @return          Whether the specified task exists.
 * @retval          OS_TRUE         The specified task exists.
 * @retval          OS_FALSE        The specified task doesn't exists.
 ***********************************************************************************************************************
 */
os_bool_t os_task_check_exist(os_task_t *task)
{
    os_task_t      *iter_task;
    os_list_node_t *node;
    os_bool_t       exist;

    OS_ASSERT(OS_NULL != task);

    exist = OS_FALSE;

    os_spin_lock(&gs_os_task_resource_list_lock);
    os_list_for_each(node, &gs_os_task_resource_list_head)
    {
        iter_task = os_list_entry(node, os_task_t, resource_node);
        if (task == iter_task)
        {
            exist = OS_TRUE;
            break;
        }
    }
    os_spin_unlock(&gs_os_task_resource_list_lock);
    
    return exist;
}

/**
 ***********************************************************************************************************************
 * @brief           Get the name of the specified task.
 *
 * @param[in]       task            Task control block.
 *
 * @return          The name of the specified task.
 ***********************************************************************************************************************
 */
const char *os_task_name(os_task_t *task)
{
    OS_ASSERT(task != OS_NULL);

    return task->name;
}

os_uint16_t os_task_get_state(os_task_t *task)
{
    OS_ASSERT(task != OS_NULL);

    return task->state;
}

os_uint32_t os_task_get_total_count(void)
{
    os_uint32_t task_count;

    os_spin_lock(&gs_os_task_resource_list_lock);
    task_count = os_list_len(&gs_os_task_resource_list_head);
    OS_ASSERT(0U != task_count);
    os_spin_unlock(&gs_os_task_resource_list_lock);

    return task_count;
}

/**
 ***********************************************************************************************************************
 * @brief           Force the current task to sleep.
 *
 * @details         If the tick is 0, giveup the cpu.
 *
 * @attention       This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       tick            The value of task sleep in tick.
 *
 * @return          The result of forcing current task to sleep.
 * @retval          OS_EOK          Force current task to sleep successfully.
 * @retval          else            Force current task to sleep failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_tsleep(os_tick_t tick)
{
    os_task_t *current_task;

    OS_KERNEL_INIT();

    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());
    OS_ASSERT(tick < (OS_TICK_MAX / 2));

    current_task = k_task_self();
    OS_ASSERT(OS_NULL != current_task);

    OS_KERNEL_ENTER();

    if (tick > 0)
    {
        k_readyq_remove(current_task);
        current_task->state &= ~OS_TASK_STATE_READY;

        current_task->state |= OS_TASK_STATE_SLEEP;
        k_tickq_put(current_task, tick);
    }
    else
    {
        k_readyq_move_tail(current_task);
    }

    OS_KERNEL_EXIT_SCHED();

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Force the current task to sleep. 
 *
 * @details         If the time is 0, giveup the cpu.
 *
 * @attention       This function actually calls os_task_tsleep, so the minimum precision is tick.
 *                  This interface is not allowed in the following cases:
 *                      1. In interrupt context.
 *                      2. Interrupt is disabled.
 *                      3. Scheduler is locked.
 *
 * @param[in]       ms              The value of task sleep in millisecond.
 * 
 * @return          The result of forcing current task to sleep.
 * @retval          OS_EOK          Force current task to sleep successfully.
 * @retval          else            Force current task to sleep failed.
 ***********************************************************************************************************************
 */
os_err_t os_task_msleep(os_uint32_t ms)
{
    os_tick_t tick;
    os_err_t  ret;

    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    tick = os_tick_from_ms(ms);
    ret  = os_task_tsleep(tick);

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will save the last error number.
 *
 * @details         If in the task context, the error number will be saved on the current task. Otherwise, it 
 *                  will be saved on the "gs_os_err_code".
 
 * @param[in]       errno           The error number to be saved.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_set_errno(os_err_t err_code)
{
    os_task_t *current_task;
    os_bool_t  irq_active;

    OS_KERNEL_INIT();

    irq_active = os_is_irq_active();
    
    if (irq_active)
    {
        OS_KERNEL_ENTER();
        gs_os_err_code = err_code;
        OS_KERNEL_EXIT();
    }
    else
    {
        current_task = k_task_self();

        /* The schduler is not start. */
        if (OS_NULL == current_task)
        {
            OS_KERNEL_ENTER();
            gs_os_err_code = err_code;
            OS_KERNEL_EXIT();
        }
        /* The schduler is start and is not in interrupt context */
        else
        {
            current_task->err_code = err_code;
        }
    }
    
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will query the last error number.
 *
 * @details         If in the task context, the error number will be queried on the current task. Otherwise, it 
 *                  will be queried on the "gs_os_errno".
 *
 * @param           None.
 *
 * @return          The error number to be queried.
 ***********************************************************************************************************************
 */
os_err_t os_get_errno(void)
{
    os_task_t *current_task;
    os_bool_t  irq_active;
    os_err_t   err_code;

    irq_active = os_is_irq_active();

    if (irq_active)
    {
        err_code = gs_os_err_code;
    }
    else
    {
        current_task = k_task_self();

        /* The schduler is not start. */
        if (OS_NULL == current_task)
        {
            err_code = gs_os_err_code;
        }
        /* The schduler is start and is not in interrupt context */
        else
        {
            err_code = current_task->err_code;
        }
    }

    return err_code;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return address of variable that save the last error number.
 *
 * @details         If in the task context, get from the current task. Otherwise, get from the "gs_os_errno".
 *
 * @param           None.
 *
 * @return          The address of variable that save the last error number.
 ***********************************************************************************************************************
 */
os_err_t *os_errno(void)
{
    os_task_t *current_task;
    os_err_t  *err_code;
    os_bool_t  irq_active;

    irq_active = os_is_irq_active();

    if (irq_active)
    {
        err_code = &gs_os_err_code;
    }
    else
    {
        current_task = k_task_self();

        /* The schduler is not start. */
        if (OS_NULL == current_task)
        {
            err_code = &gs_os_err_code;
        }
        /* The schduler is start and is not in interrupt context */
        else
        {
            err_code = &current_task->err_code;
        }
    }

    return err_code;
}

#if defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP)
#include <shell.h>

typedef struct
{
    os_task_t *task;
    void      *stack_top;
    void      *stack_begin;
    void      *stack_end;
}sh_task_info_t;

/**
 ***********************************************************************************************************************
 * @brief           Show all tasks info.
 *
 * @param[in]       argc            Command arguments count.
 * @param[in]       argv            Command arguments
 *
 * @return          The state of executting command.
 * @retval          OS_EOK          Execute command success.
 * @retval          else            Execute command failed.
 ***********************************************************************************************************************
 */
os_err_t sh_show_task_info(os_int32_t argc, char **argv)
{
    os_task_t         *iter_task;
    os_task_t         *current_task;
    os_uint16_t        len;
    os_uint32_t        max_used;
    os_uint32_t        stack_size;
    char               name[OS_NAME_MAX + 1];
    
    const static char *state_table[16] = {"Init", "Ready", "Running", "Sleep", "Block", "Suspend",
                                          "",     "",      "",        "",      "",      "",
                                          "",     "",      "",        "Close"};
    char               str_state[20];
    os_uint16_t        int_state;
    os_uint8_t         state_idx;
    
    sh_task_info_t    *task_info;
    os_uint16_t        task_count;
    os_uint16_t        task_index;
    os_err_t           ret;
    
    
    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s  Priority  State                 Stack top   Stack addr  Stack size  Max used  Left tick\r\n",
               OS_NAME_MAX,
               "Task");
    
    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    os_kprintf(     "  --------  --------------------  ----------  ----------  ----------  --------  ---------\r\n");

    os_spin_lock(&gs_os_task_resource_list_lock);
    task_count = os_list_len(&gs_os_task_resource_list_head);
    OS_ASSERT(0U != task_count);
    os_spin_unlock(&gs_os_task_resource_list_lock);

    ret = OS_EOK;

    task_info = (sh_task_info_t *)os_malloc(sizeof(sh_task_info_t) * task_count);
    if (OS_NULL == task_info)
    {
        ret = OS_ENOMEM;
    }
    else
    {
        os_spin_lock(&gs_os_task_resource_list_lock);

        task_index = 0;
        os_list_for_each_entry(iter_task, &gs_os_task_resource_list_head, os_task_t, resource_node)
        {
            task_info[task_index].task        = iter_task;
            task_info[task_index].stack_top   = iter_task->stack_top;
            task_info[task_index].stack_begin = iter_task->stack_begin;
            task_info[task_index].stack_end   = iter_task->stack_end;

            task_index++;
            if (task_index == task_count)
            {
                break;
            }
        }

        os_spin_unlock(&gs_os_task_resource_list_lock);

        if (task_index < task_count)
        {
            task_count = task_index;
        }

        for (task_index = 0; task_index < task_count; task_index++)
        {
            if (OS_KOBJ_INITED != task_info[task_index].task->object_inited)
            {
                continue;
            }

            strncpy(name, task_info[task_index].task->name, OS_NAME_MAX);
            name[OS_NAME_MAX] = '\0';
        
            os_kprintf("%-*s  %-8u",
                       OS_NAME_MAX,
                       (name[0] != '\0') ? name : "-",
                       task_info[task_index].task->current_priority);

            int_state = task_info[task_index].task->state;
            if ((int_state & OS_TASK_STATE_RUNNING) && (int_state & OS_TASK_STATE_READY))
            {
                int_state = int_state & (~OS_TASK_STATE_READY);
            }
            str_state[0] = '\0';
            while (int_state)
            {
                state_idx = os_ffs(int_state) - 1;
                strcat(str_state, state_table[state_idx]);
                int_state = int_state & (~(1<<state_idx));
                if (int_state)
                {
                    strcat(str_state, "|");
                }
            }
            os_kprintf("  %-20s", str_state);
            
            current_task = k_task_self();
            if (task_info[task_index].task == current_task)
            {
                os_kprintf("  0x%p", os_get_current_task_sp());
            }
            else
            {
                os_kprintf("  0x%p", task_info[task_index].stack_top);
            }

            stack_size = (os_uint32_t)((os_ubase_t)task_info[task_index].stack_end - (os_ubase_t)task_info[task_index].stack_begin);
            max_used   = os_hw_stack_max_used(task_info[task_index].stack_begin, stack_size);

            os_kprintf("  0x%p  %-10u  %3u%%      %-9lu\r\n",
                       task_info[task_index].stack_begin,
                       stack_size,
                       (max_used * 100) / stack_size,
                       task_info[task_index].task->remaining_time_slice);
        }

        os_free(task_info);
        task_info = OS_NULL;
    }

    return ret;  
}
SH_CMD_EXPORT(show_task, sh_show_task_info, "Show task information");

#endif /* defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP) */

