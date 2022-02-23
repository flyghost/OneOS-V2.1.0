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
 * @file        n21_ping.c
 *
 * @brief       n21 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "n21_ping.h"

#include <string.h>

#define MO_LOG_TAG "n21_ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"


#define N21_MIN_PING_PKG_LEN (36)
#define N21_MAX_PING_PKG_LEN (1500)
#define N21_MIN_PING_TIMEOUT (0)
#define N21_MAX_PING_TIMEOUT (255)

#ifdef N21_USING_PING_OPS

os_err_t n21_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    if (len > N21_MAX_PING_PKG_LEN)
    {
        ERROR("N21 ping: ping package len[%d] is out of range[%d, %d].", 
                  len, N21_MIN_PING_PKG_LEN, N21_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    os_uint32_t timeout_s = timeout / 1000; /* Milliseconds convert to seconds */

    if (timeout_s > N21_MAX_PING_TIMEOUT)
    {
        ERROR("N21 ping: ping timeout %us is out of range[%ds, %ds].", 
                  timeout_s, N21_MIN_PING_TIMEOUT, N21_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    DEBUG("N21 ping: %s, len: %d, timeout: %us", host, len, timeout);

    at_parser_t *parser = &self->parser;

    char resp_buff[256] = {0};

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 4,
                         .timeout   = os_tick_from_ms(1000 + timeout),
                        };

    at_parser_exec_lock(parser);

    /* AT+PING=<ip>[,<timeout>,<size>,<num>] */
    os_err_t result = at_parser_exec_cmd(parser, &at_resp, "AT+PING=%s,%d,%d,1", host, timeout_s, len);
    if (result != OS_EOK)
    {
        return result;
    }

    char ip_addr[IPADDR_MAX_STR_LEN + 1] = {0};
    os_int16_t   ttl       = -1;
    os_uint32_t req_time = 0;

    /* Reply from 58.60.184.213: bytes= 64 time = 873(ms), TTL = 255 */
    if (at_resp_get_data_by_kw(&at_resp,
                               "Reply",
                               "Reply from %[^:]: bytes= %*d time = %d(ms), TTL = %*d",
                               ip_addr,
                               &req_time) <= 0)
    {
        ERROR("N21 ping %s fail, check network status and try to set a longer timeout.", host);
        result = OS_ERROR;

        at_parser_exec_unlock(parser);
        return result;
    }
    else
    {
        DEBUG("N21 ping: resp parse ip[%s], req_time[%u], ttl[%d]", ip_addr, req_time, ttl);
        if (req_time <= 0)
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
        resp->data_len = len;
        resp->ttl      = 255;
        resp->time     = req_time;
    }

    at_parser_exec_unlock(parser);
    return result;
}

#endif /* N21_USING_PING_OPS */
