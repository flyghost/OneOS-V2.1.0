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
 * @file        rcvr_type.h
 *
 * @brief       gnss recevicer object type definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __RCVR_TYPE_H__
#define __RCVR_TYPE_H__

#include <oneos_config.h>
#ifdef GNSS_USING_RCVR

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum rcvr_ops_type
{
    RCVR_OPS_NULL   = 0,

#ifdef RCVR_SUPP_RESET
    RCVR_RESET_OPS,
#endif

#ifdef RCVR_SUPP_AGNSS
    RCVR_AGNSS_OPS,
#endif

    RCVR_OPS_MAX,
}rcvr_ops_type_t;

#ifdef RCVR_SUPP_RESET
typedef enum rcvr_reset_type{
    RCVR_MIN_START_TYPE = 0,
    RCVR_COLD_START,
    RCVR_WARM_START,
    RCVR_HOT_START,
    RCVR_MAX_START_TYPE,
}rcvr_reset_type_t;
#endif

#define IS_INVALID_RESET_TYPE(type) ((RCVR_MIN_START_TYPE < type) && (type < RCVR_MAX_START_TYPE))

#ifdef RCVR_SUPP_PROT
typedef enum rcvr_prot_type
{
    RCVR_PROT_NULL = -1,
#ifdef GNSS_NMEA_0183_PROT
    RCVR_NMEA_0183_PROT,
#endif

    RCVR_PROT_MAX,
}rcvr_prot_type_t;

#define IS_INVALID_PROT_TYPE(type)   ((RCVR_PROT_NULL < type) && (type < RCVR_PROT_MAX))
#define RCVR_REC_PROT_DATA_PERIOD    (1 * OS_TICK_PER_SECOND)
#endif

#ifdef GNSS_NMEA_0183_PROT
#include "nmea_0183.h"
#define RCVR_BUFF_LEN NMEA_MAX_BUFF_LEN
#endif

#define RCVR_RC_TIMEOUT                 2 * OS_TICK_PER_SECOND
#define RCVR_PROT_RC_TEMP_BUFF_LEN      1024

#define RCVR_PROT_PARSE_TASK_STACK_SIZE 2048
#define RCVR_PROT_PARSE_TASK_PRIORITY   (25)

#define AGNSS_MSG_PUB_TOPIC_SUFF     "/agnss/data"
#define AGNSS_DATA_SUB_TOPIC_SUFF    "/agnss/data_reply"

#define AGNSS_PUB_MSG_FORMAT        "{\"class\":\"%3s, %3s\",\"pos\":\"%011.5f, %012.5f, %07.2f\",\"bak\":{}}"
#define AGNSS_PUB_MSG_LEN           (127)
#define AGNSS_INFO_MSG_FORMAT 		"{\"index\":%d}"	

#define AGNSS_WAITE_MSG_TIMEOUT     (10)  // unit: sec

#define AGNSS_DATA_MAX_NUM  (99)
#define AGNSS_DATA_MIN_NUM  (10)

#define AGNSS_IS_VALID_TOTAL(total)             (AGNSS_DATA_MIN_NUM < total && total < AGNSS_DATA_MAX_NUM)
#define AGNSS_IS_VALID_INDEX(index, num, total) (index > 0 && index <= total && index == num +1)

#define AGNSS_DATA_PRE_LEN           4
#define AGNSS_INDEX_AND_DATA_SEP    '@'

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GNSS_USING_RCVR */
#endif /* __RCVR_TYPE_H__ */
