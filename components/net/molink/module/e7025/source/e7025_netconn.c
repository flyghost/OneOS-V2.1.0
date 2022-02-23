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
 * @file        e7025_netconn.c
 *
 * @brief       e7025 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "e7025_netconn.h"
#include "e7025.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DBG_EXT_TAG "e7025.netconn"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <os_dbg_ext.h>

#define SEND_DATA_MAX_SIZE    (1358)

#ifndef E7025_DATA_QUEUE_SIZE
#define E7025_DATA_QUEUE_SIZE (5)
#endif

#ifdef E7025_USING_NETCONN_OPS

static os_err_t e7025_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_IPC_WAITING_FOREVER);
}

static os_err_t e7025_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *e7025_netconn_alloc(mo_object_t *module)
{
    mo_e7025_t *e7025 = os_container_of(module, mo_e7025_t, parent);

    for (int i = 0; i < E7025_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == e7025->netconn[i].stat)
        {
            return &e7025->netconn[i];
        }
    }

    LOG_EXT_E("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *e7025_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 >= connect_id || E7025_NETCONN_NUM < connect_id)
    {
        LOG_EXT_E("Module %s get netconn by id failed, INVALID connet_id: %d", module->name, connect_id);
        return OS_NULL;
    }

    mo_e7025_t *e7025 = os_container_of(module, mo_e7025_t, parent);

    for (int i = 0; i < E7025_NETCONN_NUM; i++)
    {
        if (connect_id == e7025->netconn[i].connect_id)
        {
            return &e7025->netconn[i];
        }
    }

    return OS_NULL;
}

os_err_t e7025_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_e7025_t *e7025 = os_container_of(module, mo_e7025_t, parent);

    info->netconn_array = e7025->netconn;
    info->netconn_nums  = sizeof(e7025->netconn) / sizeof(e7025->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *e7025_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_e7025_t   *e7025   = os_container_of(module, mo_e7025_t, parent);
    at_parser_t  *parser  = &module->parser;
    os_err_t      result  = OS_EOK;
    os_int32_t    conn_id = -1;

    e7025_lock(&e7025->netconn_lock);

    mo_netconn_t *netconn = e7025_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        e7025_unlock(&e7025->netconn_lock);
        return OS_NULL;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    switch (type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser, &resp, "AT+ECSOCR=STREAM,6,,1,AF_INET");
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser, &resp, "AT+ECSOCR=DGRAM,17,,1,AF_INET");
        break;
    default:
        result = OS_ERROR;
        LOG_EXT_E("Module %s create netconn failed, type [%d] error", module->name, type);
        break;
    }

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s create %s netconn failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        e7025_unlock(&e7025->netconn_lock);
        return OS_NULL;
    }

    if (at_resp_get_data_by_line(&resp, 2, "%d", &conn_id) <= 0)
    {
        LOG_EXT_E("Module %s get %s netconn id failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        e7025_unlock(&e7025->netconn_lock);
        return OS_NULL;
    }

    if ((conn_id <= 0) || (conn_id > E7025_NETCONN_NUM))
    {
        LOG_EXT_E("Module %s get %s netconn id error", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        e7025_unlock(&e7025->netconn_lock);
        return OS_NULL;
    }

    result = os_data_queue_init(&netconn->data_queue, E7025_DATA_QUEUE_SIZE, 0, OS_NULL);
    if (result != OS_EOK)
    {
        LOG_EXT_E("%s data queue init failed, no enough memory.", module->name);
        e7025_unlock(&e7025->netconn_lock);
        return OS_NULL;
    }

    netconn->connect_id = conn_id;
    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    e7025_unlock(&e7025->netconn_lock);

    return netconn;
}

os_err_t e7025_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    LOG_EXT_I("Module %s in %d netconn status", module->name, netconn->stat);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+ECSOCL=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            LOG_EXT_E("Module %s destroy %s netconn failed",
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
        mo_netconn_data_queue_deinit(&netconn->data_queue);
    }

    LOG_EXT_I("Module %s netconn_id: %d destroyed", module->name, netconn->connect_id);

    netconn->connect_id  = -1;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);

    return result;
}

os_err_t e7025_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    at_parser_t *parser = &self->parser;

	char recvip[IPADDR_MAX_STR_LEN + 1] = {0};

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+ECDNS=\"%s\"", domain_name);
    if (result < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    /* AT+ECDNS="www.qq.com" return: +ECDNS: "121.14.77.201"\r\n\r\n OK\r\n */
    /* AT+ECDNS="8.8.8.8"    return: +ECDNS: "8.8.8.8"\r\n\r\n OK\r\n */
    if (at_resp_get_data_by_kw(&resp, "+ECDNS:", "+ECDNS: \"%[^\"]", recvip) <= 0)
    {
        LOG_EXT_E("Module %s domain resolve: resp parse fail, try again, host: %s", self->name, domain_name);
        result = OS_ERROR;
        /* If resolve failed, maybe receive an URC CRLF */
        goto __exit;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        LOG_EXT_E("Module %s domain resolve: recvip len < IPADDR_MIN_STR_LEN, len = %d", self->name, strlen(recvip));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        LOG_EXT_D("Module %s domain resolve: \"%s\" domain ip is %s, addrlen %d", self->name, domain_name, recvip, strlen(recvip));
        inet_aton(recvip, addr);

        if (IPADDR_ANY == addr->addr || IPADDR_LOOPBACK == addr->addr)
        {
            ip_addr_set_zero(addr);
            result = OS_ERROR;
            goto __exit;
        }

        result = OS_EOK;
    }

__exit:

    return result;
}

os_err_t e7025_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser, &resp, "AT+ECSOCO=%d,\"%s\",%u", netconn->connect_id, remote_ip, port);
        break;

    case NETCONN_TYPE_UDP:
        result = OS_EOK;
        break;

    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s connect to %s:%u failed!", module->name, remote_ip, port);
        return result;
    }

    ip_addr_copy(netconn->remote_ip, addr);
    netconn->remote_port = port;
    netconn->stat        = NETCONN_STAT_CONNECT;

    LOG_EXT_D("Module %s connect to %s:%u successfully!", module->name, remote_ip, port);

    return result;
}

static os_size_t e7025_tcp_udp_send(at_parser_t *parser, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    os_err_t   result       = OS_EOK;
    os_size_t  sent_size    = 0;
    os_size_t  cur_pkt_size = 0;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    char send_cmd[128]                     = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    strncpy(remote_ip, inet_ntoa(netconn->remote_ip), IPADDR_MAX_STR_LEN);

    /* Protect the E7025 data sending process, prevent other threads to send AT commands */
    at_parser_exec_lock(parser);

    while (sent_size < size)
    {
        if (netconn->stat != NETCONN_STAT_CONNECT)
        {
            ERROR("Module %s netconn %d isn't in the CONNECT state, send data fail", parser->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        /* Bytes to hex string(0x30 -> "30"), SEND_DATA_MAX_SIZE * 2 */
        if (size - sent_size < SEND_DATA_MAX_SIZE * 2)
        {
            cur_pkt_size = size - sent_size;
        }
        else
        {
            cur_pkt_size = SEND_DATA_MAX_SIZE * 2;
        }

        switch (netconn->type)
        {
        case NETCONN_TYPE_TCP:
            snprintf(send_cmd, sizeof(send_cmd), "AT+ECSOSD=%d,%d,", netconn->connect_id, (int)cur_pkt_size / 2);
            break;

        case NETCONN_TYPE_UDP:
            snprintf(send_cmd, sizeof(send_cmd), "AT+ECSOST=%d,\"%s\",%u,%d,",
                     netconn->connect_id,
                     remote_ip,
                     netconn->remote_port,
                     (int)cur_pkt_size / 2);
            break;

        default:
            LOG_EXT_E("Module %s send data failed, netconn type[%d] error", parser->name, netconn->type);
            return 0;
        }

        /* step1: send at command prefix and parameter */
        if (at_parser_send(parser, send_cmd, strlen(send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        /* step2: send data parameter */
        if (at_parser_send(parser, data + sent_size, cur_pkt_size) != cur_pkt_size)
        {
            result = OS_ERROR;
            ERROR("Drv or hardware send data fail, some data is missing.\r\n");
            goto __exit;
        }

        /* step3: send /r/n, enter the AT command execution process */
        result = at_parser_exec_cmd(parser, &resp, "");
        if (result != OS_EOK)
        {
            goto __exit;
        }

        sent_size += cur_pkt_size;
    }

__exit:

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        LOG_EXT_E("Module %s netconn %d send %d bytes data failed!",
                  parser->name,
                  netconn->connect_id,
                  cur_pkt_size);

        return 0;
    }

    return sent_size / 2;
}

os_size_t e7025_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_size_t    sent_size = 0;

    char *hexstr = calloc(1, size * 2 + 1);
    if (OS_NULL == hexstr)
    {
        LOG_EXT_E("Moudle %s netconn %d calloc %d bytes memory failed!",
                  module->name,
                  netconn->connect_id,
                  size * 2 + 1);
        return 0;
    }

    bytes_to_hexstr(data, hexstr, size);

    mo_e7025_t *e7025 = os_container_of(module, mo_e7025_t, parent);

    e7025_lock(&e7025->netconn_lock);

    sent_size = e7025_tcp_udp_send(parser, netconn, hexstr, size * 2);

    e7025_unlock(&e7025->netconn_lock);

    free(hexstr);

    return sent_size;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t err_code   = 0;

    sscanf(data, "+ECSOCLI: %d,%d", &connect_id, &err_code);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = e7025_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    LOG_EXT_W("Module %s receive close urc data of connect %d, error code: %d", module->name, connect_id, err_code);

    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    /* For ex: +ECSONMI: 1,10,"30313233343536373839" -- the actual data is "0-9"*/
    sscanf(data, "+ECSONMI: %d,%d,", &connect_id, &data_size);

    LOG_EXT_E("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = e7025_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        LOG_EXT_E("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        char *recv_buff = calloc(1, data_size * 2 + 1);
        if (recv_buff == OS_NULL)
        {
            LOG_EXT_E("Calloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
            return;
        }

        /* Get receive data to receive buffer */
        /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
        sscanf(data, "+ECSONMI: %d,%d,\"%[^\"]", &connect_id, &data_size, recv_buff);

        char *recv_str = calloc(1, data_size + 1);
        if (recv_str == OS_NULL)
        {
            LOG_EXT_E("Calloc recv str %d bytes fail, no enough memory", data_size + 1);
            free(recv_buff);
            return;
        }

        hexstr_to_bytes(recv_buff, recv_str, data_size * 2);
        mo_netconn_data_recv_notice(netconn, recv_str, data_size);

        free(recv_buff);
    }
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+ECSOCLI:", .suffix = "\r\n", .func = urc_close_func},
    {.prefix = "+ECSONMI:", .suffix = "\r\n", .func = urc_recv_func},
};

void e7025_netconn_init(mo_e7025_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < E7025_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* E7025_USING_NETCONN_OPS */
