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
 * @brief       This file implements rtc driver for imxrt.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>
#include <string.h>
#include <sys/time.h>
#include <os_clock.h>
#include <rtc/rtc.h>

#include "fsl_common.h"
#include "fsl_snvs_hp.h"
#include "drv_rtc.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

struct os_imxrt_rtc
{
    struct os_device        rtc;
    struct nxp_rtc_info    *info;
};

static time_t get_timestamp(os_device_t *dev)
{
    struct tm tm_new = {0};
    snvs_hp_rtc_datetime_t rtcDate = {0};
    
    struct os_imxrt_rtc *imxrt_rtc = (struct os_imxrt_rtc *)dev;

//    SNVS_HP_RTC_Init(imxrt_rtc->info->base, imxrt_rtc->info->config);
    
    SNVS_HP_RTC_GetDatetime(imxrt_rtc->info->base, &rtcDate);

    tm_new.tm_sec  = rtcDate.second;
    tm_new.tm_min  = rtcDate.minute;
    tm_new.tm_hour = rtcDate.hour;

    tm_new.tm_mday = rtcDate.day;
    tm_new.tm_mon  = rtcDate.month - 1;
    tm_new.tm_year = rtcDate.year - 1900;

    return mktime(&tm_new);
}

static int set_timestamp(os_device_t *dev, time_t timestamp)
{
    struct tm *p_tm;
    snvs_hp_rtc_datetime_t rtcDate = {0};

    struct os_imxrt_rtc *imxrt_rtc = (struct os_imxrt_rtc *)dev;

    p_tm = localtime(&timestamp);

    rtcDate.second = p_tm->tm_sec ;
    rtcDate.minute = p_tm->tm_min ;
    rtcDate.hour   = p_tm->tm_hour;

    rtcDate.day    = p_tm->tm_mday;
    rtcDate.month  = p_tm->tm_mon  + 1;
    rtcDate.year   = p_tm->tm_year + 1900;

    if (SNVS_HP_RTC_SetDatetime(imxrt_rtc->info->base, &rtcDate) != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "set rtc date time failed");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t imxrt_rtc_control(os_device_t *dev, int cmd, void *args)
{
    OS_ASSERT(dev != OS_NULL);

    switch(cmd)
    {
        case OS_DEVICE_CTRL_RTC_GET_TIME:
            *(uint32_t *)args = get_timestamp(dev);
            break;
        case OS_DEVICE_CTRL_RTC_SET_TIME:
            set_timestamp(dev, *(time_t *)args);
            break;
        default:
            return OS_EINVAL;
    }

    return OS_EOK;
}

const static struct os_device_ops rtc_ops = {
    .control = imxrt_rtc_control,
};

static int imxrt_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t ret = OS_EOK;

    struct os_imxrt_rtc *imxrt_rtc = OS_NULL;

    imxrt_rtc = os_calloc(1, sizeof(struct os_imxrt_rtc));

    OS_ASSERT(imxrt_rtc);
    
    imxrt_rtc->info = (struct nxp_rtc_info *)dev->info;
    
    imxrt_rtc->rtc.ops = &rtc_ops;
    
    ret = os_device_register(&imxrt_rtc->rtc, dev->name);
    if(ret != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "rtc device register failed %d!", ret);
        return ret;
    }

    return OS_EOK;
}

OS_DRIVER_INFO imxrt_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = imxrt_rtc_probe,
};

OS_DRIVER_DEFINE(imxrt_rtc_driver, PREV, OS_INIT_SUBLEVEL_LOW);


