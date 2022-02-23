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
 * @file        cmsis_mutex.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 mutex function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_mutex.h>
#include <os_errno.h>
#include <os_util.h>
#include <string.h>
#include <arch_interrupt.h>

#include "cmsis_internal.h"

#ifdef OS_USING_MUTEX

osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
    char               name[OS_NAME_MAX];
    mutex_cb_t        *mutex_cb;
    static os_uint16_t mutex_number = 1U;
    os_err_t ret;

    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "mutex%02d", mutex_number++);
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        mutex_cb = os_malloc(sizeof(mutex_cb_t));
        if (OS_NULL == mutex_cb)
        {
            return (osMutexId_t)OS_NULL;
        }
        memset(mutex_cb, 0, sizeof(mutex_cb_t));
        mutex_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(mutex_cb_t))
        {
            mutex_cb = attr->cb_mem;
            mutex_cb->flags = 0;
        }
        else
        {
            return (osMutexId_t)OS_NULL;
        }
    }

    if ((OS_NULL == attr) || (0 == attr->attr_bits))
    {
        mutex_cb->flags |= 0;
    }
    else if((attr->attr_bits&osMutexRecursive) == osMutexRecursive)
    {
        mutex_cb->flags |= osMutexRecursive;
    }
    strncpy(&mutex_cb->name[0], name, OS_NAME_MAX);
    mutex_cb->id = IdMutex;
    if((mutex_cb->flags&osMutexRecursive) == osMutexRecursive)
    {
        ret = os_mutex_init(&mutex_cb->mutex, name, OS_TRUE);
    }
    else
    {
        ret = os_mutex_init(&mutex_cb->mutex, name, OS_FALSE);
    }
    if (OS_EOK != ret)
    {
        if (mutex_cb->flags & SYS_MALLOC_CTRL_BLK)
        {
            os_free(mutex_cb);
        }
        return (osMutexId_t)OS_NULL;
    }

    return (osMutexId_t)mutex_cb;
}

const char *osMutexGetName(osMutexId_t mutex_id)
{
    mutex_cb_t *mutex_cb;

    mutex_cb = (mutex_cb_t *)mutex_id;

    if ((OS_NULL == mutex_cb) || (IdMutex != mutex_cb->id))
    {
        return OS_NULL;
    }

    return mutex_cb->name;
}

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    os_err_t    result;
    mutex_cb_t *mutex_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    mutex_cb = (mutex_cb_t *)mutex_id;

    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    if ((OS_NULL == mutex_cb) || (IdMutex != mutex_cb->id))
    {
        return osErrorParameter;
    }

    /* Avoiding non-recursion mutex is locked multiple times by the same task OS_ASSERT() will detect it*/
    os_schedule_lock();

    OS_ASSERT(((os_mutex_get_owner(&mutex_cb->mutex) == os_task_self()) &&
                ((mutex_cb->flags&osMutexRecursive) == 0)) == 0);

    os_schedule_unlock();

    if((mutex_cb->flags&osMutexRecursive) == osMutexRecursive)
    {
        result = os_mutex_recursive_lock(&mutex_cb->mutex, timeout);
    }
    else
    {
        result = os_mutex_lock(&mutex_cb->mutex, timeout);
    }

    if (OS_EOK == result)
    {
        return osOK;
    }
    else if(OS_EBUSY == result)
    {
        return osErrorResource;
    }
    else if (OS_ETIMEOUT == result)
    {
        return osErrorTimeout;
    }
    else
    {
        return osError;
    }
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    os_err_t    result;
    mutex_cb_t *mutex_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    mutex_cb = (mutex_cb_t *)mutex_id;

    /* Check parameters */
    if ((OS_NULL == mutex_cb) || (IdMutex != mutex_cb->id))
    {
        return osErrorParameter;
    }

    if((mutex_cb->flags&osMutexRecursive) == osMutexRecursive)
    {
        result = os_mutex_recursive_unlock(&mutex_cb->mutex);
    }
    else
    {
        result = os_mutex_unlock(&mutex_cb->mutex);
    }

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

osThreadId_t osMutexGetOwner(osMutexId_t mutex_id)
{
    os_task_t *os_task;
    thread_cb_t *thread_cb;
    mutex_cb_t *mutex_cb = (mutex_cb_t *)mutex_id;

    if ((OS_NULL == mutex_cb) || (IdMutex != mutex_cb->id))
    {
        return OS_NULL;
    }
    os_task = os_mutex_get_owner(&mutex_cb->mutex);

    thread_cb = os_list_entry(os_task, struct thread_control_block, task);
    return (osThreadId_t)thread_cb;
}
osStatus_t osMutexDelete(osMutexId_t mutex_id)
{
    mutex_cb_t *mutex_cb;
    os_err_t ret;
    osStatus_t status;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    mutex_cb = (mutex_cb_t *)mutex_id;

    if ((OS_NULL == mutex_cb) || (IdMutex != mutex_cb->id))
    {
        return osErrorParameter;
    }
    mutex_cb->id = IdInvalid;
    ret = os_mutex_deinit(&mutex_cb->mutex);
    if (OS_EOK == ret)
    {
        status = osOK;
    }
    else
    {
        status = osErrorResource;
    }

    if (mutex_cb->flags&SYS_MALLOC_CTRL_BLK)
    {
        os_free(mutex_cb);
    }

    return status;
}

#endif  /* OS_USING_MUTEX */

