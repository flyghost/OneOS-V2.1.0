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
 * @file        gm120.h
 *
 * @brief       gm120 module api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __GM120_H__
#define __GM120_H__

#include "mo_object.h"

#ifdef GM120_USING_GENERAL_OPS
#include "gm120_general.h"
#endif

#ifdef GM120_USING_IFCONFIG_OPS
#include "gm120_ifconfig.h"
#endif

#ifdef GM120_USING_PING_OPS
#include "gm120_ping.h"
#endif

#ifdef GM120_USING_NETCONN_OPS
#include "gm120_netconn.h"
#endif

#ifdef GM120_USING_NETSERV_OPS
#include "gm120_netserv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_GM120

#ifndef GM120_NAME
#define GM120_NAME "gm120"
#endif

#ifndef GM120_DEVICE_NAME
#define GM120_DEVICE_NAME "uart2"
#endif

#ifndef GM120_RECV_BUFF_LEN
#define GM120_RECV_BUFF_LEN 512
#endif

#ifndef GM120_NETCONN_NUM
#define GM120_NETCONN_NUM 2
#endif

typedef struct mo_gm120
{
    mo_object_t parent;
#ifdef GM120_USING_NETCONN_OPS
    mo_netconn_t netconn[GM120_NETCONN_NUM];
	
	os_int32_t curr_connect;
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void      *netconn_data;
#endif /* GM120_USING_NETCONN_OPS */
} mo_gm120_t;

mo_object_t *module_gm120_create(const char *name, void *parser_config);
os_err_t     module_gm120_destroy(mo_object_t *self);

#endif /* MOLINK_USING_GM120 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GM120_H__ */

