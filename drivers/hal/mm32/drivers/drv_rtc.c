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
 * @brief       This file implements rtc driver for mm32.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.rtc"
#include <drv_log.h>

#include "drv_rtc.h"
#include "mm32_hal.h"

#define BKUP_REG_DATA 0x5050

struct mm32_rtc {

    os_device_t rtc;

    RTC_TypeDef *hrtc;
};

static time_t mm32_rtc_get_timestamp(RTC_TypeDef *hrtc)
{
    time_t tim;
    tim = RTC_GetCounter();
    LOG_D(DRV_EXT_TAG, "get rtc time.");
    return tim;
}

static os_err_t mm32_rtc_set_time_stamp(RTC_TypeDef *hrtc, time_t time_stamp)
{
    PWR_BackupAccessCmd(ENABLE);
    RTC_SetCounter(time_stamp);
    PWR_BackupAccessCmd(DISABLE);
    LOG_D(DRV_EXT_TAG, "set rtc time.");
    return OS_EOK;
}

static void mm32_rtc_init(struct mm32_rtc_info * rtc_info)
{   
    uint8_t temp = 0;
    rtc_info->rcc_init_func(rtc_info->rtc_clk, ENABLE);

    if (BKP_ReadBackupRegister(BKP_DR1) != BKUP_REG_DATA)
    {
        BKP_DeInit();
        PWR_BackupAccessCmd(ENABLE);
#if defined(BSP_RTC_USING_LSI)
        RCC_LSEConfig(RCC_LSE_OFF);
        RCC_LSICmd(ENABLE);

        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        {
            temp++;
            if(temp >= 40)
                return;
            os_task_msleep(100);
        }       
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#elif defined(BSP_RTC_USING_LSE)
        RCC_LSEConfig(RCC_LSE_ON);
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	
        {
            temp++;
            if(temp >= 40)
                return;
            os_task_msleep(100);
        }       
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#endif
        RCC_RTCCLKCmd(ENABLE);
        RTC_WaitForLastTask();
        RTC_WaitForSynchro();

        RTC_EnterConfigMode();
        RTC_SetPrescaler(32767); 
         
        BKP_WriteBackupRegister(BKP_DR1, BKUP_REG_DATA);
        RTC_WaitForLastTask();
        RTC_WaitForSynchro();
        RTC_ExitConfigMode();
        
        PWR_BackupAccessCmd(DISABLE);
    }
    else
    {
        RTC_WaitForSynchro();
        RTC_ITConfig(RTC_IT_SEC, ENABLE);
        RTC_WaitForLastTask();
    }
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    struct mm32_rtc *m_rtc = (struct mm32_rtc *)dev;

    switch (cmd)
    {
        case OS_DEVICE_CTRL_RTC_GET_TIME:
            *(os_uint32_t *)args = mm32_rtc_get_timestamp(m_rtc->hrtc);
            LOG_D(DRV_EXT_TAG, "RTC: get rtc_time %x\r\n", *(os_uint32_t *)args);
            result = OS_EOK;
            break;

        case OS_DEVICE_CTRL_RTC_SET_TIME:
            if (mm32_rtc_set_time_stamp(m_rtc->hrtc, *(os_uint32_t *)args))
            {
                result = OS_ERROR;
            }
            LOG_D(DRV_EXT_TAG, "RTC: set rtc_time %x\r\n", *(os_uint32_t *)args);
            result = OS_EOK;
            break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static int mm32_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct mm32_rtc *m_rtc;
    struct mm32_rtc_info *rtc_info;

    m_rtc = os_calloc(1, sizeof(struct mm32_rtc));

    OS_ASSERT(m_rtc);
    
    rtc_info = (struct mm32_rtc_info *)dev->info;
    m_rtc->hrtc = (RTC_TypeDef *)rtc_info->hrtc;
    mm32_rtc_init(rtc_info);

    m_rtc->rtc.ops     = &rtc_ops;

    return os_device_register(&m_rtc->rtc, dev->name);
}

OS_DRIVER_INFO mm32_rtc_driver = {
    .name   = "RTC_TypeDef",
    .probe  = mm32_rtc_probe,
};

OS_DRIVER_DEFINE(mm32_rtc_driver, PREV, OS_INIT_SUBLEVEL_LOW);
