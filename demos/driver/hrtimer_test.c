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
 * @file        hwtimer_test.c
 *
 * @brief       The test file for hwtimer.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <timer/clocksource.h>
#include <timer/hrtimer.h>
#include <shell.h>
#include <stdlib.h>


#define DBG_TAG "hrtimer_test"
#include <dlog.h>

static int timeout_count = 0;
static int timeout_callback_print = 1;

static void hrtimer_callback(void *parameter)
{
    timeout_count++;

    if (timeout_callback_print)
    {
        LOG_I(DBG_TAG,"%s :%Ld", parameter, os_clocksource_gettime());
    }
}

static os_hrtimer_t hrtimer1 = {hrtimer_callback, "hrtimer1"};
static os_hrtimer_t hrtimer2 = {hrtimer_callback, "hrtimer2"};
static os_hrtimer_t hrtimer3 = {hrtimer_callback, "hrtimer3"};

static int hrtimer_test(int argc, char *argv[])
{
    int i;

    if (hrtimer1.state != OS_HRTIMER_STATE_NONE)
        os_hrtimer_stop(&hrtimer1);

    if (hrtimer2.state != OS_HRTIMER_STATE_NONE)
        os_hrtimer_stop(&hrtimer2);

    if (hrtimer3.state != OS_HRTIMER_STATE_NONE)
        os_hrtimer_stop(&hrtimer3);

    hrtimer1.timeout_nsec   = NSEC_PER_SEC;
    hrtimer1.period_nsec    = 0;

    hrtimer2.timeout_nsec   = NSEC_PER_SEC * 3 / 2;
    hrtimer2.period_nsec    = 0;

    hrtimer3.timeout_nsec   = NSEC_PER_SEC * 2;
    hrtimer3.period_nsec    = 0;

    os_task_t *self = os_task_self();
    os_uint8_t task_prio = self->current_priority;
    os_uint8_t high_prio = 1;

    os_task_set_priority(self,high_prio);

    /* sync tick */
    os_task_msleep(1);

    /* 1.oneshot mode */
    for (i = 0; i < 3; i++)
    {
        LOG_I(DBG_TAG,"hrtimer onshot start %d", i);      
        
        os_hrtimer_start(&hrtimer1);
        os_hrtimer_start(&hrtimer2);
        os_hrtimer_start(&hrtimer3);
        
        os_task_msleep(3000);

        os_hrtimer_stop(&hrtimer1);
        os_hrtimer_stop(&hrtimer2);
        os_hrtimer_stop(&hrtimer3);
    }

    LOG_I(DBG_TAG,"hrtimer onshot stop.");

    os_task_msleep(2000);

    /* 2.period mode */
    LOG_I(DBG_TAG,"hrtimer period start...");

    hrtimer1.period_nsec = NSEC_PER_SEC;
    hrtimer2.period_nsec = NSEC_PER_SEC * 3 / 2;
    hrtimer3.period_nsec = 0;                       /* hrtimer3 onshot mode */
    
    os_hrtimer_start(&hrtimer1);
    os_hrtimer_start(&hrtimer2);
    os_hrtimer_start(&hrtimer3);
    os_task_msleep(10300);

    LOG_I(DBG_TAG,"hrtimer period before stop.");
    
    os_hrtimer_stop(&hrtimer1);
    os_hrtimer_stop(&hrtimer2);
    os_hrtimer_stop(&hrtimer3);
    os_task_msleep(2000);
    
    LOG_I(DBG_TAG,"hrtimer period after stop.");


    os_task_set_priority(self,task_prio);
    return 0;
}

static int hrtimer_test_period(int argc, char *argv[])
{
    os_uint64_t period_nsec;

    os_uint64_t start, now;
    os_tick_t tick_start, tick_now;

    if (argc != 2)
    {
        LOG_I(DBG_TAG,"usage: hrtimer_test_period <period_nsec>");
        LOG_I(DBG_TAG,"       hrtimer_test_period 50000");
        return -1;
    }

    os_task_t *self = os_task_self();
    os_uint8_t task_prio = self->current_priority;
    os_uint8_t high_prio = 1;
  
    os_task_set_priority(self, high_prio);
    
    period_nsec = strtol(argv[1], OS_NULL, 0);

    if (hrtimer1.state != OS_HRTIMER_STATE_NONE)
        os_hrtimer_stop(&hrtimer1);

    if (hrtimer2.state != OS_HRTIMER_STATE_NONE)
        os_hrtimer_stop(&hrtimer2);

    if (hrtimer3.state != OS_HRTIMER_STATE_NONE)
        os_hrtimer_stop(&hrtimer3);

    LOG_I(DBG_TAG,"hrtimer period start %Lu", period_nsec);

    timeout_count = 0;
    timeout_callback_print = 0;

    hrtimer1.timeout_nsec = period_nsec;
    hrtimer2.timeout_nsec = period_nsec * 20;
    hrtimer3.timeout_nsec = NSEC_PER_SEC * 100;
    
    hrtimer1.period_nsec = hrtimer1.timeout_nsec;
    hrtimer2.period_nsec = hrtimer2.timeout_nsec;
    hrtimer3.period_nsec = hrtimer3.timeout_nsec;

    /* sync tick */
    os_task_msleep(1);

    tick_start = os_tick_get();
    start = os_clocksource_gettime();
    
    os_hrtimer_start(&hrtimer1);
    os_hrtimer_start(&hrtimer2);
    os_hrtimer_start(&hrtimer3);
    
    os_task_msleep(10000);

    os_hrtimer_stop(&hrtimer1);
    os_hrtimer_stop(&hrtimer2);
    //os_hrtimer_stop(&hrtimer3);

    tick_now = os_tick_get();
    now = os_clocksource_gettime();
    
    LOG_I(DBG_TAG,"hrtimer period stop %d", timeout_count);

    LOG_I(DBG_TAG,"msleep start tick: %d", tick_start);
    LOG_I(DBG_TAG,"msleep now   tick: %d             delta tick: %d",
              tick_now, tick_now - tick_start);
    LOG_I(DBG_TAG,"msleep start time: %d.%09d", (int)(start / NSEC_PER_SEC), (int)(start % NSEC_PER_SEC));
    LOG_I(DBG_TAG,"msleep now   time: %d.%09d, delta time: %d us",
              (int)(now / NSEC_PER_SEC), (int)(now % NSEC_PER_SEC), (int)((now - start) / 1000));


    timeout_callback_print = 1;

    os_task_set_priority(self, task_prio);

    return 0;
}

SH_CMD_EXPORT(hrtimer_test, hrtimer_test, "hrtimer_test");
SH_CMD_EXPORT(hrtimer_test_period, hrtimer_test_period, "hrtimer_test_period");

