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
 * @file        ec20.h
 *
 * @brief       ec20 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __EC20_H__
#define __EC20_H__

#include "mo_object.h"

#ifdef EC20_USING_GENERAL_OPS
#include "ec20_general.h"
#endif

#ifdef EC20_USING_NETSERV_OPS
#include "ec20_netserv.h"
#endif

#ifdef EC20_USING_PING_OPS
#include "ec20_ping.h"
#endif

#ifdef EC20_USING_IFCONFIG_OPS
#include "ec20_ifconfig.h"
#endif

#ifdef EC20_USING_NETCONN_OPS
#include "ec20_netconn.h"
#endif

#ifdef EC20_USING_PPP_OPS
#include "ec20_ppp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_EC20

#ifndef EC20_NAME
#define EC20_NAME "ec20"
#endif

#ifndef EC20_DEVICE_NAME
#define EC20_DEVICE_NAME "uart2"
#endif

#ifndef EC20_RECV_BUFF_LEN
#define EC20_RECV_BUFF_LEN 512
#endif

#ifndef EC20_NETCONN_NUM
#define EC20_NETCONN_NUM 12
#endif

typedef struct mo_ec20
{
    mo_object_t parent;
#ifdef EC20_USING_NETCONN_OPS
    os_bool_t   pdp_act;
#endif /* EC20_USING_NETCONN_OPS */

#ifdef EC20_USING_NETCONN_OPS
    mo_netconn_t netconn[EC20_NETCONN_NUM];

    os_int32_t curr_connect;
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void      *netconn_data;
#endif /* EC20_USING_NETCONN_OPS */
} mo_ec20_t;

mo_object_t *module_ec20_create(const char *name, void *parser_config);
os_err_t     module_ec20_destroy(mo_object_t *self);

#endif /* MOLINK_USING_EC20 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EC20_H__ */
