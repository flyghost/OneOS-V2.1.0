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
 * @brief       This file implements RTC driver for fm33.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <sys/time.h>
#include <drv_log.h>
#include "drv_rtc.h"
#include <string.h>

#define DBG_TAG "drv.rtc"
#include <drv_log.h>
#include <dlog.h>

os_uint8_t Hex_To_ByteDec(uint8_t PuB_Dat)
{
    os_uint8_t dat;
    dat = (PuB_Dat / 16) * 10 + PuB_Dat % 16;
    return dat;
}

os_uint8_t IntDec_To_Hex(int PuB_Dat)
{
    os_uint8_t dat;
    dat = (PuB_Dat / 10) * 16 + PuB_Dat % 10;
    return dat;
}

os_uint8_t RTC_GetRTC(RTC_Type *rtc,FL_RTC_InitTypeDef *InitStructer)
{
    os_uint8_t n, i;
    os_uint8_t Result = 1;

    FL_RTC_InitTypeDef TempTime1, TempTime2;

    for(n = 0 ; n < 3; n++)
    {
        FL_RTC_GetTime(rtc, &TempTime1); //读一次时间
        FL_RTC_GetTime(rtc, &TempTime2); //再读一次时间

        for(i = 0; i < 7; i++) //两者一致, 表示读取成功
        {
            if(((os_uint32_t *)(&TempTime1))[i] != ((os_uint32_t *)(&TempTime2))[i]) { break; }
        }

        if(i == 7)
        {
            Result = 0;
            memcpy((os_uint32_t *)(InitStructer), (os_uint32_t *)(&TempTime1), 7 * sizeof(os_uint32_t)); //读取正确则更新新的时间
            break;
        }
    }

    return Result;
}

os_uint8_t RTC_SetRTC(RTC_Type *rtc,FL_RTC_InitTypeDef *InitStructer)
{
    os_uint8_t n, i;
    os_uint8_t Result;
    FL_RTC_InitTypeDef TempTime1;

    for(n = 0 ; n < 3; n++)
    {
        FL_RTC_ConfigTime(rtc, InitStructer);
        Result = RTC_GetRTC(rtc, &TempTime1);    //读取确认设置结果

        if(Result == 0)
        {
            Result = 1;

            for(i = 0; i < 7; i++) //两者一致, 表示设置成功
            {
                if(((os_uint32_t *)(&TempTime1))[i] != ((os_uint32_t *)(InitStructer))[i])
                {
                    break;
                }
            }

            if(i == 7)
            {
                Result = 0;
                break;
            }
        }
    }

    return Result;
}

void RTC_AlarmTimeSet(RTC_Type *rtc,struct fm33_alarm *alarm)
{
    FL_RTC_DisableIT_Alarm(rtc);
    FL_RTC_WriteHourAlarm(rtc, alarm->hour);
    FL_RTC_WriteMinuteAlarm(rtc, alarm->hour);
    FL_RTC_WriteSecondAlarm(rtc, alarm->hour);

    FL_RTC_ClearFlag_Alarm(rtc);
    FL_RTC_EnableIT_Alarm(rtc);
}

OS_WEAK void rtc_alarm_callback(void *args)
{
    LOG_I(DBG_TAG, "rtc alarm");
}

void RTC_IRQHandler(void)
{
    if(FL_ENABLE == FL_RTC_IsEnabledIT_Alarm(RTC) &&
        FL_SET == FL_RTC_IsActiveFlag_Alarm(RTC))
    {
        rtc_alarm_callback(OS_NULL);
        FL_RTC_ClearFlag_Alarm(RTC);
    }
}

static time_t fm33_rtc_get_time(struct fm33_rtc *rtc)
{
    time_t             ret;
    struct tm          tm_new;
    FL_RTC_InitTypeDef tm_rtc;

    if (RTC_GetRTC(rtc->info->instance, &tm_rtc) == 0)
    {
        tm_new.tm_sec  = Hex_To_ByteDec(tm_rtc.second);
        tm_new.tm_min  = Hex_To_ByteDec(tm_rtc.minute);
        tm_new.tm_hour = Hex_To_ByteDec(tm_rtc.hour);
        tm_new.tm_mday = Hex_To_ByteDec(tm_rtc.day);
        tm_new.tm_mon  = Hex_To_ByteDec(tm_rtc.month) - 1;
        tm_new.tm_year = Hex_To_ByteDec(tm_rtc.year) + 100;

        ret = mktime(&tm_new);
    }
    else
    {
        LOG_D(DBG_TAG, "get rtc time failed.");
    }

    return ret;
}

static os_err_t fm33_rtc_set_time(struct fm33_rtc *rtc, time_t time_stamp)
{
    struct tm *p_tm;
    os_int8_t  ret;
    FL_RTC_InitTypeDef rtc_time;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

    rtc_time.second =  IntDec_To_Hex(p_tm->tm_sec);
    rtc_time.minute =  IntDec_To_Hex(p_tm->tm_min);
    rtc_time.hour   =  IntDec_To_Hex(p_tm->tm_hour);
    rtc_time.day    = IntDec_To_Hex(p_tm->tm_mday);
    rtc_time.month   = IntDec_To_Hex(p_tm->tm_mon + 1);
    rtc_time.year    = IntDec_To_Hex(p_tm->tm_year - 100);


    if (RTC_SetRTC(rtc->info->instance, &rtc_time) == 0)
    {
        ret = OS_EOK;
    }
    else
    {
        ret = OS_ERROR;
        LOG_D(DBG_TAG, "set rtc time failed.");
    }

    return ret;
}

static os_err_t fm33_rtc_set_alarm(struct fm33_rtc *rtc, time_t time)
{
    os_ubase_t        level;
    struct tm        *p_tm;
    struct fm33_alarm alarm;

    level = os_irq_lock();

    p_tm = localtime(&time);
    if (p_tm->tm_year < 100)
    {
        os_irq_unlock(level);
        return OS_ERROR;
    }

    alarm.hour   = IntDec_To_Hex(p_tm->tm_hour);
    alarm.minute = IntDec_To_Hex(p_tm->tm_min);
    alarm.second = IntDec_To_Hex(p_tm->tm_sec);

    RTC_AlarmTimeSet(rtc->info->instance, &alarm);

    os_irq_unlock(level);

    return OS_EOK;
}

static void fm33_rtc_init(struct fm33_rtc *rtc)
{
    FL_RTC_InitTypeDef    defaultInitStruct = DEFAULT_RTC_TIME;

    FL_RTC_Init(rtc->info->instance,&defaultInitStruct );
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t         result = OS_ERROR;
    struct fm33_rtc *fm_rtc = OS_NULL;

    OS_ASSERT(dev != OS_NULL);

    fm_rtc = (struct fm33_rtc *)dev;

    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = fm33_rtc_get_time(fm_rtc);
        result = OS_EOK;
        break;
    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (fm33_rtc_set_time(fm_rtc, *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        result = OS_EOK;
        break;
    case OS_DEVICE_CTRL_RTC_SET_ALARM:
        if (fm33_rtc_set_alarm(fm_rtc, *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        result = OS_EOK;
        break;
    default:
        result = OS_ERROR;
        break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static int fm33_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct fm33_rtc *fm_rtc;

    fm_rtc = os_calloc(1, sizeof(struct fm33_rtc));

    OS_ASSERT(fm_rtc);

    fm_rtc->info = (struct fm33_rtc_info *)dev->info;

    fm33_rtc_init(fm_rtc);

    fm_rtc->rtc.type = OS_DEVICE_TYPE_RTC;
    fm_rtc->rtc.ops  = &rtc_ops;

    return os_device_register(&fm_rtc->rtc, dev->name);
}

OS_DRIVER_INFO fm33_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = fm33_rtc_probe,
};

OS_DRIVER_DEFINE(fm33_rtc_driver,PREV,OS_INIT_SUBLEVEL_LOW);


