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
 * @brief       This file implements rtc driver for cm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <sys/time.h>
#include "time.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DBG_TAG "drv.rtc"
#include <drv_log.h>

#include "cm32m101a_rtc.h"
#include "cm32m101a_gpio.h"
#include "cm32m101a_pwr.h"
#include "drv_rtc.h"

struct cm32_rtc {
    os_device_t rtc;
};

os_uint32_t SynchPrediv, AsynchPrediv;

#define RTC_Alarm RTC_A_ALARM
#define RTC_FORMAT RTC_24HOUR_FORMAT

static int set_current_time(time_t time_stamp)
{
    RTC_DateType RTC_DateSet = {0};
    RTC_TimeType RTC_TimeSet = {0};
    struct tm *p_tm;

    p_tm = localtime(&time_stamp);
    if (p_tm->tm_year < 100)
    {
        return OS_ERROR;
    }

    RTC_TimeSet.Seconds = p_tm->tm_sec;
    RTC_TimeSet.Minutes = p_tm->tm_min;
    RTC_TimeSet.Hours   = p_tm->tm_hour;
    RTC_DateSet.Date    = p_tm->tm_mday;
    RTC_DateSet.Month   = p_tm->tm_mon + 1;
    RTC_DateSet.Year    = p_tm->tm_year - 100;

    /* Configure the RTC date register */
    if (RTC_SetDate(RTC_FORMAT_BIN, &RTC_DateSet) == ERROR)
    {
        LOG_E(DBG_TAG, "RTC Set Date failed!");
        return OS_ERROR;
    }

    /* Configure the RTC time register */
    if (RTC_ConfigTime(RTC_FORMAT_BIN, &RTC_TimeSet) == ERROR)
    {
        LOG_E(DBG_TAG, "RTC Set Time failed!");
        return OS_ERROR;
    }

    return OS_EOK;
}

static time_t  get_current_time(void)
{
    RTC_DateType RTC_DateStructure = {0};
    RTC_TimeType RTC_TimeStructure = {0};
    struct tm       tm_new;

    RTC_GetDate(RTC_FORMAT_BIN, &RTC_DateStructure);

    tm_new.tm_mday = RTC_DateStructure.Date;
    tm_new.tm_mon  = RTC_DateStructure.Month - 1;
    tm_new.tm_year = RTC_DateStructure.Year + 100;

    RTC_GetTime(RTC_FORMAT_BIN, &RTC_TimeStructure);

    tm_new.tm_sec  = RTC_TimeStructure.Seconds;
    tm_new.tm_min  = RTC_TimeStructure.Minutes;
    tm_new.tm_hour = RTC_TimeStructure.Hours;

    LOG_D(DBG_TAG, "get rtc time.");
    return mktime(&tm_new);
}

static void RTC_PrescalerConfig(void)
{
    RTC_InitType RTC_InitStructure;

    /* Configure the RTC data register and RTC prescaler */
    RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
    RTC_InitStructure.RTC_SynchPrediv  = SynchPrediv;
    RTC_InitStructure.RTC_HourFormat   = RTC_FORMAT;

    /* Check on RTC init */
    if (RTC_Init(&RTC_InitStructure) == ERROR)
    {
        LOG_E(DBG_TAG, "RTC Prescaler Config failed!");
    }
}

void RTC_CLKSourceConfig(uint8_t ClkSrc, uint8_t FirstLastCfg, uint8_t RstBKP)
{
    assert_param(IS_CLKSRC_VALUE(ClkSrc));
    assert_param(IS_FLCFG_VALUE(FirstLastCfg));

    /* Enable the PWR clock */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);
    /* Allow access to RTC */
    PWR_BackupAccessEnable(ENABLE);

    /* Reset Backup */
    //if (RstBKP == 1)
    //{
    //    BKP_DeInit();
    //}
    /* Disable RTC clock */
    RCC_EnableRtcClk(DISABLE);

    if (ClkSrc == 0x01)
    {
        LOG_D(DBG_TAG, "RTC_ClkSrc Is Set HSE128!");
        if (FirstLastCfg == 0)
        {
            /* Enable HSE */
            RCC_EnableLsi(DISABLE);
            RCC_ConfigHse(RCC_HSE_ENABLE);
            while (RCC_WaitHseStable() == ERROR)
            {
            }

            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_HSE_DIV32);
        }
        else
        {
            RCC_EnableLsi(DISABLE);
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_HSE_DIV32);

            /* Enable HSE */
            RCC_ConfigHse(RCC_HSE_ENABLE);

            while (RCC_WaitHseStable() == ERROR)
            {
            }
        }

        SynchPrediv  = 0x7A0; /* 8M/32 = 250KHz */
        AsynchPrediv = 0x7F;  /* value range: 0-7F */
    }
    else if (ClkSrc == 0x02)
    {
        LOG_D(DBG_TAG, "RTC_ClkSrc Is Set LSE!");
        AFEC->TRIMR2 |= 0xFF;
        if (FirstLastCfg == 0)
        {
            /* Enable the LSE OSC32_IN PC14 */
            RCC_EnableLsi(DISABLE); /* LSI is turned off here to ensure that only one clock is turned on */

#if (_TEST_LSE_BYPASS_)
            RCC_ConfigLse(RCC_LSE_BYPASS);
#else
            RCC_ConfigLse(RCC_LSE_ENABLE);
#endif

            while (RCC_GetFlagStatus(RCC_LDCTRL_FLAG_LSERD) == RESET)
            {
            }

            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSE);
        }
        else
        {
            /* Enable the LSE OSC32_IN PC14 */
            RCC_EnableLsi(DISABLE);
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSE);

#if (_TEST_LSE_BYPASS_)
            RCC_ConfigLse(RCC_LSE_BYPASS);
#else
            RCC_ConfigLse(RCC_LSE_ENABLE);
#endif

            while (RCC_GetFlagStatus(RCC_LDCTRL_FLAG_LSERD) == RESET)
            {
            }
        }

        SynchPrediv  = 0xFF; /* 32.768KHz */
        AsynchPrediv = 0x7F; /* value range: 0-7F */
    }
    else if (ClkSrc == 0x03)
    {
        LOG_D(DBG_TAG, "RTC_ClkSrc Is Set LSI!");
        if (FirstLastCfg == 0)
        {
            /* Enable the LSI OSC */
            RCC_EnableLsi(ENABLE);

            while (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_LSIRD) == RESET)
            {
            }

            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSI);
        }
        else
        {
            RCC_ConfigRtcClk(RCC_RTCCLK_SRC_LSI);

            /* Enable the LSI OSC */
            RCC_EnableLsi(ENABLE);

            while (RCC_GetFlagStatus(RCC_CTRLSTS_FLAG_LSIRD) == RESET)
            {
            }
        }

        SynchPrediv  = 0x14A; /* 41828Hz */
        AsynchPrediv = 0x7F;  /* value range: 0-7F */
    }
    else
    {
        LOG_D(DBG_TAG, "RTC_ClkSrc Value is error!");
    }

    /* Enable the RTC Clock */
    RCC_EnableRtcClk(ENABLE);
    RTC_WaitForSynchro();
}

void RTCAlarm_Configuration(FunctionalState Cmd)
{
    EXTI_InitType EXTI_InitStructure;
    NVIC_InitType NVIC_InitStructure;

    EXTI_ClrITPendBit(EXTI_LINE18);
    EXTI_InitStructure.EXTI_Line = EXTI_LINE18;
#ifdef __TEST_SEVONPEND_WFE_NVIC_DIS__
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
#else
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
#endif
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_InitPeripheral(&EXTI_InitStructure);

    /* Enable the RTC Alarm Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel                   = RTCAlarm_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd                = Cmd;
    NVIC_Init(&NVIC_InitStructure);
}

static int set_alarm_time(time_t time_stamp)
{
    RTC_AlarmType RTC_AlarmStructure;
    RTC_TimeType RTC_TimeStructure = {0};
    RTC_DateType RTC_DateStructure = {0};

    RTC_GetDate(RTC_FORMAT_BIN, &RTC_DateStructure);
    RTC_GetTime(RTC_FORMAT_BIN, &RTC_TimeStructure);

    int tmp_ss, tmp_mm, tmp_hh, tmp_dd;

    tmp_ss = RTC_TimeStructure.Seconds + time_stamp;
    RTC_AlarmStructure.AlarmTime.Seconds = tmp_ss % 60;

    tmp_mm = RTC_TimeStructure.Minutes + tmp_ss / 60;

    RTC_AlarmStructure.AlarmTime.Minutes = tmp_mm % 60;

    tmp_hh = RTC_TimeStructure.Hours + tmp_mm / 60;

    if (RTC_FORMAT == RTC_24HOUR_FORMAT)
    {
        RTC_AlarmStructure.AlarmTime.Hours = tmp_hh % 24;
        tmp_dd = RTC_DateStructure.Date + tmp_hh / 24;
        RTC_AlarmStructure.DateWeekValue = tmp_dd;
        RTC_AlarmStructure.DateWeekMode = RTC_ALARM_SEL_WEEKDAY_DATE;
        LOG_D(DBG_TAG, "RTC ALARM: %d %d %d %d \r\n", RTC_AlarmStructure.DateWeekValue, RTC_AlarmStructure.AlarmTime.Hours,
                   RTC_AlarmStructure.AlarmTime.Minutes, RTC_AlarmStructure.AlarmTime.Seconds);
    }
    else
    {
        RTC_AlarmStructure.AlarmTime.H12 = RTC_TimeStructure.H12;
        LOG_D(DBG_TAG, "RTC RTC_12HOUR_FORMAT to do\r\n");
    }

    /* RTC_AlarmStructure.AlarmMask : RTC_ALARMMASK_WEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES | RTC_ALARMMASK_SECONDS */
    RTC_AlarmStructure.AlarmMask = RTC_ALARMMASK_NONE;
    /* Disable the AlarmX */
    RTC_EnableAlarm(RTC_Alarm, DISABLE);

    /* Configure the RTC Alarm A register */
    RTC_SetAlarm(RTC_FORMAT_BIN, RTC_Alarm, &RTC_AlarmStructure);
    LOG_D(DBG_TAG, "\n\r>> !! RTC Set Alarm_X success. !! <<\n\r");

    if (RTC_Alarm == RTC_A_ALARM)
    {
        /* Enable the RTC Alarm A Interrupt */
        RTC_ConfigInt(RTC_INT_ALRA, ENABLE);
    }
    else
    {
        /* Enable the RTC Alarm B Interrupt */
        RTC_ConfigInt(RTC_INT_ALRB, ENABLE);
    }
    /* Enable the alarm   */
    RTC_EnableAlarm(RTC_Alarm, ENABLE);
    return OS_EOK;
}

os_err_t  os_rtc_init(void)
{
#ifdef USE_HSI_PLL
    RTC_CLKSourceConfig(3, 0, 1);
#else
    RTC_CLKSourceConfig(2, 0, 1);
#endif
    RTC_PrescalerConfig();

    return OS_EOK;
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        *(os_uint32_t *)args = get_current_time();
        LOG_D(DBG_TAG, "RTC: get rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        if (set_current_time(*(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_D(DBG_TAG, "RTC: set rtc_time %x\n", *(os_uint32_t *)args);
        result = OS_EOK;
        break;

    case OS_DEVICE_CTRL_RTC_SET_ALARM:
        result = OS_EOK;
        if (set_alarm_time(*(os_uint32_t *)args))
        {
            result = OS_ERROR;
        }
        LOG_D(DBG_TAG, "RTC: set alarm time %x\n", *(os_uint32_t *)args);
        RTCAlarm_Configuration(ENABLE);
        break;

    default:
        break;
    }

    return result;
}

const static struct os_device_ops rtc_ops = {
    .control = os_rtc_control,
};

static os_err_t os_hw_rtc_register(os_device_t *device, const char *name)
{
    OS_ASSERT(device != OS_NULL);

    device->ops         = &rtc_ops;
    device->type        = OS_DEVICE_TYPE_RTC;
    device->user_data   = OS_NULL;

    /* register a character device */
    return os_device_register(device, name);
}

static int cm32_rtc_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t result;
    struct cm32_rtc *rtc;

    rtc = os_calloc(1, sizeof(struct cm32_rtc));

    OS_ASSERT(rtc);

    os_rtc_init();

    result = os_hw_rtc_register(&rtc->rtc, dev->name);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG, "rtc register err code: %d", result);
        return result;
    }
    LOG_D(DBG_TAG, "rtc init success");
    return OS_EOK;
}

OS_DRIVER_INFO cm32_rtc_driver = {
    .name   = "RTC_Type",
    .probe  = cm32_rtc_probe,
};

OS_DRIVER_DEFINE(cm32_rtc_driver, PREV, OS_INIT_SUBLEVEL_LOW);
