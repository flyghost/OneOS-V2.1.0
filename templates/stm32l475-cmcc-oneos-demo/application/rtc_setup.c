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
 * \@file        rtc_setup.c
 *
 * \@brief       Setup the rtc time.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_errno.h>
#include <sys/time.h>
#include <rtc/rtc.h>
#include <dlog.h>
#include <board.h>

#include "rtc_setup.h"

#ifdef OS_USING_SHELL
#include <shell.h>
#endif

#define DBG_TAG "CMCC_RTC"

#ifdef OS_USING_RTC
int rtc_setup_date(int argc, char *argv[])
{
    os_err_t ret = OS_EOK;
    time_t   now;
    os_uint32_t year,month,day;

    if (argc != 4)
    {
        os_kprintf("usage:setup date <year month day,>\r\n");
        return -1;
    }
    
    year  = atoi(argv[1]);
    month = atoi(argv[2]);
    day   = atoi(argv[3]);
    
    /* Set date */
    ret = set_date(year, month, day);
    if (ret != OS_EOK)
    {
        LOG_E(DBG_TAG, "set RTC date failed %d", (int)ret);
        return ret;
    }
    
    /* Get time */
    now = time(OS_NULL);
    os_kprintf("%s\r\n", ctime(&now));
    return ret;
}

SH_CMD_EXPORT(rtc_setup_date, rtc_setup_date, "set rtc date");

int rtc_setup_time(int argc, char *argv[])
{
    os_err_t ret = OS_EOK;
    time_t   now;
    os_uint32_t hour,minute,second;
    
    if (argc != 4)
    {
        os_kprintf("usage:setup date <hour minute second>\r\n");
        return -1;
    }
    
    hour   = atoi(argv[1]);
    minute = atoi(argv[2]);
    second = atoi(argv[3]);
    
    /* Set time */
    ret = set_time(hour, minute, second);
    if (ret != OS_EOK)
    {
        LOG_E(DBG_TAG, "set RTC time failed %d", (int)ret);
        return ret;
    }

    os_task_msleep(3000);

    /* Get time */
    now = time(OS_NULL);
    os_kprintf("%s\r\n", ctime(&now));
    return ret;
}

SH_CMD_EXPORT(rtc_setup_time, rtc_setup_time, "set rtc time");

#endif /*OS_USING_RTC*/
