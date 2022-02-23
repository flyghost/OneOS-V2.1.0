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
 * @file        pthread_spin.c
 *
 * @brief       This file provides posix spin functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>

int pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
    if (!lock)
    {
        return EINVAL;
    }

    lock->lock = 0;

    return 0;
}

int pthread_spin_destroy(pthread_spinlock_t *lock)
{
    if (!lock)
    {
        return EINVAL;
    }

    return 0;
}

int pthread_spin_lock(pthread_spinlock_t *lock)
{
    if (!lock)
    {
        return EINVAL;
    }

    while (!(lock->lock))
    {
        lock->lock = 1;
    }

    return 0;
}

int pthread_spin_trylock(pthread_spinlock_t *lock)
{
    if (!lock)
    {
        return EINVAL;
    }

    if (!(lock->lock))
    {
        lock->lock = 1;

        return 0;
    }

    return EBUSY;
}

int pthread_spin_unlock(pthread_spinlock_t *lock)
{
    if (!lock)
    {
        return EINVAL;
    }
    
    if (!(lock->lock))
    {
        return EPERM;
    }

    lock->lock = 0;

    return 0;
}
