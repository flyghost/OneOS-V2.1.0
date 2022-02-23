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


struct fm33_rtc 
{
    os_device_t                 rtc;
    struct fm33_rtc_info       *info;
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

#ifdef BSP_USING_RTCA
uint8_t RTC_GetRTCA(FL_RTCA_InitTypeDef *InitStructer)
{
    uint8_t n, i;
    uint8_t Result = 1;

    FL_RTCA_InitTypeDef TempTime1, TempTime2;

    for(n = 0 ; n < 3; n++)
    {
        FL_RTCA_GetTime(RTCA, &TempTime1);
        FL_RTCA_GetTime(RTCA, &TempTime2);

        for(i = 0; i < 7; i++)
        {
            if(((uint32_t *)(&TempTime1))[i] != ((uint32_t *)(&TempTime2))[i]) { break; }
        }

        if(i == 7)
        {
            Result = 0;
            memcpy((uint32_t *)(InitStructer), (uint32_t *)(&TempTime1), 7 * sizeof(uint32_t));
            break;
        }
    }

    return Result;
}

uint8_t RTC_SetRTCA(FL_RTCA_InitTypeDef *InitStructer)
{
    uint8_t n, i;
    uint8_t Result;
    FL_RTCA_InitTypeDef TempTime1;

    for(n = 0 ; n < 3; n++)
    {
        FL_RTCA_ConfigTime(RTCA, InitStructer);
        Result = RTC_GetRTCA(&TempTime1);

        if(Result == 0)
        {
            Result = 1;

            for(i = 0; i < 7; i++)
            {
                if(((uint32_t *)(&TempTime1))[i] != ((uint32_t *)(InitStructer))[i])
                { break; }
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

uint8_t RTCA_SetAlarm(struct tm *p_tm)
{
    struct fm33_alarm alarm;

    alarm.hour   = IntDec_To_Hex(p_tm->tm_hour);
    alarm.minute = IntDec_To_Hex(p_tm->tm_min);
    alarm.second = IntDec_To_Hex(p_tm->tm_sec);

    FL_RTCA_WriteHourAlarm(RTCA, alarm.hour);
    FL_RTCA_WriteMinuteAlarm(RTCA, alarm.minute);
    FL_RTCA_WriteSecondAlarm(RTCA, alarm.second);

    FL_RTCA_ClearFlag_Alarm(RTCA);
    FL_RTCA_EnableIT_Alarm(RTCA);
    NVIC_DisableIRQ(RTCx_IRQn);
    NVIC_SetPriority(RTCx_IRQn, 2);
    NVIC_EnableIRQ(RTCx_IRQn);

    return OS_EOK;
}

#endif

#ifdef BSP_USING_RTCB
uint8_t RTC_GetRTCB(FL_RTCB_InitTypeDef *InitStructer)
{
    uint8_t n, i;
    uint8_t Result = 1;

    FL_RTCB_InitTypeDef TempTime1, TempTime2;

    for(n = 0 ; n < 3; n++)
    {
        FL_RTCB_GetTime(RTCB, &TempTime1);
        FL_RTCB_GetTime(RTCB, &TempTime2);

        for(i = 0; i < 7; i++)
        {
            if(((uint32_t *)(&TempTime1))[i] != ((uint32_t *)(&TempTime2))[i]) { break; }
        }

        if(i == 7)
        {
            Result = 0;
            memcpy((uint32_t *)(InitStructer), (uint32_t *)(&TempTime1), 7 * sizeof(uint32_t)); 
            break;
        }
    }

    return Result;
}

uint8_t RTC_SetRTCB(FL_RTCB_InitTypeDef *InitStructer)
{
    uint8_t n, i;
    uint8_t Result;
    FL_RTCB_InitTypeDef TempTime1;

    for(n = 0 ; n < 3; n++)
    {
        FL_RTCB_ConfigTime(RTCB, InitStructer);
        Result = RTC_GetRTCB(&TempTime1);

        if(Result == 0)
        {
            Result = 1;

            for(i = 0; i < 7; i++)
            {
                if(((uint32_t *)(&TempTime1))[i] != ((uint32_t *)(InitStructer))[i])
                { break; }
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
#endif
OS_WEAK void rtc_alarm_callback(void *args)
{
    LOG_I(DBG_TAG, "rtc alarm");
}

void RTC_IRQHandler(void)
{
#ifdef BSP_USING_RTCA
    if(FL_ENABLE == FL_RTCA_IsEnabledIT_Alarm(RTCA) &&
            FL_SET == FL_RTCA_IsActiveFlag_Alarm(RTCA))
    {
        rtc_alarm_callback(OS_NULL);
        FL_RTCA_ClearFlag_Alarm(RTCA);
    }
#endif
}

static time_t fm33_rtc_get_timestamp(rtc_type_t type)
{
    time_t          ret;
    struct tm       tm_new;

#ifdef BSP_USING_RTCA
    if(type == TYPE_RTCA)
    {
        FL_RTCA_InitTypeDef rtca_time;
        RTC_GetRTCA(&rtca_time);
        
        tm_new.tm_sec  = Hex_To_ByteDec(rtca_time.second);
        tm_new.tm_min  = Hex_To_ByteDec(rtca_time.minute);
        tm_new.tm_hour = Hex_To_ByteDec(rtca_time.hour);
        tm_new.tm_mday = Hex_To_ByteDec(rtca_time.week);
        tm_new.tm_mon  = Hex_To_ByteDec(rtca_time.month) - 1;
        tm_new.tm_year = Hex_To_ByteDec(rtca_time.year) + 100;
    }
#endif

#ifdef BSP_USING_RTCB
    if(type == TYPE_RTCB)
    {
        FL_RTCB_InitTypeDef rtcb_time;
        RTC_GetRTCB(&rtcb_time);
        tm_new.tm_sec  = Hex_To_ByteDec(rtcb_time.second);
        tm_new.tm_min  = Hex_To_ByteDec(rtcb_time.minute);
        tm_new.tm_hour = Hex_To_ByteDec(rtcb_time.hour);
        tm_new.tm_mday = Hex_To_ByteDec(rtcb_time.week);
        tm_new.tm_mon  = Hex_To_ByteDec(rtcb_time.month) - 1;
        tm_new.tm_year = Hex_To_ByteDec(rtcb_time.year) + 100;
    }
#endif

    LOG_D(DBG_TAG, "get rtc time.");
    ret = mktime(&tm_new);

    return ret;
}

static os_err_t fm33_rtc_set_time_stamp(rtc_type_t type, time_t time_stamp)
{
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

#ifdef BSP_USING_RTCA
    if(type ==TYPE_RTCA)
    {
        FL_RTCA_InitTypeDef rtca_time;
        
        rtca_time.second =  IntDec_To_Hex(p_tm->tm_sec);
        rtca_time.minute =  IntDec_To_Hex(p_tm->tm_min);
        rtca_time.hour   =  IntDec_To_Hex(p_tm->tm_hour);
        rtca_time.week    = IntDec_To_Hex(p_tm->tm_mday);
        rtca_time.month   = IntDec_To_Hex(p_tm->tm_mon + 1);
        rtca_time.year    = IntDec_To_Hex(p_tm->tm_year - 100);
        
        RTC_SetRTCA(&rtca_time);
    }
#endif

#ifdef BSP_USING_RTCB
        FL_RTCB_InitTypeDef rtcb_time;

        rtcb_time.second =  IntDec_To_Hex(p_tm->tm_sec);
        rtcb_time.minute =  IntDec_To_Hex(p_tm->tm_min);
        rtcb_time.hour   =  IntDec_To_Hex(p_tm->tm_hour);
        rtcb_time.week    = IntDec_To_Hex(p_tm->tm_mday);
        rtcb_time.month   = IntDec_To_Hex(p_tm->tm_mon + 1);
        rtcb_time.year    = IntDec_To_Hex(p_tm->tm_year - 100);
        RTC_SetRTCB(&rtcb_time);
#endif

    return OS_EOK;
}

static os_err_t fm33_rtc_set_alarm_stamp(rtc_type_t type, time_t time_stamp)
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
#ifdef BSP_USING_RTCA
    if(type ==TYPE_RTCA)
    {
        RTCA_SetAlarm(p_tm);
    }
#endif

#ifdef BSP_USING_RTCB
    if(type ==TYPE_RTCB)
    {
        LOG_W(DBG_TAG, "RTCB not support alarm.");
        os_irq_unlock(level);
        return OS_ERROR;
    }
#endif
    os_irq_unlock(level);

    return OS_EOK;
}

static void fm33_rtc_init(struct fm33_rtc *rtc)
{
#ifdef BSP_USING_RTCA
    if(rtc->info->type == TYPE_RTCA)
    {
        FL_RTCA_InitTypeDef      rtca_init_time = DEFAULT_RTC_TIME;

        FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA);
        FL_RTCA_Enable(RTCA);
        FL_RTCA_Init(RTCA, &rtca_init_time);
        NVIC_DisableIRQ(RTCx_IRQn);
    }
#endif

#ifdef BSP_USING_RTCB
    if(rtc->info->type == TYPE_RTCB)
    {
        FL_RTCB_InitTypeDef      rtcb_init_time = DEFAULT_RTC_TIME;

        FL_RTCB_Init(RTCB, &rtcb_init_time);
        FL_RTCB_Enable(RTCB);
        NVIC_DisableIRQ(RTCx_IRQn);
    }
#endif
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t    result = OS_ERROR;
    rtc_type_t  type;

    OS_ASSERT(dev != OS_NULL);

#ifdef OS_RTC_SOURCE_RTCA
    type = TYPE_RTCA;
#else
    type = TYPE_RTCB;
#endif

    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = fm33_rtc_get_timestamp(type);
        LOG_D(DBG_TAG, "get rtc_time %x", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (fm33_rtc_set_time_stamp(type, *(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_D(DBG_TAG, "set rtc_time %x", *(os_uint32_t *)args);
        result = OS_EOK;
        break;
    case OS_DEVICE_CTRL_RTC_SET_ALARM:
        if (fm33_rtc_set_alarm_stamp(type, *(os_uint32_t *)args))
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


