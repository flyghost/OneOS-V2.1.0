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
 * @file        a7600x.h
 *
 * @brief       a7600x factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __A7600X_H__
#define __A7600X_H__

#include "mo_object.h"

#ifdef A7600X_USING_GENERAL_OPS
#include "a7600x_general.h"
#endif

#ifdef A7600X_USING_NETSERV_OPS
#include "a7600x_netserv.h"
#endif

#ifdef A7600X_USING_PING_OPS
#include "a7600x_ping.h"
#endif

#ifdef A7600X_USING_IFCONFIG_OPS
#include "a7600x_ifconfig.h"
#endif

#ifdef A7600X_USING_NETCONN_OPS
#include "a7600x_netconn.h"
#endif

#ifdef A7600X_USING_ONENET_NB_OPS
#include "a7600x_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_A7600X

#ifndef A7600X_NAME
#define A7600X_NAME "a7600x"
#endif

#ifndef A7600X_DEVICE_NAME
#define A7600X_DEVICE_NAME "uart2"
#endif

#ifndef A7600X_RECV_BUFF_LEN
#define A7600X_RECV_BUFF_LEN (1500)
#endif

#ifndef A7600X_NETCONN_NUM
#define A7600X_NETCONN_NUM (10)
#endif

typedef struct mo_a7600x
{
    mo_object_t parent;
#ifdef A7600X_USING_NETCONN_OPS
    mo_netconn_t netconn[A7600X_NETCONN_NUM];
    os_event_t   netconn_evt;
    os_mutex_t   netconn_lock;
#endif /* A7600X_USING_NETCONN_OPS */
} mo_a7600x_t;

mo_object_t *module_a7600x_create(const char *name, void *parser_config);
os_err_t     module_a7600x_destroy(mo_object_t *self);

#endif /* MOLINK_USING_A7600X */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __A7600X_H__ */
