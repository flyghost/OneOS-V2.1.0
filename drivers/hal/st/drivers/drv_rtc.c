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
 * @brief       This file implements RTC driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>
#include <os_clock.h>
#include <rtc/rtc.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DBG_TAG "drv.rtc"
#include <drv_log.h>
#include <dlog.h>

#define BKUP_REG_DATA 0x3C5A
#define RTC_TIME_S  (946686600)     /* 2020-01-01 8:30:00 */

struct stm32_rtc {
    os_device_t rtc;
    
    RTC_HandleTypeDef *hrtc;
};

extern RTC_HandleTypeDef hrtc;
void RTC_Alarm_IRQHandler(void)
{
    HAL_RTC_AlarmIRQHandler(&hrtc);
}


void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    LOG_D(DBG_TAG, "[%s]-[%d], cur[%d]\r\n", __FILE__, __LINE__, os_tick_get());
}

static time_t stm32_rtc_get_timestamp(RTC_HandleTypeDef *hrtc)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};
    struct tm       tm_new;

    HAL_RTC_GetTime(hrtc, &RTC_TimeStruct, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(hrtc, &RTC_DateStruct, RTC_FORMAT_BIN);

    tm_new.tm_sec  = RTC_TimeStruct.Seconds;
    tm_new.tm_min  = RTC_TimeStruct.Minutes;
    tm_new.tm_hour = RTC_TimeStruct.Hours;
    tm_new.tm_mday = RTC_DateStruct.Date;
    tm_new.tm_mon  = RTC_DateStruct.Month - 1;
    tm_new.tm_year = RTC_DateStruct.Year + 100;

    LOG_D(DBG_TAG,"get rtc time.");
    return mktime(&tm_new);
}

static os_err_t stm32_rtc_set_time_stamp(RTC_HandleTypeDef *hrtc, time_t time_stamp)
{
    RTC_TimeTypeDef RTC_TimeStruct = {0};
    RTC_DateTypeDef RTC_DateStruct = {0};
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

    RTC_TimeStruct.Seconds = p_tm->tm_sec;
    RTC_TimeStruct.Minutes = p_tm->tm_min;
    RTC_TimeStruct.Hours   = p_tm->tm_hour;
    RTC_DateStruct.Date    = p_tm->tm_mday;
    RTC_DateStruct.Month   = p_tm->tm_mon + 1;
    RTC_DateStruct.Year    = p_tm->tm_year - 100;
    RTC_DateStruct.WeekDay = p_tm->tm_wday + 1;

    HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR1, ~BKUP_REG_DATA);

    if (HAL_RTC_SetTime(hrtc, &RTC_TimeStruct, RTC_FORMAT_BIN) != HAL_OK)
    {
        return OS_ERROR;
    }
    if (HAL_RTC_SetDate(hrtc, &RTC_DateStruct, RTC_FORMAT_BIN) != HAL_OK)
    {
        return OS_ERROR;
    }

    LOG_D(DBG_TAG,"set rtc time.");
    HAL_RTCEx_BKUPWrite(hrtc, RTC_BKP_DR1, BKUP_REG_DATA);
    return OS_EOK;
}

void RTC_Set_AlarmA(RTC_HandleTypeDef *hrtc, struct tm *p_tm)
{ 
    #if defined(SERIES_STM32L4)
    RTC_AlarmTypeDef RTC_AlarmSturuct = {0};
    
    RTC_AlarmSturuct.AlarmTime.Hours = p_tm->tm_hour;  
    RTC_AlarmSturuct.AlarmTime.Minutes = p_tm->tm_min; 
    RTC_AlarmSturuct.AlarmTime.Seconds = p_tm->tm_sec;
    RTC_AlarmSturuct.AlarmTime.SubSeconds = 0;
    RTC_AlarmSturuct.AlarmTime.TimeFormat = RTC_HOURFORMAT12_AM;
    
    RTC_AlarmSturuct.AlarmMask = RTC_ALARMMASK_NONE;
    RTC_AlarmSturuct.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_NONE;
    RTC_AlarmSturuct.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
    RTC_AlarmSturuct.AlarmDateWeekDay = p_tm->tm_wday + 1; 
    RTC_AlarmSturuct.Alarm = RTC_ALARM_A;     
    HAL_RTC_SetAlarm_IT(hrtc, &RTC_AlarmSturuct, RTC_FORMAT_BIN);
    
    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0x01, 0x02); 
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
    #else
    LOG_E(DBG_TAG, "Not support rtc alarm!\r\n");
    #endif

    return;
}


static os_err_t stm32_rtc_set_alarm_stamp(RTC_HandleTypeDef *hrtc, time_t time_stamp)
{
    struct tm *p_tm;
    os_ubase_t level;
    
    level = os_irq_lock();
    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        os_irq_unlock(level);
        return OS_ERROR;
    }

    RTC_Set_AlarmA(hrtc, p_tm);
    os_irq_unlock(level);
    
    return OS_EOK;
}

void RTC_AlarmAEventStop(void)
{
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);
}


static void stm32_rtc_init(RTC_HandleTypeDef *hrtc)
{    
    if (HAL_RTCEx_BKUPRead(hrtc, RTC_BKP_DR1) != BKUP_REG_DATA)
    {
        if (stm32_rtc_set_time_stamp(hrtc, RTC_TIME_S))
        {
            LOG_E(DBG_TAG,"RTC: set rtc time err !time[%d]\r\n", RTC_TIME_S);
            return;
        }
        LOG_D(DBG_TAG, "RTC: set rtc_time %d\r\n", RTC_TIME_S);
    }
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    struct stm32_rtc *st_rtc = (struct stm32_rtc *)dev;
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = stm32_rtc_get_timestamp(st_rtc->hrtc);
        LOG_D(DBG_TAG, "RTC: get rtc_time %x\r\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (stm32_rtc_set_time_stamp(st_rtc->hrtc, *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_D(DBG_TAG, "RTC: set rtc_time %x\r\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;
    case OS_DEVICE_CTRL_RTC_SET_ALARM:
        if (stm32_rtc_set_alarm_stamp(st_rtc->hrtc, *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        result = OS_EOK;
        break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static int stm32_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct stm32_rtc *st_rtc;

    st_rtc = os_calloc(1, sizeof(struct stm32_rtc));

    OS_ASSERT(st_rtc);

    st_rtc->hrtc = (RTC_HandleTypeDef *)dev->info;

    stm32_rtc_init(st_rtc->hrtc);

    st_rtc->rtc.type = OS_DEVICE_TYPE_RTC;
    st_rtc->rtc.ops  = &rtc_ops;

    return os_device_register(&st_rtc->rtc, dev->name);
}

OS_DRIVER_INFO stm32_rtc_driver = {
    .name   = "RTC_HandleTypeDef",
    .probe  = stm32_rtc_probe,
};

OS_DRIVER_DEFINE(stm32_rtc_driver,PREV,OS_INIT_SUBLEVEL_LOW);

