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
 * @file        gm120_netconn.c
 *
 * @brief       gm120 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "gm120_netconn.h"
#include "gm120.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "gm120_netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define SEND_DATA_MAX_SIZE (1400)
#define SEND_DATA_MIN_SIZE (1)

#define GM120_NETCONN_MQ_NAME "gm120_nc_mq"

#ifndef GM120_NETCONN_MQ_MSG_SIZE
#define GM120_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef GM120_NETCONN_MQ_MSG_MAX
#define GM120_NETCONN_MQ_MSG_MAX  (5)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define GM120_EVENT_CONN_OK    (1L << 0)
#define GM120_EVENT_SEND_OK    (1L << 1)
#define GM120_EVENT_RECV_OK    (1L << 2)
#define GM120_EVENT_CLOSE_OK   (1L << 3)


#ifdef GM120_USING_NETCONN_OPS


static os_err_t gm120_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t gm120_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *gm120_netconn_alloc(mo_object_t *module)
{
    mo_gm120_t *gm120 = os_container_of(module, mo_gm120_t, parent);

    for (int i = 0; i < GM120_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == gm120->netconn[i].stat)
        {
            gm120->netconn[i].connect_id = -1 - i;
            return &gm120->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *gm120_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || GM120_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_gm120_t *gm120 = os_container_of(module, mo_gm120_t, parent);
    
    for (int i = 0; i < GM120_NETCONN_NUM; i++)
    {
        if (connect_id == gm120->netconn[i].connect_id)
        {
            return &gm120->netconn[i];
        }
    }

    return OS_NULL;

}

os_err_t gm120_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_gm120_t *gm120 = os_container_of(module, mo_gm120_t, parent);

    info->netconn_array = gm120->netconn;
    info->netconn_nums  = sizeof(gm120->netconn) / sizeof(gm120->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *gm120_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_gm120_t *gm120 = os_container_of(module, mo_gm120_t, parent);
   
    gm120_lock(&gm120->netconn_lock);
    
    mo_netconn_t *netconn = gm120_netconn_alloc(module);
    if (OS_NULL == netconn)
    {
        gm120_unlock(&gm120->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(GM120_NETCONN_MQ_NAME,
                               GM120_NETCONN_MQ_MSG_SIZE,
                               GM120_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        gm120_unlock(&gm120->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;
    gm120_unlock(&gm120->netconn_lock);
    return netconn;
}

os_err_t gm120_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser      = &module->parser;
    os_err_t     result      = OS_ERROR;
    
    mo_gm120_t * gm120      = os_container_of(module, mo_gm120_t, parent);
    gm120_lock(&gm120->netconn_lock);
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 10 * OS_TICK_PER_SECOND
                     };
    os_uint32_t event = SET_EVENT(netconn->connect_id, GM120_EVENT_CONN_OK);
    os_event_recv(&gm120->netconn_evt, event, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);
    
    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
        break;
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+SCLOSE=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            ERROR("Module %s destroy %s netconn failed",
                      module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
            gm120_unlock(&gm120->netconn_lock);
            return result;
        }
        
        result = os_event_recv(&gm120->netconn_evt,
                           SET_EVENT(netconn->connect_id, 0),
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
        if (result != OS_EOK)
        {
            gm120_unlock(&gm120->netconn_lock);
            return result;
        }

       result = os_event_recv(&gm120->netconn_evt,
                           GM120_EVENT_CLOSE_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           1 * OS_TICK_PER_SECOND,
                           &event);
        if (result != OS_EOK)
        {
            gm120_unlock(&gm120->netconn_lock);
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
    
    gm120_unlock(&gm120->netconn_lock);
    
    return OS_EOK;
}


os_err_t gm120_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser     = &module->parser;
    os_err_t    result      = OS_EOK;  
    mo_gm120_t * gm120      = os_container_of(module, mo_gm120_t, parent);
    os_uint8_t  socket_id;
    
	gm120_lock(&gm120->netconn_lock);
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_event_recv(&gm120->netconn_evt, GM120_EVENT_CONN_OK, OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR, OS_NO_WAIT, OS_NULL);		
    
    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+SOPEN=0,%s,%d",
                                    remote_ip,
                                    port);
        break;
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+SOPEN=1,%s,%d",
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
    
    if (at_resp_get_data_by_kw(&resp, "+SOPEN:", "+SOPEN: %d", &socket_id) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }
    
    if (socket_id != 1 && socket_id != 0)
    {
        result = OS_ERROR;
        goto __exit;
    }
    
    netconn->connect_id = socket_id;

    result = os_event_recv(&gm120->netconn_evt,
                           GM120_EVENT_CONN_OK,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           60 * OS_TICK_PER_SECOND,
                           OS_NULL);
    if (result != OS_EOK)
    {
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
    gm120_unlock(&gm120->netconn_lock);

    return result;
}

os_size_t gm120_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
 
    mo_gm120_t *gm120 = os_container_of(module, mo_gm120_t, parent);

    at_parser_exec_lock(parser);

    gm120->curr_connect = netconn->connect_id;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND
                     };

    if (size > SEND_DATA_MAX_SIZE || size < SEND_DATA_MIN_SIZE)
    {
        result = OS_ERROR;
        ERROR("Send data size (%d) is out of range [1,1400]",size);
        goto  __exit;
    }
    
    for(int i = 0; i< size - 1; i++)
    {
        if((*(data + i)) == '\r' && (*(data + i + 1)) == '\n')
        {
            ERROR("Error: send data contain [CR LF]");            
            result = OS_ERROR;
            goto  __exit; 
        }            
    }

    result = at_parser_exec_cmd(parser, &resp, "AT+SSEND=%d,%d,\"%s\"", netconn->connect_id, size, data);
    if (result != OS_EOK)
    {
        goto __exit;
    }
    

__exit:

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return size;
}
os_err_t gm120_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser                 = &self->parser;
    os_err_t     result                 = OS_EOK;
    char recvip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[128]                 = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 15 * OS_TICK_PER_SECOND};

    mo_gm120_t *gm120 = os_container_of(self, mo_gm120_t, parent);

    gm120->netconn_data = addr;
    
    result = at_parser_exec_cmd(parser, &resp, "AT+DNS=\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }
    
    if (at_resp_get_data_by_kw(&resp, "+DNS:", "+DNS: %s", &recvip) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }
    
    recvip[IPADDR_MAX_STR_LEN] = '\0';
    inet_aton(recvip, (ip_addr_t *)gm120->netconn_data);

__exit:
    if (result != OS_EOK)
    {
        gm120->netconn_data = OS_NULL;
    }

    return result;
}


static void urc_stat_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
  
    os_int32_t   stat        = 0;
    os_int32_t   connect_id  = -1;
    mo_object_t  *module     = os_container_of(parser, mo_object_t, parser);
    mo_gm120_t   *gm120      = os_container_of(module, mo_gm120_t, parent);

    sscanf(data, "+SSTATE: %d,%d", &connect_id, &stat);

    if (stat == 1)
    {
        os_event_send(&gm120->netconn_evt, GM120_EVENT_CONN_OK);
    }
    else
    {
        os_event_send(&gm120->netconn_evt, SET_EVENT(connect_id, GM120_EVENT_CLOSE_OK));
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
        /* handle last "\"" */
        at_parser_recv(parser, temp_buff, 1, timeout);
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

    os_int32_t connect_id = tmp_ch - '0';
    
    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    
    mo_netconn_t *netconn = gm120_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }
    
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
    
    at_parser_recv(parser, &tmp_ch, 1, 0);
    
    urc_recv_data_func(parser, netconn, data_size);

    return;
}
static at_urc_t gs_urc_table[] = {
    {.prefix = "+SSTATE:", .suffix = "\r\n",         .func = urc_stat_func},
    {.prefix = "",         .suffix = "+SRECV:",      .func = urc_recv_func},
};

static void gm120_network_init(mo_object_t *module)
{
    at_parser_t *parser        = &module->parser;
    os_int32_t   enable_num    = 0;
    os_int32_t   reg_state     = 0;
    os_err_t     result        = OS_ERROR;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    result = at_parser_exec_cmd(parser, &resp, "AT+DTMODE=1");
    if (result != OS_EOK)
    {
        INFO("Send data type is not string");
    }
    
    result = at_parser_exec_cmd(parser, &resp, "AT+CEREG?");
    if (result != OS_EOK)
    {
        goto __exit;
    }
    
    if (at_resp_get_data_by_kw(&resp, "+CEREG:", "+CEREG: %d,%d", &enable_num , &reg_state) < 0)
    {
        result = OS_ERROR;
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
        WARN("GM120 network init failed");
    }
    else
    {
        WARN("GM120 network init sucess");
    }
}

void gm120_netconn_init(mo_gm120_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 1; i < GM120_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    gm120_network_init(&module->parent);

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* GM120_USING_NETCONN_OPS */

