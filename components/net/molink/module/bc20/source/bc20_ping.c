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
 * @file        bc20_ping.c
 *
 * @brief       bc20 module link kit ping api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc20_ping.h"
#include "bc20.h"
#include <string.h>
#include <stdio.h>

#define MO_LOG_TAG "bc20.ping"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifdef BC20_USING_PING_OPS

#define BC20_MIN_PING_PKG_LEN     (32)
#define BC20_MAX_PING_PKG_LEN     (200)
#define BC20_MIN_PING_TIMEOUT_SEC (1)
#define BC20_MAX_PING_TIMEOUT_SEC (255)
#define BC20_PING_INVALID_DEF     (-1)
#define BC20_PING_BUFFER_SIZE     (4 * AT_RESP_BUFF_SIZE_DEF)

#define BC20_EVENT_PING_OK       (1L << 0)
#define BC20_EVENT_PING_FAIL     (1L << 1)

os_err_t bc20_ping(mo_object_t *module, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_int16_t   ttl       = BC20_PING_INVALID_DEF;
    os_uint32_t  timeout_s = timeout / 1000;
    os_uint32_t  req_time  = 0;
    os_uint32_t  event     = 0;
    mo_bc20_t   *bc20      = os_container_of(module, mo_bc20_t, parent);

    char ip_recv[IPADDR_MAX_STR_LEN + 1]        = {0};
    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF]   = {0};
    char ping_result[BC20_PING_BUFFER_SIZE + 1] = {0};

    memset(resp, 0, sizeof(ping_resp_t));

    if (parser == OS_NULL)
    {
        ERROR("BC20 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < BC20_MIN_PING_PKG_LEN) || (len > BC20_MAX_PING_PKG_LEN))
    {
        ERROR("BC20 ping: ping package len[%d] is out of rang[%d, %d].",
                  len, BC20_MIN_PING_PKG_LEN, BC20_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout_s < BC20_MIN_PING_TIMEOUT_SEC) || (timeout_s > BC20_MAX_PING_TIMEOUT_SEC))
    {
        ERROR("BC20 ping: user set ping timeout_s value %u s is out of rang[%d, %d]s.",
                  timeout_s, BC20_MIN_PING_TIMEOUT_SEC, BC20_MAX_PING_TIMEOUT_SEC);
        return OS_ERROR;
    }

    DEBUG("BC20 ping:[%s], len:[%u], timeout_s:[%u]s", host, len, timeout_s);

    at_parser_exec_lock(parser);

    bc20->ping_data = ping_result;

    /**
     * +QPING: <result>[,<IP_address>,<bytes>,<time>,<TTL>]
     * +QPING: <finresult>[,<sent>,<rcvd>,<lost>,<min>,<max>,<avg>]
     * Because unable to synchronize time between board & module, 5 seconds reserved
     *  **/
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .timeout   = (10 + timeout_s) * OS_TICK_PER_SECOND};

    os_event_clear(&bc20->ping_evt, BC20_EVENT_PING_OK | BC20_EVENT_PING_FAIL);

    if (at_parser_exec_cmd(parser, &at_resp, "AT+QPING=1,\"%s\",%u,1,%hu", host, timeout_s, len) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+QPING=1,\"%s\",%u,1,%hu", host, timeout_s, len);
        result = OS_ERROR;
        goto __exit;
    }

    result = os_event_recv(&bc20->ping_evt,
                            BC20_EVENT_PING_OK | BC20_EVENT_PING_FAIL,
                            OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                            at_resp.timeout,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("%s-%d: get ping result timeout!", __func__, __LINE__);
        goto __exit;
    }

    if (BC20_EVENT_PING_FAIL & event)
    {
        ERROR("%s-%d: get ping result failed!", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (3 > sscanf(ping_result, "+QPING: %*d,\"%[^\"]\",%*u,%u,%hu", ip_recv, &req_time, &ttl))
    {
        ERROR("%s-%d: get ping:%s result error!", __func__, __LINE__, host);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("BC20 ping: resp parse ip[%s], req_time[%u], ttl[%hd]", ip_recv, req_time, ttl);
        if (0 > ttl || 0 == req_time)
        {
            result = OS_ERROR;
            ERROR("%s-%d: ping result invalid.", __func__, __LINE__);
            goto __exit;
        }
        else
        {
            inet_aton(ip_recv, &(resp->ip_addr));
            resp->data_len = len;
            resp->ttl      = ttl;
            resp->time     = req_time;
            result         = OS_EOK;
        }
    }

__exit:

    bc20->ping_data = OS_NULL;

    at_parser_exec_unlock(parser);

    return result;
}

static void urc_ping_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_err_t     result = OS_EOK;
    os_int32_t   ret    = 0;
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc20_t   *bc20   = os_container_of(module, mo_bc20_t, parent);
    os_int32_t   ping_result = BC20_PING_INVALID_DEF;
    char         ping_sign   = 0;

    if (OS_NULL == bc20->ping_data)
    {
        ERROR("%s-%d: ping result returning out of expectation.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (BC20_PING_BUFFER_SIZE < size)
    {
        ERROR("%s-%d: ping result is out of buffer size.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    ret = sscanf(data, "+QPING: %d,%c", &ping_result, &ping_sign);

    switch (ret)
    {
    case 2:
        /* 2 params means we got the ping result or final ping result */
        break;
    case 1:
        result = OS_ERROR;
        goto __exit;
    default:
        ERROR("%s-%d: get ping result failed!", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if ('\"' != ping_sign)
    {
        /* ignore ping final result */
        return;
    }

__exit:
    
    if (OS_EOK == result && 0 == ping_result)
    {
        memcpy(bc20->ping_data, data, size);
        os_event_send(&bc20->ping_evt, BC20_EVENT_PING_OK);
    }
    else
    {
        ERROR("%s-%d: get ping errno:%d!", __func__, __LINE__, ping_result);
        os_event_send(&bc20->ping_evt, BC20_EVENT_PING_FAIL);
    }

    return;
}

static at_urc_t ping_urc_table[] = {
    {.prefix = "+QPING:", .suffix = "\r\n", .func = urc_ping_func},
};

void bc20_ping_init(mo_bc20_t *module)
{
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, ping_urc_table, sizeof(ping_urc_table) / sizeof(at_urc_t));
}

#endif /* BC20_USING_PING_OPS */
