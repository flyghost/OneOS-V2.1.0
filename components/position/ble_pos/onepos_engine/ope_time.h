/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        ope_time.h
 * 
 * @brief       Time control
 * 
 * @details     The control system time information
 * 
 * @revision
 * Date         Author          Notes
 * 2021-04-29   HuSong          First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <time.h>
#include "ope_types.h"

/**
 ***********************************************************************************************************************
 * @struct      ope_time_tm_t
 *      
 * @brief       Time set
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_int32_t year;   /* Current year */
    ope_int32_t mon;    /* Current month */
    ope_int32_t day;    /* Current day */
    ope_int32_t hour;   /* Current hour */
    ope_int32_t min;    /* Current minute */
    ope_int32_t sec;    /* Current second */
}ope_time_tm_t;

extern ope_void_t ope_get_cur_time(ope_time_tm_t *time_info);