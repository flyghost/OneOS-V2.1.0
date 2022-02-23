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
 * @file        unicorecomm_rcvr.c
 *
 * @brief       zkw gnss recevicer function
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "rcvr_object.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ONEPOS_LOG_TAG "onepos.unicorecomm"
#define ONEPOS_LOG_LVL ONEPOS_LOG_INFO
#include <onepos_log.h>

#ifdef GNSS_USING_UNICORECOMM

#ifdef RCVR_SUPP_AGNSS

os_err_t unicorecomm_rcvr_agnss(rcvr_object_t* rcvr, ...)
{
    OS_ASSERT(OS_NULL != rcvr);

    ONEPOS_LOG_E("not support agnss for %s at this version.", rcvr->name);

    return OS_ERROR;
}
#endif

#ifdef RCVR_SUPP_RESET
os_err_t unicorecomm_rcvr_reset(rcvr_object_t* rcvr, ...)
{
    va_list             var_arg;
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
            result = rcvr_object_send(rcvr, "$RESET,0,hFF\r\n", 14);    
        break;

        case RCVR_WARM_START:
            result = rcvr_object_send(rcvr, "$RESET,0,h01\r\n", 14);
        break;

        case RCVR_HOT_START:
            result = rcvr_object_send(rcvr, "$RESET,0,h0\r\n", 13);
        break;

        default:
            ONEPOS_LOG_W("receiver reset error.");
        break;
    }

    return result;
}
#endif

rcvr_object_t *unicorecomm_rcvr_creat(void)
{
    rcvr_object_t* rcvr = (rcvr_object_t*)os_malloc(sizeof(rcvr_object_t));
	
    if(OS_NULL == rcvr)
    {
        ONEPOS_LOG_E("creat unicorecomm receiver is ERROR, no enough memory.");
        return (rcvr_object_t*)OS_NULL;
    }

    if(OS_EOK != rcvr_object_init(rcvr, "unicorecomm", UNICORECOMM_DEV_NAME))
    {
        rcvr_object_deinit(rcvr);
        ONEPOS_LOG_E("init unicorecomm receiver is ERROR.");
        return (rcvr_object_t*)OS_NULL;
    }

    #ifdef RCVR_SUPP_RESET
    rcvr->ops_table[RCVR_RESET_OPS] = (rcvr_ops_pfunc)unicorecomm_rcvr_reset;
    #endif

    #ifdef RCVR_SUPP_AGNSS
    rcvr->ops_table[RCVR_AGNSS_OPS] = (rcvr_ops_pfunc)unicorecomm_rcvr_agnss;
    #endif

    return rcvr;
}

#endif
