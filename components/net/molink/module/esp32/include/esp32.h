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
 * @file        esp32.h
 *
 * @brief       esp32 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ESP32_H__
#define __ESP32_H__

#include "mo_object.h"

#ifdef ESP32_USING_GENERAL_OPS
#include "esp32_general.h"
#endif

#ifdef ESP32_USING_PING_OPS
#include "esp32_ping.h"
#endif

#ifdef ESP32_USING_IFCONFIG_OPS
#include "esp32_ifconfig.h"
#endif

#ifdef ESP32_USING_NETCONN_OPS
#include "esp32_netconn.h"
#endif

#ifdef ESP32_USING_WIFI_OPS
#include "esp32_wifi.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_ESP32

#ifndef ESP32_NAME
#define ESP32_NAME "esp32"
#endif

#ifndef ESP32_DEVICE_NAME
#define ESP32_DEVICE_NAME "uart2"
#endif

#ifndef ESP32_RECV_BUFF_LEN
#define ESP32_RECV_BUFF_LEN 2048
#endif

#ifndef ESP32_NETCONN_NUM
#define ESP32_NETCONN_NUM 5
#endif

typedef struct mo_esp32
{
    mo_object_t parent;
#ifdef ESP32_USING_WIFI_OPS
    mo_wifi_mode_t wifi_mode;
    mo_wifi_stat_t wifi_stat;
#endif /* ESP32_USING_WIFI_OPS */

#ifdef ESP32_USING_NETCONN_OPS
    mo_netconn_t netconn[ESP32_NETCONN_NUM];
    os_int32_t   curr_connect;
    os_size_t    curr_sent_size;
    os_event_t   netconn_evt;
    os_mutex_t   netconn_lock;
    void        *netconn_data;
#endif /* ESP32_USING_NETCONN_OPS */
} mo_esp32_t;

mo_object_t *module_esp32_create(const char *name, void *parser_config);
os_err_t     module_esp32_destroy(mo_object_t *self);

#endif /* MOLINK_USING_ESP32 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ESP32_H__ */
