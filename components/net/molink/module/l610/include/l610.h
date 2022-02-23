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
 * @file        l610.h
 *
 * @brief       l610 module api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __L610_H__
#define __L610_H__

#include "mo_object.h"

#ifdef L610_USING_GENERAL_OPS
#include "l610_general.h"
#endif

#ifdef L610_USING_NETSERV_OPS
#include "l610_netserv.h"
#endif

#ifdef L610_USING_PING_OPS
#include "l610_ping.h"
#endif

#ifdef L610_USING_IFCONFIG_OPS
#include "l610_ifconfig.h"
#endif

#ifdef L610_USING_NETCONN_OPS
#include "l610_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_L610

#ifndef L610_NAME
#define L610_NAME "l610"
#endif

#ifndef L610_DEVICE_NAME
#define L610_DEVICE_NAME "uart2"
#endif

#ifndef L610_RECV_BUFF_LEN
#define L610_RECV_BUFF_LEN 512
#endif

#ifndef L610_NETCONN_NUM
#define L610_NETCONN_NUM 6
#endif

typedef struct mo_l610
{
    mo_object_t parent;
#ifdef L610_USING_NETCONN_OPS
    mo_netconn_t netconn[L610_NETCONN_NUM];
	
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
#endif /* L610_USING_NETCONN_OPS */
} mo_l610_t;

mo_object_t *module_l610_create(const char *name, void *parser_config);
os_err_t     module_l610_destroy(mo_object_t *self);

#endif /* MOLINK_USING_L610 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __L610_H__ */

