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
 * @file        mo_wifi.h
 *
 * @brief       module link kit wifi api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_WIFI_H__
#define __MO_WIFI_H__

#include "mo_object.h"
#include "mo_ipaddr.h"

#ifdef MOLINK_USING_WIFI_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MO_WIFI_SSID_MAX_LENGTH
#define MO_WIFI_SSID_MAX_LENGTH 32
#endif

/* 00:01:02:03:04:05 */
#define MO_WIFI_BSSID_MAX_LENGTH   17
#define MO_WIFI_BSSID_ARRAY_LENGTH 6

/**
 ***********************************************************************************************************************
 * @enum        mo_wifi_mode_t
 *
 * @brief       molink module wifi mode
 ***********************************************************************************************************************
 */
typedef enum mo_wifi_mode
{
    MO_WIFI_MODE_NULL = 0,
    MO_WIFI_MODE_STA,
    MO_WIFI_MODE_AP,     /* not support now */
    MO_WIFI_MODE_AP_STA, /* not support now */
    MO_WIFI_MODE_MAX
} mo_wifi_mode_t;

/**
 ***********************************************************************************************************************
 * @enum        mo_wifi_stat_t
 *
 * @brief       molink module wifi state
 ***********************************************************************************************************************
 */
typedef enum mo_wifi_stat
{
    MO_WIFI_STAT_NULL = 0,
    MO_WIFI_STAT_INIT,
    MO_WIFI_STAT_CONNECTED,
    MO_WIFI_STAT_DISCONNECTED,
    MO_WIFI_STAT_GOT_IP     = 0x8,
    MO_WIFI_STAT_GOT_IPV6   = 0x10,
    MO_WIFI_STAT_MAX
} mo_wifi_stat_t;

/**
 ***********************************************************************************************************************
 * @enum        mo_cipdinfo_mode_t
 *
 * @brief       molink module show opposite end info mode
 ***********************************************************************************************************************
 */
typedef enum mo_cipdinfo_mode
{
    MO_WIFI_CIPINFO_MODE_NULL = -1,
    MO_WIFI_CIPINFO_HIDE_IP_PORT = 0,
    MO_WIFI_CIPINFO_SHOW_IP_PORT = 1,
} mo_cipdinfo_mode_t;

/**
 ***********************************************************************************************************************
 * @enum        mo_wifi_cipv6_t
 *
 * @brief       molink module ipv6 supoort flag
 ***********************************************************************************************************************
 */
typedef enum mo_wifi_cipv6
{
    MO_WIFI_CIPV6_DISABLE   = 0,
    MO_WIFI_CIPV6_ENABLE    = 1,
} mo_wifi_cipv6_t;

/**
 ***********************************************************************************************************************
 * @enum        mo_wifi_ecn_t
 *
 * @brief       molink module wifi encryption
 ***********************************************************************************************************************
 */
typedef enum mo_wifi_ecn
{
    MO_WIFI_ECN_NULL = 0,
    MO_WIFI_ECN_OPEN,
    MO_WIFI_ECN_WEP,
    MO_WIFI_ECN_WPA_PSK,
    MO_WIFI_ECN_WPA2_PSK,
    MO_WIFI_ECN_WPA_WPA2_PSK,
    MO_WIFI_ECN_MAX
} mo_wifi_ecn_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_wifi_ssid_t
 *
 * @brief       molink module wifi ssid
 ***********************************************************************************************************************
 */
typedef struct mo_wifi_ssid
{
    os_uint8_t len;
    char val[MO_WIFI_SSID_MAX_LENGTH + 1];
} mo_wifi_ssid_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_wifi_bssid_t
 *
 * @brief       molink module wifi bssid
 ***********************************************************************************************************************
 */
typedef struct mo_wifi_bssid
{
    char       bssid_str[MO_WIFI_BSSID_MAX_LENGTH + 1]; /* hwaddr */
    os_uint8_t bssid_array[MO_WIFI_BSSID_ARRAY_LENGTH];
} mo_wifi_bssid_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_wifi_info_t
 *
 * @brief       molink module wifi infomation
 ***********************************************************************************************************************
 */
typedef struct mo_wifi_info
{
    mo_wifi_ecn_t   ecn_mode; /* encryption mode */
    os_int32_t      channel;  /* radio channel */
    os_int32_t      rssi;     /* signal strength */
    mo_wifi_ssid_t  ssid;     /* ssid */
    mo_wifi_bssid_t bssid;    /* hwaddr */
} mo_wifi_info_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_wifi_scan_result_t
 *
 * @brief       molink module wifi scan result
 ***********************************************************************************************************************
 */
typedef struct mo_wifi_scan_result
{
    mo_wifi_info_t *info_array;
    os_size_t       info_num;
} mo_wifi_scan_result_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_wifi_ops_t
 *
 * @brief       molink module network wifi ops table
 ***********************************************************************************************************************
 */
typedef struct mo_wifi_ops
{
    os_err_t       (*set_mode)(mo_object_t *module, mo_wifi_mode_t mode);
    mo_wifi_mode_t (*get_mode)(mo_object_t *module);
    mo_wifi_stat_t (*get_stat)(mo_object_t *module);
    os_err_t       (*get_sta_cip)(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask, ip_addr_t *ip6_ll, ip_addr_t *ip6_gl);
    os_err_t       (*set_ap_cip)(mo_object_t *module, char *ip, char *gw, char *mask);
    os_err_t       (*get_ap_cip)(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask);
    os_err_t       (*get_sta_mac)(mo_object_t *module, char mac[]);
    os_err_t       (*get_ap_mac)(mo_object_t *module, char mac[]);
    os_err_t       (*scan_info)(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result);
    os_err_t       (*connect_ap)(mo_object_t *module, const char *ssid, const char *password);
    os_err_t       (*disconnect_ap)(mo_object_t *module);
    // ecn, encryption method
    os_err_t       (*start_ap)(mo_object_t *module, const char *ssid, const char *password, os_uint8_t channel, os_uint8_t ecn);
    os_err_t       (*stop_ap)(mo_object_t *module);
} mo_wifi_ops_t;

os_err_t       mo_wifi_set_mode(mo_object_t *module, mo_wifi_mode_t mode);
mo_wifi_mode_t mo_wifi_get_mode(mo_object_t *module);
mo_wifi_stat_t mo_wifi_get_stat(mo_object_t *module);
os_err_t       mo_wifi_get_sta_cip(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask, ip_addr_t *ip6_ll, ip_addr_t *ip6_gl);
os_err_t       mo_wifi_set_ap_cip(mo_object_t *module, char *ip, char *gw, char *mask);
os_err_t       mo_wifi_get_ap_cip(mo_object_t *module, ip_addr_t *ip, ip_addr_t *gw, ip_addr_t *mask);
os_err_t       mo_wifi_get_sta_mac(mo_object_t *module, char mac[]);
os_err_t       mo_wifi_get_ap_mac(mo_object_t *module, char mac[]);
os_err_t       mo_wifi_scan_info(mo_object_t *module, char *ssid, mo_wifi_scan_result_t *scan_result);
void           mo_wifi_scan_info_free(mo_wifi_scan_result_t *scan_result);
os_err_t       mo_wifi_connect_ap(mo_object_t *module, const char *ssid, const char *password);
os_err_t       mo_wifi_disconnect_ap(mo_object_t *module);
os_err_t       mo_wifi_start_ap(mo_object_t *module, const char *ssid, const char *password, os_uint8_t channel, os_uint8_t ecn);
os_err_t       mo_wifi_stop_ap(mo_object_t *module);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_WIFI_OPS */

#endif /* __MO_WIFI_H__ */
