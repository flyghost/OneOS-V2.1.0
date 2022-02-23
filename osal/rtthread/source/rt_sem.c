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
 * @file        rt_sem.c
 *
 * @brief       Implementation of RT-Thread adaper semaphore function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "os_assert.h"
#include "rtthread.h"
#include "os_errno.h"

#ifdef RT_USING_SEMAPHORE
rt_err_t rt_sem_init(rt_sem_t    sem,
                     const char *name,
                     rt_uint32_t value,
                     rt_uint8_t  flag)
{
    os_err_t ret;

    OS_ASSERT(sem);
    OS_ASSERT(value < OS_SEM_MAX_VALUE);

    ret = os_sem_init(&sem->os_sem, name, (os_uint32_t)value, OS_SEM_MAX_VALUE);
    if (OS_EOK != ret)
    {
         return -RT_ERROR;
    }

    if (RT_IPC_FLAG_PRIO == flag)
    {
        ret = os_sem_set_wake_type(&sem->os_sem, OS_SEM_WAKE_TYPE_PRIO);
    }
    else
    {
        ret = os_sem_set_wake_type(&sem->os_sem, OS_SEM_WAKE_TYPE_FIFO);
    }
    if (OS_EOK != ret)
    {
         return -RT_ERROR;
    }

    sem->is_static = OS_TRUE;

    return RT_EOK;
}

rt_err_t rt_sem_detach(rt_sem_t sem)
{
    os_err_t ret;

    OS_ASSERT(sem);
    OS_ASSERT(OS_TRUE == sem->is_static);

    ret = os_sem_deinit(&sem->os_sem);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

#ifdef RT_USING_HEAP
rt_sem_t rt_sem_create(const char *name, rt_uint32_t value, rt_uint8_t flag)
{
    rt_sem_t sem;
    os_err_t ret;

    OS_ASSERT(value < OS_SEM_MAX_VALUE);

    sem = (rt_sem_t)os_malloc(sizeof(struct rt_semaphore));
    if(OS_NULL == sem)
    {
       return RT_NULL;
    }

    ret = os_sem_init(&sem->os_sem, name, (os_uint16_t)value, OS_SEM_MAX_VALUE);
    if (OS_EOK != ret)
    {
        os_free(sem);
        return RT_NULL;
    }

    if (RT_IPC_FLAG_PRIO == flag)
    {
       ret = os_sem_set_wake_type(&sem->os_sem, OS_SEM_WAKE_TYPE_PRIO);
    }
    else
    {
       ret = os_sem_set_wake_type(&sem->os_sem, OS_SEM_WAKE_TYPE_FIFO);
    }
    if (OS_EOK != ret)
    {
        return RT_NULL;
    }

    sem->is_static = OS_FALSE;

    return sem;
}

rt_err_t rt_sem_delete(rt_sem_t sem)
{
    os_err_t ret;
    OS_ASSERT(sem);
    OS_ASSERT(OS_FALSE == sem->is_static);

    ret = os_sem_deinit(&sem->os_sem);
    os_free(sem);
    if (OS_EOK != ret)
    {
         return -RT_ERROR;
    }

    return RT_EOK;
}
#endif  /* RT_USING_HEAP */

rt_err_t rt_sem_take(rt_sem_t sem, rt_int32_t time)
{
    os_tick_t timeout;
    os_err_t  ret;

    OS_ASSERT(sem);

    /*For OneOS,only support -1 for timeout,so set timeout is -1 when timeout is less than zero*/
    if (time < 0)
    {
        timeout = OS_WAIT_FOREVER;
    }
    else if (0 == time)
    {
        timeout = OS_NO_WAIT;
    }
    else
    {
        timeout = (os_tick_t)time;
    }

    ret = os_sem_wait(&sem->os_sem, timeout);
    if (OS_EOK != ret)
    {
        if (OS_EBUSY == ret || OS_ETIMEOUT == ret)
        {
            return -RT_ETIMEOUT;
        }

        return -RT_ERROR;
    }

    return RT_EOK;
}

rt_err_t rt_sem_trytake(rt_sem_t sem)
{
    return rt_sem_take(sem, 0);
}

rt_err_t rt_sem_release(rt_sem_t sem)
{
    os_err_t ret;

    OS_ASSERT(sem);

    ret = os_sem_post(&sem->os_sem);
    if (OS_EOK != ret)
    {
         return -RT_ERROR;
    }
    return RT_EOK;
}
#endif /* RT_USING_SEMAPHORE */

