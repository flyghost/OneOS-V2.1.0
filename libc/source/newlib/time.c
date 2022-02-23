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
 * @file        time.c
 *
 * @brief       This file provides some standard time interfaces.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <sys/time.h>
#include <oneos_config.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_clock.h>
#include <device.h>
#if defined(OS_USING_RTC)
#include <rtc.h>
#endif

#ifdef OS_USING_RTC
/**
 ***********************************************************************************************************************
 * @brief           Get the current time,unsupport us.
 *
 * @param[in]       ignore    Ignored.
 * @param[out]      tp        The timestamp pointer, if not used, keep NULL.
 *
 * @return          Returns the current time.
 ***********************************************************************************************************************
 */
int gettimeofday(struct timeval *tp, void *ignore)
{
    time_t time = 0;
    os_device_t *device;

    device = os_device_find("rtc");
    OS_ASSERT(device != OS_NULL);

#if defined(OS_USING_RTC)
    os_device_control(device, OS_DEVICE_CTRL_RTC_GET_TIME, &time);
#endif
    if (tp != OS_NULL)
    {
        tp->tv_sec = time;
        tp->tv_usec = 0;
    }

    return time;
}
#endif

/**
 ***********************************************************************************************************************
 * @brief           Get the current time.
 *
 * @param[out]      t         The timestamp pointer, if not used, keep NULL.
 *
 * @return          Returns the current timestamp.
 ***********************************************************************************************************************
 */
time_t time(time_t *t)
{
    time_t time_now = 0;

#ifdef OS_USING_RTC
    static os_device_t *device = OS_NULL;

    /* Find rtc device only first. */
    if (device == OS_NULL)
    {
        device = os_device_find(OS_RTC_DEV_NAME);
    }

    /* Read timestamp from RTC device. */
    if (device != OS_NULL)
    {
        if (os_device_open(device) == OS_EOK)
        {
            os_device_control(device, OS_DEVICE_CTRL_RTC_GET_TIME, &time_now);
            os_device_close(device);
        }
    }
#endif /* OS_USING_RTC */

    /* if t is not NULL, write timestamp to *t */
    if (t != OS_NULL)
    {
        *t = time_now;
    }

    return time_now;
}

/**
 ***********************************************************************************************************************
 * @brief           Get current clock.
 *
 * @attention       This function is weak, and could be implemented in App.
 *
 * @param           No parameter.
 *
 * @return          Current tick count.
 ***********************************************************************************************************************
 */
OS_WEAK clock_t clock(void)
{
    return os_tick_get();
}
