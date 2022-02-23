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
 * @file        cms_config.h
 *
 * @brief       define cms confing.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-09   OneOS Team      First version.
 ***********************************************************************************************************************/
#ifndef __CMS_CON_USER_CONFIG_H__
#define __CMS_CON_USER_CONFIG_H__
#include "oneos_config.h"
#include <stddef.h>
#include <stdint.h>
#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    ENUM_NONE,
    ENUM_CMS_MQTT,
    ENUM_COAP,
    ENUM_LWM2M,
    ENUM_UDP,
    ENUM_TCP
} CMS_PROTOCOL_TYPE_T;

typedef enum
{
    ENUM_NONE_AUTH,
    ENUM_MUTUAL_AUTH,
    ENUM_UNILATERALISM_AUTH
} CMS_AUTH_MODE_T;

CMS_PROTOCOL_TYPE_T cms_con_get_protocol_type(void);

const char *cms_con_get_server_url(void);

const char *cms_con_get_server_port(void);

int cms_con_get_send_buff_length(void);

int cms_con_get_recv_buff_length(void);

int cms_con_get_heartbeat_enable(void);

int cms_con_get_heartbeat_interval(void);

int cms_con_get_log_level(void);

const char *cms_id_get_pid(void);

const char *cms_id_get_connet(void);

const char *cms_id_get_key(void);

void cms_id_srand(void);

int cms_id_set_did(uint8_t *did_t,size_t did_max_len, size_t *did_len_t);

CMS_AUTH_MODE_T cms_id_get_auth_mode(void);

#if defined(CMS_USING_VF_LINUX)

const char *get_config_file_name(void);

const char *get_tmp_config_file_name(void);

#endif

#if defined(__cplusplus)
extern "C"
}
#endif

#endif
