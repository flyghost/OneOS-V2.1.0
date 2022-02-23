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
 * @file        gm190_netconn.c
 *
 * @brief       gm190 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "gm190_netconn.h"
#include "gm190.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "gm190_netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define SEND_DATA_MAX_SIZE    (1400)
#define GM190_NETCONN_MQ_NAME "gm190_nc_mq"

#ifndef GM190_NETCONN_MQ_MSG_SIZE
#define GM190_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef GM190_NETCONN_MQ_MSG_MAX
#define GM190_NETCONN_MQ_MSG_MAX  (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define GM190_EVENT_SEND_OK    (1L << 1)
#define GM190_EVENT_RECV_OK    (1L << 2)
#define GM190_EVENT_SEND_FAIL  (1L << 5)
#define GM190_EVENT_SEND_READY (1L << 6)

#ifdef GM190_USING_NETCONN_OPS

static os_bool_t  gm190_pdp_set(mo_object_t *module)
{
    at_parser_t *parser       = &module->parser;
    char resp_buff[256] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IPV4V6\",\"cmnet\"") != OS_EOK)
    {
        return OS_FALSE;
    }

    if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=0") != OS_EOK)
    {
        return OS_FALSE;
    }

    if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=1") != OS_EOK)
    {
        return OS_FALSE;
    }

    return OS_TRUE;

}

static os_bool_t gm190_check_zipcall(mo_object_t *module, os_int32_t connect_id)
{
    at_parser_t *parser           = &module->parser;
    char         zipcall[30]      = {0};
    char         resp_buff[256]   = {0};
    char         zpas[30]         = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 20 * AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+ZPAS?") != OS_EOK)
    {
        return OS_FALSE;
    }

    if (at_resp_get_data_by_kw(&resp, "+ZPAS:", "+ZPAS: %s", &zpas) <= 0)
    {
        return OS_FALSE;
    }

    if (OS_NULL != strstr(zpas, "No"))
    {
        ERROR("There is no net service!");
        return OS_FALSE;
    }

    if (at_parser_exec_cmd(parser, &resp, "AT+ZIPCALL?") != OS_EOK)
    {
        ERROR("Get ip call failed");
        return OS_FALSE;
    }

    if (at_resp_get_data_by_kw(&resp, "+ZIPCALL:", "+ZIPCALL: %s", &zipcall) <= 0)
    {
        ERROR("Get ip call failed");
        return OS_FALSE;
    }

    if ('1' != zipcall[0] )
    {
        if (at_parser_exec_cmd(parser, &resp, "AT+ZIPCALL=1") != OS_EOK)
        {
            ERROR("Get ip call not ready");
            return OS_FALSE;
        }
        os_task_msleep(500);
    }

    INFO("Check call ip ready!");
    return OS_TRUE;

}

static os_err_t gm190_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t gm190_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_err_t gm190_check_state(mo_object_t *module,os_int32_t connect_id)
{
    at_parser_t *parser         = &module->parser;
    char         resp_buff[256] = {0};
    os_err_t     result         = OS_ERROR;
    os_uint16_t  recv_id        = 0;
    os_uint16_t  recv_stat      = 0;

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    if (at_parser_exec_cmd(parser, &resp, "AT+ZIPSTAT=%d",connect_id) != OS_EOK)
    {
        ERROR("Check connecd id %d state failed!",connect_id);
        result = OS_ERROR;
        goto __exit;
    }

    if(at_resp_get_data_by_kw(&resp, "+ZIPSTAT:", "+ZIPSTAT: %d,%d",&recv_id, &recv_stat) <= 0)
    {
        ERROR("Get %s module zipstat key word failed.", module->name);
        result = OS_ERROR;
        goto __exit;
    }

    if (recv_id != connect_id)
    {
        ERROR("Module %s netconn id %d wait check result timeout!", module->name, connect_id);
        result = OS_ERROR;
        goto __exit;
    }

    if (recv_stat == 0)
    {
        result = OS_EOK;
        goto __exit;
    }
    else
    {
        result = OS_ERROR;
        goto __exit;
    }
__exit:

    if (OS_EOK == result)
    {
        INFO("Module %s check id %d :ready!", module->name, connect_id);
    }
    else
    {
        INFO("Module %s check id %d :used!", module->name, connect_id);
    }

    return result;
}

static mo_netconn_t *gm190_netconn_alloc(mo_object_t *module)
{
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    for (int i = 1; i < GM190_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == gm190->netconn[i].stat)
        {
            if (OS_EOK == gm190_check_state(module, i))
            {
                gm190->netconn[i].connect_id = i;

                return &gm190->netconn[i];
            }
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *gm190_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || GM190_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);
    for (int i = 0; i < GM190_NETCONN_NUM; i++)
    {
        if (connect_id == gm190->netconn[i].connect_id)
        {
            return &gm190->netconn[i];
        }
    }

    return OS_NULL;

}

os_err_t gm190_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    info->netconn_array = gm190->netconn;
    info->netconn_nums  = sizeof(gm190->netconn) / sizeof(gm190->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *gm190_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    gm190_lock(&gm190->netconn_lock);

    mo_netconn_t *netconn = gm190_netconn_alloc(module);
    if (OS_NULL == netconn)
    {
        gm190_unlock(&gm190->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(GM190_NETCONN_MQ_NAME,
                               GM190_NETCONN_MQ_MSG_SIZE,
                               GM190_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        gm190_unlock(&gm190->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;
    gm190_unlock(&gm190->netconn_lock);
    return netconn;
}

os_err_t gm190_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;
    char         zipstat[10] = {0};
    int          i           = 0;

    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);
    gm190_lock(&gm190->netconn_lock);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 10 * OS_TICK_PER_SECOND
                     };

    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;

    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+ZIPCLOSE=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            ERROR("Module %s destroy %s netconn failed",
                      module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");

            gm190_unlock(&gm190->netconn_lock);
            return result;
        }

        do
        {
            os_task_msleep(1000);
            result = at_parser_exec_cmd(parser, &resp, "AT+ZIPSTAT=%d",netconn->connect_id);

            if (result != OS_EOK)
            {
                result = OS_ERROR;
                gm190_unlock(&gm190->netconn_lock);
                return result;
            }

            if (at_resp_get_data_by_kw(&resp, "+ZIPSTAT:", "+ZIPSTAT: %s", &zipstat) <= 0 || i > 3)
            {
                result = OS_ERROR;
                gm190_unlock(&gm190->netconn_lock);
                return result;
            }
            i++;
        }
        while('3' == zipstat[2]);

        if ('0' != zipstat[2])
        {
            result = OS_ERROR;
            gm190_unlock(&gm190->netconn_lock);
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

    gm190_unlock(&gm190->netconn_lock);

    return OS_EOK;
}



os_err_t gm190_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
#define ZIP_CALL_TIMES (3)
    at_parser_t *parser = &module->parser;
    os_err_t    result  = OS_EOK;
    mo_gm190_t * gm190  = os_container_of(module, mo_gm190_t, parent);

    gm190_lock(&gm190->netconn_lock);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    if (OS_TRUE != gm190_check_zipcall(module, netconn->connect_id))
    {
        if (OS_FALSE == gm190_pdp_set(module))
        {
            ERROR("Module %s set !", module->name);
            result = OS_ERROR;
            goto __exit;
        }

        int i = 0;
        while (OS_TRUE != gm190_check_zipcall(module, netconn->connect_id))
        {
            ERROR("Wait module %s call ip !", module->name);
            i++;
            if(i > ZIP_CALL_TIMES)
            {
                ERROR("Wait module %s call ip failed !", module->name);
                break;
            }
            os_task_msleep(5000);
        }
    }

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+ZIPOPEN=%d,0,%s,%d",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+ZIPOPEN=%d,1,%s,%d",
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

    os_task_msleep(1000);

    if (OS_EOK == gm190_check_state(module,netconn->connect_id))
    {
        ERROR("Module %s netconn id %d connect fail", module->name, netconn->connect_id);
        result = OS_ERROR;
        goto __exit;
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
    gm190_unlock(&gm190->netconn_lock);

    return result;
}

os_size_t gm190_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser     = &module->parser;
    os_err_t     result     = OS_EOK;
    os_size_t    sent_size  = 0;
    int          curr_size  = 0;
    os_uint32_t  event      = 0;
    char         at_cmd[30] = {0};
    int          cmd_len    = 0;
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    at_parser_exec_lock(parser);

    gm190->curr_connect = netconn->connect_id;

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

        cmd_len = sprintf(at_cmd,"AT+ZIPSENDRAW=%d,%d\r\n", netconn->connect_id, curr_size);
        if(cmd_len <= 0)
        {
            goto __exit;
        }
        INFO("cmd_len = %d, strlen(at_cmd) = %d",cmd_len,strlen(at_cmd));

        if (at_parser_send(parser, at_cmd, cmd_len) <= 0)
        {
            goto __exit;
        }

        result = os_event_recv(&gm190->netconn_evt,
                               SET_EVENT(netconn->connect_id, GM190_EVENT_SEND_READY),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               30 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait \"CONNECT\" urc timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, curr_size) != curr_size)
        {
            result = OS_ERROR;
            ERROR("Drv or hardware send data fail, some data is missing.\r\n");
            goto __exit;
        }

        result = os_event_recv(&gm190->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               30 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&gm190->netconn_evt,
                               GM190_EVENT_SEND_OK | GM190_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & GM190_EVENT_SEND_FAIL)
        {
            ERROR("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += curr_size;

        os_task_msleep(100);
    }

__exit:

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;
    mo_object_t *module   = os_container_of(parser, mo_object_t, parser);
    mo_gm190_t *gm190     = os_container_of(module, mo_gm190_t, parent);

    sscanf(data, "+ZIPSENDRAW: %d,%d", &connect_id, &data_size);

    os_int32_t curr_connect = gm190->curr_connect;

    if (data_size > 0)
    {
        os_event_send(&gm190->netconn_evt, SET_EVENT(curr_connect, GM190_EVENT_SEND_OK));
    }
    else
    {
        os_event_send(&gm190->netconn_evt, SET_EVENT(curr_connect, GM190_EVENT_SEND_FAIL));
    }
}

static void urc_recv_data_func(struct at_parser *parser, mo_netconn_t *netconn, os_size_t data_size)
{
    mo_object_t *module  = os_container_of(parser, mo_object_t, parser);
    os_int32_t   timeout = data_size > 10 ? data_size : 10;

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

    at_parser_recv(parser, temp_buff, 2, timeout);

    mo_netconn_data_recv_notice(netconn, recv_buff, data_size);
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    char tmp_ch;

    at_parser_recv(parser, &tmp_ch, 1, 0);
    at_parser_recv(parser, &tmp_ch, 1, 0);

    os_int32_t connect_id = tmp_ch - '0';

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = gm190_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }
    /* Handle ip and port */
    for (int count = 0; count < 3;)
    {
        at_parser_recv(parser, &tmp_ch, 1, 0);
        if (',' == tmp_ch)
        {
            count++;
        }
    }

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

    urc_recv_data_func(parser, netconn, data_size);

    return;
}

static void urc_conn_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module           = os_container_of(parser, mo_object_t, parser);
    mo_gm190_t *gm190 = os_container_of(module, mo_gm190_t, parent);

    os_int32_t curr_connect = gm190->curr_connect;

    os_event_send(&gm190->netconn_evt, SET_EVENT(curr_connect, GM190_EVENT_SEND_READY));
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+ZIPSENDRAW:", .suffix = "\r\n",           .func = urc_send_func},
    {.prefix = "",             .suffix = "+ZIPRECV:",      .func = urc_recv_func},
    {.prefix = "CONNECT",      .suffix = "\r\n",           .func = urc_conn_func},
};

static void gm190_network_init(mo_object_t *module)
{
    at_parser_t *parser        = &module->parser;
    os_int32_t   enable_num    = 0;
    os_int32_t   reg_state     = 0;
    os_err_t     result        = OS_ERROR;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    result = at_parser_exec_cmd(parser, &resp, "AT+CREG?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    if (at_resp_get_data_by_kw(&resp, "+CREG:", "+CREG: %d,%d", &enable_num , &reg_state) < 0)
    {
        result = OS_ERROR;
        goto __exit;
    }
    if (1 == reg_state || 5 == reg_state)
    {
        result = OS_EOK;
        goto __exit;
    }

    resp.timeout = 10 * OS_TICK_PER_SECOND;

    if (at_parser_exec_cmd(parser, &resp, "AT+CGDCONT=1,\"IPV4V6\",\"cmnet\"") != OS_EOK)
    {
        result = OS_ERROR;
        goto __exit;

    }

    if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=0") != OS_EOK)
    {
        result = OS_ERROR;
        goto __exit;

    }

    if (at_parser_exec_cmd(parser, &resp, "AT+CFUN=1") != OS_EOK)
    {
        result = OS_ERROR;
        goto __exit;
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+CREG?");
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "+CREG:", "+CREG: %d,%d", &enable_num , &reg_state) < 0)
    {
        goto __exit;
    }

    if (1 == reg_state || 5 == reg_state)
    {
        result = OS_EOK;
        goto __exit;
    }
    else
    {
        result = OS_ERROR;
        goto __exit;
    }

__exit:
    if(result != OS_EOK)
    {
        WARN("GM190 network init failed");
    }
    else
    {
        WARN("GM190 network init sucess");
    }
}

void gm190_netconn_init(mo_gm190_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 1; i < GM190_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    gm190_network_init(&module->parent);

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* GM190_USING_NETCONN_OPS */


