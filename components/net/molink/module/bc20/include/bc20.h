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
 * @file        bc20.h
 *
 * @brief       bc20 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BC20_H__
#define __BC20_H__

#include "mo_object.h"

#ifdef BC20_USING_GENERAL_OPS
#include "bc20_general.h"
#endif

#ifdef BC20_USING_NETSERV_OPS
#include "bc20_netserv.h"
#endif

#ifdef BC20_USING_PING_OPS
#include "bc20_ping.h"
#endif

#ifdef BC20_USING_IFCONFIG_OPS
#include "bc20_ifconfig.h"
#endif

#ifdef BC20_USING_NETCONN_OPS
#include "bc20_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_BC20

#ifndef BC20_NAME
#define BC20_NAME "bc20"
#endif

#ifndef BC20_DEVICE_NAME
#define BC20_DEVICE_NAME "uart1"
#endif

#ifndef BC20_RECV_BUFF_LEN
#define BC20_RECV_BUFF_LEN (1024)
#endif

#ifndef BC20_NETCONN_NUM
#define BC20_NETCONN_NUM   (5)
#endif

typedef struct mo_bc20
{
    mo_object_t  parent;
#ifdef BC20_USING_NETCONN_OPS
    mo_netconn_t netconn[BC20_NETCONN_NUM];
    os_int32_t   curr_connect;
    os_event_t   netconn_evt;
    os_mutex_t   netconn_lock;
    void        *netconn_data;
#endif /* BC20_USING_NETCONN_OPS */

#ifdef BC20_USING_PING_OPS
    char        *ping_data;
    os_event_t   ping_evt;
#endif /* BC20_USING_PING_OPS */
} mo_bc20_t;

mo_object_t *module_bc20_create(const char *name, void *parser_config);
os_err_t     module_bc20_destroy(mo_object_t *self);

#endif /* MOLINK_USING_BC20 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BC20_H__ */
