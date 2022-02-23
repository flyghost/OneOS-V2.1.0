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
 * @file        pos_rtcm_msg_parse.h
 *
 * @brief       rtcm msg parse function
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
#ifndef __POS_RTCM_MSG_PARSE_H__
#define __POS_RTCM_MSG_PARSE_H__

#include <os_types.h>

#ifdef __cplusplus
     extern "C" {
#endif

#include "pos_rtcm_msg.h"

extern os_int32_t onepos_parse_rtcm_msg_type(char *buff, os_uint32_t buf_len);
extern os_int32_t decode_msm5(os_uint8_t *buff, os_uint32_t buf_len, os_int32_t sys, rtcm_msg_msm5_t *result);
extern os_int32_t decode_type1005(os_uint8_t *buff, os_uint32_t buf_len, rtcm_msg_1005_t *result);
extern os_int32_t decode_type1033(os_uint8_t *buff, os_uint32_t buf_len, rtcm_msg_1033_t *result);

#endif

