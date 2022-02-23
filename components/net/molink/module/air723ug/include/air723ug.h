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
 * @file        air723ug.h
 *
 * @brief       air723ug factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __AIR723UG_H__
#define __AIR723UG_H__

#include "mo_object.h"

#ifdef AIR723UG_USING_GENERAL_OPS
#include "air723ug_general.h"
#endif

#ifdef AIR723UG_USING_NETSERV_OPS
#include "air723ug_netserv.h"
#endif

#ifdef AIR723UG_USING_PING_OPS
#include "air723ug_ping.h"
#endif

#ifdef AIR723UG_USING_IFCONFIG_OPS
#include "air723ug_ifconfig.h"
#endif

#ifdef AIR723UG_USING_NETCONN_OPS
#include "air723ug_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_AIR723UG

#ifndef AIR723UG_NAME
#define AIR723UG_NAME "air723ug"
#endif

#ifndef AIR723UG_DEVICE_NAME
#define AIR723UG_DEVICE_NAME "uart2"
#endif

#ifndef AIR723UG_RECV_BUFF_LEN
#define AIR723UG_RECV_BUFF_LEN 512
#endif

#ifndef AIR723UG_NETCONN_NUM
#define AIR723UG_NETCONN_NUM 6
#endif

typedef struct mo_air723ug
{
    mo_object_t parent;
#ifdef AIR723UG_USING_NETCONN_OPS
    mo_netconn_t    netconn[AIR723UG_NETCONN_NUM];

    os_event_t      netconn_evt;
    os_mutex_t      netconn_lock;
    void            *netconn_data;
#endif /* AIR723UG_USING_NETCONN_OPS */
} mo_air723ug_t;

mo_object_t *module_air723ug_create(const char *name, void *parser_config);
os_err_t     module_air723ug_destroy(mo_object_t *self);

#endif /* MOLINK_USING_AIR723UG */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AIR723UG_H__ */
