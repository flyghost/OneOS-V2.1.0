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
 * @file        esp32_ping.c
 *
 * @brief       esp32 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp32_ping.h"

#define MO_LOG_TAG "esp32.ping"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#ifdef ESP32_USING_PING_OPS

os_err_t esp32_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser    = &self->parser;
    os_uint32_t  ping_time = 0;

    char ip_addr[IPADDR_MAX_STR_LEN + 1]  = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};

    at_resp_t at_resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &at_resp, "AT+CIPDOMAIN=\"%s\"", host);

    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&at_resp, "+CIPDOMAIN:", "+CIPDOMAIN:%s", ip_addr) < 0)
    {
        ERROR("ping: get the IP address failed");
        result = OS_ERROR;
        goto __exit;
    }

    at_resp.timeout = (5 + timeout / 1000) * OS_TICK_PER_SECOND;

    DEBUG("ESP32 series module does not support setting ping package size and timeout.");

    /* send ping commond "AT+PING=<IP>" and wait response */
    result = at_parser_exec_cmd(parser, &at_resp, "AT+PING=\"%s\"", ip_addr);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if ((at_resp_get_data_by_kw(&at_resp,  "+", "+%u", &ping_time) < 0) && (at_resp_get_data_by_kw(&at_resp,  "+PING", "+PING:%u", &ping_time) < 0))
    {
        ERROR("ping: get the ping time error");
        result = OS_ERROR;
        goto __exit;
    }

    if (ping_time != 0)
    {
        inet_aton(ip_addr, &(resp->ip_addr));
        resp->data_len = 32; /* The default ICMP payload length is 32 bytes */
        resp->ttl      = 0;  /* There is no TTL data in the response */
        resp->time     = ping_time;
    }

__exit:

    return result;
}

#endif /* ESP32_USING_PING_OPS */
