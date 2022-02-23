/**
 ***********************************************************************************************************************
 * Copyright (c) China Mobile Communications Group Co.,Ltd.
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
 * @file        onepos_common.h
 *
 * @brief
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-12  OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __POS_COMMON_H__
#define __POS_COMMON_H__

#include <math.h>
#include <os_types.h>

/* determine cache size */
#define CACHE_MEM_NUM                   20


/* Similar as the NULL in C library */
#ifdef __cplusplus
#define OS_NULL                         0
#else
#define OS_NULL                         ((void *)0)
#endif

/* Boolean value definitions */
#define OS_FALSE                        0
#define OS_TRUE                         1

#define OS_WAIT_FOREVER       OS_TICK_MAX

#define SEM_VALUE_TO_MUTEX              1

#define POS_ESEM                      -11          /* error: error in semaphore section                              */
#define POS_EINIT                     -12          /* error: the initialization state does not meet the requirements */
#define POS_EFREE                     -13          /* error: source not released                                     */
#define POS_ESIZE                     -14          /* error: wrong size                                              */
#define POS_ENULL                     -15          /* error: invaild pointer addr                                    */
#define POS_EMALLOC                   -16          /* error: wrong due to os_malloc()                                */
#define POS_ESTATE                    -17          /* error: wrong state to proceed to the next step                 */
#define POS_WEMPTY                    -18          /* warring: no resources to acquire                               */

typedef struct
{
    os_int64_t  value;
    os_uint32_t dec_len;
} onepos_com_float_t;

#define tras_loca_float(data) ((double)(data.value / pow(10, data.dec_len)))

/**
 ***********************************************************************************************************************
 * @struct      onepos_date_t
 *
 * @brief       onepos date
 ***********************************************************************************************************************
 */
typedef struct
{
    os_uint32_t day;
    os_uint32_t month;
    os_uint32_t year;
} onepos_date_t;

/**
 ***********************************************************************************************************************
 * @struct      onepos_time_t
 *
 * @brief       onepos time
 ***********************************************************************************************************************
 */
typedef struct
{
    os_uint32_t hours;
    os_uint32_t minutes;
    os_uint32_t seconds;
    os_uint32_t microseconds;
} onepos_time_t;

/**
 ***********************************************************************************************************************
 * @struct      onepos_pos_t
 *
 * @brief       onepos position result(WGS-84)
 ***********************************************************************************************************************
 */
typedef struct
{
    os_uint32_t time;           /* Information acquisition time (UNIX time stamp : s)*/
    double      lat_coordinate; /* Latitude coordinate */
    double      lon_coordinate; /* Longitude coordinate */
} onepos_pos_t;

#define _onepos_isdigit(argv) ((((*argv) - '0') < 10u) && (((*argv) - '0') >= 0u))

#define 	TEMP_DEBUG_BUFF_LEN	(64)
#define onepos_msg_dbg_show(name, pdata, len)                           \
do{                                                                     \
    char        temp[TEMP_DEBUG_BUFF_LEN + 1]  = {0,};                  \
    os_uint32_t cnt        = 0;                                         \
	os_kprintf("%s :\n", name);                                         \
    while(cnt < len)                                            		\
    {                                                                   \
        memset(temp, 0, sizeof(temp));                                  \
        if(len - cnt > TEMP_DEBUG_BUFF_LEN)                     		\
        {                                                               \
            memcpy(temp, ((char*)pdata + cnt), TEMP_DEBUG_BUFF_LEN);  	\
            temp[TEMP_DEBUG_BUFF_LEN] = '\0';                           \
            cnt += TEMP_DEBUG_BUFF_LEN;                                 \
            os_kprintf("%s", temp);                                     \
        }                                                               \
        else                                                            \
        {                                                               \
            memcpy(temp, ((char*)pdata + cnt), len - cnt);              \
            temp[len - cnt + 1] = '\0';                                 \
            cnt = len;                                                  \
            os_kprintf("%s\r\n", temp);                                 \
        }                                                               \
    }                                                                   \
}while(0)                                                                   
/**
 ***********************************************************************************************************************
 * @def         ONEPOS_MIN_INTERVAL
 *
 * @brief       onepos support the minimum interval
 ***********************************************************************************************************************
 */
#define ONEPOS_MIN_INTERVAL 3    // s
#define ONEPOS_MAX_INTERVAL OS_UINT16_MAX
#define GNSS_BUFFER_LEN_MAX 3072

#define ONEPOS_MSG_SEPARATOR     "|"
#define ONEPOS_MSG_SEPARATOR_LEN  1

#define ONEPOS_LAT_LON_STR_LEN          30        // bytes

#define ONEPOS_DEV_WAITE_READY_TIME     1000      // ms
#define ONEPOS_WAIT_DEVICE_INTI_READY   1000      // ms

#define ONEPOS_COMM_TEMP_LEN            512

#define ONEPOS_MIN_LAT                  (0.0f - 90.0f)
#define ONEPOS_MAX_LAT                  (90.0f)
#define ONEPOS_MIN_LON                  (0.0f - 180.0f)
#define ONEPOS_MAX_LON                  (180.0f)
#define ONEPOS_DEFAULT_LAT              (30.0f)
#define ONEPOS_DEFAULT_LON              (104.0f)
#define IS_VAILD_LAT(lat) (lat >= ONEPOS_MIN_LAT && lat <= ONEPOS_MAX_LAT)
#define IS_VAILD_LON(lon) (lon >= ONEPOS_MIN_LON && lon <= ONEPOS_MAX_LON)
#endif /* __POS_COMMON_H__ */
