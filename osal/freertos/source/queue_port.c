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
 * @file        queue_port.c
 *
 * @brief       This file implements some port functions of FreeRTOS queue.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-03   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include "FreeRTOS.h"
#include "portmacro.h"
#include "queue.h"
#include "task.h"
#include "os_stddef.h"
#include "os_errno.h"
#include "os_assert.h"
#include "os_util.h"
#include "os_mutex.h"
#include "os_sem.h"
#include "os_mq.h"
#include "os_memory.h"
#include "freertos_internal.h"

#define ADAPT_DEBUG_QUEUE           0
#define QUEUEHANDLE_TO_OSMUTEX(x) (&((x->u).os_mutex))
#define QUEUEHANDLE_TO_OSSEM(x) (&((x->u).os_sem))
#define QUEUEHANDLE_TO_OSMQ(x) (&((x->u).os_mq))

static uint32_t m = 0;             /* used for index for mutex name    */
static uint32_t s = 0;             /* used for index for sem name     */
static uint32_t q = 0;             /* used for index for queue name    */

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
QueueHandle_t xQueueGenericCreate( const UBaseType_t uxQueueLength,
                                   const UBaseType_t uxItemSize,
                                   const uint8_t ucQueueType )
{
#ifdef OS_USING_SYS_HEAP
#else
#error "OS_USING_SYS_HEAP should be defined in menuconfig, otherwise xxx_create will no use"
#endif
    QueueHandle_t queue_handle_t;
    char name[OS_NAME_MAX];          /* "assume max num is MAX_UINT32"    */
    char idx[UINT32_TOCHAR_LEN];
    os_err_t ret;
    os_size_t align_msg_size;
    os_size_t msg_pool_size;
    void *msg_pool;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGenericCreate 0x%x\n",ucQueueType));

    memset(name, 0, sizeof(name));
    memset(idx,  0, sizeof(idx));

    queue_handle_t = (QueueHandle_t)os_malloc(sizeof(struct QueueDefinition));
    if (OS_NULL == queue_handle_t)
    {
        return NULL;
    }

    if (queueQUEUE_TYPE_RECURSIVE_MUTEX == ucQueueType)
    {
        oneos_itoa(m++, idx, DECIMAL_TYPE);
        strncat(name, "m_", strlen("m_"));
        strncat(name, idx, strlen(idx));
        ret = os_mutex_init(QUEUEHANDLE_TO_OSMUTEX(queue_handle_t), name, OS_TRUE);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_RECURSIVE_MUTEX;
        queue_handle_t->is_static = OS_FALSE;
    }
    else if (queueQUEUE_TYPE_MUTEX == ucQueueType)
    {
        oneos_itoa(m++, idx, DECIMAL_TYPE);
        strncat(name, "m_", strlen("m_"));
        strncat(name, idx, strlen(idx));
        ret = os_mutex_init(QUEUEHANDLE_TO_OSMUTEX(queue_handle_t), name, OS_FALSE);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_MUTEX;
        queue_handle_t->is_static = OS_FALSE;
    }
    else if (queueQUEUE_TYPE_BINARY_SEMAPHORE == ucQueueType)
    {
        oneos_itoa(s++, idx, DECIMAL_TYPE);
        strncat(name, "s_", strlen("s_"));
        strncat(name, idx, strlen(idx));
        ret = os_sem_init(QUEUEHANDLE_TO_OSSEM(queue_handle_t), name, 0, uxQueueLength);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE;
        queue_handle_t->is_static = OS_FALSE;
    }
    else if (queueQUEUE_TYPE_COUNTING_SEMAPHORE == ucQueueType)
    {
        /* never reach here , because counting sem is adapted in xQueueCreateCountingSemaphore */
        OS_ASSERT(0);
    }
    else
    {
        oneos_itoa(q++, idx, DECIMAL_TYPE);
        strncat(name, "q_", strlen("q_"));
        strncat(name, idx, strlen(idx));
        align_msg_size = OS_ALIGN_UP(uxItemSize, OS_ALIGN_SIZE);
        msg_pool_size  = uxQueueLength * (align_msg_size + sizeof(os_mq_msg_hdr_t));
        msg_pool = os_aligned_malloc(OS_ALIGN_SIZE, msg_pool_size);

        if (OS_NULL == msg_pool)
        {
            os_free(queue_handle_t);
            return NULL;
        }
        ret = os_mq_init(QUEUEHANDLE_TO_OSMQ(queue_handle_t), name, msg_pool, msg_pool_size, align_msg_size);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_MESSAGEQUEUE;
        queue_handle_t->is_static = OS_FALSE;
        queue_handle_t->msg_pool = msg_pool;
        queue_handle_t->max_msg_size = align_msg_size;
        queue_handle_t->original_msg_size = uxItemSize;
    }

    if (OS_EOK != ret)
    {
        if(OS_NULL != msg_pool)
        {
            os_free(msg_pool);
        }
        os_free(queue_handle_t);
        return     NULL;
    }
    else
    {
        return queue_handle_t;
    }
}
#endif

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
QueueHandle_t xQueueGenericCreateStatic( const UBaseType_t uxQueueLength,
                                             const UBaseType_t uxItemSize,
                                             uint8_t * pucQueueStorage,
                                             StaticQueue_t * pxStaticQueue,
                                             const uint8_t ucQueueType )
{
    QueueHandle_t queue_handle_t;
    os_err_t ret;
    os_size_t align_msg_size;

    char name[OS_NAME_MAX];          /* "assume max num is MAX_UINT32"    */
    char idx[UINT32_TOCHAR_LEN];

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGenericCreateStatic 0x%x\n",ucQueueType));

    memset(name, 0, sizeof(name));
    memset(idx,  0, sizeof(idx));

    queue_handle_t = (QueueHandle_t)pxStaticQueue;

    if (queueQUEUE_TYPE_RECURSIVE_MUTEX == ucQueueType)
    {
        oneos_itoa(m++, idx, DECIMAL_TYPE);
        strncat(name, "m_", strlen("m_"));
        strncat(name, idx, strlen(idx));
        ret = os_mutex_init(QUEUEHANDLE_TO_OSMUTEX(queue_handle_t), name, OS_TRUE);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_RECURSIVE_MUTEX;
        queue_handle_t->is_static = OS_TRUE;
    }
    else if (queueQUEUE_TYPE_MUTEX == ucQueueType)
    {
        oneos_itoa(m++, idx, DECIMAL_TYPE);
        strncat(name, "m_", strlen("m_"));
        strncat(name, idx, strlen(idx));
        ret = os_mutex_init(QUEUEHANDLE_TO_OSMUTEX(queue_handle_t), name, OS_FALSE);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_MUTEX;
        queue_handle_t->is_static = OS_TRUE;
    }
    else if (queueQUEUE_TYPE_BINARY_SEMAPHORE == ucQueueType)
    {
        oneos_itoa(s++, idx, DECIMAL_TYPE);
        strncat(name, "s_", strlen("s_"));
        strncat(name, idx, strlen(idx));
        ret = os_sem_init(QUEUEHANDLE_TO_OSSEM(queue_handle_t), name, 0, uxQueueLength);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE;
        queue_handle_t->is_static = OS_TRUE;
    }
    else if (queueQUEUE_TYPE_COUNTING_SEMAPHORE == ucQueueType)
    {
        oneos_itoa(s++, idx, DECIMAL_TYPE);
        strncat(name, "s_", strlen("s_"));
        strncat(name, idx, strlen(idx));
        ret = os_sem_init(QUEUEHANDLE_TO_OSSEM(queue_handle_t), name, 0, uxQueueLength);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE;
        queue_handle_t->is_static = OS_TRUE;
    }
    else
    {
        oneos_itoa(q++, idx, DECIMAL_TYPE);
        strncat(name, "q_", strlen("q_"));
        strncat(name, idx, strlen(idx));
        align_msg_size = OS_ALIGN_UP(uxItemSize, OS_ALIGN_SIZE);
        ret = os_mq_init(QUEUEHANDLE_TO_OSMQ(queue_handle_t), name, pucQueueStorage, (os_size_t)(uxQueueLength * uxItemSize), align_msg_size);
        queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_MESSAGEQUEUE;
        queue_handle_t->is_static = OS_TRUE;
        queue_handle_t->max_msg_size = align_msg_size;
        queue_handle_t->original_msg_size = uxItemSize;
    }

    if (OS_EOK != ret)
    {
        return NULL;
    }
    else
    {
        return queue_handle_t;
    }
}
#endif

BaseType_t xQueueReceive( QueueHandle_t xQueue,
                          void * const pvBuffer,
                          TickType_t xTicksToWait )
{

    os_err_t ret = OS_EOK;
    os_size_t recv_size = 0;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueReceive in:0x%x\n", xQueue));

    if (OS_ADAPT_QUEUE_TYPE_MUTEX == xQueue->queue_type)
    {
        ret = os_mutex_lock(QUEUEHANDLE_TO_OSMUTEX(xQueue), xTicksToWait);
    }
    else if (OS_ADAPT_QUEUE_TYPE_RECURSIVE_MUTEX == xQueue->queue_type)
    {
        ret = os_mutex_recursive_lock(QUEUEHANDLE_TO_OSMUTEX(xQueue), xTicksToWait);
    }
    else if (OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE == xQueue->queue_type)
    {
        ret = os_sem_wait(QUEUEHANDLE_TO_OSSEM(xQueue), xTicksToWait);
    }
    else if (OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE == xQueue->queue_type)
    {
        ret = os_sem_wait(QUEUEHANDLE_TO_OSSEM(xQueue), xTicksToWait);
    }
    else
    {
        ret = os_mq_recv(QUEUEHANDLE_TO_OSMQ(xQueue),
                         pvBuffer,
                         xQueue->original_msg_size,
                         xTicksToWait,
                         &recv_size);
    }

    if (OS_EOK == ret)
    {
        return pdPASS;
    }
    else
    {
        return errQUEUE_EMPTY;
    }
}

BaseType_t xQueueReceiveFromISR( QueueHandle_t xQueue,
                               void * const pvBuffer,
                               BaseType_t * const pxHigherPriorityTaskWoken )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueReceiveFromISR\n"));

    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    return xQueueReceive(xQueue, pvBuffer, 0);
}

BaseType_t xQueueGenericSend( QueueHandle_t xQueue,
                              const void * const pvItemToQueue,
                              TickType_t xTicksToWait,
                              const BaseType_t xCopyPosition )
{
    os_err_t ret;

    OS_ASSERT(queueOVERWRITE != xCopyPosition);

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE,("xQueueGenericSend in xQueue:0x%x\n", xQueue));

    if (OS_ADAPT_QUEUE_TYPE_MUTEX == xQueue->queue_type)
    {
        ret = os_mutex_unlock(QUEUEHANDLE_TO_OSMUTEX(xQueue));
    }
    else if (OS_ADAPT_QUEUE_TYPE_RECURSIVE_MUTEX == xQueue->queue_type)
    {
        ret = os_mutex_recursive_unlock(QUEUEHANDLE_TO_OSMUTEX(xQueue));
    }
    else if (OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE == xQueue->queue_type)
    {
        ret = os_sem_post(QUEUEHANDLE_TO_OSSEM(xQueue));
    }
    else if (OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE == xQueue->queue_type)
    {
        ret = os_sem_post(QUEUEHANDLE_TO_OSSEM(xQueue));
    }
    else
    {
        if (queueSEND_TO_BACK == xCopyPosition)
        {
            ret = os_mq_send(QUEUEHANDLE_TO_OSMQ(xQueue),
                             (void *)pvItemToQueue,
                             xQueue->original_msg_size,
                             xTicksToWait);
        }
        else if (queueSEND_TO_FRONT == xCopyPosition)
        {
            ret = os_mq_send_urgent(QUEUEHANDLE_TO_OSMQ(xQueue),
                             (void *)pvItemToQueue,
                             xQueue->original_msg_size,
                             xTicksToWait);
        }
        else
        {
            ret = OS_ENOSYS;
        }
    }

    if (OS_EOK == ret)
    {
        return pdPASS;
    }
    else
    {
        return errQUEUE_FULL;
    }

}

BaseType_t xQueueGenericSendFromISR(QueueHandle_t      xQueue,
                                    const void * const pvItemToQueue,
                                    BaseType_t * const pxHigherPriorityTaskWoken,
                                    const BaseType_t   xCopyPosition )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGenericSendFromISR\n"));

    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    return xQueueGenericSend(xQueue, pvItemToQueue, 0, xCopyPosition);
}

void vQueueDelete(QueueHandle_t xQueue)
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("vQueueDelete 0x%x\n",xQueue));

    if ((OS_ADAPT_QUEUE_TYPE_MUTEX == xQueue->queue_type) || (OS_ADAPT_QUEUE_TYPE_RECURSIVE_MUTEX == xQueue->queue_type))
    {
        os_mutex_deinit(QUEUEHANDLE_TO_OSMUTEX(xQueue));
    }
    else if ((OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE == xQueue->queue_type) || (OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE == xQueue->queue_type))
    {
        os_sem_deinit(QUEUEHANDLE_TO_OSSEM(xQueue));
    }
    else
    {
        os_mq_deinit(QUEUEHANDLE_TO_OSMQ(xQueue));
        if (OS_FALSE == xQueue->is_static)
        {
            if (OS_NULL != xQueue->msg_pool)
            {
                os_free(xQueue->msg_pool);
            }
        }
    }

    if (OS_FALSE == xQueue->is_static)
    {
        os_free(xQueue);
    }
}

BaseType_t xQueueGiveFromISR(QueueHandle_t xQueue, BaseType_t * const pxHigherPriorityTaskWoken)
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGiveFromISR\n"));

    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    return xQueueGenericSend(xQueue, OS_NULL, 0, 0);
}

UBaseType_t uxQueueMessagesWaiting( const QueueHandle_t xQueue )
{
    OS_ASSERT(OS_NULL != xQueue);

    os_uint32_t spaces;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueMessagesWaiting\n"));

    if (OS_ADAPT_QUEUE_TYPE_MESSAGEQUEUE == (xQueue->queue_type))
    {
        spaces = os_mq_get_used_entry_count(QUEUEHANDLE_TO_OSMQ(xQueue));
    }
    else if (OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE == (xQueue->queue_type) || OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE == (xQueue->queue_type))
    {
        spaces = os_sem_get_count(QUEUEHANDLE_TO_OSSEM(xQueue));
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueMessagesWaiting Not support orther queue type\n"));
        OS_ASSERT(0);
    }

    return (UBaseType_t)spaces;
}

UBaseType_t uxQueueSpacesAvailable(const QueueHandle_t xQueue)
{
    OS_ASSERT(OS_NULL != xQueue);

    os_uint32_t spaces;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueSpacesAvailable\n"));

    if (OS_ADAPT_QUEUE_TYPE_MESSAGEQUEUE == (xQueue->queue_type))
    {
        spaces = os_mq_get_unused_entry_count(QUEUEHANDLE_TO_OSMQ(xQueue));
    }
    else if ((OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE == (xQueue->queue_type)) || (OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE == (xQueue->queue_type)))
    {
        spaces = os_sem_get_max_count(QUEUEHANDLE_TO_OSSEM(xQueue)) - os_sem_get_count(QUEUEHANDLE_TO_OSSEM(xQueue));
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueSpacesAvailable Not support orther queue type\n"));
        OS_ASSERT(0);
    }

    return (UBaseType_t)spaces;
}

UBaseType_t uxQueueMessagesWaitingFromISR( const QueueHandle_t xQueue )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("uxQueueMessagesWaitingFromISR\n"));

    return uxQueueMessagesWaiting(xQueue);
}

BaseType_t xQueueIsQueueEmptyFromISR( const QueueHandle_t xQueue )
{
    BaseType_t result;
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueIsQueueEmptyFromISR\n"));

    if (OS_ADAPT_QUEUE_TYPE_MESSAGEQUEUE == (xQueue->queue_type))
    {
        result = (BaseType_t)os_mq_is_empty(QUEUEHANDLE_TO_OSMQ(xQueue));
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueIsQueueEmptyFromISR Not support orther queue type\n"));
        OS_ASSERT(0);
    }

    return result;
}

BaseType_t xQueueIsQueueFullFromISR( const QueueHandle_t xQueue )
{
    BaseType_t result;
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueIsQueueFullFromISR\n"));

    if (OS_ADAPT_QUEUE_TYPE_MESSAGEQUEUE == (xQueue->queue_type))
    {
        result = (BaseType_t)os_mq_is_full(QUEUEHANDLE_TO_OSMQ(xQueue));
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueIsQueueFullFromISR Not support orther queue type\n"));
        OS_ASSERT(0);
    }

    return result;
}

#if ( ( configUSE_MUTEXES == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
QueueHandle_t xQueueCreateMutex( const uint8_t ucQueueType )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueCreateMutex %d\n",ucQueueType));

    return xQueueGenericCreate(1, 0, ucQueueType);
}
#endif

#if ( ( configUSE_MUTEXES == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
QueueHandle_t xQueueCreateMutexStatic( const uint8_t ucQueueType,
                                       StaticQueue_t * pxStaticQueue )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueCreateMutexStatic %d\n",ucQueueType));

    return xQueueGenericCreateStatic(1, 0, OS_NULL, pxStaticQueue, ucQueueType);
}
#endif

#if ( configUSE_RECURSIVE_MUTEXES == 1 )
BaseType_t xQueueTakeMutexRecursive( QueueHandle_t xMutex,
                                    TickType_t xTicksToWait )
{
    return xQueueReceive(xMutex, 0, xTicksToWait);
}

BaseType_t xQueueGiveMutexRecursive( QueueHandle_t xMutex )
{
    return xQueueGenericSend(xMutex, OS_NULL, 0, 0);
}
#endif

#if ( ( configUSE_MUTEXES == 1 ) && ( INCLUDE_xSemaphoreGetMutexHolder == 1 ) )
TaskHandle_t xQueueGetMutexHolder( QueueHandle_t xSemaphore )
{
    os_task_t *owner;
    TaskHandle_t task = OS_NULL;

    OS_ASSERT(OS_NULL != xSemaphore);

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGetMutexHolder\n"));

    if ((OS_ADAPT_QUEUE_TYPE_MUTEX == xSemaphore->queue_type) || (OS_ADAPT_QUEUE_TYPE_RECURSIVE_MUTEX == xSemaphore->queue_type))
    {
        owner = os_mutex_get_owner(QUEUEHANDLE_TO_OSMUTEX(xSemaphore));
        task = (TaskHandle_t)os_container_of(owner, struct QueueDefinition, u);
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueGetMutexHolder Not support orther queue type\n"));
        OS_ASSERT(0);
    }

    return task;
}

TaskHandle_t xQueueGetMutexHolderFromISR( QueueHandle_t xSemaphore )
{
    return xQueueGetMutexHolder(xSemaphore);
}
#endif

#if ( ( configUSE_COUNTING_SEMAPHORES == 1 ) && ( configSUPPORT_DYNAMIC_ALLOCATION == 1 ) )
QueueHandle_t xQueueCreateCountingSemaphore( const UBaseType_t uxMaxCount,
                                                 const UBaseType_t uxInitialCount )
{
#ifdef OS_USING_SYS_HEAP
#else
#error "OS_USING_SYS_HEAP should be defined in menuconfig, otherwise xxx_create will no use"
#endif
    QueueHandle_t queue_handle_t;
    char name[OS_NAME_MAX];          /* "assume max num is MAX_UINT32"    */
    char idx[UINT32_TOCHAR_LEN];
    os_err_t ret;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueCreateCountingSemaphore\n"));

    memset(name, 0, sizeof(name));
    memset(idx,  0, sizeof(idx));

    queue_handle_t = (QueueHandle_t)os_malloc(sizeof(struct QueueDefinition));
    if (OS_NULL == queue_handle_t)
    {
        return NULL;
    }
    oneos_itoa(s++, idx, DECIMAL_TYPE);
    strncat(name, "s_", strlen("s_"));
    strncat(name, idx, strlen(idx));
    ret = os_sem_init(QUEUEHANDLE_TO_OSSEM(queue_handle_t), name, uxInitialCount, uxMaxCount);
    queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE;
    queue_handle_t->is_static = OS_FALSE;

    if (OS_EOK != ret)
    {
        return     NULL;
    }
    else
    {
        return queue_handle_t;
    }

}
#endif

#if ( ( configUSE_COUNTING_SEMAPHORES == 1 ) && ( configSUPPORT_STATIC_ALLOCATION == 1 ) )
QueueHandle_t xQueueCreateCountingSemaphoreStatic( const UBaseType_t uxMaxCount,
                                                       const UBaseType_t uxInitialCount,
                                                       StaticQueue_t * pxStaticQueue )
{
    QueueHandle_t queue_handle_t;
    os_err_t ret;

    char name[OS_NAME_MAX];          /* "assume max num is MAX_UINT32"    */
    char idx[UINT32_TOCHAR_LEN];

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueCreateCountingSemaphoreStatic\n"));

    memset(name, 0, sizeof(name));
    memset(idx,  0, sizeof(idx));

    queue_handle_t = (QueueHandle_t)pxStaticQueue;
    if (OS_NULL == queue_handle_t)
    {
        return NULL;
    }

    oneos_itoa(s++, idx, DECIMAL_TYPE);
    strncat(name, "s_", strlen("s_"));
    strncat(name, idx, strlen(idx));
    ret = os_sem_init(QUEUEHANDLE_TO_OSSEM(queue_handle_t), name, uxInitialCount, uxMaxCount);
    queue_handle_t->queue_type = OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE;
    queue_handle_t->is_static = OS_TRUE;

    if (OS_EOK != ret)
    {
        return NULL;
    }
    else
    {
        return queue_handle_t;
    }

}
#endif

BaseType_t xQueueSemaphoreTake( QueueHandle_t xQueue,
                                TickType_t xTicksToWait )
{
    BaseType_t result;

    if ((OS_ADAPT_QUEUE_TYPE_MUTEX == (xQueue->queue_type)) ||
        (OS_ADAPT_QUEUE_TYPE_RECURSIVE_MUTEX == (xQueue->queue_type)) ||
        (OS_ADAPT_QUEUE_TYPE_BINARY_SEMAPHORE == (xQueue->queue_type)) ||
        (OS_ADAPT_QUEUE_TYPE_COUNTING_SEMAPHORE == (xQueue->queue_type)))
    {
        result = xQueueReceive(xQueue, OS_NULL, xTicksToWait);
    }
    else
    {
        FREERTOS_ADAPT_LOG(ADAPT_DEBUG_QUEUE, ("xQueueSemaphoreTake Not support orther queue type\n"));
        OS_ASSERT(0);
    }

    return result;
}

BaseType_t xQueueGenericReset( QueueHandle_t xQueue,
                               BaseType_t xNewQueue )
{
    if ( OS_ADAPT_QUEUE_TYPE_MESSAGEQUEUE == xQueue->queue_type )
    {
        os_mq_reset(QUEUEHANDLE_TO_OSMQ(xQueue));
    }
    else
    {

        FREERTOS_ADAPT_LOG(1, ("Warning: xQueueGenericReset not support\n"));
        OS_ASSERT(0);
    }

    return pdPASS;
}

