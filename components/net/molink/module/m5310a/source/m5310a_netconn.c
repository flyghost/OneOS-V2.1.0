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
 * @file        m5310a_netconn.c
 *
 * @brief       m5310-a module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5310a_netconn.h"
#include "m5310a.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "m5310a.netconn"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define PROTOCOL_TYPE_TCP (6)
#define PROTOCOL_TYPE_UDP (17)

#define SEND_DATA_MAX_SIZE              (1358)
#define M5310A_SEND_HEXDATA_BLOCK_SIZE  (256)

#define M5310A_NETCONN_MQ_NAME "m5310a_nc_mq"

#ifndef M5310A_NETCONN_MQ_MSG_SIZE
#define M5310A_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef M5310A_NETCONN_MQ_MSG_MAX
#define M5310A_NETCONN_MQ_MSG_MAX  (8)
#endif

#ifdef M5310A_USING_NETCONN_OPS

static os_err_t m5310a_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t m5310a_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *m5310a_netconn_alloc(mo_object_t *module)
{
    mo_m5310a_t *m5310a = os_container_of(module, mo_m5310a_t, parent);

    for (int i = 0; i < M5310A_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == m5310a->netconn[i].stat)
        {
            return &m5310a->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *m5310a_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || M5310A_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_m5310a_t *m5310a = os_container_of(module, mo_m5310a_t, parent);

    for (int i = 0; i < M5310A_NETCONN_NUM; i++)
    {
        if (connect_id == m5310a->netconn[i].connect_id)
        {
            return &m5310a->netconn[i];
        }
    }

    return OS_NULL;
}

os_err_t m5310a_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_m5310a_t *m5310a = os_container_of(module, mo_m5310a_t, parent);

    info->netconn_array = m5310a->netconn;
    info->netconn_nums  = sizeof(m5310a->netconn) / sizeof(m5310a->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *m5310a_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_m5310a_t *m5310a = os_container_of(module, mo_m5310a_t, parent);
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    m5310a_lock(&m5310a->netconn_lock);

    mo_netconn_t *netconn = m5310a_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        m5310a_unlock(&m5310a->netconn_lock);
        return OS_NULL;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    switch (type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCR=\"STREAM\",%d,0,2", PROTOCOL_TYPE_TCP);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser, &resp, "AT+NSOCR=\"DGRAM\",%d,0,2", PROTOCOL_TYPE_UDP);
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        ERROR("Module %s create %s netconn failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        m5310a_unlock(&m5310a->netconn_lock);
        return OS_NULL;
    }

    if (at_resp_get_data_by_line(&resp, 1, "%d", &netconn->connect_id) <= 0)
    {
        ERROR("Module %s get %s netconn id failed", module->name, (type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        m5310a_unlock(&m5310a->netconn_lock);
        return OS_NULL;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+NSOCFG=%d,1,1", netconn->connect_id);
    if (OS_EOK != result)
    {
        ERROR("Module %s set netconn %d data format failed", module->name, netconn->connect_id);
        m5310a_unlock(&m5310a->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(M5310A_NETCONN_MQ_NAME, M5310A_NETCONN_MQ_MSG_SIZE, M5310A_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        m5310a_unlock(&m5310a->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    m5310a_unlock(&m5310a->netconn_lock);

    return netconn;
}

os_err_t m5310a_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    INFO("Module %s in %d netconn status", module->name, netconn->stat);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

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

    INFO("Module %s netconn_id:%d destroyed", module->name, netconn->connect_id);

    netconn->connect_id  = -1;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);

    return OS_EOK;
}

os_err_t m5310a_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    at_parser_t *parser = &self->parser;

    char recvip[IPADDR_MAX_STR_LEN + 1] = {0};

    char resp_buff[AT_RESP_BUFF_SIZE_256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 2,
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CMDNS=\"%s\"", domain_name);
    if (result < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    /* AT+CMDNS="www.baidu.com" return: OK \r\n  +CMDNS:183.232.231.172 \r\n */
    /* AT+CMDNS="8.8.8.8" return: +CMDNS:8.8.8.8 \r\n  OK */
    if (at_resp_get_data_by_kw(&resp, "+CMDNS:", "+CMDNS:%s", recvip) <= 0)
    {
        ERROR("M5310-A domain resolve: resp parse fail, try again, host: %s", domain_name);
        result = OS_ERROR;
        /* If resolve failed, maybe receive an URC CRLF */
        goto __exit;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        ERROR("M5310-A domain resolve: recvip len < IPADDR_MIN_STR_LEN, len = %d", strlen(recvip));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("M5310-A domain resolve: \"%s\" domain ip is %s, addrlen %d", domain_name, recvip, strlen(recvip));
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

static os_err_t m5310a_tcp_connect(at_parser_t *parser, os_int32_t connect_id, char *ip_addr, os_uint16_t port)
{
    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 2,
                      .timeout   = 40 * OS_TICK_PER_SECOND};

    char buf[16] = {0};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+NSOCO=%d,%s,%d", connect_id, ip_addr, port);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "CONNECT", "CONNECT %s", buf) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    if (strcmp(buf, "OK"))
    {
        INFO("Module connect[%d]:%s!", connect_id, buf);
        result = OS_ERROR;
        goto __exit;
    }

__exit:

    return result;
}

os_err_t m5310a_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = m5310a_tcp_connect(parser, netconn->connect_id, remote_ip, port);
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
        ERROR("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
        return result;
    }

    ip_addr_copy(netconn->remote_ip, addr);
    netconn->remote_port = port;
    netconn->stat        = NETCONN_STAT_CONNECT;

    DEBUG("Module %s connect to %s:%d successfully!", module->name, remote_ip, port);

    return OS_EOK;
}

static os_size_t m5310a_one_hexdata_block_send(at_parser_t *parser, const char *data, os_size_t size)
{
    os_size_t  sent_size     = 0; /* raw data sent size */
    os_size_t  curr_pkt_size = 0; /* raw data current packet size */

    char hex_str_buff[M5310A_SEND_HEXDATA_BLOCK_SIZE + 1] = {0};

    while (size > sent_size)
    {
        if ((size - sent_size) < (M5310A_SEND_HEXDATA_BLOCK_SIZE / 2))
        {
            curr_pkt_size = size - sent_size;
        }
        else
        {
            curr_pkt_size = M5310A_SEND_HEXDATA_BLOCK_SIZE / 2;
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

os_size_t m5310a_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser      = &module->parser;
    os_err_t    result       = OS_EOK;
    os_size_t   sent_size    = 0;
    os_size_t   cur_pkt_size = 0;
    os_int32_t  connect_id   = -1;
    os_size_t   cnt          = 0;

    char send_cmd [AT_RESP_BUFF_SIZE_DEF]  = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    if (netconn->type == NETCONN_TYPE_UDP)
    {
        strncpy(remote_ip, inet_ntoa(netconn->remote_ip), IPADDR_MAX_STR_LEN);
    }

    /* Protect the M5310A data sending process, prevent other threads to send AT commands */
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

        switch (netconn->type)
        {
        case NETCONN_TYPE_TCP:
            snprintf(send_cmd, sizeof(send_cmd), "AT+NSOSD=%d,%d,", netconn->connect_id, (int)cur_pkt_size);
            break;

        case NETCONN_TYPE_UDP:
            snprintf(send_cmd,
                     sizeof(send_cmd),
                     "AT+NSOST=%d,%s,%u,%d,",
                     netconn->connect_id,
                     remote_ip,
                     netconn->remote_port,
                     (int)cur_pkt_size);
            break;

        default:
            ERROR("Module %s netconn %d type error!", parser->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        /* step1: send at command prefix and parameter */
        if (at_parser_send(parser, send_cmd, strlen(send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        /* step2: send data context */
        if (cur_pkt_size != m5310a_one_hexdata_block_send(parser, data + sent_size, cur_pkt_size))
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

        result = OS_ERROR;
        for (int i = 1; i <= resp.line_counts; i++)
        {
            if ((at_resp_get_data_by_line(&resp, i, "%d,%d", &connect_id, &cnt) > 0) && (cnt == cur_pkt_size))
            {
                result = OS_EOK;
                break;
            }
        }

        if (result == OS_ERROR)
        {
            goto __exit;
        }

        sent_size += cur_pkt_size;
        os_task_msleep(50);
    }

__exit:

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s netconn %d send %d bytes data failed!", parser->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "+NSOCLI: %d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = m5310a_get_netconn_by_id(module, connect_id);
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

    sscanf(data, "+NSORF:%d,%*[^,],%*d,%d,", &connect_id, &data_size);

    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = m5310a_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    if (netconn->stat == NETCONN_STAT_CONNECT)
    {
        /* bufflen >= strsize + 1 */
        char *recv_buff = os_calloc(1, data_size * 2 + 1);
        if (recv_buff == OS_NULL)
        {
            ERROR("Calloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
            return;
        }

        /* Get receive data to receive buffer */
        /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
        sscanf(data, "+NSORF:%*d,%*[^,],%*d,%*d,%[^,]", recv_buff);

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

static at_urc_t gs_urc_table[] = {
    {.prefix = "+NSOCLI:", .suffix = "\r\n", .func = urc_close_func},
    {.prefix = "+NSORF:",  .suffix = "\r\n", .func = urc_recv_func},
};

void m5310a_netconn_init(mo_m5310a_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < M5310A_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return;
}

#endif /* M5310A_USING_NETCONN_OPS */
