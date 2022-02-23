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

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

#include "drv_rtc.h"

#define BKUP_REG_DATA 0x3C5A

struct gd32_rtc {
    os_device_t rtc;
    
    os_uint32_t hrtc;
};


static time_t gd32_rtc_get_timestamp(os_uint32_t hrtc)
{
    time_t ret;
    ret = rtc_counter_get();
    return ret;
}

static os_err_t gd32_rtc_set_time_stamp(os_uint32_t hrtc, time_t time_stamp)
{
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    /* change the current time */
    rtc_counter_set(time_stamp);
    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
    
    return OS_EOK;
}

static void gd32_rtc_init(os_uint32_t hrtc)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_LXTAL);
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_LXTAL);
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_LXTAL);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(32767);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    struct gd32_rtc *st_rtc = (struct gd32_rtc *)dev;
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = gd32_rtc_get_timestamp(st_rtc->hrtc);
        LOG_EXT_D("RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (gd32_rtc_set_time_stamp(st_rtc->hrtc, *(os_uint32_t *)args))
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
    struct gd32_rtc *st_rtc;

    st_rtc = os_calloc(1, sizeof(struct gd32_rtc));

    OS_ASSERT(st_rtc);

    st_rtc->hrtc = (os_uint32_t)dev->info;

    gd32_rtc_init(st_rtc->hrtc);

    os_device_default(&st_rtc->rtc, OS_DEVICE_TYPE_RTC);
    
    st_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&st_rtc->rtc, dev->name, OS_DEVICE_FLAG_RDWR);
}

OS_DRIVER_INFO gd32_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = gd32_rtc_probe,
};

OS_DRIVER_DEFINE(gd32_rtc_driver, "1");

