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
 * @file        bc28.h
 *
 * @brief       bc28 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BC28_H__
#define __BC28_H__

#include "mo_object.h"

#ifdef BC28_USING_GENERAL_OPS
#include "bc28_general.h"
#endif

#ifdef BC28_USING_NETSERV_OPS
#include "bc28_netserv.h"
#endif

#ifdef BC28_USING_PING_OPS
#include "bc28_ping.h"
#endif

#ifdef BC28_USING_IFCONFIG_OPS
#include "bc28_ifconfig.h"
#endif

#ifdef BC28_USING_NETCONN_OPS
#include "bc28_netconn.h"
#endif

#ifdef BC28_USING_ONENET_NB_OPS
#include "bc28_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_BC28

#ifndef BC28_NAME
#define BC28_NAME "bc28"
#endif

#ifndef BC28_DEVICE_NAME
#define BC28_DEVICE_NAME "uart1"
#endif

#ifndef BC28_RECV_BUFF_LEN
#define BC28_RECV_BUFF_LEN  (1500)
#endif

#ifndef BC28_NETCONN_NUM
#define BC28_NETCONN_NUM    (5)                 /* module service occupied 2 socket */
#endif

typedef struct mo_bc28
{
    mo_object_t     parent;
#ifdef BC28_USING_NETCONN_OPS
    mo_netconn_t    netconn[BC28_NETCONN_NUM];
    os_mutex_t      netconn_lock;
    char           *netconn_data;
    os_event_t      netconn_evt;
#endif /* BC28_USING_NETCONN_OPS */
#ifdef BC28_USING_ONENET_NB_OPS
    mo_onenet_cb_t *regist_cb;                  /* module onenet register callback */
#endif /* BC28_USING_ONENET_NB_OPS */

} mo_bc28_t;

mo_object_t *module_bc28_create(const char *name, void *parser_config);
os_err_t     module_bc28_destroy(mo_object_t *self);

#endif /* MOLINK_USING_BC28 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BC28_H__ */
