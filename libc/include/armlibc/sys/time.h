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
 * @file        time.h
 *
 * @brief       Supplement to the standard C library file.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-13   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <time.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _TIMEVAL_DEFINED
#define _TIMEVAL_DEFINED

/* Structure returned by gettimeofday(2) system call, and used in other calls. */
struct timeval
{
    long tv_sec;        /* Seconds. */
    long tv_usec;       /* Microseconds. */
};
#endif /* _TIMEVAL_DEFINED */

#ifndef _TIMESPEC_DEFINED
#define _TIMESPEC_DEFINED

/* Structure defined by POSIX.1b to be like a timeval. */
struct timespec
{
    time_t  tv_sec;     /* Seconds. */
    long    tv_nsec;    /* Nanoseconds. */
};
#endif /* _TIMESPEC_DEFINED */ 

struct timezone
{
    int tz_minuteswest; /* Minutes west of Greenwich. */
    int tz_dsttime;     /* Type of dst correction. */
};

extern int        gettimeofday(struct timeval *tp, void *ignore);
extern struct tm *gmtime_r(const time_t *timep, struct tm *r);

#ifdef __cplusplus
}
#endif

#endif /* __SYS_TIME_H__ */

