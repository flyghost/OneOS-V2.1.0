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
 * @file        onepos_timer.c
 *
 * @brief       timer and timer-meter
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-27   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_errno.h>
#include <os_memory.h>     
#include "onepos_common.h"
#include "onepos_timer.h"

#define  MS_PER_S                  1000

/**
 ***********************************************************************************************************************
 * @brief           The onepos_timer callback function in the os_timer
 *
 * @param[in]       sync_sem    The semaphore in the corresponding onepos_timer.
 ***********************************************************************************************************************
 */
static void onepos_timer_timerout(void *sync_sem)
{
    os_sem_post((os_sem_t *)sync_sem);
}


/**
  ***********************************************************************************************************************
  * @brief           This function convert ticks to milliseconds.
  *
  * @param[in]       tick    The number of ticks to convert.
  *
  * @return          The converted milliseconds.
  ***********************************************************************************************************************
  */
 static double onepos_ms_from_tick(os_tick_t tick)
 {
     double ms_per_tick;
     ms_per_tick  = (MS_PER_S * 1.0) / OS_TICK_PER_SECOND;
     return tick * ms_per_tick; 
 }


/**
  ***********************************************************************************************************************
  * @brief           Record the start time of the clock
  *
  * @param[in]       clock_handle    The pointer to a onepos_clock_t.
  ***********************************************************************************************************************
  */
void onepos_clock_start(onepos_clock_t *clock_handle)
{
    clock_handle->start_tick = os_tick_get();
}


/**
  ***********************************************************************************************************************
  * @brief           Stop the clock and return to the elapsed time
  *
  * @param[in]       clock_handle    The pointer to the onepos_clock_t with a start time.
  *
  * @return          The elapsed time(ms).
  * @retval          double
  ***********************************************************************************************************************
  */
double onepos_clock_stop(onepos_clock_t *clock_handle)
{
    os_tick_t elapsed_tick;      
    elapsed_tick = os_tick_get() - clock_handle->start_tick;
    
    return onepos_ms_from_tick(elapsed_tick);
}


/**
  ***********************************************************************************************************************
  * @brief           create a onepos_timer
  *
  * @param[in]       name               onepos_timer name.
  * @param[in]       delay_time         wait time (ms).
  *
  * @return          The pointer to the onepos_timer_t that is created
  * @retval          onepos_timer_t*    operation succeeded
  * @retval          OS_NULL            operation failed
***********************************************************************************************************************
*/
onepos_timer_t *onepos_timer_create(const char *name, os_uint32_t delay_time)  
{
    os_sem_t       *sem          = OS_NULL;
    os_timer_t     *os_timer     = OS_NULL;
    onepos_timer_t *onepos_timer = OS_NULL;
    os_tick_t       delay_tick   = 0;
        
    onepos_timer = (onepos_timer_t *)os_malloc(sizeof(onepos_timer_t));
    if(OS_NULL == onepos_timer)
    {
        return OS_NULL;
    }

    sem = os_sem_create(name, 0, SEM_VALUE_TO_MUTEX);            
    if(OS_NULL == sem)                                                   
    {
        os_free(onepos_timer);
        return OS_NULL;
    }

    delay_tick = os_tick_from_ms(delay_time);
    os_timer   = os_timer_create(name, onepos_timer_timerout, sem, delay_tick, OS_TIMER_FLAG_ONE_SHOT);   
    if(OS_NULL == os_timer)
    {
        os_free(onepos_timer);
        os_sem_destroy(sem);
        return OS_NULL;
    }

    onepos_timer->sync_sem   = sem;
    onepos_timer->delay_time = delay_time;
    onepos_timer->timer      = os_timer;
    
    return onepos_timer;
}


/**
  ***********************************************************************************************************************
  * @brief           destroy a onepos_timer
  *
  * @param[in]       onepos_timer         onepos_timer to destroy.
  *
  * @return          the operation result
  * @retval          OS_EOK               operation succeeded
  * @retval          POS_ENULL            operation failed
***********************************************************************************************************************
*/
os_err_t onepos_timer_destroy(onepos_timer_t *onepos_timer)
{
    if(OS_NULL == onepos_timer || OS_NULL == onepos_timer->sync_sem || OS_NULL == onepos_timer->timer)
    {
        return POS_ENULL;
    }
    
    os_timer_destroy(onepos_timer->timer);
    os_sem_destroy(onepos_timer->sync_sem);
    os_free(onepos_timer);

    return OS_EOK;
}


/**
  ***********************************************************************************************************************
  * @brief           start a onepos_timer
  *
  * @param[in]       onepos_timer         onepos_timer to start.
  *
  * @return          the operation result
  * @retval          OS_EOK               operation succeeded
  * @retval          POS_ENULL            operation failed
***********************************************************************************************************************
*/
os_err_t onepos_timer_start(onepos_timer_t *onepos_timer)
{
    if(OS_NULL == onepos_timer || OS_NULL == onepos_timer->sync_sem || OS_NULL == onepos_timer->timer)
    {
        return POS_ENULL;
    }
    
    return os_timer_start(onepos_timer->timer);
}


/**
  ***********************************************************************************************************************
  * @brief           stop a onepos_timer
  *
  * @param[in]       onepos_timer         onepos_timer to stop.
  *
  * @return          the operation result
  * @retval          OS_EOK               operation succeeded
  * @retval          POS_ENULL            operation failed
***********************************************************************************************************************
*/
os_err_t onepos_timer_stop(onepos_timer_t *onepos_timer)
{
    if(OS_NULL == onepos_timer || OS_NULL == onepos_timer->sync_sem || OS_NULL == onepos_timer->timer)
    {
        return POS_ENULL;
    }
    
    return os_timer_stop(onepos_timer->timer);
}


/**
  ***********************************************************************************************************************
  * @brief           onepos_timer wait
  *
  * @param[in]       onepos_timer         make onepos_timer wait.
  *
  * @return          the operation result
  * @retval          OS_EOK               operation succeeded
  * @retval          POS_ENULL            operation failed
  * @retval          OS_ETIMEOUT
***********************************************************************************************************************
*/
os_err_t onepos_timer_wait(onepos_timer_t *onepos_timer)
{
    if(OS_NULL == onepos_timer || OS_NULL == onepos_timer->sync_sem || OS_NULL == onepos_timer->timer)
    {
        return POS_ENULL;
    }
    os_tick_t delay_tick = os_tick_from_ms(onepos_timer->delay_time);
    return os_sem_wait(onepos_timer->sync_sem, 2*delay_tick);
}


/**
  ***********************************************************************************************************************
  * @brief           set the delay time of the onepos_timer
  *
  * @param[in]       onepos_timer           
  * @param[in]       delay_time(ms)        onepos_timer delay time
  *
  * @return          the operation result
  * @retval          OS_EOK                operation succeeded
  * @retval          POS_ENULL             operation failed
***********************************************************************************************************************
*/
os_err_t onepos_timer_set_delaytime(onepos_timer_t *onepos_timer, os_uint32_t delay_time)
{
    if(OS_NULL == onepos_timer || OS_NULL == onepos_timer->sync_sem || OS_NULL == onepos_timer->timer)
    {
        return POS_ENULL;
    }

    os_tick_t delay_tick = 0;

    onepos_timer->delay_time = delay_time;
    delay_tick = os_tick_from_ms(delay_time);
    return os_timer_set_timeout_ticks(onepos_timer->timer, delay_tick);
}

