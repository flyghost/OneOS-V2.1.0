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
 * @file        cmsis_timer.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 timer function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-02   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_clock.h>
#include <os_errno.h>
#include <os_timer.h>
#include <os_task.h>
#include <os_util.h>
#include <string.h>
#include <arch_interrupt.h>

#include "cmsis_internal.h"

osTimerId_t osTimerNew(osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr)
{
    char               name[OS_NAME_MAX];
    timer_cb_t        *timer_cb;
    static os_uint16_t timer_number = 1U;

    if ((OS_NULL == func) || ((type != osTimerOnce) && (type != osTimerPeriodic)))
    {
        return (osTimerId_t)OS_NULL;
    }

    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "timer%02d", timer_number++);
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        timer_cb = os_malloc(sizeof(timer_cb_t));
        if (OS_NULL == timer_cb)
        {
            return (osTimerId_t)OS_NULL;
        }
        memset(timer_cb, 0, sizeof(timer_cb_t));
        timer_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(timer_cb_t))
        {
            timer_cb = attr->cb_mem;
            timer_cb->flags = 0;
        }
        else
        {
            return (osTimerId_t)OS_NULL;
        }
    }

    strncpy(&timer_cb->name[0], name, OS_NAME_MAX);
    timer_cb->id = IdTimer;
    os_timer_init(&timer_cb->timer, name, func, argument, 0, (type == osTimerPeriodic ? OS_TIMER_FLAG_PERIODIC : OS_TIMER_FLAG_ONE_SHOT));

    return (osTimerId_t)timer_cb;
}

const char *osTimerGetName(osTimerId_t timer_id)
{
    timer_cb_t *timer_cb;

    timer_cb = (timer_cb_t *)timer_id;

    if ((OS_NULL == timer_cb) || (IdTimer != timer_cb->id))
    {
        return OS_NULL;
    }

    return timer_cb->name;
}

osStatus_t osTimerStart(osTimerId_t timer_id, uint32_t ticks)
{
    os_err_t    result;
    timer_cb_t *timer_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    timer_cb = (timer_cb_t *)timer_id;

    if ((OS_NULL == timer_cb) || (0 == ticks) || (IdTimer != timer_cb->id))
    {
        return osErrorParameter;
    }

    result = os_timer_set_timeout_ticks(&(timer_cb->timer), ticks);
    if (OS_EOK != result)
    {
        return osError;
    }

    result = os_timer_start(&(timer_cb->timer));
    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osError;
    }
}

osStatus_t osTimerStop(osTimerId_t timer_id)
{
    os_err_t    result;
    timer_cb_t *timer_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    timer_cb = (timer_cb_t *)timer_id;

    if ((OS_NULL == timer_cb) || (IdTimer != timer_cb->id))
    {
        return osErrorParameter;
    }

    result = os_timer_stop(&timer_cb->timer);

    if (OS_EOK == result)
    {
        return osOK;
    }
    else
    {
        return osErrorResource;
    }
}

uint32_t osTimerIsRunning(osTimerId_t timer_id)
{
    timer_cb_t *timer_cb;

    timer_cb = (timer_cb_t *)timer_id;

    if ((OS_NULL == timer_cb) || (IdTimer != timer_cb->id))
    {
        return 0U;
    }

    if (os_timer_is_active(&timer_cb->timer))
    {
        return 1;
    }
    else
        return 0U;
}

osStatus_t osTimerDelete(osTimerId_t timer_id)
{
    timer_cb_t *timer_cb;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    timer_cb = (timer_cb_t *)timer_id;

    if ((OS_NULL == timer_cb) || (IdTimer != timer_cb->id))
    {
        return osErrorParameter;
    }
    timer_cb->id = IdInvalid;
    os_timer_deinit(&timer_cb->timer);

    if (timer_cb->flags & SYS_MALLOC_CTRL_BLK)
	{
        os_free(timer_cb);
	}

    return osOK;
}

osStatus_t osDelay(uint32_t ticks)
{

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    os_task_tsleep(ticks);

    return osOK;
}

osStatus_t osDelayUntil(uint32_t ticks)
{
    uint64_t cur_ticks;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    cur_ticks = os_tick_get();

    if (ticks == cur_ticks)
    {
        return osOK;
    }
    else if (ticks > cur_ticks)
    {
        os_task_tsleep(ticks - cur_ticks);
    }
    else
    {
        if((((os_uint32_t)(-1)) - cur_ticks + ticks) > (OS_TICK_MAX / 2))
        {
            return osErrorParameter;
        }

        os_task_tsleep(((os_uint32_t)(-1)) - cur_ticks + ticks);
    }

    return osOK;
}

