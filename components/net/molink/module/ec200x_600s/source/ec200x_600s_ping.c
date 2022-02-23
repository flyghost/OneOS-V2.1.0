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
 * @file        ec200x_600s_ping.c
 *
 * @brief       ec200x_600s module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec200x_600s_ping.h"

#define MO_LOG_TAG "ec200x_600s.ping"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define EC200X_600S_MIN_PING_TIME   (1)
#define EC200X_600S_MAX_PING_TIME   (255)
#define EC200X_600S_PING_OK         (0)
#define EC200X_600S_PING_TIMEOUT    (569)

#ifdef EC200X_600S_USING_PING_OPS

os_err_t ec200x_600s_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser        = &self->parser;
    os_err_t     result        = OS_EOK;
    os_int32_t   response      = -1;
    os_uint16_t  recv_data_len = 0;
    os_uint32_t  ping_time     = 0;
    os_int16_t   ttl           = -1;
    os_uint32_t  timeout_s     = timeout / 1000; /* Milliseconds convert to seconds */

    char ip_addr[IPADDR_MAX_STR_LEN + 1] = {0};

    DEBUG("EC200X_600S series module does not support setting ping package size.");

    /* EC200X_600S ping timeout range: 1s-255s */
    if ((timeout_s < EC200X_600S_MIN_PING_TIME) || (timeout_s > EC200X_600S_MAX_PING_TIME))
    {
        ERROR("EC200X_600S ping: ping timeout %us is out of range[%ds, %ds].",
              timeout_s,
              EC200X_600S_MIN_PING_TIME,
              EC200X_600S_MAX_PING_TIME);
        return OS_ERROR;
    }

    DEBUG("EC200X_600S ping: %s, timeout: %u s", host, timeout_s);

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    /* Need to wait for 3 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 3,
                         .timeout   = (5 + timeout_s) * OS_TICK_PER_SECOND};

    /* REF: EC200S_600S QPING */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+QPING=1,%s,%u,1", host, timeout_s) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+QPING=1,%s,%u,1", host, timeout_s);
        result = OS_ERROR;
        goto __exit;
    }

    /* Received the ping response from the server */
    at_resp_get_data_by_kw(&at_resp, "+QPING:", "+QPING:%d", &response);
    if (response == EC200X_600S_PING_OK)
    {
        if (at_resp_get_data_by_kw(&at_resp,
                                   "+QPING:",
                                   "+QPING:%d,\"%[^\"]\",%hu,%u,%hd",
                                   &response,
                                   ip_addr,
                                   &recv_data_len,
                                   &ping_time,
                                   &ttl) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }
    }

    /* prase response number */
    switch (response)
    {
    case EC200X_600S_PING_OK:
        inet_aton(ip_addr, &(resp->ip_addr));

        resp->data_len = recv_data_len;
        resp->time     = ping_time;
        resp->ttl      = ttl;
        result         = OS_EOK;
        break;

    case EC200X_600S_PING_TIMEOUT:
        result = OS_ETIMEOUT;
        break;

    default:
        result = OS_ERROR;
        break;
    }

__exit:

    return result;
}

#endif /* EC200X_600S_USING_PING_OPS */
