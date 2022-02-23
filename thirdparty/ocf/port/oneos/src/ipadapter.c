/*
// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "oc_buffer.h"
#include "oc_endpoint.h"
#include "port/oc_connectivity.h"
#include <errno.h>
#include <stdio.h>
#include "os_mutex.h"
#include "os_assert.h"

#ifdef NET_USING_MOLINK
#include "mo_api.h"
#include "sys/socket.h"
#endif

#ifdef NET_USING_LWIP
#include "os_clock.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/sockets.h"
#endif

#ifdef NET_USING_LWIP
extern void lwip_get_intf_ipaddr(char *intf_name, ip_addr_t *addr);
#endif

/* For synchronizing the network receive thread with IoTivity-Lite's
 * event loop.
 */
static os_mutex_t oc_ev_mutex;

void oc_network_event_handler_mutex_init(void)
{
    // k_sem_init(&sem, 0, 1);
    // k_sem_give(&sem);
#ifdef ONEOS_2_0
    os_mutex_init(&oc_ev_mutex, "ocmt", OS_FALSE);
#else
    os_mutex_init(&oc_ev_mutex, "ocmt", OS_IPC_FLAG_FIFO, OS_FALSE);  
#endif
}

void oc_network_event_handler_mutex_lock(void)
{
    // k_sem_take(&sem, K_FOREVER);
#ifdef ONEOS_2_0
    os_mutex_lock(&oc_ev_mutex, OS_WAIT_FOREVER);
#else
    os_mutex_lock(&oc_ev_mutex, OS_IPC_WAITING_FOREVER);
#endif
}

void oc_network_event_handler_mutex_unlock(void)
{
    // k_sem_give(&sem);
#ifdef ONEOS_2_0
    os_mutex_unlock(&oc_ev_mutex);
#else
    os_mutex_unlock(&oc_ev_mutex);  
#endif
}

void oc_network_event_handler_mutex_destroy(void) {}

#ifdef OC_IPV6
/* Server's receive socket */
static struct net_context *udp_recv6;

/* "All OCF nodes" multicast address and port */
#define OCF_MCAST_IP6ADDR                                                  \
    {                                                                      \
        {                                                                  \
            {                                                              \
                0xff, 0x02, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x01, 0x58 \
            }                                                              \
        }                                                                  \
    }
static struct in6_addr in6addr_mcast = OCF_MCAST_IP6ADDR;
#define OCF_MCAST_PORT (5683)
/* Multicast receive socket */
static struct net_context *mcast_recv6;
static struct sockaddr_in6 mcast_addr6;
static struct sockaddr_in6 my_addr6;

static struct in6_addr in6addr_my;

#ifdef OC_SECURITY
/* DTLS receive socket */
static struct net_context *dtls_recv6;
static struct sockaddr_in6 dtls_addr6;
#define MY_DTLS_PORT (56789)
#endif /* OC_SECURITY */
static oc_endpoint_t *eps;

static void oc_network_receive(struct net_context *context, struct net_pkt *pkt,
                               int status, void *user_data)
{
    oc_message_t *message = oc_allocate_message();

    if (message)
    {
        uint16_t pos;
        struct net_udp_hdr *udp =
            (struct net_udp_hdr *)((u8_t *)(NET_IPV6_HDR(pkt)) +
                                   sizeof(struct net_ipv6_hdr));
        size_t bytes_read = net_pkt_appdatalen(pkt);
        if (bytes_read < 0)
        {
            oc_message_unref(message);
            return;
        }

        size_t offset_from_start = net_pkt_get_len(pkt) - bytes_read;
        bytes_read = (bytes_read < OC_PDU_SIZE) ? bytes_read : OC_PDU_SIZE;
        struct net_buf *frag = net_frag_read(pkt->frags, offset_from_start, &pos,
                                             bytes_read, message->data);
        if (!frag && pos == 0xffff)
        {
            net_pkt_unref(pkt);
            oc_message_unref(message);
            return;
        }

        message->length = bytes_read;
        if (user_data != NULL)
        {
            message->endpoint.flags = IPV6 | SECURED;
#ifdef OC_SECURITY
            message->encrypted = 1;
#endif /* OC_SECURITY */
        }
        else
            message->endpoint.flags = IPV6;
        memcpy(message->endpoint.addr.ipv6.address, &NET_IPV6_HDR(pkt)->src, 16);
        message->endpoint.addr.ipv6.scope = 0;
        message->endpoint.addr.ipv6.port = ntohs(udp->src_port);
        message->endpoint.device = 0;

#ifdef OC_DEBUG
        PRINT("Incoming message of size %d bytes from ", (int)message->length);
        PRINTipaddr(message->endpoint);
        PRINT("\n\n");
#endif /* OC_DEBUG */

        oc_network_event(message);
    }

    net_pkt_unref(pkt);
}

static inline void
udp_sent(struct net_context *context, int status, void *token, void *user_data)
{
    if (!status)
    {
        OC_DBG("oc_send_buffer: sent %d bytes", POINTER_TO_UINT(token));
    }
    else if (status < 0)
    {
        OC_DBG("oc_send_buffer: failed: (%d)", status);
    }
}

int oc_send_buffer(oc_message_t *message)
{
#ifdef OC_DEBUG
    PRINT("Outgoing message of size %d bytes to ", message->length);
    PRINTipaddr(message->endpoint);
    PRINT("\n\n");
#endif /* OC_DEBUG */

    /* Populate destination address structure */
    struct sockaddr_in6 peer_addr;
    memcpy(peer_addr.sin6_addr.in6_u.u6_addr8,
           message->endpoint.addr.ipv6.address, 16);
    peer_addr.sin6_family = AF_INET6;
    peer_addr.sin6_port = htons(message->endpoint.addr.ipv6.port);

    /* Network buffer to hold data to be sent */
    struct net_pkt *send_pkt;
#ifdef OC_SECURITY
    if (message->endpoint.flags & SECURED)
    {
        send_pkt = net_pkt_get_tx(dtls_recv6, K_NO_WAIT);
    }
    else
#endif /* OC_SECURITY */
    {
        send_pkt = net_pkt_get_tx(udp_recv6, K_NO_WAIT);
    }
    if (!send_pkt)
    {
        OC_WRN("oc_send_buffer: cannot acquire send_pkt");
        return -1;
    }

    bool status = net_pkt_append_all(send_pkt, message->length, message->data, K_NO_WAIT);
    if (!status)
    {
        OC_WRN("oc_send_buffer: cannot populate send_pkt");
        return -1;
    }

    int ret = net_context_sendto(
        send_pkt, (struct sockaddr *)&peer_addr, sizeof(struct sockaddr_in6),
        udp_sent, 0, UINT_TO_POINTER(net_pkt_get_len(send_pkt)), NULL);
    if (ret < 0)
    {
        OC_WRN("oc_send_buffer: cannot send data to peer (%d)", ret);
        net_pkt_unref(send_pkt);
        return ret;
    }
    return message->length;
}

static void
free_endpoints(void)
{
    oc_endpoint_t *ep = eps, *next;
    while (ep != NULL)
    {
        next = ep->next;
        oc_free_endpoint(ep);
        ep = next;
    }
}

oc_endpoint_t *
oc_connectivity_get_endpoints(size_t device)
{
    (void)device;
    if (!eps)
    {
        oc_endpoint_t *ep = oc_new_endpoint();
        if (!ep)
        {
            return NULL;
        }
        eps = ep;
        memset(ep, 0, sizeof(oc_endpoint_t));
        ep->flags = IPV6;
        net_addr_pton(AF_INET6, CONFIG_NET_APP_MY_IPV6_ADDR, ep->addr.ipv6.address);
        ep->addr.ipv6.port = ntohs(my_addr6.sin6_port);
        ep->device = 0;
#ifdef OC_SECURITY
        oc_endpoint_t *ep_sec = oc_new_endpoint();
        if (ep_sec)
        {
            memset(ep_sec, 0, sizeof(oc_endpoint_t));
            ep_sec->flags = IPV6 | SECURED;
            net_addr_pton(AF_INET6, CONFIG_NET_APP_MY_IPV6_ADDR,
                          ep_sec->addr.ipv6.address);
            ep_sec->addr.ipv6.port = ntohs(dtls_addr6.sin6_port);
            ep_sec->device = 0;
            ep->next = ep_sec;
        }
#endif /* OC_SECURITY */
    }
    return eps;
}

int oc_connectivity_init(size_t device)
{
    (void)device;
    int ret;

    /* Record OCF's multicast address with network interface */
    net_if_ipv6_maddr_add(net_if_get_default(), &in6addr_mcast);

    net_ipaddr_copy(&mcast_addr6.sin6_addr, &in6addr_mcast);
    mcast_addr6.sin6_family = AF_INET6;
    mcast_addr6.sin6_port = htons(OCF_MCAST_PORT);

    /* Wildcard address set for server with randomly chosen port */
    my_addr6.sin6_family = AF_INET6;

    /* Add unicast IPV6 address to interface so that node can communicate */
    /* Would be good to have the address auto-configured in case some router is
     * distributing pre-fixes*/
    if (net_addr_pton(AF_INET6, CONFIG_NET_APP_MY_IPV6_ADDR, &in6addr_my) < 0)
    {
        NET_ERR("Invalid IPv6 address %s", CONFIG_NET_APP_MY_IPV6_ADDR);
    }

#ifdef OC_DEBUG
    struct net_if_addr *ifaddr =
#endif
        net_if_ipv6_addr_add(net_if_get_default(), &in6addr_my, NET_ADDR_MANUAL, 0);
#ifdef OC_DEBUG
    OC_DBG("=====>>>Interface unicast address added @ %p", ifaddr);
#endif

#ifdef OC_SECURITY
    dtls_addr6.sin6_port = htons(MY_DTLS_PORT);
    dtls_addr6.sin6_family = AF_INET6;
#endif /* OC_SECURITY */

    ret = net_context_get(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, &udp_recv6);
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: cannot get UDP network context for server"
               "receive (%d)",
               ret);
        goto error;
    }

    ret = net_context_bind(udp_recv6, (struct sockaddr *)&my_addr6,
                           sizeof(struct sockaddr_in6));
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: cannot bind UDP port %d to server's network"
               "context (%d)",
               ntohs(my_addr6.sin6_port), ret);
        goto error;
    }

    ret = net_context_get(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, &mcast_recv6);
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: cannot get UDP network context for OCF"
               "multicast receive (%d)",
               ret);
        goto error;
    }

    ret = net_context_bind(mcast_recv6, (struct sockaddr *)&mcast_addr6,
                           sizeof(struct sockaddr_in6));
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: cannot bind OCF multicast network context"
               "(%d)",
               ret);
        goto error;
    }

#ifdef OC_SECURITY
    ret = net_context_get(AF_INET6, SOCK_DGRAM, IPPROTO_UDP, &dtls_recv6);
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: cannot get DTLS network context"
               "(%d)",
               ret);
        goto error;
    }

    ret = net_context_bind(dtls_recv6, (struct sockaddr *)&dtls_addr6,
                           sizeof(struct sockaddr_in6));
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: cannot bind DTLS network context"
               "(%d)",
               ret);
        goto error;
    }
#endif /* OC_SECURITY */

    ret = net_context_recv(mcast_recv6, oc_network_receive, 0, NULL);
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: net_context_recv error from multicast socket:"
               "(%d)",
               ret);
        goto error;
    }

    ret = net_context_recv(udp_recv6, oc_network_receive, 0, NULL);
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: net_context_recv error from server socket:"
               "(%d)",
               ret);
        goto error;
    }

#ifdef OC_SECURITY
    static uint16_t dtls_port = MY_DTLS_PORT;
    ret = net_context_recv(dtls_recv6, oc_network_receive, 0, &dtls_port);
    if (ret < 0)
    {
        OC_WRN("oc_connectivity_init: net_context_recv error from DTLS socket:"
               "(%d)",
               ret);
        goto error;
    }
#endif /* OC_SECURITY */

    OC_DBG("oc_connectivity_init: successfully initialized connectivity");
    return 0;

error:
    OC_ERR("oc_connectivity_init: failed to initialize connectivity");
    return -1;
}
#endif

#ifdef OC_IPV4
static oc_endpoint_t *eps;
#define OCF_IPv4_MULTICAST "224.0.1.187"
#define OCF_MCAST_PORT (5683)
#define OCF_UDPV4_PORT (9004)
#ifdef OC_SECURITY
#define MY_DTLS_PORT (56789)
#endif /* OC_SECURITY */
#ifdef NET_USING_LWIP
static struct udp_pcb *gs_client_pcb = OS_NULL;
static struct udp_pcb *gs_tls4_pcb = OS_NULL;
#endif
#ifdef NET_USING_MOLINK
mo_object_t *mo_obj;
#endif

static void get_ipv4_address(uint8_t *ipv4_address)
{
#ifdef NET_USING_LWIP
    ip_addr_t ip_addr;
    do
    {
#ifdef OS_USING_WIFI
        lwip_get_intf_ipaddr(OS_WLAN_DEVICE_STA_NAME, &ip_addr);
#else
        lwip_get_intf_ipaddr("e0", &ip_addr);
#endif
        if (ip_addr.addr)
        {
            OC_DBG("device ip address: %s\n", inet_ntoa(ip_addr));
            memcpy(ipv4_address, (uint8_t *)&ip_addr, 4);
            break;
        }
        os_task_msleep(100);
    } while (1);
#endif
#ifdef NET_USING_MOLINK
    char ipaddr_str[IPADDR_MAX_STR_LEN] = {0};
    ip_addr_t ip_addr;

    do
    {
        mo_get_ipaddr(mo_obj, ipaddr_str);
        inet_aton(ipaddr_str, &ip_addr);
        if (ip_addr.addr)
        {
            OC_DBG("device ip address: %s\n", ipaddr_str);
            memcpy(ipv4_address, (uint8_t *)&ip_addr, 4);
            break;
        }
        os_task_msleep(100);
    } while (1);
#endif
}

oc_endpoint_t *oc_connectivity_get_endpoints(size_t device)
{
    (void)device;
    if (!eps)
    {
        oc_endpoint_t *ep = oc_new_endpoint();
        if (!ep)
        {
            return NULL;
        }
        eps = ep;
        memset(ep, 0, sizeof(oc_endpoint_t));
        ep->flags = IPV4;
        get_ipv4_address(ep->addr.ipv4.address);
#ifdef NET_USING_LWIP       
        ep->addr.ipv4.port = gs_client_pcb->local_port;
#endif
#ifdef NET_USING_MOLINK
        ep->addr.ipv4.port = OCF_UDPV4_PORT; /* TODO: 这里最好从molink从取到发送数据用的本地端口，复用发送和接收监听端口，这样可以省一个socket连接 */
#endif
        ep->device = 0;
#ifdef OC_SECURITY
        oc_endpoint_t *ep_sec = oc_new_endpoint();
        if (ep_sec)
        {
            memset(ep_sec, 0, sizeof(oc_endpoint_t));
            ep_sec->flags = IPV4 | SECURED;
            // net_addr_pton(AF_INET6, CONFIG_NET_APP_MY_IPV6_ADDR,
            //               ep_sec->addr.ipv6.address);
            get_ipv4_address(ep_sec->addr.ipv4.address);

            ep_sec->addr.ipv4.port = MY_DTLS_PORT; //ntohs(dtls_addr6.sin6_port);
            ep_sec->device = 0;
            ep->next = ep_sec;
        }
#endif /* OC_SECURITY */
    }
    return eps;
}

// static void oc_network_receive(struct net_context *context, struct net_pkt *pkt, int status, void *user_data)
static oc_message_t *oc_network_receive_common(ip_addr_t remote_addr, os_uint16_t remote_port, char *data, os_int32_t len)
{
    oc_message_t *message = OS_NULL;

    if (!data || len < 0 || len > OC_PDU_SIZE)
    {
        OC_DBG("rx err %d.", len);
    }
    else
    {
        message = oc_allocate_message();
        if (!message)
        {
            OC_DBG("oc_allocate_message null");
        }
        else
        {
            message->endpoint.device = 0;
            memcpy((void *)message->data, data, len);
            message->length = (size_t)len;
            memcpy(message->endpoint.addr.ipv4.address, &remote_addr.addr, sizeof(remote_addr.addr));
            message->endpoint.addr.ipv4.port = remote_port;
        }
    }

    return message;
}

static void oc_network_receive_udp4(ip_addr_t remote_addr, os_uint16_t remote_port, char *data, os_int32_t len)
{
    oc_message_t *message = oc_network_receive_common(remote_addr, remote_port, data, len);
    if (message)
    {
        message->endpoint.flags = IPV4;
#ifdef OC_DEBUG
        PRINT("\nudp4 Incoming message of size %d bytes from ", len);
        PRINTipaddr(message->endpoint);
        PRINT("\n\n");
#endif /* OC_DEBUG */
        oc_network_event(message);
    }
}

#ifdef OC_SECURITY
static void oc_network_receive_tls4(ip_addr_t remote_addr, os_uint16_t remote_port, char *data, os_int32_t len)
{
    oc_message_t *message = oc_network_receive_common(remote_addr, remote_port, data, len);
    if (message)
    {
        message->endpoint.flags = IPV4 | SECURED;
        message->encrypted = 1;

#ifdef OC_DEBUG
        PRINT("\ntls4 Incoming message of size %d bytes from ", len);
        PRINTipaddr(message->endpoint);
        PRINT("\n\n");
#endif /* OC_DEBUG */
        oc_network_event(message);
    }
}
#endif

static void oc_network_receive_mcast4(ip_addr_t remote_addr, os_uint16_t remote_port, char *data, os_int32_t len)
{
    oc_message_t *message = oc_network_receive_common(remote_addr, remote_port, data, len);
    if (message)
    {
        message->endpoint.flags = IPV4 | MULTICAST;
#ifdef OC_DEBUG
        PRINT("\nmcast Incoming message of size %d bytes from ", len);
        PRINTipaddr(message->endpoint);
        PRINT("\n\n");
#endif /* OC_DEBUG */
        oc_network_event(message);
    }
}

#ifdef NET_USING_MOLINK
int gs_mcat_socket_fd = -1;
int gs_udp4_socket_fd = -1;
#ifdef OC_SECURITY
int gs_tls4_socket_fd = -1;
#endif

static int create_udp_socket(int *socket_fd, char *ip_str, uint16_t port, mo_netconn_data_callback cb)
{
    struct sockaddr_in addr;
    int ret = -1;

    *socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (*socket_fd < 0)
    {
        OC_DBG("Socket error, errno=%d", *socket_fd);
    }
    else
    {
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr(ip_str);
        addr.sin_port = htons(port);

        ret = bind_with_cb(*socket_fd, (struct sockaddr *)&addr, sizeof(addr), cb);
        if (ret < 0)
        {
            OC_ERR("connect %s:%d error, errno=%d", ip_str, port, ret);
            closesocket(*socket_fd);
        }
        else
        {
            OC_DBG("connect %s:%d success", ip_str, port);
        }
    }
    return ret;
}

static void mcast_udp_recv(void *var, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size)
{
    oc_network_receive_mcast4(addr, port, data, size);
}

#ifdef OC_SECURITY
static void tls_udp_recv(void *var, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size)
{
    oc_network_receive_tls4(addr, port, data, size);
}
#endif

static void unicast_udp_recv(void *var, ip_addr_t addr, os_uint16_t port, char *data, os_size_t size)
{
    oc_network_receive_udp4(addr, port, data, size);
}

int oc_connectivity_init(size_t device)
{
    char ipaddr_str[IPADDR_MAX_STR_LEN] = {0};

    //不支持多接口
    OC_DBG(">>>>oc_connectivity_init(%d)", device);
    OS_ASSERT(device == 0);
    OS_ASSERT(mo_obj == OS_NULL);

    mo_obj = mo_get_default();
    OS_ASSERT(mo_obj);

    /* 创建监听多播udp server */
    create_udp_socket(&gs_mcat_socket_fd, OCF_IPv4_MULTICAST, OCF_MCAST_PORT, mcast_udp_recv);

    mo_get_ipaddr(mo_obj, ipaddr_str);
    
    /* 创建监听单播udp server*/
    create_udp_socket(&gs_udp4_socket_fd, ipaddr_str, OCF_UDPV4_PORT, unicast_udp_recv);  


#ifdef OC_SECURITY
    /* 创建监听加密单播udp server */
    create_udp_socket(&gs_tls4_socket_fd, ipaddr_str, MY_DTLS_PORT, tls_udp_recv);
#endif /* OC_SECURITY */

    OC_DBG("oc_connectivity_init: successfully initialized connectivity");
    return 0;
}
#endif

#ifdef NET_USING_LWIP
static void mcast_udp_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    oc_network_receive_mcast4(*addr, port, (char *)p->payload, p->tot_len);
    pbuf_free(p);
}

#ifdef OC_SECURITY
static void tls_udp_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    oc_network_receive_tls4(*addr, port, (char *)p->payload, p->tot_len);
    pbuf_free(p);
}
#endif

static void unicast_udp_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    oc_network_receive_udp4(*addr, port, (char *)p->payload, p->tot_len);
    pbuf_free(p);
}

int oc_connectivity_init(size_t device)
{
    struct udp_pcb *mcast4_pcb = OS_NULL;
    ip_addr_t ip_addr;
    err_t ret = ERR_ARG;

    OC_DBG(">>>>oc_connectivity_init(%d)", device);

    //不支持多接口
    OS_ASSERT(device == 0);

    inet_aton(OCF_IPv4_MULTICAST, &ip_addr);

    /*-------------创建多播监听server-------------------- */
    /* 加入多播组 */
    igmp_joingroup(IP_ADDR_ANY, (struct ip4_addr *)(&ip_addr));

    if (NULL == mcast4_pcb)
        mcast4_pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    OS_ASSERT(mcast4_pcb);

    ip_set_option(mcast4_pcb, SO_BROADCAST);
    ret = udp_bind(mcast4_pcb, IP_ADDR_ANY, OCF_MCAST_PORT);
    if (ERR_OK != ret)
    {
        OC_DBG("udp_bind in port %d err: %d", OCF_MCAST_PORT, ret);
        return ret;
    }
    else
    {
        OC_DBG("udp_bind in port %d success", OCF_MCAST_PORT);
    }
    udp_recv(mcast4_pcb, mcast_udp_recv, NULL);
    /*-------------创建多播监听server-------------------- */

    /* ------------创建发送udp客户端-------------------- */
    if (NULL == gs_client_pcb)
        gs_client_pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    OS_ASSERT(gs_client_pcb);

    ret = udp_bind(gs_client_pcb, IP_ADDR_ANY, OCF_UDPV4_PORT);
    if (ERR_OK != ret)
    {
        OC_DBG("udp_bind any err: %d", ret);
        return ret;
    }
    else
    {
        OC_DBG("udp_bind in port %d success", gs_client_pcb->local_port);
    }
    udp_recv(gs_client_pcb, unicast_udp_recv, NULL);
    /* ------------创建发送udp客户端-------------------- */

#ifdef OC_SECURITY
    /*-------------创建加密监听server-------------------- */
    if (NULL == gs_tls4_pcb)
        gs_tls4_pcb = udp_new_ip_type(IPADDR_TYPE_V4);
    OS_ASSERT(gs_tls4_pcb);

    ret = udp_bind(gs_tls4_pcb, IP_ADDR_ANY, MY_DTLS_PORT);
    if (ERR_OK != ret)
    {
        OC_DBG("udp_bind in port %d err: %d", MY_DTLS_PORT, ret);
        return ret;
    }
    else
    {
        OC_DBG("udp_bind in port %d success", MY_DTLS_PORT);
    }
    udp_recv(gs_tls4_pcb, tls_udp_recv, NULL);
    /*-------------创建加密监听server-------------------- */

#endif /* OC_SECURITY */

    OC_DBG("oc_connectivity_init: successfully initialized connectivity");
    return 0;
}

int send_msg(struct udp_pcb *send_pcb, oc_message_t *message, ip_addr_t remote_addr, os_uint16_t remote_port)
{
    int send_size = 0;
    struct pbuf *p_reply = pbuf_alloc(PBUF_TRANSPORT, message->length, PBUF_RAM);

    if (p_reply)
    {
        memcpy(p_reply->payload, (const char *)message->data, message->length);
        if (ERR_OK == udp_sendto(send_pcb, p_reply, &remote_addr, remote_port))
            send_size = message->length;
        else
            OC_ERR("udp send msg failed");
        pbuf_free(p_reply);
    }
    else
    {
        OC_ERR("pbuf alloc failed");
    }
    OC_DBG("Send %d bytes", send_size);
    return send_size;
}
#endif

int oc_send_buffer(oc_message_t *message)
{
#ifdef OC_DEBUG
    PRINT("Outgoing message of size %d bytes to ", message->length);
    PRINTipaddr(message->endpoint);
    PRINT("\n\n");
#endif /* OC_DEBUG */

#ifdef NET_USING_LWIP
    struct udp_pcb *send_pcb = NULL;
#endif

#ifdef NET_USING_MOLINK
    int send_socket_fd = -1;
#endif

#ifdef OC_SECURITY
    if (message->endpoint.flags & SECURED)
    {
#ifdef OC_IPV4
        if (message->endpoint.flags & IPV4)
        {
#ifdef NET_USING_MOLINK
            send_socket_fd = gs_tls4_socket_fd;
#endif
#ifdef NET_USING_LWIP
            send_pcb = gs_tls4_pcb;
#endif
        }
        else
        {
#ifdef NET_USING_MOLINK
            send_socket_fd = gs_tls4_socket_fd;
#endif
#ifdef NET_USING_LWIP
            send_pcb = gs_tls4_pcb;
#endif
            OS_ASSERT(0);
        }
#else  /* OC_IPV4 */
        OS_ASSERT(0);
#endif /* !OC_IPV4 */
    }
    else
#endif /* OC_SECURITY */
#ifdef OC_IPV4
        if (message->endpoint.flags & IPV4)
    {
#ifdef NET_USING_MOLINK
            send_socket_fd = gs_udp4_socket_fd;
#endif
#ifdef NET_USING_LWIP
            send_pcb = gs_client_pcb;
#endif
    }
    else
    {
        OS_ASSERT(0);
    }
#else /* OC_IPV4 */
    {
        OS_ASSERT(0);
    }
#endif /* !OC_IPV4 */

    ip_addr_t remote_addr;
    memcpy(&remote_addr.addr, message->endpoint.addr.ipv4.address, sizeof(remote_addr.addr));
    os_uint16_t remote_port = message->endpoint.addr.ipv4.port;
#ifdef NET_USING_MOLINK
    struct sockaddr_in addr_in;

    memset(&addr_in, 0, sizeof(addr_in));
    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = remote_addr.addr;
    addr_in.sin_port = htons(remote_port);
    int sent_size = sendto(send_socket_fd, (const char *)message->data, message->length, 0,  (struct sockaddr *)&addr_in, sizeof(struct sockaddr));
    OS_ASSERT(sent_size == message->length);

    return sent_size;
#endif
#ifdef NET_USING_LWIP
    return send_msg(send_pcb, message, remote_addr, remote_port);
#endif
}
#endif

void oc_connectivity_shutdown(size_t device)
{
    (void)device;
#ifdef OC_IPV6
#ifdef OC_SECURITY
    net_context_put(dtls_recv6);
#endif /* OC_SECURITY */
    net_context_put(udp_recv6);
    net_context_put(mcast_recv6);
    free_endpoints();
#endif
}
