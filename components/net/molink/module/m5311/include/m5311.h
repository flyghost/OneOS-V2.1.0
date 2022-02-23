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
 * @file        m5311.h
 *
 * @brief       m5311 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __M5311_H__
#define __M5311_H__

#include "mo_object.h"

#ifdef M5311_USING_GENERAL_OPS
#include "m5311_general.h"
#endif

#ifdef M5311_USING_NETSERV_OPS
#include "m5311_netserv.h"
#endif

#ifdef M5311_USING_PING_OPS
#include "m5311_ping.h"
#endif

#ifdef M5311_USING_IFCONFIG_OPS
#include "m5311_ifconfig.h"
#endif

#ifdef M5311_USING_NETCONN_OPS
#include "m5311_netconn.h"
#endif

#ifdef M5311_USING_ONENET_NB_OPS
#include "m5311_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_M5311

#ifndef M5311_NAME
#define M5311_NAME "m5311"
#endif

#ifndef M5311_DEVICE_NAME
#define M5311_DEVICE_NAME "uart2"
#endif

#ifndef M5311_RECV_BUFF_LEN
#define M5311_RECV_BUFF_LEN (1500)
#endif

#ifndef M5311_NETCONN_NUM
#define M5311_NETCONN_NUM   (5)
#endif

typedef struct mo_m5311 {
    mo_object_t parent;
#ifdef M5311_USING_NETCONN_OPS
    mo_netconn_t netconn[M5311_NETCONN_NUM];
    os_mutex_t   netconn_lock;
    char        *netconn_data;
    os_event_t   netconn_evt;
#endif /* M5311_USING_NETCONN_OPS */
#ifdef M5311_USING_ONENET_NB_OPS
    os_mutex_t   onenetnb_lock;
    os_event_t   onenetnb_evt;
#endif /* M5311_USING_ONENET_NB_OPS */
} mo_m5311_t;

mo_object_t *module_m5311_create(const char *name, void *parser_config);
os_err_t     module_m5311_destroy(mo_object_t *self);

#endif /* MOLINK_USING_M5311 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __M5311_H__ */
