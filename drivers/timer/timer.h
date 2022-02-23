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
 * @file        timer.h
 *
 * @brief       this file implements timer related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRIVER_TIMER_H__
#define __DRIVER_TIMER_H__

#include <os_task.h>
#include <device.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MSEC_PER_SEC    1000ul
#define USEC_PER_MSEC   1000ul
#define NSEC_PER_USEC   1000ul
#define NSEC_PER_MSEC   1000000ul
#define USEC_PER_SEC    1000000ul
#define NSEC_PER_SEC    1000000000ull
#define FSEC_PER_SEC    1000000000000000ull

typedef struct os_clockval
{
    os_uint32_t sec;  /* second */
    os_uint32_t nsec; /* microsecond */
} os_clockval_t;

void calc_mult_shift(os_uint32_t *mult, os_uint32_t *shift, os_uint32_t from, os_uint32_t to, os_uint32_t max_from);

static inline os_uint32_t time_escape_us(os_clockval_t *start, os_clockval_t *now)
{
    return (now->sec - start->sec) * USEC_PER_SEC + now->nsec / 1000 - start->nsec / 1000;
}

#ifdef __cplusplus
}
#endif

#endif  /* __DRIVER_TIMER_H__ */
