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
 * @file        rtc.h
 *
 * @brief       This file provides rtc functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __RTC_H__
#define __RTC_H__

#include <os_assert.h>
#include <os_errno.h>
#include <device.h>
#include <sys/time.h>

#define OS_DEVICE_CTRL_RTC_GET_TIME     IOC_RTC(0)        /* Get time. */
#define OS_DEVICE_CTRL_RTC_SET_TIME     IOC_RTC(1)        /* Set time. */
#define OS_DEVICE_CTRL_RTC_GET_ALARM    IOC_RTC(2)        /* Get alarm. */
#define OS_DEVICE_CTRL_RTC_SET_ALARM    IOC_RTC(3)        /* Set alarm. */

time_t   rtc_get(void);
os_err_t rtc_set(time_t time);
os_err_t set_date(os_uint32_t year, os_uint32_t month, os_uint32_t day);
os_err_t set_time(os_uint32_t hour, os_uint32_t minute, os_uint32_t second);
os_err_t set_rtc_alarm(time_t time);

int os_soft_rtc_init(void);
int os_rtc_ntp_sync_init(void);

#endif /* __RTC_H__ */
