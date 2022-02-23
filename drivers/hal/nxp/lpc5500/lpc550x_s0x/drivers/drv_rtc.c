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
 * @file        drv_rtc.c
 *
 * @brief       This file implements rtc driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>
#include <drv_rtc.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

typedef struct lpc_rtc
{
    os_device_t rtc;
    
    lpc_rtc_info_t *rtc_info;
}lpc_rtc_t;

static time_t lpc_rtc_get_timestamp(lpc_rtc_t *lpc_rtc)
{
    struct tm       tm_new;
    rtc_datetime_t rtc_datestruct = {0};

    RTC_GetDatetime(lpc_rtc->rtc_info->rtc_base, &rtc_datestruct);

    tm_new.tm_sec  = rtc_datestruct.second; 
    tm_new.tm_min  = rtc_datestruct.minute; 
    tm_new.tm_hour = rtc_datestruct.hour;
    
    tm_new.tm_mday = rtc_datestruct.day; 
    tm_new.tm_mon  = rtc_datestruct.month - 1; 
    tm_new.tm_year = rtc_datestruct.year - 1900;

    LOG_D(DRV_EXT_TAG, "get rtc time");
    return mktime(&tm_new);
}

static os_err_t lpc_rtc_set_time_stamp(lpc_rtc_t *lpc_rtc, time_t time_stamp)
{
    struct tm *p_tm;
    rtc_datetime_t rtc_dateStruct = {0};
    
    p_tm = localtime(&time_stamp);
    
    rtc_dateStruct.second = p_tm->tm_sec ;
    rtc_dateStruct.minute = p_tm->tm_min ;
    rtc_dateStruct.hour   = p_tm->tm_hour;

    rtc_dateStruct.day    = p_tm->tm_mday;
    rtc_dateStruct.month  = p_tm->tm_mon  + 1;
    rtc_dateStruct.year   = p_tm->tm_year + 1900;
    
    RTC_StopTimer(lpc_rtc->rtc_info->rtc_base);
    
    RTC_SetDatetime(lpc_rtc->rtc_info->rtc_base, &rtc_dateStruct);

    RTC_StartTimer(lpc_rtc->rtc_info->rtc_base);
    
    return OS_EOK;
}

static void lpc_rtc_init(lpc_rtc_t *lpc_rtc)
{
    RTC_StartTimer(lpc_rtc->rtc_info->rtc_base);
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    lpc_rtc_t *lpc_rtc = (lpc_rtc_t *)dev;
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = lpc_rtc_get_timestamp(lpc_rtc);
        LOG_D(DRV_EXT_TAG, "RTC: get rtc_time %x", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (lpc_rtc_set_time_stamp(lpc_rtc, *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_D(DRV_EXT_TAG, "RTC: set rtc_time %x", *(os_uint32_t *)args);
        result = OS_EOK;
        break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static int lpc_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    lpc_rtc_t *lpc_rtc;

    lpc_rtc = os_calloc(1, sizeof(lpc_rtc_t));

    OS_ASSERT(lpc_rtc);

    lpc_rtc->rtc_info = (lpc_rtc_info_t *)dev->info;

    lpc_rtc_init(lpc_rtc);

    lpc_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&lpc_rtc->rtc, dev->name);
}

OS_DRIVER_INFO lpc_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = lpc_rtc_probe,
};

OS_DRIVER_DEFINE(lpc_rtc_driver, DEVICE, OS_INIT_SUBLEVEL_LOW);

