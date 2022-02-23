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
 * @file        pthread_tls.c
 *
 * @brief       This file provides posix tls functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>
#include "pthread_internal.h"

_pthread_key_data_t g_thread_keys[PTHREAD_KEY_MAX];

void pthread_key_system_init()
{
    memset(&g_thread_keys[0], 0, sizeof(g_thread_keys));
}

void *pthread_getspecific(pthread_key_t key)
{
    _pthread_data_t *ptd;

    if (OS_NULL == os_task_self())
    {
        return NULL;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(NULL != ptd);

    if (NULL == ptd->tls)
    {
        return NULL;
    }

    if ((key < PTHREAD_KEY_MAX) && (g_thread_keys[key].is_used))
    {
        return ptd->tls[key];
    }

    return NULL;
}
EXPORT_SYMBOL(pthread_getspecific);

int pthread_setspecific(pthread_key_t key, const void *value)
{
    _pthread_data_t *ptd;

    if (OS_NULL == os_task_self())
    {
        return 0;
    }

    /* Get pthread data from user data of thread. */
    ptd = (_pthread_data_t *)os_task_self()->user_data;
    OS_ASSERT(NULL != ptd);

    /* Check tls area */
    if (NULL == ptd->tls)
    {
        ptd->tls = (void**)os_malloc(sizeof(void*) * PTHREAD_KEY_MAX);
    }

    if ((key < PTHREAD_KEY_MAX) && g_thread_keys[key].is_used)
    {
        ptd->tls[key] = (void *)value;

        return 0;
    }

    return EINVAL;
}
EXPORT_SYMBOL(pthread_setspecific);

int pthread_key_create(pthread_key_t *key, void (*destructor)(void*))
{
    os_uint32_t index;

    os_schedule_lock();
    
    for (index = 0; index < PTHREAD_KEY_MAX; index ++)
    {
        if (0 == g_thread_keys[index].is_used)
        {
            g_thread_keys[index].is_used = 1;
            g_thread_keys[index].destructor = destructor;

            *key = index;

            os_schedule_unlock();

            return 0;
        }
    }

    os_schedule_unlock();

    return EAGAIN;
}
EXPORT_SYMBOL(pthread_key_create);

int pthread_key_delete(pthread_key_t key)
{
    if (key >= PTHREAD_KEY_MAX)
    {
        return EINVAL;
    }

    os_schedule_lock();
    
    g_thread_keys[key].is_used = 0;
    g_thread_keys[key].destructor = 0;
    
    os_schedule_unlock();

    return 0;
}
EXPORT_SYMBOL(pthread_key_delete);

