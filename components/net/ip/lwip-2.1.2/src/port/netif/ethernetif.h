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
#ifndef __NETIF_ETHERNETIF_H__
#define __NETIF_ETHERNETIF_H__

#include "lwip/netif.h"

#include <device.h>
#include <os_sem.h>
#include <os_errno.h>
#include <os_clock.h>

#define NIOCTL_GADDR 0x01
#ifndef OS_LWIP_ETH_MTU
#define ETHERNET_MTU 1500
#else
#define ETHERNET_MTU OS_LWIP_ETH_MTU
#endif

/* eth flag with auto_linkup or phy_linkup */
#define ETHIF_LINK_AUTOUP 0x0000
#define ETHIF_LINK_PHYUP  0x0100

struct eth_device
{
    /* inherit from os_device */
    struct os_device parent;

    /* network interface for lwip */
    struct netif       *netif;
    struct os_semaphore tx_ack;

    os_uint16_t flags;
    os_uint8_t  link_changed;
    os_uint8_t  link_status;

    /* eth device interface */
    struct pbuf *(*eth_rx)(os_device_t *dev);
    os_err_t (*eth_tx)(os_device_t *dev, struct pbuf *p);
};

#ifdef __cplusplus
extern "C" {
#endif

os_err_t eth_device_ready(struct eth_device *dev);
os_err_t eth_device_init(struct eth_device *dev, const char *name);
os_err_t eth_device_init_with_flag(struct eth_device *dev, const char *name, os_uint16_t flag);
os_err_t eth_device_linkchange(struct eth_device *dev, os_bool_t up);
void     eth_device_deinit(struct eth_device *dev);

int eth_system_device_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __NETIF_ETHERNETIF_H__ */
