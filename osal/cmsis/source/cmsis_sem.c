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
 * @file        cmsis_sem.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 semaphore function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_errno.h>
#include <os_util.h>
#include <string.h>
#include <os_sem.h>
#include <arch_interrupt.h>

#include "cmsis_internal.h"

#ifdef OS_USING_SEMAPHORE

osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
    char                name[OS_NAME_MAX];
    sem_cb_t           *sem_cb;
    static os_uint16_t  semaphore_number = 1U;
    os_err_t ret;

    if ((0U == max_count) || (initial_count > max_count))
    {
        return (osSemaphoreId_t)OS_NULL;
    }

    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "sem%02d", semaphore_number++);
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        sem_cb = os_malloc(sizeof(sem_cb_t));
        if (OS_NULL == sem_cb)
        {
            return (osSemaphoreId_t)OS_NULL;
        }
        memset(sem_cb, 0, sizeof(sem_cb_t));
        sem_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(sem_cb_t))
        {
            sem_cb = attr->cb_mem;
            sem_cb->flags = 0;
        }
        else
        {
            return (osSemaphoreId_t)OS_NULL;
        }
    }
    strncpy(&sem_cb->name[0], name, OS_NAME_MAX);
    sem_cb->id = IdSemaphore;

    ret = os_sem_init(&sem_cb->sem, name, initial_count, max_count);
    if (OS_EOK != ret)
    {
        if (sem_cb->flags & SYS_MALLOC_CTRL_BLK)
        {
            os_free(sem_cb);
        }
        return (osSemaphoreId_t)OS_NULL;
    }

    return (osSemaphoreId_t)sem_cb;
}

const char *osSemaphoreGetName(osSemaphoreId_t semaphore_id)
{
    sem_cb_t *sem_cb;

    sem_cb = (sem_cb_t *)semaphore_id;

    if ((OS_NULL == sem_cb) || (IdSemaphore != sem_cb->id))
    {
        return OS_NULL;
    }

    return sem_cb->name;
}

osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
    os_err_t result;
    sem_cb_t *sem_cb = (sem_cb_t *)semaphore_id;

    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    if ((OS_NULL == sem_cb) || (IdSemaphore != sem_cb->id))
    {
        return osErrorParameter;
    }

    result = os_sem_wait(&sem_cb->sem, timeout);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else if (OS_EBUSY == result)
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

osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id)
{
    os_err_t  result;
    sem_cb_t *sem_cb;

    sem_cb = (sem_cb_t *)semaphore_id;

    if ((OS_NULL == sem_cb) || (IdSemaphore != sem_cb->id))
    {
        return osErrorParameter;
    }

    result = os_sem_post(&sem_cb->sem);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osError;
    }
}

uint32_t osSemaphoreGetCount(osSemaphoreId_t semaphore_id)
{
    sem_cb_t *sem_cb;

    sem_cb = (sem_cb_t *)semaphore_id;

    if ((OS_NULL == sem_cb) || (IdSemaphore != sem_cb->id))
    {
        return 0U;
    }

    return (uint32_t)os_sem_get_count(&sem_cb->sem);
}

osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id)
{
    sem_cb_t *sem_cb;
    os_err_t ret;
    osStatus_t status;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    sem_cb = (sem_cb_t *)semaphore_id;

    if ((OS_NULL == sem_cb) || (IdSemaphore != sem_cb->id))
    {
        return osErrorParameter;
    }

    sem_cb->id = IdInvalid;

    ret = os_sem_deinit(&sem_cb->sem);
    if (OS_EOK == ret)
    {
        status = osOK;
    }
    else
    {
        status = osErrorResource;
    }

    if (sem_cb->flags & SYS_MALLOC_CTRL_BLK)
    {
        os_free(sem_cb);
    }

    return status;
}

#endif  /* OS_USING_SEMAPHORE */

