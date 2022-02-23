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
 * @file        wlan_dev.c
 *
 * @brief       This file implements wlan driver.
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
#include <dlog.h>
#include <string.h>
#include <device.h>
#include <sdio.h>

#include "wlan_dev.h"
#include "wlan_lwip.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wlan_dev"
#include <drv_log.h>

os_err_t os_wlan_ap_start(os_device_t *device, const char *ssid, const char *password, os_wlan_country_code_t country, os_wlan_security_t security, os_uint8_t channel)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }

    if (wlan_dev->flags != OS_WLAN_FLAG_AP_ONLY)
    {
        LOG_E(DRV_EXT_TAG, "%s is sta only, not support ap", device->name);
        return OS_ERROR;
    }

    wlan_dev->info.ssid         = ssid;
    wlan_dev->info.password     = password;
    wlan_dev->info.security     = security;
    wlan_dev->info.channel      = channel;
    wlan_dev->info.country      = country;
    
    if (wlan_dev->ops->ap_start)
    {
        return wlan_dev->ops->ap_start(wlan_dev);
    }
    
    return OS_EOK;
}

os_err_t os_wlan_ap_stop(os_device_t *device)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }

    if (wlan_dev->flags != OS_WLAN_FLAG_AP_ONLY)
    {
        LOG_E(DRV_EXT_TAG, "%s is sta only, not support ap", device->name);
        return OS_ERROR;
    }

    if (wlan_dev->ops->ap_stop)
        return wlan_dev->ops->ap_stop(wlan_dev);
    
    return OS_EOK;
}

os_err_t os_wlan_sta_start(os_device_t *device, os_wlan_country_code_t country, os_wlan_security_t security)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }
    
    if (wlan_dev->flags != OS_WLAN_FLAG_STA_ONLY)
    {
        LOG_E(DRV_EXT_TAG, "%s is sta only, not support ap", device->name);
        return OS_ERROR;
    }

    wlan_dev->info.security     = security;
    wlan_dev->info.country      = country;

    if (wlan_dev->ops->sta_start)
    {
        return wlan_dev->ops->sta_start(wlan_dev);
    }
    
    return OS_EOK;
}

os_err_t os_wlan_sta_stop(os_device_t *device)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }

    if (wlan_dev->flags != OS_WLAN_FLAG_STA_ONLY)
    {
        LOG_E(DRV_EXT_TAG, "%s is sta only, not support ap", device->name);
        return OS_ERROR;
    }

    if (wlan_dev->ops->sta_stop)
    {
        return wlan_dev->ops->sta_stop(wlan_dev);
    }
    
    return OS_EOK;
}

void os_wlan_irq_handler(void *device)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return;
    }

    if (wlan_dev->ops->irq_handler)
    {
        wlan_dev->ops->irq_handler(wlan_dev);
    }
}

os_err_t os_wlan_get_mac(os_device_t *device, os_uint8_t *mac)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }

    if (wlan_dev->ops->get_mac)
    {
        return wlan_dev->ops->get_mac(wlan_dev, mac);
    }
    
    return OS_EOK;
}

os_err_t os_wlan_join(os_device_t *device, const char *ssid, const char *password)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }

    wlan_dev->info.ssid = ssid;
    wlan_dev->info.password = password;
    
    if (wlan_dev->ops->join)
    {
        if (wlan_dev->ops->join(wlan_dev) != OS_EOK)
        {
            LOG_E(DRV_EXT_TAG, "os_wlan_join failed!");
            return OS_ERROR;
        }
    }
    
    return OS_EOK;
}

os_err_t os_wlan_check_join_status(os_device_t *device)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }

    if (wlan_dev->ops->check_join_status)
    {
        return wlan_dev->ops->check_join_status(wlan_dev);
    }
    
    return OS_EOK;
}

os_err_t os_wlan_leave(os_device_t *device)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_EIO;
    }

    if (wlan_dev->ops->leave)
    {
        if (wlan_dev->ops->leave(wlan_dev) != OS_EOK)
        {
            LOG_E(DRV_EXT_TAG, "os_wlan_leave failed!");
            return OS_ERROR;
        }
    }
    
    return OS_EOK;
}

struct os_wlan_scan_result *os_wlan_scan(os_device_t *device, os_uint32_t msec)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
       return OS_NULL;
    }

   return wlan_dev->ops->wlan_scan(wlan_dev, msec);
}

os_err_t os_wlan_scan_clean_result(os_device_t *device, struct os_wlan_scan_result *info)
{
     struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;
    
     if (device == OS_NULL)
     {
        return OS_EIO;
     }
    
    return wlan_dev->ops->wlan_scan_clean_result(wlan_dev, info);
}

os_err_t os_wlan_scan_stop(os_device_t *device)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)device;

    if (device == OS_NULL)
    {
        return OS_EIO;
    }

    return wlan_dev->ops->wlan_scan_stop(wlan_dev);
}

os_err_t os_wlan_init_callback(struct os_wlan_device *wlan_dev)
{
    os_uint8_t i = 0;
    
    for(i = 0;i < OS_WLAN_PROTOCOL_NUM_MAX;i++)
    {
        if ((wlan_dev->protocol[i] != OS_NULL) && (wlan_dev->protocol[i]->init != OS_NULL))
        {
            wlan_dev->protocol[i]->init(wlan_dev, wlan_dev->protocol[i]);
        }
    }
    
    return OS_EOK;
}

static os_err_t os_wlan_event_callback(os_device_t *dev, os_ubase_t event, os_ubase_t args)
{
    os_uint8_t i = 0;
    
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)dev;
    
    for(i = 0;i < OS_WLAN_PROTOCOL_NUM_MAX;i++)
    {
        if ((wlan_dev->protocol[i] != OS_NULL) && (wlan_dev->protocol[i]->event_handler != OS_NULL))
        {
            wlan_dev->protocol[i]->event_handler(wlan_dev, wlan_dev->protocol[i], event, args);
        }
    }
    
    return OS_EOK;
}

os_err_t os_wlan_event_register(struct os_wlan_device *wlan_dev)
{
    return os_device_notify_register(&wlan_dev->parent, os_wlan_event_callback, OS_NULL);
}

os_err_t os_wlan_protocol_send(struct os_wlan_device *wlan_dev, char *buff)
{
    return wlan_dev->ops->protocol_send(wlan_dev, buff);
}

os_err_t os_wlan_protocol_report(struct os_wlan_device *wlan_dev, char *buff)
{
    os_uint8_t i = 0;
    
    for(i = 0;i < OS_WLAN_PROTOCOL_NUM_MAX;i++)
    {
        if ((wlan_dev->protocol[i] != OS_NULL) && (wlan_dev->protocol[i]->report != OS_NULL))
        {
            wlan_dev->protocol[i]->report(wlan_dev, wlan_dev->protocol[i], buff);
        }
    }
    
    return OS_EOK;
}

os_err_t os_wlan_register(struct os_wlan_device *wlan_dev, const char *name)
{
    if (os_device_register(&wlan_dev->parent, name) != OS_EOK)
    {
        return OS_ERROR;
    }

    os_wlan_event_register(wlan_dev);

    return OS_EOK;
}

os_err_t os_wlan_protocol_register(struct os_wlan_device *wlan_dev, struct os_wlan_protocol *protocol)
{
    os_uint8_t i = 0;
    
    for(i = 0;i < OS_WLAN_PROTOCOL_NUM_MAX;i++)
    {
        if (wlan_dev->protocol[i] == OS_NULL)
        {
            wlan_dev->protocol[i] = protocol;
            return OS_EOK;
        }
    }
    
    if (i == OS_WLAN_PROTOCOL_NUM_MAX)
        return OS_EFULL;
    
    return OS_EOK;
}





