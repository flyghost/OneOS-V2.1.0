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
 * @file        gm510_netconn.c
 *
 * @brief       gm510 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "gm510_netconn.h"
#include "gm510.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "gm510_netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define SEND_DATA_MAX_SIZE (1400)
#define GM510_NETCONN_MQ_NAME "gm510_nc_mq"

#ifndef GM510_NETCONN_MQ_MSG_SIZE
#define GM510_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef GM510_NETCONN_MQ_MSG_MAX
#define GM510_NETCONN_MQ_MSG_MAX  (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define GM510_EVENT_CONN_OK    (1L << 0)
#define GM510_EVENT_SEND_OK    (1L << 1)
#define GM510_EVENT_RECV_OK    (1L << 2)
#define GM510_EVENT_CLOSE_OK   (1L << 3)
#define GM510_EVENT_CONN_FAIL  (1L << 4)
#define GM510_EVENT_SEND_FAIL  (1L << 5)
#define GM510_EVENT_DOMAIN_OK  (1L << 6)
#define GM510_EVENT_STAT_OK    (1L << 7)
#define GM510_EVENT_STAT_FAIL  (1L << 8)



#ifdef GM510_USING_NETCONN_OPS

static os_bool_t  gm510_pdp_set(mo_object_t *module)
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

static os_bool_t gm510_check_zipcall(mo_object_t *module)
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
        os_task_msleep(1000);
    }
    INFO("Check call ip ready!");

    return OS_TRUE;
}

static os_err_t gm510_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t gm510_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static os_err_t gm510_check_state(mo_object_t *module,os_int32_t connect_id)
{
    at_parser_t *parser         = &module->parser;
    char         resp_buff[256] = {0};
    char         zipstat[30]    = {0};

    os_err_t     result         = OS_ERROR;

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

	result = at_parser_exec_cmd(parser, &resp, "AT+ZIPSTAT=%d",connect_id);

    if (result != OS_EOK)
    {
        result = OS_ERROR;
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "+ZIPSTAT:", "+ZIPSTAT: %s", &zipstat) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }


    if ('1' == zipstat[2])
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
        ERROR("Module %s check id %d :used!", module->name, connect_id);
    }

    return result;
}

static mo_netconn_t *gm510_netconn_alloc(mo_object_t *module)
{
    mo_gm510_t *gm510 = os_container_of(module, mo_gm510_t, parent);

    for (int i = 1; i < GM510_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == gm510->netconn[i].stat)
        {
            if (OS_EOK == gm510_check_state(module, i))
            {
                gm510->netconn[i].connect_id = i;

                return &gm510->netconn[i];
            }
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *gm510_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (1 > connect_id || GM510_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_gm510_t *gm510 = os_container_of(module, mo_gm510_t, parent);
    for (int i = 0; i < GM510_NETCONN_NUM; i++)
    {
        if (connect_id == gm510->netconn[i].connect_id)
        {
            return &gm510->netconn[i];
        }
    }

    return OS_NULL;

}

os_err_t gm510_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_gm510_t *gm510 = os_container_of(module, mo_gm510_t, parent);

    info->netconn_array = gm510->netconn;
    info->netconn_nums  = sizeof(gm510->netconn) / sizeof(gm510->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *gm510_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_gm510_t *gm510 = os_container_of(module, mo_gm510_t, parent);
    gm510_lock(&gm510->netconn_lock);
    mo_netconn_t *netconn = gm510_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        gm510_unlock(&gm510->netconn_lock);
        return OS_NULL;
    }
    netconn->mq = os_mq_create(GM510_NETCONN_MQ_NAME,
                               GM510_NETCONN_MQ_MSG_SIZE,
                               GM510_NETCONN_MQ_MSG_MAX);

    if (netconn->stat != NETCONN_STAT_NULL)
    {
        mo_netconn_mq_destroy(netconn->mq);
        netconn->mq = OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;
    gm510_unlock(&gm510->netconn_lock);
    return netconn;
}

os_err_t gm510_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser      = &module->parser;
    os_err_t     result      = OS_ERROR;
    char         zipstat[10] = {0};

    mo_gm510_t * gm510      = os_container_of(module, mo_gm510_t, parent);
    gm510_lock(&gm510->netconn_lock);

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
            gm510_unlock(&gm510->netconn_lock);
            return result;
        }
        if (at_resp_get_data_by_kw(&resp, "+ZIPSTAT:", "+ZIPSTAT: %s", &zipstat) <= 0)
		{
		    result = OS_ERROR;
            gm510_unlock(&gm510->netconn_lock);
			return result;
		}

		if ('0' != zipstat[2])
		{
            result = OS_ERROR;
            gm510_unlock(&gm510->netconn_lock);
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

    gm510_unlock(&gm510->netconn_lock);

    return OS_EOK;
}



os_err_t gm510_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
#define ZIP_CALL_TIMES (6)
    at_parser_t *parser     = &module->parser;
    os_err_t    result      = OS_EOK;
    char        zipstat[10] = {0};
    mo_gm510_t * gm510      = os_container_of(module, mo_gm510_t, parent);
    gm510_lock(&gm510->netconn_lock);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_uint32_t event = SET_EVENT(netconn->connect_id, GM510_EVENT_CONN_OK | GM510_EVENT_CONN_FAIL);
    os_event_recv(&gm510->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);
    if (OS_TRUE != gm510_check_zipcall(module))
    {
        if (OS_FALSE == gm510_pdp_set(module))
        {
            ERROR("Module %s set !", module->name);
            result = OS_ERROR;
            goto __exit;
        }

        int i = 0;
        while (OS_TRUE != gm510_check_zipcall(module))
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

    do
    {
        os_task_msleep(1000);
        result = at_parser_exec_cmd(parser, &resp, "AT+ZIPSTAT=%d",netconn->connect_id);

        if (result != OS_EOK)
        {
            result = OS_ERROR;
            goto __exit;
        }

        if (at_resp_get_data_by_kw(&resp, "+ZIPSTAT:", "+ZIPSTAT: %s", &zipstat) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }
    }
    while('3' == zipstat[2]);

    if ('1' != zipstat[2])
    {
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
    gm510_unlock(&gm510->netconn_lock);

    return result;
}

os_size_t gm510_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;
    mo_gm510_t *gm510 = os_container_of(module, mo_gm510_t, parent);

    at_parser_exec_lock(parser);

    gm510->curr_connect = netconn->connect_id;

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

        result = at_parser_exec_cmd(parser, &resp, "AT+ZIPSENDRAW=%d,%d", netconn->connect_id, curr_size);
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

        result = os_event_recv(&gm510->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&gm510->netconn_evt,
                               GM510_EVENT_SEND_OK | GM510_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & GM510_EVENT_SEND_FAIL)
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
os_err_t gm510_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser                 = &self->parser;
    os_err_t     result                 = OS_EOK;
    char recvip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[128]                 = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 6 * OS_TICK_PER_SECOND};

    mo_gm510_t *gm510 = os_container_of(self, mo_gm510_t, parent);

    gm510->netconn_data = addr;

    if(OS_FALSE == gm510_check_zipcall(self))
    {
        result = OS_ERROR;
        goto __exit;
    };

    result = at_parser_exec_cmd(parser, &resp, "AT+ZDNSGETIP=%s", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "+ZDNSGETIP:", "+ZDNSGETIP: %s", &recvip) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    recvip[IPADDR_MAX_STR_LEN] = '\0';
    inet_aton(recvip, (ip_addr_t *)gm510->netconn_data);

__exit:
    if (result != OS_EOK)
    {
        gm510->netconn_data = OS_NULL;
    }

    return result;
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;
    mo_object_t *module   = os_container_of(parser, mo_object_t, parser);
    mo_gm510_t *gm510     = os_container_of(module, mo_gm510_t, parent);

    sscanf(data, "+ZIPSENDRAW: %d,%d", &connect_id, &data_size);

    os_int32_t curr_connect = gm510->curr_connect;

    if (data_size > 0)
    {
        os_event_send(&gm510->netconn_evt, SET_EVENT(curr_connect, GM510_EVENT_SEND_OK));
    }
    else
    {
        os_event_send(&gm510->netconn_evt, SET_EVENT(curr_connect, GM510_EVENT_SEND_FAIL));
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

    mo_netconn_t *netconn = gm510_get_netconn_by_id(module, connect_id);
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
static at_urc_t gs_urc_table[] = {
    {.prefix = "+ZIPSENDRAW:", .suffix = "\r\n",           .func = urc_send_func},
    {.prefix = "",             .suffix = "+ZIPRECV:",      .func = urc_recv_func},
};

static void gm510_network_init(mo_object_t *module)
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
        WARN("GM510 network init failed");
    }
    else
    {
        WARN("GM510 network init sucess");
    }
}

void gm510_netconn_init(mo_gm510_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 1; i < GM510_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    gm510_network_init(&module->parent);

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* GM510_USING_NETCONN_OPS */

