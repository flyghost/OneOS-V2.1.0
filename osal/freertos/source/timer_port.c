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
 * @brief       This file implements some port functions of FreeRTOS timer.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-05   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <string.h>
#include "FreeRTOS.h"
#include "timers.h"
#include "portmacro.h"
#include "os_stddef.h"
#include "os_errno.h"
#include "os_assert.h"
#include "os_util.h"
#include "os_memory.h"
#include "os_clock.h"
#include "os_timer.h"
#include "freertos_internal.h"

#define ADAPT_DEBUG_TIMER 0
#define TIMERHANDLE_TO_OSTIMER(x) (&(x->os_timer))

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
TimerHandle_t xTimerCreate( const char * const pcTimerName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                            const TickType_t xTimerPeriodInTicks,
                            const UBaseType_t uxAutoReload,
                            void * const pvTimerID,
                            TimerCallbackFunction_t pxCallbackFunction )
{
    TimerHandle_t     timer_handle_t;
    char            name[OS_NAME_MAX];     /* "assume max num is MAX_UINT32" */

    memset(name, 0, sizeof(name));
    strncpy(name, pcTimerName, strlen(pcTimerName) < OS_NAME_MAX ? strlen(pcTimerName) : (OS_NAME_MAX-1));

    timer_handle_t = (TimerHandle_t)os_malloc(sizeof(struct tmrTimerControl));
    if(OS_NULL == timer_handle_t)
    {
        return NULL;
    }
    os_timer_init(TIMERHANDLE_TO_OSTIMER(timer_handle_t),
                    name,
                    (void (*)(void *))pxCallbackFunction,
                    OS_NULL,
                    xTimerPeriodInTicks,
                    (uxAutoReload == 1 ? OS_TIMER_FLAG_PERIODIC : OS_TIMER_FLAG_ONE_SHOT));

    timer_handle_t->pvTimerID = pvTimerID;
    timer_handle_t->is_static = OS_FALSE;

    return timer_handle_t;
}
#endif

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
TimerHandle_t xTimerCreateStatic( const char * const pcTimerName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
                                 const TickType_t xTimerPeriodInTicks,
                                 const UBaseType_t uxAutoReload,
                                 void * const pvTimerID,
                                 TimerCallbackFunction_t pxCallbackFunction,
                                 StaticTimer_t * pxTimerBuffer )
{
    TimerHandle_t     timer_handle_t;
    char            name[OS_NAME_MAX];     /* "assume max num is MAX_UINT32" */

    memset(name, 0, sizeof(name));
    strncpy(name, pcTimerName, strlen(pcTimerName) < OS_NAME_MAX ? strlen(pcTimerName) : (OS_NAME_MAX-1));
    timer_handle_t = (TimerHandle_t)pxTimerBuffer;

    os_timer_init(TIMERHANDLE_TO_OSTIMER(timer_handle_t),
                    name,
                    (void (*)(void *))pxCallbackFunction,
                    OS_NULL,
                    xTimerPeriodInTicks,
                    (uxAutoReload == 1 ? OS_TIMER_FLAG_PERIODIC : OS_TIMER_FLAG_ONE_SHOT));

    timer_handle_t->pvTimerID = pvTimerID;
    timer_handle_t->is_static = OS_TRUE;

    return timer_handle_t;
}
#endif

BaseType_t xTimerGenericCommand( TimerHandle_t xTimer,
                                     const BaseType_t xCommandID,
                                     const TickType_t xOptionalValue,
                                     BaseType_t * const pxHigherPriorityTaskWoken,
                                     const TickType_t xTicksToWait )
{
    os_err_t        ret;

    switch (xCommandID)
    {
        case tmrCOMMAND_START_DONT_TRACE:
        case tmrCOMMAND_RESET:
        case tmrCOMMAND_START:
        case tmrCOMMAND_RESET_FROM_ISR:
        case tmrCOMMAND_START_FROM_ISR:
            ret = os_timer_start(TIMERHANDLE_TO_OSTIMER(xTimer));
            break;
        case tmrCOMMAND_STOP:
        case tmrCOMMAND_STOP_FROM_ISR:
            ret = os_timer_stop(TIMERHANDLE_TO_OSTIMER(xTimer));
            break;
        case tmrCOMMAND_DELETE:
            os_timer_deinit(TIMERHANDLE_TO_OSTIMER(xTimer));
            if (OS_FALSE == xTimer->is_static)
            {
                os_free(xTimer);
            }
            ret= OS_EOK;
            break;
        case tmrCOMMAND_CHANGE_PERIOD:
        case tmrCOMMAND_CHANGE_PERIOD_FROM_ISR:
            ret = os_timer_set_timeout_ticks(TIMERHANDLE_TO_OSTIMER(xTimer), xOptionalValue);
            break;
        default:
            FREERTOS_ADAPT_LOG(ADAPT_DEBUG_TIMER, ("timer cmd(%d) not realize yet\r\n", xCommandID));
            ret = OS_ENOSYS;
            break;
    }

    if (OS_NULL != pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    return (ret == OS_EOK) ? pdPASS : pdFAIL;
}

void * pvTimerGetTimerID( const TimerHandle_t xTimer )
{
    OS_ASSERT(OS_NULL != xTimer);
    return xTimer->pvTimerID;
}

const char * pcTimerGetName( TimerHandle_t xTimer )
{
    OS_ASSERT(OS_NULL != xTimer);
    return TIMERHANDLE_TO_OSTIMER(xTimer)->name;
}

void vTimerSetTimerID( TimerHandle_t xTimer,
                       void * pvNewID )
{
    OS_ASSERT(OS_NULL != xTimer);
    xTimer->pvTimerID = pvNewID;
}

BaseType_t xTimerIsTimerActive( TimerHandle_t xTimer )
{
    return (BaseType_t)os_timer_is_active(TIMERHANDLE_TO_OSTIMER(xTimer));
}

void vTimerSetReloadMode( TimerHandle_t xTimer,
                          const UBaseType_t uxAutoReload )
{
    if(pdTRUE == uxAutoReload)
    {
        os_timer_set_periodic(TIMERHANDLE_TO_OSTIMER(xTimer));
    }
    else
    {
        os_timer_set_oneshot(TIMERHANDLE_TO_OSTIMER(xTimer));
    }
}

UBaseType_t uxTimerGetReloadMode( TimerHandle_t xTimer )
{
    return (UBaseType_t)os_timer_is_periodic(TIMERHANDLE_TO_OSTIMER(xTimer));
}

TickType_t xTimerGetPeriod( TimerHandle_t xTimer )
{
    return (TickType_t)os_timer_get_timeout_ticks(TIMERHANDLE_TO_OSTIMER(xTimer));
}

TickType_t xTimerGetExpiryTime( TimerHandle_t xTimer )
{
    return (TickType_t)(os_tick_get() + os_timer_get_remain_ticks(TIMERHANDLE_TO_OSTIMER(xTimer)));
}

