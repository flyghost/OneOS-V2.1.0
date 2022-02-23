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
 * @file        ec200x_600s.h
 *
 * @brief       ec200x_600s factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __EC200X_600S_H__
#define __EC200X_600S_H__

#include "mo_object.h"

#ifdef EC200X_600S_USING_GENERAL_OPS
#include "ec200x_600s_general.h"
#endif

#ifdef EC200X_600S_USING_NETSERV_OPS
#include "ec200x_600s_netserv.h"
#endif

#ifdef EC200X_600S_USING_PING_OPS
#include "ec200x_600s_ping.h"
#endif

#ifdef EC200X_600S_USING_IFCONFIG_OPS
#include "ec200x_600s_ifconfig.h"
#endif

#ifdef EC200X_600S_USING_NETCONN_OPS
#include "ec200x_600s_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_EC200X_600S

#ifndef EC200X_600S_NAME
#define EC200X_600S_NAME "ec200x_600s"
#endif

#ifndef EC200X_600S_DEVICE_NAME
#define EC200X_600S_DEVICE_NAME "uart2"
#endif

#ifndef EC200X_600S_RECV_BUFF_LEN
#define EC200X_600S_RECV_BUFF_LEN 512
#endif

#ifndef EC200X_600S_NETCONN_NUM
#define EC200X_600S_NETCONN_NUM 12
#endif

typedef struct mo_ec200x_600s
{
    mo_object_t parent;
#ifdef EC200X_600S_USING_NETCONN_OPS
    mo_netconn_t netconn[EC200X_600S_NETCONN_NUM];

    os_int32_t curr_connect;
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void      *netconn_data;
#endif /* EC200X_600S_USING_NETCONN_OPS */
} mo_ec200x_600s_t;

mo_object_t *module_ec200x_600s_create(const char *name, void *parser_config);
os_err_t     module_ec200x_600s_destroy(mo_object_t *self);

#endif /* MOLINK_USING_EC200X_600S */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EC200X_600S_H__ */
