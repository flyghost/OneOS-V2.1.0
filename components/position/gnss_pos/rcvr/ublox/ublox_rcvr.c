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
 * @file        ublox.c
 *
 * @brief       ublox gnss recevicer function
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <string.h>

#include "rcvr_object.h"

#define ONEPOS_LOG_TAG "onepos.ublox_rcvr"
#define ONEPOS_LOG_LVL ONEPOS_LOG_INFO
#include <onepos_log.h>

#ifdef GNSS_USING_UBLOX

#ifdef RCVR_SUPP_AGNSS
#define RCVR_CLASS  "ub"
#define RCVR_TYPE   "f9p"

os_err_t ublox_rcvr_agnss(rcvr_object_t* rcvr, ...)
{
    OS_ASSERT(OS_NULL != rcvr);

    return rcvr_agnss_func(rcvr, RCVR_CLASS, RCVR_TYPE, ONEPOS_DEFAULT_LAT, ONEPOS_DEFAULT_LON, ONEPOS_DEFAULT_LAT);
}
#endif

#ifdef RCVR_SUPP_RESET
os_err_t ublox_rcvr_reset(rcvr_object_t* rcvr, ...)
{
    va_list             var_arg;
    const char          hotstart_cmd[]        = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x10, 0x68};
    const char          warmstart_cmd[]       = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x01, 0x00, 0x02, 0x00, 0x11, 0x6C};
    const char          coldstart_cmd[]       = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0xFF, 0xB9, 0x02, 0x00, 0xC8, 0x8F};
    os_int32_t			tmp	 				  = 0;
    rcvr_reset_type_t   start_type            = RCVR_MIN_START_TYPE;
    os_err_t            result                = OS_ERROR;

    OS_ASSERT(OS_NULL != rcvr);
	
	va_start(var_arg, rcvr);
    tmp = va_arg(var_arg, os_int32_t);
    va_end(var_arg);	
	
	start_type = (rcvr_reset_type_t)tmp;

    switch(start_type)
    {
        case RCVR_COLD_START:
            result = rcvr_object_send(rcvr, coldstart_cmd, sizeof(coldstart_cmd));    
        break;

        case RCVR_WARM_START:
            result = rcvr_object_send(rcvr, warmstart_cmd, sizeof(warmstart_cmd));
        break;

        case RCVR_HOT_START:
            result = rcvr_object_send(rcvr, hotstart_cmd, sizeof(hotstart_cmd));
        break;

        default:
            ONEPOS_LOG_W("receiver reset error.");
        break;
    }

    return result;
}
#endif

rcvr_object_t *ublox_rcvr_creat(void)
{
    rcvr_object_t* rcvr = (rcvr_object_t*)os_malloc(sizeof(rcvr_object_t));

    if(OS_NULL == rcvr)
    {
        ONEPOS_LOG_E("creat ublox receiver is ERROR, no enough memory.");
        return (rcvr_object_t*)OS_NULL;
    }
    
    if(OS_EOK != rcvr_object_init(rcvr, "ublox", UBLOX_DEV_NAME))
    {
        rcvr_object_deinit(rcvr);
		rcvr_object_destroy(rcvr);
        ONEPOS_LOG_E("init ublox receiver is ERROR.");
        return (rcvr_object_t*)OS_NULL;
    }

    #ifdef RCVR_SUPP_RESET
    rcvr->ops_table[RCVR_RESET_OPS] = (rcvr_ops_pfunc)ublox_rcvr_reset;
    #endif

    #ifdef RCVR_SUPP_AGNSS
    rcvr->ops_table[RCVR_AGNSS_OPS] = (rcvr_ops_pfunc)ublox_rcvr_agnss;
    #endif

    return rcvr;
}

#endif /* ONEPOS_USING_UBLOX_GNSS_RCVR */


