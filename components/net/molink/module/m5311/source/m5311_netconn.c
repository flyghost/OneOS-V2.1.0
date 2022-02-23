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
 * @file        m5311_netconn.c
 *
 * @brief       m5311 module link kit netconnect api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "m5311_netconn.h"
#include "m5311.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "mo_lib.h"

#ifdef M5311_USING_NETCONN_OPS

#define MO_LOG_TAG "m5311.netconn"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define M5311_NETCONN_TIMEOUT_DFT (2 * OS_TICK_PER_SECOND)

#define M5311_EVENT_CONN_OK       (1L << 0)
#define M5311_EVENT_CONN_FAIL     (1L << 1)
#define M5311_EVENT_DOMAIN_OK     (1L << 2)
#define M5311_EVENT_DOMAIN_FAIL   (1L << 3)

#define M5311_NETCONN_ID_INVALID  (-1)
#define M5311_TCP_SEND_MAX_SIZE   (720)
#define M5311_UDP_SEND_MAX_SIZE   (712)
#define M5311_SEND_BLOCK_SIZE     (64)

#define M5311_NETCONN_MQ_NAME     "m5311_nc_mq"
#ifndef M5311_NETCONN_MQ_MSG_SIZE
#define M5311_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif /* M5311_NETCONN_MQ_MSG_SIZE */
#ifndef M5311_NETCONN_MQ_MSG_MAX
#define M5311_NETCONN_MQ_MSG_MAX  (5)
#endif /* M5311_NETCONN_MQ_MSG_MAX */

static os_err_t m5311_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t m5311_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *m5311_netconn_alloc(mo_object_t *module)
{
    os_int32_t current_connect_id = M5311_NETCONN_ID_INVALID;
    mo_m5311_t *m5311 = os_container_of(module, mo_m5311_t, parent);

    for (int i = 0; i < M5311_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == m5311->netconn[i].stat)
        {
            /* reset netconn prevent reuse content */
            current_connect_id = m5311->netconn[i].connect_id;
            memset(&m5311->netconn[i], 0, sizeof(mo_netconn_t));
            m5311->netconn[i].connect_id = current_connect_id;

            INFO("Moduel %s NO[%d]:connect_id[%d]!", module->name, i, m5311->netconn[i].connect_id);

            return &m5311->netconn[i];
        }
    }

    ERROR("Moduel %s alloc netconn failed!", module->name);

    return OS_NULL;
}

static mo_netconn_t *m5311_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || M5311_NETCONN_NUM <= connect_id)
    {
        ERROR("%s-%d: INVALID connet_id:[%d]!", __func__, __LINE__, connect_id);
        return OS_NULL;
    }

    mo_m5311_t *m5311 = os_container_of(module, mo_m5311_t, parent);

    for (int i = 0; i < M5311_NETCONN_NUM; i++)
    {
        if (connect_id == m5311->netconn[i].connect_id)
        {
            return &m5311->netconn[i];
        }
    }
    INFO("Moduel %s netconn all connect_id was occupied.", module->name);

    return OS_NULL;
}

os_err_t m5311_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_m5311_t *m5311 = os_container_of(module, mo_m5311_t, parent);

    info->netconn_array = m5311->netconn;
    info->netconn_nums  = sizeof(m5311->netconn) / sizeof(m5311->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *m5311_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_m5311_t   *m5311 = os_container_of(module, mo_m5311_t, parent);
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_EOK;

    m5311_lock(&m5311->netconn_lock);

    mo_netconn_t *netconn = m5311_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        m5311_unlock(&m5311->netconn_lock);
        return OS_NULL;
    }

    if (type != NETCONN_TYPE_TCP && type != NETCONN_TYPE_UDP)
    {
        m5311_unlock(&m5311->netconn_lock);
        return OS_NULL;
    }

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETCONN_TIMEOUT_DFT};

    result = at_parser_exec_cmd(parser, &resp, "AT+IPRCFG=1,0,1");

    if (OS_EOK != result)
    {
        ERROR("Module %s set netconn autorcv data HEX format failed", module->name);
        m5311_unlock(&m5311->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(M5311_NETCONN_MQ_NAME,
                               M5311_NETCONN_MQ_MSG_SIZE,
                               M5311_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s message queue create failed, no enough memory.", module->name);
        m5311_unlock(&m5311->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    m5311_unlock(&m5311->netconn_lock);
    return netconn;
}

os_err_t m5311_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    INFO("Module %s in %d netconn status", module->name, netconn->stat);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETCONN_TIMEOUT_DFT};

    switch (netconn->stat)
    {
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+IPCLOSE=%d", netconn->connect_id);
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

    netconn->stat        = NETCONN_STAT_NULL;
    netconn->type        = NETCONN_TYPE_NULL;
    netconn->remote_port = 0;
    inet_aton("0.0.0.0", &netconn->remote_ip);

    INFO("Module %s netconn_id:%d destroyed", module->name, netconn->connect_id);

    return OS_EOK;
}

os_err_t m5311_netconn_gethostbyname(mo_object_t *module, const char *domain_name, ip_addr_t *addr)
{
    os_err_t     result = OS_EOK;
    os_uint32_t  event  = 0;
    at_parser_t *parser = &module->parser;
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);

    char resp_buff[4 * AT_RESP_BUFF_SIZE_DEF] = {0};
    char addr_str[IPADDR_MAX_STR_LEN + 1]     = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    m5311_lock(&m5311->netconn_lock);

    os_event_clear(&m5311->netconn_evt, M5311_EVENT_DOMAIN_OK | M5311_EVENT_DOMAIN_FAIL);

    m5311->netconn_data = addr_str;

    result = at_parser_exec_cmd(parser, &resp, "AT+CMDNS=\"%s\"", domain_name);
    if (OS_EOK != result)
    {
        ERROR("%s-%d: execute dns cmd failed!", __func__, __LINE__);
        goto __exit;
    }

    result = os_event_recv(&m5311->netconn_evt,
                           M5311_EVENT_DOMAIN_OK | M5311_EVENT_DOMAIN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           41 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("%s-%d: get dns result timeout!", __func__, __LINE__);
        goto __exit;
    }

    if (M5311_EVENT_DOMAIN_FAIL & event)
    {
        ERROR("%s-%d: get dns result failed!", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (IPADDR_MIN_STR_LEN > strlen(addr_str) || IPADDR_MAX_STR_LEN < strlen(addr_str))
    {
        ERROR("M5311 domain resolve: recv len invalid:%d", strlen(addr_str));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("M5311 domain resolve: \"%s\" domain ip is %s, addrlen %d", domain_name, addr_str, strlen(addr_str));
        inet_aton(addr_str, addr);

        if (IPADDR_ANY == addr->addr || IPADDR_LOOPBACK == addr->addr)
        {
            ip_addr_set_zero(addr);
            result = OS_ERROR;
            goto __exit;
        }

        result = OS_EOK;
    }

__exit:

    m5311->netconn_data = OS_NULL;

    m5311_unlock(&m5311->netconn_lock);

    return result;
}

static os_err_t m5311_tcp_connect(mo_object_t *module, os_int32_t connect_id, char *ip_addr, os_uint16_t port)
{
    os_err_t     result = OS_EOK;
    os_uint32_t  event  = 0;
    at_parser_t *parser = &module->parser;
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);

    char resp_buff[2 * AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 41 * OS_TICK_PER_SECOND}; /* about 40s return failed */

    /* M5311 return 'CONNECT OK' behind 'OK' without id. so we add lock for mutithread. */
    m5311_lock(&m5311->netconn_lock);

    os_event_clear(&m5311->netconn_evt, M5311_EVENT_CONN_OK | M5311_EVENT_CONN_FAIL);

    result = at_parser_exec_cmd(parser, &resp, "AT+IPSTART=%d,\"TCP\",%s,%hu", connect_id, ip_addr, port);
    if (result != OS_EOK)
    {
        ERROR("%s-%d: connect id[%d], execute connect cmd failed!", __func__, __LINE__, connect_id);
        goto __exit;
    }

    result = os_event_recv(&m5311->netconn_evt,
                           M5311_EVENT_CONN_OK | M5311_EVENT_CONN_FAIL,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           41 * OS_TICK_PER_SECOND,
                           &event);
    if (result != OS_EOK)
    {
        ERROR("connect[%d] to %s timeout!", connect_id, ip_addr);
        goto __exit;
    }

    if (M5311_EVENT_CONN_FAIL & event)
    {
        ERROR("%s-%d: connect[%d] to %s failed!", __func__, __LINE__, connect_id, ip_addr);
        result = OS_ERROR;
        goto __exit;
    }
    INFO("connect[%d] to %s", connect_id, ip_addr);

__exit:

    m5311_unlock(&m5311->netconn_lock);

    return result;
}

static os_err_t m5311_udp_connect(mo_object_t *module, os_int32_t connect_id, char *ip_addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = M5311_NETCONN_TIMEOUT_DFT};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+IPSTART=%d,\"UDP\",%s,%hu", connect_id, ip_addr, port);

    return result;
}

os_err_t m5311_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    os_err_t     result = OS_EOK;

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = m5311_tcp_connect(module, netconn->connect_id, remote_ip, port);
        break;
    case NETCONN_TYPE_UDP:
        result = m5311_udp_connect(module, netconn->connect_id, remote_ip, port);
        break;
    default:
        result = OS_ERROR;
        break;
    }

    if (result != OS_EOK)
    {
        ERROR("Module %s connect to %s:%hu failed!", module->name, remote_ip, port);
        return result;
    }

    ip_addr_copy(netconn->remote_ip, addr);
    netconn->remote_port = port;
    netconn->stat        = NETCONN_STAT_CONNECT;

    DEBUG("Module %s connect to %s:%hu successfully!", module->name, remote_ip, port);

    return OS_EOK;
}

static os_size_t m5311_single_packet_send(at_parser_t *parser, const char *data, os_size_t size)
{
    os_size_t  sent_size     = 0; /* raw data sent size */
    os_size_t  curr_pkt_size = 0; /* raw data current packet size */

    char hex_str_buff[M5311_SEND_BLOCK_SIZE] = {0};

    while (size > sent_size)
    {
        curr_pkt_size = (size - sent_size) < (M5311_SEND_BLOCK_SIZE / 2) ? (size - sent_size) : M5311_SEND_BLOCK_SIZE / 2;

        bytes_to_hexstr(data + sent_size, hex_str_buff, curr_pkt_size);

        if (at_parser_send(parser, hex_str_buff, 2 * curr_pkt_size) <= 0)
        {
            goto __exit;
        }

        sent_size += curr_pkt_size;
    }

__exit:

    return sent_size;
}

static os_size_t m5311_hexdata_send(at_parser_t *parser, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    os_err_t   result            = OS_EOK;
    os_int32_t connect_id        = M5311_NETCONN_ID_INVALID;
    os_size_t  sent_size         = 0;
    os_size_t  cur_pkt_size      = 0;
    os_size_t  cnt               = 0;
    char prefix_send_cmd[AT_RESP_BUFF_SIZE_DEF] = {0};
    char suffix_send_cmd[AT_RESP_BUFF_SIZE_DEF] = {0};
    char remote_ip[IPADDR_MAX_STR_LEN + 1]      = {0};
    char resp_buff[AT_RESP_BUFF_SIZE_DEF]       = {0};
    at_resp_t resp = {.buff = resp_buff, .buff_size = AT_RESP_BUFF_SIZE_DEF, .timeout =  10 * OS_TICK_PER_SECOND};

    /* M5311 UDP send size <= 712, different from the spec size 720 */
    const os_int32_t send_max_size = netconn->type == NETCONN_TYPE_TCP ? M5311_TCP_SEND_MAX_SIZE : M5311_UDP_SEND_MAX_SIZE;

    strncpy(remote_ip, inet_ntoa(netconn->remote_ip), IPADDR_MAX_STR_LEN);

    at_parser_exec_lock(parser);

    while (sent_size < size)
    {
        if (netconn->stat != NETCONN_STAT_CONNECT)
        {
            ERROR("Module %s netconn %d isn't in the CONNECT state, send data fail", parser->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        if (size - sent_size < send_max_size)
        {
            cur_pkt_size = size - sent_size;
        }
        else
        {
            cur_pkt_size = send_max_size;
        }

        snprintf(prefix_send_cmd, sizeof(prefix_send_cmd), "AT+IPSEND=%d,%d,", netconn->connect_id, (int)cur_pkt_size);
        snprintf(suffix_send_cmd, sizeof(suffix_send_cmd), ",%s,%hu", remote_ip, netconn->remote_port);

        /* send prefix cmd */
        if (at_parser_send(parser, prefix_send_cmd, strlen(prefix_send_cmd)) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        /* send data */
        if (cur_pkt_size != m5311_single_packet_send(parser, data + sent_size, cur_pkt_size))
        {
            result = OS_ERROR;
            ERROR("Drv or hardware send data fail, some data is missing.\r\n");
            goto __exit;
        }

        /* send suffix cmd: UDP needs tail for addr&port */
        if (netconn->type == NETCONN_TYPE_UDP)
        {
            if (at_parser_send(parser, suffix_send_cmd, strlen(suffix_send_cmd)) <= 0)
            {
                result = OS_ERROR;
                goto __exit;
            }
        }

        result = at_parser_exec_cmd(parser, &resp, "");
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_resp_get_data_by_kw(&resp, "+IPSEND:", "+IPSEND: %d,%d", &connect_id, &cnt) <= 0 || cnt != cur_pkt_size)
        {
            result = OS_ERROR;
            goto __exit;
        }

        sent_size += cur_pkt_size;
    }

__exit:

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s netconn %d send %d bytes data failed!",
               parser->name,
               netconn->connect_id,
               cur_pkt_size);

        return 0;
    }

    return sent_size;
}

os_size_t m5311_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_size_t    sent_size = 0;
    mo_m5311_t  *m5311     = os_container_of(module, mo_m5311_t, parent);

    if (OS_EOK != m5311_lock(&m5311->netconn_lock))
    {
        ERROR("Moudle %s netconn %d send lock failed.", module->name, netconn->connect_id);
        return 0;
    }

    if (NETCONN_TYPE_TCP == netconn->type || NETCONN_TYPE_UDP == netconn->type)
    {
        sent_size = m5311_hexdata_send(parser, netconn, data, size);
    }
    else
    {
        ERROR("Moudle %s netconn %d type %d error.", module->name, netconn->connect_id, netconn->type);
    }

    if (OS_EOK != m5311_unlock(&m5311->netconn_lock))
    {
        ERROR("Moudle %s netconn %d send unlock failed.", module->name, netconn->connect_id);
        return 0;
    }

    return sent_size;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = M5311_NETCONN_ID_INVALID;

    sscanf(data, "+IPCLOSE: %d", &connect_id);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = m5311_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s get netconn error, when receive urc close code of conn_id:%d", module->name, connect_id);
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

    os_int32_t connect_id = M5311_NETCONN_ID_INVALID;
    os_int32_t data_size  = 0;

    sscanf(data, "+IPRD: %d,%d,", &connect_id, &data_size);
    INFO("Moudle %s netconn %d receive %d bytes data", parser->name, connect_id, data_size);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);

    mo_netconn_t *netconn = m5311_get_netconn_by_id(module, connect_id);
    if (OS_NULL == netconn)
    {
        ERROR("Module %s request receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    /* bufflen >= HEX string size + 1 */
    char *recv_buff = os_calloc(1, data_size * 2 + 1);
    if (recv_buff == OS_NULL)
    {
        ERROR("alloc recv buff %d bytes fail, no enough memory", data_size * 2 + 1);
        return;
    }

    /* Get receive data to receive buffer */
    /* Alert! if using sscanf stores strings, be rember allocating enouth memory! */
    sscanf(data, "+IPRD: %*d,%*d,%s", recv_buff);

    char *recv_str = os_calloc(1, data_size + 1);
    if (recv_str == OS_NULL)
    {
        ERROR("alloc recv str %d bytes fail, no enough memory", data_size + 1);
        os_free(recv_buff);
        return;
    }

    /* from mo_lib */
    hexstr_to_bytes(recv_buff, recv_str, data_size * 2);

    mo_netconn_data_recv_notice(netconn, recv_str, data_size);

    os_free(recv_buff);

    return;
}

static void urc_dns_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_err_t     result = OS_EOK;
    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);
    char ret_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    if (OS_NULL == m5311->netconn_data)
    {
        ERROR("%s-%d: no useable buffer.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    /* AT+CMDNS="www.baidu.com" return: OK \r\n  +CMDNS:183.232.231.172 \r\n */
    /* AT+CMDNS="8.8.8.8" return: +CMDNS:8.8.8.8 \r\n  OK */
    if (0 >= sscanf(data, "+CMDNS: %s", ret_buff))
    {
        ERROR("%s-%d: get dns result failed.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    if (IPADDR_MAX_STR_LEN < strlen(ret_buff))
    {
        ERROR("%s-%d: dns return invalid len.", __func__, __LINE__);
        result = OS_ERROR;
        goto __exit;
    }

    memcpy(m5311->netconn_data, ret_buff, strlen(ret_buff));

__exit:

    if (OS_EOK == result)
    {
        os_event_send(&m5311->netconn_evt, M5311_EVENT_DOMAIN_OK);
    }
    else
    {
        os_event_send(&m5311->netconn_evt, M5311_EVENT_DOMAIN_FAIL);
    }

    return;
}

static void urc_connect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_m5311_t  *m5311  = os_container_of(module, mo_m5311_t, parent);

    if (OS_NULL != strstr(data, "CONNECT OK"))
    {
        os_event_send(&m5311->netconn_evt, M5311_EVENT_CONN_OK);
    }
    else
    {
        os_event_send(&m5311->netconn_evt, M5311_EVENT_CONN_FAIL);
    }

    return;
}

static at_urc_t nc_urc_table[] = {
    {.prefix = "+IPCLOSE:",    .suffix = "\r\n", .func = urc_close_func  },
    {.prefix = "+IPRD:",       .suffix = "\r\n", .func = urc_recv_func   },
    {.prefix = "+CMDNS:",      .suffix = "\r\n", .func = urc_dns_func    },
    {.prefix = "CONNECT OK",   .suffix = "\r\n", .func = urc_connect_func},
    {.prefix = "CONNECT FAIL", .suffix = "\r\n", .func = urc_connect_func},
};

void m5311_netconn_init(mo_m5311_t *module)
{
    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < M5311_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = i;
    }

    /* Set netconn urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, nc_urc_table, sizeof(nc_urc_table) / sizeof(nc_urc_table[0]));
}

#endif /* M5311_USING_NETCONN_OPS */
