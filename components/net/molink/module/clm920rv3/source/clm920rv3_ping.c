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
 * @file        clm920rv3_ping.c
 *
 * @brief       clm920rv3 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "clm920rv3_ping.h"
#include <string.h>

#ifdef CLM920RV3_USING_PING_OPS

#define MO_LOG_TAG "clm920rv3.ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

os_err_t clm920rv3_ping(mo_object_t *module, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    /* CLM920RV3 ping function just for test use, not officially support for now. */
    
    at_parser_t *parser        = &module->parser;
    os_err_t     result        = OS_EOK;
    os_int32_t   response      = -1;
    os_uint16_t  recv_data_len = 0;
    os_uint32_t  ping_time     = 0;
    os_int16_t   ttl           = -1;

    memset(resp, 0, sizeof(ping_resp_t));

    char ip_addr[IPADDR_MAX_STR_LEN + 1] = {0};

    if (parser == OS_NULL)
    {
        ERROR("CLM920RV3 ping: at parser is NULL.");
        return OS_ERROR;
    }

    DEBUG("CLM920RV3 series module does not support setting ping package size&timeout.");
    DEBUG("CLM920RV3 ping: %s", host);

    char resp_buff[8 * AT_RESP_BUFF_SIZE_DEF] = {0};

    /* Need to wait for 7 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 2,
                         .timeout   = 4 * OS_TICK_PER_SECOND};

    /* REF: CLM920RV3 CPING */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+CPING=%s", host) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+CPING=%s", host);
        result = OS_ERROR;
        goto __exit;
    }

    /* Received the ping response from the server */
    at_resp_get_data_by_kw(&at_resp, "+CPING:", "+CPING:%d", &response);
    if (response == 1)
    {
        if (at_resp_get_data_by_kw(&at_resp,
                                   "+CPING:",
                                   "+CPING:%d,%[^,],%hu,%u,%hd",
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
    case 1:
        inet_aton(ip_addr, &(resp->ip_addr));

        resp->data_len = recv_data_len;
        resp->time     = ping_time;
        resp->ttl      = ttl;

        result = OS_EOK;
        break;

    default:
        result = OS_ERROR;
        break;
    }

__exit:

    if (OS_EOK != result) ERROR("CLM920RV3 ping failed.");

    return result;
}

#endif /* CLM920RV3_USING_PING_OPS */
