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
 * @file        wlan_lwip.c
 *
 * @brief       This file implements wlan_lwip driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <os_types.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_stddef.h>
#include <dlog.h>

#include <device.h>

#include "wlan_dev.h"
#include "wlan_lwip.h"
#include "wwd_network_interface.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wlan_lwip"
#include <drv_log.h>

#define OS_WLAN_WORK_TASK_ENABLE

static void timer_callback(void *parameter)
{
    ip_addr_t              ip_addr_zero = {0};

    struct os_wlan_lwip *wlan_lwip = (struct os_wlan_lwip *)parameter;
    
    if (ip_addr_cmp(&(wlan_lwip->eth_dev.netif->ip_addr), &ip_addr_zero) != 0)
    {
        os_timer_start(&wlan_lwip->timer);
    }
}

static void netif_set_connected(struct os_wlan_lwip *wlan_lwip)
{
    if (wlan_lwip->connet_status)
    {
        if (wlan_lwip->wlan_dev->mode == OS_WLAN_STATION)
        {
            netifapi_netif_common(wlan_lwip->eth_dev.netif, netif_set_link_up, NULL);
#ifdef LWIP_USING_DHCP
            dhcp_start(wlan_lwip->eth_dev.netif);
#endif
            os_timer_start(&wlan_lwip->timer);
        }
        else if (wlan_lwip->wlan_dev->mode == OS_WLAN_AP)
        {
            netifapi_netif_common(wlan_lwip->eth_dev.netif, netif_set_link_up, NULL);
#ifdef LWIP_USING_DHCPD
            {
                char netif_name[OS_NAME_MAX];

                memset(netif_name, 0, sizeof(netif_name));
                memcpy(netif_name, wlan_lwip->eth_dev.netif->name, sizeof(wlan_lwip->eth_dev.netif->name));
                dhcpd_start(netif_name);
            }
#endif
        }
    }
    else
    {
        netifapi_netif_common(wlan_lwip->eth_dev.netif, netif_set_link_down, NULL);
        os_timer_stop(&wlan_lwip->timer);
#ifdef LWIP_USING_DHCP
        {
            dhcp_stop(wlan_lwip->eth_dev.netif);
#if LWIP_IPV4
            ip4_addr_t ip4_addr = {0};
            netif_set_addr(wlan_lwip->eth_dev.netif, &ip4_addr, &ip4_addr, &ip4_addr);
#endif
            
#if LWIP_IPV6
            ip6_addr_t ip6_addr = {0};
            netif_ip6_addr_set(wlan_lwip->eth_dev.netif, 1, &ip6_addr);
#endif
        }
#endif
#ifdef LWIP_USING_DHCPD
        {
            char netif_name[OS_NAME_MAX];
            
            memset(netif_name, 0, sizeof(netif_name));
            memcpy(netif_name, wlan_lwip->eth_dev.netif->name, sizeof(wlan_lwip->eth_dev.netif->name));
//            dhcpd_stop(netif_name);
        }
#endif
    }
}

static os_err_t os_wlan_lwip_event_handle(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol, os_ubase_t event, os_ubase_t args)
{
    os_bool_t old_status;
    
    struct os_wlan_lwip *wlan_lwip = os_container_of(protocol, struct os_wlan_lwip, protocol);

    old_status = wlan_lwip->connet_status;

    switch (event)
    {
    case ION_WLAN_JOIN:
    case ION_WLAN_AP_START:
        wlan_lwip->connet_status = OS_TRUE;
        break;
    case ION_WLAN_LEAVE:
    case ION_WLAN_AP_STOP:
        wlan_lwip->connet_status = OS_FALSE;
        break;
    case ION_WLAN_AP_ASSOCIATED:
    case ION_WLAN_AP_DISASSOCIATED:
        break;
    default:
        LOG_D(DRV_EXT_TAG, "event: UNKNOWN");
        break;
    }
    
    if (old_status != wlan_lwip->connet_status)
    {
        netif_set_connected(wlan_lwip);
    }
    return OS_EOK;
}

static os_err_t os_wlan_lwip_protocol_report(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol, void *buff)
{
    uint16_t        ethertype;

    struct pbuf *p                  = (struct pbuf *)buff;
    struct eth_hdr *header          = OS_NULL;
    struct os_wlan_lwip *wlan_lwip  = os_container_of(protocol, struct os_wlan_lwip, protocol);

    header = (struct eth_hdr *)p->payload;
    ethertype = htons(header->type);

    if (ethertype == ETHTYPE_VLAN)
    {
        uint8_t temp_buffer[12];
        
        memcpy(temp_buffer, p->payload, 12);
        memcpy(((uint8_t*) p->payload ) + 4, temp_buffer, 12);

        p->payload = ((uint8_t*) p->payload) + 4;
        p->len = (u16_t) (p->len - 4);

        header = (struct eth_hdr *) p->payload;
        ethertype = htons(header->type);
    }
    
    switch(ethertype)
    {
    case ETHTYPE_IP:
    case ETHTYPE_ARP:
    case ETHTYPE_IPV6:
        if ((wlan_lwip->eth_dev.netif->input(p, wlan_lwip->eth_dev.netif)) != ERR_OK)
        {
            pbuf_free(p);
            return OS_ERROR;
        }
        break;
    default:
        break;
    }
    
    return OS_EOK;
}

os_err_t os_wlan_lwip_send(os_device_t *device, struct pbuf *p)
{
    struct os_wlan_lwip *wlan_lwip  = os_container_of(device, struct os_wlan_lwip, eth_dev);
    
    return os_wlan_protocol_send(wlan_lwip->wlan_dev, (char *)p);
}

static os_err_t os_wlan_lwip_control(os_device_t *device, int cmd, void *args)
{
    os_err_t err = OS_EOK;
    
    struct os_wlan_lwip *wlan_lwip  = os_container_of(device, struct os_wlan_lwip, eth_dev);

    switch (cmd)
    {
    case NIOCTL_GADDR:
        err  = os_wlan_get_mac(&wlan_lwip->wlan_dev->parent, (os_uint8_t *)args);
        break;
    default:
        break;
    }
    return err;
}

const static struct os_device_ops lwip_ops =
{
    .control = os_wlan_lwip_control
};

os_err_t os_wlan_lwip_init(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol)
{
    struct os_wlan_lwip *wlan_lwip = os_container_of(protocol, struct os_wlan_lwip, protocol);
    
    wlan_lwip->eth_dev.parent.ops         = &lwip_ops;
    wlan_lwip->eth_dev.eth_rx             = OS_NULL;
    wlan_lwip->eth_dev.eth_tx             = os_wlan_lwip_send;

    if (eth_device_init(&wlan_lwip->eth_dev, wlan_lwip->name) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "eth device init failed!");
        return OS_ERROR;
    }

    netif_set_up(wlan_lwip->eth_dev.netif);
    LOG_I(DRV_EXT_TAG, "eth device init ok name:%s", wlan_lwip->name);
    
    return OS_EOK;
}

os_err_t os_wlan_lwip_register(os_device_t * device)
{
    os_uint8_t              id  = 0;
    char                    eth_name[4];
    char                    timer_name[16] = "tim_";
    struct os_wlan_lwip    *wlan_lwip   = OS_NULL;
    
    wlan_lwip = os_calloc(1, sizeof(struct os_wlan_lwip));
    
    OS_ASSERT_EX(wlan_lwip, "wlan_lwip os_calloc failed!");
    
    wlan_lwip->wlan_dev = (struct os_wlan_device *)device;
    
    do
    {
        /* find ETH device name */
        eth_name[0] = 'w';
        eth_name[1] = '0' + id++;
        eth_name[2] = '\0';
        device      = os_device_find(eth_name);
    } while (device);

    if (id > 9)
    {
        LOG_E(DRV_EXT_TAG, "etn not find empty name");
        return OS_ERROR;
    }
    
    memcpy(&timer_name[4], eth_name, sizeof(eth_name));

    os_timer_init(&wlan_lwip->timer,
                  timer_name,
                  timer_callback,
                  wlan_lwip,
                  os_tick_from_ms(1000),
                  OS_TIMER_FLAG_ONE_SHOT);
    
    memcpy(wlan_lwip->name, eth_name, sizeof(eth_name));

    wlan_lwip->connet_status            = OS_FALSE;
    wlan_lwip->protocol.init            = os_wlan_lwip_init;
    wlan_lwip->protocol.report          = os_wlan_lwip_protocol_report;
    wlan_lwip->protocol.event_handler   = os_wlan_lwip_event_handle;

    return os_wlan_protocol_register(wlan_lwip->wlan_dev, &wlan_lwip->protocol);
}

