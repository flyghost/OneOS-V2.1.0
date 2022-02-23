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
 * @file        cmiot_client.h
 *
 * @brief       The client header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_CLIENT_H__
#define __CMIOT_CLIENT_H__

#include "cmiot_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CMIOT_DOWNLOAD_MD5_LEN      16
#define CMIOT_DOWNLOAD_HOST_MAX_LEN 32
#define CMIOT_DOWNLOAD_URI_MAX_LEN  80

typedef struct
{
    cmiot_char   download_host[CMIOT_DOWNLOAD_HOST_MAX_LEN + 1];
    cmiot_char   download_url[CMIOT_DOWNLOAD_URI_MAX_LEN + 1];
    cmiot_char   download_md5[CMIOT_DOWNLOAD_MD5_LEN + 1];
    cmiot_uint32 download_delta_id;
    cmiot_uint32 download_delta_size;
    cmiot_uint32 download_port;
} CMIOT_ALIGN(1) cmiot_download_uri_info_t;

#define DECLARE_VOID_FUNC(PREFIX, SUFFIX) cmiot_int8 PREFIX##_##SUFFIX(void);

cmiot_uint8                cmiot_get_current_state(void);
cmiot_uint8                cmiot_get_conn_try_count(void);
cmiot_char *               cmiot_get_send_buf(void);
cmiot_uint16               cmiot_get_send_buf_len(void);
void                       cmiot_reset_send_buf(void);
cmiot_uint16               cmiot_get_recieve_buf_len(void);
cmiot_download_uri_info_t *cmiot_get_download_uri_info(void);
cmiot_uint32               cmiot_get_download_index(void);
cmiot_uint32               cmiot_get_download_index_max(void);
cmiot_bool                 cmiot_get_domain(cmiot_char *server, cmiot_char *domain, cmiot_uint32 *port);

DECLARE_VOID_FUNC(cmiot_upgrade, coap)
DECLARE_VOID_FUNC(cmiot_upgrade, http)
DECLARE_VOID_FUNC(cmiot_report_upgrade, coap)
DECLARE_VOID_FUNC(cmiot_report_upgrade, http)

#ifdef __cplusplus
}
#endif

#endif
