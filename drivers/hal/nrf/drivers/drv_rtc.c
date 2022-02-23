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
 * @brief       This file implements RTC driver for nrf5.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */


#include <os_memory.h>
#include <os_assert.h>
#include <sys/time.h>
#include <bus.h>
#include "drv_rtc_52.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DBG_TAG "drv.rtc"
#include <drv_log.h>
#include <dlog.h>

static uint8_t s_timeCount1second = 0;
volatile uint32_t g_timestamp = 1609430400;

struct nrf5_rtc {
    os_device_t rtc;
    
    os_uint32_t nrf_rtc;
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

static time_t nrf5_rtc_get_timestamp(void)
{
    return (time_t)g_timestamp;

}

static os_err_t nrf5_rtc_set_time_stamp(time_t time_stamp)
{
    g_timestamp = time_stamp;
    return OS_EOK;
}

static void rtcCallbackFunc(nrf_drv_rtc_int_type_t interruptType)
{
    if(interruptType == NRF_DRV_RTC_INT_COMPARE0)			
    {
    }
    else if(interruptType == NRF_DRV_RTC_INT_TICK)	
    {
        if(s_timeCount1second >= 7)	// 125ms * 8 = 1s 
        {
            s_timeCount1second = 0;
            g_timestamp++;		
        }
        else
        {
            s_timeCount1second++;
        }
    }
}


static void nrf5_rtc_init(struct nrf5_rtc_info *info)
{
    nrf_drv_rtc_config_t rtcConfig = NRF_DRV_RTC_DEFAULT_CONFIG;        //Initialize RTC instance
    rtcConfig.prescaler = 4095; 
    
    nrf_drv_rtc_init(&info->rtc_dev, &rtcConfig, rtcCallbackFunc);

    nrf_drv_rtc_tick_enable(&info->rtc_dev, true);                        // Enable tick event & interrupt 
    nrf_drv_rtc_enable(&info->rtc_dev);                                   

}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = nrf5_rtc_get_timestamp();
        LOG_D(DBG_TAG, "RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (nrf5_rtc_set_time_stamp( *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_D(DBG_TAG, "RTC: set rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static int nrf5_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct nrf5_rtc *nrf_rtc;
    struct nrf5_rtc_info *info;

    nrf_rtc = os_calloc(1, sizeof(struct nrf5_rtc));

    OS_ASSERT(nrf_rtc);

    info = (struct nrf5_rtc_info *)dev->info;

    nrf5_rtc_init(info);
    
    nrf_rtc->rtc.type = OS_DEVICE_TYPE_RTC;
    nrf_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&nrf_rtc->rtc, dev->name);
}

OS_DRIVER_INFO nrf5_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = nrf5_rtc_probe,
};

//OS_DRIVER_DEFINE(nrf5_rtc_driver, PREV, OS_INIT_SUBLEVEL_LOW);

