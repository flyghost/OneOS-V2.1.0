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
 * @file        me3616_netconn.c
 *
 * @brief       me3616 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "me3616_netconn.h"
#include "me3616.h"
#include "mo_lib.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "me3616_netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define SEND_DATA_MAX_SIZE (512)
#define SEND_DATA_MIN_SIZE (1)

#define ME3616_NETCONN_MQ_NAME "me3616_nc_mq"

#ifndef ME3616_NETCONN_MQ_MSG_SIZE
#define ME3616_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef ME3616_NETCONN_MQ_MSG_MAX
#define ME3616_NETCONN_MQ_MSG_MAX  (5)
#endif

#ifdef ME3616_USING_NETCONN_OPS


static os_err_t me3616_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t me3616_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *me3616_netconn_alloc(mo_object_t *module, mo_netconn_type_t type)
{
    at_parser_t *parser      = &module->parser;
    mo_me3616_t *me3616      = os_container_of(module, mo_me3616_t, parent);
    os_uint8_t   socket_id   = 0; 
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 10 * OS_TICK_PER_SECOND
                     };
    
    for (int i = 0; i < ME3616_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == me3616->netconn[i].stat)
        { 
            if(OS_EOK != at_parser_exec_cmd(parser, &resp, "AT+ESOC=1,%d,1", type))  
            {
                continue;
            }

            if (at_resp_get_data_by_kw(&resp, "+ESOC=", "+ESOC=%d", &socket_id) <= 0)
            {
                continue;
            }            
            me3616->netconn[i].connect_id = socket_id;
            return &me3616->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *me3616_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || ME3616_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_me3616_t *me3616 = os_container_of(module, mo_me3616_t, parent);
    
    for (int i = 0; i < ME3616_NETCONN_NUM; i++)
    {
        if (connect_id == me3616->netconn[i].connect_id)
        {
            return &me3616->netconn[i];
        }
    }

    return OS_NULL;

}

os_err_t me3616_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_me3616_t *me3616 = os_container_of(module, mo_me3616_t, parent);

    info->netconn_array = me3616->netconn;
    info->netconn_nums  = sizeof(me3616->netconn) / sizeof(me3616->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *me3616_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_me3616_t *me3616 = os_container_of(module, mo_me3616_t, parent);
    me3616_lock(&me3616->netconn_lock);
    mo_netconn_t *netconn = me3616_netconn_alloc(module, type);

    netconn->mq = os_mq_create(ME3616_NETCONN_MQ_NAME,
                               ME3616_NETCONN_MQ_MSG_SIZE,
                               ME3616_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        me3616_unlock(&me3616->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;
    me3616_unlock(&me3616->netconn_lock);
    return netconn;
}

os_err_t me3616_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser      = &module->parser;
    os_err_t     result      = OS_ERROR;
    
    mo_me3616_t * me3616      = os_container_of(module, mo_me3616_t, parent);
    me3616_lock(&me3616->netconn_lock);
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 10 * OS_TICK_PER_SECOND
                     };
    
    switch (netconn->stat)
    {
    case NETCONN_STAT_INIT:
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+ESOCL=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            ERROR("Module %s destroy %s netconn failed",
                      module->name,
                      (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
            me3616_unlock(&me3616->netconn_lock);
            return result;
        }
   
        break;
    default:
        /* add handler when we need */
        break;
    }

    if (netconn->stat != NETCONN_STAT_NULL)
    {
        os_mq_destroy(netconn->mq);
        netconn->mq = OS_NULL;
    }

    INFO("Module %s netconn id %d destroyed", module->name, netconn->connect_id);

    netconn->connect_id  = -1;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);
    
    me3616_unlock(&me3616->netconn_lock);
    
    return OS_EOK;
}



os_err_t me3616_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser     = &module->parser;
    os_err_t    result      = OS_EOK;  
    mo_me3616_t * me3616      = os_container_of(module, mo_me3616_t, parent);
    
	me3616_lock(&me3616->netconn_lock);
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 60 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);	
  
    result = at_parser_exec_cmd(parser, &resp, "AT+ESOCON=%d,%d,\"%s\"", 
                                               netconn->connect_id,
                                               port,
                                               remote_ip);    
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
    me3616_unlock(&me3616->netconn_lock);

    return result;
}

os_size_t me3616_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser         = &module->parser;
    os_err_t     result         = OS_EOK;
    
    mo_me3616_t *me3616         = os_container_of(module, mo_me3616_t, parent);

    at_parser_exec_lock(parser);
    
    me3616->curr_connect        = netconn->connect_id;   
    char        *hexstr         = os_calloc(1, size * 2 + 1);    
    char         resp_buff[128] = {0};    
    at_resp_t    resp           = {.buff      = resp_buff,
                                   .buff_size = sizeof(resp_buff),
                                   .timeout   = 5 * OS_TICK_PER_SECOND
                                  }; 
    
    if(size > SEND_DATA_MAX_SIZE || size < SEND_DATA_MIN_SIZE) 
    {
        ERROR("Moudle %s send data len: %d  out of the range [1,512]!",
                  module->name,
                  size );       
        result = OS_ERROR;
        goto __exit;
    }
    
    if (OS_NULL == hexstr)
    {
        ERROR("Moudle %s netconn %d calloc %d bytes memory failed!",
                  module->name,
                  netconn->connect_id,
                  size * 2 + 1);
        result = OS_ERROR;
        goto __exit;
    }

    bytes_to_hexstr(data, hexstr, size);
 
    result = at_parser_exec_cmd(parser, &resp, "AT+ESOSEND=%d,%d,%s", netconn->connect_id, size, hexstr);
    if (result != OS_EOK)
    {
        goto __exit;
    }                     

__exit:

    at_parser_exec_unlock(parser);

    os_free(hexstr);
    if (result != OS_EOK)
    {
        ERROR("Module %s connect id %d send %d bytes data failed", module->name, netconn->connect_id, size);
        return 0;
    }

    return size;
}
os_err_t me3616_netconn_gethostbyname(mo_object_t *self, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    at_parser_t *parser                 = &self->parser;
    os_err_t     result                 = OS_EOK;
    char recvip[IPADDR_MAX_STR_LEN + 1] = {0};
    char resp_buff[128]                 = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 30 * OS_TICK_PER_SECOND};

    mo_me3616_t *me3616 = os_container_of(self, mo_me3616_t, parent);

    me3616->netconn_data = addr;
    
    result = at_parser_exec_cmd(parser, &resp, "AT+EDNS=\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }
    
    if (at_resp_get_data_by_kw(&resp, "IPV4:", "IPV4:%s", &recvip) <= 0)
    {
        result = OS_ERROR;
        goto __exit;
    }

    recvip[IPADDR_MAX_STR_LEN] = '\0';
    inet_aton(recvip, (ip_addr_t *)me3616->netconn_data);

__exit:
    if (result != OS_EOK)
    {
        me3616->netconn_data = OS_NULL;
    }

    return result;
}


static void urc_err_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
  
    os_int32_t   error_code    = 0;
    os_int32_t   connect_id    = -1;
    mo_object_t  *module       = os_container_of(parser, mo_object_t, parser);

    sscanf(data, "+ESOERR=%d,%d", &connect_id, &error_code);
          
    mo_netconn_t *netconn = me3616_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        return;
    }
    ERROR("Module %s connect_id: %d recv error_code: %d and the netconn destroyed", module->name, connect_id, error_code);
    
    netconn->connect_id  = -1;
    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = -1;
    os_int32_t data_size  = 0;

    sscanf(data, "+ESONMI=%d,%d,", &connect_id, &data_size);
    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = me3616_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s request receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    /*  bufflen >= strsize + 1 */
    char *recv_buff = os_calloc(1, data_size * 2 + 1);
    if (recv_buff == OS_NULL)
    {
        ERROR("Calloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
        return;
    }

    /* Get receive data to receive buffer */
    /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
    sscanf(data, "+ESONMI=%*d,%*d,%s", recv_buff);

    char *recv_str = os_calloc(1, data_size + 1);
    if (recv_str == OS_NULL)
    {
        ERROR("Calloc recv str %d bytes fail, no enough memory", data_size + 1);
        os_free(recv_buff);
        return;
    }

    /* from mo_lib */
    hexstr_to_bytes(recv_buff, recv_str, data_size * 2);
    
    mo_netconn_data_recv_notice(netconn, recv_str, data_size);

    os_free(recv_buff);
    
    return; 
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+ESONMI=",     .suffix = "\r\n",      .func = urc_recv_func},
    {.prefix = "+ESOERR=",     .suffix = "\r\n",      .func = urc_err_func},
};

static void me3616_network_init(mo_object_t *module)
{
    at_parser_t *parser        = &module->parser;
    os_int32_t   enable_num    = 0;
    os_int32_t   reg_state     = 0;
    os_err_t     result        = OS_ERROR;
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};
    
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
        WARN("ME3616 network init failed");
    }
    else
    {
        WARN("ME3616 network init sucess");
    }
}

void me3616_netconn_init(mo_me3616_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 1; i < ME3616_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    me3616_network_init(&module->parent);

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* ME3616_USING_NETCONN_OPS */
