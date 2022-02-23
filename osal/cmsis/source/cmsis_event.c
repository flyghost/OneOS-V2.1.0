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
 * @file        cmsis_event.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 event function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_errno.h>
#include <os_event.h>
#include <os_util.h>
#include <string.h>
#include <arch_interrupt.h>

#include "cmsis_internal.h"

#ifdef OS_USING_EVENT
osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t *attr)
{
    char               name[OS_NAME_MAX];
    event_cb_t        *event_cb;
    static os_uint16_t event_number = 1U;
    os_err_t ret;

    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "event%02d", event_number++);
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        event_cb = os_malloc(sizeof(event_cb_t));
        if (OS_NULL == event_cb)
        {
            return (osEventFlagsId_t)OS_NULL;
        }
        memset(event_cb, 0, sizeof(event_cb_t));

        event_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(event_cb_t))
        {
            event_cb = attr->cb_mem;
            event_cb->flags = 0;
        }
        else
        {
            return (osEventFlagsId_t)OS_NULL;
        }
    }
    strncpy(&event_cb->name[0], name, OS_NAME_MAX);
    event_cb->id = IdEventFlags;

    ret = os_event_init(&event_cb->event, name);
    if (OS_EOK != ret)
    {
        if (event_cb->flags & SYS_MALLOC_CTRL_BLK)
        {
            os_free(event_cb);
        }
        return (osEventFlagsId_t)OS_NULL;
    }

    return (osEventFlagsId_t)event_cb;
}

const char *osEventFlagsGetName(osEventFlagsId_t ef_id)
{
    event_cb_t *event_cb;

    event_cb = (event_cb_t *)ef_id;

    if ((OS_NULL == event_cb) || (IdEventFlags != event_cb->id))
    {
        return OS_NULL;
    }

    return event_cb->name;
}

uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags)
{
    os_err_t    result;
    os_uint32_t set_flags;
    event_cb_t *event_cb;

    event_cb = (event_cb_t *)ef_id;

    /* Check parameters */
    if ((OS_NULL == event_cb) || (IdEventFlags != event_cb->id))
    {
        return ((uint32_t)osFlagsErrorParameter);
    }

    /* Check flag value to avoid highest bits set */
    if((flags&(~(OS_UINT32_MAX >> 1U))) != 0)
    {
        return osFlagsErrorParameter;
    }

    result    = os_event_send(&(event_cb->event), flags);

    set_flags = os_event_get(&event_cb->event);

    if (OS_EOK == result)
    {

        return set_flags;
    }
    else
    {
        return osFlagsError;
    }
}

uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags)
{
    os_uint32_t         set_flags;
    event_cb_t         *event_cb;

    event_cb = (event_cb_t *)ef_id;

    if ((OS_NULL == event_cb) || (IdEventFlags != event_cb->id))
    {
        return ((uint32_t)osFlagsErrorParameter);
    }

    /* Check flag value to avoid highest bits clear */
    if((flags&(~(OS_UINT32_MAX >> 1U))) != 0)
    {
        return osFlagsErrorParameter;
    }

    set_flags = os_event_get(&event_cb->event);
    os_event_clear(&event_cb->event, flags);

    return set_flags;
}

uint32_t osEventFlagsGet(osEventFlagsId_t ef_id)
{
    event_cb_t *event_cb;

    event_cb = (event_cb_t *)ef_id;

    if ((OS_NULL == event_cb) || (IdEventFlags != event_cb->id))
    {
        return 0U;
    }

     return (uint32_t)os_event_get(&event_cb->event);
}

uint32_t osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout)
{
    os_err_t    result;
    os_uint32_t rt_recv;
    os_uint32_t  rt_options = 0U;
    event_cb_t *event_cb;

    event_cb = (event_cb_t *)ef_id;

    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    if ((OS_NULL == event_cb) || (IdEventFlags != event_cb->id))
    {
        return ((uint32_t)osFlagsErrorParameter);
    }

    if (options & osFlagsWaitAll)
    {
        rt_options |= OS_EVENT_OPTION_AND;
    }
    else
    {
        rt_options |= OS_EVENT_OPTION_OR;
    }

    if (!(options & osFlagsNoClear))
    {
        rt_options |= OS_EVENT_OPTION_CLEAR;
    }

    result = os_event_recv(&event_cb->event, flags, rt_options, (os_tick_t)timeout, &rt_recv);

    if (OS_EOK == result)
    {
        return rt_recv;
    }
    else if (OS_EEMPTY == result)
    {
        return osFlagsErrorResource;
    }
    else if (OS_ETIMEOUT == result)
    {
        return osFlagsErrorTimeout;
    }
    else
    {
        return osFlagsErrorUnknown;
    }
}

osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id)
{
    event_cb_t *event_cb;
    os_err_t ret;
    osStatus_t status;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    event_cb = (event_cb_t *)ef_id;

    if ((OS_NULL == event_cb) || (IdEventFlags != event_cb->id))
    {
        return osErrorParameter;
    }
    event_cb->id = IdInvalid;
    ret = os_event_deinit(&event_cb->event);
    if (OS_EOK == ret)
    {
        status = osOK;
    }
    else
    {
        status = osErrorResource;
    }

    if (event_cb->flags & SYS_MALLOC_CTRL_BLK)
    {
        os_free(event_cb);
    }

    return status;
}
#endif  /* OS_USING_EVENT */

