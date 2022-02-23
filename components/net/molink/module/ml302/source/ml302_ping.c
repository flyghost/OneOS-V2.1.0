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
 * @file        ml302_ping.c
 *
 * @brief       ml302 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ml302_ping.h"

#define MO_LOG_TAG "ml302.ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define ML302_MIN_PING_PKG_LEN (36)
#define ML302_MAX_PING_PKG_LEN (1500)
#define ML302_MIN_PING_TIMEOUT (1)
#define ML302_MAX_PING_TIMEOUT (255)

#ifdef ML302_USING_PING_OPS

os_err_t ml302_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser    = &self->parser;
    os_err_t     result    = OS_EOK;
    os_uint32_t  req_time  = 0;
    os_uint16_t  rcv_len   = 0;
    os_int16_t   ttl       = -1;
    os_uint32_t  timeout_s = timeout / 1000; /* Milliseconds convert to seconds */

    char ip_addr[IPADDR_MAX_STR_LEN + 1] = {0};
    char ret_buff[36]                    = {0};

    if (parser == OS_NULL)
    {
        ERROR("ML302 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < ML302_MIN_PING_PKG_LEN) || (len > ML302_MAX_PING_PKG_LEN))
    {
        ERROR("ML302 ping: ping package len[%d] is out of range[%d, %d].", 
                  len, ML302_MIN_PING_PKG_LEN, ML302_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout_s < ML302_MIN_PING_TIMEOUT) || (timeout_s > ML302_MAX_PING_TIMEOUT))
    {
        ERROR("ML302 ping: ping timeout %us is out of range[%ds, %ds].", 
                  timeout_s, ML302_MIN_PING_TIMEOUT, ML302_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    DEBUG("ML302 ping: %s, len: %d, timeout: %us", host, len, timeout_s);

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 3,
                         .timeout   = (5 + timeout_s) * OS_TICK_PER_SECOND
                        };

    /* Send commond "AT+MPING=<domain name> or ip addr" and wait response */
    /* Exec AT+MPING="www.baidu.com",2,1,64 and return: 0 183.232.231.172: bytes = 36 time = 96(ms), TTL = 255 */

    if (at_parser_exec_cmd(parser, &at_resp, "AT+MPING=\"%s\",%u,1,%d", host, timeout_s, len) < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp,
                               "TTL",
                               "0 %[^:]: bytes = %hu time = %u(ms), TTL = %hd",
                               ip_addr,
                               &rcv_len,
                               &req_time,
                               &ttl) <= 0)
    {
        if (at_resp_get_data_by_kw(&at_resp, "+", "+%s", ret_buff) <= 0)
        {
            ERROR("AT+NPING resp parse \"+NPINGERR\" fail.");
        }

        ERROR("ML302 ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("ML302 ping: resp parse ip[%s], req_time[%u], ttl[%d]", ip_addr, req_time, ttl);
        if (ttl <= 0)
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
        inet_aton(ip_addr, &(resp->ip_addr));
        resp->data_len = rcv_len;
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

__exit:

    return result;
}

#endif /* ML302_USING_PING_OPS */
