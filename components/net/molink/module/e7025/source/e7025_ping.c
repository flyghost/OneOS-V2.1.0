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
 * @file        e7025_ping.c
 *
 * @brief       e7025 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "e7025_ping.h"

#define DBG_EXT_TAG "e7025.ping"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define E7025_MIN_PING_PKG_LEN (1)
#define E7025_MAX_PING_PKG_LEN (5000)
#define E7025_MIN_PING_TIMEOUT (1000)
#define E7025_MAX_PING_TIMEOUT (600000)

#ifdef E7025_USING_PING_OPS

os_err_t e7025_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser   = &self->parser;
    os_err_t     result   = OS_EOK;
    os_int16_t   req_time = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};

    if (parser == OS_NULL)
    {
        LOG_EXT_E("Module %s ping: at parser is NULL.", self->name);
        return OS_ERROR;
    }

    if (len > E7025_MAX_PING_PKG_LEN)
    {
        LOG_EXT_E("Module %s ping: ping package len[%d] is out of range[%d, %d].",
                  self->name,
                  len,
                  E7025_MIN_PING_PKG_LEN,
                  E7025_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout < E7025_MIN_PING_TIMEOUT) || (timeout > E7025_MAX_PING_TIMEOUT))
    {
        LOG_EXT_E("Module %s ping: user set ping timeout value %ums is out of range[%dms, %dms].",
                  self->name,
                  timeout,
                  E7025_MIN_PING_TIMEOUT,
                  E7025_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    LOG_EXT_D("Module %s ping: %s, len: %d, timeout: %ums", self->name, host, len, timeout);

    char resp_buff[256] = {0};
    /* At least, need to wait for 4 lines response msg */
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .line_num  = 4,
                         .timeout   = (5 + timeout / 1000) * OS_TICK_PER_SECOND};

    /* Exec AT+ECPING="180.101.147.115",1,64,10000; */
    /* Def ping timeout: +ECPING: FAIL, dest: 180.101.147.115, time out: 10000 ms*/
    /* If ping  success: +ECPING: SUCC, dest: 180.101.147.115, RTT: 725 ms */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+ECPING=\"%s\",1,%d,%d", host, len, timeout) < 0)
    {
        LOG_EXT_E("Module %s ping: AT cmd exec fail: AT+ECPING=\"%s\",1,%d,%d", self->name, host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+ECPING: SUCC", "+ECPING: SUCC, dest: %[^,], RTT: %u ms", ipaddr, &req_time) <= 0)
    {
        if(at_resp_get_data_by_kw(&at_resp, "+ECPING: FAIL", "+ECPING: FAIL, dest: %[^,], time out: %u ms", ipaddr, &req_time) <= 0)
        {
            LOG_EXT_E("Module %s ping %s fail: check network status or try to set a longer timeout.", self->name, host);
            result = OS_ERROR;
            goto __exit;
        }
    }


    LOG_EXT_D("Module %s ping: resp parse ip[%s], req_time[%d]", self->name, ipaddr, req_time);
    if (req_time == timeout)
    {
        result = OS_ETIMEOUT;
    }
    else
    {
        result = OS_EOK;
    }

    inet_aton(ipaddr, &(resp->ip_addr));
    resp->data_len = len;
    resp->time     = req_time;

__exit:

    return result;
}

#endif /* E7025_USING_PING_OPS */
