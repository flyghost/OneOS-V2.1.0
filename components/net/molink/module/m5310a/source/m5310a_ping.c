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
 * @file        m5310a_ping.c
 *
 * @brief       m5310a module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5310a_ping.h"

#define MO_LOG_TAG "m5310a.ping"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define M5310A_MIN_PING_PKG_LEN (8)
#define M5310A_MAX_PING_PKG_LEN (1460)
#define M5310A_MIN_PING_TIMEOUT (10)
#define M5310A_MAX_PING_TIMEOUT (60000)

#ifdef M5310A_USING_PING_OPS

os_err_t m5310a_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser   = &self->parser;
    os_err_t     result   = OS_EOK;
    os_uint32_t  req_time = 0;
    os_int16_t   ttl      = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1]  = {0};
    char ret_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    if (parser == OS_NULL)
    {
        ERROR("M5310-A ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < M5310A_MIN_PING_PKG_LEN) || (len > M5310A_MAX_PING_PKG_LEN))
    {
        ERROR("M5310-A ping: ping package len[%d] is out of range[%d, %d].",
              len,
              M5310A_MIN_PING_PKG_LEN,
              M5310A_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout < M5310A_MIN_PING_TIMEOUT) || (timeout > M5310A_MAX_PING_TIMEOUT))
    {
        ERROR("M5310-A ping: user set ping timeout value %ums is out of range[%dms, %dms].",
              timeout,
              M5310A_MIN_PING_TIMEOUT,
              M5310A_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    DEBUG("M5310-A ping: %s, len: %d, timeout: %ums", host, len, timeout);

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};
    /* Need to wait for 4 lines response msg, and timeout comming with ms */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 2,
                         .timeout   = (5 + timeout / 1000) * OS_TICK_PER_SECOND};

    /* Default set timeout to 5000ms */
    /* It is found that the ping packet of M5310-A takes 4 seconds to return */
    /* Exec commond "AT+NPING=www.baidu.com,64,5000,4 and wait response */
    /* Return: success: +NPING:183.232.231.174,54,1974  fail: +NPINGERR:1 */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+NPING=%s,%d,%u,1", host, len, timeout) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+NPING=%s,%d,%u", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+NPING", "+NPING:%[^,],%hd,%u", ipaddr, &ttl, &req_time) <= 0)
    {
        if (at_resp_get_data_by_kw(&at_resp, "+", "+%s", ret_buff) <= 0)
        {
            ERROR("AT+NPING resp parse \"+NPINGERR\" fail.");
        }

        ERROR("M5310-A ping %s fail: %s, check network status and try to set a longer timeout.", host, ret_buff);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("M5310-A ping: resp parse ip[%s], req_time[%u], ttl[%d]", ipaddr, req_time, ttl);
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
        inet_aton(ipaddr, &(resp->ip_addr));
        resp->data_len = len;
        resp->ttl      = ttl;
        resp->time     = req_time;
    }

__exit:

    return result;
}

#endif /* M5310A_USING_PING_OPS */
