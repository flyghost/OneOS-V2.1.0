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
 * @file        gmtime_r.c
 *
 * @brief       This file provides some common time related interfaces.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-14   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_stddef.h>
#if defined(__CC_ARM) || defined(__CLANG_ARM) || defined (__IAR_SYSTEMS_ICC__)  
#include <sys/time.h>

/* Seconds per day. */
#define SPD (24 * 60 * 60)

/* Days per month -- nonleap! */
const short __spm[13] =
{
    0,
    (31),
    (31 + 28),
    (31 + 28 + 31),
    (31 + 28 + 31 + 30),
    (31 + 28 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),
    (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31),
};

int __isleap(int year)
{
    /* Every fourth year is a leap year except for century years that are
     * not divisible by 400. */
    /*  Return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)); */
    return (!(year % 4) && ((year % 100) || !(year % 400)));
}

/**
 ***********************************************************************************************************************
 * @brief           This function will convert Time (Restartable).
 *
 * @param[in]       timep     The timestamp.
 * @param[out]      r         Structure to stores information.
 *
 * @return          The structure to stores information.
 ***********************************************************************************************************************
 */
struct tm *gmtime_r(const time_t *timep, struct tm *r)
{
    time_t i;
    register time_t work = *timep % (SPD);
    
    r->tm_sec  = work % 60;
    work      /= 60;
    r->tm_min  = work % 60;
    r->tm_hour = work / 60;
    work       = *timep / (SPD);
    r->tm_wday = (4 + work) % 7;

    for (i = 1970; ; ++i)
    {
        register time_t k = __isleap(i) ? 366 : 365;
        if (work >= k)
        {
            work -= k;
        }
        else
        {
            break;
        }
    }
    r->tm_year = i - 1900;
    r->tm_yday = work;

    r->tm_mday = 1;
    if (__isleap(i) && (work > 58))
    {
        if (work == 59)
        {
            r->tm_mday = 2; /* 29.2. */
        }
        
        work -= 1;
    }

    for (i = 11; i && (__spm[i] > work); --i);
    
    r->tm_mon = i;
    r->tm_mday += work - __spm[i];
    
    return r;
}
#endif /* end of __CC_ARM or __CLANG_ARM or __IAR_SYSTEMS_ICC__ */

