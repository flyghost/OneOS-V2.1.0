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
 * @file        gm190.h
 *
 * @brief       gm190 module api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __GM190_H__
#define __GM190_H__

#include "mo_object.h"

#ifdef GM190_USING_GENERAL_OPS
#include "gm190_general.h"
#endif

#ifdef GM190_USING_IFCONFIG_OPS
#include "gm190_ifconfig.h"
#endif

#ifdef GM190_USING_NETCONN_OPS
#include "gm190_netconn.h"
#endif

#ifdef GM190_USING_NETSERV_OPS
#include "gm190_netserv.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_GM190

#ifndef GM190_NAME
#define GM190_NAME "gm190"
#endif

#ifndef GM190_DEVICE_NAME
#define GM190_DEVICE_NAME "uart2"
#endif

#ifndef GM190_RECV_BUFF_LEN
#define GM190_RECV_BUFF_LEN 512
#endif

#ifndef GM190_NETCONN_NUM
#define GM190_NETCONN_NUM 6
#endif

typedef struct mo_gm190
{
    mo_object_t parent;
#ifdef GM190_USING_NETCONN_OPS
    mo_netconn_t netconn[GM190_NETCONN_NUM];
	os_int32_t curr_connect;
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void      *netconn_data;
#endif /* GM190_USING_NETCONN_OPS */
} mo_gm190_t;

mo_object_t *module_gm190_create(const char *name, void *parser_config);
os_err_t     module_gm190_destroy(mo_object_t *self);

#endif /* MOLINK_USING_GM190 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __GM190_H__ */

