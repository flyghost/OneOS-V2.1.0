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
 * @file        air720uh.h
 *
 * @brief       air720uh factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __AIR720UH_H__
#define __AIR720UH_H__

#include "mo_object.h"

#ifdef AIR720UH_USING_GENERAL_OPS
#include "air720uh_general.h"
#endif

#ifdef AIR720UH_USING_NETSERV_OPS
#include "air720uh_netserv.h"
#endif

#ifdef AIR720UH_USING_PING_OPS
#include "air720uh_ping.h"
#endif

#ifdef AIR720UH_USING_IFCONFIG_OPS
#include "air720uh_ifconfig.h"
#endif

#ifdef AIR720UH_USING_NETCONN_OPS
#include "air720uh_netconn.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_AIR720UH

#ifndef AIR720UH_NAME
#define AIR720UH_NAME "air720uh"
#endif

#ifndef AIR720UH_DEVICE_NAME
#define AIR720UH_DEVICE_NAME "uart2"
#endif

#ifndef AIR720UH_RECV_BUFF_LEN
#define AIR720UH_RECV_BUFF_LEN 512
#endif

#ifndef AIR720UH_NETCONN_NUM
#define AIR720UH_NETCONN_NUM 6
#endif

typedef struct mo_air720uh
{
    mo_object_t parent;
#ifdef AIR720UH_USING_NETCONN_OPS
    mo_netconn_t    netconn[AIR720UH_NETCONN_NUM];

    os_event_t      netconn_evt;
    os_mutex_t      netconn_lock;
    void            *netconn_data;
#endif /* AIR720UH_USING_NETCONN_OPS */
} mo_air720uh_t;

mo_object_t *module_air720uh_create(const char *name, void *parser_config);
os_err_t     module_air720uh_destroy(mo_object_t *self);

#endif /* MOLINK_USING_AIR720UH */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AIR720UH_H__ */
