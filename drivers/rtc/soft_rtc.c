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
 * @file        soft_rtc.c
 *
 * @brief       This file provides soft_rtc device registered interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <sys/time.h>
#include <string.h>
#include <os_task.h>
#include <os_clock.h>
#include <rtc/rtc.h>

/* 2020/4/8 13:13:13 = RTC_TIME_INIT(2020, 4, 8, 13, 13, 13)  */
#define RTC_TIME_INIT(year, month, day, hour, minute, second)  \
    {                                     \
        .tm_year = year - 1900,           \
        .tm_mon   = month - 1,            \
        .tm_mday = day,                   \
        .tm_hour = hour,                  \
        .tm_min = minute,                 \
        .tm_sec = second                  \
    }

#ifndef SOFT_RTC_TIME_DEFAULT
#define SOFT_RTC_TIME_DEFAULT RTC_TIME_INIT(2020, 4, 1, 0, 0, 0)
#endif

static struct os_device soft_rtc_dev;
static os_tick_t        init_tick;
static time_t           init_time;

static os_err_t soft_rtc_control(os_device_t *dev, int cmd, void *args)
{
    time_t *  time;
    struct tm time_temp;

    OS_ASSERT(dev != OS_NULL);
    memset(&time_temp, 0, sizeof(struct tm));

    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        time  = (time_t *)args;
        *time = init_time + (os_tick_get() - init_tick) / OS_TICK_PER_SECOND;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
    {
        time      = (time_t *)args;
        init_time = *time - (os_tick_get() - init_tick) / OS_TICK_PER_SECOND;
        break;
    }
    }

    return OS_EOK;
}

const static struct os_device_ops soft_rtc_ops = {
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    soft_rtc_control
};

int os_soft_rtc_init(void)
{
    static os_bool_t init_ok  = OS_FALSE;
    struct tm        time_new = SOFT_RTC_TIME_DEFAULT;

    if (init_ok)
    {
        return 0;
    }
    /* make sure only one 'rtc' device */
    OS_ASSERT(!os_device_find("rtc"));

    init_tick = os_tick_get();
    init_time = mktime(&time_new);

    soft_rtc_dev.type    = OS_DEVICE_TYPE_RTC;

    /* register rtc device */
    soft_rtc_dev.ops     = &soft_rtc_ops;

    /* no private */
    soft_rtc_dev.user_data = OS_NULL;

    os_device_register(&soft_rtc_dev, "rtc");

    init_ok = OS_TRUE;

    return 0;
}
OS_DEVICE_INIT(os_soft_rtc_init);

