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
 * @file        sim7600ce_ping.c
 *
 * @brief       sim7600ce module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "sim7600ce_ping.h"

#define MO_LOG_TAG "sim7600ce.ping"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>
#define SIM7600CE_MIN_PING_PKG_LEN (4)
#define SIM7600CE_MAX_PING_PKG_LEN (188)
#define SIM7600CE_MIN_PING_TIMEOUT (10000)
#define SIM7600CE_MAX_PING_TIMEOUT (100000)

#ifdef SIM7600CE_USING_PING_OPS

typedef enum sim7600ce_ping_ret_tpye
{
    PING_OK = 1,
    PING_TIMEOUT,
    PING_SUMMARY,
    PING_UNKNOWN_RET
}sim7600ce_ping_ret_tpye_t;

os_err_t sim7600ce_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser    = &self->parser;
    os_err_t     result    = OS_EOK;
    os_int16_t   req_time  = -1;
    os_int16_t   ttl       = -1;
    os_int16_t   req_len   = -1;
    os_uint8_t   ping_stat = 0;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    if (parser == OS_NULL)
    {
        ERROR("%s ping: at parser is NULL.", self->name);
        return OS_ERROR;
    }

    if ((len > SIM7600CE_MAX_PING_PKG_LEN) || (len < SIM7600CE_MIN_PING_PKG_LEN))
    {
        ERROR("%s ping: ping package len[%d] is out of range[%d, %d].",
              self->name,
              len,
              SIM7600CE_MIN_PING_PKG_LEN,
              SIM7600CE_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if (timeout > SIM7600CE_MAX_PING_TIMEOUT)
    {
        ERROR("%s ping: user set ping timeout value %ums is out of range[%dms, %dms].",
              self->name,
              timeout,
              SIM7600CE_MIN_PING_TIMEOUT,
              SIM7600CE_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    /* Def timeout value is 5000ms */
    if (timeout < SIM7600CE_MIN_PING_TIMEOUT)
    {
        DEBUG("%s ping: config timeout %ums is less than the min effective value %dms, set timeout to %d.",
              self->name,
              timeout,
              SIM7600CE_MIN_PING_TIMEOUT,
              SIM7600CE_MIN_PING_TIMEOUT);

        timeout = SIM7600CE_MIN_PING_TIMEOUT;
    }

    DEBUG("%s ping: %s, len: %d, timeout: %ums", self->name, host, len, timeout);

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};
    /* Need to wait for 6 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 3,
                         .timeout   = (5 + timeout / 1000) * OS_TICK_PER_SECOND};

    /* Exec AT+CPING="www.baidu.com",1,1,64,,10000; */
    /* Def ping timeout: OK\r\n\r\n +CPING: 2\r\n\r\n +CPING: 3,1,0,1,0,0,0\r\n */
    /* Def ping success: OK\r\n\r\n +CPING: 1,183.232.231.174,64,71,255\r\n\r\n  +CPING: 3,1,1,0,71,71,71\r\n */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+CPING=\"%s\",1,1,%d,,%d", host, len, timeout) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+CPING=\"%s\",1,1,%d,,%d", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_line(&at_resp, 2, "+CPING: %hhu", &ping_stat) < 0)
    {
        ERROR("%s ping: result type parse fail", self->name);
        result = OS_ERROR;
        goto __exit;
    }

    switch(ping_stat)
    {
    case PING_OK:
        if (at_resp_get_data_by_line(&at_resp, 2, "+CPING: 1,%[^,],%d,%d,%d", ipaddr, &req_len, &req_time, &ttl) < 0)
        {
            ERROR("%s ping %s fail: check network status and try to set a longer timeout.", self->name, host);
            result = OS_ERROR;
            goto __exit;
        }
        else
        {
            DEBUG("%s ping: resp parse ip[%s], req_time[%d], ttl[%d]", self->name, ipaddr, req_time, ttl);
            inet_aton(ipaddr, &(resp->ip_addr));
            resp->data_len = len;
            resp->ttl      = ttl;
            resp->time     = req_time;
            result         = OS_EOK;
        }
        break;

    case PING_TIMEOUT:
        ERROR("Ping timeout, check net status and please try again");
        result = OS_ETIMEOUT;
        break;

    default:
        ERROR("%s ping fail: ping result type parse failed, unknown type [%u]", ping_stat);
        result = OS_ERROR;
        break;
    }

__exit:

    return result;
}

#endif /* SIM7600CE_USING_PING_OPS */
