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
 * @file        air723ug_ping.c
 *
 * @brief       air723ug module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "air723ug_ping.h"

#define MO_LOG_TAG "air723ug_ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define AIR723UG_MIN_PING_PKG_LEN (0)
#define AIR723UG_MAX_PING_PKG_LEN (5120)
#define AIR723UG_MIN_PING_TIMEOUT (1)
#define AIR723UG_MAX_PING_TIMEOUT (600)

#ifdef AIR723UG_USING_PING_OPS

os_err_t air723ug_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser          = &self->parser;
    os_err_t     result          = OS_EOK;
    os_int16_t   req_time        = -1;
    os_int16_t   replyid         = -1;
    os_int16_t   ttl             = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    if (parser == OS_NULL)
    {
        ERROR("%s ping: at parser is NULL.", self->name);
        return OS_ERROR;
    }

    if (len > AIR723UG_MAX_PING_PKG_LEN)
    {
        ERROR("%s ping: ping package len[%d] is out of rang[%d, %d].",
                  self->name,
                  len,
                  AIR723UG_MIN_PING_PKG_LEN,
                  AIR723UG_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    /* Def timeout value is 5000ms, AIR723UG in units of 100 ms */
    timeout /= 100;
    if ((timeout < AIR723UG_MIN_PING_TIMEOUT) || (timeout > AIR723UG_MAX_PING_TIMEOUT))
    {
        ERROR("%s ping: user set ping timeout value %ums is out of rang[%dms, %dms].",
                  self->name,
                  timeout * 100,
                  AIR723UG_MIN_PING_TIMEOUT * 100,
                  AIR723UG_MAX_PING_TIMEOUT * 100);
        return OS_ERROR;
    }

    DEBUG("%s ping: %s, len: %d, timeout: %ums", self->name, host, len, timeout * 100);

    char resp_buff[256] = {0};
    /* Need to wait for 3 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                        .buff_size = sizeof(resp_buff),
                        .line_num  = 2,
                        .timeout   =(5 + timeout / 10) * OS_TICK_PER_SECOND};

    /* Exec AT+CIPPING="www.baidu.com",4,64,50; def ping timeout: +CIPPING: 1,39.156.66.14,50,0 */
    /* Success: +CIPPING: 1,39.156.66.14,84,52 */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+CIPPING=\"%s\",1,%d,%d,64", host, len, timeout) < 0)
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
        DEBUG("%s ping: resp prase ip[%s], req_time[%d], ttl[%d]", self->name, ipaddr, req_time, ttl);
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
#endif /* AIR723UG_USING_PING_OPS */
