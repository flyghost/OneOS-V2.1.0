/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include <os_assert.h>
#include <os_memory.h>

#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/netif.h"
#include "lwip/netifapi.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"

#include "netif/etharp.h"
#include "netif/ethernetif.h"

#include "lwip/inet.h"

#if LWIP_IPV6
#include "lwip/ethip6.h"
#endif /* LWIP_IPV6 */

#if LWIP_IPV6_DHCP6
#include "lwip/dhcp6.h"
#endif

#ifdef OS_USING_SHELL
#include <shell.h>
#endif

#define netifapi_netif_set_link_up(n)   netifapi_netif_common(n, netif_set_link_up, NULL)
#define netifapi_netif_set_link_down(n) netifapi_netif_common(n, netif_set_link_down, NULL)

#ifndef LWIP_ETH_TASK_PRIORITY
#define ETHERNETIF_TASK_PREORITY 0x90
#else
#define ETHERNETIF_TASK_PREORITY LWIP_ETH_TASK_PRIORITY
#endif

#ifndef LWIP_NO_TX_TASK
/**
 * Tx message structure for Ethernet interface
 */
struct eth_tx_msg
{
    struct netif *netif;
    struct pbuf  *buf;
};

static struct os_mb    eth_tx_task_mb;
static struct os_task  eth_tx_task;
#ifndef LWIP_ETH_TASK_MBOX_SIZE
static char eth_tx_task_mb_pool[32 * 4];
static char eth_tx_task_stack[512];
#else
static char eth_tx_task_mb_pool[LWIP_ETH_TASK_MBOX_SIZE * 4];
static char eth_tx_task_stack[LWIP_ETH_TASK_STACKSIZE];
#endif
#endif

#ifndef LWIP_NO_RX_TASK
static struct os_mb    eth_rx_task_mb;
static struct os_task  eth_rx_task;
#ifndef LWIP_ETH_TASK_MBOX_SIZE
static char eth_rx_task_mb_pool[48 * 4];
static char eth_rx_task_stack[1024];
#else
static char eth_rx_task_mb_pool[LWIP_ETH_TASK_MBOX_SIZE * 4];
static char eth_rx_task_stack[LWIP_ETH_TASK_STACKSIZE];
#endif
#endif

static err_t ethernetif_linkoutput(struct netif *netif, struct pbuf *p)
{
#ifndef LWIP_NO_TX_TASK
    struct eth_tx_msg  msg;
    struct eth_device *enetif = OS_NULL;

    OS_ASSERT(netif != OS_NULL);
    enetif = (struct eth_device *)netif->state;

    /* send a message to eth tx task */
    msg.netif = netif;
    msg.buf   = p;
    if (os_mb_send(&eth_tx_task_mb, (os_ubase_t)&msg, OS_NO_WAIT) == OS_EOK)
    {
        /* waiting for ack */
        os_sem_wait(&(enetif->tx_ack), OS_WAIT_FOREVER);
    }
#else
    struct eth_device *enetif = OS_NULL;

    OS_ASSERT(netif != OS_NULL);
    enetif = (struct eth_device*)netif->state;

    if (enetif->eth_tx(&(enetif->parent), p) != OS_EOK)
    {
        return ERR_IF;
    }
#endif
    return ERR_OK;
}

static err_t eth_netif_device_init(struct netif *netif)
{
    struct eth_device *ethif = OS_NULL;

    ethif = (struct eth_device *)netif->state;
    if (ethif != OS_NULL)
    {
        os_device_t *device = OS_NULL;

        /* get device object */
        device = (os_device_t*)ethif;
        if (os_device_open(device) != OS_EOK)
        {
            return ERR_IF;
        }

        /* copy device flags to netif flags */
        netif->flags = (ethif->flags & 0xff);
        netif->mtu   = ETHERNET_MTU;

        /* set output */
        netif->output = etharp_output;

#if LWIP_IPV6
        netif->output_ip6             = ethip6_output;
        netif->ip6_autoconfig_enabled = 1;
        netif_create_ip6_linklocal_address(netif, 1);

#if LWIP_IPV6_MLD
        netif->flags |= NETIF_FLAG_MLD6;

        /*
         * For hardware/netifs that implement MAC filtering.
         * All-nodes link-local is handled by default, so we must let the hardware know
         * to allow multicast packets in.
         * Should set mld_mac_filter previously. */
        if (netif->mld_mac_filter != NULL)
        {
            ip6_addr_t ip6_allnodes_ll;
            ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
            netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
        }
#endif /* LWIP_IPV6_MLD */

#endif /* LWIP_IPV6 */

        /* set default netif */
        if (netif_default == OS_NULL)
            netif_set_default(ethif->netif);

        /* set interface up */
        netif_set_up(ethif->netif);
#if LWIP_DHCP
        /* if this interface uses DHCP, start the DHCP client v4*/
        dhcp_start(ethif->netif);
#endif

#if LWIP_IPV6_DHCP6
        dhcp6_enable_stateless(ethif->netif);
#endif

        if (ethif->flags & ETHIF_LINK_PHYUP)
        {
            /* set link_up for this netif */
            netif_set_link_up(ethif->netif);
        }

        return ERR_OK;
    }

    return ERR_IF;
}

/* Keep old drivers compatible */
os_err_t eth_device_init_with_flag(struct eth_device *dev, const char *name, os_uint16_t flags)
{
    struct netif *netif = OS_NULL;
#if LWIP_NETIF_HOSTNAME
#define LWIP_HOSTNAME_LEN 16
    char *hostname = OS_NULL;

    netif = (struct netif*)malloc(sizeof(struct netif) + LWIP_HOSTNAME_LEN);
#else
    netif = (struct netif*)malloc(sizeof(struct netif));
#endif
    if (netif == OS_NULL)
    {
        os_kprintf("malloc netif failed\r\n");
        return OS_ERROR;
    }
    memset(netif, 0, sizeof(struct netif));

    /* set netif */
    dev->netif = netif;
    /* device flags, which will be set to netif flags when initializing */
    dev->flags = flags;
    /* link changed status of device */
    dev->link_changed = 0x00;
    dev->parent.type  = OS_DEVICE_TYPE_NETIF;
    /* register to CMCC IOT device manager */
    os_device_register(&(dev->parent), name);
    os_sem_init(&(dev->tx_ack), name, 0, OS_SEM_MAX_VALUE);

    /* set name */
    netif->name[0] = name[0];
    netif->name[1] = name[1];

    /* set hw address to 6 */
    netif->hwaddr_len = 6;
    /* maximum transfer unit */
    netif->mtu = ETHERNET_MTU;

    /* set linkoutput */
    netif->linkoutput = ethernetif_linkoutput;

    /* get hardware MAC address */
    os_device_control(&(dev->parent), NIOCTL_GADDR, netif->hwaddr);

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    hostname = (char *)netif + sizeof(struct netif);
    sprintf(hostname, "OneOS task_%02x%02x", name[0], name[1]);
    netif->hostname = hostname;
#endif /* LWIP_NETIF_HOSTNAME */

    /* if tcp task has been started up, we add this netif to the system */
    if (os_task_find("tcpip") != OS_NULL)
    {
        ip4_addr_t ipaddr;
        ip4_addr_t netmask;
        ip4_addr_t gw;

#if !LWIP_DHCP
        ipaddr.addr  = inet_addr(LWIP_STATIC_IPADDR);
        gw.addr      = inet_addr(LWIP_STATIC_GWADDR);
        netmask.addr = inet_addr(LWIP_STATIC_MSKADDR);
#else
        IP4_ADDR(&ipaddr, 0, 0, 0, 0);
        IP4_ADDR(&gw, 0, 0, 0, 0);
        IP4_ADDR(&netmask, 0, 0, 0, 0);
#endif
        netifapi_netif_add(netif, &ipaddr, &netmask, &gw, dev, eth_netif_device_init, tcpip_input);
    }

    return OS_EOK;
}

os_err_t eth_device_init(struct eth_device *dev, const char *name)
{
    os_uint16_t flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_IGMP
    /* IGMP support */
    flags |= NETIF_FLAG_IGMP;
#endif

    return eth_device_init_with_flag(dev, name, flags);
}

void eth_device_deinit(struct eth_device *dev)
{
    struct netif *netif = dev->netif;

#if LWIP_DHCP
    dhcp_stop(netif);
    dhcp_cleanup(netif);
#endif
    netif_set_down(netif);
    netif_remove(netif);

    os_device_close(&(dev->parent));
    os_device_unregister(&(dev->parent));
    os_sem_deinit(&(dev->tx_ack));
    free(netif);
}

os_err_t eth_device_ready(struct eth_device *dev)
{
#ifndef LWIP_NO_RX_TASK
    if (dev->netif)
    {
        /* post message to Ethernet task */
        return os_mb_send(&eth_rx_task_mb, (os_ubase_t)dev, OS_NO_WAIT);
    }
    else
#endif        
    {
        return ERR_OK; /* netif is not initialized yet, just return. */
    }
}

#ifndef LWIP_NO_RX_TASK
os_err_t eth_device_linkchange(struct eth_device *dev, os_bool_t up)
{
    os_uint32_t level;

    OS_ASSERT(dev != OS_NULL);

    level = os_irq_lock();

    dev->link_changed = 0x01;
    if (up == OS_TRUE)
    {
        dev->link_status = 0x01;
    }
    else
    {
         dev->link_status = 0x00;
    }

    os_irq_unlock(level);

    /* post message to ethernet task */
    return os_mb_send(&eth_rx_task_mb, (os_ubase_t)dev, OS_NO_WAIT);
}
#else
/* NOTE: please not use it in interrupt when no RxThread exist */
os_err_t eth_device_linkchange(struct eth_device *dev, os_bool_t up)
{
    if (up == OS_TRUE)
    {
        netifapi_netif_set_link_up(dev->netif);
    }
    else
    {
        netifapi_netif_set_link_down(dev->netif);
    }

    return OS_EOK;
}
#endif

#ifndef LWIP_NO_TX_TASK
/* Ethernet Tx Thread */
static void eth_tx_task_entry(void *parameter)
{
    struct eth_tx_msg *msg = OS_NULL;

    while (1)
    {
        if (os_mb_recv(&eth_tx_task_mb, (os_ubase_t *)&msg, OS_WAIT_FOREVER) == OS_EOK)
        {
            struct eth_device *enetif = OS_NULL;

            OS_ASSERT(msg->netif != OS_NULL);
            OS_ASSERT(msg->buf != OS_NULL);

            enetif = (struct eth_device *)msg->netif->state;
            if (enetif != OS_NULL)
            {
                /* call driver's interface */
                if (enetif->eth_tx(&(enetif->parent), msg->buf) != OS_EOK)
                {
                    /* transmit eth packet failed */
                }
            }

            /* send ACK */
            os_sem_post(&(enetif->tx_ack));
        }
    }
}
#endif

#ifndef LWIP_NO_RX_TASK
/* Ethernet Rx Thread */
static void eth_rx_task_entry(void *parameter)
{
    struct eth_device *device = OS_NULL;

    while (1)
    {
        if (os_mb_recv(&eth_rx_task_mb, (os_ubase_t *)&device, OS_WAIT_FOREVER) == OS_EOK)
        {
            struct pbuf *p = OS_NULL;

            /* check link status */
            if (device->link_changed)
            {
                int         status;
                os_uint32_t level;

                level                = os_irq_lock();
                status               = device->link_status;
                device->link_changed = 0x00;
                os_irq_unlock(level);

                if (status)
                {
                    netifapi_netif_set_link_up(device->netif);
                }
                else
                {
                    netifapi_netif_set_link_down(device->netif);
                }
            }

            /* receive all of buffer */
            while (1)
            {
                if (device->eth_rx == OS_NULL)
                {
                    break;
                }
                p = device->eth_rx(&(device->parent));
                if (p != OS_NULL)
                {
                    /* notify to upper layer */
                    if (device->netif->input(p, device->netif) != ERR_OK)
                    {
                        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: Input error\r\n"));
                        pbuf_free(p);
                        p = OS_NULL;
                    }
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            LWIP_ASSERT("Should not happen!\r\n", 0);
        }
    }
}
#endif

/* this function does not need,
 * use eth_system_device_init_private()
 * call by lwip_system_init().
 */
int eth_system_device_init(void)
{
    return 0;
}

int eth_system_device_init_private(void)
{
    os_err_t result = OS_EOK;

    /* initialize Rx task. */
#ifndef LWIP_NO_RX_TASK
    /* initialize mailbox and create Ethernet Rx task */
    result = os_mb_init(&eth_rx_task_mb,
                        "erxmb",
                        &eth_rx_task_mb_pool[0],
                        sizeof(eth_rx_task_mb_pool) / 4);
    OS_ASSERT(result == OS_EOK);

    result = os_task_init(&eth_rx_task,
                          "erx",
                          eth_rx_task_entry,
                          OS_NULL,
                          &eth_rx_task_stack[0],
                          sizeof(eth_rx_task_stack),
                          ETHERNETIF_TASK_PREORITY);
    OS_ASSERT(result == OS_EOK);

    result = os_task_startup(&eth_rx_task);
    OS_ASSERT(result == OS_EOK);
#endif

    /* initialize Tx task */
#ifndef LWIP_NO_TX_TASK
    /* initialize mailbox and create Ethernet Tx task */
    result = os_mb_init(&eth_tx_task_mb,
                        "etxmb",
                        &eth_tx_task_mb_pool[0],
                        sizeof(eth_tx_task_mb_pool) / 4);
    OS_ASSERT(result == OS_EOK);

    result = os_task_init(&eth_tx_task,
                          "etx",
                          eth_tx_task_entry,
                          OS_NULL,
                          &eth_tx_task_stack[0],
                          sizeof(eth_tx_task_stack),
                          ETHERNETIF_TASK_PREORITY);
    OS_ASSERT(result == OS_EOK);

    result = os_task_startup(&eth_tx_task);
    OS_ASSERT(result == OS_EOK);
#endif

    return (int)result;
}

void set_if(char *netif_name, char *ip_addr, char *gw_addr, char *nm_addr)
{
    struct netif *netif = netif_list;
    ip4_addr_t   *ip    = OS_NULL;
    ip4_addr_t    addr;

    if (strlen(netif_name) > sizeof(netif->name))
    {
        os_kprintf("network interface name too long!\r\n");
        return;
    }

    while (netif != OS_NULL)
    {
        if (strncmp(netif_name, netif->name, sizeof(netif->name)) == 0)
            break;

        netif = netif->next;
        if (netif == OS_NULL)
        {
            os_kprintf("network interface: %s not found!\r\n", netif_name);
            return;
        }
    }

    ip = (ip4_addr_t *)&addr;

    /* set ip address */
    if ((ip_addr != OS_NULL) && ip4addr_aton(ip_addr, &addr))
    {
        netif_set_ipaddr(netif, ip);
    }

    /* set gateway address */
    if ((gw_addr != OS_NULL) && ip4addr_aton(gw_addr, &addr))
    {
        netif_set_gw(netif, ip);
    }

    /* set netmask address */
    if ((nm_addr != OS_NULL) && ip4addr_aton(nm_addr, &addr))
    {
        netif_set_netmask(netif, ip);
    }
}

#ifdef OS_USING_SHELL

static int set_if_cmd(int argc, char **argv)
{
		if (argc != 5)
		{
				os_kprintf("input error!\r\n"
							 "example:\r\n"
							 "set_if <device> <IP_address> <gatway> <mask>\r\n");
				return (-1);
		}
		
		set_if(argv[1], argv[2], argv[3], argv[4]);
		
		return 0;
}

SH_CMD_EXPORT(set_if, set_if_cmd, "set network interface address");

#if LWIP_DNS
#include <lwip/dns.h>

void set_dns(uint8_t dns_num, char *dns_server)
{
    ip_addr_t addr;

    if ((dns_server != OS_NULL) && ipaddr_aton(dns_server, &addr))
    {
        dns_setserver(dns_num, &addr);
    }
}

SH_CMD_EXPORT(set_dns, set_dns, "set DNS server address");
#endif /* LWIP_DNS */

void lwip_ifconfig(void)
{
    os_ubase_t    index;
    struct netif *netif = OS_NULL;

    os_schedule_lock();

    netif = netif_list;

    while (netif != OS_NULL)
    {
        os_kprintf("network interface: %c%c%s\r\n",
                   netif->name[0],
                   netif->name[1],
                   (netif == netif_default) ? " (Default)" : "");
        os_kprintf("MTU: %d\r\n", netif->mtu);
        os_kprintf("MAC: ");
        for (index = 0; index < netif->hwaddr_len; index++)
            os_kprintf("%02x ", netif->hwaddr[index]);
        os_kprintf("\r\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP)
            os_kprintf(" UP");
        else
            os_kprintf(" DOWN");
        if (netif->flags & NETIF_FLAG_LINK_UP)
            os_kprintf(" LINK_UP");
        else
            os_kprintf(" LINK_DOWN");
        if (netif->flags & NETIF_FLAG_ETHARP)
            os_kprintf(" ETHARP");
        if (netif->flags & NETIF_FLAG_BROADCAST)
            os_kprintf(" BROADCAST");
        if (netif->flags & NETIF_FLAG_IGMP)
            os_kprintf(" IGMP");
        os_kprintf("\r\n");
        os_kprintf("ip address: %s\r\n", ipaddr_ntoa(&(netif->ip_addr)));
        os_kprintf("gw address: %s\r\n", ipaddr_ntoa(&(netif->gw)));
        os_kprintf("net mask  : %s\r\n", ipaddr_ntoa(&(netif->netmask)));
#if LWIP_IPV6
        {
            ip6_addr_t *addr = OS_NULL;
            int         addr_state;
            int         i;

            addr       = (ip6_addr_t*)&netif->ip6_addr[0];
            addr_state = netif->ip6_addr_state[0];

            os_kprintf("\r\nipv6 link-local: %s state:%02X %s\r\n",
                       ip6addr_ntoa(addr),
                       addr_state,
                       ip6_addr_isvalid(addr_state) ? "VALID" : "INVALID");

            for (i = 1; i < LWIP_IPV6_NUM_ADDRESSES; i++)
            {
                addr       = (ip6_addr_t*)&netif->ip6_addr[i];
                addr_state = netif->ip6_addr_state[i];

                os_kprintf("ipv6[%d] address: %s state:%02X %s\r\n",
                           i,
                           ip6addr_ntoa(addr),
                           addr_state,
                           ip6_addr_isvalid(addr_state) ? "VALID" : "INVALID");
            }
        }
        os_kprintf("\r\n");
#endif /* LWIP_IPV6 */
        netif = netif->next;
    }

#if LWIP_DNS
    {
        const ip_addr_t *ip_addr;

        for (index = 0; index < DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            os_kprintf("dns server #%d: %s\r\n", index, ipaddr_ntoa(ip_addr));
        }
    }
#endif /* LWIP_DNS */

    os_schedule_unlock();
}

SH_CMD_EXPORT(lwip_ifconfig, lwip_ifconfig, "list network interface information");

#if LWIP_TCP
#include <lwip/priv/tcp_priv.h>
#include <lwip/tcp.h>

void list_tcps(void)
{
    os_uint32_t     num               = 0;
    struct tcp_pcb* pcb               = OS_NULL;
    char            local_ip_str[16]  = {0};
    char            remote_ip_str[16] = {0};

    extern struct tcp_pcb         *tcp_active_pcbs;
    extern struct tcp_pcb         *tcp_tw_pcbs;
    extern union tcp_listen_pcbs_t tcp_listen_pcbs;

    os_kprintf("Active PCB states:\r\n");
    os_schedule_lock();
    for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        os_kprintf("#%d %s:%d <==> %s:%d snd_nxt 0x%08X rcv_nxt 0x%08X ",
                   num++,
                   local_ip_str,
                   pcb->local_port,
                   remote_ip_str,
                   pcb->remote_port,
                   pcb->snd_nxt,
                   pcb->rcv_nxt);
        os_kprintf("state: %s\r\n", tcp_debug_state_str(pcb->state));
    }

    os_kprintf("Listen PCB states:\r\n");
    num = 0;
    for (pcb = (struct tcp_pcb*)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next)
    {
        os_kprintf("#%d local port %d ", num++, pcb->local_port);
        os_kprintf("state: %s\r\n", tcp_debug_state_str(pcb->state));
    }

    os_kprintf("TIME-WAIT PCB states:\r\n");
    num = 0;
    for (pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        os_kprintf("#%d %s:%d <==> %s:%d snd_nxt 0x%08X rcv_nxt 0x%08X ",
                   num++,
                   local_ip_str,
                   pcb->local_port,
                   remote_ip_str,
                   pcb->remote_port,
                   pcb->snd_nxt,
                   pcb->rcv_nxt);
        os_kprintf("state: %s\r\n", tcp_debug_state_str(pcb->state));
    }
    os_schedule_unlock();
}

SH_CMD_EXPORT(list_tcps, list_tcps, "list all of tcp connections");
#endif /* LWIP_TCP */

#if LWIP_UDP
#include "lwip/udp.h"

void list_udps(void)
{
    struct udp_pcb* pcb = OS_NULL;
    os_uint32_t     num = 0;

    char local_ip_str[16]  = {0};
    char remote_ip_str[16] = {0};

    os_kprintf("Active UDP PCB states:\r\n");
    os_schedule_lock();
    for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        os_kprintf("#%d %d %s:%d <==> %s:%d \r\n",
                   num,
                   (int)pcb->flags,
                   local_ip_str,
                   pcb->local_port,
                   remote_ip_str,
                   pcb->remote_port);

        num++;
    }
    os_schedule_unlock();
}

SH_CMD_EXPORT(list_udps, list_udps, "list all of udp connections");
#endif /* LWIP_UDP */

#endif /* OS_USING_SHELL */
