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
 * @file        pthread_rwlock.c
 *
 * @brief       This file provides posix rwlock functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
    if (!attr)
    {
        return EINVAL;
    }
    
    *attr = PTHREAD_PROCESS_PRIVATE;

    return 0;
}
EXPORT_SYMBOL(pthread_rwlockattr_init);

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
    if (!attr)
    {
        return EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(pthread_rwlockattr_destroy);

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)
{
    if (!attr || !pshared)
    {
        return EINVAL;
    }

    *pshared = PTHREAD_PROCESS_PRIVATE;

    return 0;
}
EXPORT_SYMBOL(pthread_rwlockattr_getpshared);

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
    if (!attr || (PTHREAD_PROCESS_PRIVATE != pshared))
    {
        return EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(pthread_rwlockattr_setpshared);

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
    if (!rwlock)
    {
        return EINVAL;
    }

    rwlock->attr = PTHREAD_PROCESS_PRIVATE;
    
    pthread_mutex_init(&rwlock->rw_mutex, NULL);
    pthread_cond_init(&rwlock->rw_condreaders, NULL);
    pthread_cond_init(&rwlock->rw_condwriters, NULL);
    
    rwlock->rw_nwaitwriters = 0;
    rwlock->rw_nwaitreaders = 0;
    rwlock->rw_refcount = 0;

    return 0;
}
EXPORT_SYMBOL(pthread_rwlock_init);

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }

    /* Rwlock is not initialized. */
    if (-1 == rwlock->attr)
    {
        return 0; 
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    if ((0 != rwlock->rw_refcount) || (0 != rwlock->rw_nwaitreaders) || (0 != rwlock->rw_nwaitwriters))
    {
        result = EBUSY;

        return result;
    }
    else
    {
        pthread_cond_destroy(&rwlock->rw_condreaders);
        pthread_cond_destroy(&rwlock->rw_condwriters);
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);
    
    pthread_mutex_destroy(&rwlock->rw_mutex);

    memset(rwlock, 0, sizeof(pthread_rwlock_t));
    rwlock->attr = -1;

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_destroy);

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    
    if (-1 == rwlock->attr)
    {
        pthread_rwlock_init(rwlock, NULL);
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    /* Give preference to waiting writers */
    while ((rwlock->rw_refcount < 0) || (rwlock->rw_nwaitwriters > 0))
    {
        rwlock->rw_nwaitreaders++;
        
        /* rw_mutex will be released when waiting for rw_condreaders. */
        result = pthread_cond_wait(&rwlock->rw_condreaders, &rwlock->rw_mutex);
        
        /* rw_mutex should have been taken again when returned from waiting. */
        rwlock->rw_nwaitreaders--;

        /* Wait error. */
        if (0 != result)
        {
            break;
        }
    }

    /* Another reader has a read lock. */
    if (0 == result)
    {
        rwlock->rw_refcount++;
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_rdlock);

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    
    if (-1 == rwlock->attr)
    {
        pthread_rwlock_init(rwlock, NULL);
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    /* Held by a writer or waiting writers */
    if ((rwlock->rw_refcount < 0) || (rwlock->rw_nwaitwriters > 0))
    {
        result = EBUSY;
    }
    /* Increment count of reader locks. */
    else
    {
        rwlock->rw_refcount++;
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_tryrdlock);

int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwlock, const struct timespec *abstime)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    
    if (-1 == rwlock->attr)
    {
        pthread_rwlock_init(rwlock, NULL);
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    /* Give preference to waiting writers */
    while ((rwlock->rw_refcount < 0) || (rwlock->rw_nwaitwriters > 0))
    {
        rwlock->rw_nwaitreaders++;
        
        /* rw_mutex will be released when waiting for rw_condreaders. */
        result = pthread_cond_timedwait(&rwlock->rw_condreaders, &rwlock->rw_mutex, abstime);
        
        /* rw_mutex should have been taken again when returned from waiting. */
        rwlock->rw_nwaitreaders--;
        
        if (0 != result)
        {
            break;
        }
    }

    /* Another reader has a read lock */
    if (0 == result)
    {
        rwlock->rw_refcount++;
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_timedrdlock);

int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock, const struct timespec *abstime)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    
    if (-1 == rwlock->attr)
    {
        pthread_rwlock_init(rwlock, NULL);
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    while (0 != rwlock->rw_refcount)
    {
        rwlock->rw_nwaitwriters++;
        
        /* rw_mutex will be released when waiting for rw_condwriters. */
        result = pthread_cond_timedwait(&rwlock->rw_condwriters, &rwlock->rw_mutex, abstime);
        
        /* rw_mutex should have been taken again when returned from waiting. */
        rwlock->rw_nwaitwriters--;
        
        if (0 != result)
        {
            break;
        }
    }

    if (0 == result)
    {
        rwlock->rw_refcount = -1;
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_timedwrlock);

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    
    if (-1 == rwlock->attr)
    {
        pthread_rwlock_init(rwlock, NULL);
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    /* Held by either writer or reader(s) */
    if (0 != rwlock->rw_refcount)
    {
        result = EBUSY;
    }
     /* Available, indicate a writer has it */
    else
    {
        rwlock->rw_refcount = -1;
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_trywrlock);

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    
    if (-1 == rwlock->attr)
    {
        pthread_rwlock_init(rwlock, NULL);
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    /* Releasing a reader */
    if (rwlock->rw_refcount > 0)
    {
        rwlock->rw_refcount--;
    }
    /* Releasing a writer */
    else if(-1 == rwlock->rw_refcount)
    {
        rwlock->rw_refcount = 0;
    }

    /* Give preference to waiting writers over waiting readers */
    if (rwlock->rw_nwaitwriters > 0)
    {
        if (0 == rwlock->rw_refcount)
        {
            result = pthread_cond_signal(&rwlock->rw_condwriters);
        }
    }
    else if (rwlock->rw_nwaitreaders > 0)
    {
        result = pthread_cond_broadcast(&rwlock->rw_condreaders);
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_unlock);

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    int result;

    if (!rwlock)
    {
        return EINVAL;
    }
    
    if (-1 == rwlock->attr)
    {
        pthread_rwlock_init(rwlock, NULL);
    }

    if (0 != (result = pthread_mutex_lock(&rwlock->rw_mutex)))
    {
        return result;
    }

    while (0 != rwlock->rw_refcount)
    {
        rwlock->rw_nwaitwriters++;
        
        /* rw_mutex will be released when waiting for rw_condwriters. */
        result = pthread_cond_wait(&rwlock->rw_condwriters, &rwlock->rw_mutex);
        
        /* rw_mutex should have been taken again when returned from waiting. */
        rwlock->rw_nwaitwriters--;
        
        if (0 != result)
        {
            break;
        }
    }

    if (0 == result)
    {
        rwlock->rw_refcount = -1;
    }

    pthread_mutex_unlock(&rwlock->rw_mutex);

    return result;
}
EXPORT_SYMBOL(pthread_rwlock_wrlock);

