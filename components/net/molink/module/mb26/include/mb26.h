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
 * @file        mb26.h
 *
 * @brief       mb26 factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MB26_H__
#define __MB26_H__

#include "mo_object.h"

#ifdef MB26_USING_GENERAL_OPS
#include "mb26_general.h"
#endif

#ifdef MB26_USING_NETSERV_OPS
#include "mb26_netserv.h"
#endif

#ifdef MB26_USING_PING_OPS
#include "mb26_ping.h"
#endif

#ifdef MB26_USING_IFCONFIG_OPS
#include "mb26_ifconfig.h"
#endif

#ifdef MB26_USING_NETCONN_OPS
#include "mb26_netconn.h"
#endif

#ifdef MB26_USING_CTM2M_OPS
#include "mb26_ctm2m.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef MOLINK_USING_MB26

#ifndef MB26_NAME
#define MB26_NAME "mb26"
#endif

#ifndef MB26_DEVICE_NAME
#define MB26_DEVICE_NAME "uart1"
#endif

#ifndef MB26_RECV_BUFF_LEN
#define MB26_RECV_BUFF_LEN 1088
#endif

#ifndef MB26_NETCONN_NUM
#define MB26_NETCONN_NUM 5
#endif

typedef struct mo_mb26
{
    mo_object_t parent;
#ifdef MB26_USING_NETCONN_OPS
    mo_netconn_t netconn[MB26_NETCONN_NUM];
    os_mutex_t   netconn_lock;
#endif /* MB26_USING_NETCONN_OPS */
} mo_mb26_t;

mo_object_t *module_mb26_create(const char *name, void *parser_config);
os_err_t     module_mb26_destroy(mo_object_t *self);

#endif /* MOLINK_USING_MB26 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MB26_H__ */
