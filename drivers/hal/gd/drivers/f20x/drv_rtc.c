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
 * @brief       This file implements RTC driver for gd32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>
#include <drv_log.h>
#include "drv_rtc.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"


struct gd32_rtc {
    os_device_t rtc;
    
    os_uint32_t gd_rtc;
};

uint8_t Hex_To_ByteDec(uint8_t PuB_Dat)
{
    uint8_t dat;
    dat = (PuB_Dat / 16) * 10 + PuB_Dat % 16;
    return dat;
}

uint8_t IntDec_To_Hex(int PuB_Dat)
{
    uint8_t dat;
    dat = (PuB_Dat / 10) * 16 + PuB_Dat % 10;
    return dat;
}

static time_t gd32_rtc_get_timestamp(void)
{
    time_t ret;
    rtc_parameter_struct rtc_initpara;

    rtc_current_time_get(&rtc_initpara);

    struct tm       tm_new;

    tm_new.tm_sec  = Hex_To_ByteDec(rtc_initpara.second);
    tm_new.tm_min  = Hex_To_ByteDec(rtc_initpara.minute);
    tm_new.tm_hour = Hex_To_ByteDec(rtc_initpara.hour);
    tm_new.tm_mday = Hex_To_ByteDec(rtc_initpara.date);
    tm_new.tm_mon  = Hex_To_ByteDec(rtc_initpara.month) - 1;
    tm_new.tm_year = Hex_To_ByteDec(rtc_initpara.year) + 100;

    LOG_EXT_D("get rtc time.");
    ret = mktime(&tm_new);

    return ret;
}

static os_err_t gd32_rtc_set_time_stamp(time_t time_stamp)
{
    struct tm *p_tm;
    rtc_parameter_struct rtc_initpara;
    uint32_t prescaler_a = 0, prescaler_s = 0;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

    prescaler_s = 0x13F;
    prescaler_a = 0x63;
    rtc_initpara.factor_asyn = prescaler_a;
    rtc_initpara.factor_syn = prescaler_s;
    rtc_initpara.second = IntDec_To_Hex(p_tm->tm_sec);
    rtc_initpara.minute = IntDec_To_Hex(p_tm->tm_min);
    rtc_initpara.hour   = IntDec_To_Hex(p_tm->tm_hour);
    rtc_initpara.date    = IntDec_To_Hex(p_tm->tm_mday);
    rtc_initpara.month   = IntDec_To_Hex(p_tm->tm_mon + 1);
    rtc_initpara.year    = IntDec_To_Hex(p_tm->tm_year - 100);
    rtc_initpara.day_of_week = IntDec_To_Hex(p_tm->tm_wday + 1);
    rtc_initpara.display_format = RTC_24HOUR;
    if(rtc_initpara.hour < 0x12)
        rtc_initpara.am_pm = RTC_AM;
    else
        rtc_initpara.am_pm = RTC_PM;

    rtc_init(&rtc_initpara);
    return OS_EOK;
}


static void gd32_rtc_init(os_uint32_t hrtc)
{
    /* enable PMU clock */
    rcu_periph_clock_enable(RCU_PMU);
    /* enable the access of the RTC registers */
    pmu_backup_write_enable();

    /* enable the IRC32K oscillator */
    rcu_osci_on(RCU_IRC32K);
    /* wait till IRC32K is ready */
    rcu_osci_stab_wait(RCU_IRC32K);
    /* select the RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_IRC32K);
    
    rcu_periph_clock_enable(RCU_RTC);
    rtc_register_sync_wait();
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = gd32_rtc_get_timestamp();
        LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (gd32_rtc_set_time_stamp( *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_EXT_D("RTC: set rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static int gd32_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct gd32_rtc *gd_rtc;

    gd_rtc = os_calloc(1, sizeof(struct gd32_rtc));

    OS_ASSERT(gd_rtc);

    gd_rtc->gd_rtc = (os_uint32_t)dev->info;

    gd32_rtc_init(gd_rtc->gd_rtc);

    os_device_default(&gd_rtc->rtc, OS_DEVICE_TYPE_RTC);
    
    gd_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&gd_rtc->rtc, dev->name, OS_DEVICE_FLAG_RDWR);
}

OS_DRIVER_INFO gd32_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = gd32_rtc_probe,
};

OS_DRIVER_DEFINE(gd32_rtc_driver, "1");

