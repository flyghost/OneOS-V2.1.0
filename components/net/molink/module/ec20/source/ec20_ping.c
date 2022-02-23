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
 * @file        ec20_ping.c
 *
 * @brief       ec20 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ec20_ping.h"
#include <string.h>

#define MO_LOG_TAG "ec20_ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define EC20_MIN_PING_TIME   (1)
#define EC20_MAX_PING_TIME   (255)

#ifdef EC20_USING_PING_OPS

os_err_t ec20_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser        = &self->parser;
    os_err_t     result        = OS_EOK;
    os_int32_t   response      = -1;
    os_uint16_t  recv_data_len = 0;
    os_uint32_t  ping_time     = 0;
    os_int16_t   ttl           = -1;
    os_uint32_t  timeout_s     = timeout / 1000;

    memset(resp, 0, sizeof(ping_resp_t));

    char ip_addr[IPADDR_MAX_STR_LEN + 1] = {0};

    if (parser == OS_NULL)
    {
       ERROR("EC20 ping: at parser is NULL.");
        return OS_ERROR;
    }

   DEBUG("EC20 series module does not support setting ping package size.");

    /* ec20 ping timeout_s range: 1s-255s */
    if ((timeout_s < EC20_MIN_PING_TIME) || (timeout_s > EC20_MAX_PING_TIME))
    {
       ERROR("EC20 ping: ping timeout_s %us is out of range[%ds, %ds].",
                  timeout_s, EC20_MIN_PING_TIME, EC20_MAX_PING_TIME);
        return OS_ERROR;
    }

   DEBUG("EC20 ping: %s, timeout_s: %us", host, timeout_s);

    char resp_buff[256] = {0};

    /* Need to wait for 4 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 3,
                         .timeout   = (5 + timeout_s) * OS_TICK_PER_SECOND};

    /* REF: EC20 QPING */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+QPING=1,\"%s\",%u,1", host, timeout_s) < 0)
    {
       ERROR("Ping: AT cmd exec fail: AT+QPING=1,\"%s\",%u,1", host, timeout_s);
        result = OS_ERROR;
        goto __exit;
    }

    /* Received the ping response from the server */
    at_resp_get_data_by_kw(&at_resp, "+QPING:", "+QPING:%d", &response);
    if (response == 0)
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
    case 0:
        inet_aton(ip_addr, &(resp->ip_addr));

        resp->data_len = recv_data_len;
        resp->time     = ping_time;
        resp->ttl      = ttl;

        result = OS_EOK;
        break;

    case 569:
        result = OS_ETIMEOUT;
        break;

    default:
        result = OS_ERROR;
        break;
    }

__exit:

    return result;
}

#endif /* EC20_USING_PING_OPS */
