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
 * @file        task_port.c
 *
 * @brief       This file implements some port functions of FreeRTOS task.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-02   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include "FreeRTOS.h"
#include "portmacro.h"
#include "task.h"
#include "os_task.h"
#include "os_stddef.h"
#include "os_errno.h"
#include "os_assert.h"
#include "os_util.h"
#include "os_clock.h"
#include "os_memory.h"
#include "freertos_internal.h"

#define ADAPT_DEBUG_TASK     0
#define TASKHANDLE_TO_OSTASK(x) (&(x->os_task))

/* convert priority from freertos to OneOs
 * in freertos, 0 is the lowest priority, so convert "0"&"1" to OS_TASK_PRIORITY_MAX - pri - 1
 * the others convert to OS_TASK_PRIORITY_MAX - pri - 219 - 1
 */
os_uint8_t free_to_oneos_pri_convert(os_uint8_t pri)
{
    /* OS_TASK_PRIORITY_MAX = configMAX_PRIORITIES */
#if (OS_TASK_PRIORITY_MAX != configMAX_PRIORITIES)
    #error "OS_TASK_PRIORITY_MAX value must equal configMAX_PRIORITIES Value"
#endif

    OS_ASSERT(pri < configMAX_PRIORITIES);

    return OS_TASK_PRIORITY_MAX - pri - 1;
}

/*convert priority from oneos to freertos*/
os_uint8_t oneos_to_free_pri_convert(os_uint8_t pri)
{
    /* OS_TASK_PRIORITY_MAX = configMAX_PRIORITIES */
#if (OS_TASK_PRIORITY_MAX != configMAX_PRIORITIES)
    #error "OS_TASK_PRIORITY_MAX value must equal configMAX_PRIORITIES Value"
#endif

    OS_ASSERT(pri < OS_TASK_PRIORITY_MAX);

    return configMAX_PRIORITIES - pri - 1;
}

void freertos_cleanup(void *user_data)
{
    TaskHandle_t task_handle;

    task_handle = (TaskHandle_t)user_data;

    task_handle->os_task.cleanup = OS_NULL;
    if(task_handle->is_static == OS_TRUE)
    {
        return ;
    }
    os_free(task_handle->stack_begin);
    os_free(task_handle);
}

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
BaseType_t xTaskCreate( TaskFunction_t pxTaskCode,
                        const char * const pcName,     /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                        const configSTACK_DEPTH_TYPE usStackDepth,
                        void * const pvParameters,
                        UBaseType_t uxPriority,
                        TaskHandle_t * const pxCreatedTask )
{

#ifdef OS_USING_SYS_HEAP
#else
#error "OS_USING_SYS_HEAP should be defined in menuconfig, otherwise xxx_create will no use"
#endif
    TaskHandle_t task_handle_t;
    os_err_t ret;
    void *stack_begin;

    OS_ASSERT((( uxPriority & ( ~portPRIVILEGE_BIT ) ) < configMAX_PRIORITIES));

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("xTaskCreate %s\n", pcName));

    task_handle_t = (TaskHandle_t)os_malloc(sizeof(struct tskTaskControlBlock));
    if (OS_NULL == task_handle_t)
    {
        return pdFAIL;
    }

    stack_begin = os_malloc(usStackDepth * sizeof(StackType_t));
    if (OS_NULL == stack_begin)
    {
        os_free(task_handle_t);
        return pdFAIL;
    }

    ret = os_task_init(TASKHANDLE_TO_OSTASK(task_handle_t),
                       pcName,
                       pxTaskCode,
                       pvParameters,
                       stack_begin,
                       usStackDepth * sizeof(StackType_t),
                       free_to_oneos_pri_convert(uxPriority));

    if (OS_EOK != ret)
    {
        os_free(task_handle_t);
        os_free(stack_begin);
        return pdFAIL;
    }

    task_handle_t->is_static = OS_FALSE;
    task_handle_t->stack_begin = stack_begin;

    os_task_set_cleanup_callback(TASKHANDLE_TO_OSTASK(task_handle_t), freertos_cleanup, task_handle_t);

    if (OS_NULL != pxCreatedTask)
    {
        *pxCreatedTask = task_handle_t;
    }

    ret = os_task_startup(TASKHANDLE_TO_OSTASK(task_handle_t));
    if (OS_EOK != ret)
    {
        os_free(task_handle_t);
        os_free(stack_begin);
        return pdFAIL;
    }

    return pdPASS;
}
#endif

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
TaskHandle_t xTaskCreateStatic( TaskFunction_t pxTaskCode,
                                const char * const pcName,     /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                const uint32_t ulStackDepth,
                                void * const pvParameters,
                                UBaseType_t uxPriority,
                                StackType_t * const puxStackBuffer,
                                StaticTask_t * const pxTaskBuffer )
{
    os_err_t ret;

    OS_ASSERT((( uxPriority & ( ~portPRIVILEGE_BIT ) ) < configMAX_PRIORITIES));

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("xTaskCreateStatic %s\n", pcName));

    ret = os_task_init(TASKHANDLE_TO_OSTASK(pxTaskBuffer),
                       pcName,
                       pxTaskCode,
                       pvParameters,
                       puxStackBuffer,
                       ulStackDepth * sizeof(StackType_t),
                       free_to_oneos_pri_convert(uxPriority));
    if (OS_EOK != ret)
    {
        return NULL;
    }
    pxTaskBuffer->is_static = OS_TRUE;

    ret = os_task_startup(TASKHANDLE_TO_OSTASK(pxTaskBuffer));
    if (OS_EOK != ret)
    {
        return NULL;
    }

    return (TaskHandle_t)pxTaskBuffer;
}
#endif

#if ( INCLUDE_vTaskDelete == 1 )
void vTaskDelete( TaskHandle_t xTaskToDelete )
{
    os_task_t *os_task;
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("vTaskDelete\n"));

    if (OS_NULL == xTaskToDelete)
    {
        os_task = os_task_self();
    }
    else
    {
        os_task = TASKHANDLE_TO_OSTASK(xTaskToDelete);
    }
    os_task_deinit(os_task);
}
#endif

#if ( INCLUDE_vTaskDelay == 1 )
void vTaskDelay( const TickType_t xTicksToDelay )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("vTaskDelay %d\n", xTicksToDelay));
    os_task_tsleep((os_tick_t)xTicksToDelay);
}
#endif

#if ( INCLUDE_uxTaskPriorityGet == 1 )
UBaseType_t uxTaskPriorityGet( const TaskHandle_t xTask )
{
    os_task_t *os_task;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("uxTaskPriorityGet\n"));
    if (OS_NULL == xTask)
    {
        os_task = os_task_self();
    }
    else
    {
        os_task = TASKHANDLE_TO_OSTASK(xTask);
    }

    return (UBaseType_t)oneos_to_free_pri_convert(os_task_get_priority(os_task));
}
#endif

#if ( INCLUDE_uxTaskPriorityGet == 1 )
UBaseType_t uxTaskPriorityGetFromISR( const TaskHandle_t xTask )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("uxTaskPriorityGetFromISR\n"));
    return uxTaskPriorityGet(xTask);
}
#endif

#if ( ( INCLUDE_eTaskGetState == 1 ) || ( configUSE_TRACE_FACILITY == 1 ) || ( INCLUDE_xTaskAbortDelay == 1 ) )
eTaskState eTaskGetState( TaskHandle_t xTask )
{
    os_uint16_t     state;

    OS_ASSERT(xTask);

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("eTaskGetState\n"));

    state = os_task_get_state(TASKHANDLE_TO_OSTASK(xTask));

    if (state & OS_TASK_STATE_RUNNING)
    {
        return eRunning;
    }
    else if (state & OS_TASK_STATE_READY)
    {
        return eReady;
    }
    else if (state & OS_TASK_STATE_BLOCK)
    {
        return eBlocked;
    }
    else if (state & OS_TASK_STATE_SUSPEND)
    {
        return eSuspended;
    }
    else if (state & OS_TASK_STATE_SLEEP)
    {
        return eSuspended;
    }
    else
    {
        return eDeleted;
    }
}
#endif

#if ( INCLUDE_vTaskPrioritySet == 1 )
void vTaskPrioritySet( TaskHandle_t xTask,
                       UBaseType_t uxNewPriority )
{
    os_task_t *os_task;

    OS_ASSERT( ( ( uxNewPriority & ( ~portPRIVILEGE_BIT ) ) < configMAX_PRIORITIES ) );

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("vTaskPrioritySet\n"));

    if (OS_NULL == xTask)
    {
        os_task = os_task_self();
    }
    else
    {
        os_task = TASKHANDLE_TO_OSTASK(xTask);
    }

    os_task_set_priority(os_task, free_to_oneos_pri_convert(uxNewPriority));
}
#endif

#if ( INCLUDE_vTaskSuspend == 1 )
void vTaskSuspend( TaskHandle_t xTaskToSuspend )
{
    os_task_t *os_task;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("vTaskSuspend\n"));

    if (OS_NULL == xTaskToSuspend)
    {
        os_task = os_task_self();
    }
    else
    {
        os_task = TASKHANDLE_TO_OSTASK(xTaskToSuspend);
    }
    os_task_suspend(os_task);
}
#endif

#if ( INCLUDE_vTaskSuspend == 1 )
void vTaskResume( TaskHandle_t xTaskToResume )
{
    OS_ASSERT(xTaskToResume);

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("vTaskResume\n"));

    os_task_resume(TASKHANDLE_TO_OSTASK(xTaskToResume));
}
#endif

#if ( ( INCLUDE_xTaskResumeFromISR == 1 ) && ( INCLUDE_vTaskSuspend == 1 ) )
BaseType_t xTaskResumeFromISR( TaskHandle_t xTaskToResume )
{
    vTaskResume(xTaskToResume);

    return pdFALSE;
}
#endif

void vTaskStartScheduler( void )
{
    /* do nothing */
}

void vTaskEndScheduler( void )
{
    os_schedule_lock();
}

void vTaskSuspendAll( void )
{
    os_schedule_lock();
}

BaseType_t xTaskResumeAll( void )
{
    os_schedule_unlock();

    return pdFALSE;
}

void xPortYIELD(void)
{
    OS_KERNEL_INIT();
    OS_KERNEL_ENTER();
    OS_KERNEL_EXIT_SCHED();
}

TickType_t xTaskGetTickCount( void )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("xTaskGetTickCount\n"));
    return (TickType_t)os_tick_get();
}

TickType_t xTaskGetTickCountFromISR( void )
{
    return (TickType_t)os_tick_get();
}

UBaseType_t uxTaskGetNumberOfTasks( void )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("uxTaskGetNumberOfTasks\n"));
    return (UBaseType_t)os_task_get_total_count();
}

char * pcTaskGetName( TaskHandle_t xTaskToQuery )
{
    os_task_t *os_task;
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("pcTaskGetName\n"));
    if (OS_NULL == xTaskToQuery)
    {
        os_task = os_task_self();
    }
    else
    {
        os_task = TASKHANDLE_TO_OSTASK(xTaskToQuery);
    }
    return (char *)os_task_name(os_task);
}

TaskHandle_t xTaskGetHandle( const char * pcNameToQuery )
{
    TaskHandle_t task_handle_t;
    os_task_t *os_task;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("xTaskGetHandle\n"));

    os_task = os_task_find(pcNameToQuery);
    task_handle_t = os_container_of(os_task, struct tskTaskControlBlock, os_task);
    return task_handle_t;
}

#if ( ( INCLUDE_xTaskGetCurrentTaskHandle == 1 ) || ( configUSE_MUTEXES == 1 ) )
TaskHandle_t xTaskGetCurrentTaskHandle( void )
{
    TaskHandle_t task_handle_t;
    os_task_t *os_task;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("xTaskGetCurrentTaskHandle\n"));

    os_task = os_task_self();
    task_handle_t = os_container_of(os_task, struct tskTaskControlBlock, os_task);
    return task_handle_t;
    }
#endif

#if ( configUSE_TICKLESS_IDLE != 0 )
void vTaskStepTick( const TickType_t xTicksToJump )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("vTaskStepTick\n"));

    os_tick_set(os_tick_get() + (os_tick_t)xTicksToJump);
}
#endif

#if (( INCLUDE_xTaskGetSchedulerState == 1 ) || ( configUSE_TIMERS == 1 ))
BaseType_t xTaskGetSchedulerState( void )
{
    BaseType_t state;

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TASK, ("xTaskGetSchedulerState\n"));

    if (OS_NULL == os_task_self())
    {
        state = taskSCHEDULER_NOT_STARTED;
    }
    else if (OS_TRUE == os_is_schedule_locked())
    {
        state = taskSCHEDULER_SUSPENDED;
    }
    else
    {
        state = taskSCHEDULER_RUNNING;
    }

    return state;
}
#endif

