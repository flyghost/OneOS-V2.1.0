/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        canfestival_timer.c
 *
 * @brief       This file implement canfestival timer function
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#if defined(OS_USING_CANFESTIVAL) && !defined(OS_CAN_USING_HDR)
#include "cf_canfestival.h"
#include <drv_cfg.h>
#include <hrtimer.h>
#include <os_event.h>
#include <os_memory.h>
#include <os_sem.h>
#include <timer/clocksource.h>

#include "dlog.h"
#define TAG "CF"

typedef struct
{
    os_bool_t     is_init;
    os_hrtimer_t *hrtimer;
    os_sem_t *    sem;
    os_task_t *   task;
} cf_timer_t;
static cf_timer_t sg_cf_timer = {
    .is_init = FALSE,
};

void setTimer(TIMEVAL value)
{
    os_hrtimer_t *hrtimer = sg_cf_timer.hrtimer;
    hrtimer->timeout_nsec = os_clocksource_gettime() + (value * 1000);
    hrtimer->period_nsec  = 0;
}

TIMEVAL getElapsedTime(void)
{
    volatile os_int64_t diff;
    os_hrtimer_t *      hrtimer;
    hrtimer = sg_cf_timer.hrtimer;
    diff    = (hrtimer->timeout_nsec - os_clocksource_gettime()) / 1000;
    return (diff > 0) ? diff : 0;
}

static void cf_timer_entry(void *param)
{
    cf_timer_t *const cf_timer = (cf_timer_t *)param;
    while (1)
    {
        os_sem_wait(cf_timer->sem, OS_WAIT_FOREVER);
        CanOpen_EnterMutex();
        TimeDispatch();
        CanOpen_LeaveMutex();
    }
}

OS_USED static void hrtimer_callback(void *param)
{
    cf_timer_t *cf_timer = (cf_timer_t *)param;
    OS_ASSERT(cf_timer);
    os_sem_post(cf_timer->sem);
}

void CanOpen_TimerInit(void)
{
    os_task_t * task;
    cf_timer_t *cf_timer;
    cf_timer = &sg_cf_timer;
    if (cf_timer->is_init)
    {
        LOG_W(TAG, "timer has already been initialized.");
        return;
    }
    cf_timer->hrtimer = os_malloc(sizeof(os_hrtimer_t));
    OS_ASSERT(cf_timer->hrtimer);
    cf_timer->hrtimer->timeout_func = hrtimer_callback;
    cf_timer->hrtimer->parameter    = cf_timer;
    cf_timer->sem                   = os_sem_create("cf_timer_sem", 0, OS_SEM_MAX_VALUE);
    task                            = os_task_create("cf_timer", cf_timer_entry, cf_timer, 1024 * 1, 10);
    OS_ASSERT(task);
    os_task_startup(task);
    cf_timer->task    = task;
    cf_timer->is_init = TRUE;
    LOG_I(TAG, "timer init sucessful.");
}

void CanOpen_TimerCleanup(void)
{
    cf_timer_t *cf_timer;
    cf_timer = &sg_cf_timer;
    if (!cf_timer->is_init)
    {
        LOG_W(TAG, "timer has already been closed.");
        return;
    }
    os_hrtimer_stop(cf_timer->hrtimer);

    os_free(cf_timer->hrtimer);
    os_task_destroy(cf_timer->task);
    os_sem_destroy(cf_timer->sem);
    cf_timer->hrtimer = NULL;
    cf_timer->sem     = NULL;
    cf_timer->task    = NULL;
    cf_timer->is_init = FALSE;
    LOG_I(TAG, "timer close sucessful.");
}

/* 禁止重复调用 */
void CanOpen_StartTimerLoop(TimerCallback_t _init_callback)
{
    cf_timer_t *cf_timer = &sg_cf_timer;
    CanOpen_EnterMutex();
    os_hrtimer_start(cf_timer->hrtimer);
    CanOpen_LeaveMutex();
}

void CanOpen_StopTimerLoop(TimerCallback_t Callback)
{
    cf_timer_t *cf_timer = &sg_cf_timer;
    CanOpen_EnterMutex();
    os_hrtimer_stop(cf_timer->hrtimer);
    CanOpen_LeaveMutex();
}
#endif
