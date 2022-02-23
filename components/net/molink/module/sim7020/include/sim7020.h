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
 * @file        sim7020.h
 *
 * @brief       sim7020 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SIM7020_H__
#define __SIM7020_H__

#include "mo_object.h"

#ifdef SIM7020_USING_GENERAL_OPS
#include "sim7020_general.h"
#endif

#ifdef SIM7020_USING_NETSERV_OPS
#include "sim7020_netserv.h"
#endif

#ifdef SIM7020_USING_PING_OPS
#include "sim7020_ping.h"
#endif

#ifdef SIM7020_USING_IFCONFIG_OPS
#include "sim7020_ifconfig.h"
#endif

#ifdef SIM7020_USING_NETCONN_OPS
#include "sim7020_netconn.h"
#endif

#ifdef SIM7020_USING_ONENET_NB_OPS
#include "sim7020_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_SIM7020

#ifndef SIM7020_NAME
#define SIM7020_NAME "sim7020"
#endif

#ifndef SIM7020_DEVICE_NAME
#define SIM7020_DEVICE_NAME "uart2"
#endif

#ifndef SIM7020_RECV_BUFF_LEN
#define SIM7020_RECV_BUFF_LEN 512
#endif

#ifndef SIM7020_NETCONN_NUM
#define SIM7020_NETCONN_NUM 5
#endif

typedef struct mo_sim7020
{
    mo_object_t parent;
#ifdef SIM7020_USING_NETCONN_OPS
    mo_netconn_t netconn[SIM7020_NETCONN_NUM];
    os_mutex_t   netconn_lock;
#endif /* SIM7020_USING_NETCONN_OPS */
} mo_sim7020_t;

mo_object_t *module_sim7020_create(const char *name, void *parser_config);
os_err_t     module_sim7020_destroy(mo_object_t *self);

#endif /* MOLINK_USING_SIM7020 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SIM7020_H__ */
