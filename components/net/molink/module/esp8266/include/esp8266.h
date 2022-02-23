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
 * @file        esp8266.h
 *
 * @brief       esp8266 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "mo_object.h"

#ifdef ESP8266_USING_GENERAL_OPS
#include "esp8266_general.h"
#endif

#ifdef ESP8266_USING_PING_OPS
#include "esp8266_ping.h"
#endif

#ifdef ESP8266_USING_IFCONFIG_OPS
#include "esp8266_ifconfig.h"
#endif

#ifdef ESP8266_USING_NETCONN_OPS
#include "esp8266_netconn.h"
#endif

#ifdef ESP8266_USING_WIFI_OPS
#include "esp8266_wifi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_ESP8266

#ifndef ESP8266_NAME
#define ESP8266_NAME "esp8266"
#endif

#ifndef ESP8266_DEVICE_NAME
#define ESP8266_DEVICE_NAME "uart2"
#endif

#ifndef ESP8266_RECV_BUFF_LEN
#define ESP8266_RECV_BUFF_LEN 512
#endif

#ifndef ESP8266_NETCONN_NUM
#define ESP8266_NETCONN_NUM 5
#endif

typedef struct mo_esp8266
{
    mo_object_t parent;
#ifdef ESP8266_USING_WIFI_OPS
    mo_wifi_mode_t wifi_mode;
    mo_wifi_stat_t wifi_stat;
    mo_cipdinfo_mode_t wifi_ipdinfo;
    mo_wifi_cipv6_t wifi_ipv6;
#endif /* ESP8266_USING_WIFI_OPS */

#ifdef ESP8266_USING_NETCONN_OPS
    mo_netconn_t netconn[ESP8266_NETCONN_NUM];
    os_int32_t   curr_connect;
    os_size_t    curr_sent_size;
    os_event_t   netconn_evt;
    os_mutex_t   netconn_lock;
    void        *netconn_data;
#endif /* ESP8266_USING_NETCONN_OPS */
} mo_esp8266_t;

mo_object_t *module_esp8266_create(const char *name, void *parser_config);
os_err_t     module_esp8266_destroy(mo_object_t *self);

#ifdef ESP8266_USING_HW_CONTROL
void esp8266_hw_rst(os_base_t rst_pin);
#endif

#endif /* MOLINK_USING_ESP8266 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESP8266_H__ */
