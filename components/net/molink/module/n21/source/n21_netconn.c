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
 * @file        n21.c
 *
 * @brief       n21 module link kit netconn api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "n21_netconn.h"
#include "n21.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "n21_netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"


#define TCP_SEND_DATA_MAX_SIZE (4096)
#define UDP_SEND_DATA_MAX_SIZE (1024)

#define N21_NETCONN_MQ_NAME "n21_nc_mq"

#ifndef N21_NETCONN_MQ_MSG_SIZE
#define N21_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef N21_NETCONN_MQ_MSG_MAX
#define N21_NETCONN_MQ_MSG_MAX  (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define N21_EVENT_CONN_OK   (1L << 0)
#define N21_EVENT_CONN_FAIL (1L << 1)
#define N21_EVENT_SEND_OK   (1L << 2)
#define N21_EVENT_RECV_OK   (1L << 3)
#define N21_EVNET_CLOSE_OK  (1L << 4)
#define N21_EVENT_SEND_FAIL (1L << 5)
#define N21_EVENT_DOMAIN_OK (1L << 6)

#ifdef N21_USING_NETCONN_OPS

static os_err_t n21_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t n21_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_bool_t n21_check_netconn_state(mo_object_t *module, os_int32_t connect_id)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+IPSTATUS=%d", connect_id) != OS_EOK)
    {
        return OS_FALSE;
    }

    char netconn_stat[11] = {0};

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

static mo_netconn_t *n21_netconn_alloc(mo_object_t *module)
{
    mo_n21_t *n21 = os_container_of(module, mo_n21_t, parent);

    for (int i = 0; i < N21_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == n21->netconn[i].stat)
        {
            if (n21_check_netconn_state(module, i))
            {
                n21->netconn[i].connect_id = i;

                return &n21->netconn[i];
            }
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *n21_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || N21_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_n21_t *n21 = os_container_of(module, mo_n21_t, parent);

    for (int i = 0; i < N21_NETCONN_NUM; i++)
    {
        if (connect_id == n21->netconn[i].connect_id)
        {
            return &n21->netconn[i];
        }
    }

    return OS_NULL;
}

os_err_t n21_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_n21_t *n21 = os_container_of(module, mo_n21_t, parent);

    info->netconn_array = n21->netconn;
    info->netconn_nums  = sizeof(n21->netconn) / sizeof(n21->netconn[0]);

    return OS_EOK;
}

static os_err_t n21_netconn_set_format(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+RECVMODE?") != OS_EOK)
    {
        return OS_ERROR;
    }

    os_int32_t recv_mode = 0;
    os_int32_t data_mode = 0;

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

mo_netconn_t *n21_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    if (n21_netconn_set_format(module) != OS_EOK)
    {
        return OS_NULL;
    }

    mo_n21_t *n21 = os_container_of(module, mo_n21_t, parent);

    n21_lock(&n21->netconn_lock);

    mo_netconn_t *netconn = n21_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        n21_unlock(&n21->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(N21_NETCONN_MQ_NAME,
                               N21_NETCONN_MQ_MSG_SIZE,
                               N21_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        n21_unlock(&n21->netconn_lock);
        return OS_NULL;
    }
    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    n21_unlock(&n21->netconn_lock);

    return netconn;
}

static os_err_t n21_netconn_do_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF,
                      .line_num  = 1};

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

    if (strstr(source_line, "OK") != OS_NULL ||
        n21_check_netconn_state(module, netconn->connect_id)) /* Reconfirm that the connection is closed */

    {
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t n21_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    os_err_t result = OS_EOK;

    DEBUG("Module %s in %d netconn status", module->name, netconn->stat);

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;
    case NETCONN_STAT_CONNECT:
        result = n21_netconn_do_destroy(module, netconn);
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

    return OS_EOK;
}

os_err_t n21_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;

    mo_n21_t *n21 = os_container_of(module, mo_n21_t, parent);

    n21_lock(&n21->netconn_lock);

    os_uint32_t event = SET_EVENT(netconn->connect_id, N21_EVENT_CONN_OK | N21_EVENT_CONN_FAIL);

    os_event_recv(&n21->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 60 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_err_t result = OS_EOK;
    /* AT+UDPSETUP=<n>,<ip>,<port> */

    if (NETCONN_TYPE_TCP == netconn->type)
    {   /* AT+TCPSETUP=<n>,<ip>,<port> */
        result = at_parser_exec_cmd(parser, &resp, "AT+TCPSETUP=%d,%s,%d", netconn->connect_id, remote_ip, port);
    }
    else /* NETCONN_TYPE_UDP == netconn->type */
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+UDPSETUP=%d,%s,%d", netconn->connect_id, remote_ip, port);
    }

    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&n21->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait conect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&n21->netconn_evt,
                           N21_EVENT_CONN_OK | N21_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait connect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & N21_EVENT_CONN_FAIL)
    {
        result = OS_ERROR;
        ERROR("Module %s netconn id %d connect failed!", module->name, netconn->connect_id);
    }

__exit:
    n21_unlock(&n21->netconn_lock);

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

os_size_t n21_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    mo_n21_t *n21 = os_container_of(module, mo_n21_t, parent);

    at_parser_exec_lock(parser);

    n21->curr_connect = netconn->connect_id;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 12 * OS_TICK_PER_SECOND};

    at_parser_set_end_mark(parser, ">", 1);

    os_size_t sent_max_size  = 0;
    char      cmd_format[17] = {0};
    if (NETCONN_TYPE_TCP == netconn->type)
    {
        /* AT+TCPSEND=<n>,<length> */
        strncpy(cmd_format, "AT+TCPSEND=%d,%d", 16);
        sent_max_size = TCP_SEND_DATA_MAX_SIZE;
    }
    else
    {
        /* AT+UDPSETUP=<n>,<ip>,<port> */
        strncpy(cmd_format, "AT+UDPSEND=%d,%d", 16);
        sent_max_size = UDP_SEND_DATA_MAX_SIZE;
    }

    while (sent_size < size)
    {
        if (netconn->stat != NETCONN_STAT_CONNECT)
        {
            ERROR("Module %s netconn %d isn't in the CONNECT state, send data fail", parser->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        if (size - sent_size < sent_max_size)
        {
            curr_size = size - sent_size;
        }
        else
        {
            curr_size = sent_max_size;
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

        result = os_event_recv(&n21->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               30 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&n21->netconn_evt,
                               N21_EVENT_SEND_OK | N21_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & N21_EVENT_SEND_FAIL)
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

    at_parser_exec_unlock(parser);
    if (result != OS_EOK)
    {
        ERROR("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

os_err_t n21_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 15 * OS_TICK_PER_SECOND,
                      .line_num  = 2};

    result = at_parser_exec_cmd(parser, &resp, "AT+CDNSGIP=\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        return result;
    }

    os_int32_t resolve_result = 0;

    if (at_resp_get_data_by_kw(&resp, "+CDNSGIP:", "+CDNSGIP: %d", &resolve_result) <= 0)
    {
        ERROR("Module %s get host by name %s failed!", self->name, domain_name);
        return OS_ERROR;
    }

    if (3 == resolve_result || 8 == resolve_result)
    {
        ERROR("Module %s get host by name %s failed!", self->name, domain_name);
        return OS_ERROR;
    }

    char recvip[IPADDR_MAX_STR_LEN + 1] = {0};

    if (at_resp_get_data_by_kw(&resp, "+CDNSGIP:", "%*[^,],%*[^,],\"%[^\"]", recvip) <= 0)
    {
        ERROR("Module %s gethostbyname %s failed!", self->name, domain_name);
        return OS_ERROR;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        ERROR("Module %s domain resolve: recvip len < IPADDR_MIN_STR_LEN, len = %d", self->name, strlen(recvip));
        return OS_ERROR;
    }
    else
    {
        DEBUG("Module %s domain resolve: \"%s\" domain ip is %s, addrlen %d",
                  domain_name,
                  recvip,
                  strlen(recvip));
        inet_aton(recvip, addr);

        if (IPADDR_ANY == addr->addr || IPADDR_LOOPBACK == addr->addr)
        {
            ip_addr_set_zero(addr);
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    os_int32_t connect_id = 0;

    sscanf(data, "+TCPCLOSE: %d,&*s", &connect_id);

    mo_netconn_t *netconn = n21_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    WARN("Module %s receive close urc data of connect %d", module->name, connect_id);
    mo_netconn_pasv_close_notice(netconn);
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_n21_t    *n21    = os_container_of(module, mo_n21_t, parent);

    os_int32_t connect_id = 0;

    sscanf(data, "%*[^:]: %d,%*s", &connect_id);

    mo_netconn_t *netconn = n21_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    if (strstr(data, "OK") != OS_NULL)
    {
        os_event_send(&n21->netconn_evt, SET_EVENT(connect_id, N21_EVENT_CONN_OK));
    }
    else
    {
        os_event_send(&n21->netconn_evt, SET_EVENT(connect_id, N21_EVENT_CONN_FAIL));
    }

    return;
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_n21_t   *n21   = os_container_of(module, mo_n21_t, parent);

    os_int32_t connect_id = 0;

    sscanf(data, "%*[^:]: %d,%*s", &connect_id);

    mo_netconn_t *netconn = n21_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    if (strstr(data, "ERROR") != OS_NULL || strstr(data, "FAILED") != OS_NULL || strstr(data, "EXPIRED") != OS_NULL)
    {
        os_event_send(&n21->netconn_evt, SET_EVENT(connect_id, N21_EVENT_SEND_FAIL));
    }
    else
    {
        os_event_send(&n21->netconn_evt, SET_EVENT(connect_id, N21_EVENT_SEND_OK));
    }
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    /* Get connecnt id */
    char tmp_ch;

    /* handle ' ' */
    at_parser_recv(parser, &tmp_ch, 1, 0);
    at_parser_recv(parser, &tmp_ch, 1, 0);

    os_int32_t connect_id = tmp_ch - '0';

    mo_netconn_t *netconn = n21_get_netconn_by_id(module, connect_id);
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

    os_int32_t timeout = data_size > 10 ? data_size : 10;

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
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+TCPSEND",    .suffix = "\r\n",      .func = urc_send_func},
    {.prefix = "+UDPSEND",    .suffix = "\r\n",      .func = urc_send_func},
    {.prefix = "+TCPSETUP:",  .suffix = "\r\n",      .func = urc_connect_func},
    {.prefix = "+UDPSETUP:",  .suffix = "\r\n",      .func = urc_connect_func},
    {.prefix = "+TCPCLOSE:",  .suffix = "Closed\r\n",.func = urc_close_func},
    {.prefix = "",            .suffix = "+TCPRECV:", .func = urc_recv_func},
    {.prefix = "",            .suffix = "+UDPRECV:", .func = urc_recv_func},
};

void n21_netconn_init(mo_n21_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < N21_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* N21_USING_NETCONN_OPS */
