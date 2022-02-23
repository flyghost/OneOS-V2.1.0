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
 * @file        drv_rtc.h
 *
 * @brief       This file implements rtc driver for fm33.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_RTC_H__
#define __DRV_RTC_H__

#include <device.h>
#include <board.h>

struct fm33_alarm
{
    os_uint32_t hour;
    os_uint32_t minute;
    os_uint32_t second;
};

struct fm33_rtc_info
{
    RTC_Type *instance;
};

struct fm33_rtc 
{
    os_device_t                 rtc;
    struct fm33_rtc_info       *info;
};

#define DEFAULT_RTC_TIME        \
{                               \
    0x20,                       \
    0x02,                       \
    0x21,                       \
    0x04,                       \
    0x09,                       \
    0x00,                       \
    0x00,                       \
}

#endif

