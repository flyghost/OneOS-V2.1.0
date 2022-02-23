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
 * @file        event_port.c
 *
 * @brief       This file implements some port functions of FreeRTOS event.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-15   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include "FreeRTOS.h"
#include "event_groups.h"
#include <os_event.h>
#include <os_memory.h>
#include "freertos_internal.h"

#define ADAPT_DEBUG_EVENT            0
#define TASKHANDLE_TO_OSEVENT(x) (&(x->os_event))

static uint32_t e = 0;

#if ( configSUPPORT_DYNAMIC_ALLOCATION == 1 )
EventGroupHandle_t xEventGroupCreate( void )
{
    os_err_t     ret;
    EventGroupHandle_t event_handle_t;
    char            name[OS_NAME_MAX] = "e_";
    char            idx[UINT32_TOCHAR_LEN] = {0,};

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_EVENT, ("xEventGroupCreate\n"));

    oneos_itoa(e++, idx, DECIMAL_TYPE);
    strncat(name, idx, strlen(idx));
    event_handle_t = (EventGroupHandle_t)os_malloc(sizeof (struct EventGroupDef_t));
    if (OS_NULL == event_handle_t)
    {
        return NULL;
    }

    ret = os_event_init(TASKHANDLE_TO_OSEVENT(event_handle_t), name);
    if (OS_EOK != ret)
    {
        os_free(event_handle_t);
        return NULL;
    }
    event_handle_t->is_static = OS_FALSE;

    return event_handle_t;
}
#endif

#if ( configSUPPORT_STATIC_ALLOCATION == 1 )
EventGroupHandle_t xEventGroupCreateStatic( StaticEventGroup_t * pxEventGroupBuffer )
{
    os_err_t     ret;
    EventGroupHandle_t event_handle_t;
    char            name[OS_NAME_MAX] = "e_";
    char            idx[UINT32_TOCHAR_LEN] = {0,};

    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_EVENT, ("xEventGroupCreateStatic\n"));

    oneos_itoa(e++, idx, DECIMAL_TYPE);
    strncat(name, idx, strlen(idx));
    event_handle_t = (EventGroupHandle_t)pxEventGroupBuffer;

    ret = os_event_init(TASKHANDLE_TO_OSEVENT(event_handle_t), name);
    if (OS_EOK != ret)
    {
        return NULL;
    }
    event_handle_t->is_static = OS_TRUE;

    return event_handle_t;
}
#endif

EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToWaitFor,
                                 const BaseType_t xClearOnExit,
                                 const BaseType_t xWaitForAllBits,
                                 TickType_t xTicksToWait )
{
    os_uint8_t option = 0;
    os_uint32_t event = 0;

    if (xClearOnExit)
    {
        option |= OS_EVENT_OPTION_CLEAR;
    }

    if (xWaitForAllBits)
    {
        option |= OS_EVENT_OPTION_AND;
    }
    else
    {
        option |= OS_EVENT_OPTION_OR;
    }

    (void)os_event_recv(TASKHANDLE_TO_OSEVENT(xEventGroup), uxBitsToWaitFor, option, xTicksToWait, &event);

    return event;
}

EventBits_t xEventGroupClearBits( EventGroupHandle_t xEventGroup,
                                  const EventBits_t uxBitsToClear )
{
    os_uint32_t event;
    event = os_event_get(TASKHANDLE_TO_OSEVENT(xEventGroup));
    os_event_clear(TASKHANDLE_TO_OSEVENT(xEventGroup), uxBitsToClear);
    return (EventBits_t)event;
}

#if ( configUSE_TRACE_FACILITY == 1 )
BaseType_t xEventGroupClearBitsFromISR( EventGroupHandle_t xEventGroup,
                                       const EventBits_t uxBitsToClear )
{
    os_bool_t ret;
    ret = os_event_clear(TASKHANDLE_TO_OSEVENT(xEventGroup), uxBitsToClear);
    if (OS_EOK == ret)
    {
        return pdPASS;
    }
    else
    {
        return pdFAIL;
    }
}
#endif

EventBits_t xEventGroupGetBitsFromISR( EventGroupHandle_t xEventGroup )
{
    return (EventBits_t)os_event_get(TASKHANDLE_TO_OSEVENT(xEventGroup));
}

void vEventGroupDelete( EventGroupHandle_t xEventGroup )
{
    os_event_deinit(TASKHANDLE_TO_OSEVENT(xEventGroup));
    if (OS_FALSE == xEventGroup->is_static)
    {
        os_free(xEventGroup);
    }
}

EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup,
                                const EventBits_t uxBitsToSet )
{
    os_uint32_t event;
    os_event_send(TASKHANDLE_TO_OSEVENT(xEventGroup), uxBitsToSet);
    event = os_event_get(TASKHANDLE_TO_OSEVENT(xEventGroup));
    return (EventBits_t)event;
}

#if ( configUSE_TRACE_FACILITY == 1 )
BaseType_t xEventGroupSetBitsFromISR( EventGroupHandle_t xEventGroup,
                                      const EventBits_t uxBitsToSet,
                                      BaseType_t * pxHigherPriorityTaskWoken )
{
    FREERTOS_ADAPT_LOG(ADAPT_DEBUG_EVENT, ("xEventGroupSetBitsFromISR\n"));

    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }

    if (OS_EOK == (os_event_send(TASKHANDLE_TO_OSEVENT(xEventGroup), uxBitsToSet)))
    {
        return pdPASS;
    }
    else
    {
        return pdFAIL;
    }
}
#endif

EventBits_t xEventGroupSync( EventGroupHandle_t xEventGroup,
                             const EventBits_t uxBitsToSet,
                             const EventBits_t uxBitsToWaitFor,
                             TickType_t xTicksToWait )
{
    os_uint32_t event = 0;

    OS_ASSERT(xEventGroup);

    (void)os_event_sync(TASKHANDLE_TO_OSEVENT(xEventGroup), uxBitsToSet, uxBitsToWaitFor, xTicksToWait, &event);

    return event;
}

