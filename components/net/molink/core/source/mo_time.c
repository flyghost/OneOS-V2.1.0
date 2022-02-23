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
 * @file        mo_time.c
 *
 * @brief       module general inner function
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_time.h"
#include <os_stddef.h>

static os_bool_t in_leap_year(os_int32_t year)
{
    return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? OS_TRUE : OS_FALSE;
}

static os_int32_t day_of_the_week(os_int32_t year, os_int32_t month, os_int32_t day)
{
    os_int32_t y,m;

    /* Kim larsen calculation formula: w=(d+2m+3(m+1)/5+y+y/4-y/100+y/400+1)%7 */
    if (1 == month || 2 == month) 
    {
        m = month + 12;
        y = year  - 1;
    }
    return (day+2*m+3*(m+1)/5+y+y/4-y/100+y/400+1)%7;
}

/**
 ***********************************************************************************************************************
 * @brief           Convert mo_tm_t to basic struct tm
 *
 * @details         Convert original time info to basic struct tm
 *                  Attention: it will take changes to source stuct info.
 *
 * @param[in]       mo_tm           mo_tm_t that hold original time info
 * @param[in]       _tm             pointer to struct tm that user gives to
 * 
 * @return          void
 ***********************************************************************************************************************
 */
void time_struct_convert(_mo_tm_t *mo_tm, struct tm *_tm)
{
    os_int32_t days_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; /* days table of an year */

    /* UTC time without timezone cast, if in the leap year, set days_table */
    if (in_leap_year(mo_tm->tm_year))
    {
        days_table[1] = 29;  // when leap year, feburary has 29 days
    }

    /* set month & years for struct tm: (Month.[0-11]. Year since 1900.) */
    mo_tm->tm_mon   -= 1;    // struct's tm_mon  [0-11]
    mo_tm->tm_year  -= 1900; // struct's tm_year since 1900

    /* timezone handle process */
    if (0 == mo_tm->tm_q_off)
    {
        goto __final;
    }

    /* Hours */
    mo_tm->tm_hour += mo_tm->tm_q_off / 4;
    if (0 > mo_tm->tm_hour || 24 <= mo_tm->tm_hour)
    {
        mo_tm->tm_hour = ((0 < mo_tm->tm_q_off) ? -mo_tm->tm_hour : mo_tm->tm_hour) + 24;
    }
    else 
    {
        goto __final; // not enffect the month day count, finished.
    }

    /* days-month-years */
    if (0 < mo_tm->tm_q_off)
    {
        // next day is not in the next month
        if (mo_tm->tm_mday <= days_table[mo_tm->tm_mon] - 1) 
        {
            mo_tm->tm_mday++;
        }
        // next month is not in the next year
        else if (11 > mo_tm->tm_mon + 1) 
        {
            mo_tm->tm_mday = 1;
            mo_tm->tm_mon++;
        }
        // next day is in the next year's first month
        else 
        {
            mo_tm->tm_mday = 1;
            mo_tm->tm_mon  = 0;
            mo_tm->tm_year++;
            if (in_leap_year(mo_tm->tm_year + 1900)) days_table[1] = 29;
        }
    }
    else 
    {
        // previous day is not in the previous month
        if (1 < mo_tm->tm_mday) 
        {
            mo_tm->tm_mday--;
        }
        // previous month is not in the previous year
        else if (0 <= mo_tm->tm_mon - 1) 
        {
            mo_tm->tm_mday = days_table[mo_tm->tm_mon - 1];
            mo_tm->tm_mon--;
        }
        // previous day is in the last year's december
        else 
        {
            mo_tm->tm_mon  = 11;
            mo_tm->tm_mday = days_table[mo_tm->tm_mon];
            mo_tm->tm_year--;
            if (in_leap_year(mo_tm->tm_year + 1900)) days_table[1] = 29;
        }
    }

__final:

    /* caculate the tm_wday: Kim larsen calculation formula */
    _tm->tm_wday = day_of_the_week(mo_tm->tm_year + 1900, mo_tm->tm_mon + 1, mo_tm->tm_mday);

    /* caculate the tm_yday: days in year */
    for (int i = mo_tm->tm_mon - 1; i >= 0; i--)
    {
        _tm->tm_yday += days_table[i];
    }
    _tm->tm_yday += mo_tm->tm_mday - 1;

    /* for readable reason, copy param here. */
    _tm->tm_isdst = -1;             // -1 DST means let system choose
    _tm->tm_sec   = mo_tm->tm_sec;
    _tm->tm_min   = mo_tm->tm_min;
    _tm->tm_hour  = mo_tm->tm_hour;
    _tm->tm_mday  = mo_tm->tm_mday;
    _tm->tm_mon   = mo_tm->tm_mon;
    _tm->tm_year  = mo_tm->tm_year;

    return;
}
