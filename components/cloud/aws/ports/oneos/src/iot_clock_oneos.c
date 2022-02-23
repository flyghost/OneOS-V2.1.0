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
 * @file        iot_clock_oneos.c
 * 
 * @brief       Implementation of the functions in iot_clock.h for OneOS systems. 
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-08-25   OneOs Team      First Version
 ***********************************************************************************************************************
 */

/* The config header is always included first. */
#include "iot_config.h"

/* Standard includes. */
#include <stdio.h>
#include <sys/time.h>

/* Platform clock include. */
#include "platform/iot_clock.h"

/* Configure logs for the functions in this file. */
#ifdef IOT_LOG_LEVEL_PLATFORM
    #define LIBRARY_LOG_LEVEL        IOT_LOG_LEVEL_PLATFORM
#else
    #ifdef IOT_LOG_LEVEL_GLOBAL
        #define LIBRARY_LOG_LEVEL    IOT_LOG_LEVEL_GLOBAL
    #else
        #define LIBRARY_LOG_LEVEL    IOT_LOG_NONE
    #endif
#endif

#define LIBRARY_LOG_NAME    ( "CLOCK" )
#include "iot_logging_setup.h"

/* When building tests, the Unity framework's malloc overrides are used to track
 * calls to platform resource creation and destruction. This ensures that all
 * platform resources are destroyed before the tests finish. When not testing,
 * define the Unity malloc functions to nothing. */
#if IOT_BUILD_TESTS == 1
#include "unity_fixture_malloc_overrides.h"
#else
#define UnityMalloc_AllocateResource()    true
#define UnityMalloc_FreeResource()
#endif

/**
 * @brief The format of timestrings printed in logs.
 *
 * For more information on timestring formats, see [this link.]
 * (http://pubs.opengroup.org/onlinepubs/9699919799/functions/strftime.html)
 */
#define TIMESTRING_FORMAT              ( "%F %R:%S" )

#if OS_TIMER_TASK_STACK_SIZE < 2048
#error "OS_TIMER_TASK_STACK_SIZE need more than or equal to 2048 bytes"
#endif

/*-----------------------------------------------------------*/

bool IotClock_GetTimestring( char * pBuffer,
                             size_t bufferSize,
                             size_t * pTimestringLength )
{
    bool status = true;
    const time_t nowTime = time(OS_NULL);
    struct tm localTime = {0};
    size_t timeStringLength = 0;

    /* localtime_r is the thread-safe variant of localtime. Its return value
     * should be the pointer to the localTime struct. */
    if (localtime_r(&nowTime, &localTime) != &localTime)
    {
        status = false;
    }

    if (status == true)
    {
        /* Convert the localTime struct to a string. */
        timeStringLength = strftime( pBuffer, bufferSize, TIMESTRING_FORMAT, &localTime );

        /* Check for error from strftime. */
        if( timeStringLength == 0 )
        {
            status = false;
        }
        else
        {
            /* Set the output parameter. */
            *pTimestringLength = timeStringLength;
        }
    }

    return status;
}

/*-----------------------------------------------------------*/

uint64_t IotClock_GetTimeMs( void )
{
    time_t nowTime = 0;
    
    nowTime = time(OS_NULL);

    return (uint64_t)(nowTime * 1000);
}

/*-----------------------------------------------------------*/

void IotClock_SleepMs( uint32_t sleepTimeMs )
{
    os_task_msleep(sleepTimeMs);
}

/*-----------------------------------------------------------*/

bool IotClock_TimerCreate( IotTimer_t * pNewTimer,
                           IotThreadRoutine_t expirationRoutine,
                           void * pArgument )
{
    bool status = true;
    os_timer_t *iotTimer = OS_NULL;
    char name[OS_NAME_MAX] = {0};
    static os_uint16_t iotTimerNum = 0;

    if (!pNewTimer)
    {   
        IotLogError("Failed to create new timer, invalid argument.");
        
        return false;
    }
    
    status = UnityMalloc_AllocateResource();
    
    snprintf(name, sizeof(name), "iotTimer%02d", iotTimerNum++);

    if (true == status)
    {
        IotLogDebug("Creating new timer %p.", pNewTimer);

        iotTimer = os_timer_create(name,
                                   expirationRoutine,
                                   pArgument,
                                   0,
                                   OS_TIMER_FLAG_ONE_SHOT);
        
        if (OS_NULL == iotTimer)
        {
            IotLogError("Failed to create new timer %p.", pNewTimer);
            
            UnityMalloc_FreeResource();
            
            status = false;
        }
        else
        {
            *pNewTimer = iotTimer;
            
            status = true;
        }
    }

    return status;
}

/*-----------------------------------------------------------*/

void IotClock_TimerDestroy( IotTimer_t * pTimer )
{
    os_timer_t *iotTimer = *pTimer;
    
    if (!iotTimer)
    {
        IotLogError("Failed to destroy timer, invalid argument.");

        abort();
    }
    
    IotLogDebug("Destroying timer %p.", pTimer);

    /* Decrement the number of platform resources in use. */
    UnityMalloc_FreeResource();

    os_timer_stop(iotTimer);
    
    if (os_timer_destroy(iotTimer) != OS_EOK)
    {
        /* This block should not be reached; log an error and abort if it is. */
        IotLogError("Failed to destroy timer %p.", pTimer);

        abort();
    }
}

/*-----------------------------------------------------------*/

bool IotClock_TimerArm( IotTimer_t * pTimer,
                        uint32_t relativeTimeoutMs,
                        uint32_t periodMs )
{
    os_err_t timerError = 0;
    os_timer_t *iotTimer = *pTimer;
    os_tick_t initialTimeoutTick = 0;
    os_tick_t rescheduleTimeoutTick = 0;

    if (!iotTimer)
    {
        IotLogError("Failed to start timer, invalid argument.");

        abort();
    }
    
    initialTimeoutTick = os_tick_from_ms(relativeTimeoutMs); /* convert milliseconds to ticks */

    if (0 == periodMs) /* oneshot timer */
    {
        os_timer_stop(iotTimer);

        os_timer_set_timeout_ticks(iotTimer, initialTimeoutTick);

        timerError = os_timer_start(iotTimer);

        if (OS_EOK != timerError)
        {
            IotLogError("Failed to start timer.");
            
            return false;
        }
    }
    else /* periodic timer */
    {   
        os_timer_stop(iotTimer);

        os_timer_set_timeout_ticks(iotTimer, initialTimeoutTick); /* initial expiration time in OneOS clock ticks. */

        timerError = os_timer_start(iotTimer);

        if (OS_EOK != timerError)
        {
            IotLogError("Failed to start timer.");
            
            return false;
        }

        rescheduleTimeoutTick = os_tick_from_ms(periodMs); /* period time after initial timeout */
        
        os_timer_set_periodic(iotTimer); /* set the timer as a periodic timer after oneshot timer start */
        
        os_timer_set_timeout_ticks(iotTimer, rescheduleTimeoutTick);
    }
    
    return true;
}

/*-----------------------------------------------------------*/
