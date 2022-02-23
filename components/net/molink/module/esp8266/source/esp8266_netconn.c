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
 * @file        esp8266.c
 *
 * @brief       esp8266 module link kit netconn api implement
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "esp8266_netconn.h"
#include "esp8266.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MO_LOG_TAG "esp8266.netconn"
#define MO_LOG_LVL MO_LOG_INFO
#include <mo_log.h>

#define SEND_DATA_MAX_SIZE (2048)

#define ESP8266_NETCONN_MQ_NAME "esp8266_nc_mq"

#ifndef ESP8266_NETCONN_MQ_MSG_SIZE
#define ESP8266_NETCONN_MQ_MSG_SIZE (sizeof(mo_notconn_msg_t))
#endif

#ifndef ESP8266_NETCONN_MQ_MSG_MAX
#define ESP8266_NETCONN_MQ_MSG_MAX  (10)
#endif

#define SET_EVENT(socket, event) (((socket + 1) << 16) | (event))

#define ESP8266_EVENT_SEND_OK   (1L << 0)
#define ESP8266_EVENT_RECV_OK   (1L << 1)
#define ESP8266_EVNET_CLOSE_OK  (1L << 2)
#define ESP8266_EVENT_CONN_FAIL (1L << 3)
#define ESP8266_EVENT_SEND_FAIL (1L << 4)

#define ESP8266_MULTI_CONN_ENABLE  (1)
#define ESP8266_MULTI_CONN_DISABLE (0)

#ifdef ESP8266_USING_NETCONN_OPS

static os_err_t esp8266_lock(os_mutex_t *mutex)
{
    return os_mutex_recursive_lock(mutex, OS_WAIT_FOREVER);
}

static os_err_t esp8266_unlock(os_mutex_t *mutex)
{
    return os_mutex_recursive_unlock(mutex);
}

static mo_netconn_t *esp8266_netconn_alloc(mo_object_t *module)
{
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    at_parser_t *parser = &module->parser;

    char check_kw[AT_RESP_BUFF_SIZE_DEF / 4] = {0};

    char *resp_buff = os_calloc(1, AT_RESP_BUFF_SIZE_384);

    at_resp_t resp = {.buff = resp_buff, .buff_size = AT_RESP_BUFF_SIZE_384, .timeout = 5 * OS_TICK_PER_SECOND};

    at_parser_exec_cmd(parser, &resp, "AT+CIPSTATUS");

    for (int i = 0; i < ESP8266_NETCONN_NUM; i++)
    {
        if (NETCONN_STAT_NULL == esp8266->netconn[i].stat)
        {
            snprintf(check_kw, sizeof(check_kw), "+CIPSTATUS:%d", i);

            if (at_resp_get_line_by_kw(&resp, check_kw) == OS_NULL)
            {
                esp8266->netconn[i].connect_id = i;
                os_free(resp_buff);

                return &esp8266->netconn[i];
            }
        }
        else
        {
            DEBUG("netconn[%d] stat=%d", i, esp8266->netconn[i].stat);
        }
    }

    ERROR("Module %s alloc netconn failed!", module->name);

    os_free(resp_buff);

    return OS_NULL;
}

static mo_netconn_t *esp8266_get_netconn_by_id(mo_object_t *module, os_int32_t connect_id)
{
    OS_ASSERT(OS_NULL != module);

    if (0 > connect_id || ESP8266_NETCONN_NUM <= connect_id)
    {
        ERROR("%s INVALID connet_id:[%d]!", __func__, connect_id);
        return OS_NULL;
    }

    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    return &esp8266->netconn[connect_id];
}

os_err_t esp8266_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info)
{
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    info->netconn_array = esp8266->netconn;
    info->netconn_nums  = sizeof(esp8266->netconn) / sizeof(esp8266->netconn[0]);

    return OS_EOK;
}

mo_netconn_t *esp8266_netconn_create(mo_object_t *module, mo_netconn_type_t type)
{
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    esp8266_lock(&esp8266->netconn_lock);

    mo_netconn_t *netconn = esp8266_netconn_alloc(module);

    if (OS_NULL == netconn)
    {
        esp8266_unlock(&esp8266->netconn_lock);
        return OS_NULL;
    }

    netconn->mq = os_mq_create(ESP8266_NETCONN_MQ_NAME, ESP8266_NETCONN_MQ_MSG_SIZE, ESP8266_NETCONN_MQ_MSG_MAX);
    if (OS_NULL == netconn->mq)
    {
        ERROR("%s data queue create failed, no enough memory.", module->name);
        esp8266_unlock(&esp8266->netconn_lock);
        return OS_NULL;
    }

    netconn->stat = NETCONN_STAT_INIT;
    netconn->type = type;

    esp8266_unlock(&esp8266->netconn_lock);

    return netconn;
}

os_err_t esp8266_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn)
{
    at_parser_t *parser = &module->parser;
    os_err_t     result = OS_ERROR;

    DEBUG("Module %s in %d netconn status", module->name, netconn->stat);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 5 * OS_TICK_PER_SECOND};
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);
    if (esp8266 == OS_NULL)
    {
        ERROR("Module %s destroy netconn failed, get netconn_lock failed", module->name);
        return OS_ERROR;
    }
    esp8266_lock(&esp8266->netconn_lock);

    switch (netconn->stat)
    {
    case NETCONN_STAT_CONNECT:
        result = at_parser_exec_cmd(parser, &resp, "AT+CIPCLOSE=%d", netconn->connect_id);
        if (result != OS_EOK)
        {
            ERROR("Module %s destroy %s netconn failed",
                  module->name,
                  (netconn->type == NETCONN_TYPE_TCP) ? "TCP" : "UDP");
            esp8266_unlock(&esp8266->netconn_lock);
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
#ifdef MOLINK_USING_SOCKETS_OPS
    netconn->socket_id = -1;
    netconn->evt_func = OS_NULL;
    netconn->data_func = OS_NULL;
#endif
    inet_aton("0.0.0.0", &netconn->remote_ip);

    esp8266_unlock(&esp8266->netconn_lock);

    return OS_EOK;
}

os_err_t esp8266_netconn_bind(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 10 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    //INFO("loacl addr step 1:0x%08x", addr.addr);
    //addr.addr |= 0x000000FF;
    //INFO("loacl addr step 2:0x%08x", addr.addr);
    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    INFO("loacl addr:%s", remote_ip);
    os_err_t result = OS_EOK;

    switch (netconn->type)
    {
    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+CIPSTART=%d,\"UDP\",\"%s\",0,%d,2",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);                                   
        break;
    case NETCONN_TYPE_UDP_V6:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+CIPSTART=%d,\"UDPv6\",\"::/128\",65535,%d",
                                    netconn->connect_id,
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
    
__exit:
    if (OS_EOK == result)
    {
        ip_addr_copy(netconn->remote_ip, addr);
        netconn->remote_port = port;
        netconn->stat        = NETCONN_STAT_CONNECT;

        INFO("Module %s connect to %s:%d successfully!", module->name, remote_ip, port);
    }
    else
    {
        ERROR("Module %s connect to %s:%d failed!", module->name, remote_ip, port);
    }

    return result;
}

os_err_t esp8266_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = 150 * OS_TICK_PER_SECOND};

    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    os_err_t result = OS_EOK;

    switch (netconn->type)
    {
    case NETCONN_TYPE_TCP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+CIPSTART=%d,\"TCP\",\"%s\",%d,60",
                                    netconn->connect_id,
                                    remote_ip,
                                    port);
        break;

    case NETCONN_TYPE_UDP:
        result = at_parser_exec_cmd(parser,
                                    &resp,
                                    "AT+CIPSTART=%d,\"UDP\",\"%s\",%d",
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

    return result;
}

os_size_t esp8266_netconn_sendto(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port, const char *data, os_size_t size)
{
    char remote_ip[IPADDR_MAX_STR_LEN + 1] = {0};
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    strncpy(remote_ip, inet_ntoa(addr), IPADDR_MAX_STR_LEN);

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND};
    at_parser_exec_lock(parser);

    esp8266->curr_connect = netconn->connect_id;
    at_parser_set_end_mark(parser, ">", 1);

    while (sent_size < size)
    {
        if (size - sent_size < SEND_DATA_MAX_SIZE)
        {
            curr_size = size - sent_size;
        }
        else
        {
            curr_size = SEND_DATA_MAX_SIZE;
        }

        /* Protect the data sending process, prevent other threads to send AT commands */

        result = at_parser_exec_cmd(parser, &resp, "AT+CIPSEND=%d,%d,\"%s\",%d", netconn->connect_id, curr_size, remote_ip, port);
        if (result != OS_EOK)
        {
            goto __exit;
        }

        if (at_parser_send(parser, data + sent_size, curr_size) <= 0)
        {
            result = OS_ERROR;
            goto __exit;
        }

        result = os_event_recv(&esp8266->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&esp8266->netconn_evt,
                               ESP8266_EVENT_SEND_OK | ESP8266_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & ESP8266_EVENT_SEND_FAIL)
        {
            ERROR("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        if (netconn->type == NETCONN_TYPE_TCP)
        {
            curr_size = esp8266->curr_sent_size;
        }

        sent_size += curr_size;
        os_task_msleep(30);
    }

__exit:

    at_parser_set_end_mark(parser, OS_NULL, 0);
    esp8266->curr_connect = -1;

    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s netconn %d send %d bytes data failed!", parser->name, netconn->connect_id, size);
        
        return 0;
    }

    return sent_size;
}

os_size_t esp8266_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    at_parser_t *parser    = &module->parser;
    os_err_t     result    = OS_EOK;
    os_size_t    sent_size = 0;
    os_size_t    curr_size = 0;
    os_uint32_t  event     = 0;

    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);
    
    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND};
					  
	/* Protect the data sending process, prevent other threads to send AT commands */
    at_parser_exec_lock(parser);

    esp8266->curr_connect = netconn->connect_id;
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

        /* Protect the data sending process, prevent other threads to send AT commands */

        result = at_parser_exec_cmd(parser, &resp, "AT+CIPSEND=%d,%d", netconn->connect_id, curr_size);
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

        result = os_event_recv(&esp8266->netconn_evt,
                               SET_EVENT(netconn->connect_id, 0),
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               10 * OS_TICK_PER_SECOND,
                               OS_NULL);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait event timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        result = os_event_recv(&esp8266->netconn_evt,
                               ESP8266_EVENT_SEND_OK | ESP8266_EVENT_SEND_FAIL,
                               OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                               1 * OS_TICK_PER_SECOND,
                               &event);
        if (result != OS_EOK)
        {
            ERROR("Module %s connect id %d wait send result timeout!", module->name, netconn->connect_id);
            goto __exit;
        }

        if (event & ESP8266_EVENT_SEND_FAIL)
        {
            ERROR("Module %s connect id %d send failed!", module->name, netconn->connect_id);
            result = OS_ERROR;
            goto __exit;
        }

        if (netconn->type == NETCONN_TYPE_TCP)
        {
            curr_size = esp8266->curr_sent_size;
        }

        sent_size += curr_size;
        os_task_msleep(30);
    }

__exit:

    at_parser_set_end_mark(parser, OS_NULL, 0);
    esp8266->curr_connect = -1;
    at_parser_exec_unlock(parser);

    if (result != OS_EOK)
    {
        ERROR("Module %s netconn %d send %d bytes data failed!", parser->name, netconn->connect_id, size);
        return 0;
    }

    return sent_size;
}

os_err_t esp8266_netconn_gethostbyname(mo_object_t *module, const char *domain_name, ip_addr_t *addr)
{
    OS_ASSERT(OS_NULL != domain_name);
    OS_ASSERT(OS_NULL != addr);

    char recvip[IPADDR_MAX_STR_LEN + 1] = {0};

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 20 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIPDOMAIN=\"%s\"", domain_name);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_resp_get_data_by_kw(&resp, "+CIPDOMAIN:", "+CIPDOMAIN:%s", recvip) <= 0)
    {
        ERROR("Module %s domain resolve: resp parse fail, try again, host: %s", module->name, domain_name);
        result = OS_ERROR;
        /* If resolve failed, maybe receive an URC CRLF */
        goto __exit;
    }

    if (strlen(recvip) < IPADDR_MIN_STR_LEN)
    {
        ERROR("Module %s domain resolve: recvip len < IPADDR_MIN_STR_LEN, len = %d", module->name, strlen(recvip));
        result = OS_ERROR;
        goto __exit;
    }
    else
    {
        DEBUG("Module %s domain resolve: \"%s\" domain ip is %s, addrlen %d",
              module->name,
              domain_name,
              recvip,
              strlen(recvip));

        inet_aton(recvip, addr);
#ifdef MOLINK_USING_IPV4
#ifdef MOLINK_USING_IPV6
        if (IPADDR_ANY == addr->u_addr.ip4.addr || IPADDR_LOOPBACK == addr->u_addr.ip4.addr) 
#else
        if (IPADDR_ANY == addr->addr || IPADDR_LOOPBACK == addr->addr)
#endif        
        {
            ip_addr_set_zero(addr);
            result = OS_ERROR;
            goto __exit;
        }
#endif
        result = OS_EOK;
    }

__exit:

    return result;
}

static void urc_send_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    os_int32_t curr_connect = esp8266->curr_connect;

    if (strstr(data, "SEND OK"))
    {
        os_event_send(&esp8266->netconn_evt, SET_EVENT(curr_connect, ESP8266_EVENT_SEND_OK));
    }
    else if (strstr(data, "SEND FAIL"))
    {
        os_event_send(&esp8266->netconn_evt, SET_EVENT(curr_connect, ESP8266_EVENT_SEND_FAIL));
    }

    return;
}

static void urc_close_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;

    sscanf(data, "%d,CLOSED", &connect_id);

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_netconn_t *netconn = esp8266_get_netconn_by_id(module, connect_id);

    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error close urc data of connect %d", module->name, connect_id);
        return;
    }

    INFO("Module %s receive close urc data of connect %d", module->name, connect_id);

    mo_netconn_pasv_close_notice(netconn);

    return;
}

static void urc_send_bfsz_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_esp8266_t *esp8266 = os_container_of(module, mo_esp8266_t, parent);

    os_int32_t send_bfsz = 0;

    sscanf(data, "Recv %d bytes", &send_bfsz);

    esp8266->curr_sent_size = send_bfsz;

    return;
}

static void urc_recv_read_param(struct at_parser *parser, char *buf, int commas_cnt, int colon_cnt)
{
    int recv_commas_cnt;
    int recv_colon_cnt;
    int recv_cnt;
    os_size_t size;

    recv_commas_cnt = 0;
    recv_colon_cnt = 0;
    recv_cnt = 0;

    while (recv_commas_cnt < commas_cnt)
    {
        size = at_parser_recv(parser, buf + recv_cnt, 1, OS_WAIT_FOREVER);
        if (size > 0)
        {
            if (',' == buf[recv_cnt])
            {
                recv_commas_cnt++;
            }
            recv_cnt++;
        }
    }

    while (recv_colon_cnt < colon_cnt)
    {
        size = at_parser_recv(parser, buf + recv_cnt, 1, OS_WAIT_FOREVER);
        if (size > 0)
        {
            if (':' == buf[recv_cnt])
            {
                recv_colon_cnt++;
            }
            recv_cnt++;
        }
    }

    return;
}

static void urc_recv_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t connect_id = 0;
    os_int32_t data_size  = 0;
    char remote_ip[IPADDR_MAX_STR_LEN + 1 + 2] = {0};
    char ipd_prefix[80];
    ip_addr_t addr;
    os_uint16_t port;
    int len;

    mo_object_t  *module  = os_container_of(parser, mo_object_t, parser);
    mo_esp8266_t *esp8266  = os_container_of(module, mo_esp8266_t, parent);

    memset(ipd_prefix, 0, sizeof(ipd_prefix));
    memcpy(ipd_prefix, data, size);
#ifdef ESP8266_USING_WIFI_OPS
    if (MO_WIFI_CIPINFO_SHOW_IP_PORT == esp8266->wifi_ipdinfo)
    {
        urc_recv_read_param(parser, ipd_prefix + size, 3, 1);
        sscanf(ipd_prefix, "+IPD,%d,%d,%[^,],%hu:", &connect_id, &data_size, remote_ip, &port);
        if (remote_ip[0] == '"')
        {
            len = strlen(remote_ip);
            memmove(remote_ip, remote_ip + 1, len - 2);
            remote_ip[len - 2] = 0x00;
        }
    }
    else
#endif
    {
        urc_recv_read_param(parser, ipd_prefix + size, 1, 1);
        sscanf(ipd_prefix, "+IPD,%d,%d:", &connect_id, &data_size);
    }

    os_int32_t timeout = data_size > 10 ? data_size : 10;
	// INFO("--%s", ipd_prefix);
    // INFO("Moudle %s netconn %d receive %d bytes data from[%s,%d]", parser->name, connect_id, data_size, remote_ip, port);

    mo_netconn_t *netconn = esp8266_get_netconn_by_id(module, connect_id);

    if (OS_NULL == netconn)
    {
        ERROR("Module %s receive error recv urc data of connect %d", module->name, connect_id);
        return;
    }

    char *recv_buff = os_calloc(1, data_size + 1);
    if (recv_buff == OS_NULL)
    {
        /* read and clean the coming data */
        ERROR("os_calloc recv buff %d bytes fail, no enough memory", data_size);
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
    
    ip_addr_set_zero(&addr);
    if (NETCONN_TYPE_UDP_V6 == netconn->type || NETCONN_TYPE_TCP_V6 == netconn->type)
    {
        IP_SET_TYPE(&addr, IPADDR_TYPE_V6);   
    }
    
    inet_aton(remote_ip, &addr);
    mo_wifi_netconn_data_recv_notice(netconn, addr, (os_uint16_t)port, recv_buff, data_size);

    return;
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "SEND OK",   .suffix = "\r\n",        .func = urc_send_func},
    {.prefix = "SEND FAIL", .suffix = "\r\n",        .func = urc_send_func},
    {.prefix = "Recv",      .suffix = "bytes\r\n",   .func = urc_send_bfsz_func},
    {.prefix = "+IPD",      .suffix = ",",           .func = urc_recv_func},
    {.prefix = "",          .suffix = ",CLOSED\r\n", .func = urc_close_func},
};

os_err_t esp8266_netconn_init(mo_esp8266_t *module)
{
    at_parser_t *parser = &(module->parent.parser);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff = resp_buff, .buff_size = sizeof(resp_buff), .timeout = AT_RESP_TIMEOUT_DEF};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+CIPMUX?");
    if (result != OS_EOK)
    {
        return result;
    }

    os_int32_t mode = 0;

    if (at_resp_get_data_by_kw(&resp, "+CIPMUX:", "+CIPMUX:%d", &mode) <= 0)
    {
        return OS_ERROR;
    }

    if (ESP8266_MULTI_CONN_ENABLE == mode)
    {
        /* Close all connections */
        result = at_parser_exec_cmd(parser, &resp, "AT+CIPCLOSE=5");
    }
    else
    {
        result = at_parser_exec_cmd(parser, &resp, "AT+CIPMUX=1");
    }

    if (result != OS_EOK)
    {
        return result;
    }

    /* Init module netconn array */
    memset(module->netconn, 0, sizeof(module->netconn));
    for (int i = 0; i < ESP8266_NETCONN_NUM; i++)
    {
        module->netconn[i].connect_id = -1;
    }

    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));

    return result;
}

#endif /* ESP8266_USING_NETCONN_OPS */
