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
 * @file        pthread_mutex.c
 *
 * @brief       This file provides posix mutex functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>

#define  MUTEXATTR_SHARED_MASK 0x0010
#define  MUTEXATTR_TYPE_MASK   0x000f

const pthread_mutexattr_t g_pthread_default_mutexattr = PTHREAD_PROCESS_PRIVATE;

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (attr)
    {
        *attr = g_pthread_default_mutexattr;

        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_mutexattr_init);

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (attr)
    {
        *attr = -1;

        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_mutexattr_destroy);

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (attr && type)
    {
        int atype = (*attr & MUTEXATTR_TYPE_MASK);

        if ((atype >= PTHREAD_MUTEX_NORMAL) && (atype <= PTHREAD_MUTEX_ERRORCHECK))
        {
            *type = atype;

            return 0;
        }
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_mutexattr_gettype);

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (attr && (type >= PTHREAD_MUTEX_NORMAL) && (type <= PTHREAD_MUTEX_ERRORCHECK))
    {
        *attr = (*attr & ~MUTEXATTR_TYPE_MASK) | type;

        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_mutexattr_settype);

int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
    if (!attr)
    {
        return EINVAL;
    }

    switch (pshared)
    {
    case PTHREAD_PROCESS_PRIVATE:
        *attr &= ~MUTEXATTR_SHARED_MASK;
        return 0;

    case PTHREAD_PROCESS_SHARED:
        *attr |= MUTEXATTR_SHARED_MASK;
        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_mutexattr_setpshared);

int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared)
{
    if (!attr || !pshared)
    {
        return EINVAL;
    }

    *pshared = (*attr & MUTEXATTR_SHARED_MASK) ? PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE;
    
    return 0;
}
EXPORT_SYMBOL(pthread_mutexattr_getpshared);

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    os_err_t  result;
    char      name[OS_NAME_MAX];
    os_bool_t recursive;
    static os_uint16_t pthread_mutex_number = 0;

    if (!mutex)
    {
        return EINVAL;
    }

    os_snprintf(name, sizeof(name), "pmtx%02d", pthread_mutex_number++);
    
    if (OS_NULL == attr)
    {
        mutex->attr = g_pthread_default_mutexattr;
    }
    else
    {
        mutex->attr = *attr;
    }

    if (mutex->attr & PTHREAD_MUTEX_RECURSIVE)
    {
        recursive = OS_TRUE;
    }
    else
    {
        recursive = OS_FALSE;
    }

    result = os_mutex_init(&mutex->lock, name, recursive);
    if (OS_EOK != result)
    {
        return EINVAL;
    }

    /* Detach the object from system object container. */
    /* os_object_deinit(&mutex->lock.parent.parent); */
    /* mutex->lock.parent.parent.type = OS_OBJECT_MUTEX; */

    return 0;
}
EXPORT_SYMBOL(pthread_mutex_init);

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    if ((!mutex) || (-1 == mutex->attr))
    {
        return EINVAL;
    }

    /* It's busy */
    if (OS_NULL != os_mutex_get_owner(&mutex->lock))
    {
        return EBUSY;
    }

    os_mutex_deinit(&mutex->lock);

    mutex->attr = -1;

    return 0;
}
EXPORT_SYMBOL(pthread_mutex_destroy);

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    int      mtype;
    os_err_t result;

    if (!mutex)
    {
        return EINVAL;
    }

    if (-1 == mutex->attr)
    {
        pthread_mutex_init(mutex, OS_NULL);
    }

    mtype = mutex->attr & MUTEXATTR_TYPE_MASK;
    
    os_schedule_lock();
    
    if ((os_mutex_get_owner(&mutex->lock) == os_task_self()) && (PTHREAD_MUTEX_RECURSIVE != mtype))
    {
        os_schedule_unlock();

        return EDEADLK;
    }
    
    os_schedule_unlock();

    if (PTHREAD_MUTEX_RECURSIVE != mtype)
    {
        result = os_mutex_lock(&mutex->lock, OS_WAIT_FOREVER);
    }
    else
    {
        result = os_mutex_recursive_lock(&mutex->lock, OS_WAIT_FOREVER);
    }
    
    if (OS_EOK == result)
    {
        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_mutex_lock);

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    os_err_t result;
    int      mtype;

    if (!mutex)
    {
        return EINVAL;
    }
    
    if (-1 == mutex->attr)
    {
        pthread_mutex_init(mutex, OS_NULL);
    }

    mtype = mutex->attr & MUTEXATTR_TYPE_MASK;

    if (os_mutex_get_owner(&mutex->lock) != os_task_self())
    {
        if (PTHREAD_MUTEX_ERRORCHECK == mtype)
        {
            return EPERM;
        }

        /* No thread waiting on this mutex. */
        if (OS_NULL == os_mutex_get_owner(&mutex->lock))
        {
            return 0;
        }
    }

    if (PTHREAD_MUTEX_RECURSIVE != mtype)
    {
        result = os_mutex_unlock(&mutex->lock);
    }
    else
    {
        result = os_mutex_recursive_unlock(&mutex->lock);
    }
    
    if (OS_EOK == result)
    {
        return 0;
    }
    
    return EINVAL;
}
EXPORT_SYMBOL(pthread_mutex_unlock);

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    os_err_t result;
    int      mtype;

    if (!mutex)
    {
        return EINVAL;
    }
    
    if (-1 == mutex->attr)
    {
        pthread_mutex_init(mutex, OS_NULL);
    }

    mtype = mutex->attr & MUTEXATTR_TYPE_MASK;
    
    os_schedule_lock();
    
    if ((os_mutex_get_owner(&mutex->lock) == os_task_self()) && (PTHREAD_MUTEX_RECURSIVE != mtype))
    {
        os_schedule_unlock();

        return EDEADLK;
    }
    
    os_schedule_unlock();

    if (PTHREAD_MUTEX_RECURSIVE != mtype)
    {
        result = os_mutex_lock(&mutex->lock, OS_NO_WAIT);
    }
    else
    {
        result = os_mutex_recursive_lock(&mutex->lock, OS_NO_WAIT);
    }
    
    if (OS_EOK == result)
    {
        return 0;
    }

    return EBUSY;
}
EXPORT_SYMBOL(pthread_mutex_trylock);
