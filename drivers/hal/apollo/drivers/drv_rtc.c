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
 * @brief       This file implements rtc driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <os_device.h>
#include <os_irq.h>
#include <os_memory.h>
#include "am_mcu_apollo.h"

#define XT   1
#define LFRC 2

#define RTC_CLK_SRC XT

static os_err_t os_rtc_open(os_device_t *dev, os_uint16_t oflag)
{
    if (dev->cb_table[OS_DEVICE_CB_TYPE_RX].cb != OS_NULL)
    {
        /* Open Interrupt */
    }

    return OS_EOK;
}

static os_size_t os_rtc_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    return 0;
}

static os_err_t os_rtc_control(os_device_t *dev, int cmd, void *args)
{
    time_t           *time;
    struct tm         time_temp;
    struct tm        *time_new;
    am_hal_rtc_time_t hal_time;

    OS_ASSERT(dev != OS_NULL);
    memset(&time_temp, 0, sizeof(struct tm));

    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        time = (time_t *)args;

        /* Get the current Time */
        am_hal_rtc_time_get(&hal_time);

        /* Years since 1900 : 0-99 range */
        time_temp.tm_year = hal_time.ui32Year + 2000 - 1900;
        /* Months *since* january 0-11 : RTC_Month_Date_Definitions 1 - 12 */
        time_temp.tm_mon = hal_time.ui32Month - 1;
        /* Day of the month 1-31 : 1-31 range */
        time_temp.tm_mday = hal_time.ui32DayOfMonth;
        /* Hours since midnight 0-23 : 0-23 range */
        time_temp.tm_hour = hal_time.ui32Hour;
        /* Minutes 0-59 : the 0-59 range */
        time_temp.tm_min = hal_time.ui32Minute;
        /* Seconds 0-59 : the 0-59 range */
        time_temp.tm_sec = hal_time.ui32Second;

        *time = mktime(&time_temp);

        break;

    case OS_DEVICE_CTRL_RTC_SET_TIME:
        time     = (time_t *)args;
        time_new = localtime(time);

        hal_time.ui32Hour       = time_new->tm_hour;
        hal_time.ui32Minute     = time_new->tm_min;
        hal_time.ui32Second     = time_new->tm_sec;
        hal_time.ui32Hundredths = 00;
        hal_time.ui32Weekday    = time_new->tm_wday;
        hal_time.ui32DayOfMonth = time_new->tm_mday;
        hal_time.ui32Month      = time_new->tm_mon + 1;
        hal_time.ui32Year       = time_new->tm_year + 1900 - 2000;
        hal_time.ui32Century    = 0;

        am_hal_rtc_time_set(&hal_time);

        break;
    }

    return OS_EOK;
}

int os_hw_rtc_init(void)
{
    static struct os_device rtc;

#if RTC_CLK_SRC == LFRC
    /* Enable the LFRC for the RTC */
    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_LFRC);

    /* Select LFRC for RTC clock source */
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);
#endif

#if RTC_CLK_SRC == XT
    /* Enable the XT for the RTC */
    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);

    /* Select XT for RTC clock source */
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);
#endif

    /* Enable the RTC */
    am_hal_rtc_osc_enable();

    /* register rtc device */
    rtc.type    = OS_DEVICE_TYPE_RTC;
    rtc.init    = OS_NULL;
    rtc.open    = os_rtc_open;
    rtc.close   = OS_NULL;
    rtc.read    = os_rtc_read;
    rtc.write   = OS_NULL;
    rtc.control = os_rtc_control;

    /* no private */
    rtc.user_data = OS_NULL;

    os_device_register(&rtc, "rtc", OS_DEVICE_FLAG_RDWR);

    return 0;
}
#ifdef OS_USING_COMPONENTS_INIT
INIT_BOARD_EXPORT(os_hw_rtc_init);
#endif
