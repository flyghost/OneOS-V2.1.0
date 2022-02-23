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
 * @file        mo_time.h
 *
 * @brief       time's inner function definitions
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-09   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_TIME_H__
#define __MO_TIME_H__

#include <os_types.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 ***********************************************************************************************************************
 * @struct      _mo_tm_t
 *
 * @brief       molink inner struct def
 ***********************************************************************************************************************
 */
typedef struct _mo_tm
{
    os_int32_t tm_sec;   /* Seconds.   [0-60] (1 leap second) */
    os_int32_t tm_min;   /* Minutes.   [0-59]   */
    os_int32_t tm_hour;  /* Hours.     [0-23]   */
    os_int32_t tm_mday;  /* Day.       [1-31]   */
    os_int32_t tm_mon;   /* Month.     [1-12]   */
    os_int32_t tm_year;  /* Years.     eg.2021  */
    os_int32_t tm_isdst; /* DST.       [-1/0/1] */
    os_int32_t tm_q_off; /* East quater of UTC. */
} _mo_tm_t;

void time_struct_convert(_mo_tm_t *mo_tm, struct tm *_tm);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MO_TIME_H__ */
