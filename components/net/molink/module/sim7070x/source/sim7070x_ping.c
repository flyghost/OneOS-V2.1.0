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
 * @file        sim7070x_ping.c
 *
 * @brief       sim7070x module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "sim7070x.h"
#include "sim7070x_ping.h"

#define MO_LOG_TAG "sim7070x_ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define SIM7070X_MIN_PING_PKG_LEN (1)
#define SIM7070X_MAX_PING_PKG_LEN (1400)
#define SIM7070X_MIN_PING_TIMEOUT (1)
#define SIM7070X_MAX_PING_TIMEOUT (60000)

#ifdef SIM7070X_USING_PING_OPS

os_err_t sim7070x_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser    = &self->parser;
    os_err_t     result    = OS_EOK;
    os_int32_t   req_time  = -1;

    char ipaddr[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[256]                 = {0};

    if (parser == OS_NULL)
    {
        ERROR("%s ping: at parser is NULL.", self->name);
        return OS_ERROR;
    }

    if ((len > SIM7070X_MAX_PING_PKG_LEN) || (len < SIM7070X_MIN_PING_PKG_LEN))
    {
        ERROR("%s ping: ping package len[%d] is out of range[%d, %d].",
                  self->name,
                  len,
                  SIM7070X_MIN_PING_PKG_LEN,
                  SIM7070X_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout > SIM7070X_MAX_PING_TIMEOUT) || (timeout == 0))
    {
        ERROR("%s ping: user set ping timeout value %ums is out of range[%dms, %dms].",
                  self->name,
                  timeout,
                  SIM7070X_MIN_PING_TIMEOUT,
                  SIM7070X_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    DEBUG("%s ping: %s, len: %d, timeout: %ums", self->name, host, len, timeout);

    /* step 1: activate the app network 0th PDP */
    result = module_sim7070x_app_network_pdpidx0_init(self);
    if (result != OS_EOK)
    {
        return result;
    }

    /* step 2: config auto select defined PDP index to ping */
    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = (5 * OS_TICK_PER_SECOND)};
    if (at_parser_exec_cmd(parser, &at_resp, "AT+SNPDPID=0") < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+SNPDPID=0");
        result = OS_ERROR;
        goto __exit;
    }

    /* step 3: ping IPv4 address and parse resp */
    at_resp.timeout = (5 + timeout / 1000) * OS_TICK_PER_SECOND;

    /* Exec AT+SNPING4="www.qq.com",1,100,5000 */
    /* Def ping timeout: +SNPING4: 1,120.241.22.157,60000\r\n\r\nOK\r\n */
    /* Def ping success: +SNPING4: 1,120.241.22.157,4714\r\n\r\nOK\r\n */
    if (at_parser_exec_cmd(parser, &at_resp, "AT+SNPING4=\"%s\",1,%u,%u", host, len, timeout) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+SNPING4=\"%s\",1,%u,%u", host, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+SNPING4:", "+SNPING4: 1,%[^,],%u", ipaddr, &req_time) < 0)
    {
        ERROR("%s ping %s fail: check network status and try to set a longer timeout.", self->name, host);
        result = OS_ERROR;
        goto __exit;
    }

    if (req_time == SIM7070X_MAX_PING_TIMEOUT)
    {
        DEBUG("Ping timeout, check network status and please try to ping again");
        result = OS_ETIMEOUT;
    }
    else
    {
        DEBUG("%s ping: resp parse ip[%s], req_time[%u]", self->name, ipaddr, req_time);
        inet_aton(ipaddr, &(resp->ip_addr));
        resp->data_len = len;
        resp->time     = req_time;
        result         = OS_EOK;
    }

__exit:

    return result;
}

#endif /* SIM7070X_USING_PING_OPS */
