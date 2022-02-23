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
 * @file        wwd_dev.c
 *
 * @brief       This file implements wwd driver.
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <os_types.h>
#include <os_clock.h>
#include <os_errno.h>
#include <os_assert.h>
#include <dlog.h>

#include <device.h>
#include <sdio.h>
#include <wwd_dev.h>
#include <wlan_dev.h>

#include "platform/wwd_platform_interface.h"
#include "platform/wwd_bus_interface.h"
#include "wwd_management.h"
#include "network/wwd_buffer_interface.h"
#include "network/wwd_network_interface.h"
#include "network/wwd_network_constants.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wwd_wifi.h"
#include "wwd_poll.h"
#include "wwd_events.h"
#include <string.h>
#include "NoOS_canned_send.h"
#include "wwd_debug.h"
#include "wiced_utilities.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wwd_dev"
#include <drv_log.h>

struct os_wwd_device *wwd_dev = OS_NULL;

const wwd_event_num_t event_nums[] =
{
    WLC_E_AP_STARTED,
    WLC_E_LINK,
    WLC_E_NONE                      /***the last member must be WLC_E_NONE***/
};

void *os_wwd_event_handler(const wwd_event_header_t *event_header, const uint8_t *event_data, void *handler_user_data)
{
    os_ubase_t event = 0;
    
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)handler_user_data;

    switch(event_header->event_type)
    {
    case WLC_E_AP_STARTED:
        event = ION_WLAN_AP_START;
    break;
    case WLC_E_LINK:
        event = ION_WLAN_JOIN;
    break;
    default:
    break;
    }
    
    os_device_notify(&wlan_dev->parent, event, (os_ubase_t)wlan_dev);

    return handler_user_data;
}

os_err_t os_wwd_register_event_handler(struct os_wlan_device *wlan_dev)
{
    wwd_interface_t interface;
    
    if (wlan_dev->mode == OS_WLAN_STATION)
        interface = WWD_STA_INTERFACE;
    else
        interface = WWD_AP_INTERFACE;
    
    wwd_management_set_event_handler(event_nums, os_wwd_event_handler, wlan_dev, interface);
    
    return OS_EOK;
}

void os_wwd_init_task(void *parameter)
{
    struct os_wlan_device *wlan_dev = (struct os_wlan_device *)parameter;
    
    os_wwd_bus_init();
    
    os_wwd_wait_bus();
    
    NoOS_setup_timing();

    wwd_buffer_init(OS_NULL);

    if(wwd_management_wifi_on((wiced_country_code_t)wlan_dev->info.country) != WWD_SUCCESS)
    {
        LOG_E(DRV_EXT_TAG, "Error when starting WICED!");
    }
    else
    {
        os_wwd_register_event_handler(wlan_dev);
        
        os_wlan_init_callback((struct os_wlan_device *)parameter);
        
        if (wlan_dev->mode == OS_WLAN_AP)
        {
            wiced_ssid_t ap_ssid = {0};
            os_uint8_t ssid_len     = strlen(wlan_dev->info.ssid);
            os_uint8_t password_len = strlen(wlan_dev->info.password);
            
            memcpy(&ap_ssid.value, wlan_dev->info.ssid, ssid_len);
            ap_ssid.length = ssid_len;
                
            if (wwd_wifi_start_ap(&ap_ssid, (wiced_security_t)wlan_dev->info.security, (uint8_t*)wlan_dev->info.password, password_len, wlan_dev->info.channel) != WWD_SUCCESS)
            {
                LOG_E(DRV_EXT_TAG, "wwd_wifi_start_ap fialed!");
            }
        }
        
        wwd_dev->status = WWD_STATE_RUN;
    }
}

void os_wwd_init(struct os_wlan_device *wlan_dev)
{
    os_task_t *task;
    
    task = os_task_create("wwd_init", os_wwd_init_task, wlan_dev, OS_WLAN_INIT_TASK_STACK_SIZE, OS_WLAN_INIT_TASK_PRIORITY);
    OS_ASSERT(task);
    os_task_startup(task);
}

os_err_t os_wwd_ap_start(struct os_wlan_device *wlan_dev)
{
    os_wwd_init(wlan_dev);
    
    return OS_EOK;
}

os_err_t os_wwd_ap_stop(struct os_wlan_device *wlan_dev)
{
    if(wwd_wifi_stop_ap() != WWD_SUCCESS)
    {
        return OS_ERROR;
    }
    
    return OS_EOK;
}

os_err_t os_wwd_sta_start(struct os_wlan_device *wlan_dev)
{
    os_wwd_init(wlan_dev);
    
    return OS_EOK;
}

os_err_t os_wwd_sta_stop(struct os_wlan_device *wlan_dev)
{
    return OS_EOK;
}

void os_wwd_irq_handler(void *param)
{
    wwd_thread_notify_irq();
}

os_err_t os_wwd_protocol_send(struct os_wlan_device *wlan_dev, char *buff)
{
    wwd_interface_t interface;
    
    if (wlan_dev->mode == OS_WLAN_STATION)
        interface = WWD_STA_INTERFACE;
    else
        interface = WWD_AP_INTERFACE;
    
    host_network_send_ethernet_data(buff, interface);
    
    return OS_EOK;
}

void host_network_process_ethernet_data(wiced_buffer_t buffer, wwd_interface_t interface)
{
    if (interface == WWD_STA_INTERFACE)
    {
        os_wlan_protocol_report(&wwd_dev->wlan_sta_dev, (char *)buffer);
    }
    else if (interface == WWD_AP_INTERFACE)
    {
        os_wlan_protocol_report(&wwd_dev->wlan_ap_dev, (char *)buffer);
    }
}

static os_err_t os_wwd_join(struct os_wlan_device *wlan_dev)
{
    wiced_ssid_t ap_ssid = {0};
    os_uint8_t ssid_len     = strlen(wlan_dev->info.ssid);
    os_uint8_t password_len = strlen(wlan_dev->info.password);

    if (wlan_dev->mode != OS_WLAN_STATION)
    {
        LOG_E(DRV_EXT_TAG, "%s is ap only, not support join", wlan_dev->parent.name);
        return OS_ERROR;
    }

    if (ssid_len >= SSID_NAME_SIZE)
    {
        LOG_E(DRV_EXT_TAG, "ssid_len is more than SSID_NAME_SIZE!");
        return OS_ERROR;
    }
    
    if (wwd_dev->status != WWD_STATE_RUN)
    {
        return OS_ERROR;
    }
    
    memcpy(&ap_ssid.value, wlan_dev->info.ssid, ssid_len);
    ap_ssid.length = ssid_len;
    
    if (wwd_wifi_join(&ap_ssid, (wiced_security_t)wlan_dev->info.security, (uint8_t*)wlan_dev->info.password, password_len, OS_NULL) != WWD_SUCCESS)
    {
       return OS_ERROR;
    }
    
    return OS_EOK;
}

static os_err_t os_wwd_leave(struct os_wlan_device *wlan_dev)
{
    wwd_interface_t interface;
    
    if (wlan_dev->mode == OS_WLAN_STATION)
    {
        interface = WWD_STA_INTERFACE;
    }
    else
    {
        return OS_ERROR;
    }
    
    if (wwd_wifi_leave(interface) != WWD_SUCCESS)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t os_wwd_get_mac(struct os_wlan_device *wlan_dev, os_uint8_t *mac)
{
    wwd_interface_t interface;
    
    if (wlan_dev->mode == OS_WLAN_STATION)
        interface = WWD_STA_INTERFACE;
    else
        interface = WWD_AP_INTERFACE;
    
    if (wwd_wifi_get_mac_address((wiced_mac_t *)mac, interface) != WWD_SUCCESS)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static void scan_results_handler(wiced_scan_result_t** result_ptr, void* user_data, wiced_scan_status_t status)
{
    int i = 0;
    wiced_scan_result_t *result = (*result_ptr);
    struct os_wlan_scan_info   *info = OS_NULL;
    struct os_wwd_scan_process_info *process_info = user_data;
    
    if (result_ptr == OS_NULL)
    {
        os_sem_post(&process_info->sem);
        return;
    }
    
    while (NULL_MAC(process_info->scan_result->scan_info[i].bssid) == WICED_FALSE)
    {
      if (CMP_MAC(result->BSSID.octet, process_info->scan_result->scan_info[i].bssid))
      {
          return;
      }
      i++;
    }
    
    process_info->scan_result->num++;

    info = os_realloc(process_info->scan_result->scan_info, process_info->scan_result->num * sizeof(struct os_wlan_scan_info));
    
    if (info == OS_NULL)
    {
        process_info->scan_result->num--;
        LOG_E(DRV_EXT_TAG, "no enough memery to store scan info!");
        os_sem_post(&process_info->sem);
        return;
    }

    process_info->scan_result->scan_info = info;
    
    info = &process_info->scan_result->scan_info[process_info->scan_result->num-1];

    memcpy(&info->bssid[0], result->BSSID.octet, 6);
    strncpy((char *)&info->ssid.val[0], (const char *)result->SSID.value, OS_WLAN_SSID_MAX_LENGTH);

    info->ssid.len          = result->SSID.length;
    info->band              = (os_wlan_802_11_band_t)result->band;
    info->channel           = result->channel;
    info->max_data_rate     = result->max_data_rate;
    info->security          = (os_wlan_security_t)result->security;
    info->signal_strength   = result->signal_strength;
}


static struct os_wlan_scan_result *os_wwd_scan(struct os_wlan_device *wlan, os_uint32_t msec)
{
    wiced_scan_result_t             *result_ptr = OS_NULL;
    struct os_wlan_scan_result      *scan_result = OS_NULL;
    struct os_wwd_scan_process_info *process_info = OS_NULL;

    result_ptr = os_calloc(1, sizeof(wiced_scan_result_t));
    if (result_ptr == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_calloc wiced_scan_result_t failed!");
        return OS_NULL;
    }

    scan_result = os_calloc(1, sizeof(struct os_wlan_scan_result));
    if(scan_result == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_calloc os_wlan_scan_result failed!");
        os_free(result_ptr);
        return OS_NULL;
    }
    
    process_info = os_calloc(1, sizeof(struct os_wwd_scan_process_info));
    if(process_info == OS_NULL)
    {
        os_free(scan_result);
        os_free(result_ptr);
        LOG_E(DRV_EXT_TAG, "os_calloc os_wwd_scan_process_info failed!");
        return OS_NULL;
    }

    process_info->scan_result = scan_result;
    
    if (os_sem_init(&process_info->sem, "scan", 0, 1) != OS_EOK)
    {
        goto end;
    }
    
    if (OS_EOK != wwd_wifi_scan(WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, NULL, NULL, NULL, NULL, scan_results_handler, (wiced_scan_result_t **) &result_ptr, process_info, WWD_STA_INTERFACE ) )
    {
        LOG_E(DRV_EXT_TAG, "Error starting scan");
        goto end;
    }
    
    if (os_sem_wait(&process_info->sem, os_tick_from_ms(msec)) != OS_EOK)
    {
        wwd_wifi_abort_scan();
    }

    os_sem_deinit(&process_info->sem);
    
    os_free(result_ptr);
    os_free(process_info);
        
    return scan_result;
end:
    os_sem_deinit(&process_info->sem);
    
    os_free(scan_result);
    os_free(result_ptr);
    os_free(process_info);
    return OS_NULL;
}

static os_err_t os_wwd_scan_stop(struct os_wlan_device *wlan)
{
    wwd_wifi_abort_scan();

    return OS_EOK;
}

static os_err_t os_wwd_scan_clean_result(struct os_wlan_device *wlan, struct os_wlan_scan_result *info)
{
    os_free(info->scan_info);
    os_free(info);

    return OS_EOK;
}

static const struct os_wlan_device_ops wlan_ops = 
{
    .ap_start               = os_wwd_ap_start,
    .ap_stop                = os_wwd_ap_stop,
    .sta_start              = os_wwd_sta_start,
    .sta_stop               = os_wwd_sta_stop,
    .get_mac                = os_wwd_get_mac,
    .join                   = os_wwd_join,
    .check_join_status      = OS_NULL,
    .leave                  = os_wwd_leave,
    .irq_handler            = os_wwd_irq_handler,
    .protocol_send          = os_wwd_protocol_send,
    .wlan_scan              = os_wwd_scan,
    .wlan_scan_stop         = os_wwd_scan_stop,
    .wlan_scan_clean_result = os_wwd_scan_clean_result
};

int os_wlan_wwd_probe(void)
{
    wwd_dev = os_calloc(1, sizeof(struct os_wwd_device));
    OS_ASSERT_EX(wwd_dev, "wwd_dev os_calloc failed!");

    wwd_dev->status = WWD_STATE_UNUSED;
    
    wwd_dev->wlan_sta_dev.ops            = &wlan_ops;
    wwd_dev->wlan_sta_dev.flags          = OS_WLAN_FLAG_STA_ONLY;
    wwd_dev->wlan_sta_dev.mode           = OS_WLAN_STATION;
    wwd_dev->wlan_sta_dev.parent.type    = OS_DEVICE_TYPE_MISCELLANEOUS;
    
    if (os_wlan_register(&wwd_dev->wlan_sta_dev, OS_WLAN_DEVICE_STA_NAME) != OS_EOK)
    {
        return OS_ERROR;
    }
    
    wwd_dev->wlan_ap_dev.ops             = &wlan_ops;
    wwd_dev->wlan_ap_dev.flags           = OS_WLAN_FLAG_AP_ONLY;
    wwd_dev->wlan_ap_dev.mode            = OS_WLAN_AP;
    wwd_dev->wlan_ap_dev.parent.type     = OS_DEVICE_TYPE_MISCELLANEOUS;
    
    if (os_wlan_register(&wwd_dev->wlan_ap_dev, OS_WLAN_DEVICE_AP_NAME) != OS_EOK)
    {
        return OS_ERROR;
    }
    
    return OS_EOK;
}
OS_CMPOENT_INIT(os_wlan_wwd_probe, OS_INIT_SUBLEVEL_MIDDLE);

