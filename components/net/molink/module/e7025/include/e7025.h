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
 * @file        e7025.h
 *
 * @brief       e7025 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __E7025_H__
#define __E7025_H__

#include "mo_object.h"

#ifdef E7025_USING_GENERAL_OPS
#include "e7025_general.h"
#endif

#ifdef E7025_USING_NETSERV_OPS
#include "e7025_netserv.h"
#endif

#ifdef E7025_USING_PING_OPS
#include "e7025_ping.h"
#endif

#ifdef E7025_USING_IFCONFIG_OPS
#include "e7025_ifconfig.h"
#endif

#ifdef E7025_USING_NETCONN_OPS
#include "e7025_netconn.h"
#endif

#ifdef E7025_USING_ONENET_NB_OPS
#include "e7025_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_E7025

#ifndef E7025_NAME
#define E7025_NAME "e7025"
#endif

#ifndef E7025_DEVICE_NAME
#define E7025_DEVICE_NAME "uart2"
#endif

#ifndef E7025_RECV_BUFF_LEN
#define E7025_RECV_BUFF_LEN ((1358 + 64) * 2)
#endif

#ifndef E7025_NETCONN_NUM
#define E7025_NETCONN_NUM 5
#endif

typedef struct mo_e7025
{
    mo_object_t parent;
#ifdef E7025_USING_NETCONN_OPS
    mo_netconn_t netconn[E7025_NETCONN_NUM];
    os_mutex_t   netconn_lock;
#endif /* E7025_USING_NETCONN_OPS */
} mo_e7025_t;

mo_object_t *module_e7025_create(const char *name, void *parser_config);
os_err_t     module_e7025_destroy(mo_object_t *self);

#endif /* MOLINK_USING_E7025 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __E7025_H__ */
