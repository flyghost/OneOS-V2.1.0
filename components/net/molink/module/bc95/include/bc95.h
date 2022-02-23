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
 * @file        bc95.h
 *
 * @brief       bc95 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BC95_H__
#define __BC95_H__

#include "mo_object.h"

#ifdef BC95_USING_GENERAL_OPS
#include "bc95_general.h"
#endif

#ifdef BC95_USING_NETSERV_OPS
#include "bc95_netserv.h"
#endif

#ifdef BC95_USING_PING_OPS
#include "bc95_ping.h"
#endif

#ifdef BC95_USING_IFCONFIG_OPS
#include "bc95_ifconfig.h"
#endif

#ifdef BC95_USING_NETCONN_OPS
#include "bc95_netconn.h"
#endif

#ifdef BC95_USING_ONENET_NB_OPS
#include "bc95_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_BC95

#ifndef BC95_NAME
#define BC95_NAME "bc95"
#endif

#ifndef BC95_DEVICE_NAME
#define BC95_DEVICE_NAME "uart1"
#endif

#ifndef BC95_RECV_BUFF_LEN
#define BC95_RECV_BUFF_LEN  (1500)
#endif

#ifndef BC95_NETCONN_NUM
#define BC95_NETCONN_NUM    (5)                 /* module service occupied 2 socket */
#endif

typedef struct mo_bc95
{
    mo_object_t     parent;
#ifdef BC95_USING_NETCONN_OPS
    mo_netconn_t    netconn[BC95_NETCONN_NUM];
    os_mutex_t      netconn_lock;
    char           *netconn_data;
    os_event_t      netconn_evt;
#endif /* BC95_USING_NETCONN_OPS */
#ifdef BC95_USING_ONENET_NB_OPS
    mo_onenet_cb_t *regist_cb;                  /* module onenet register callback */
#endif /* BC95_USING_ONENET_NB_OPS */

} mo_bc95_t;

mo_object_t *module_bc95_create(const char *name, void *parser_config);
os_err_t     module_bc95_destroy(mo_object_t *self);

#endif /* MOLINK_USING_BC95 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BC95_H__ */
