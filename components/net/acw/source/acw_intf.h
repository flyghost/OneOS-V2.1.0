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
 * @file        acw.h
 *
 * @brief       acw declaration  
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-24   OneOS Team      first version
 ***********************************************************************************************************************
 */

#ifndef __ACW_INTF_H__
#define __ACW_INTF_H__

#include <sys/socket.h>

#include "acw.h"

#define WIFI_SSID_MAX_LENGTH 32
typedef struct acw_intf_wifi_ssid
{
    char val[WIFI_SSID_MAX_LENGTH + 1];
} acw_intf_wifi_ssid_t;

typedef struct acw_intf_wifi_info
{
    os_int32_t      channel;  /* radio channel */
    os_int32_t      rssi;     /* signal strength */
    acw_intf_wifi_ssid_t  ssid;     /* ssid */
} acw_intf_wifi_info_t;

typedef struct acw_intf_wifi_scan_result
{
    acw_intf_wifi_info_t    *info_array;
    os_size_t               info_num;
} acw_intf_wifi_scan_result_t;

typedef enum
{
    acw_intf_type_ap    = 1,
    acw_intf_type_sta   = 2,
    acw_intf_type_max   = 3
} acw_intf_type_t;

typedef enum
{
    ACW_INTF_STAT_INIT = 0,
    ACW_INTF_STAT_RESET,
    ACW_INTF_STAT_STA,
    ACW_INTF_STAT_AP,
    ACW_INTF_STAT_MAX
} acw_intf_stat_t;

typedef void (*acw_recv_cb_t) (ip_addr_t addr, os_int32_t remote_port, char *recv_buff, os_int32_t data_size);

extern os_bool_t acw_check_intf_connected(void);
extern void acw_get_intf_ipaddr(acw_intf_type_t type, ip_addr_t *addr);
extern os_err_t acw_get_intf_ipaddr_timeout(acw_intf_type_t type, int loop_ms, unsigned int loop_cnt);
extern void acw_get_intf_gateway(acw_intf_type_t type, ip_addr_t *gw);
extern os_err_t acw_get_intf_mac(acw_intf_type_t type, char mac[]);
extern void acw_intf_set_ap_ip(const char* ip);
extern os_err_t acw_intf_start_ap(char *ssid, char *passwd);
extern os_err_t acw_intf_connect_home_ap(char *ssid, char *passwd);
extern os_err_t acw_intf_disconnect_ap(void);
extern os_err_t acw_intf_ap_start_recv_proc(acw_recv_cb_t recv_cb);
extern void acw_intf_ap_send_send_resp(ip_addr_t addr, os_int32_t remote_port, char *resp, os_int32_t len);
extern os_err_t acw_intf_stop_ap(void);
extern os_err_t acw_intf_do_wifi_scan(char *ssid, int *channel, int timout_s, acw_intf_wifi_scan_result_t *scan_result);
extern os_err_t acw_intf_init(acw_intf_t intf);

#endif /* end of __ACW_INTF_H__ */
