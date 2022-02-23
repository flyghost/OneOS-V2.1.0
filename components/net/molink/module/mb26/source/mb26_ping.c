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
 * @file        mb26_ping.c
 *
 * @brief       mb26 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mb26_ping.h"

#define MO_LOG_TAG "mb26.ping"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#define MB26_MIN_PING_PKG_LEN (1)
#define MB26_MAX_PING_PKG_LEN (1500)
#define MB26_MIN_PING_TIMEOUT (1)
#define MB26_MAX_PING_TIMEOUT (600000)

#ifdef MB26_USING_PING_OPS

os_err_t mb26_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_uint32_t  req_time        = 0;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    if (parser == OS_NULL)
    {
        ERROR("MB26 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < MB26_MIN_PING_PKG_LEN) || (len > MB26_MAX_PING_PKG_LEN))
    {
        ERROR("MB26 ping: ping package len[%u] is out of range[%u, %u].",
                  len, MB26_MIN_PING_PKG_LEN, MB26_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout < MB26_MIN_PING_TIMEOUT) || (timeout > MB26_MAX_PING_TIMEOUT))
    {
        ERROR("MB26 ping: user set ping timeout value %ums is out of range[%ums, %ums].",
                  timeout, MB26_MIN_PING_TIMEOUT, MB26_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    DEBUG("MB26 ping: %s, len: %u, timeout: %ums", host, len, timeout);

    char resp_buff[256] = {0};
    /* Need to wait for 4 lines response msg, and timeout comming with ms */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 6,
                         .timeout   = (5 + timeout / 1000) * OS_TICK_PER_SECOND};

    /* Default set timeout to 5000ms */
    /* Exec commond "AT+ECPING="180.101.147.115",1,64,10000 and wait response */
    /* Return: success: +ECPING: SUCC, dest: 180.101.147.115, RTT: 3340 ms */
    /* Return: fail:    +ECPING: FAIL, dest: 180.101.147.115, RTT: 10000 ms */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+ECPING=\"%s\",1,%u,%u", host, len, timeout) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+ECPING=\"%s\",1,%u,%u", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+ECPING: SUCC", "+ECPING: SUCC, dest: %[^,], RTT: %u", ipaddr, &req_time) <= 0)
    {
        ERROR("MB26 ping %s fail: check network status and try to set a longer timeout.", host);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("MB26 ping: resp parse ip[%s], req_time[%u]", ipaddr, req_time);
        if (req_time >= timeout)
        {
            result = OS_ETIMEOUT;
        }
        else
        {
            result = OS_EOK;
        }
    }

    if (0 != req_time)
    {
        inet_aton(ipaddr, &(resp->ip_addr));
        resp->data_len = len;
        resp->time     = req_time;
    }

__exit:

    return result;
}

#endif /* MB26_USING_PING_OPS */
