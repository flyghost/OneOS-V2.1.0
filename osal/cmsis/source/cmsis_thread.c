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
 * @file        cmsis_thread.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 thread function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-01   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_errno.h>
#include <os_util.h>
#include <os_task.h>
#include <string.h>
#include <arch_interrupt.h>
#include <os_timer.h>
#include <os_clock.h>

#include "cmsis_internal.h"

#define MALLOC_STACK                        0x04
#define CMSIS_DEFAULT_TICK                  5
#define CMSIS_DEFAULT_STACK_SIZE            1024

#define WAITING_THREAD_FLAGS                0x08

static os_list_node_t gs_os_thread_adapt_resource_list_head = OS_LIST_INIT(gs_os_thread_adapt_resource_list_head);

static void thread_cleanup(void *user_data)
{
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)(user_data);

    os_list_del(&thread_cb->resource_node);
    os_timer_deinit(&thread_cb->timer);

    if (thread_cb->flags&osThreadJoinable)
    {
        os_sem_post(thread_cb->joinable_sem);
    }
    else
    {
        thread_cb->id = IdInvalid;
        if (thread_cb->flags&MALLOC_STACK)
        {
            os_free(thread_cb->stack_start);
            thread_cb->stack_start = OS_NULL;
        }

        if (thread_cb->flags&SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
            thread_cb = OS_NULL;
        }
    }
}

static void thread_timeout(void *parameter)
{
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)parameter;

    /* Set error number. */
    thread_cb->error = OS_ETIMEOUT;
    os_task_resume(&thread_cb->task);
}


/**
 ***********************************************************************************************************************
 * @brief           The function osThreadNew starts a thread function by adding it to the list of active threads and sets it to state READY.
 *                  Arguments for the thread function are passed using the parameter pointer *argument. When the priority of the created
 *                  thread function is higher than the current RUNNING thread, the created thread function starts instantly and becomes
 *                  the new RUNNING thread. Thread attributes are defined with the parameter pointer attr. Attributes include settings for
 *                  thread priority, stack size, or memory allocation.
 *
 * @attention       CMSIS-RTOS APIs define 56 priorities and the priority increases as the number increases,but OneOS does the opposite.in
 *                  addition to this,OneOS can config the number of priority,so this function use an algorithm to match the priority with
 *                  OneOS.
 *
 * @param[in]       func            thread function
 * @param[in]       argument        pointer that is passed to the thread function as start argument.
 * @param[in]       attr            thread attributes; NULL: default values.
 *
 * @return          thread ID for reference by other functions or NULL in case of error.
 ***********************************************************************************************************************
 */
osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
    char                name[OS_NAME_MAX];
    void               *stack;
    os_uint8_t          os_task_prio;
    os_uint32_t         stack_size;
    thread_cb_t        *thread_cb;
    static os_uint16_t  thread_number = 1U;
    os_err_t ret;

    if (OS_NULL == func)
    {
        return (osThreadId_t)OS_NULL;
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        thread_cb = os_malloc(sizeof(thread_cb_t));
        if (OS_NULL == thread_cb)
        {
            return (osThreadId_t)OS_NULL;
        }
        memset(thread_cb, 0, sizeof(thread_cb_t));
        thread_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(thread_cb_t))
        {
            thread_cb = (thread_cb_t *)attr->cb_mem;
            thread_cb->flags = 0;
        }
        else
        {
            return (osThreadId_t)OS_NULL;
        }
    }

    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "th%02d", thread_number);
        thread_number++;
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (osPriorityNone == attr->priority))
    {
        thread_cb->prio = osPriorityNormal;
    }
    else
    {
        if ((attr->priority < osPriorityIdle) || (attr->priority > osPriorityISR))
        {
            return (osThreadId_t)OS_NULL;
        }

        thread_cb->prio = attr->priority;
    }
    if ((OS_NULL == attr) || (0U == attr->stack_size))
    {
        stack_size = CMSIS_DEFAULT_STACK_SIZE;
    }
    else
    {
        stack_size = attr->stack_size;
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->stack_mem))
    {
        stack = os_malloc(stack_size);
        if (OS_NULL == stack)
        {
            if (thread_cb->flags & SYS_MALLOC_CTRL_BLK)
            {
                os_free(thread_cb);
            }
            return (osThreadId_t)OS_NULL;
        }
        thread_cb->stack_start = stack;
        thread_cb->flags |= MALLOC_STACK;
    }
    else
    {
        stack = (void *)(attr->stack_mem);
    }

    if ((OS_NULL != attr)&&(0 != attr->attr_bits))
    {
        thread_cb->flags |= attr->attr_bits;
    }

    /* Algorithm match the priorities CMSIS RTOS defined with OneOS */
    os_task_prio = (osPriorityISR - thread_cb->prio) * OS_TASK_PRIORITY_MAX / osPriorityISR;

    ret = os_task_init(&thread_cb->task,
                  name,
                  func,
                  argument,
                  stack,
                  stack_size,
                  os_task_prio);
    if (OS_EOK != ret)
    {
        if (thread_cb->flags & SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
        }
        if (thread_cb->flags & MALLOC_STACK)
        {
            os_free(stack);
        }
        return (osThreadId_t)OS_NULL;
    }

    if (thread_cb->flags&osThreadJoinable)
    {
        thread_cb->joinable_sem = os_sem_create(name, 0, 1);
        if (OS_NULL == thread_cb->joinable_sem)
        {
            if (thread_cb->flags & SYS_MALLOC_CTRL_BLK)
            {
                os_free(thread_cb);
            }
            if (thread_cb->flags & MALLOC_STACK)
            {
                os_free(stack);
            }

            /* After os_task_init,task has been inserted into task queue */
            os_task_deinit(&thread_cb->task);
            return (osThreadId_t)OS_NULL;
        }
    }
    else
    {
        thread_cb->joinable_sem = OS_NULL;
    }

    os_task_set_cleanup_callback(&thread_cb->task, thread_cleanup, thread_cb);

    os_timer_init(&thread_cb->timer, name, thread_timeout, (void *)thread_cb, 0, OS_TIMER_FLAG_ONE_SHOT);

    os_schedule_lock();
    os_list_add_tail(&gs_os_thread_adapt_resource_list_head, &thread_cb->resource_node);
    os_schedule_unlock();
    thread_cb->id = IdThread;

    ret = os_task_startup(&thread_cb->task);
    if (OS_EOK != ret)
    {
        if (thread_cb->flags & SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
        }
        if (thread_cb->flags & MALLOC_STACK)
        {
            os_free(stack);
        }

        return (osThreadId_t)OS_NULL;
    }
    
    return (osThreadId_t)thread_cb;
}

const char *osThreadGetName(osThreadId_t thread_id)
{
    thread_cb_t *thread_cb;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return NULL;
    }

    return os_task_name(&thread_cb->task);
}

osThreadId_t osThreadGetId(void)
{
    os_task_t *os_task;
    thread_cb_t *thread_cb;

    os_task = os_task_self();
    thread_cb = os_list_entry(os_task, struct thread_control_block, task);

    return (osThreadId_t)thread_cb;
}

osThreadState_t osThreadGetState(osThreadId_t thread_id)
{
    thread_cb_t    *thread_cb;
    osThreadState_t state;
    os_uint16_t     os_state;

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id) || (OS_TRUE == os_is_irq_active()))
    {
        return osThreadError;
    }

    os_state = os_task_get_state(&thread_cb->task);

    if (os_state & OS_TASK_STATE_INIT)
    {
        state = osThreadInactive;
    }
    else if (os_state & OS_TASK_STATE_RUNNING)
    {
        state =  osThreadRunning;
    }
    else if (os_state & OS_TASK_STATE_READY)
    {
        state = osThreadReady;
    }
    else if (os_state & OS_TASK_STATE_BLOCK)
    {
        state = osThreadBlocked;
    }
    else if (os_state & OS_TASK_STATE_SUSPEND)
    {
        state = osThreadBlocked;
    }
    else if (os_state & OS_TASK_STATE_SLEEP)
    {
        state = osThreadBlocked;
    }
    else if (os_state & OS_TASK_STATE_CLOSE)
    {
        state = osThreadTerminated;
    }
    else
    {
        state = osThreadError;
    }

    return state;
}

osStatus_t osThreadSetPriority(osThreadId_t thread_id, osPriority_t priority)
{
    os_uint8_t   oneos_priority;
    thread_cb_t *thread_cb;
    os_err_t ret;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    thread_cb = (thread_cb_t *)thread_id;

    if((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return osErrorParameter;
    }

    if((priority < osPriorityNone) || (priority > osPriorityISR))
    {
        return osErrorParameter;
    }

    thread_cb->prio = priority;

    oneos_priority = (osPriorityISR - thread_cb->prio) * OS_TASK_PRIORITY_MAX / osPriorityISR;

    ret = os_task_set_priority(&thread_cb->task, oneos_priority);
    if (OS_EOK == ret)
    {
        return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

osPriority_t osThreadGetPriority(osThreadId_t thread_id)
{
    thread_cb_t *thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id) || (OS_TRUE == os_is_irq_active()))
    {
        return osPriorityError;
    }

    return (osPriority_t)thread_cb->prio;
}

osStatus_t osThreadYield(void)
{
    os_err_t ret;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    ret = os_task_yield();
    if (OS_EOK == ret)
    {
        return osOK;
    }
    else
    {
        return osError;
    }
}

osStatus_t osThreadSuspend(osThreadId_t thread_id)
{
    os_err_t     result;
    thread_cb_t *thread_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return osErrorParameter;
    }

    result = os_task_suspend(&thread_cb->task);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

osStatus_t osThreadResume(osThreadId_t thread_id)
{
    os_err_t     result;
    thread_cb_t *thread_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return osErrorParameter;
    }

    result = os_task_resume(&thread_cb->task);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

osStatus_t osThreadDetach(osThreadId_t thread_id)
{
    thread_cb_t       *thread_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return osErrorParameter;
    }

    if ((thread_cb->flags&osThreadJoinable) == 0U)
    {
        return osErrorResource;
    }

    if (osThreadTerminated == osThreadGetState(thread_id))
    {
        /* Release system resource */
        thread_cb->id = IdInvalid;
        thread_cb->flags &= ~osThreadJoinable;
        os_sem_destroy(thread_cb->joinable_sem);

        if (thread_cb->flags&MALLOC_STACK)
        {
            os_free(thread_cb->stack_start);
        }

        if (thread_cb->flags&SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
        }
    }
    else
    {
        os_schedule_lock();
        /* Set task attribute to osThreadDetached */
        thread_cb->flags &= ~osThreadJoinable;
        os_schedule_unlock();
    }

    return osOK;
}

osStatus_t osThreadJoin(osThreadId_t thread_id)
{
    os_err_t     result;
    thread_cb_t *thread_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return osErrorParameter;
    }
    if (((&thread_cb->task) == os_task_self()) ||
         (0 == (thread_cb->flags & osThreadJoinable)))
    {
        /* Join self or join a detached thread*/
        return osErrorResource;
    }

    result = os_sem_wait(thread_cb->joinable_sem, OS_WAIT_FOREVER);
    if (OS_EOK == result)
    {
        thread_cb->id = IdInvalid;
        /* Release system resource */
        if (thread_cb->flags&osThreadJoinable)
        {
            os_sem_destroy(thread_cb->joinable_sem);
        }

        if (thread_cb->flags&MALLOC_STACK)
        {
            os_free(thread_cb->stack_start);
        }

        if (thread_cb->flags&SYS_MALLOC_CTRL_BLK)
        {
            os_free(thread_cb);
        }
    }
    else
    {
        return osError;
    }

    return osOK;
}

__NO_RETURN void osThreadExit(void)
{
    os_task_t *os_task;

    os_task = os_task_self();
    os_task_deinit(os_task);

#ifdef OS_USING_ASSERT
    /* Monitor whether the mission is successfully closed or not,if not OS_ASSERT() will detect it*/
    OS_ASSERT(0);
#else
    /*Because OS_ASSERT() can be turned off by configrable options ,use while(1) to monitor*/
    while(1);
#endif
}

osStatus_t osThreadTerminate(osThreadId_t thread_id)
{
    os_err_t ret;
    thread_cb_t *thread_cb ;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    thread_cb = (thread_cb_t *)thread_id;

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return osErrorParameter;
    }

    ret = os_task_deinit(&thread_cb->task);
    if (OS_EOK == ret)
    {
         return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Number of active threads.
 *
 * @attention       This function only return task osThreadNew create number
 *
 * @param[in]       None
 *
 * @return          The number of active threads or 0 in case of an error.
 ***********************************************************************************************************************
 */
uint32_t osThreadGetCount(void)
{
    os_uint32_t count;

    os_schedule_lock();
    count = os_list_len(&gs_os_thread_adapt_resource_list_head);
    os_schedule_unlock();

    return count;
}

/**
 ***********************************************************************************************************************
 * @brief           Enumerate active threads.
 *
 * @attention       This function only return task ID osThreadNew create
 *
 * @param[out]      thread_array       Pointer to array for retrieving thread IDs
 * @param[in]       array_items        Maximum number of items in array for retrieving thread IDs.
 *
 * @return          Number of enumerated threads.
 ***********************************************************************************************************************
 */
uint32_t osThreadEnumerate(osThreadId_t *thread_array, uint32_t array_items)
{
    thread_cb_t           *iter_thread_cb;
    os_uint32_t            thread_count = 0U;
    os_list_node_t *pos;

    if ((OS_NULL == thread_array) || (0U == array_items))
    {
        return 0U;
    }

    os_schedule_lock();
    os_list_for_each(pos, &gs_os_thread_adapt_resource_list_head)
    {
        iter_thread_cb = os_list_entry(pos, struct thread_control_block, resource_node);
        if (OS_NULL != iter_thread_cb)
        {
            thread_array[thread_count] = (osThreadId_t)iter_thread_cb;
            thread_count++;
        }

        if (thread_count >= array_items)
        {
            break;
        }
    }
    os_schedule_unlock();

    return thread_count;
}

uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    thread_cb_t        *thread_cb;
    uint32_t            return_value;
    register os_base_t  status;
    register os_ubase_t level;

    thread_cb = (thread_cb_t *)(thread_id);

    if ((OS_NULL == thread_cb) || (IdThread != thread_cb->id))
    {
        return osFlagsErrorParameter;
    }

    /* Check flag value to avoid highest bits set */
    if((flags&(~(OS_UINT32_MAX >> 1U))) != 0)
    {
        return osFlagsErrorParameter;
    }

    level = os_irq_lock();

    thread_cb->thread_flags |= flags;

    /* Check if Thread is waiting for Thread Flags */
    if (thread_cb->flags_options&WAITING_THREAD_FLAGS)
    {
        status = OS_ERROR;
        if (thread_cb->flags_options & osFlagsWaitAll)
        {
            if ((thread_cb->wait_flags&thread_cb->thread_flags) == thread_cb->wait_flags)
            {
                status = OS_EOK;
            }
        }
        else
        {
            if (thread_cb->wait_flags&thread_cb->thread_flags)
            {
                thread_cb->wait_flags &= thread_cb->thread_flags;
                status = OS_EOK;
            }
        }

        /* Condition is satisfied, resume thread */
        if (OS_EOK == status)
        {
            thread_cb->flags_options &= ~WAITING_THREAD_FLAGS;

            if (!(thread_cb->flags_options&osFlagsNoClear))
            {
                thread_cb->thread_flags &= ~thread_cb->wait_flags;
            }
            os_timer_stop(&thread_cb->timer);
            os_task_resume(&thread_cb->task);
        }
    }

    return_value = thread_cb->thread_flags;

    os_irq_unlock(level);

    return return_value;
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    os_uint32_t  flag;
    os_task_t *os_task;
    thread_cb_t *thread_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  (uint32_t)osErrorISR;
    }

    os_task = os_task_self();

    if (OS_NULL == os_task)
    {
        return osFlagsErrorParameter;
    }

    /* Check flag value to avoid highest bits set */
    if((flags&(~(OS_UINT32_MAX >> 1U))) != 0)
    {
        return osFlagsErrorParameter;
    }


    os_schedule_lock();
    thread_cb = os_list_entry(os_task, struct thread_control_block, task);
    flag = thread_cb->thread_flags;
    thread_cb->thread_flags &= ~flags;
    os_schedule_unlock();

    return flag;
}

uint32_t osThreadFlagsGet(void)
{
    os_task_t *os_task;
    thread_cb_t *thread_cb;

    os_task = os_task_self();

    if (OS_NULL == os_task)
    {
        return osFlagsErrorParameter;
    }

    thread_cb = os_list_entry(os_task, struct thread_control_block, task);

    return thread_cb->thread_flags;
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
    os_task_t          *os_task;
    uint32_t         return_value;
    thread_cb_t        *thread_cb;
    register os_ubase_t level;
    register os_base_t  status = OS_ERROR;

    if (OS_TRUE == os_is_irq_active())
    {
        return  (uint32_t)osErrorISR;
    }

    os_task = os_task_self();
    thread_cb = os_list_entry(os_task, struct thread_control_block, task);
    thread_cb->error = OS_EOK;

    level = os_irq_lock();

    if (options&osFlagsWaitAll)
    {
        if ((thread_cb->thread_flags & flags) == flags)
        {
            status = OS_EOK;
        }
    }
    else
    {
        if (thread_cb->thread_flags & flags)
        {
            status = OS_EOK;
        }
    }

    if (OS_EOK == status)
    {
        return_value = thread_cb->thread_flags&flags;
        if (!(options & osFlagsNoClear))
        {
            thread_cb->thread_flags &= ~flags;
        }
    }
    else if (0U == timeout)
    {
        os_irq_unlock(level);
        return osFlagsErrorResource;
    }
    else
    {
        thread_cb->wait_flags  = flags;
        thread_cb->flags_options = options | WAITING_THREAD_FLAGS;

        if ((timeout > 0U) && (timeout != osWaitForever))
        {
           os_timer_set_timeout_ticks(&thread_cb->timer, timeout);
           os_timer_start(&thread_cb->timer);
        }
        os_irq_unlock(level);
        os_task_suspend(&thread_cb->task);

        if (thread_cb->error != OS_EOK)
        {
            return (uint32_t)thread_cb->error;
        }

        level = os_irq_lock();
        return_value = thread_cb->thread_flags;
    }

    os_irq_unlock(level);

    return return_value;
}

