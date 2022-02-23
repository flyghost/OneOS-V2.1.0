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
 * @file        l610_ping.c
 *
 * @brief       l610 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "l610_ping.h"

#define MO_LOG_TAG "l610.ping"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define L610_MIN_PING_PKG_LEN (0)
#define L610_MAX_PING_PKG_LEN (1372)
#define L610_MIN_PING_TIMEOUT (1)
#define L610_MAX_PING_TIMEOUT (65535)

#ifdef L610_USING_PING_OPS

os_err_t l610_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser = &self->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};
    char ip_addr[IPADDR_MAX_STR_LEN + 1]  = {0};

    os_int32_t req_time = 0;
    os_int32_t ttl      = 0;

    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 3,
                         .timeout   = (5 + timeout / 1000) * OS_TICK_PER_SECOND,};

    if (len > L610_MAX_PING_PKG_LEN)
    {
        ERROR("L610 ping: ping package len[%d] is out of range[%d, %d].",
              len,
              L610_MIN_PING_PKG_LEN,
              L610_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout < L610_MIN_PING_TIMEOUT) || (timeout > L610_MAX_PING_TIMEOUT))
    {
        ERROR("L610 ping: ping timeout %ums is out of range[%dms, %dms].",
              timeout,
              L610_MIN_PING_TIMEOUT,
              L610_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    DEBUG("L610 ping: %s, len: %d, timeout: %ums", host, len, timeout);

    /* +MPING=<mode>[,<Destination_IP/hostname>[,<count>[,<size>[,<TTL>[,<TOS>[,<TimeOut>]]]]]] */
    os_err_t result = at_parser_exec_cmd(parser, &at_resp, "AT+MPING=1,\"%s\",1,%d,255,0,%d", host, len, timeout);
    if (result != OS_EOK)
    {
        ERROR("L610 ping %s fail, ping cmd exec failed.", host);
        return result;
    }

    /* +MPINGSTAT: 0,"39.156.66.18",1,1,168 */
    if (at_resp_get_data_by_kw(&at_resp,
                               "+MPINGSTAT:",
                               "+MPINGSTAT: %*d,\"%[^\"]\",%*d,%*d,%hd",
                               ip_addr,
                               &req_time,
                               &ttl) <= 0)
    {
        ERROR("L610 ping %s fail, check network status and try to set a longer timeout.", host);
        result = OS_ERROR;
        return result;
    }
    else
    {
        DEBUG("L610 ping: resp parse ip[%s], req_time[%d], ttl[%d]", ip_addr, req_time, ttl);
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
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

    return result;
}

#endif /* L610_USING_PING_OPS */
