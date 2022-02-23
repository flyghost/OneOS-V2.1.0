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
 * @file        wlan_lwip.h
 *
 * @brief       This file implements wlan_lwip interface.
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __WLAN_LWIP_H__
#define __WLAN_LWIP_H__

#include <os_timer.h>
#include <os_workqueue.h>
#include <os_timer.h>
#include "wlan_dev.h"

#include <netif/ethernetif.h>
#include <lwip/netifapi.h>
#ifdef LWIP_USING_DHCPD
#include <apps/dhcpd/dhcp_server.h>
#endif

#define OS_WLAN_LWIP_NAME_MAX    10

struct os_wlan_lwip
{
    struct os_wlan_protocol protocol;
    struct eth_device       eth_dev;
    struct os_wlan_device  *wlan_dev;
    char                    name[OS_WLAN_LWIP_NAME_MAX];
    os_bool_t               connet_status;
    os_timer_t              timer;
};

os_err_t os_wlan_lwip_register(struct os_device *device);

#endif

