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
 * @file        sim7600ce.h
 *
 * @brief       sim7600ce factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SIM7600CE_H__
#define __SIM7600CE_H__

#include "mo_object.h"

#ifdef SIM7600CE_USING_GENERAL_OPS
#include "sim7600ce_general.h"
#endif

#ifdef SIM7600CE_USING_NETSERV_OPS
#include "sim7600ce_netserv.h"
#endif

#ifdef SIM7600CE_USING_PING_OPS
#include "sim7600ce_ping.h"
#endif

#ifdef SIM7600CE_USING_IFCONFIG_OPS
#include "sim7600ce_ifconfig.h"
#endif

#ifdef SIM7600CE_USING_NETCONN_OPS
#include "sim7600ce_netconn.h"
#endif

#ifdef SIM7600CE_USING_ONENET_NB_OPS
#include "sim7600ce_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_SIM7600CE

#ifndef SIM7600CE_NAME
#define SIM7600CE_NAME "sim7600ce"
#endif

#ifndef SIM7600CE_DEVICE_NAME
#define SIM7600CE_DEVICE_NAME "uart2"
#endif

#ifndef SIM7600CE_RECV_BUFF_LEN
#define SIM7600CE_RECV_BUFF_LEN (1500)
#endif

#ifndef SIM7600CE_NETCONN_NUM
#define SIM7600CE_NETCONN_NUM (10)
#endif

typedef struct mo_sim7600ce
{
    mo_object_t parent;
#ifdef SIM7600CE_USING_NETCONN_OPS
    mo_netconn_t netconn[SIM7600CE_NETCONN_NUM];
    os_event_t   netconn_evt;
    os_mutex_t   netconn_lock;
#endif /* SIM7600CE_USING_NETCONN_OPS */
} mo_sim7600ce_t;

mo_object_t *module_sim7600ce_create(const char *name, void *parser_config);
os_err_t     module_sim7600ce_destroy(mo_object_t *self);

#endif /* MOLINK_USING_SIM7600CE */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SIM7600CE_H__ */
