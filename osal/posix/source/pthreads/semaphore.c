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
 * @file        semaphore.c
 *
 * @brief       This file implements the posix semaphore functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include <libc_ext.h>
#include <fcntl.h>
#include <semaphore.h>
#include "pthread_internal.h"

static sem_t *gs_posix_sem_list = OS_NULL;
static os_sem_t gs_posix_sem_lock;

void posix_sem_system_init()
{
    os_sem_init(&gs_posix_sem_lock, "psem", 1, 1); // TODO: need check
}

OS_INLINE void posix_sem_insert(sem_t *psem)
{
    psem->next = gs_posix_sem_list;
    gs_posix_sem_list = psem;
}

static void posix_sem_delete(sem_t *psem)
{
    sem_t *iter;
    
    if (gs_posix_sem_list == psem)
    {
        gs_posix_sem_list = psem->next;

        os_sem_destroy(psem->sem);

        return;
    }
    
    for (iter = gs_posix_sem_list; iter->next != OS_NULL; iter = iter->next)
    {
        if (iter->next == psem)
        {
            /* Delete this mq. */
            if(OS_NULL != psem->next)
            {
                iter->next = psem->next;
            }
            else
            {
                iter->next = OS_NULL;
            }

            /* Destroy OneOS mqueue. */
            os_sem_destroy(psem->sem);

            return ;
        }
    }
}

static sem_t *posix_sem_find(const char* name)
{
    sem_t       *iter;
    /* os_object_t *object; */

    for (iter = gs_posix_sem_list; iter != OS_NULL; iter = iter->next)
    {
        /* object = &iter->sem->parent.parent; */

        if (strncmp(iter->sem->name, name, OS_NAME_MAX) == 0)
        {
            return iter;
        }
    }

    return OS_NULL;
}

int sem_close(sem_t *sem)
{
    if (OS_NULL == sem)
    {
        os_set_errno(EINVAL);

        return -1;
    }

    /* Lock posix semaphore list. */
    os_sem_wait(&gs_posix_sem_lock, OS_WAIT_FOREVER);
    
    sem->refcount--;
    if (0 == sem->refcount)
    {
        /* Delete from posix semaphore list. */
        if (sem->unlinked)
        {
            posix_sem_delete(sem);
            os_free(sem);
        }
        
        sem = OS_NULL;
    }
    
    os_sem_post(&gs_posix_sem_lock);

    return 0;
}
EXPORT_SYMBOL(sem_close);

int sem_destroy(sem_t *sem)
{
    if ((!sem) || !(sem->unamed))
    {
        os_set_errno(EINVAL);

        return -1;
    }

    /* Lock posix semaphore list. */
    os_sem_wait(&gs_posix_sem_lock, OS_WAIT_FOREVER);

    /* Delete an unamed posix semaphore. */
    posix_sem_delete(sem);
    
    os_sem_post(&gs_posix_sem_lock);

    return 0;
}
EXPORT_SYMBOL(sem_destroy);

int sem_unlink(const char *name)
{
    sem_t *psem;

    /* Lock posix semaphore list. */
    os_sem_wait(&gs_posix_sem_lock, OS_WAIT_FOREVER);
    
    psem = posix_sem_find(name);
    if (OS_NULL != psem)
    {
        psem->unlinked = 1;
        if (0 == psem->refcount)
        {
            /* Delete this semaphore. */
            posix_sem_delete(psem);
            os_free(psem);
        }
        os_sem_post(&gs_posix_sem_lock);

        return 0;
    }
    
    os_sem_post(&gs_posix_sem_lock);

    /* No this entry. */
    os_set_errno(ENOENT);

    return -1;
}
EXPORT_SYMBOL(sem_unlink);

int sem_getvalue(sem_t *sem, int *sval)
{
    if (!sem || !sval)
    {
        os_set_errno(EINVAL);

        return -1;
    }
    
    *sval = sem->sem->count;

    return 0;
}
EXPORT_SYMBOL(sem_getvalue);

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
    char name[OS_NAME_MAX];
    static os_uint16_t psem_number = 0;

    if (OS_NULL == sem)
    {
        os_set_errno(EINVAL);

        return -1;
    }

    os_snprintf(name, sizeof(name), "psem%02d", psem_number++);
    
    sem->sem = os_sem_create(name, value, OS_SEM_MAX_VALUE); // TODO: need check
    if (OS_NULL== sem)
    {
        os_set_errno(ENOMEM);

        return -1;
    }

    /* Initialize posix semaphore. */
    sem->refcount = 1;
    sem->unlinked = 0;
    sem->unamed   = 1;
    
    /* Lock posix semaphore list. */
    os_sem_wait(&gs_posix_sem_lock, OS_WAIT_FOREVER);
    posix_sem_insert(sem);
    os_sem_post(&gs_posix_sem_lock);

    return 0;
}
EXPORT_SYMBOL(sem_init);

sem_t *sem_open(const char *name, int oflag, ...)
{
    sem_t        *sem;
    va_list       arg;
    mode_t        mode;
    unsigned int  value;

    sem = OS_NULL;

    /* Lock posix semaphore list. */
    os_sem_wait(&gs_posix_sem_lock, OS_WAIT_FOREVER);
    
    if (oflag & O_CREAT)
    {
        va_start(arg, oflag);
        mode  = (mode_t)va_arg(arg, unsigned int);
        mode  = mode;
        value = va_arg(arg, unsigned int);
        va_end(arg);

        if (oflag & O_EXCL)
        {
            if (OS_NULL != posix_sem_find(name))
            {
                os_set_errno(EEXIST);
                goto __return;
            }
        }
        
        sem = (sem_t *)os_malloc(sizeof(struct posix_sem));
        if (OS_NULL == sem)
        {
            os_set_errno(ENFILE);
            goto __return;
        }

        /* Create OneOS semaphore. */
        sem->sem = os_sem_create(name, value, OS_SEM_MAX_VALUE); // TODO: need check
        if (OS_NULL == sem->sem)
        {
            os_set_errno(ENFILE);
            goto __return;
        }
        
        /* Initialize reference count. */
        sem->refcount = 1;
        sem->unlinked = 0;
        sem->unamed   = 0;

        /* Insert semaphore to posix semaphore list. */
        posix_sem_insert(sem);
    }
    else
    {
        sem = posix_sem_find(name);
        if (OS_NULL != sem)
        {
            sem->refcount++;
        }
        else
        {
            os_set_errno(ENOENT);
            goto __return;
        }
    }
    os_sem_post(&gs_posix_sem_lock);

    return sem;

__return:
    /* Release lock. */
    os_sem_post(&gs_posix_sem_lock);

    /* Release allocated memory. */
    if (OS_NULL != sem)
    {
        /* Destroy OneOS semaphore */
        if (OS_NULL != sem->sem)
        {
            os_sem_destroy(sem->sem);
        }
        
        os_free(sem);
    }

    return OS_NULL;
}
EXPORT_SYMBOL(sem_open);

int sem_post(sem_t *sem)
{
    os_err_t result;

    if (!sem)
    {
        os_set_errno(EINVAL);

        return -1;
    }

    result = os_sem_post(sem->sem);
    if (OS_EOK == result)
    {
        return 0;
    }

    os_set_errno(EINVAL);

    return -1;
}
EXPORT_SYMBOL(sem_post);

int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout)
{
    os_err_t   result;
    os_int32_t tick;

    if (!sem || !abs_timeout)
    {
        return EINVAL;
    }

    /* Calculate os tick. */
    tick = clock_time_to_tick(abs_timeout);

    result = os_sem_wait(sem->sem, tick);
    if (OS_ETIMEOUT == result)
    {
        os_set_errno(ETIMEDOUT);

        return -1;
    }
    
    if (OS_EOK == result)
    {
        return 0;
    }

    os_set_errno(EINTR);

    return -1;
}
EXPORT_SYMBOL(sem_timedwait);

int sem_trywait(sem_t *sem)
{
    os_err_t result;

    if (!sem)
    {
        os_set_errno(EINVAL);

        return -1;
    }

    result = os_sem_wait(sem->sem, OS_NO_WAIT);
    if (OS_EBUSY == result)
    {
        os_set_errno(EAGAIN);

        return -1;
    }
	
    if (OS_EOK == result)
    {
        return 0;
    }

    os_set_errno(EINTR);

    return -1;
}
EXPORT_SYMBOL(sem_trywait);

int sem_wait(sem_t *sem)
{
    os_err_t result;

    if (!sem)
    {
        os_set_errno(EINVAL);

        return -1;
    }

    result = os_sem_wait(sem->sem, OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        return 0;
    }

    os_set_errno(EINTR);

    return -1;
}
EXPORT_SYMBOL(sem_wait);

