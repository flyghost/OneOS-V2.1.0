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
 * @file        clm920rv3.h
 *
 * @brief       clm920rv3 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CLM920RV3_H__
#define __CLM920RV3_H__

#include "mo_object.h"

#ifdef CLM920RV3_USING_GENERAL_OPS
#include "clm920rv3_general.h"
#endif

#ifdef CLM920RV3_USING_NETSERV_OPS
#include "clm920rv3_netserv.h"
#endif

#ifdef CLM920RV3_USING_PING_OPS
#include "clm920rv3_ping.h"
#endif

#ifdef CLM920RV3_USING_IFCONFIG_OPS
#include "clm920rv3_ifconfig.h"
#endif

#ifdef CLM920RV3_USING_NETCONN_OPS
#include "clm920rv3_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_CLM920RV3

#ifndef CLM920RV3_NAME
#define CLM920RV3_NAME "clm920rv3"
#endif

#ifndef CLM920RV3_DEVICE_NAME
#define CLM920RV3_DEVICE_NAME   "uart2"
#endif

#ifndef CLM920RV3_RECV_BUFF_LEN
#define CLM920RV3_RECV_BUFF_LEN (1500)
#endif

#ifndef CLM920RV3_NETCONN_NUM
#define CLM920RV3_NETCONN_NUM   (6)
#endif

typedef struct mo_clm920rv3
{
    mo_object_t parent;
#ifdef CLM920RV3_USING_NETCONN_OPS
    mo_netconn_t netconn[CLM920RV3_NETCONN_NUM];

    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void      *netconn_data;
#endif /* CLM920RV3_USING_NETCONN_OPS */
} mo_clm920rv3_t;

mo_object_t *module_clm920rv3_create(const char *name, void *parser_config);
os_err_t     module_clm920rv3_destroy(mo_object_t *self);

#endif /* MOLINK_USING_CLM920RV3 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __CLM920RV3_H__ */
