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
 * @file        cputime.c
 *
 * @brief       This file provides functions for cputime calculation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <arch_interrupt.h>
#include <device.h>
#include <rtc/rtc.h>
#include <os_task.h>
#include <os_errno.h>
#include <os_assert.h>
#include <timer/clocksource.h>
#include <timer/timekeeping.h>

static os_int64_t gs_time_offset = 0;

os_uint64_t timekeeping_gettimeofday(struct timeval *tp, void *ignore)
{
    os_uint64_t nsec;
    
    nsec = gs_time_offset + os_clocksource_gettime();

    if (tp != OS_NULL)
    {
        tp->tv_sec  = nsec / NSEC_PER_SEC;
        tp->tv_usec = nsec % NSEC_PER_SEC / NSEC_PER_USEC;
    }

    return nsec;
}

os_err_t os_timekeeping_init(void)
{
    time_t       time;
    os_device_t *device;

    gs_time_offset = -os_clocksource_gettime();

    device = os_device_find("rtc");

    if (device != OS_NULL)
    {
        os_device_control(device, OS_DEVICE_CTRL_RTC_GET_TIME, &time);

        os_kprintf("timekeeping load rtc %Lu second.\r\n", (os_uint64_t)time);
        
        gs_time_offset += NSEC_PER_SEC * time;
    }

    os_kprintf("timekeeping init %Ld nsec.\r\n", gs_time_offset);

    return OS_EOK;
}

OS_INIT_EXPORT(os_timekeeping_init, "4", OS_INIT_SUBLEVEL_LOW);

