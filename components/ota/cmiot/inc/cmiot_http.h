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
 * @file        cmiot_http.h
 *
 * @brief       The http header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_HTTP_H__
#define __CMIOT_HTTP_H__

#include "cmiot_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    cmiot_char   type[32];
    cmiot_uint16 content_len;
    cmiot_char * content;
} CMIOT_ALIGN(1) cmiot_http_parameter_t;

cmiot_char * cmiot_make_http_data(cmiot_uint8 state);
cmiot_bool   cmiot_parse_http_data(cmiot_char *data, cmiot_http_parameter_t *http_parameter, cmiot_uint16 count);
cmiot_char * cmiot_is_http_data(cmiot_char *data, cmiot_uint16 len);
cmiot_uint32 cmiot_http_callback(cmiot_uint8 state, cmiot_char *data);
cmiot_char * cmiot_get_http_server_host(void);

#ifdef __cplusplus
}
#endif

#endif
