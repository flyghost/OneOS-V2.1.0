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
 * @file
 *
 * @brief
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "cms_config.h"
#include <time.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

CMS_PROTOCOL_TYPE_T cms_con_get_protocol_type(void)
{
#if defined(CMS_CONNECT_MQTT)
    return ENUM_CMS_MQTT;
#elif defined(CMS_CONNECT_COAP)
    return ENUM_COAP;
#elif defined(CMS_CONNECT_LWM2M)
    return ENUM_LWM2M;
#elif defined(CMS_CONNECT_UDP)
    return ENUM_UDP;
#elif defined(CMS_CONNECT_TCP)
    return ENUM_TCP;
#else
    return ENUM_NONE;
#endif
}

const char *cms_con_get_server_url(void)
{
    return CMS_SERVER_ADDRESS;
}

const char *cms_con_get_server_port(void)
{
    return CMS_SERVER_PORT;
}

int cms_con_get_send_buff_length(void)
{
    return CMS_SEND_BUFF;
}

int cms_con_get_recv_buff_length(void)
{
    return CMS_RECV_BUFF;
}

int cms_con_get_heartbeat_interval(void)
{
    return CMS_HEARTBEAT_INTERVAL;
}

int cms_con_get_heartbeat_enable(void)
{
#if defined(CMS_HEARTBEAT_ENABLE)
    return 1;
#else
    return 0;
#endif
}

int cms_con_get_log_level(void)
{
#if defined(CMS_LOG_DEBUG)
    return 7;
#elif defined(CMS_LOG_INFO)
    return 6;
#elif defined(CMS_LOG_WARNING)
    return 4;
#else
    return 3;
#endif
}
#if defined(CMS_USING_ID)
const char *cms_id_get_pid(void)
{
    return CMS_ID_SET_PID;
}

const char *cms_id_get_key(void)
{
    return CMS_ID_KEY;
}

int cms_id_set_did(uint8_t *did, size_t did_max_len, size_t *did_len)
{
    int status = 1;

    uint8_t buf[] = "1qlLagQHY12cCucp2555";
    *did_len      = sizeof(buf) - 1;
    if (did_max_len < *did_len)
    {
        return status;
    }
    else
    {
        memcpy(did, buf, *did_len);
        status = 0;
        return status;
    }
}

void cms_id_srand(void)
{
    srand((unsigned)time(NULL));
}

CMS_AUTH_MODE_T cms_id_get_auth_mode(void)
{
#if defined(MUTUAL_AUTH)
    return ENUM_MUTUAL_AUTH;
#elif defined(UNILATERALISM_AUTH)
    return ENUM_UNILATERALISM_AUTH;
#else
    return ENUM_NONE_AUTH;
#endif
}

#endif

#if defined(CMS_USING_VF_LINUX)

const char *get_config_file_name(void)
{
#if __amd64
    return "./cms_config";
#else
    return CMS_CONFIG_FILE_NAME;
#endif
}
const char *get_tmp_config_file_name(void)
{
#if __amd64
    return "./cms_config_bak";
#else
    return CMS_CONFIG_TMP_FILE_NAME;
#endif
}

#endif
