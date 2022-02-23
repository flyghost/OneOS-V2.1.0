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
 * @file        mo_socket.c
 *
 * @brief       module link kit socket api
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "mo_common.h"
#include "mo_socket.h"
#include "os_sem.h"

#include <sys/errno.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <os_assert.h>
#include <os_list.h>
#include <os_task.h>

#define MO_LOG_TAG "molink.socket"
#define MO_LOG_LVL MO_LOG_INFO
#include "mo_log.h"

#ifdef MOLINK_USING_SOCKETS_OPS

#define MO_SYS_ARCH_PROTECT   os_schedule_lock()
#define MO_SYS_ARCH_UNPROTECT os_schedule_unlock()

#define HTONS_PORT(x) ((((x)&0x00ffUL) << 8) | (((x)&0xff00UL) >> 8))

// Description for a task waiting in select
struct mo_select_cb
{
    os_slist_node_t slist;
    // readset passed to select
    fd_set *readset;
    // writeset passed to select
    fd_set *writeset;
    // unimplemented: exceptset passed to select
    fd_set *exceptset;
    // don't signal the same semaphore twice: set to 1 when signalled
    int sem_signalled;
    // semaphore to wake up a task waiting for select
    os_sem_t sem;
};

static mo_sock_t       gs_sockets[MOLINK_NUM_SOCKETS] = {0};
static os_slist_node_t gs_mo_select_cb_slist          = {OS_NULL};
static volatile int    gs_mo_select_cb_ctr;

static mo_sock_t *mo_get_socket(int socket)
{
    if (socket < 0 || socket >= MOLINK_NUM_SOCKETS)
    {
        ERROR("Invalid socket number %d!", socket);
        return OS_NULL;
    }

    mo_sock_t *sock = &gs_sockets[socket];

    // when netconn is close, sock is regard as null
    if (OS_NULL == sock->netconn)
    {
        return OS_NULL;
    }

    return sock;
}

/**
 * the func is the same as mo_get_socket, just don't care the netconn is null
 */
static mo_sock_t *mo_tryget_socket(int socket)
{
    if (socket < 0 || socket >= MOLINK_NUM_SOCKETS)
    {
        ERROR("Invalid socket number %d!", socket);
        return OS_NULL;
    }

    return &gs_sockets[socket];
}

/**
 * Callback registered in the netconn layer for each socket-netconn.
 * Processes recvevent (data available) and wakes up tasks waiting for select.
 */
static void mo_event_callback(mo_netconn_t *netconn, mo_netconn_evt_t evt, os_uint16_t len)
{
    os_bool_t            do_check_waiter;
    struct mo_select_cb *scb;
    os_slist_node_t *    node;
    int                  last_select_cb_ctr;
    int                  do_signal;
    os_int32_t           s;
    mo_sock_t *          sock;
#ifdef OS_USING_IO_MULTIPLEXING
    os_uint16_t          evt_key = 0;
    os_slist_node_t     *poll_node;
    poll_req_t          *poll_req;
#endif

    // Get socket
    if (NULL == netconn || (netconn->socket_id < 0))
    {
        return;
    }

    sock = mo_get_socket(netconn->socket_id);
    if (NULL == sock)
    {
        return;
    }
    MO_SYS_ARCH_PROTECT;
    do_check_waiter = OS_FALSE;
    switch (evt)
    {
    case MO_NETCONN_EVT_RCVPLUS:
        sock->rcvevent++;
        do_check_waiter = OS_TRUE;
        break;

    case MO_NETCONN_EVT_SENDPLUS:
        sock->sendevent = 1;
        break;

    case MO_NETCONN_EVT_ERROR:
        do_check_waiter = OS_TRUE;
        sock->errevent  = 1;
        break;

    case MO_NETCONN_EVT_RCVMINUS:
        sock->rcvevent--;
        break;

    case MO_NETCONN_EVT_SENDMINUS:
        sock->sendevent = 0;
        break;

    default:
        WARN("unknown event = %d", evt);
        break;
    }

#ifdef OS_USING_IO_MULTIPLEXING
    os_slist_for_each(poll_node, &sock->req_slist_head)
    {
        poll_req = os_slist_entry(poll_node, poll_req_t, req_list);

        switch (evt)
        {
        case MO_NETCONN_EVT_RCVPLUS:
            if (poll_req->req && poll_req->req->events & POLLIN)
            {
                evt_key = POLLIN;
            }
            break;

        case MO_NETCONN_EVT_SENDPLUS:
            if (poll_req->req && poll_req->req->events & POLLOUT)
            {
                evt_key = POLLOUT;
            }
            break;

        case MO_NETCONN_EVT_ERROR:
            if (poll_req->req && poll_req->req->events & POLLERR)
            {
                evt_key = POLLERR;
            }
            break;

        default:
            break;
        }

        if (poll_req->req && do_check_waiter && evt_key)
        {
            devfs_poll_notify(poll_req->req, evt_key);
        }
    }
#endif

    if (!sock->select_waiting || OS_FALSE == do_check_waiter)
    {
        /* no one is waiting for this socket, no need to check select_cb_list */
        MO_SYS_ARCH_UNPROTECT;
        return;
    }

    /** NOTE: This code goes through the select_cb_list list multiple times
     * ONLY IF a select was actually waiting. We go through the list the number
     * of waiting select calls + 1. This list is expected to be small.
     */

    do
    {
        // remember the state of select_cb_list to detect changes
        last_select_cb_ctr = gs_mo_select_cb_ctr;
        os_slist_for_each(node, &gs_mo_select_cb_slist)
        {
            scb = os_slist_entry(node, struct mo_select_cb, slist);
            // semaphore has signalled yet, do nothing
            if (scb->sem_signalled)
            {
                continue;
            }
            do_signal = 0;

            s = netconn->socket_id;
            // Test this select call for our socket */
            if (sock->rcvevent > 0)
            {
                if (scb->readset && FD_ISSET(s, scb->readset))
                {
                    do_signal = 1;
                }
            }

            if (sock->sendevent != 0)
            {
                if (!do_signal && scb->writeset && FD_ISSET(s, scb->writeset))
                {
                    do_signal = 1;
                }
            }

            if (sock->errevent != 0)
            {
                if (!do_signal && scb->exceptset && FD_ISSET(s, scb->exceptset))
                {
                    do_signal = 1;
                }
            }

            if (do_signal)
            {
                scb->sem_signalled = 1;
                os_sem_post(&scb->sem);
                /** Don't call SYS_ARCH_UNPROTECT() before signaling the semaphore, as this might
                 * lead to the select thread taking itself off the list, invalidating the semaphore.
                 */
            }
        }

        // unlock interrupts with each step
        MO_SYS_ARCH_UNPROTECT;
        // this makes sure interrupt protection time is short */
        MO_SYS_ARCH_PROTECT;
    } while (last_select_cb_ctr != gs_mo_select_cb_ctr);

    MO_SYS_ARCH_UNPROTECT;

    return;
}

static int alloc_socket(mo_netconn_t *netconn)
{
    MO_SYS_ARCH_PROTECT;

    for (int i = 0; i < MOLINK_NUM_SOCKETS; i++)
    {
        if (OS_NULL == gs_sockets[i].netconn)
        {
            // wait for mo_select release the resource
            if (!gs_sockets[i].select_waiting)
            {
                netconn->socket_id = i;
                netconn->evt_func  = mo_event_callback;
                MO_SYS_ARCH_UNPROTECT;

                return i;
            }
            else
            {
                WARN("Unbelieveable, socket is selected");
            }
        }
    }

    MO_SYS_ARCH_UNPROTECT;

    ERROR("Alloc socket failed!");

    return -1;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will alloc a molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       domain        Domain description, only AF_INET for now.
 * @param[in]       type          Socket type.
 * @param[in]       protocol      Protocol socket used.
 *
 * @return          status.
 * @retval          -1            failed.
 * @retval          others        socket id.
 ***********************************************************************************************************************
 */
int mo_socket(mo_object_t *module, int domain, int type, int protocol)
{
    OS_ASSERT(module != OS_NULL);

    mo_netconn_t *netconn = OS_NULL;
    mo_netconn_type_t ntc_type;

    switch (type)
    {
    case SOCK_DGRAM:
        if (PF_INET6 == domain)
        {
            ntc_type = NETCONN_TYPE_UDP_V6;
        }   
        else
        {
            ntc_type = NETCONN_TYPE_UDP;
        }
        netconn = mo_netconn_create(module, ntc_type);
        break;
    case SOCK_STREAM:
        if (PF_INET6 == domain)
        {
            ntc_type = NETCONN_TYPE_TCP_V6;
        }   
        else
        {
            ntc_type = NETCONN_TYPE_TCP;
        }
        netconn = mo_netconn_create(module, ntc_type);
        break;
    default:
        WARN("Module %s Don't support socket type %d", module->name, type);
        break;
    }

    if (OS_NULL == netconn)
    {
        return -1;
    }

    int socket_id = alloc_socket(netconn);

    if (-1 == socket_id)
    {
        mo_netconn_destroy(module, netconn);
        return -1;
    }

    gs_sockets[socket_id].netconn      = netconn;
    gs_sockets[socket_id].lastdata     = OS_NULL;
    gs_sockets[socket_id].lastoffset   = 0;
    gs_sockets[socket_id].recv_timeout = 0;

    gs_sockets[socket_id].rcvevent = 0;
    // when sock create successful, sock can write
    gs_sockets[socket_id].sendevent      = 1;
    gs_sockets[socket_id].errevent       = 0;
    gs_sockets[socket_id].select_waiting = 0;
#ifdef OS_USING_IO_MULTIPLEXING
    os_slist_init(&gs_sockets[socket_id].req_slist_head);
#endif

    return socket_id;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will close a molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket to close.
 *
 * @return          status.
 * @retval          0             success.
 * @retval          -1            failed.
 * @retval          -2            time out.
 ***********************************************************************************************************************
 */
int mo_closesocket(mo_object_t *module, int socket)
{
    OS_ASSERT(module != OS_NULL);   

    int ret;
    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock)
    {
        ERROR("Module %s get close socket %d failed!", module->name, socket);
        return -1;
    }

    // wakeup sleep thread
    if (sock->netconn->evt_func)
    {
        // send send recv or except event, to wakeup sleep thread
        sock->netconn->evt_func(sock->netconn, MO_NETCONN_EVT_RCVPLUS, 0);
        sock->netconn->evt_func(sock->netconn, MO_NETCONN_EVT_ERROR, 0);
        // sock is always writeable, don't send sendplus event
        // sock->netconn->evt_func(sock->netconn, MO_NETCONN_EVT_SENDPLUS, 0);
    }

    MO_SYS_ARCH_PROTECT;
    mo_netconn_t *netconn = sock->netconn;
    sock->netconn         = OS_NULL;
    MO_SYS_ARCH_UNPROTECT;

    if (OS_NULL != sock->lastdata)
    {
        os_free(sock->lastdata);
    }

    MO_SYS_ARCH_PROTECT;
    sock->lastdata = OS_NULL;
    MO_SYS_ARCH_UNPROTECT;

    // just for multithreaded call safety
    if (OS_NULL != netconn)
    {
        ret = mo_netconn_destroy(module, netconn);
        if(ret != OS_EOK)
        {
            ERROR("Module %s close socket %d failed!", module->name, socket);
            return ret;          
        }
    }

    return 0;
}

#ifdef MOLINK_USING_IPV4
#define IP4ADDR_PORT_TO_SOCKADDR(sin, ipaddr, port) do { \
      (sin)->sin_len = sizeof(struct sockaddr_in); \
      (sin)->sin_family = AF_INET; \
      (sin)->sin_port = htons(*(port)); \
      inet_addr_from_ip4addr(&(sin)->sin_addr, ipaddr); \
      memset((sin)->sin_zero, 0, SIN_ZERO_LEN); }while(0)
#define SOCKADDR4_TO_IP4ADDR_PORT(sin, ipaddr, port) do { \
    inet_addr_to_ip4addr(ip_2_ip4(ipaddr), &((sin)->sin_addr)); \
    *(port) = ntohs((sin)->sin_port); }while(0)
#endif /* MOLINK_USING_IPV4 */

#ifdef MOLINK_USING_IPV6

#define ip6_addr_set_zone(ip6addr, zone_idx)

#define IP6ADDR_PORT_TO_SOCKADDR(sin6, ipaddr, port) do { \
      (sin6)->sin6_len = sizeof(struct sockaddr_in6); \
      (sin6)->sin6_family = AF_INET6; \
      (sin6)->sin6_port = htons(*(port)); \
      (sin6)->sin6_flowinfo = 0; \
      inet6_addr_from_ip6addr(&(sin6)->sin6_addr, ipaddr); \
      (sin6)->sin6_scope_id = ip6_addr_zone(ipaddr); }while(0)
#define SOCKADDR6_TO_IP6ADDR_PORT(sin6, ipaddr, port) do { \
    inet6_addr_to_ip6addr(ip_2_ip6(ipaddr), &((sin6)->sin6_addr)); \
    if (ip6_addr_has_scope(ip_2_ip6(ipaddr), IP6_UNKNOWN)) { \
      ip6_addr_set_zone(ip_2_ip6(ipaddr), (uint8_t)((sin6)->sin6_scope_id)); \
    } \
    *(port) = ntohs((sin6)->sin6_port); }while(0)
#endif /* MOLINK_USING_IPV6 */

#if defined(MOLINK_USING_IPV4) && defined(MOLINK_USING_IPV6)
/* get IP address and port by socketaddr structure information */
static void sockaddr_to_ipaddr_port(const struct sockaddr *sockaddr, ip_addr_t *ipaddr, uint16_t *port)
{
#define AF_INET  2
#define AF_INET6 10
  if (AF_INET6 == sockaddr->sa_family) {
    SOCKADDR6_TO_IP6ADDR_PORT((const struct sockaddr_in6 *)(const void *)(sockaddr), ipaddr, port);
    ipaddr->type = IPADDR_TYPE_V6;
  } else {
    SOCKADDR4_TO_IP4ADDR_PORT((const struct sockaddr_in *)(const void *)(sockaddr), ipaddr, port);
    ipaddr->type = IPADDR_TYPE_V4;
  }
}

#define IPADDR_PORT_TO_SOCKADDR(sockaddr, ipaddr, port) do { \
    if (IP_IS_ANY_TYPE_VAL(*ipaddr) || IP_IS_V6_VAL(*ipaddr)) { \
      IP6ADDR_PORT_TO_SOCKADDR((struct sockaddr_in6*)(void*)(sockaddr), ip_2_ip6(ipaddr), port); \
    } else { \
      IP4ADDR_PORT_TO_SOCKADDR((struct sockaddr_in*)(void*)(sockaddr), ip_2_ip4(ipaddr), port); \
    } } while(0)
#define SOCKADDR_TO_IPADDR_PORT(sockaddr, ipaddr, port) sockaddr_to_ipaddr_port(sockaddr, ipaddr, port)
#elif defined (MOLINK_USING_IPV6)
#define IPADDR_PORT_TO_SOCKADDR(sockaddr, ipaddr, port) \
        IP6ADDR_PORT_TO_SOCKADDR((struct sockaddr_in6*)(void*)(sockaddr), ip_2_ip6(ipaddr), port)
#define SOCKADDR_TO_IPADDR_PORT(sockaddr, ipaddr, port) \
        SOCKADDR6_TO_IP6ADDR_PORT((const struct sockaddr_in6*)(const void*)(sockaddr), ipaddr, port)
#else
#define IPADDR_PORT_TO_SOCKADDR(sockaddr, ipaddr, port) \
        IP4ADDR_PORT_TO_SOCKADDR((struct sockaddr_in*)(void*)(sockaddr), ip_2_ip4(ipaddr), port)
#define SOCKADDR_TO_IPADDR_PORT(sockaddr, ipaddr, port) \
        SOCKADDR4_TO_IP4ADDR_PORT((const struct sockaddr_in*)(const void*)(sockaddr), ipaddr, port)
#endif
/**
 ***********************************************************************************************************************
 * @brief           This function will bind socket to local addr.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket to bind.
 * @param[in]       name          loaal address.
 * @param[in]       namelen       address lenth.
 *
 * @return          status.
 * @retval          0             success.
 * @retval          others        failed.
 ***********************************************************************************************************************
 */
int mo_bind(mo_object_t *module, int socket, const struct sockaddr *name, socklen_t namelen)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(name != OS_NULL);

    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock)
    {
        ERROR("Module %s get bind socket %d failed!", module->name, socket);
        return -1;
    }

    ip_addr_t   remote_ip   = {0};
    os_uint16_t remote_port = 0;

    //socketaddr_to_ipaddr_port(name, &remote_ip, &remote_port);
    SOCKADDR_TO_IPADDR_PORT(name, &remote_ip, &remote_port);

    os_err_t result = mo_netconn_bind(module, sock->netconn, remote_ip, remote_port);
    if (result != OS_EOK)
    {
        return -1;
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will bind socket to local addr with data recv callback.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket to bind.
 * @param[in]       name          loaal address.
 * @param[in]       namelen       address lenth.
 * @param[in]       cb            data recv callback.
 *
 * @return          status.
 * @retval          0             success.
 * @retval          others        failed.
 ***********************************************************************************************************************
 */
int mo_bind_with_cb(mo_object_t *module, int socket, const struct sockaddr *name, socklen_t namelen, mo_netconn_data_callback cb)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(name != OS_NULL);

    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock || OS_NULL == sock->netconn)
    {
        ERROR("Module %s get bind socket %d failed!", module->name, socket);
        return -1;
    }

    ip_addr_t   remote_ip   = {0};
    os_uint16_t remote_port = 0;

    //socketaddr_to_ipaddr_port(name, &remote_ip, &remote_port);
    SOCKADDR_TO_IPADDR_PORT(name, &remote_ip, &remote_port);

    os_err_t result = mo_netconn_bind(module, sock->netconn, remote_ip, remote_port);
    if (result != OS_EOK)
    {
        return -1;
    }

#ifdef MOLINK_USING_SOCKETS_OPS
    sock->netconn->data_func = cb;
#endif
    
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will connect to server via the molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket to close.
 * @param[in]       name          server address.
 * @param[in]       namelen       address lenth.
 *
 * @return          status.
 * @retval          0             success.
 * @retval          others        failed.
 ***********************************************************************************************************************
 */
int mo_connect(mo_object_t *module, int socket, const struct sockaddr *name, socklen_t namelen)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(name != OS_NULL);

    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock)
    {
        ERROR("Module %s get connect socket %d failed!", module->name, socket);
        return -1;
    }

    ip_addr_t   remote_ip   = {0};
    os_uint16_t remote_port = 0;

    //socketaddr_to_ipaddr_port(name, &remote_ip, &remote_port);
    SOCKADDR_TO_IPADDR_PORT(name, &remote_ip, &remote_port);

    os_err_t result = mo_netconn_connect(module, sock->netconn, remote_ip, remote_port);
    if (result != OS_EOK)
    {
        return -1;
    }

    return 0;
}

static int module_tcp_send(mo_object_t *module, mo_netconn_t *netconn, const char *data, os_size_t size)
{
    if (netconn->stat != NETCONN_STAT_CONNECT)
    {
        ERROR("Module %s send data failed, socket %d is not connected", module->name, netconn->socket_id);
        return -1;
    }

    os_size_t sent_size = mo_netconn_send(module, netconn, data, size);
    if (sent_size <= 0)
    {
        return -1;
    }

    return sent_size;
}

static int module_udp_sendto(mo_object_t *          module,
                             mo_netconn_t *         netconn,
                             const char *           data,
                             os_size_t              size,
                             const struct sockaddr *to,
                             socklen_t              tolen)
{
    OS_ASSERT(module != OS_NULL);
    
    ip_addr_t   remote_ip   = {0};
    os_uint16_t remote_port = 0;
    os_size_t   sent_size   = 0;

    /* XXX molink sockaddr validation check */
    if ((to != NULL) && (tolen != 0))
    {
        SOCKADDR_TO_IPADDR_PORT(to, &remote_ip, &remote_port);
        if (netconn->stat != NETCONN_STAT_CONNECT)
        {
            /* UDP netconn is not connected */
            os_err_t result = mo_netconn_connect(module, netconn, remote_ip, remote_port);
            if (result != OS_EOK)
            {
                return -1;
            }
        }
        sent_size = mo_netconn_sendto(module, netconn, remote_ip, remote_port, data, size);
    }
    else
    {
        if (netconn->stat != NETCONN_STAT_CONNECT)
        {
            // remote_port = 0;
            // ip_addr_set_any((NETCONN_TYPE_UDP_V6 == netconn->type), &addr);
            ERROR("Module %s send without specific addr & port before connect!", module->name);
            return -1;
        }
        else
        {
            sent_size = mo_netconn_send(module, netconn, data, size);
        }
    }

    if (sent_size <= 0)
    {
        return -1;
    }

    return sent_size;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will send data via molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket.
 * @param[in]       dataptr       data to send.
 * @param[in]       size          buffer lenth.
 * @param[in]       flags         operation type.
 * @param[in]       to            address where data send to.
 * @param[in]       tolen         address lenth.
 *
 * @return          status.
 * @retval          -1            failed.
 * @retval          others        size of data sent successfully
 ***********************************************************************************************************************
 */
int mo_sendto(mo_object_t *          module,
              int                    socket,
              const void *           data,
              size_t                 size,
              int                    flags,
              const struct sockaddr *to,
              socklen_t              tolen)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(data != OS_NULL);

    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock)
    {
        ERROR("Module %s get send socket %d failed!", module->name, socket);
        return -1;
    }

    int result = -1;
    switch (sock->netconn->type)
    {
    case NETCONN_TYPE_TCP:
    case NETCONN_TYPE_TCP_V6:
        result = module_tcp_send(module, sock->netconn, (const char *)data, size);
        break;
    case NETCONN_TYPE_UDP:
    case NETCONN_TYPE_UDP_V6:
        result = module_udp_sendto(module, sock->netconn, (const char *)data, size, to, tolen);
        break;     
    default:
        break;
    }

    if (result < 0)
    {
        ERROR("Module %s socket id %d send failed!", module->name, socket);
        return -1;
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will send data via molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket.
 * @param[in]       dataptr       data to send.
 * @param[in]       size          buffer lenth.
 * @param[in]       flags         operation type.
 * @param[in]       to            address where data send to.
 * @param[in]       tolen         address lenth.
 *
 * @return          status.
 * @retval          -1            failed.
 * @retval          others        size of data sent successfully
 ***********************************************************************************************************************
 */
int mo_send(mo_object_t *module, int socket, const void *data, size_t size, int flags)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(data != OS_NULL);

    return mo_sendto(module, socket, data, size, flags, OS_NULL, 0);
}

static int module_recv_tcp(mo_object_t *module, mo_sock_t *sock, void *mem, size_t len, int flags)
{
    os_size_t   recvd    = 0;
    void *      data_ptr = OS_NULL;
    os_size_t   data_len = 0;
    os_err_t    result   = OS_EOK;
    os_uint16_t copylen  = 0;

    os_size_t recv_left = (len <= OS_UINT32_MAX) ? (ssize_t)len : OS_UINT32_MAX;

    os_tick_t timeout = os_tick_from_ms(sock->recv_timeout);

    do
    {
        MO_SYS_ARCH_PROTECT;
        /* Check if there is data left from the last recv operation. */
        if (sock->lastdata != OS_NULL)
        {
            data_ptr = sock->lastdata;
            data_len = sock->lastlen;
        }
        else
        {
            MO_SYS_ARCH_UNPROTECT;
            /* No data was left from the previous operation, so we try to get some from the network. */
            result =  mo_netconn_recvfrom(module, sock->netconn, &data_ptr, &data_len, OS_NULL, OS_NULL, timeout);
            if (result != OS_EOK)
            {
                if (recvd > 0)
                {
                    return recvd;
                }

                if (OS_ERROR == result)
                {
                    ERROR("Module %s receive error, socket %d state %d error",
                          module->name,
                          sock->netconn->socket_id,
                          sock->netconn->stat);
                    return 0;
                }
                else if (OS_ETIMEOUT == result || OS_EEMPTY == result)
                {
                    DEBUG("Module %s socket %d receive (%d ticks) timeout",
                         module->name,
                         sock->netconn->socket_id,
                         os_tick_from_ms(sock->recv_timeout));

                    errno = EAGAIN;
                    return -1;
                }
            }

            MO_SYS_ARCH_PROTECT;
            sock->lastdata   = data_ptr;
            sock->lastlen    = data_len;
            sock->lastoffset = 0;
        }

        if (recv_left > data_len)
        {
            copylen = data_len;
        }
        else
        {
            copylen = (os_uint16_t)recv_left;
        }
        if (recvd + copylen < recvd)
        {
            /* overflow */
            copylen = (os_uint16_t)(OS_UINT32_MAX - recvd);
        }

        memcpy((os_uint8_t *)mem + recvd, (os_uint8_t *)data_ptr + sock->lastoffset, copylen);

        recvd += copylen;
        recv_left -= copylen;

        /* ... check if there is data left in the buf */
        if (sock->lastlen - copylen > 0)
        {
            sock->lastlen -= copylen;
            sock->lastoffset += copylen;
            MO_SYS_ARCH_UNPROTECT;
        }
        else
        {
            sock->lastdata   = OS_NULL;
            sock->lastlen    = 0;
            sock->lastoffset = 0;
            MO_SYS_ARCH_UNPROTECT;

            os_free(data_ptr);
        }

        /* once we have some data to return, only add more if we don't need to wait */
        timeout = OS_NO_WAIT;

    } while (recv_left > 0);

    return recvd;
}

static int module_recvfrom_udp(mo_object_t *module, mo_sock_t *sock, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    os_size_t data_len = 0;
    void *    data_tmp = OS_NULL;
    os_err_t  result   = OS_EOK;
    ip_addr_t addr;
    os_uint16_t port;
    
    result =  mo_netconn_recvfrom(module, sock->netconn, &data_tmp, &data_len, &addr, &port, os_tick_from_ms(sock->recv_timeout));
    if (OS_ERROR == result)
    {
        ERROR("Module %s receive error, socket %d state %d error",
              module->name,
              sock->netconn->socket_id,
              sock->netconn->stat);
        return 0;
    }
    else if (OS_ETIMEOUT == result || OS_EEMPTY == result)
    {
#if 0        
        ERROR("Module %s socket %d receive (%d ticks) timeout",
              module->name,
              sock->netconn->socket_id,
              os_tick_from_ms(sock->recv_timeout));
#endif              
        errno = EAGAIN;
        return -1;
    }

    if (data_len > len)
    {
        WARN("The actual udp data received %d is longer than expected %d, and the excess data will be truncated",
             data_len,
             len);
        data_len = len;
    }

    memcpy(mem, data_tmp, data_len);
    os_free(data_tmp);
	//TODO: to check fromlen ?
	IPADDR_PORT_TO_SOCKADDR(from, &addr, &port);
    

    if (data_len > 0)
    {
        os_set_errno(0);
    }

    return data_len;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will receive data via molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket.
 * @param[in]       mem           buffer to receive data.
 * @param[in]       len           buffer lenth.
 * @param[in]       flags         operation type.
 * @param[in]       from          address from where to receive.
 * @param[in]       fromlen       address lenth.
 *
 * @return          status.
 * @retval          -1            failed.
 * @retval          others        size of received data.
 ***********************************************************************************************************************
 */
int mo_recvfrom(mo_object_t *    module,
                int              socket,
                void *           mem,
                size_t           len,
                int              flags,
                struct sockaddr *from,
                socklen_t *      fromlen)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(mem != OS_NULL);

    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock)
    {
        ERROR("Module %s get recv socket %d failed!", module->name, socket);
        return -1;
    }

    if (NETCONN_TYPE_TCP == sock->netconn->type)
    {
        return module_recv_tcp(module, sock, mem, len, flags);
    }
    else /* NETCONN_TYPE_UDP */
    {
        return module_recvfrom_udp(module, sock, mem, len, flags, from, fromlen);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function will receive data via molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       s             socket.
 * @param[in]       mem           data to send.
 * @param[in]       len           buffer lenth.
 * @param[in]       flags         operation type.
 *
 * @return          status.
 * @retval          -1            failed.
 * @retval          others        size of received data.
 ***********************************************************************************************************************
 */
int mo_recv(mo_object_t *module, int socket, void *mem, size_t len, int flags)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(mem != OS_NULL);

    return mo_recvfrom(module, socket, mem, len, flags, OS_NULL, OS_NULL);
}

/**
 ***********************************************************************************************************************
 * @brief           This function will get option of the molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket.
 * @param[in]       level         level of option.
 * @param[in]       optname       option to get.
 * @param[in]       optval        option value.
 * @param[in]       optlen        option lenth
 *
 * @return          status.
 * @retval          0             success.
 * @retval          others        failed.
 ***********************************************************************************************************************
 */
int mo_getsockopt(mo_object_t *module, int socket, int level, int optname, void *optval, socklen_t *optlen)
{
    OS_ASSERT(optval != OS_NULL);
    OS_ASSERT(optlen != OS_NULL);

    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock)
    {
        ERROR("Module %s get getsockopt socket %d failed!", module->name, socket);
        return -1;
    }

    os_int32_t timeout = 0;

    switch (level)
    {
    case SOL_SOCKET:
        switch (optname)
        {
        case SO_RCVTIMEO:
            timeout                               = sock->recv_timeout;
            ((struct timeval *)(optval))->tv_sec  = (timeout) / 1000U;
            ((struct timeval *)(optval))->tv_usec = (timeout % 1000U) * 1000U;
            break;
        default:
            ERROR("Module %s socket %d not support option name :%d.", module->name, socket, optname);
            return -1;
        }
        break;
    default:
        ERROR("Module %s socket %d not support option level : %d.", module->name, socket, level);
        return -1;
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set option of the molink module socket.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       socket        socket.
 * @param[in]       level         level of option.
 * @param[in]       optname       option to get.
 * @param[in]       optval        option value.
 * @param[in]       optlen        option lenth
 *
 * @return          status.
 * @retval          0             success.
 * @retval          others        failed.
 ***********************************************************************************************************************
 */
int mo_setsockopt(mo_object_t *module, int socket, int level, int optname, const void *optval, socklen_t optlen)
{
    OS_ASSERT(optval != OS_NULL);

    mo_sock_t *sock = mo_get_socket(socket);
    if (OS_NULL == sock)
    {
        ERROR("Module %s get setsockopt socket %d failed!", module->name, socket);
        return -1;
    }

    switch (level)
    {
    case SOL_SOCKET:
        switch (optname)
        {
        case SO_RCVTIMEO:
            sock->recv_timeout =
                ((const struct timeval *)optval)->tv_sec * 1000 + ((const struct timeval *)optval)->tv_usec / 1000;
            break;
        case SO_SNDTIMEO:
            break;
        default:
            ERROR("Module %s socket %d not support option name :%d.", module->name, socket, optname);
            return -1;
        }
        break;
    default:
        ERROR("Module %s socket %d not support option level : %d.", module->name, socket, level);
        return -1;
    }

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will return hosten struct of the host.
 *
 * @param[in]       module        The descriptor of molink module instance
 * @param[in]       name          host to query.
 *
 * @return          hostent struct.
 ***********************************************************************************************************************
 */
struct hostent *mo_gethostbyname(mo_object_t *module, const char *name)
{
    OS_ASSERT(module != OS_NULL);
    OS_ASSERT(name != OS_NULL);

    ip_addr_t addr;

    /* buffer variables for mo_gethostbyname() */
    static struct hostent s_hostent;
    static char *         s_aliases;
    static ip_addr_t      s_hostent_addr;
    static ip_addr_t *    s_phostent_addr[2];
    static char           s_hostname[MOLINK_DNS_MAX_NAME_LEN + 1];

    os_err_t result = mo_netconn_gethostbyname(module, name, &addr);
    if (result != OS_EOK)
    {
        ERROR("Module %s gethostbyname failed", module->name);
        return OS_NULL;
    }

    /* fill hostent */
    s_hostent_addr     = addr;
    s_phostent_addr[0] = &s_hostent_addr;
    s_phostent_addr[1] = OS_NULL;

    strncpy(s_hostname, name, MOLINK_DNS_MAX_NAME_LEN);
    s_hostname[MOLINK_DNS_MAX_NAME_LEN] = 0;

    s_hostent.h_name      = s_hostname;
    s_aliases             = OS_NULL;
    s_hostent.h_aliases   = &s_aliases;
    s_hostent.h_addrtype  = AF_INET;
    s_hostent.h_length    = sizeof(ip_addr_t);
    s_hostent.h_addr_list = (char **)&s_phostent_addr;

    return &s_hostent;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will get address information.
 *
 * @param[in]       nodename      host.
 * @param[in]       servname      server.
 * @param[in]       hints         address info need to get.
 * @param[out]      res           used internal.
 *
 * @return          status.
 * @retval          -1            failed.
 * @retval          others        success.
 ***********************************************************************************************************************
 */
int mo_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
    mo_object_t *module = mo_get_default();
    if (OS_NULL == module)
    {
        return EAI_FAIL;
    }

    int ai_family;

    if (OS_NULL == res)
    {
        return EAI_FAIL;
    }

    *res = OS_NULL;

    if ((OS_NULL == nodename) && (OS_NULL == servname))
    {
        return EAI_NONAME;
    }

    if (hints != OS_NULL)
    {
        ai_family = hints->ai_family;
        if (ai_family != AF_UNSPEC && ai_family != AF_INET)
        {
            return EAI_FAMILY;
        }
    }

    int port_nr = 0;

    if (servname != OS_NULL)
    {
        /* service name specified: convert to port number */
        port_nr = atoi(servname);
        if ((port_nr <= 0) || (port_nr > 0xffff))
        {
            return EAI_SERVICE;
        }
    }

    ip_addr_t addr;

    if (nodename != OS_NULL)
    {
        /* service location specified, try to resolve */
        if ((hints != OS_NULL) && (hints->ai_flags & AI_NUMERICHOST))
        {
            /* no DNS lookup, just parse for an address string */
            if (!inet_aton(nodename, &addr))
            {
                return EAI_NONAME;
            }

            if (AF_INET == ai_family)
            {
                return EAI_NONAME;
            }
        }
        else
        {
            if (mo_netconn_gethostbyname(module, nodename, &addr) != OS_EOK)
            {
                return EAI_FAIL;
            }
        }
    }
    else
    {
        /* TODO: service location specified, use loopback address */
    }

    os_size_t total_size = sizeof(struct addrinfo) + sizeof(struct sockaddr_storage);
    os_size_t namelen    = 0;
    if (nodename != OS_NULL)
    {
        namelen = strlen(nodename);
        if (namelen > MOLINK_DNS_MAX_NAME_LEN)
        {
            /* invalid name length */
            return EAI_FAIL;
        }
        OS_ASSERT(total_size + namelen + 1 > total_size);
        total_size += namelen + 1;
    }

    OS_ASSERT(total_size <= sizeof(struct addrinfo) + sizeof(struct sockaddr_storage) + MOLINK_DNS_MAX_NAME_LEN + 1);
    struct addrinfo *ai = os_calloc(1, total_size);
    if (OS_NULL == ai)
    {
        return EAI_MEMORY;
    }

    /* cast through void* to get rid of alignment warnings */
    struct sockaddr_storage *sa  = (struct sockaddr_storage *)(void *)((uint8_t *)ai + sizeof(struct addrinfo));
    struct sockaddr_in *     sa4 = (struct sockaddr_in *)sa;
    /* set up sockaddr */
#if defined(MOLINK_USING_IPV4) && defined(MOLINK_USING_IPV6)
    sa4->sin_addr.s_addr = addr.u_addr.ip4.addr;
    //sa4->type            = IPADDR_TYPE_V4;
#elif defined(MOLINK_USING_IPV4)
    sa4->sin_addr.s_addr = addr.addr;
#elif defined(MOLINK_USING_IPV6)
#error "Not support IPV6."
#endif /* MOLINK_USING_IPV4 && MOLINK_USING_IPV6 */
    sa4->sin_family = AF_INET;
    sa4->sin_len    = sizeof(struct sockaddr_in);
    sa4->sin_port   = htons((os_uint16_t)port_nr);
    ai->ai_family   = AF_INET;

    /* set up addrinfo */
    if (hints != OS_NULL)
    {
        /* copy socktype & protocol from hints if specified */
        ai->ai_socktype = hints->ai_socktype;
        ai->ai_protocol = hints->ai_protocol;
    }
    if (nodename != OS_NULL)
    {
        /* copy nodename to canonname if specified */
        ai->ai_canonname = ((char *)ai + sizeof(struct addrinfo) + sizeof(struct sockaddr_storage));
        memcpy(ai->ai_canonname, nodename, namelen);
        ai->ai_canonname[namelen] = 0;
    }
    ai->ai_addrlen = sizeof(struct sockaddr_storage);
    ai->ai_addr    = (struct sockaddr *)sa;

    *res = ai;

    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will free the addrinfo.
 *
 * @param[in]       ai            address info struct.
 ***********************************************************************************************************************
 */
void mo_freeaddrinfo(struct addrinfo *ai)
{
    struct addrinfo *next = OS_NULL;

    while (ai != OS_NULL)
    {
        next = ai->ai_next;
        os_free(ai);
        ai = next;
    }
}

/**
 * Go through the readset and writeset lists and see which socket of the sockets
 * set in the sets has events. On return, readset, writeset and exceptset have
 * the sockets enabled that had events.
 *
 * @param maxfdp1 the highest socket index in the sets
 * @param readset_in    set of sockets to check for read events
 * @param writeset_in   set of sockets to check for write events
 * @param exceptset_in  set of sockets to check for error events
 * @param readset_out   set of sockets that had read events
 * @param writeset_out  set of sockets that had write events
 * @param exceptset_out set os sockets that had error events
 * @return number of sockets that had events (read/write/exception) (>= 0)
 */
static int mo_selscan(int     maxfdp1,
                      fd_set *readset_in,
                      fd_set *writeset_in,
                      fd_set *exceptset_in,
                      fd_set *readset_out,
                      fd_set *writeset_out,
                      fd_set *exceptset_out)
{
    fd_set     lreadset;
    fd_set     lwriteset;
    fd_set     lexceptset;
    mo_sock_t *sock;
    void *     lastdata;
    os_int8_t  rcvevent;
    os_uint8_t sendevent;
    os_uint8_t errevent;
    int        nready;
    int        index;

    FD_ZERO(&lreadset);
    FD_ZERO(&lwriteset);
    FD_ZERO(&lexceptset);
    nready = 0;

    // Go through each socket in each list to count number of sockets which currently match
    for (index = 0; index < maxfdp1; index++)
    {
        // if this FD is not in the set, continue
        if (!(readset_in && FD_ISSET(index, readset_in)) && !(writeset_in && FD_ISSET(index, writeset_in)) &&
            !(exceptset_in && FD_ISSET(index, exceptset_in)))
        {
            continue;
        }
        // First get the sock's status (protected)
        MO_SYS_ARCH_PROTECT;
        sock = mo_get_socket(index);
        if (NULL != sock)
        {
            lastdata  = sock->lastdata;
            rcvevent  = sock->rcvevent;
            sendevent = sock->sendevent;
            errevent  = sock->errevent;

            MO_SYS_ARCH_UNPROTECT;

            // See if netconn of this socket is ready for read
            if (readset_in && FD_ISSET(index, readset_in) && ((lastdata != NULL) || (rcvevent > 0)))
            {
                FD_SET(index, &lreadset);
                nready++;
            }
            // See if netconn of this socket is ready for write
            if (writeset_in && FD_ISSET(index, writeset_in) && (sendevent != 0))
            {
                FD_SET(index, &lwriteset);
                nready++;
            }
            // See if netconn of this socket had an error
            if (exceptset_in && FD_ISSET(index, exceptset_in) && (errevent != 0))
            {
                FD_SET(index, &lexceptset);
                nready++;
            }
        }
        else
        {
            MO_SYS_ARCH_UNPROTECT;
            // continue on to next FD in list
        }
    }

    // copy local sets to the ones provided as arguments
    *readset_out   = lreadset;
    *writeset_out  = lwriteset;
    *exceptset_out = lexceptset;

    return nready;
}

static os_err_t mo_do_sem_init(os_sem_t *sem, os_uint32_t value, os_uint32_t max_value)
{
    char       name[OS_NAME_MAX + 1];
    static int cnt = 0;

    snprintf(name, OS_NAME_MAX, "slt_sem_%d", cnt++);
    return os_sem_init(sem, name, value, max_value);
}

/**
 *
 * timeout, timeout == NULL, wait forever
 */
int mo_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    fd_set              lreadset;
    fd_set              lwriteset;
    fd_set              lexceptset;
    struct mo_select_cb select_cb;
    mo_sock_t *         sock;
    os_err_t            op_err;
    os_tick_t           timeout_tick;
    int                 maxfdp2;
    int                 nready;
    int                 index;

    do
    {
        if (maxfdp1 > MOLINK_SOCKETS_FD_MAX)
        {
            maxfdp1 = MOLINK_SOCKETS_FD_MAX;
        }

        nready = mo_selscan(maxfdp1, readset, writeset, exceptset, &lreadset, &lwriteset, &lexceptset);
        /*
         * case 1: find fdset is set, return
         * case 2: no fdset is set and timeout==0,
         * these cases direct return
         */
        if (nready > 0)
        {
            break;
        }
        else if (timeout && timeout->tv_sec == 0 && timeout->tv_usec == 0)
        {
            break;
        }

        // create select_cb wait event
        os_slist_init(&select_cb.slist);
        select_cb.readset       = readset;
        select_cb.writeset      = writeset;
        select_cb.exceptset     = exceptset;
        select_cb.sem_signalled = 0;
        // init sem invalid, one sem for thread
        op_err = mo_do_sem_init(&select_cb.sem, 0, OS_SEM_MAX_VALUE);
        if (OS_EOK != op_err)
        {
            return OS_ERROR;
        }

        // Protect the select_cb_list
        MO_SYS_ARCH_PROTECT;

        // Put this select_cb on head of slist[mo_select_cb_slist]
        os_slist_add(&gs_mo_select_cb_slist, &select_cb.slist);
        // Increasing this counter tells event_callback that the list has changed
        gs_mo_select_cb_ctr++;

        /* Now we can safely unprotect */
        MO_SYS_ARCH_UNPROTECT;

        /* Increase select_waiting for each socket we are interested in */
        maxfdp2 = maxfdp1;
        for (index = 0; index < maxfdp1; index++)
        {
            if ((readset && FD_ISSET(index, readset)) || (writeset && FD_ISSET(index, writeset)) ||
                (exceptset && FD_ISSET(index, exceptset)))
            {
                MO_SYS_ARCH_PROTECT;
                sock = mo_tryget_socket(index);
                if (OS_NULL != sock)
                {
                    sock->select_waiting++;
                }

                // Not a valid socket, socket may closed
                if (OS_NULL == sock || OS_NULL == sock->netconn)
                {
                    nready  = -1;
                    maxfdp2 = index + 1;
                    MO_SYS_ARCH_UNPROTECT;
                    break;
                }

                MO_SYS_ARCH_UNPROTECT;
            }
        }
        op_err = OS_EOK;

        if (!nready)
        {
            /** Call mo_selscan again: there could have been events between
             *  the last scan (without us on the list) and putting us on the list!
             */
            nready = mo_selscan(maxfdp1, readset, writeset, exceptset, &lreadset, &lwriteset, &lexceptset);
            if (!nready)
            {
                // Still none ready, just wait to be woken
                if (NULL == timeout)
                {
                    timeout_tick = OS_WAIT_FOREVER;
                }
                else
                {
                    timeout_tick = os_tick_from_ms((timeout->tv_sec * 1000) + ((timeout->tv_usec + 500) / 1000));
                    if (!timeout_tick)
                    {
                        timeout_tick = 1;
                    }
                }

                op_err = os_sem_wait(&select_cb.sem, timeout_tick);
            }
        }

        /* Decrease select_waiting for each socket we are interested in */
        for (index = 0; index < maxfdp2; index++)
        {
            if ((readset && FD_ISSET(index, readset)) || (writeset && FD_ISSET(index, writeset)) ||
                (exceptset && FD_ISSET(index, exceptset)))
            {
                MO_SYS_ARCH_PROTECT;
                sock = mo_tryget_socket(index);
                if (OS_NULL != sock)
                {
                    if (sock->select_waiting > 0)
                    {
                        sock->select_waiting--;
                    }
                }
                if (OS_NULL == sock || OS_NULL == sock->netconn)
                {
                    // Not a valid socket, socket may closed
                    nready = -1;
                }
                MO_SYS_ARCH_UNPROTECT;
            }
        }

        // Take us off the list
        MO_SYS_ARCH_PROTECT;
        os_slist_del(&gs_mo_select_cb_slist, &select_cb.slist);
        // Increasing this counter tells event_callback that the list has changed
        gs_mo_select_cb_ctr++;
        MO_SYS_ARCH_UNPROTECT;
        os_sem_deinit(&select_cb.sem);

        if (nready < 0)
        {
            return OS_ERROR;
        }
        else if (OS_ETIMEOUT == op_err)
        {
            DEBUG("mo_select: timeout expired");
            break;
        }

        /* See what's set */
        nready = mo_selscan(maxfdp1, readset, writeset, exceptset, &lreadset, &lwriteset, &lexceptset);
    } while (0);

    if (readset)
    {
        *readset = lreadset;
    }
    if (writeset)
    {
        *writeset = lwriteset;
    }
    if (exceptset)
    {
        *exceptset = lexceptset;
    }

    return nready;
}

#ifdef OS_USING_IO_MULTIPLEXING
int mo_poll(int socket, struct vfs_pollfd *req, os_bool_t poll_setup)
{
    mo_sock_t       *sock;
    int              mask = 0;
    os_slist_node_t *node;
    os_slist_node_t *tmp_node;
    poll_req_t      *poll_req = OS_NULL;

    sock = mo_get_socket(socket);

    if (OS_NULL == sock)
    {
        mask = POLLERR;
        return mask;
    }

    if (OS_TRUE == poll_setup)
    {
        poll_req = os_calloc(1, sizeof(poll_req_t));
        if (OS_NULL == poll_req)
        {
            ERROR("%s: no enough memory.", __func__);
            mask = POLLERR;
            return mask;
        }

        MO_SYS_ARCH_PROTECT;
        poll_req->req = req;
        os_slist_add_tail(&sock->req_slist_head, &poll_req->req_list);
        MO_SYS_ARCH_UNPROTECT;
    }
    else // poll end
    {
        os_slist_for_each_safe(node, tmp_node, &sock->req_slist_head)
        {
            poll_req = os_slist_entry(node, poll_req_t, req_list);
            if (req == poll_req->req)
            {
                MO_SYS_ARCH_PROTECT;
                os_slist_del(&sock->req_slist_head, &poll_req->req_list);
                MO_SYS_ARCH_UNPROTECT;

                os_free(poll_req);
            }
        }
        return 0;
    }

    MO_SYS_ARCH_PROTECT;

    /* POLLIN */
    if (req->events & POLLIN)
    {
        if (sock->rcvevent || sock->lastdata != NULL)
        {
            mask |= POLLIN;
        }
    }

    /* POLLOUT */
    if (req->events & POLLOUT)
    {
        if (sock->sendevent)
        {
            mask |= POLLOUT;
        }
    }

    /* ERROR */
    if (req->events & POLLERR)
    {
        if (sock->errevent)
        {
            mask |= POLLERR;
        }
    }

    MO_SYS_ARCH_UNPROTECT;

    if (mask != 0)
    {
        devfs_poll_notify(req, mask);
    }

    return mask;
}
#endif

#endif /* MOLINK_USING_SOCKETS_OPS */
