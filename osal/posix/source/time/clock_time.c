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
 * @file        clock_time.c
 *
 * @brief       This file provides posix time related operations.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <pthread.h>
#include <device.h>
#include "clock_time.h"
#include <rtc/rtc.h>

static struct timeval gs_time_value;

int clock_time_system_init()
{
    time_t       time;
    os_tick_t    tick;
    os_device_t *device;

    time = 0;
    
    device = os_device_find("rtc");
    if (OS_NULL != device)
    {
        /* Get realtime seconds */
        os_device_control(device, OS_DEVICE_CTRL_RTC_GET_TIME, &time);
    }

    tick = os_tick_get();

    gs_time_value.tv_usec = (tick % OS_TICK_PER_SECOND) * MICROSECOND_PER_TICK;
    gs_time_value.tv_sec  = time - tick / OS_TICK_PER_SECOND - 1;

    return 0;
}
OS_CMPOENT_INIT(clock_time_system_init, "3");

int clock_time_to_tick(const struct timespec *time)
{
    int tick;
    int nsecond;
    int second;
    struct timespec tp;

    OS_ASSERT(OS_NULL != time);

    /* Get current tp. */
    clock_gettime(CLOCK_REALTIME, &tp);

    if ((time->tv_nsec - tp.tv_nsec) < 0)
    {
        nsecond = NANOSECOND_PER_SECOND - (tp.tv_nsec - time->tv_nsec);
        second  = time->tv_sec - tp.tv_sec - 1;
    }
    else
    {
        nsecond = time->tv_nsec - tp.tv_nsec;
        second  = time->tv_sec - tp.tv_sec;
    }

    tick = second * OS_TICK_PER_SECOND + nsecond * OS_TICK_PER_SECOND / NANOSECOND_PER_SECOND;
    
    if (tick < 0)
    {
        tick = 0;
    }

    return tick;
}
EXPORT_SYMBOL(clock_time_to_tick);

int clock_getres(clockid_t clockid, struct timespec *res)
{
    int ret = 0;

    if (OS_NULL == res)
    {
        os_set_errno(EINVAL);
        return -1;
    }

    switch (clockid)
    {
    case CLOCK_REALTIME:
        res->tv_sec  = 0;
        res->tv_nsec = NANOSECOND_PER_SECOND / OS_TICK_PER_SECOND;
        break;

#ifdef OS_USING_CPUTIME
    case CLOCK_CPUTIME_ID:
        res->tv_sec  = 0;
        res->tv_nsec = clock_cpu_getres();
        break;
#endif

    default:
        ret = -1;
        os_set_errno(EINVAL);
        break;
    }

    return ret;
}
EXPORT_SYMBOL(clock_getres);

int clock_gettime(clockid_t clockid, struct timespec *tp)
{
    int ret = 0;

    if (OS_NULL == tp)
    {
        os_set_errno(EINVAL);
        return -1;
    }

    switch (clockid)
    {
    case CLOCK_REALTIME:
        {
            int tick = os_tick_get();

            tp->tv_sec  = gs_time_value.tv_sec + tick / OS_TICK_PER_SECOND;
            tp->tv_nsec = (gs_time_value.tv_usec + (tick % OS_TICK_PER_SECOND) * MICROSECOND_PER_TICK) * 1000;
        }
        break;

#ifdef OS_USING_CPUTIME
    case CLOCK_CPUTIME_ID:
        {
            float     unit = 0;
            long long cpu_tick;

            unit     = clock_cpu_getres();
            cpu_tick = clock_cpu_gettime();

            tp->tv_sec  = ((int)(cpu_tick * unit)) / NANOSECOND_PER_SECOND;
            tp->tv_nsec = ((int)(cpu_tick * unit)) % NANOSECOND_PER_SECOND;
        }
        break;
#endif
    default:
        os_set_errno(EINVAL);
        ret = -1;
    }

    return ret;
}
EXPORT_SYMBOL(clock_gettime);

int clock_settime(clockid_t clockid, const struct timespec *tp)
{
    int          second;
    os_tick_t    tick;
    os_device_t *device;

    if ((CLOCK_REALTIME != clockid) || (OS_NULL == tp))
    {
        os_set_errno(EINVAL);

        return -1;
    }

    second = tp->tv_sec;
    
    tick = os_tick_get();

    /* Update timevalue */
    gs_time_value.tv_usec = MICROSECOND_PER_SECOND - (tick % OS_TICK_PER_SECOND) * MICROSECOND_PER_TICK;
    gs_time_value.tv_sec = second - tick / OS_TICK_PER_SECOND - 1;

    /* Update for RTC device */
    device = os_device_find("rtc");
    if (OS_NULL != device)
    {
        /* Set realtime seconds */
        os_device_control(device, OS_DEVICE_CTRL_RTC_SET_TIME, &second);
    }
    else
    {
        return -1;
    }

    return 0;
}
EXPORT_SYMBOL(clock_settime);
