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
 * @file        bc26.h
 *
 * @brief       bc26 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BC26_H__
#define __BC26_H__

#include "mo_object.h"

#ifdef BC26_USING_GENERAL_OPS
#include "bc26_general.h"
#endif

#ifdef BC26_USING_NETSERV_OPS
#include "bc26_netserv.h"
#endif

#ifdef BC26_USING_PING_OPS
#include "bc26_ping.h"
#endif

#ifdef BC26_USING_IFCONFIG_OPS
#include "bc26_ifconfig.h"
#endif

#ifdef BC26_USING_NETCONN_OPS
#include "bc26_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_BC26

#ifndef BC26_NAME
#define BC26_NAME "bc26"
#endif

#ifndef BC26_DEVICE_NAME
#define BC26_DEVICE_NAME "uart1"
#endif

#ifndef BC26_RECV_BUFF_LEN
#define BC26_RECV_BUFF_LEN (1024)
#endif

#ifndef BC26_NETCONN_NUM
#define BC26_NETCONN_NUM   (5)
#endif

typedef struct mo_bc26
{
    mo_object_t  parent;
#ifdef BC26_USING_NETCONN_OPS
    mo_netconn_t netconn[BC26_NETCONN_NUM];
    os_int32_t   curr_connect;
    os_event_t   netconn_evt;
    os_mutex_t   netconn_lock;
    void        *netconn_data;
#endif /* BC26_USING_NETCONN_OPS */

#ifdef BC26_USING_PING_OPS
    char        *ping_data;
    os_event_t   ping_evt;
#endif /* BC26_USING_PING_OPS */
} mo_bc26_t;

mo_object_t *module_bc26_create(const char *name, void *parser_config);
os_err_t     module_bc26_destroy(mo_object_t *self);

#endif /* MOLINK_USING_BC26 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BC26_H__ */
