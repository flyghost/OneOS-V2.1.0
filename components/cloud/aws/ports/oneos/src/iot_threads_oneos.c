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
 * @file        iot_threads_oneos.c
 * 
 * @brief       Implementation of the functions in iot_threads.h for OneOS systems. 
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

/* Platform threads include. */
#include "platform/iot_threads.h"

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

#define LIBRARY_LOG_NAME    ( "THREAD" )
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

/*-----------------------------------------------------------*/

bool Iot_CreateDetachedThread( IotThreadRoutine_t threadRoutine,
                               void * pArgument,
                               int32_t priority,
                               size_t stackSize )
{
    bool status = true;
    os_task_t *iotTask = OS_NULL;
    int32_t taskPriority;
    size_t taskStackSize;
    char name[OS_NAME_MAX] = {0};
    static os_uint16_t iotThreadNum = 0;

    if (!threadRoutine)
    {   
        IotLogError("Failed to create new thread, invalid argument.");
        
        return false;
    }
    
    snprintf(name, sizeof(name), "iotthd%02d", iotThreadNum++);

    if (IOT_THREAD_IGNORE_STACK_SIZE != stackSize)
    {
        taskStackSize = stackSize;
    }
    else
    {
        taskStackSize = IOT_THREAD_DEFAULT_STACK_SIZE;
    }

    if (IOT_THREAD_IGNORE_PRIORITY != priority)
    {
        taskPriority = priority;
    }
    else
    {
        taskPriority = IOT_THREAD_DEFAULT_PRIORITY;
    }

    /* unity test will not count thread malloc num */

    iotTask = os_task_create(name, threadRoutine, pArgument, taskStackSize, taskPriority);
    
    if (OS_NULL != iotTask)
    {
        os_task_startup(iotTask);
        
        status = true;
        
        /* IotLogInfo("succeed to create new thread %p, thread name %s", name); */
    }
    else
    {
        IotLogError("Failed to create new thread.");
        
        status = false;
    }

    return status;
}

/*-----------------------------------------------------------*/

bool IotMutex_Create( IotMutex_t * pNewMutex, bool recursive )
{
    bool status = true;
    os_mutex_t *iotMutex = OS_NULL;
    char name[OS_NAME_MAX] = {0};
    static os_uint16_t iotMutexNum = 0;
    
    if (!pNewMutex)
    {   
        IotLogError("Failed to create new mutex, invalid argument.");
        
        return false;
    }
    
    snprintf(name, sizeof(name), "iotmtx%02d", iotMutexNum++);
    
    status = UnityMalloc_AllocateResource();
    
    if (true == status)
    {   
        iotMutex = os_mutex_create(name, recursive);
        
        if (OS_NULL == iotMutex)
        {
            IotLogError("Failed to create new mutex %p.", pNewMutex);
            
            UnityMalloc_FreeResource();
            
            status = false;
        }
        else
        {
            *pNewMutex = iotMutex;
            
            status = true;
        }
    }
    
    return status;
}

/*-----------------------------------------------------------*/

void IotMutex_Destroy( IotMutex_t * pMutex )
{
    /* Decrement the number of platform resources in use. */
    os_err_t mutexError = 0;
    os_mutex_t *iotMutex = *pMutex;
    
    UnityMalloc_FreeResource();
    
    /* It's busy */
    if (OS_NULL != iotMutex->owner)
    {   
        IotLogError("Failed to destroy mutex %p. busy now.", pMutex);

        OS_ASSERT( 0 );
    }
    
    mutexError = os_mutex_destroy(iotMutex);

    if (OS_EOK != mutexError)
    {
        /* This block should not be reached; log an error and abort if it is. */
        IotLogError("Failed to destroy mutex %p. errno=%d.", pMutex, mutexError);

        OS_ASSERT( 0 );
    }

    *pMutex = NULL;
    
    /* Don't add IotLog here, otherwise unity test will assert because of IotLog using mutex.
       If needed, use oneos dlog. */
}

/*-----------------------------------------------------------*/

void IotMutex_Lock( IotMutex_t * pMutex )
{
    int mutexType = 0;
    os_err_t mutexError = 0;
    os_mutex_t *iotMutex = *pMutex;
    
    mutexType = iotMutex->is_recursive;

    os_schedule_lock();
    
    if ((iotMutex->owner == os_task_self()) && (OS_TRUE != mutexType))
    {
        os_schedule_unlock();
        
        IotLogError("Failed to lock mutex %p. illegal use of non-recursive lock.", pMutex);

        OS_ASSERT( 0 );
    }
    
    os_schedule_unlock();

    if (OS_TRUE != mutexType)
    {
        mutexError = os_mutex_lock(iotMutex, OS_WAIT_FOREVER);
    }
    else
    {
        mutexError = os_mutex_recursive_lock(iotMutex, OS_WAIT_FOREVER);
    }

    if (OS_EOK != mutexError)
    {
        /* This block should not be reached; log an error and abort if it is. */
        IotLogError("Failed to lock mutex %p. errno=%d.", pMutex, mutexError);

        OS_ASSERT( 0 );
    }
}

/*-----------------------------------------------------------*/

bool IotMutex_TryLock( IotMutex_t * pMutex )
{
    bool status = true;
    int mutexType = 0;
    os_err_t mutexError = 0;
    os_mutex_t *iotMutex = *pMutex;
    
    mutexType = iotMutex->is_recursive;
    
    os_schedule_lock();
    
    if ((iotMutex->owner == os_task_self()) && (OS_TRUE != mutexType))
    {
        os_schedule_unlock();
        
        IotLogError("Failed to trylock mutex %p. non-recursive lock has been taken.", pMutex);
        
        return false;
    }
    
    os_schedule_unlock();

    if (OS_TRUE != mutexType)
    {
        mutexError = os_mutex_lock(iotMutex, OS_NO_WAIT);
    }
    else
    {
        mutexError = os_mutex_recursive_lock(iotMutex, OS_NO_WAIT);
    }

    if (OS_EOK != mutexError)
    {
        IotLogDebug("Mutex mutex %p is not available. errno=%d.", pMutex, mutexError);

        status = false;
    }

    return status;
}

/*-----------------------------------------------------------*/

void IotMutex_Unlock( IotMutex_t * pMutex )
{
    int mutexType = 0;
    os_err_t mutexError = 0;
    os_mutex_t *iotMutex = *pMutex;

    mutexType = iotMutex->is_recursive;
    
    os_schedule_lock();
    
    if (iotMutex->owner != os_task_self())
    {
        /* No thread waiting on this mutex. */
        if (OS_NULL == iotMutex->owner)
        {   
            os_kprintf("iotMutex unlock fail, iotMutex owner is NULL.%s\n");
            
            os_schedule_unlock();
            
            return;
        }
        else
        {   
            os_kprintf("iotMutex unlock before, owner: %s\n", iotMutex->owner->name);
            
            OS_ASSERT( 0 );
        }
    }
    
    os_schedule_unlock();

    if (OS_TRUE != mutexType)
    {
        mutexError = os_mutex_unlock(iotMutex);
    }
    else
    {
        mutexError = os_mutex_recursive_unlock(iotMutex);
    }

    if (OS_EOK != mutexError)
    {
        /* This block should not be reached; log an error and abort if it is. */
        IotLogError("Failed to unlock mutex %p. errno=%d.", pMutex, mutexError);
        
        OS_ASSERT( 0 );
    }
}

/*-----------------------------------------------------------*/

bool IotSemaphore_Create( IotSemaphore_t * pNewSemaphore,
                          uint32_t initialValue,
                          uint32_t maxValue )
{
    bool status = true;
    os_sem_t *iotSem = OS_NULL;
    char name[OS_NAME_MAX] = {0};
    static os_uint16_t iotSemNum = 0;
    
    if (!pNewSemaphore)
    {   
        IotLogError("Failed to create new semaphore, invalid argument.");
        
        return false;
    }
    
    if (maxValue > (uint32_t)OS_SEM_MAX_VALUE)
    {
        IotLogError("%lu is larger than the maximum value a semaphore may"
                     " have on this system.", maxValue);

        return false;
    }
    
    status = UnityMalloc_AllocateResource();

    if (true == status)
    {
        snprintf(name, sizeof(name), "iotsem%02d", iotSemNum++);
        
        iotSem = os_sem_create(name, initialValue, maxValue);
        
        if (OS_NULL == iotSem)
        {
            IotLogError("Failed to create new semaphore %p, because of no memory.", pNewSemaphore);
            
            UnityMalloc_FreeResource();
            
            status = false;
        }
        else
        {
            *pNewSemaphore = iotSem;
            
            status = true;

            IotLogInfo("Succeed to create semaphore %p. %p.", pNewSemaphore, *pNewSemaphore);
        }
        
    }

    return status;
}

/*-----------------------------------------------------------*/

void IotSemaphore_Destroy( IotSemaphore_t * pSemaphore )
{
    os_err_t semError = 0;
    os_sem_t *iotSem = *pSemaphore;
    
    if (!iotSem)
    {
        IotLogError("Failed to destroy semaphore %p, invalid argument.", pSemaphore);

        OS_ASSERT( 0 );
    }

     /* Decrement the number of platform resources in use. */
    UnityMalloc_FreeResource();
    
    semError = os_sem_destroy(iotSem);
    
    if (OS_EOK != semError)
    {
        /* This block should not be reached; log an error and abort if it is. */
        IotLogError("Failed to destroy semaphore %p", pSemaphore);

        OS_ASSERT( 0 );
    }
    
    *pSemaphore = NULL;
    
    IotLogInfo("Succeed to destroy semaphore %p.%p.", pSemaphore, *pSemaphore);
}

/*-----------------------------------------------------------*/

uint32_t IotSemaphore_GetCount( IotSemaphore_t * pSemaphore )
{
    int iotSemCount = 0;
    os_sem_t *iotSem = *pSemaphore;

    if (!iotSem)
    {
        IotLogError("Failed to get count on semaphore %p, invalid argument.", pSemaphore);

        OS_ASSERT( 0 );
    }
    
    iotSemCount = iotSem->count;

    return (uint32_t)iotSemCount;
}

/*-----------------------------------------------------------*/

void IotSemaphore_Wait( IotSemaphore_t * pSemaphore )
{
    os_err_t semError = 0;
    os_sem_t *iotSem = *pSemaphore;

    if (!iotSem)
    {
        IotLogError("Failed to wait on semaphore %p, invalid argument.", pSemaphore);

        OS_ASSERT( 0 );
    }

    semError = os_sem_wait(iotSem, OS_WAIT_FOREVER);
    
    if (OS_EOK != semError)
    {   
         /* This block should not be reached; log an error and abort if it is. */
        IotLogError("Failed to wait on semaphore %p. errno=%d.", pSemaphore, semError);

        OS_ASSERT( 0 );
    }
}

/*-----------------------------------------------------------*/

bool IotSemaphore_TryWait( IotSemaphore_t * pSemaphore )
{
    bool status = true;
    os_err_t semError = 0;
    os_sem_t *iotSem = *pSemaphore;

    if (!iotSem)
    {
        IotLogError("Failed to trywait on semaphore %p, invalid argument.", pSemaphore);

        OS_ASSERT( 0 );
    }

    semError = os_sem_wait(iotSem, OS_NO_WAIT);
    
    if (OS_EOK != semError)
    {
        /* This block should not be reached; log an error and abort if it is. */
        IotLogDebug("Trywait on semaphore %p is not available. errno=%d.", pSemaphore, semError);

        status = false;
    }

    return status;
}

/*-----------------------------------------------------------*/

bool IotSemaphore_TimedWait( IotSemaphore_t * pSemaphore,
                             uint32_t timeoutMs )
{
    bool status = true;
    os_err_t semError = 0;
    os_tick_t ticksToWait = os_tick_from_ms(timeoutMs); /* convert milliseconds to ticks */
    os_sem_t *iotSem = *pSemaphore;

    if (!iotSem)
    {
        IotLogError("Failed to Timedwait on semaphore %p, invalid argument.", pSemaphore);

        OS_ASSERT( 0 );
    }

    semError = os_sem_wait(iotSem, ticksToWait);
    
    if (OS_EOK != semError)
    {   
        IotLogDebug("Timedwait on semaphore %p is not available. errno=%d.", pSemaphore, semError);

        status = false;
    }

    return status;
}

/*-----------------------------------------------------------*/

void IotSemaphore_Post( IotSemaphore_t * pSemaphore )
{
    os_err_t semError = 0;
    os_sem_t *iotSem = *pSemaphore;

    if (!iotSem)
    {
        IotLogError("Failed to post on semaphore %p, invalid argument.", pSemaphore);
     
        OS_ASSERT( 0 );
    }

    semError = os_sem_post(iotSem);

    if (OS_EOK != semError)
    {
        /* This block should not be reached; log an error and abort if it is. */
        IotLogError("Failed to post to semaphore %p. errno=%d.", pSemaphore, semError);

        OS_ASSERT( 0 );
    }
}

/*-----------------------------------------------------------*/
