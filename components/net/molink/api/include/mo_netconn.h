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
 * @file        mo_netconn.h
 *
 * @brief       module link kit netconnect api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_NETCONN_H__
#define __MO_NETCONN_H__

#include "mo_type.h"
#include "mo_ipaddr.h"
#include "mo_object.h"

#include <os_mq.h>

#ifdef MOLINK_USING_NETCONN_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 ***********************************************************************************************************************
 * @enum        mo_netconn_stat_t
 *
 * @brief       molink module network connect state
 ***********************************************************************************************************************
 */
typedef enum mo_netconn_stat
{
    NETCONN_STAT_NULL = 0, /* netconn has not been created */
    NETCONN_STAT_INIT,     /* netconn was created but not connected */
    NETCONN_STAT_CONNECT,  /* netconn connect OK*/
    NETCONN_STAT_CLOSE,    /* netconn was closed but all connect info were not deleted */
    NETCONN_STAT_UNDEFINED /* netconn undefined status */
} mo_netconn_stat_t;

/**
 ***********************************************************************************************************************
 * @enum        mo_netconn_type_t
 *
 * @brief       molink module network connect type
 ***********************************************************************************************************************
 */
typedef enum mo_netconn_type
{
    NETCONN_TYPE_NULL = 0,
    NETCONN_TYPE_TCP,
    NETCONN_TYPE_UDP,
    NETCONN_TYPE_TCP_V6,
    NETCONN_TYPE_UDP_V6,
} mo_netconn_type_t;

/**
 ***********************************************************************************************************************
 * Used to inform the callback function about changes
 * 
 * RCVPLUS events say: Safe to perform a potentially blocking call call once more. 
 * They are counted in sockets - three RCVPLUS events for accept mbox means you are safe
 * to call netconn_accept 3 times without being blocked.
 * Same thing for receive mbox.
 * 
 * RCVMINUS events say: Your call to to a possibly blocking function is "acknowledged".
 * Socket implementation decrements the counter.
 * 
 * For TX, there is no need to count, its merely a flag. SENDPLUS means you may send something.
 * SENDPLUS occurs when enough data was delivered to peer so netconn_send() can be called again.
 * A SENDMINUS event occurs when the next call to a netconn_send() would be blocking.
 ***********************************************************************************************************************
 */
typedef enum mo_netconn_evt
{
    MO_NETCONN_EVT_RCVPLUS,
    MO_NETCONN_EVT_RCVMINUS,
    MO_NETCONN_EVT_SENDPLUS,
    MO_NETCONN_EVT_SENDMINUS,
    MO_NETCONN_EVT_ERROR
} mo_netconn_evt_t;

struct mo_netconn;

typedef void (*mo_netconn_evt_callback)(struct mo_netconn *netconn, mo_netconn_evt_t evt, os_uint16_t len);
typedef void (*mo_netconn_data_callback)(void *var, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size);

/**
 ***********************************************************************************************************************
 * @struct      mo_netconn_t
 *
 * @brief       molink module network connect object
 ***********************************************************************************************************************
 */
typedef struct mo_netconn
{
#ifdef MOLINK_USING_SOCKETS_OPS
    os_int32_t socket_id;
    mo_netconn_evt_callback evt_func;
    mo_netconn_data_callback data_func;
#endif
    os_int32_t connect_id;

    mo_netconn_stat_t stat;
    mo_netconn_type_t type;
    
    ip_addr_t   remote_ip;
    os_uint16_t remote_port;
    os_uint16_t local_port;
    
    os_mq_t     *mq;

} mo_netconn_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_netconn_info_t
 *
 * @brief       struct that holds molink module netconn information
 ***********************************************************************************************************************
 */
typedef struct mo_netconn_info
{
    os_uint32_t         netconn_nums;
    const mo_netconn_t *netconn_array;
} mo_netconn_info_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_notconn_msg_t
 *
 * @brief       struct that holds molink recv mq data information
 ***********************************************************************************************************************
 */
typedef struct mo_notconn_msg
{
    ip_addr_t addr;
    os_uint16_t port;
    char       *data;
    os_uint32_t data_len;
} mo_notconn_msg_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_netconn_ops_t
 *
 * @brief       molink module network connect ops table
 ***********************************************************************************************************************
 */
typedef struct mo_netconn_ops
{
    mo_netconn_t *(*create)(mo_object_t *module, mo_netconn_type_t type);
    os_err_t      (*destroy)(mo_object_t *module, mo_netconn_t *netconn);
    os_err_t      (*connect)(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port);
    os_err_t      (*bind)(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port);
    os_size_t     (*sendto)(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t remote_ip, os_uint16_t remote_port, const char *data, os_size_t size);
    os_size_t     (*send)(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size);
    os_err_t      (*gethostbyname)(mo_object_t *module, const char *domain_name, ip_addr_t *addr);
    os_err_t      (*get_info)(mo_object_t *module, mo_netconn_info_t *info);
} mo_netconn_ops_t;

mo_netconn_t *mo_netconn_create(mo_object_t *module, mo_netconn_type_t type);
os_err_t      mo_netconn_destroy(mo_object_t *module, mo_netconn_t *netconn);
os_err_t      mo_netconn_bind(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port);
os_err_t      mo_netconn_connect(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port);
os_size_t     mo_netconn_sendto(mo_object_t *module, mo_netconn_t *netconn, ip_addr_t remote_ip, os_uint16_t remote_port, const char *data, os_size_t size);
os_size_t     mo_netconn_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size);
os_err_t       mo_netconn_recvfrom(mo_object_t *module,
                              mo_netconn_t *netconn,
                              void **data,
                              os_size_t *size, 
                              ip_addr_t *addr, 
                              os_uint16_t *port,
                              os_tick_t timeout);
os_err_t      mo_netconn_gethostbyname(mo_object_t *module, const char *domain_name, ip_addr_t *addr);

/**
 ***********************************************************************************************************************
 * @note These functions are called by the molink component
 ***********************************************************************************************************************
 */
os_err_t      mo_netconn_get_info(mo_object_t *module, mo_netconn_info_t *info);
void          mo_netconn_pasv_close_notice(mo_netconn_t *netconn);
void          mo_wifi_netconn_data_recv_notice(mo_netconn_t *netconn, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size);
void          mo_netconn_data_recv_notice(mo_netconn_t *netconn, char *data, os_size_t size);

void          mo_netconn_mq_destroy(os_mq_t *mq);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_NETCONN_OPS */

#endif /* __MO_NETCONN_H__ */
