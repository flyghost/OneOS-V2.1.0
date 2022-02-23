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
 * @file        sim7070x.h
 *
 * @brief       sim7070x factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SIM7070X_H__
#define __SIM7070X_H__

#include "mo_object.h"

#ifdef SIM7070X_USING_GENERAL_OPS
#include "sim7070x_general.h"
#endif

#ifdef SIM7070X_USING_NETSERV_OPS
#include "sim7070x_netserv.h"
#endif

#ifdef SIM7070X_USING_PING_OPS
#include "sim7070x_ping.h"
#endif

#ifdef SIM7070X_USING_IFCONFIG_OPS
#include "sim7070x_ifconfig.h"
#endif

#ifdef SIM7070X_USING_NETCONN_OPS
#include "sim7070x_netconn.h"
#endif

#ifdef SIM7070X_USING_ONENET_NB_OPS
#include "sim7070x_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_SIM7070X

#ifndef SIM7070X_NAME
#define SIM7070X_NAME "sim7070x"
#endif

#ifndef SIM7070X_DEVICE_NAME
#define SIM7070X_DEVICE_NAME "uart2"
#endif

#ifndef SIM7070X_RECV_BUFF_LEN
#define SIM7070X_RECV_BUFF_LEN (1564)
#endif

#ifndef SIM7070X_NETCONN_NUM
#define SIM7070X_NETCONN_NUM (13)
#endif

typedef struct mo_sim7070x
{
    mo_object_t parent;
#ifdef SIM7070X_USING_NETCONN_OPS
    mo_netconn_t netconn[SIM7070X_NETCONN_NUM];
    os_event_t   netconn_evt;
    os_mutex_t   netconn_lock;
#endif /* SIM7070X_USING_NETCONN_OPS */
} mo_sim7070x_t;

mo_object_t *module_sim7070x_create(const char *name, void *parser_config);
os_err_t     module_sim7070x_destroy(mo_object_t *self);
os_err_t     module_sim7070x_app_network_pdpidx0_init(mo_object_t *self);

#endif /* MOLINK_USING_SIM7070X */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SIM7070X_H__ */
