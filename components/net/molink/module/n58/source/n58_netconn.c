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
 * @file        n58.c
 *
 * @brief       n58 module link kit netconn api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-30   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "n58_netconn.h"
#include "n58.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "n58.netconn"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define SEND_DATA_MAX_SIZE (1024)

#define N58_NETCONN_MQ_NAME "n58_nc_mq"

#ifndef N58_NETCONN_MQ_MSG_SIZE
#define N58_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef N58_NETCONN_MQ_MSG_MAX
#define N58_NETCONN_MQ_MSG_MAX  (10)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define N58_EVENT_CONN_OK   (1L << 0)
#define N58_EVENT_CONN_FAIL (1L << 1)
#define N58_EVENT_SEND_OK   (1L << 2)
#define N58_EVENT_RECV_OK   (1L << 3)
#define N58_EVNET_CLOSE_OK  (1L << 4)
#define N58_EVENT_SEND_FAIL (1L << 5)
#define N58_EVENT_DOMAIN_OK (1L << 6)

#ifdef N58_USING_NETCONN_OPS

static os_err_t n58_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t n58_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_bool_t n58_check_netconn_state(mo_object_t *module, os_int32_t connect_id)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    char netconn_stat[11]                 = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND, .line_num = 1};

    if (at_parser_exec_cmd(parser, &resp, "AT+IPSTATUS=%d", connect_id) != OS_EOK)
    {
        return OS_FALSE;
    }

    if (at_resp_get_data_by_kw(&resp, "+IPSTATUS:", "+IPSTATUS: %*d,%[^,]%*s", netconn_stat) <= 0)
    {
        ERROR("Check connecd id %d state failed!", connect_id);
        return OS_FALSE;
    }

    if (strncmp(netconn_stat, "DISCONNECT", strlen("DISCONNECT")) == 0)
    {
        return OS_TRUE;
    }
    else
    {
        return OS_FALSE;
    }
}

static mo_netconn_t *n58_netconn_alloc(mo_object_t *module)
{
    mo_n58_t *n58 = os_container_of(module, mo_n58_t, parent);

    for (int i = 0; i < N58_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == n58->netconn[i].stat)
        {
            if (n58_check_netconn_state(module, i))
            {
                n58->netconn[i].connect_id = i;

                return &n58->netconn[i];
            }
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *n58_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    mo_n58_t *n58 = os_container_of(module, mo_n58_t, parent);

    for (int i = 0; i < N58_NETCONN_NUM; i++)
    {
        if (connect_id == n58->netconn[i].connect_id)
        {
            return &n58->netconn[i];
        }
    }

    return OS_NULL;
}

os_err_t n58_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_n58_t *n58 = os_container_of(module, mo_n58_t, parent);

    info->netconn_array = n58->netconn;
    info->netconn_nums  = sizeof(n58->netconn) / sizeof(n58->netconn[0]);

    return OS_EOK;
}

static os_err_t n58_netconn_set_format(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    os_int32_t recv_mode = -1;
    os_int32_t data_mode = -1;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+RECVMODE?") != OS_EOK)
    {
        return OS_ERROR;
    }

    if (at_resp_get_data_by_kw(&resp, "+RECVMODE:", "+RECVMODE: %d,%d", &recv_mode, &data_mode) <= 0)
    {
        ERROR("Get module %s tcp/ip receive data format failed!", module->name);
        return OS_ERROR;
    }

    if (1 == recv_mode && 0 == data_mode)
    {
        return OS_EOK;
    }
    else
    {
        return at_parser_exec_cmd(parser, &resp, "AT+RECVMODE=1,0");
    }
}

mo_netconn_t *n58_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    if (n58_netconn_set_format(module) != OS_EOK)
    {
        return OS_NULL;
    }

    mo_n58_t *n58 = os_container_of(module, mo_n58_t, parent);

    n58_lock(&n58->netconn_lock);

    mo_netconn_t *netconn = n58_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        n58_unlock(&n58->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(N58_NETCONN_MQ_NAME, N58_NETCONN_MQ_MSG_SIZE, N58_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        n58_unlock(&n58->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    n58_unlock(&n58->netconn_lock);

    return netconn;
}

static os_err_t n58_netconn_do_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND, .line_num = 1};

    os_err_t result = OS_EOK;

    if (NETCONN_TYPE_TCP == netconn->type)
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+TCPCLOSE=%d", netconn->connect_id);
    }
    else /* NETCONN_TYPE_UDP == netconn->type */
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+UDPCLOSE=%d", netconn->connect_id);
    }

    if (result != OS_EOK)
    {
        ERROR("Module %s destroy %s netconn failed",
               module->name,
               (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        return result;
    }

    const char *source_line = at_resp_get_line(&resp, 1);
    if (OS_NULL == source_line)
    {
        ERROR("Module %s destroy %s netconn failed",
               module->name,
               (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        return OS_ERROR;
    }

    /* Reconfirm that the connection is closed */
    if ((strstr(source_line, "OK") != OS_NULL) || (n58_check_netconn_state(module, netconn->connect_id)))
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t n58_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    os_err_t result = OS_EOK;

    DEBUG("Module %s in %d netconn status", module->name, netconn->stat);

    mo_n58_t *n58  = os_container_of(module, mo_n58_t, parent);
    if (n58 == OS_NULL)
    {
        ERROR("Module %s destroy netconn failed, get netconn_lock failed", module->name);
        return OS_ERROR;
    }

    n58_lock(&n58->netconn_lock);

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;

    case NETCONN_STAT_CONNECT:
        result = n58_netconn_do_destroy(module, netconn);
        if (result != OS_EOK)
        {
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

    INFO("Module %s netconn id %d destroyed", module->name, netconn->connect_id);

    netconn->connect_id  = -1;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);

    n58_unlock(&n58->netconn_lock);

    return OS_EOK;
}

os_err_t n58_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    mo_n58_t    *n58    = os_container_of(module, mo_n58_t, parent);
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 60 * OS_TICK_PER_SECOND};

    n58_lock(&n58->netconn_lock);

    os_uint32_t event = SET_EVENT(netconn->connect_id, N58_EVENT_CONN_OK | N58_EVENT_CONN_FAIL);
    os_event_recv(&n58->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        /* AT+TCPSETUP=<n>,<ip>,<port> */
        result = at_parser_exec_cmd(parser, &resp, "AT+TCPSETUP=%d,%s,%d", netconn->connect_id, remote_ip, port);
        break;

    case NETCONN_TYPE_UDP:
        /* AT+UDPSETUP=<n>,<ip>,<port> */
        result = at_parser_exec_cmd(parser, &resp, "AT+UDPSETUP=%d,%s,%d", netconn->connect_id, remote_ip, port);
        break;

    default:
        ERROR("Module %s netconn id %d, netconn type error", module->name, netconn->connect_id);
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&n58->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait conect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&n58->netconn_evt,
                           N58_EVENT_CONN_OK | N58_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait connect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & N58_EVENT_CONN_FAIL)
    {
        result = OS_ERROR;
        ERROR("Module %s netconn id %d connect failed!", module->name, netconn->connect_id);
    }

__exit:

    n58_unlock(&n58->netconn_lock);

    if (OS_EOK == result)
    {
        ip_addr_copy(netconn->remote_ip, addr);
        netconn->remote_port = port;
        netconn->stat        = NETCONN_STAT_CONNECT;
        DEBUG("Module %s connect to %s:%d successfully!", module->name, remote_ip, port);
    }
    else
    {
        ERROR("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
    }

    return result;
}

os_size_t n58_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    char resp_buff [AT_RESP_BUFF_SIZE_128] = {0};
    char cmd_format[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 12 * OS_TICK_PER_SECOND};
    mo_n58_t *n58  = os_container_of(module, mo_n58_t, parent);

    /* Protect the data sending process, prevent other threads to send AT commands */
    at_parser_exec_lock(parser);

    n58->curr_connect = netconn->connect_id;

    at_parser_set_end_mark(parser, ">", 1);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        /* AT+TCPSEND=<n>,<length> */
        strncpy(cmd_format, "AT+TCPSEND=%d,%d", 16);
        break;

    case NETCONN_TYPE_UDP:
        /* AT+UDPSETUP=<n>,<length>  */
        strncpy(cmd_format, "AT+UDPSEND=%d,%d", 16);
        break;

    default:
        ERROR("Module %s netconn id %d, netconn type error", module->name, netconn->connect_id);
        result = OS_ERROR;
        goto __exit;
    }

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
            curr_size = size - sent_size;
        }
        else
        {
            curr_size = SEND_DATA_MAX_SIZE;
        }

        result = at_parser_exec_cmd(parser, &resp, cmd_format, netconn->connect_id, curr_size);
        if (result != OS_EOK)
        {
            goto __exit;
        }

        os_task_msleep(100);

        if (at_parser_send(parser, data + sent_size, curr_size) != curr_size)
        {
            result = OS_ERROR;
            ERROR("Drv or hardware send data fail, some data is missing.\r\n");
            goto __exit;
        }

        result = os_event_recv(&n58->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               30 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&n58->netconn_evt,
                               N58_EVENT_SEND_OK | N58_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & N58_EVENT_SEND_FAIL)
        {
            ERROR("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += curr_size;
        os_task_msleep(10);
    }

__exit:

    at_parser_set_end_mark(parser, OS_NULL, 0);
    n58->curr_connect = -1;

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module    = os_container_of(parser, mo_object_t, parser);
    os_int32_t  connect_id = 0;

    sscanf(data, "+TCPCLOSE: %d,&*s", &connect_id);

    mo_netconn_t *netconn = n58_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    WARN("Module %s receive close urc data of connect %d", module->name, connect_id);
    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module    = os_container_of(parser, mo_object_t, parser);
    mo_n58_t    *n58       = os_container_of(module, mo_n58_t, parent);
    os_int32_t  connect_id = 0;

    sscanf(data, "%*[^:]: %d,%*s", &connect_id);

    mo_netconn_t *netconn = n58_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    if (strstr(data, "OK") != OS_NULL)
    {
        os_event_send(&n58->netconn_evt, SET_EVENT(connect_id, N58_EVENT_CONN_OK));
    }
    else
    {
        os_event_send(&n58->netconn_evt, SET_EVENT(connect_id, N58_EVENT_CONN_FAIL));
    }

    return;
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module    = os_container_of(parser, mo_object_t, parser);
    mo_n58_t    *n58       = os_container_of(module, mo_n58_t, parent);
    os_int32_t  connect_id = 0;

    sscanf(data, "%*[^:]: %d,%*s", &connect_id);

    mo_netconn_t *netconn = n58_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    if (strstr(data, "ERROR") != OS_NULL || strstr(data, "FAILED") != OS_NULL || strstr(data, "EXPIRED") != OS_NULL)
    {
        os_event_send(&n58->netconn_evt, SET_EVENT(connect_id, N58_EVENT_SEND_FAIL));
    }
    else
    {
        os_event_send(&n58->netconn_evt, SET_EVENT(connect_id, N58_EVENT_SEND_OK));
    }

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    char tmp_ch;  /* Get connecnt id */

    /* handle the blank space */
    at_parser_recv(parser, &tmp_ch, 1, 0);
    at_parser_recv(parser, &tmp_ch, 1, 0);

    os_int32_t connect_id = tmp_ch - '0';

    mo_netconn_t *netconn = n58_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    /* handle ',' */
    at_parser_recv(parser, &tmp_ch, 1, 0);

    /* Get data size */
    char tmp_str[5] = {0};

    for (int i = 0;; i++)
    {
        at_parser_recv(parser, &tmp_ch, 1, 0);
        if (',' == tmp_ch)
        {
            break;
        }

        tmp_str[i] = tmp_ch;
    }

    os_size_t data_size = atoi(tmp_str);
    os_int32_t timeout  = data_size > 10 ? data_size : 10;

    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, netconn->connect_id, data_size);

    char *recv_buff    = os_calloc(1, data_size);
    char  temp_buff[8] = {0};
    if (recv_buff == OS_NULL)
    {
        /* read and clean the coming data */
        ERROR("Calloc recv buff %d bytes fail, no enough memory", data_size);
        os_size_t temp_size = 0;
        while (temp_size < data_size)
        {
            if (data_size - temp_size > sizeof(temp_buff))
            {
                at_parser_recv(parser, temp_buff, sizeof(temp_buff), timeout);
            }
            else
            {
                at_parser_recv(parser, temp_buff, data_size - temp_size, timeout);
            }
            temp_size += sizeof(temp_buff);
        }

        /* handle "\r\n" */
        at_parser_recv(parser, temp_buff, 2, timeout);

        return;
    }

    if (at_parser_recv(parser, recv_buff, data_size, timeout) != data_size)
    {
        ERROR("Module %s netconnt id %d recv %d bytes data failed!", module->name, netconn->connect_id, data_size);
        os_free(recv_buff);

        return;
    }

    /* handle "\r\n" */
    at_parser_recv(parser, temp_buff, 2, timeout);

    mo_netconn_data_recv_notice(netconn, recv_buff, data_size);

    return;
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+TCPSEND",   .suffix = "\r\n",       .func = urc_send_func},
    {.prefix = "+UDPSEND",   .suffix = "\r\n",       .func = urc_send_func},
    {.prefix = "+TCPSETUP:", .suffix = "\r\n",       .func = urc_connect_func},
    {.prefix = "+UDPSETUP:", .suffix = "\r\n",       .func = urc_connect_func},
    {.prefix = "+TCPCLOSE:", .suffix = "Closed\r\n", .func = urc_close_func},
    {.prefix = "",           .suffix = "+TCPRECV:",  .func = urc_recv_func},
    {.prefix = "",           .suffix = "+UDPRECV:",  .func = urc_recv_func},
};

void n58_netconn_init(mo_n58_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < N58_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return;
}

#endif /* N58_USING_NETCONN_OPS */
