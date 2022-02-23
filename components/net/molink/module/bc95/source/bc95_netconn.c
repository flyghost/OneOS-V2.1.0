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
 * @file        bc95_netconn.c
 *
 * @brief       bc95 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc95_netconn.h"
#include "bc95.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef BC95_USING_NETCONN_OPS

#define MO_LOG_TAG "bc95.netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define BC95_EVENT_DOMAIN_OK     (1L << 0)
#define BC95_EVENT_DOMAIN_FAIL   (1L << 1)
#ifdef  BC95_USING_PING_OPS
#define BC95_EVENT_PING_OK       (1L << 2)
#define BC95_EVENT_PING_FAIL     (1L << 3)
#endif /* BC95_USING_PING_OPS */

#define PROTOCOL_TYPE_TCP        (6)
#define PROTOCOL_TYPE_UDP        (17)
#define SEND_DATA_MAX_SIZE       (1358)
#define BC95_CONN_ID_NULL        (-1)
#define BC95_SEND_BLOCK_SIZE     (128)

#ifdef  BC95_USING_PING_OPS
#define BC95_MIN_PING_PKG_LEN    (12)
#define BC95_MAX_PING_PKG_LEN    (1500)
#define BC95_MIN_PING_TIMEOUT    (10)
#define BC95_MAX_PING_TIMEOUT    (600000)
#define BC95_PING_INVALID_DEF    (-1)
#define BC95_PING_BUFFER_SIZE    (128)
#endif /* BC95_USING_PING_OPS */

#define BC95_NETCONN_MQ_NAME     "bc95_nc_mq"
#ifndef BC95_NETCONN_MQ_MSG_SIZE
#define BC95_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif /* BC95_NETCONN_MQ_MSG_SIZE */
#ifndef BC95_NETCONN_MQ_MSG_MAX
#define BC95_NETCONN_MQ_MSG_MAX  (5)
#endif /* BC95_NETCONN_MQ_MSG_MAX */

static os_err_t bc95_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t bc95_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *bc95_netconn_alloc(mo_object_t *module)
{
    mo_bc95_t *bc95 = os_container_of(module, mo_bc95_t, parent);

    for (int i = 0; i < BC95_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == bc95->netconn[i].stat)
        {
            /* TODO: Remove after adding the API of setting local port  */
            /* NOTE: BC95 module dosen't support auto allocate udp port */
            bc95->netconn[i].local_port = i + 1;

            return &bc95->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *bc95_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_bc95_t *bc95 = os_container_of(module, mo_bc95_t, parent);

    for (int i = 0; i < BC95_NETCONN_NUM; i++)
    {
        if (connect_id == bc95->netconn[i].connect_id)
        {
            return &bc95->netconn[i];
        }
    }

    return OS_NULL;
}

os_err_t bc95_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_bc95_t *bc95 = os_container_of(module, mo_bc95_t, parent);

    info->netconn_array = bc95->netconn;
    info->netconn_nums  = sizeof(bc95->netconn) / sizeof(bc95->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *bc95_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_bc95_t   *bc95   = os_container_of(module, mo_bc95_t, parent);
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    bc95_lock(&bc95->netconn_lock);

    mo_netconn_t *netconn = bc95_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        bc95_unlock(&bc95->netconn_lock);
        return OS_NULL;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    /* enable user data URC eg. +NSONMI: <socket>,<length>,<data>. */
    result = at_parser_exec_cmd(parser, &resp, "AT+NSONMI=2");
    if (OS_EOK != result)
    {
        ERROR("Module %s set enable netconn urc autorecv data failed", module->name);
        bc95_unlock(&bc95->netconn_lock);
        return OS_NULL;
    }

    switch (type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCR=STREAM,%d,0,1", PROTOCOL_TYPE_TCP);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCR=DGRAM,%d,%hu,1", PROTOCOL_TYPE_UDP, netconn->local_port);
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        ERROR("Module %s create %s netconn failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        bc95_unlock(&bc95->netconn_lock);
        return OS_NULL;
    }

    if (at_resp_get_data_by_line(&resp, 1, "%d", &netconn->connect_id) <= 0)
    {
        ERROR("Module %s get %s netconn id failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        bc95_unlock(&bc95->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(BC95_NETCONN_MQ_NAME,
                               BC95_NETCONN_MQ_MSG_SIZE,
                               BC95_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s message queue create failed, no enough memory.", module->name);
        bc95_unlock(&bc95->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    bc95_unlock(&bc95->netconn_lock);
    return netconn;
}

os_err_t bc95_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser  = &module->parser;
    os_err_t     result  = OS_ERROR;
    os_int32_t   conn_id = BC95_CONN_ID_NULL;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    INFO("Module %s in %d netconn status", module->name, netconn->stat);

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCL=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            ERROR("Module %s destroy %s netconn failed",
                      module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
            return result;
        }
        break;
    default:
        /* add handler when we need */
        break;
    }

    if (netconn->stat != NETCONN_STAT_NULL)
    {
        mo_netconn_mq_destroy(netconn->mq);
        netconn->mq = OS_NULL;
    }

    conn_id              = netconn->connect_id;
    netconn->connect_id  = BC95_CONN_ID_NULL;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    netconn->local_port  = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);

    INFO("Module %s netconn_id: %d destroyed", module->name, conn_id);

    return OS_EOK;
}

os_err_t bc95_netconn_gethostbyname(mo_object_t *module, const char *domain_name, ip_addr_t *addr)
{
    os_err_t     result = OS_EOK;
    os_uint32_t  event  = 0;
    at_parser_t *parser = &module->parser;
    mo_bc95_t   *bc95   = os_container_of(module, mo_bc95_t, parent);

    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};
    char addr_str[IPADDR_MAX_STR_LEN + 1]     = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    bc95_lock(&bc95->netconn_lock);

    bc95->netconn_data = addr_str;

    os_event_clear(&bc95->netconn_evt, BC95_EVENT_DOMAIN_OK | BC95_EVENT_DOMAIN_FAIL);

    result = at_parser_exec_cmd(parser, &resp, "AT+QDNS=0,\"%s\"", domain_name);
    if (OS_EOK != result)
    {
        ERROR("%s-%d: execute dns cmd failed!", __func__, __LINE__);
        goto __exit;
    }

    result = os_event_recv(&bc95->netconn_evt,
                           BC95_EVENT_DOMAIN_OK | BC95_EVENT_DOMAIN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           41 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("%s-%d: get dns result timeout!", __func__, __LINE__);
        goto __exit;
    }

    if (BC95_EVENT_DOMAIN_FAIL & event)
    {
        ERROR("%s-%d: get dns result failed!", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (IPADDR_MIN_STR_LEN > strlen(addr_str) || IPADDR_MAX_STR_LEN < strlen(addr_str))
    {
        ERROR("BC95 domain resolve: recv invalid addr len:%d", strlen(addr_str));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("BC95 domain resolve: \"%s\" domain ip is %s, addrlen %d", domain_name, addr_str, strlen(addr_str));
        inet_aton(addr_str, addr);

        if (IPADDR_ANY == addr->addr || IPADDR_LOOPBACK == addr->addr)
        {
            ip_addr_set_zero(addr);
            result = OS_ERROR;
            goto __exit;
        }

        result = OS_EOK;
    }

__exit:

    bc95->netconn_data = OS_NULL;

    bc95_unlock(&bc95->netconn_lock);

    return result;
}

static os_err_t bc95_tcp_connect(at_parser_t *parser, os_int32_t connect_id, char *ip_addr, os_uint16_t port)
{
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NSOCO=%d,%s,%hu", connect_id, ip_addr, port);

    return result;
}

os_err_t bc95_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = bc95_tcp_connect(parser, netconn->connect_id, remote_ip, port);
        break;
    case NETCONN_TYPE_UDP:
        result = OS_EOK;    /* UDP does not need to connect */
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        ERROR("Module %s connect to %s:%u failed!", module->name, remote_ip, port);
        return result;
    }

    ip_addr_copy(netconn->remote_ip, addr);
    netconn->remote_port = port;
    netconn->stat        = NETCONN_STAT_CONNECT;

    DEBUG("Module %s connect to %s:%u successfully!", module->name, remote_ip, port);

    return OS_EOK;
}

static os_size_t bc95_single_packet_send(at_parser_t *parser, const char *data, os_size_t size)
{
    os_size_t  sent_size     = 0; /* raw data sent size */
    os_size_t  curr_pkt_size = 0; /* raw data current packet size */

    char hex_str_buff[BC95_SEND_BLOCK_SIZE] = {0};

    while (size > sent_size)
    {
        curr_pkt_size = (size - sent_size) < (BC95_SEND_BLOCK_SIZE / 2) ? (size - sent_size) : BC95_SEND_BLOCK_SIZE / 2;

        bytes_to_hexstr(data + sent_size, hex_str_buff, curr_pkt_size);

        if (at_parser_send(parser, hex_str_buff, 2 * curr_pkt_size) <= 0)
        {
            goto __exit;
        }

        sent_size += curr_pkt_size;
    }

__exit:

    return sent_size;
}

static os_size_t bc95_hexdata_send(at_parser_t *parser, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    os_err_t   result            = OS_EOK;
    os_int32_t connect_id        = BC95_CONN_ID_NULL;
    os_size_t  sent_size         = 0;
    os_size_t  cur_pkt_size      = 0;
    os_size_t  cnt               = 0;

    char send_cmd[AT_RESP_BUFF_SIZE_DEF]   = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = AT_RESP_BUFF_SIZE_DEF, .timeout =  10 * OS_TICK_PER_SECOND};

    strncpy(remote_ip, inet_ntoa(netconn->remote_ip), IPADDR_MAX_STR_LEN);

    at_parser_exec_lock(parser);

    while (sent_size < size)
    {
        if (netconn->stat != NETCONN_STAT_CONNECT)
        {
            ERROR("Module %s netconn %d isn't in the CONNECT state, send data fail", parser->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        if (size - sent_size < SEND_DATA_MAX_SIZE)
        {
            cur_pkt_size = size - sent_size;
        }
        else
        {
            cur_pkt_size = SEND_DATA_MAX_SIZE;
        }

        if (NETCONN_TYPE_TCP == netconn->type)
        {
            snprintf(send_cmd, sizeof(send_cmd),
                    "AT+NSOSD=%d,%d,",
                     netconn->connect_id,
                    (int)cur_pkt_size);
        }
        else /* other cases judged by upper function */
        {
            snprintf(send_cmd, sizeof(send_cmd),
                    "AT+NSOST=%d,%s,%d,%hu,",
                     netconn->connect_id,
                     remote_ip,
                     netconn->remote_port,
                    (int)cur_pkt_size);
        }

        /* send cmd prefix */
        if (at_parser_send(parser, send_cmd, strlen(send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        /* send data */
        if (cur_pkt_size != bc95_single_packet_send(parser, data + sent_size, cur_pkt_size))
        {
            result = OS_ERROR;
            ERROR("Drv or hardware send data fail, some data is missing.\r\n");
            goto __exit;
        }

        result = at_parser_exec_cmd(parser, &resp, "");
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_resp_get_data_by_line(&resp, 1, "%d,%d", &connect_id, &cnt) <= 0 || cnt != cur_pkt_size)
        {
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += cur_pkt_size;
    }

__exit:

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s netconn %d send %d bytes data failed!",
               parser->name,
               netconn->connect_id,
               cur_pkt_size);
        return 0;
    }

    return sent_size;
}


os_size_t bc95_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_size_t    sent_size = 0;
    mo_bc95_t   *bc95      = os_container_of(module, mo_bc95_t, parent);

    if (OS_EOK != bc95_lock(&bc95->netconn_lock))
    {
        ERROR("Moudle %s netconn %d send lock failed.", module->name, netconn->connect_id);
        return 0;
    }

    if (NETCONN_TYPE_TCP == netconn->type || NETCONN_TYPE_UDP == netconn->type)
    {
        sent_size = bc95_hexdata_send(parser, netconn, data, size);
    }
    else
    {
        ERROR("Moudle %s netconn %d type %d error.", module->name, netconn->connect_id, netconn->type);
    }

    if (OS_EOK != bc95_unlock(&bc95->netconn_lock))
    {
        ERROR("Moudle %s netconn %d send unlock failed.", module->name, netconn->connect_id);
        return 0;
    }

    return sent_size;
}

#ifdef BC95_USING_PING_OPS
os_err_t bc95_ping_handler(mo_object_t *module, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp)
{
    at_parser_t *parser   = &module->parser;
    os_err_t     result   = OS_EOK;
    os_int16_t   ttl      = BC95_PING_INVALID_DEF;
    ip_addr_t    addr     = {0};
    os_uint32_t  req_time = 0;
    os_uint32_t  event    = 0;
    mo_bc95_t   *bc95     = os_container_of(module, mo_bc95_t, parent);

    char ip_send[IPADDR_MAX_STR_LEN + 1]  = {0};
    char ip_recv[IPADDR_MAX_STR_LEN + 1]  = {0};
    char ret_buff[BC95_PING_BUFFER_SIZE]  = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    bc95_lock(&bc95->netconn_lock);

    if (OS_NULL == parser)
    {
        ERROR("BC95 ping: at parser is NULL.");
        return OS_ERROR;
    }

    if ((len < BC95_MIN_PING_PKG_LEN) || (len > BC95_MAX_PING_PKG_LEN))
    {
        ERROR("BC95 ping: ping package len[%d] is out of range[%d, %d].",
                  len, BC95_MIN_PING_PKG_LEN, BC95_MAX_PING_PKG_LEN);
        return OS_ERROR;
    }

    if ((timeout < BC95_MIN_PING_TIMEOUT) || (timeout > BC95_MAX_PING_TIMEOUT))
    {
        ERROR("BC95 ping: user set ping timeout value %ums is out of range[%dms, %dms].",
                  timeout, BC95_MIN_PING_TIMEOUT, BC95_MAX_PING_TIMEOUT);
        return OS_ERROR;
    }

    DEBUG("BC95 ping: %s, len: %u, timeout: %ums", host, len, timeout);

    /* DNS: bc95 dosen't support ping domain name, do dns before ping */
    result = bc95_netconn_gethostbyname(module, host, &addr);
    if (OS_EOK != result)
    {
        ERROR("BC95 ping: dns error, host: %s", host);
        return result;
    }

    strncpy(ip_send, inet_ntoa(addr), IPADDR_MAX_STR_LEN);
    DEBUG("BC95 ping: domain ip is %s, addrlen %d", ip_send, strlen(ip_send));
    /* DNS: finished */

    /**
     * Exec commond "AT+NPING=183.232.231.174,64,5000 and wait response
     * Return: success: +NPING:183.232.231.174,54,1974  fail: +NPINGERR:1
     * Module only support IPV4, BC95 only support Dotted Dec/Hex/Oct Notation
     * Because unable to synchronize time between board & module, 5 seconds reserved
     *  **/
    at_resp_t at_resp = {.buff      = resp_buff,
                         .buff_size = sizeof(resp_buff),
                         .timeout   = (5 + timeout / 1000) * OS_TICK_PER_SECOND};

    os_event_clear(&bc95->netconn_evt, BC95_EVENT_PING_OK | BC95_EVENT_PING_FAIL);

    bc95->netconn_data = ret_buff;

    if (at_parser_exec_cmd(parser, &at_resp, "AT+NPING=%s,%hd,%u", ip_send, len, timeout) < 0)
    {
        ERROR("Ping: AT cmd exec fail: AT+NPING=%s,%u,%u", ip_send, len, timeout);
        result = OS_ERROR;
        goto __exit;
    }

    result = os_event_recv(&bc95->netconn_evt,
                           BC95_EVENT_PING_OK | BC95_EVENT_PING_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           at_resp.timeout,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("%s-%d: get ping result timeout!", __func__, __LINE__);
        goto __exit;
    }

    if (BC95_EVENT_PING_FAIL & event)
    {
        ERROR("%s-%d: get ping result failed!", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    /* 3 param required: ip_recv,ttl,req_time */
    if (3 > sscanf(ret_buff, "+NPING: %[^,],%hd,%u", ip_recv, &ttl, &req_time))
    {
        ERROR("BC95 ping %s fail, invalid ping result.", host);
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("BC95 ping: resp parse ip[%s], req_time[%u], ttl[%hd]", ip_recv, req_time, ttl);
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

    bc95->netconn_data = OS_NULL;

    bc95_unlock(&bc95->netconn_lock);

    return result;
}

static void urc_ping_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_err_t     result = OS_EOK;
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc95_t   *bc95   = os_container_of(module, mo_bc95_t, parent);

    if (OS_NULL == bc95->netconn_data)
    {
        ERROR("%s-%d: no useable buffer.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (BC95_PING_BUFFER_SIZE <= size)
    {
        ERROR("%s-%d: ping result size is bigger than buffer.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    memcpy(bc95->netconn_data, data, size);

__exit:

    if (OS_EOK == result)
    {
        os_event_send(&bc95->netconn_evt, BC95_EVENT_PING_OK);
    }
    else
    {
        os_event_send(&bc95->netconn_evt, BC95_EVENT_PING_FAIL);
    }

    return;
}

static void urc_ping_err_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc95_t   *bc95   = os_container_of(module, mo_bc95_t, parent);

    ERROR("%s-%d: ping error[%s]", __func__, __LINE__, data);

    os_event_send(&bc95->netconn_evt, BC95_EVENT_PING_FAIL);

    return;
}

#endif /* BC95_USING_PING_OPS */

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "+NSOCLI: %d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = bc95_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    WARN("Module %s receive close urc data of connect %d", module->name, connect_id);

    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    sscanf(data, "+NSONMI:%d,%*[^,],%*d,%d,", &connect_id, &data_size);

    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = bc95_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        /*  bufflen >= strsize + 1 */
        char *recv_buff = os_calloc(1, data_size * 2 + 1);
        if (recv_buff == OS_NULL)
        {
            ERROR("Calloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
            return;
        }

        /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
        /* Get receive data to receive buffer */
        sscanf(data, "+NSONMI:%*d,%*[^,],%*d,%*d,%s", recv_buff);

        char *recv_str = os_calloc(1, data_size + 1);
        if (recv_str == OS_NULL)
        {
            ERROR("Calloc recv str %d bytes fail, no enough memory", data_size + 1);
            os_free(recv_buff);
            return;
        }

        hexstr_to_bytes(recv_buff, recv_str, data_size * 2);

        mo_netconn_data_recv_notice(netconn, recv_str, data_size);

        os_free(recv_buff);
    }

    return;
}

static void urc_dns_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_err_t     result = OS_EOK;
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc95_t   *bc95   = os_container_of(module, mo_bc95_t, parent);
    char ret_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    if (OS_NULL == bc95->netconn_data)
    {
        ERROR("%s-%d: no useable buffer.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (0 >= sscanf(data, "+QDNS: %s", ret_buff))
    {
        ERROR("%s-%d: get dns result failed.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (IPADDR_MAX_STR_LEN < strlen(ret_buff))
    {
        ERROR("%s-%d: dns return invalid len.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    memcpy(bc95->netconn_data, ret_buff, strlen(ret_buff));

__exit:

    if (OS_EOK == result)
    {
        os_event_send(&bc95->netconn_evt, BC95_EVENT_DOMAIN_OK);
    }
    else
    {
        os_event_send(&bc95->netconn_evt, BC95_EVENT_DOMAIN_FAIL);
    }

    return;
}

static at_urc_t netconn_urc_table[] = {
    {.prefix = "+NSOCLI:", .suffix = "\r\n", .func = urc_close_func},
    {.prefix = "+NSONMI:", .suffix = "\r\n", .func = urc_recv_func },
    {.prefix = "+QDNS:",   .suffix = "\r\n", .func = urc_dns_func  },
};

#ifdef BC95_USING_PING_OPS
static at_urc_t ping_urc_table[] = {
    {.prefix = "+NPING:",    .suffix = "\r\n", .func = urc_ping_func    },
    {.prefix = "+NPINGERR:", .suffix = "\r\n", .func = urc_ping_err_func},
};
#endif /* BC95_USING_PING_OPS */

void bc95_netconn_init(mo_bc95_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < BC95_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = BC95_CONN_ID_NULL;
    }

    /* Set urc table */
    at_parser_t *parser = &(module->parent.parser);

    at_parser_set_urc_table(parser, netconn_urc_table, sizeof(netconn_urc_table) / sizeof(at_urc_t));

#ifdef BC95_USING_PING_OPS
    at_parser_set_urc_table(parser, ping_urc_table, sizeof(ping_urc_table) / sizeof(at_urc_t));
#endif /* BC95_USING_PING_OPS */
}

#endif /* BC95_USING_NETCONN_OPS */
