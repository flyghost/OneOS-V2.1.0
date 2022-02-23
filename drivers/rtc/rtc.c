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
 * @file        rtc.c
 *
 * @brief       This file provides rtc date and time set function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <sys/time.h>
#include <string.h>
#include <os_task.h>
#include <os_errno.h>
#include <device.h>
#include <rtc/rtc.h>
#include <dlog.h>
#include <timer/timekeeping.h>
#include <os_util.h>
#define DBG_TAG "rtc"

/* Using NTP auto sync RTC time */
#ifdef RTC_SYNC_USING_NTP
/* NTP first sync delay time for network connect, unit: second */
#ifndef RTC_NTP_FIRST_SYNC_DELAY
#define RTC_NTP_FIRST_SYNC_DELAY (30)
#endif
/* NTP sync period, unit: second */
#ifndef RTC_NTP_SYNC_PERIOD
#define RTC_NTP_SYNC_PERIOD (1L * 60L * 60L)
#endif
#endif /* RTC_SYNC_USING_NTP */

time_t rtc_get(void)
{
    time_t time_now = 0;

    os_device_t *device = os_device_find(OS_RTC_DEV_NAME);

    /* Find rtc device only first. */
    if (device == OS_NULL)
    {
        os_kprintf("find no rtc device\r\n");
        return 0;
    }

    /* Read timestamp from RTC device. */
    if (os_device_open(device) == OS_EOK)
    {
        os_device_control(device, OS_DEVICE_CTRL_RTC_GET_TIME, &time_now);
        os_device_close(device);
    }
    else
    {
        os_kprintf("get rtc, open fail\r\n");
    }

    return time_now;
}

os_err_t rtc_set(time_t time)
{
    os_err_t ret = OS_ERROR;

    os_device_t *device = os_device_find(OS_RTC_DEV_NAME);

    /* Find rtc device only first. */
    if (device == OS_NULL)
    {
        os_kprintf("find no rtc device\r\n");
        return ret;
    }

    /* Read timestamp from RTC device. */
    if (os_device_open(device) == OS_EOK)
    {
        ret = os_device_control(device, OS_DEVICE_CTRL_RTC_SET_TIME, &time); 
        os_device_close(device);
    }
    else
    {
        os_kprintf("set rtc, open fail\r\n");
    }
    
#ifdef OS_USING_TIMEKEEPING
    os_timekeeping_init();
#endif

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Set rtc data function: it will judge if the input value is legal,then set the rtc date.
 *
 * @param[in]       os_uint32_t year       e.g: 2020.
 * @param[in]       os_uint32_t month      e.g: 2 (1~12).
 * @param[in]       os_uint32_t day        e.g: 20.
 *
 * @return          Return set date state(os_err_t).
 * @retval          OS_EOK                 Set success.
 * @retval          OS_ERROR               Set failed.
 ***********************************************************************************************************************
 */
os_err_t set_date(os_uint32_t year, os_uint32_t month, os_uint32_t day)
{
    time_t       now;
    struct tm   *p_tm;
    struct tm    tm_new;
    os_err_t     ret = OS_ERROR;
    os_int32_t   year_range[2];

    year_range[0] = 2000;
    year_range[1] = 2099;

    /* parameter check */
    if (year > year_range[1] || year < year_range[0])
    {
        os_kprintf("year is out of range [%d-%d]\r\n", year_range[0], year_range[1]);
        ret = OS_ERROR;
        return ret;
    }
    if (month == 0 || month > 12)
    {
        os_kprintf("month is out of range [1-12]\r\n");
        ret = OS_ERROR;
        return ret;
    }
    if (day == 0 || day > 31)
    {
        os_kprintf("day is out of range [1-31]\r\n");
        ret = OS_ERROR;
        return ret;
    }

    /* get current time */
    now = rtc_get();

    /* lock scheduler. */
    os_schedule_lock();
    /* converts calendar time time into local time. */
    p_tm = localtime(&now);
    /* copy the statically located variable */
    memcpy(&tm_new, p_tm, sizeof(struct tm));
    /* unlock scheduler. */
    os_schedule_unlock();

    /* update date. */
    tm_new.tm_year = year - 1900;
    tm_new.tm_mon  = month - 1; /* tm_mon: 0~11 */
    tm_new.tm_mday = day;

    /* converts the local time in time to calendar time. */
    now = mktime(&tm_new);

    /* update to RTC device. */
    
    ret = rtc_set(now);

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Set rtc time function: it will judge if the input value is legal,then set the rtc time.
 *
 * @param[in]       os_uint32_t hour       e.g: 0~23.
 * @param[in]       os_uint32_t minute     e.g: 0~59.
 * @param[in]       os_uint32_t second     e.g: 0~59.
 *
 * @return          Return set time state(os_err_t).
 * @retval          OS_EOK                 Set success.
 * @retval          OS_ERROR               Set failed.
 ***********************************************************************************************************************
 */
os_err_t set_time(os_uint32_t hour, os_uint32_t minute, os_uint32_t second)
{
    time_t       now;
    struct tm   *p_tm;
    struct tm    tm_new;
    os_err_t     ret = OS_ERROR;

    if (hour > 23)
    {
        os_kprintf("hour is out of range [0-23]\r\n");
        return ret;
    }
    if (minute > 59)
    {
        os_kprintf("minute is out of range [0-59]\r\n");
        return ret;
    }
    if (second > 59)
    {
        os_kprintf("second is out of range [0-59]\r\n");
        return ret;
    }

    /* get current time */
    now = rtc_get();

    /* lock scheduler. */
    os_schedule_lock();
    /* converts calendar time time into local time. */
    p_tm = localtime(&now);
    /* copy the statically located variable */
    memcpy(&tm_new, p_tm, sizeof(struct tm));
    /* unlock scheduler. */
    os_schedule_unlock();

    /* update time. */
    tm_new.tm_hour = hour;
    tm_new.tm_min  = minute;
    tm_new.tm_sec  = second;

    /* converts the local time in time to calendar time. */
    now = mktime(&tm_new);

    /* update to RTC device. */
    ret = rtc_set(now);

    return ret;
}

os_err_t set_rtc_alarm(time_t time)
{
    os_err_t ret = OS_ERROR;

    os_device_t *device = os_device_find(OS_RTC_DEV_NAME);

    /* Find rtc device only first. */
    if (device == OS_NULL)
    {
        os_kprintf("find no rtc device\r\n");
        return ret;
    }

    /* Read timestamp from RTC device. */
    if (os_device_open(device) == OS_EOK)
    {
        ret = os_device_control(device, OS_DEVICE_CTRL_RTC_SET_ALARM, &time); 
        os_device_close(device);
    }
    else
    {
        os_kprintf("set rtc, open fail\r\n");
    }

    return ret;
}


#ifdef RTC_SYNC_USING_NTP
static void ntp_sync_task_enrty(void *param)
{
    extern time_t ntp_sync_to_rtc(const char *host_name);

    os_task_msleep(RTC_NTP_FIRST_SYNC_DELAY * 1000);

    while (1)
    {
        ntp_sync_to_rtc(NULL);
        os_task_msleep(RTC_NTP_SYNC_PERIOD * 1000);
    }
}

int os_rtc_ntp_sync_init(void)
{
    static os_bool_t init_ok = OS_FALSE;
    os_task_t *task;

    if (init_ok)
    {
        return 0;
    }

    task = os_task_create("ntp_sync", ntp_sync_task_enrty, OS_NULL, 1536, 26, 2);
    if (task)
    {
        os_task_startup(task);
    }
    else
    {
        return OS_ENOMEM;
    }

    init_ok = OS_TRUE;

    return OS_EOK;
}
OS_CMPOENT_INIT(os_rtc_ntp_sync_init);
#endif /* RTC_SYNC_USING_NTP */

