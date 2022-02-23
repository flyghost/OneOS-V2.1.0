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
 * @file        rt_thread.c
 *
 * @brief       Implementation of RT-Thread adaper thread function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-19   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "rtthread.h"
#include "os_task.h"
#include "os_assert.h"
#include "os_memory.h"
#include "os_errno.h"

static void rt_thread_cleanup(void *user_data)
{
    rt_thread_t rt_thread;

    rt_thread = (rt_thread_t)(user_data);
    
    if(OS_FALSE == rt_thread->is_static)
    {
        os_free(rt_thread->stack_begin);
        os_free(rt_thread);
    }
}

rt_err_t rt_thread_init(struct rt_thread *thread,
                        const char       *name,
                        void            (*entry)(void *parameter),
                        void             *parameter,
                        void             *stack_start,
                        rt_uint32_t       stack_size,
                        rt_uint8_t        priority,
                        rt_uint32_t       tick)
{
    os_err_t ret;

    ret = os_task_init(&thread->os_task,
                       name, 
                       entry, 
                       parameter,
                       stack_start, 
                       stack_size, 
                       priority);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    ret = os_task_set_time_slice(&thread->os_task, tick);
    if (OS_EOK != ret)
    {
         return -RT_ERROR;
    }

    thread->is_static = OS_TRUE;

    return RT_EOK;
}

rt_err_t rt_thread_detach(rt_thread_t thread)
{
    os_err_t ret;

    OS_ASSERT(thread);
    OS_ASSERT(OS_TRUE == thread->is_static);

    ret = os_task_deinit(&thread->os_task);
    if (OS_EOK != ret)
    {
           return -RT_ERROR;
    }

    return RT_EOK;
}

#ifdef RT_USING_HEAP
rt_thread_t rt_thread_create(const char *name,
                             void      (*entry)(void *parameter),
                             void       *parameter,
                             rt_uint32_t stack_size,
                             rt_uint8_t  priority,
                             rt_uint32_t tick)
{
    void       *stack_begin;
    os_err_t    ret;
    rt_thread_t thread;

    thread = (rt_thread_t)os_malloc(sizeof(struct rt_thread));
    if (OS_NULL == thread)
    {
        return RT_NULL;
    }

    stack_size  = OS_ALIGN_UP(stack_size, OS_ALIGN_SIZE);
    stack_begin = os_malloc(stack_size);
    if (OS_NULL == stack_begin)
    {
        os_free(thread);
        return RT_NULL;
    }
    
    ret = os_task_init(&thread->os_task, name, entry, parameter, stack_begin, stack_size, priority);
    if (OS_EOK != ret)
    {
        os_free(stack_begin);
        os_free(thread);

        return RT_NULL;
    }

    ret = os_task_set_time_slice(&(thread->os_task), tick);
    if (OS_EOK != ret)
    {
         return RT_NULL;
    }
    /* To achieve dynamic memory release */
    os_task_set_cleanup_callback(&thread->os_task, rt_thread_cleanup, thread);

    thread->is_static = OS_FALSE;
    thread->stack_begin = stack_begin;

    return thread;
}

rt_err_t rt_thread_delete(rt_thread_t thread)
{
    os_err_t    ret;

    OS_ASSERT(thread);
    OS_ASSERT(OS_FALSE == thread->is_static);

    ret = os_task_deinit(&(thread->os_task));
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}
#endif  /* RT_USING_HEAP */

rt_thread_t rt_thread_self(void)
{
    rt_thread_t  thread;
    os_task_t   *os_task_ptr;

    os_task_ptr = os_task_self();
    thread   = os_container_of(os_task_ptr, struct rt_thread, os_task);

    return thread;
}

rt_thread_t rt_thread_find(char *name)
{
    rt_thread_t  thread;
    os_task_t   *os_task_ptr;

    OS_ASSERT(name);

    os_task_ptr = os_task_find(name);
    thread   = os_container_of(os_task_ptr, struct rt_thread, os_task);

    return thread;
}

rt_err_t rt_thread_startup(rt_thread_t thread)
{
    os_err_t ret;

    ret = os_task_startup(&thread->os_task);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_thread_yield(void)
{
    os_err_t ret;
    
    ret = os_task_yield();
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_thread_delay(rt_tick_t tick)
{
    os_err_t ret;
    
    ret = os_task_tsleep((os_tick_t)tick);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_thread_mdelay(rt_int32_t ms)
{
    os_err_t ret;

    ret = os_task_msleep((os_uint32_t)ms);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_thread_control(rt_thread_t thread, int cmd, void *arg)
{
    os_err_t ret;
    
    switch (cmd)
    {
    case RT_THREAD_CTRL_CHANGE_PRIORITY:
        ret = os_task_set_priority(&(thread->os_task), *((os_uint8_t *)arg));
        break;
    case RT_THREAD_CTRL_STARTUP:
        return rt_thread_startup(thread);
#ifdef RT_USING_HEAP
    case RT_THREAD_CTRL_CLOSE:
        return rt_thread_delete(thread);
#endif
    default:
        break;
    }

    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_thread_suspend(rt_thread_t thread)
{
    os_err_t ret;
    
    ret = os_task_suspend(&thread->os_task);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_thread_resume(rt_thread_t thread)
{
    os_err_t ret;

    ret = os_task_resume(&thread->os_task);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

void rt_enter_critical(void)
{
    os_schedule_lock();
}

void rt_exit_critical(void)
{
    os_schedule_unlock();
}

rt_uint16_t rt_critical_level(void)
{
    return (rt_uint16_t)os_is_schedule_locked();
}
