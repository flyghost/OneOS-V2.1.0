/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_sdmmc_osa.h"

#include <string.h>
#include <os_event.h>
#include <os_types.h>
#include <dlog.h>
#include <os_clock.h>
#include <os_task.h>
#include <os_errno.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sdmmc_osa"
#include <drv_log.h>


/*******************************************************************************
 * Definitons
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * brief Initialize OSA.
 */
void SDMMC_OSAInit(void)
{
    /* Intentional empty */
}

/*!
 * brief OSA Create event.
 * param event handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventCreate(void *eventHandle)
{
    assert(eventHandle != NULL);
    
     if(os_event_init((os_event_t *)eventHandle, "sdmmc_event") != OS_EOK)
        return kStatus_Fail;
     else
        return kStatus_Success;
}

/*!
 * brief Wait event.
 *
 * param eventHandle The event type
 * param eventType Timeout time in milliseconds.
 * param timeoutMilliseconds timeout value in ms.
 * param event event flags.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventWait(void *eventHandle, uint32_t eventType, uint32_t timeoutMilliseconds, uint32_t *event)
{
    assert(eventHandle != NULL);
    
    if (os_event_recv((os_event_t *)eventHandle, eventType, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, 
        os_tick_from_ms(timeoutMilliseconds), (os_uint32_t *)event) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "wait event timeout");
        return kStatus_Fail;
    } 

    return kStatus_Success;
}

/*!
 * brief set event.
 * param event event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventSet(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);

    if(os_event_send((os_event_t *)eventHandle, eventType) != OS_EOK)
       return kStatus_Fail;
    else
       return kStatus_Success;
}

/*!
 * brief Get event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * param flag pointer to store event value.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventGet(void *eventHandle, uint32_t eventType, uint32_t *flag)
{
    assert(eventHandle != NULL);
    assert(flag != NULL);
    
    os_event_t *event = eventHandle;
    
    *flag = event->set & eventType;

    return kStatus_Success;
}

/*!
 * brief clear event flag.
 * param eventHandle event handle.
 * param eventType The event type
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAEventClear(void *eventHandle, uint32_t eventType)
{
    assert(eventHandle != NULL);

    os_event_t *event = eventHandle;
    
    event->set &= ~eventType;

    return kStatus_Success;
}

/*!
 * brief Delete event.
 * param event The event handle.
 */
status_t SDMMC_OSAEventDestroy(void *eventHandle)
{
    assert(eventHandle != NULL);

    if(os_event_deinit((os_event_t *)eventHandle) != OS_EOK)
       return kStatus_Fail;
    else
       return kStatus_Success;
}

/*!
 * brief Create a mutex.
 * param mutexHandle mutex handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAMutexCreate(void *mutexHandle)
{
    assert(mutexHandle != NULL);

    if(os_mutex_init((os_mutex_t *)mutexHandle, "sdmmc_mutex", OS_FALSE) != OS_EOK)
       return kStatus_Fail;
    else
       return kStatus_Success;
}

/*!
 * brief set event.
 * param mutexHandle mutex handle.
 * param millisec The maximum number of milliseconds to wait for the mutex.
 *                 If the mutex is locked, Pass the value osaWaitForever_c will
 *                 wait indefinitely, pass 0 will return KOSA_StatusTimeout
 *                 immediately.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAMutexLock(void *mutexHandle, uint32_t millisec)
{
    assert(mutexHandle != NULL);
    
    if(os_mutex_lock((os_mutex_t *)mutexHandle, os_tick_from_ms(millisec)) != OS_EOK)
       return kStatus_Fail;
    else
       return kStatus_Success;
}

/*!
 * brief Get event flag.
 * param mutexHandle mutex handle.
 * retval kStatus_Fail or kStatus_Success.
 */
status_t SDMMC_OSAMutexUnlock(void *mutexHandle)
{
    assert(mutexHandle != NULL);

    if(os_mutex_unlock((os_mutex_t *)mutexHandle) != OS_EOK)
       return kStatus_Fail;
    else
       return kStatus_Success;
}

/*!
 * brief Delete mutex.
 * param mutexHandle The mutex handle.
 */
status_t SDMMC_OSAMutexDestroy(void *mutexHandle)
{
    assert(mutexHandle != NULL);

    if(os_mutex_deinit((os_mutex_t *)mutexHandle) != OS_EOK)
       return kStatus_Fail;
    else
       return kStatus_Success;
}

/*!
 * brief sdmmc delay.
 * param milliseconds time to delay
 */
void SDMMC_OSADelay(uint32_t milliseconds)
{
    os_task_msleep(milliseconds);
}

/*!
 * brief sdmmc delay us.
 * param microseconds time to delay
 * return actual delayed microseconds
 */
uint32_t SDMMC_OSADelayUs(uint32_t microseconds)
{
    uint32_t milliseconds = microseconds / 1000U + ((microseconds % 1000U) == 0U ? 0U : 1U);
    os_task_msleep(milliseconds);
    return milliseconds * 1000U;

}

