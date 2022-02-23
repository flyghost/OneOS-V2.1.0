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
 * @file        mb26_netconn.c
 *
 * @brief       mb26 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mb26_netconn.h"
#include "mb26.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "mb26.netconn"
#define MO_LOG_LVL  MO_LOG_DEBUG
#include "mo_log.h"

#define PROTOCOL_TYPE_TCP (6)
#define PROTOCOL_TYPE_UDP (17)

#define SEND_DATA_MAX_SIZE            (512)
#define MB26_SEND_HEXDATA_BLOCK_SIZE  (256)

#define MB26_NETCONN_MQ_NAME     "mb26_nc_mq"
#ifndef MB26_NETCONN_MQ_MSG_SIZE
#define MB26_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif /* MB26_NETCONN_MQ_MSG_SIZE */
#ifndef MB26_NETCONN_MQ_MSG_MAX
#define MB26_NETCONN_MQ_MSG_MAX  (5)
#endif /* MB26_NETCONN_MQ_MSG_MAX */

enum mb26_socket_type
{
    TCP_SOCKET_TYPE = 1,
    UDP_SOCKET_TYPE = 2,
};

#ifdef MB26_USING_NETCONN_OPS

static os_err_t mb26_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t mb26_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *mb26_netconn_alloc(mo_object_t *module)
{
    mo_mb26_t *mb26 = os_container_of(module, mo_mb26_t, parent);

    for (int i = 0; i < MB26_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == mb26->netconn[i].stat)
        {
            return &mb26->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *mb26_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    /* MB26 connect id range is 1-5 */
    if ((connect_id <= 0) || (connect_id > MB26_NETCONN_NUM))
    {
        ERROR("Module %s get netconn by id failed, INVALID connet_id: %d", module->name, connect_id);
        return OS_NULL;
    }

    mo_mb26_t *mb26 = os_container_of(module, mo_mb26_t, parent);

    for (int i = 0; i < MB26_NETCONN_NUM; i++)
    {
        if (connect_id == mb26->netconn[i].connect_id)
        {
            return &mb26->netconn[i];
        }
    }

    return OS_NULL;
}

os_err_t mb26_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_mb26_t *mb26 = os_container_of(module, mo_mb26_t, parent);

    info->netconn_array = mb26->netconn;
    info->netconn_nums  = sizeof(mb26->netconn) / sizeof(mb26->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *mb26_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_mb26_t   *mb26    = os_container_of(module, mo_mb26_t, parent);
    at_parser_t *parser  = &module->parser;
    os_err_t     result  = OS_EOK;
    os_int32_t   conn_id = -1;

    mb26_lock(&mb26->netconn_lock);

    mo_netconn_t *netconn = mb26_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        mb26_unlock(&mb26->netconn_lock);
        return OS_NULL;
    }

    char resp_buff[64] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 8 * OS_TICK_PER_SECOND};

    switch (type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser, &resp, "AT+SKTCREATE=1,%d,%d", TCP_SOCKET_TYPE, PROTOCOL_TYPE_TCP);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser, &resp, "AT+SKTCREATE=1,%d,%d", UDP_SOCKET_TYPE, PROTOCOL_TYPE_UDP);
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        ERROR("Module %s create %s netconn failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        mb26_unlock(&mb26->netconn_lock);
        return OS_NULL;
    }

    if (at_resp_get_data_by_kw(&resp, "+SKTCREATE:", "+SKTCREATE: %d", &conn_id) <= 0)
    {
        ERROR("Module %s get %s netconn id failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        mb26_unlock(&mb26->netconn_lock);
        return OS_NULL;
    }

    /* The MB26 connect id range is 1-5 */
    if ((conn_id > 0) && (conn_id <= MB26_NETCONN_NUM))
    {
        netconn->connect_id = conn_id;
    }
    else
    {
        ERROR("Module %s parse netconn id failed, id[%d] error", module->name, conn_id);
        mb26_unlock(&mb26->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(MB26_NETCONN_MQ_NAME,
                               MB26_NETCONN_MQ_MSG_SIZE,
                               MB26_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s message queue create failed, no enough memory.", module->name);
        mb26_unlock(&mb26->netconn_lock);
        netconn->connect_id = -1;
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    mb26_unlock(&mb26->netconn_lock);

    return netconn;
}

os_err_t mb26_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    INFO("Module %s in %d netconn status", module->name, netconn->stat);

    char resp_buff[64] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 8 * OS_TICK_PER_SECOND};

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+SKTDELETE=%d", netconn->connect_id);
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

    INFO("Module %s netconn_id: %d destroyed", module->name, netconn->connect_id);

    netconn->connect_id  = -1;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);

    return OS_EOK;
}

os_err_t mb26_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
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

    /* AT+ECDNS="www.baidu.com" return: \r\n+ECDNS: "14.215.177.39"\r\nOK\r\n */
    if (at_resp_get_data_by_kw(&resp, "+ECDNS:", "+ECDNS: \"%[^\"]", recvip) <= 0)
    {
        ERROR("Module %s domain resolve: resp parse fail, try again, host: %s", self->name, domain_name);
        result = OS_ERROR;
        /* If resolve failed, maybe receive an URC CRLF */
        goto __exit;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        ERROR("Module %s domain resolve: recvip len < IPADDR_MIN_STR_LEN, len = %d", self->name, strlen(recvip));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("Module %s domain resolve: \"%s\" domain ip is %s, addrlen %d",
                   self->name,
                   domain_name,
                   recvip,
                   strlen(recvip));

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

os_err_t mb26_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);
    result = at_parser_exec_cmd(parser, &resp, "AT+SKTCONNECT=%d,%s,%d", netconn->connect_id, remote_ip, port);
    if (result != OS_EOK)
    {
        ERROR("Module %s connection[%d] connect to %s:%d fail", module->name, netconn->connect_id, remote_ip, port);
        return result;
    }

    ip_addr_copy(netconn->remote_ip, addr);
    netconn->remote_port = port;
    netconn->stat        = NETCONN_STAT_CONNECT;

    DEBUG("Module %s connection[%d] connect to %s: %d success", module->name, netconn->connect_id, remote_ip, port);

    return OS_EOK;
}

static os_size_t mb26_one_hexdata_block_send(at_parser_t *parser, const char *data, os_size_t size)
{
    os_size_t  sent_size     = 0; /* raw data sent size */
    os_size_t  curr_pkt_size = 0; /* raw data current packet size */

    char hex_str_buff[MB26_SEND_HEXDATA_BLOCK_SIZE + 1] = {0};

    while (size > sent_size)
    {
        if ((size - sent_size) < (MB26_SEND_HEXDATA_BLOCK_SIZE / 2))
        {
            curr_pkt_size = size - sent_size;
        }
        else
        {
            curr_pkt_size = MB26_SEND_HEXDATA_BLOCK_SIZE / 2;
        }

        bytes_to_hexstr(data + sent_size, hex_str_buff, curr_pkt_size);

        if (at_parser_send(parser, hex_str_buff, curr_pkt_size * 2) <= 0)
        {
            goto __exit;
        }

        sent_size += curr_pkt_size;
    }

__exit:

    return sent_size;
}

os_size_t mb26_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser      = &module->parser;
    os_err_t    result       = OS_EOK;
    os_size_t   sent_size    = 0;
    os_size_t   cur_pkt_size = 0;

    char send_cmd [AT_RESP_BUFF_SIZE_DEF] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    /* Protect the data sending process, prevent other threads to send AT commands */
    at_parser_exec_lock(parser);

    while (sent_size < size)
    {
        if (size - sent_size < SEND_DATA_MAX_SIZE)
        {
            cur_pkt_size = size - sent_size;
        }
        else
        {
            cur_pkt_size = SEND_DATA_MAX_SIZE;
        }

        snprintf(send_cmd, sizeof(send_cmd), "AT+SKTSEND=%d,%d,", netconn->connect_id, (int)cur_pkt_size);

        /* step1: send at command prefix and parameter */
        if (at_parser_send(parser, send_cmd, strlen(send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        /* step2: send data parameter */
        if (cur_pkt_size != mb26_one_hexdata_block_send(parser, data + sent_size, cur_pkt_size))
        {
            result = OS_ERROR;
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
        ERROR("Module %s netconn %d send %d bytes data failed!",
                  parser->name,
                  netconn->connect_id,
                  size);
        return 0;
    }

    return sent_size;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = -1;
    os_int32_t err_code   = -1;

    /* URC msg: +SKTERR: fd,<errno> */
    sscanf(data, "+SKTERR: %d,%d", &connect_id,&err_code);

    if((connect_id <= 0) || (connect_id > MB26_NETCONN_NUM))
    {
        ERROR("Module %s receive error close urc data of connect %d", parser->name, connect_id);
        return;
    }

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = mb26_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s connect[%d] netconn is NULL", module->name, connect_id);
        return;
    }

    WARN("Module %s receive close urc data of connect %d, err_code: %d", module->name, connect_id, err_code);

    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    /* +SKTRECV: 1,10,"30313233343536373839" */
    sscanf(data, "+SKTRECV: %d,%d,", &connect_id, &data_size);

    if((connect_id <= 0) || (connect_id > MB26_NETCONN_NUM))
    {
        ERROR("Module %s receive error recv urc data of connect %d", parser->name, connect_id);
        return;
    }

    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);
    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = mb26_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s connect[%d] netconn is NULL", module->name, connect_id);
        return;
    }

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        /* It actually takes double the memory to receive and process, bufflen >= strsize * 2 + 1 */
        char *recv_buff = os_calloc(1, data_size * 2 + 1);
        if (recv_buff == OS_NULL)
        {
            ERROR("Calloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
            return;
        }

        /* Get receive data to receive buffer */
        /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
        sscanf(data, "+SKTRECV: %*d,%*d,\"%[^\"]", recv_buff);

        char *recv_str = os_calloc(1, data_size + 1);
        if (recv_str == OS_NULL)
        {
            ERROR("Calloc recv str %d bytes fail, no enough memory", data_size + 1);
            return;
        }

        /* Hex char "303132333435" -->  char "012345" */
        hexstr_to_bytes(recv_buff, recv_str, data_size * 2);
        mo_netconn_data_recv_notice(netconn, recv_str, data_size);

        os_free(recv_buff);
    }

    return;
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+SKTERR:",  .suffix = "\r\n", .func = urc_close_func},
    {.prefix = "+SKTRECV:", .suffix = "\r\n", .func = urc_recv_func},
};

void mb26_netconn_init(mo_mb26_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < MB26_NETCONN_NUM + 1; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return;
}

#endif /* MB26_USING_NETCONN_OPS */
