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
 * @file        pthread_cond.c
 *
 * @brief       This file provides posix condition functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>
#include "pthread_internal.h"

int pthread_condattr_destroy(pthread_condattr_t *attr)
{
    if (!attr)
    {
        return EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(pthread_condattr_destroy);

int pthread_condattr_init(pthread_condattr_t *attr)
{
    if (!attr)
    {
        return EINVAL;
    }
    
    *attr = PTHREAD_PROCESS_PRIVATE;

    return 0;
}
EXPORT_SYMBOL(pthread_condattr_init);

int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id)
{
    return 0;
}
EXPORT_SYMBOL(pthread_condattr_getclock);

int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id)
{
    return 0;
}
EXPORT_SYMBOL(pthread_condattr_setclock);

int pthread_condattr_getpshared(const pthread_condattr_t *attr, int *pshared)
{
    if (!attr || !pshared)
    {
        return EINVAL;
    }

    *pshared = PTHREAD_PROCESS_PRIVATE;

    return 0;
}
EXPORT_SYMBOL(pthread_condattr_getpshared);

int pthread_condattr_setpshared(pthread_condattr_t*attr, int pshared)
{
    if ((PTHREAD_PROCESS_PRIVATE != pshared) && (PTHREAD_PROCESS_SHARED != pshared))
    {
        return EINVAL;
    }

    if(PTHREAD_PROCESS_PRIVATE != pshared)
    {
        return ENOSYS;
    }
    
    return 0;
}
EXPORT_SYMBOL(pthread_condattr_setpshared);

int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
    os_err_t result;
    char cond_name[OS_NAME_MAX];
    static os_uint16_t cond_num = 0;

    if (OS_NULL == cond)
    {
        return EINVAL;
    }
    
    if ((OS_NULL != attr) && (PTHREAD_PROCESS_PRIVATE != *attr))
    {
        return EINVAL;
    }

    os_snprintf(cond_name, sizeof(cond_name), "cond%02d", cond_num++);

    if (OS_NULL == attr)
    {   
        cond->attr = PTHREAD_PROCESS_PRIVATE;
    }
    else 
    {   
        cond->attr = *attr;
    }

    result = os_sem_init(&cond->sem, cond_name, 0, 1);
    if (OS_EOK != result)
    {
        return EINVAL;
    }

    /* Detach the object from system object container. */
    /* os_object_deinit(&cond->sem.parent.parent); */
    /* cond->sem.parent.parent.type = OS_OBJECT_SEMAPHORE; */

    return OS_EOK;
}
EXPORT_SYMBOL(pthread_cond_init);

int pthread_cond_destroy(pthread_cond_t *cond)
{
    if (OS_NULL == cond)
    {
        return EINVAL;
    }
    
    if (-1 == cond->attr)
    {
        return OS_EOK;
    }

    (void)os_sem_deinit(&cond->sem);

    /* Clean condition. */
    memset(cond, 0, sizeof(pthread_cond_t));
    cond->attr = -1;

    return OS_EOK;
}
EXPORT_SYMBOL(pthread_cond_destroy);

int pthread_cond_broadcast(pthread_cond_t *cond)
{
    os_err_t result;

    if (OS_NULL == cond)
    {
        return EINVAL;
    }
    
    if (-1 == cond->attr)
    {
        pthread_cond_init(cond, OS_NULL);
    }

    os_schedule_lock();
    
    while(1)
    {
        /* Try to take condition semaphore. */
        result = os_sem_wait(&cond->sem, OS_NO_WAIT);
        if (OS_EBUSY == result)
        {
            /* It's timeout, release this semaphore. */
            os_sem_post(&cond->sem);
        }
        else if (OS_EOK == result)
        {
            break;
        }
        else
        {
            os_schedule_unlock();

            return EINVAL;
        }
    }
    
    os_schedule_unlock();

    return OS_EOK;
}
EXPORT_SYMBOL(pthread_cond_broadcast);

int pthread_cond_signal(pthread_cond_t *cond)
{
    os_err_t result;

    if (OS_NULL == cond)
    {
        return EINVAL;
    }
    
    if (-1 == cond->attr)
    {
        pthread_cond_init(cond, OS_NULL);
    }

    result = os_sem_post(&cond->sem);
    if (OS_EOK == result)
    {
        return OS_EOK;
    }

    return OS_EOK;
}
EXPORT_SYMBOL(pthread_cond_signal);

static os_err_t _pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, os_int32_t timeout)
{
    os_err_t result;

    if (!cond || !mutex)
    {
        return OS_ERROR;
    }
    
    /* Check whether initialized */
    if (-1 == cond->attr)
    {
        pthread_cond_init(cond, OS_NULL);
    }

    /* The mutex was not owned by the current thread at the time of the call. */
    if (os_mutex_get_owner(&mutex->lock) != os_task_self())
    {
        return OS_ERROR;
    }
    
    /* Unlock a mutex failed */
    if (0 != pthread_mutex_unlock(mutex))
    {
        return OS_ERROR;
    }

    result = os_sem_wait(&cond->sem, timeout);
    
    /* Lock mutex again */
    pthread_mutex_lock(mutex);

    return result;
}

int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    os_err_t result;

    result = _pthread_cond_timedwait(cond, mutex, OS_WAIT_FOREVER);
    if (OS_EOK == result)
    {
        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_cond_wait);

int pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    int      timeout;
    os_err_t result;

    timeout = clock_time_to_tick(abstime);
    
    result = _pthread_cond_timedwait(cond, mutex, timeout);
    if (OS_EOK == result)
    {
        return 0;
    }
    
    if ((OS_ETIMEOUT == result) || (OS_EBUSY == result))
    {
        return ETIMEDOUT;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_cond_timedwait);

