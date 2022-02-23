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
 * @file        onepos_timer.h
 *
 * @brief       timer and timer-meter
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-27   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ONEPOS_TIMER_H__
#define __ONEPOS_TIMER_H__

#include <os_types.h>
#include <os_clock.h>
#include <os_timer.h>
#include <os_sem.h>
#include <oneos_config.h>  //for OS_TICK_PER_SECOND

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    os_tick_t start_tick;
}onepos_clock_t;


typedef struct
{
    os_sem_t    *sync_sem;
    os_timer_t  *timer;
    os_uint32_t  delay_time;      /* uint: 1ms */
}onepos_timer_t;


extern  void            onepos_clock_start(onepos_clock_t *clock_handle);
extern  double          onepos_clock_stop(onepos_clock_t *clock_handle);
extern  onepos_timer_t *onepos_timer_create(const char *name, os_uint32_t delay_time);
extern  os_err_t        onepos_timer_destroy(onepos_timer_t *onepos_timer);
extern  os_err_t        onepos_timer_start(onepos_timer_t *onepos_timer);
extern  os_err_t        onepos_timer_stop(onepos_timer_t *onepos_timer);
extern  os_err_t        onepos_timer_wait(onepos_timer_t *onepos_timer);
extern  os_err_t        onepos_timer_set_delaytime(onepos_timer_t *onepos_timer, os_uint32_t delay_time);


#ifdef __cplusplus
}
#endif

#endif /*__ONEPOS_TIMER_H__ */

