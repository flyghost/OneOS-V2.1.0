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
 * @file        wlan_test.c
 *
 * @brief       The test file for wlan.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <wlan_dev.h>
#include <shell.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "wlan_lwip"
#include <drv_log.h>

#define wlan_name "wlan0"
#if 0
static int wifi_scan(int argc, char **argv)
{
    int   index;
    os_device_t    *device = OS_NULL;
    struct os_wlan_scan_result *scan_result = OS_NULL;

    device = os_device_find(wlan_name);
    if (device == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_device_find failed!");
    }
    
    scan_result = os_wlan_scan(device, 2000);
    if (scan_result == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_wlan_scan failed!");
    }

    os_kprintf("             SSID                      MAC            security    rssi chn Mbps\r\n");
    os_kprintf("------------------------------- -----------------  -------------- ---- --- ----\r\n");
    for (index = 0; index < scan_result->num; index++)
    {
        os_kprintf("%s\r\n", &scan_result->scan_info[index].ssid.val[0]);
        os_kprintf("%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                   scan_result->scan_info[index].bssid[0],
                   scan_result->scan_info[index].bssid[1],
                   scan_result->scan_info[index].bssid[2],
                   scan_result->scan_info[index].bssid[3],
                   scan_result->scan_info[index].bssid[4],
                   scan_result->scan_info[index].bssid[5]);
    }
    os_wlan_scan_clean_result(device, scan_result);

    return 0;
}
#endif
static int wifi_scan(int argc, char **argv)
{
    int   index;
    char *security;
    os_device_t    *device = OS_NULL;
    struct os_wlan_scan_result *scan_result = OS_NULL;

    device = os_device_find(wlan_name);
    if (device == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_device_find failed!");
    }
    
    scan_result = os_wlan_scan(device, 2000);
    if (scan_result == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_wlan_scan failed!");
    }

    os_kprintf("             SSID                      MAC            security    rssi chn Mbps\r\n");
    os_kprintf("------------------------------- -----------------  -------------- ---- --- ----\r\n");
    for (index = 0; index < scan_result->num; index++)
    {
        os_kprintf("%-32.32s", &scan_result->scan_info[index].ssid.val[0]);
        os_kprintf("%02x:%02x:%02x:%02x:%02x:%02x  ",
                   scan_result->scan_info[index].bssid[0],
                   scan_result->scan_info[index].bssid[1],
                   scan_result->scan_info[index].bssid[2],
                   scan_result->scan_info[index].bssid[3],
                   scan_result->scan_info[index].bssid[4],
                   scan_result->scan_info[index].bssid[5]);
        switch (scan_result->scan_info[index].security)
        {
        case OS_WLAN_SECURITY_OPEN:
            security = "OPEN";
            break;
        case OS_WLAN_SECURITY_WEP_PSK:
            security = "WEP_PSK";
            break;
        case OS_WLAN_SECURITY_WEP_SHARED:
            security = "WEP_SHARED";
            break;
        case OS_WLAN_SECURITY_WPA_TKIP_PSK:
            security = "WPA_TKIP_PSK";
            break;
        case OS_WLAN_SECURITY_WPA_AES_PSK:
            security = "WPA_AES_PSK";
            break;
        case OS_WLAN_SECURITY_WPA2_AES_PSK:
            security = "WPA2_AES_PSK";
            break;
        case OS_WLAN_SECURITY_WPA2_TKIP_PSK:
            security = "WPA2_TKIP_PSK";
            break;
        case OS_WLAN_SECURITY_WPA2_MIXED_PSK:
            security = "WPA2_MIXED_PSK";
            break;
        case OS_WLAN_SECURITY_WPS_OPEN:
            security = "WPS_OPEN";
            break;
        case OS_WLAN_SECURITY_WPS_SECURE:
            security = "WPS_SECURE";
            break;
        default:
            security = "UNKNOWN";
            break;
        }
        os_kprintf("%-14.14s ", security);
        os_kprintf("%-4d ", scan_result->scan_info[index].signal_strength);
        os_kprintf("%3d ", scan_result->scan_info[index].channel);
        os_kprintf("%4d\r\n", scan_result->scan_info[index].max_data_rate / 1000);
    }
    
    os_wlan_scan_clean_result(device, scan_result);
    
    return 0;
}
SH_CMD_EXPORT(wifi_scan, wifi_scan, "wifi_scan");

static int wifi_join(int argc, char **argv)
{
    const char     *ssid = OS_NULL;
    const char     *key = OS_NULL;
    os_device_t    *device = OS_NULL;

    if (argc >= 2)
    {
        /* ssid */
        ssid = argv[1];
    }

    if (argc >= 3)
    {
        /* Password */
        key = argv[2];
    }
    
    device = os_device_find(wlan_name);
    if (device == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_device_find failed!");
    }
    
    if (os_wlan_join(device, ssid, key) != OS_EOK)
        LOG_E(DRV_EXT_TAG, "os_wlan_join failed! %s %s", ssid, key);
    else
        LOG_I(DRV_EXT_TAG, "os_wlan_join success! %s %s", ssid, key);

    return 0;
}
SH_CMD_EXPORT(wifi_join, wifi_join, "wifi_join");

static int wifi_leave(int argc, char *argv[])
{
    os_device_t    *device = OS_NULL;
    
    device = os_device_find(wlan_name);
    if (device == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "os_device_find failed!");
    }
    
    if (os_wlan_leave(device) != OS_EOK)
        LOG_E(DRV_EXT_TAG, "os_wlan_leave failed!");
    else
        LOG_I(DRV_EXT_TAG, "os_wlan_leave success!");
    return 0;
}
SH_CMD_EXPORT(wifi_leave, wifi_leave, "wifi_leave");
