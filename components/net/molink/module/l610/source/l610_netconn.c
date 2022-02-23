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
 * @file        l610.c
 *
 * @brief       l610 module link kit netconn api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "l610_netconn.h"
#include "l610.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "l610.netconn"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define SEND_DATA_MAX_SIZE (2048)

#define L610_NETCONN_MQ_NAME "l610_nc_mq"

#ifndef L610_NETCONN_MQ_MSG_SIZE
#define L610_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef L610_NETCONN_MQ_MSG_MAX
#define L610_NETCONN_MQ_MSG_MAX  (10)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define L610_EVENT_CONN_OK   (1L << 0)
#define L610_EVENT_SEND_OK   (1L << 1)
#define L610_EVENT_RECV_OK   (1L << 2)
#define L610_EVNET_CLOSE_OK  (1L << 3)
#define L610_EVENT_CONN_FAIL (1L << 4)
#define L610_EVENT_SEND_FAIL (1L << 5)
#define L610_EVENT_DOMAIN_OK (1L << 6)

#ifdef L610_USING_NETCONN_OPS

static os_err_t l610_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t l610_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *l610_netconn_alloc(mo_object_t *module)
{
    mo_l610_t *l610 = os_container_of(module, mo_l610_t, parent);

    for (int i = 0; i < L610_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == l610->netconn[i].stat)
        {
            /* L610 connect id is 1 to 6 */
            l610->netconn[i].connect_id = i + 1;
            return &l610->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *l610_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    /* L610 connect id is 1 to 6 */
    if (1 > connect_id || L610_NETCONN_NUM < connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_l610_t *l610 = os_container_of(module, mo_l610_t, parent);

    for (int i = 0; i < L610_NETCONN_NUM; i++)
    {
        if (connect_id == l610->netconn[i].connect_id)
        {
            return &l610->netconn[i];
        }
    }

    return OS_NULL;
}

os_err_t l610_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_l610_t *l610 = os_container_of(module, mo_l610_t, parent);

    info->netconn_array = l610->netconn;
    info->netconn_nums  = sizeof(l610->netconn) / sizeof(l610->netconn[0]);

    return OS_EOK;
}

static os_err_t l610_netconn_set_format(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+GTSET=\"IPRFMT\",2") != OS_EOK)
    {
        ERROR("Set module %s tcp/ip data format failed!", module->name);
        return OS_ERROR;
    }

    return OS_EOK;
}

mo_netconn_t *l610_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_l610_t    *l610 = os_container_of(module, mo_l610_t, parent);
    mo_netconn_t *netconn = l610_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        return OS_NULL;
    }

    l610_lock(&l610->netconn_lock);

    if (l610_netconn_set_format(module) != OS_EOK)
    {
        return OS_NULL;
    }

    netconn->mq = os_mq_create(L610_NETCONN_MQ_NAME, L610_NETCONN_MQ_MSG_SIZE, L610_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        l610_unlock(&l610->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    l610_unlock(&l610->netconn_lock);

    return netconn;
}

static os_err_t l610_netconn_do_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    mo_l610_t   *l610   = os_container_of(module, mo_l610_t, parent);
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 2 * OS_TICK_PER_SECOND};

    if (l610 == OS_NULL)
    {
        ERROR("Module %s destroy netconn failed, get netconn_lock failed", module->name);
        return OS_ERROR;
    }

    os_uint32_t event = SET_EVENT(netconn->connect_id, L610_EVNET_CLOSE_OK);

    os_event_recv(&l610->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+MIPCLOSE=%d", netconn->connect_id);
    if (result != OS_EOK)
    {
        ERROR("Module %s destroy %s netconn failed",
              module->name,
              (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
        return result;
    }

    result = os_event_recv(&l610->netconn_evt,
                           event,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);

    if (result != OS_EOK)
    {
        ERROR("Module %s destroy %s netconn failed",
              module->name,
              (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
    }

    return result;
}

os_err_t l610_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    mo_l610_t *l610  = os_container_of(module, mo_l610_t, parent);
    os_err_t  result = OS_EOK;

    if (l610 == OS_NULL)
    {
        ERROR("Module %s destroy netconn failed, get netconn_lock failed", module->name);
        return OS_ERROR;
    }

    DEBUG("Module %s in %d netconn status", module->name, netconn->stat);

    l610_lock(&l610->netconn_lock);

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;

    case NETCONN_STAT_CONNECT:
        result = l610_netconn_do_destroy(module, netconn);
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

    l610_unlock(&l610->netconn_lock);

    return OS_EOK;
}

os_err_t l610_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    mo_l610_t   *l610   = os_container_of(module, mo_l610_t, parent);
    os_uint32_t event   = SET_EVENT(netconn->connect_id, L610_EVENT_CONN_OK | L610_EVENT_CONN_FAIL);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF]  = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 60 * OS_TICK_PER_SECOND};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    l610_lock(&l610->netconn_lock);
    os_event_recv(&l610->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    os_int32_t netconn_type = (netconn->type == NETCONN_TYPE_TCP) ? 0 : 1;

    /* AT+MIPOPEN=1,,"sz3.fibocom.com",9000,0 */
    os_err_t result = at_parser_exec_cmd(parser,
                                         &resp,
                                         "AT+MIPOPEN=%d,,\"%s\",%d,%d",
                                         netconn->connect_id,
                                         remote_ip,
                                         port,
                                         netconn_type);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d connect cmd exec failed!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&l610->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait conect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&l610->netconn_evt,
                           L610_EVENT_CONN_OK | L610_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           3 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait connect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & L610_EVENT_CONN_FAIL)
    {
        result = OS_ERROR;
        ERROR("Module %s netconn id %d connect failed!", module->name, netconn->connect_id);
    }

__exit:

    l610_unlock(&l610->netconn_lock);

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

os_size_t l610_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};

    mo_l610_t *l610 = os_container_of(module, mo_l610_t, parent);
    at_resp_t resp  = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout   = 12 * OS_TICK_PER_SECOND};

    at_parser_exec_lock(parser);

    at_parser_set_end_mark(parser, ">", 1);

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

        result = at_parser_exec_cmd(parser, &resp, "AT+MIPSEND=%d,%d", netconn->connect_id, curr_size);
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, curr_size) != curr_size)
        {
            result = OS_ERROR;
            ERROR("Drv or hardware send data fail, some data is missing.\r\n");
            goto __exit;
        }

        result = os_event_recv(&l610->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               30 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&l610->netconn_evt,
                               L610_EVENT_SEND_OK | L610_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & L610_EVENT_SEND_FAIL)
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

os_err_t l610_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};
    char recvip[IPADDR_MAX_STR_LEN + 1]   = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 15 * OS_TICK_PER_SECOND};

    result = at_parser_exec_cmd(parser, &resp, "AT+MIPDNS=\"%s\",2", domain_name);
    if (result != OS_EOK)
    {
        ERROR("Module %s gethostbyname %s failed, AT+MIPDNS cmd exec failed!", self->name, domain_name);
        return result;
    }

    if (at_resp_get_data_by_kw(&resp, "+MIPDNS: ", "%*[^,],%[^,]", recvip) <= 0)
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
              self->name,
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
    mo_l610_t   *l610   = os_container_of(module, mo_l610_t, parent);

    os_int32_t connect_id = 0;

    sscanf(data, "+MIPCLOSE: %d,&*d", &connect_id);

    os_int32_t event = SET_EVENT(connect_id, L610_EVNET_CLOSE_OK);

    os_event_send(&l610->netconn_evt, event);

    return;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_l610_t   *l610   = os_container_of(module, mo_l610_t, parent);

    os_int32_t connect_id = 0;
    os_int32_t stat       = 0;

    sscanf(data, "+MIPOPEN: %d,%d", &connect_id, &stat);

    mo_netconn_t *netconn = l610_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    if (NETCONN_STAT_INIT == netconn->stat && 1 == stat)
    {
        os_event_send(&l610->netconn_evt, SET_EVENT(connect_id, L610_EVENT_CONN_OK));
    }

    return;
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_l610_t   *l610   = os_container_of(module, mo_l610_t, parent);

    os_int32_t connect_id = 0;
    os_int32_t stat       = 0;

    sscanf(data, "+MIPSEND: %d,%d,%*d", &connect_id, &stat);

    mo_netconn_t *netconn = l610_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    if (0 == stat)
    {
        os_event_send(&l610->netconn_evt, SET_EVENT(connect_id, L610_EVENT_SEND_OK));
    }
    else
    {
        os_event_send(&l610->netconn_evt, SET_EVENT(connect_id, L610_EVENT_SEND_FAIL));
    }

    return;
}

static void urc_state_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "+MIPSTAT: %d,&*d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_l610_t   *l610   = os_container_of(module, mo_l610_t, parent);

    mo_netconn_t *netconn = l610_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    if (NETCONN_STAT_INIT == netconn->stat)
    {
        os_event_send(&l610->netconn_evt, SET_EVENT(connect_id, L610_EVENT_CONN_FAIL));
    }
    else if (NETCONN_STAT_CONNECT == netconn->stat)
    {
        WARN("Module %s receive close urc data of connect %d", module->name, connect_id);
        mo_netconn_pasv_close_notice(netconn);
    }

    return;
}

static void urc_recv_func(struct at_parser *parser, mo_netconn_t *netconn, os_size_t data_size)
{
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    os_int32_t  timeout = data_size > 10 ? data_size : 10;

    char  temp_buff[8] = {0};

    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, netconn->connect_id, data_size);

    char *recv_buff = os_calloc(1, data_size);
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

    at_parser_recv(parser, temp_buff, 2, timeout);

    mo_netconn_data_recv_notice(netconn, recv_buff, data_size);

    return;
}

static void urc_tcprecv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    /* Get connecnt id */
    char tmp_ch;

    /* handle the blank space */
    at_parser_recv(parser, &tmp_ch, 1, 0);
    at_parser_recv(parser, &tmp_ch, 1, 0);

    os_int32_t connect_id = tmp_ch - '0';

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = l610_get_netconn_by_id(module, connect_id);
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

    urc_recv_func(parser, netconn, data_size);

    return;
}

static void urc_udprecv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    char tmp_ch;

    /* Handle ip and port */
    for (int count = 0; count < 2;)
    {
        at_parser_recv(parser, &tmp_ch, 1, 0);
        if (',' == tmp_ch)
        {
            count++;
        }
    }

    /* Get connect id */
    at_parser_recv(parser, &tmp_ch, 1, 0);

    os_int32_t connect_id = tmp_ch - '0';

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = l610_get_netconn_by_id(module, connect_id);
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

    urc_recv_func(parser, netconn, data_size);

    return;
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+MIPSEND:",   .suffix = "\r\n",      .func = urc_send_func},
    {.prefix = "+MIPOPEN:",   .suffix = "\r\n",      .func = urc_connect_func},
    {.prefix = "+MIPSTAT:",   .suffix = "\r\n",      .func = urc_state_func},
    {.prefix = "+MIPCLOSE:",  .suffix = "\r\n",      .func = urc_close_func},
    {.prefix = "",            .suffix = "+MIPRTCP:", .func = urc_tcprecv_func},
    {.prefix = "",            .suffix = "+MIPRUDP:", .func = urc_udprecv_func},
};

void l610_netconn_init(mo_l610_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < L610_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return;
}

#endif /* L610_USING_NETCONN_OPS */
