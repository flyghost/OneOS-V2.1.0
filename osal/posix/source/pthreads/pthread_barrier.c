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
 * @file        pthread_barrier.c
 *
 * @brief       This file provides posix barrier functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>

int pthread_barrierattr_destroy(pthread_barrierattr_t *attr)
{
    if (!attr)
    {
        return EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(pthread_barrierattr_destroy);

int pthread_barrierattr_init(pthread_barrierattr_t *attr)
{
    if (!attr)
    {
        return EINVAL;
    }
    
    *attr = PTHREAD_PROCESS_PRIVATE;

    return 0;
}
EXPORT_SYMBOL(pthread_barrierattr_init);

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *attr, int *pshared)
{
    if (!attr)
    {
        return EINVAL;
    }
    
    *pshared = (int)*attr;

    return 0;
}
EXPORT_SYMBOL(pthread_barrierattr_getpshared);

int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared)
{
    if (!attr)
    {
        return EINVAL;
    }
    
    if (PTHREAD_PROCESS_PRIVATE == pshared)
    {
        attr = PTHREAD_PROCESS_PRIVATE;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_barrierattr_setpshared);

int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    os_err_t result;

    if (!barrier)
    {
        return EINVAL;
    }

    result = pthread_cond_destroy(&barrier->cond);

    return result;
}
EXPORT_SYMBOL(pthread_barrier_destroy);

int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count)
{
    if (!barrier)
    {
        return EINVAL;
    }
    
    if (attr && (PTHREAD_PROCESS_PRIVATE != *attr))
    {
        return EINVAL;
    }

    barrier->count = count;
    pthread_cond_init(&barrier->cond, NULL);
    pthread_mutex_init(&barrier->mutex, NULL);

    return 0;
}
EXPORT_SYMBOL(pthread_barrier_init);

int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    os_err_t result;
    
    if (!barrier)
    {
        return EINVAL;
    }

    result = pthread_mutex_lock(&barrier->mutex);
    if (0 != result)
    {
        return EINVAL;
    }

    if (0 == barrier->count)
    {
        result = EINVAL;
    }
    else
    {
        barrier->count -= 1;

        /* Broadcast condition. */
        if (0 == barrier->count)
        {
            pthread_cond_broadcast(&barrier->cond);
        }
        else
        {
            pthread_cond_wait(&barrier->cond, &barrier->mutex);
        }
    }

    pthread_mutex_unlock(&barrier->mutex);

    return result;
}
EXPORT_SYMBOL(pthread_barrier_wait);

