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
 * @file        m5310a.h
 *
 * @brief       m5310-a factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __M5310A_H__
#define __M5310A_H__

#include "mo_object.h"

#ifdef M5310A_USING_GENERAL_OPS
#include "m5310a_general.h"
#endif

#ifdef M5310A_USING_NETSERV_OPS
#include "m5310a_netserv.h"
#endif

#ifdef M5310A_USING_PING_OPS
#include "m5310a_ping.h"
#endif

#ifdef M5310A_USING_IFCONFIG_OPS
#include "m5310a_ifconfig.h"
#endif

#ifdef M5310A_USING_NETCONN_OPS
#include "m5310a_netconn.h"
#endif

#ifdef M5310A_USING_ONENET_NB_OPS
#include "m5310a_onenet_nb.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_M5310A

#ifndef M5310A_NAME
#define M5310A_NAME "m5310a"
#endif

#ifndef M5310A_DEVICE_NAME
#define M5310A_DEVICE_NAME "uart2"
#endif

#ifndef M5310A_RECV_BUFF_LEN
#define M5310A_RECV_BUFF_LEN 3072
#endif

#ifndef M5310A_NETCONN_NUM
#define M5310A_NETCONN_NUM 7
#endif

typedef struct mo_m5310a
{
    mo_object_t parent;
#ifdef M5310A_USING_NETCONN_OPS
    mo_netconn_t netconn[M5310A_NETCONN_NUM];
    os_mutex_t   netconn_lock;
#endif /* M5310A_USING_NETCONN_OPS */
} mo_m5310a_t;

mo_object_t *module_m5310a_create(const char *name, void *parser_config);
os_err_t     module_m5310a_destroy(mo_object_t *self);

#endif /* MOLINK_USING_M5310A */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __M5310A_H__ */
