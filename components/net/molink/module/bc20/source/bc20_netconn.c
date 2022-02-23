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
 * @file        bc20_netconn.c
 *
 * @brief       bc20 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "bc20_netconn.h"
#include "bc20.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "bc20.netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define BC20_CONN_ID_NULL  (-1)
#define SEND_DATA_MAX_SIZE (1024)

#define BC20_NETCONN_MQ_NAME     "bc20_nc_mq"
#ifndef BC20_NETCONN_MQ_MSG_SIZE
#define BC20_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif /* BC20_NETCONN_MQ_MSG_SIZE */
#ifndef BC20_NETCONN_MQ_MSG_MAX
#define BC20_NETCONN_MQ_MSG_MAX  (5)
#endif /* BC20_NETCONN_MQ_MSG_MAX */

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define BC20_EVENT_CONN_OK   (1L << 0)
#define BC20_EVENT_SEND_OK   (1L << 1)
#define BC20_EVENT_RECV_OK   (1L << 2)
#define BC20_EVNET_CLOSE_OK  (1L << 3)
#define BC20_EVENT_CONN_FAIL (1L << 4)
#define BC20_EVENT_SEND_FAIL (1L << 5)
#define BC20_EVENT_DOMAIN_OK (1L << 6)

#define BC20_NETCONN_BUFF_DEF (256)

#ifdef BC20_USING_NETCONN_OPS

static os_err_t bc20_lock(os_mutex_t *mutex)
{
    OS_ASSERT(OS_NULL != mutex);
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t bc20_unlock(os_mutex_t *mutex)
{
    OS_ASSERT(OS_NULL != mutex);
    return os_mutex_recursive_unlock(mutex);
}

static os_bool_t bc20_check_state(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    at_parser_t *parser = &module->parser;

    char resp_buff[BC20_NETCONN_BUFF_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    /* query_type 1: Query connection status of the specified socket service */
    if (at_parser_exec_cmd(parser, &resp, "AT+QISTATE=1,%d", connect_id) != OS_EOK)
    {
        ERROR("Check connecd id %d state failed!", connect_id);
        return OS_FALSE;
    }

    if (at_resp_get_line_by_kw(&resp, "+QISTATE:") != OS_NULL)
    {
        /* connect id already in use */
        return OS_FALSE;
    }

    /* query_type 2: set netconn echo mode off, in case of restore deepsleep case */
    if (OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+QICFG=\"echomode\",0"))
    {
        ERROR("%s set echo mode off failed!", __func__, connect_id);
        return OS_FALSE;
    }

    return OS_TRUE;
}

static mo_netconn_t *bc20_netconn_alloc(mo_object_t *module)
{
    OS_ASSERT(OS_NULL != module);

    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    for (int i = 0; i < BC20_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == bc20->netconn[i].stat)
        {
            if (bc20_check_state(module, i))
            {
                bc20->netconn[i].connect_id = i;

                return &bc20->netconn[i];
            }
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *bc20_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || BC20_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    return &bc20->netconn[connect_id];
}

os_err_t bc20_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    info->netconn_array = bc20->netconn;
    info->netconn_nums  = sizeof(bc20->netconn) / sizeof(bc20->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *bc20_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    bc20_lock(&bc20->netconn_lock);

    mo_netconn_t *netconn = bc20_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        bc20_unlock(&bc20->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(BC20_NETCONN_MQ_NAME,
                               BC20_NETCONN_MQ_MSG_SIZE,
                               BC20_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s message queue create failed, no enough memory.", module->name);
        bc20_unlock(&bc20->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    bc20_unlock(&bc20->netconn_lock);

    return netconn;
}

os_err_t bc20_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;
    mo_bc20_t     *bc20 = os_container_of(module, mo_bc20_t, parent);

    bc20_lock(&bc20->netconn_lock);

    DEBUG("Module %s in %d netconn status", module->name, netconn->stat);

    char resp_buff[BC20_NETCONN_BUFF_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 4 * OS_TICK_PER_SECOND};

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
    case NETCONN_STAT_CONNECT:

        os_event_recv(&bc20->netconn_evt,
                  BC20_EVNET_CLOSE_OK,
                  OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                  OS_NO_WAIT,
                  OS_NULL);

        result = at_parser_exec_cmd(parser, &resp, "AT+QICLOSE=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            ERROR("Module %s destroy %s netconn failed",
                       module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
            goto __exit;
        }

        /* get async close info */
        result = os_event_recv(&bc20->netconn_evt,
                            BC20_EVNET_CLOSE_OK,
                            OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                            4 * OS_TICK_PER_SECOND,
                            OS_NULL);

        if(OS_EOK != result)
        {
            ERROR("%s destroy netconn:%d failed", __func__, netconn->connect_id);
            goto __exit;
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

    netconn->connect_id  = BC20_CONN_ID_NULL;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);


__exit:
    bc20_unlock(&bc20->netconn_lock);
    return result;
}

os_err_t bc20_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;
    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    char resp_buff[BC20_NETCONN_BUFF_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_uint32_t event = SET_EVENT(netconn->connect_id, BC20_EVENT_CONN_OK | BC20_EVENT_CONN_FAIL);

    os_event_recv(&bc20->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);

    os_err_t result = OS_EOK;

    bc20_lock(&bc20->netconn_lock);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+QIOPEN=1,%d,\"TCP\",\"%s\",%d,0,1",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+QIOPEN=1,%d,\"UDP\",\"%s\",%d,0,1",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    default:
        ERROR("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
        result = OS_ERROR;
        goto __exit;
    }

    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&bc20->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait connect event timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    result = os_event_recv(&bc20->netconn_evt,
                           BC20_EVENT_CONN_OK | BC20_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("Module %s netconn id %d wait conect result timeout!", module->name, netconn->connect_id);
        goto __exit;
    }

    if (event & BC20_EVENT_CONN_FAIL)
    {
        result = OS_ERROR;
        ERROR("Module %s netconn id %d connect failed!", module->name, netconn->connect_id);
    }

__exit:
    bc20_unlock(&bc20->netconn_lock);

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

os_size_t bc20_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    at_parser_exec_lock(parser);

    bc20->curr_connect = netconn->connect_id;

    char resp_buff[2 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 10 * OS_TICK_PER_SECOND};

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

        result = at_parser_exec_cmd(parser, &resp, "AT+QISEND=%d,%d", netconn->connect_id, curr_size);
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

        result = os_event_recv(&bc20->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&bc20->netconn_evt,
                               BC20_EVENT_SEND_OK | BC20_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & BC20_EVENT_SEND_FAIL)
        {
            ERROR("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += curr_size;
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

os_err_t bc20_netconn_gethostbyname(mo_object_t *module, const char *domain_name, ip_addr_t *addr)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    char resp_buff[2 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = OS_TICK_PER_SECOND};

    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);
    if (OS_NULL == bc20)
    {
        ERROR("%s invalid param", __func__);
        return OS_ERROR;
    }

    bc20_lock(&bc20->netconn_lock);

    os_event_recv(&bc20->netconn_evt,
                  BC20_EVENT_DOMAIN_OK,
                  OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                  OS_NO_WAIT,
                  OS_NULL);

    bc20->netconn_data = addr;

    result = at_parser_exec_cmd(parser, &resp, "AT+QIDNSGIP=1,\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    result = os_event_recv(&bc20->netconn_evt,
                           BC20_EVENT_DOMAIN_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);

__exit:
    bc20->netconn_data = OS_NULL;

    bc20_unlock(&bc20->netconn_lock);

    return result;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    os_int32_t connect_id = 0;
    os_int32_t result     = 0;

    sscanf(data, "+QIOPEN: %d,%d", &connect_id , &result);

    if (0 == result)
    {
        os_event_send(&bc20->netconn_evt, SET_EVENT(connect_id, BC20_EVENT_CONN_OK));
    }
    else
    {
        os_event_send(&bc20->netconn_evt, SET_EVENT(connect_id, BC20_EVENT_CONN_FAIL));
    }
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    os_int32_t curr_connect = bc20->curr_connect;

    if (strstr(data, "SEND OK"))
    {
        os_event_send(&bc20->netconn_evt, SET_EVENT(curr_connect, BC20_EVENT_SEND_OK));
    }
    else if (strstr(data, "SEND FAIL"))
    {
        os_event_send(&bc20->netconn_evt, SET_EVENT(curr_connect, BC20_EVENT_SEND_FAIL));
    }
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "+QIURC: \"closed\",%d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = bc20_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    WARN("Module %s receive close urc data of connect %d", module->name, connect_id);

    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_close_result_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    os_event_send(&bc20->netconn_evt, BC20_EVNET_CLOSE_OK);

    DEBUG("Module %s receive close OK message.", module->name);

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;

    sscanf(data, "+QIURC: \"recv\",%d,%d", &connect_id, &data_size);

    os_int32_t timeout = data_size > 10 ? data_size : 10;

    // INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = bc20_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    char *recv_buff    = os_calloc(1, data_size);
    char  temp_buff[8] = {0};
    if (recv_buff == OS_NULL)
    {
        /* read and clean the coming data */
        ERROR("alloc recv buff %d bytes fail, no enough memory", data_size);
        os_size_t temp_size    = 0;
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

    // INFO("Module %s netconnt id %d recv [%s]!", module->name, netconn->connect_id, recv_buff);
    // os_task_mdelay(500);
    mo_netconn_data_recv_notice(netconn, recv_buff, data_size);

    return;
}

static void urc_pdpdeact_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    int context_id = 0;

    sscanf(data, "+QIURC: \"pdpdeact\",%d", &context_id);

    ERROR("context (%d) is deactivated.", context_id);
}

static void urc_dnsqip_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_bc20_t *bc20 = os_container_of(module, mo_bc20_t, parent);

    int j = 0;

    for (int i = 0; i < size; i++)
    {
        if (*(data + i) == '.')
            j++;
    }
    /* There would be several dns result, we just pickup one */
    if (3 == j)
    {
        char recvip[IPADDR_MAX_STR_LEN + 1] = {0};

        sscanf(data, "+QIURC: \"dnsgip\",\"%[^\"]", recvip);
        recvip[IPADDR_MAX_STR_LEN] = '\0';

        inet_aton(recvip, (ip_addr_t *)bc20->netconn_data);

        os_event_send(&bc20->netconn_evt, BC20_EVENT_DOMAIN_OK);
    }
    else
    {
        DEBUG("Not required dns URC data, not processed");
    }
}

static void urc_func(struct at_parser *parsert, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != data);

    INFO("URC data : %.*s", size, data);
}

static void urc_qiurc_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    switch (*(data + 9))
    {
    case 'c':
        urc_close_func(parser, data, size);
        break;
    case 'r':
        urc_recv_func(parser, data, size);
        break;
    case 'p':
        urc_pdpdeact_func(parser, data, size);
        break;
    case 'd':
        urc_dnsqip_func(parser, data, size);
        break;
    default:
        urc_func(parser, data, size);
        break;
    }
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "SEND",     .suffix = "\r\n", .func = urc_send_func},
    {.prefix = "+QIOPEN:", .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "+QIURC:",  .suffix = "\r\n", .func = urc_qiurc_func},
    {.prefix = "CLOSE OK", .suffix = "\r\n", .func = urc_close_result_func},
};

void bc20_netconn_init(mo_bc20_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < BC20_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = BC20_CONN_ID_NULL;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* BC20_USING_NETCONN_OPS */
