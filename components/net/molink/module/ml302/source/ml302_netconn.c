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
 * @file        ml302_netconn.c
 *
 * @brief       ml302 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "ml302_netconn.h"
#include "ml302.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <os_task.h>

#define MO_LOG_TAG "ml302.netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define SEND_DATA_MAX_SIZE    (1460)
#define ML302_NETCONN_MQ_NAME "ml302_nc_mq"

#ifndef ML302_NETCONN_MQ_MSG_SIZE
#define ML302_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef ML302_NETCONN_MQ_MSG_MAX
#define ML302_NETCONN_MQ_MSG_MAX  (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define ML302_EVENT_CONN_OK     (1L << 0)
#define ML302_EVENT_SEND_OK     (1L << 1)
#define ML302_EVENT_RECV_OK     (1L << 2)
#define ML302_EVNET_CLOSE_OK    (1L << 3)
#define ML302_EVENT_CONN_FAIL   (1L << 4)
#define ML302_EVENT_SEND_FAIL   (1L << 5)
#define ML302_EVENT_DOMAIN_OK   (1L << 6)
#define ML302_EVENT_DOMAIN_FAIL (1L << 7)

#ifdef ML302_USING_NETCONN_OPS

static os_err_t ml302_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t ml302_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_bool_t ml302_check_state(mo_object_t *module, os_int32_t connect_id)
{
    at_parser_t *parser       = &module->parser;
    char         mipstate[50] = {0};
    const char   connect[10]  = "CONNECT";
    const char   listen[10]   = "LISTEN";

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+MIPSTATE=%d", connect_id) != OS_EOK)
    {
        ERROR("Check connecd id %d state failed!", connect_id);
        return OS_FALSE;
    }

    if (at_resp_get_data_by_kw(&resp, "+MIPSTATE:", "+MIPSTATE:%s", &mipstate) <= 0)
    {
        ERROR("Get connect_id :%d  ip state failed", connect_id);
        return OS_ERROR;
    }
    if (OS_NULL == strstr(mipstate, connect) && OS_NULL == strstr(mipstate, listen))
    {
        INFO("Check connect id %d state free!", connect_id);
        return OS_TRUE;
    }

    INFO("Check connect id %d state used!", connect_id);
    return OS_FALSE;
}

static mo_netconn_t *ml302_netconn_alloc(mo_object_t *module)
{
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    for (int i = 0; i < ML302_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == ml302->netconn[i].stat)
        {
            if (ml302_check_state(module, i))
            {
                ml302->netconn[i].connect_id = i;

                return &ml302->netconn[i];
            }
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *ml302_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || ML302_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    return &ml302->netconn[connect_id];
}

os_err_t ml302_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    info->netconn_array = ml302->netconn;
    info->netconn_nums  = sizeof(ml302->netconn) / sizeof(ml302->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *ml302_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_ml302_t *ml302  = os_container_of(module, mo_ml302_t, parent);

    ml302_lock(&ml302->netconn_lock);

    mo_netconn_t *netconn = ml302_netconn_alloc(module);
    if (OS_NULL == netconn)
    {
        ml302_unlock(&ml302->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(ML302_NETCONN_MQ_NAME,
                               ML302_NETCONN_MQ_MSG_SIZE,
                               ML302_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        ml302_unlock(&ml302->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    ml302_unlock(&ml302->netconn_lock);

    return netconn;
}

static os_err_t ml302_netconn_do_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[256] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND,
                      .line_num  = 1};

    os_err_t result = OS_EOK;

    result = at_parser_exec_cmd(parser, &resp, "AT+MIPCLOSE=%d", netconn->connect_id);

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

    if ((strstr(source_line, ",CLOSE OK") != OS_NULL &&
        (source_line[0] - 0X30) ==  netconn->connect_id) ||
        ml302_check_state(module, netconn->connect_id)) /* Reconfirm that the connection is closed */
    {
        return OS_EOK;
    }

    return OS_ERROR;
}

os_err_t ml302_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    os_err_t     result = OS_ERROR;

    DEBUG("Module %s in %d netconn status", module->name, netconn->stat);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);
    ml302_lock(&ml302->netconn_lock);

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;
    case NETCONN_STAT_CONNECT:
        result = ml302_netconn_do_destroy(module, netconn);
        if (result != OS_EOK)
        {
            ERROR("Module %s destroy %s netconn failed",
                      module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
            ml302_unlock(&ml302->netconn_lock);

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

    ml302_unlock(&ml302->netconn_lock);

    return OS_EOK;
}

os_err_t ml302_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;


    mo_ml302_t * ml302  = os_container_of(module, mo_ml302_t, parent);
    ml302_lock(&ml302->netconn_lock);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_uint32_t event = SET_EVENT(netconn->connect_id, ML302_EVENT_CONN_OK | ML302_EVENT_CONN_FAIL);

    os_event_recv(&ml302->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    os_err_t result = OS_EOK;

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+MIPOPEN=%d,\"TCP\",\"%s\",%d",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+MIPOPEN=%d,\"UDP\",\"%s\",%d",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    default:
        result = OS_ERROR;
        goto __exit;
    }

    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&ml302->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait conect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&ml302->netconn_evt,
                           ML302_EVENT_CONN_OK | ML302_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait conect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & ML302_EVENT_CONN_FAIL)
    {
        result = OS_ERROR;
        ERROR("Module %s netconn id %d conect failed!", module->name, netconn->connect_id);
    }

__exit:
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
    ml302_unlock(&ml302->netconn_lock);
    return result;
}

os_size_t ml302_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    at_parser_exec_lock(parser);

    ml302->curr_connect = netconn->connect_id;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND
                     };

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

        result = os_event_recv(&ml302->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&ml302->netconn_evt,
                               ML302_EVENT_SEND_OK | ML302_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & ML302_EVENT_SEND_FAIL)
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

os_err_t ml302_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser = &self->parser;
    os_err_t     result = OS_EOK;
    os_uint32_t  event  = 0;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    mo_ml302_t *ml302 = os_container_of(self, mo_ml302_t, parent);

    ml302_lock(&ml302->netconn_lock);

    os_event_clear(&ml302->netconn_evt, ML302_EVENT_DOMAIN_OK | ML302_EVENT_DOMAIN_FAIL);

    ml302->netconn_data = addr;

    result = at_parser_exec_cmd(parser, &resp, "AT+MDNSGIP=\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&ml302->netconn_evt,
                           ML302_EVENT_DOMAIN_OK | ML302_EVENT_DOMAIN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s wait gethostbyname result timeout!", self->name);
        goto __exit;
    }

    if (event & ML302_EVENT_DOMAIN_FAIL)
    {
        ERROR("Module %s gethostbyname failed!", self->name);
        result = OS_ERROR;
        goto __exit;
    }

__exit:
    ml302->netconn_data = OS_NULL;

    ml302_unlock(&ml302->netconn_lock);

    return result;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);

    os_int32_t connect_id = -1;
    connect_id = data[0] - '0';
    if (strstr(data, "CONNECT OK"))
    {
        os_event_send(&ml302->netconn_evt, SET_EVENT(connect_id, ML302_EVENT_CONN_OK));
    }
    else
    {
        os_event_send(&ml302->netconn_evt, SET_EVENT(connect_id, ML302_EVENT_CONN_FAIL));
    }
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);

    os_int32_t curr_connect = ml302->curr_connect;

    if (strstr(data, "SEND OK"))
    {

        os_event_send(&ml302->netconn_evt, SET_EVENT(curr_connect, ML302_EVENT_SEND_OK));
    }
    else
    {
        os_event_send(&ml302->netconn_evt, SET_EVENT(curr_connect, ML302_EVENT_SEND_FAIL));
    }
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    sscanf(data, "+MIPURC: \"recv\",%d,%d", &connect_id, &data_size);

    os_int32_t timeout = data_size > 10 ? data_size : 10;

    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = ml302_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    char *recv_buff = os_calloc(1, data_size);
    if (recv_buff == OS_NULL)
    {
        /* read and clean the coming data */
        ERROR("Calloc recv buff %d bytes fail, no enough memory", data_size * 2);
        os_size_t temp_size    = 0;
        char      temp_buff[8] = {0};
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
        return;
    }

    if (at_parser_recv(parser, recv_buff, data_size, timeout) != data_size)
    {
        ERROR("Module %s netconnt id %d recv %d bytes data failed!", module->name, netconn->connect_id, data_size);
        os_free(recv_buff);

        return;
    }

    mo_netconn_data_recv_notice(netconn, recv_buff, data_size);

    return;
}

static void urc_state_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id    = 0;
    os_int32_t connect_state = 0;

    sscanf(data, "+MIPURC: \"STATE\",%d,%d", &connect_id, &connect_state);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = ml302_get_netconn_by_id(module, connect_id);

    switch (connect_state)
    {
    case 1:
        WARN("Module %s receive close urc data of connect %d, server closed the connection.",
                  module->name,
                  connect_id);
        break;
    case 2:
        WARN("Module %s receive close urc data of connect %d, connection exception.",
                  module->name,
                  connect_id);
        break;
    default:
        break;
    }

    mo_netconn_pasv_close_notice(netconn);
}

static void urc_mdnsgip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
#define HOST_NAME_MAX_LEN 50
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);

    os_int32_t result = 0;

    sscanf(data, "+MDNSGIP: %d,%*s", &result);

    if (0 == result)
    {
        char recvip[IPADDR_MAX_STR_LEN + 1]   = {0};
        sscanf(data, "+MDNSGIP: %*d,\"%*[^\"]\",\"%[^\"]", recvip);
        recvip[IPADDR_MAX_STR_LEN] = '\0';
        inet_aton(recvip, (ip_addr_t *)ml302->netconn_data);
        os_event_send(&ml302->netconn_evt, ML302_EVENT_DOMAIN_OK);
    }
    else /* 1 == result */
    {
        os_event_send(&ml302->netconn_evt, ML302_EVENT_DOMAIN_FAIL);
    }
}

static void urc_mipurc_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    switch (*(data + 10))
    {
    case 'r':
        urc_recv_func(parser, data, size);
        break;
    case 'S':
        urc_state_func(parser, data, size);
        break;
    default:
        break;
    }
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "%d,CLOSED", &connect_id);

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = ml302_get_netconn_by_id(module, connect_id);

    if (NETCONN_STAT_CONNECT == netconn->stat)
    {
        WARN("Module %s receive close urc data of connect %d", module->name, connect_id);

        mo_netconn_pasv_close_notice(netconn);
    }
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "",          .suffix = "CONNECT OK\r\n", .func = urc_connect_func},
    {.prefix = "",          .suffix = "SEND OK\r\n",    .func = urc_send_func},
    {.prefix = "+MIPURC:",  .suffix = "\r\n",           .func = urc_mipurc_func},
    {.prefix = "+MDNSGIP:", .suffix = "\r\n",           .func = urc_mdnsgip_func},
    {.prefix = "",          .suffix = ",CLOSED\r\n",    .func = urc_close_func},
};

void ml302_netconn_init(mo_ml302_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < ML302_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* ML302_USING_NETCONN_OPS */
