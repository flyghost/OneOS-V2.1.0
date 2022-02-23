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
 * @file        sim7020_ping.c
 *
 * @brief       sim7020 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "sim7020_ping.h"

#define MO_LOG_TAG "sim7020.ping"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define SIM7020_MIN_PING_PKG_LEN (0)
#define SIM7020_MAX_PING_PKG_LEN (5120)
#define SIM7020_MIN_PING_TIMEOUT (1)
#define SIM7020_MAX_PING_TIMEOUT (600)

#ifdef SIM7020_USING_PING_OPS

os_err_t sim7020_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser   = &self->parser;
    os_err_t     result   = OS_EOK;
    os_int16_t   req_time = -1;
    os_int16_t   replyid  = -1;
    os_int16_t   ttl      = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    if (parser == OS_NULL)
    {
        ERROR("SIM7020 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if (len > SIM7020_MAX_PING_PKG_LEN)
    {
        ERROR("SIM7020 ping: ping package len[%d] is out of range[%d, %d].",
                  len,
                  SIM7020_MIN_PING_PKG_LEN,
                  SIM7020_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    /* Def timeout value is 5000ms, SIM7020C in units of 100 ms */
    timeout /= 100;
    if ((timeout < SIM7020_MIN_PING_TIMEOUT) || (timeout > SIM7020_MAX_PING_TIMEOUT))
    {
        ERROR("SIM7020 ping: user set ping timeout value %ums is out of range[%dms, %dms].",
                  timeout * 100,
                  SIM7020_MIN_PING_TIMEOUT * 100,
                  SIM7020_MAX_PING_TIMEOUT * 100);
        return OS_ERROR;
    }

    DEBUG("SIM7020 ping: %s, len: %d, timeout: %ums", host, len, timeout * 100);

    char resp_buff[256] = {0};
    /* Need to wait for 2 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 2,
                         .timeout   = (5 + timeout / 10) * OS_TICK_PER_SECOND};

    /* Exec AT+CIPPING="www.baidu.com",4,64,50; def ping timeout: +CIPPING: 1,39.156.66.14,50,0 */
    /* Success: +CIPPING: 1,39.156.66.14,84,52 */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+CIPPING=\"%s\",1,%d,%d", host, len, timeout) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+CIPPING=\"%s\",1,%d,%d", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+CIPPING", "+CIPPING: %d,%[^,],%d,%d", &replyid, ipaddr, &req_time, &ttl) <= 0)
    {
        ERROR("%s ping %s fail: check network status and try to set a longer timeout.", self->name, host);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("SIM7020 ping: resp parse ip[%s], req_time[%d], ttl[%d]", ipaddr, req_time, ttl);
        if (ttl <= 0)
        {
            result = OS_ETIMEOUT;
        }
        else
        {
            result = OS_EOK;
        }
    }

    if (req_time)
    {
        inet_aton(ipaddr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = ttl;
        resp->time     = (req_time * 100);
    }

__exit:

    return result;
}

#endif /* SIM7020_USING_PING_OPS */
