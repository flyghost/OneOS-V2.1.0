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
 * @file        rt_timer.c
 *
 * @brief       Implementation of RT-Thread adaper timer function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "os_assert.h"
#include "rtthread.h"
#include "os_errno.h"
#include "os_clock.h"

rt_tick_t rt_tick_get(void)
{
    return (rt_tick_t)os_tick_get();
}

void rt_tick_set(rt_tick_t tick)
{
    os_tick_set((os_tick_t)tick);
}

rt_tick_t  rt_tick_from_millisecond(rt_int32_t ms)
{
    return (rt_tick_t)os_tick_from_ms((os_uint32_t)ms);
}

void rt_timer_init(rt_timer_t   timer,
                   const char  *name,
                   void       (*timeout)(void *parameter),
                   void        *parameter,
                   rt_tick_t    time,
                   rt_uint8_t   flag)
{
    OS_ASSERT(time < (OS_TICK_MAX / 2));
    OS_ASSERT(timer);

    os_timer_init(&timer->os_timer,
                   name, 
                   timeout, 
                   parameter, 
                   (os_tick_t)time, 
                   flag);

    timer->is_static = OS_TRUE;
}

rt_err_t rt_timer_detach(rt_timer_t timer)
{
    OS_ASSERT(timer);
    OS_ASSERT(OS_TRUE == timer->is_static);

    os_timer_deinit(&timer->os_timer);

    return RT_EOK;
}

#ifdef RT_USING_HEAP
rt_timer_t rt_timer_create(const char *name,
                           void      (*timeout)(void *parameter),
                           void       *parameter,
                           rt_tick_t   time,
                           rt_uint8_t  flag)
{
    OS_ASSERT(time < (OS_TICK_MAX / 2));

    rt_timer_t timer;

    timer = (rt_timer_t)os_malloc(sizeof(struct rt_timer));
    if(OS_NULL == timer)
    {
        return RT_NULL;
    }
    
    os_timer_init(&timer->os_timer, name, timeout, parameter, time, flag);

    timer->is_static = OS_FALSE;

    return timer;
}

rt_err_t rt_timer_delete(rt_timer_t timer)
{
    OS_ASSERT(timer);
    OS_ASSERT(OS_FALSE == timer->is_static);

    os_timer_deinit(&timer->os_timer);
    os_free(timer);
   
    return RT_EOK;
}
#endif  /* RT_USING_HEAP */

rt_err_t rt_timer_start(rt_timer_t timer)
{
    os_err_t ret;

    OS_ASSERT(timer);
    
    ret = os_timer_start(&timer->os_timer);

    return (rt_err_t)ret;
}

rt_err_t rt_timer_stop(rt_timer_t timer)
{
    os_err_t ret;

    OS_ASSERT(timer);
    
    ret = os_timer_stop(&timer->os_timer);

    return (rt_err_t)ret;
}

rt_err_t rt_timer_control(rt_timer_t timer, int cmd, void *arg)
{
    OS_ASSERT(timer);

    switch (cmd)
    {
    case RT_TIMER_CTRL_GET_TIME:
        *(rt_tick_t *)arg = (rt_tick_t)os_timer_get_timeout_ticks(&timer->os_timer);
        break;
    case RT_TIMER_CTRL_SET_TIME:
        os_timer_set_timeout_ticks(&timer->os_timer, (os_tick_t)(*(rt_tick_t *)arg));
        break;
    case RT_TIMER_CTRL_SET_ONESHOT:
        os_timer_set_oneshot(&timer->os_timer);
        break;
    case RT_TIMER_CTRL_SET_PERIODIC:
        os_timer_set_periodic(&timer->os_timer);
        break;
    }

   return RT_EOK;
}
