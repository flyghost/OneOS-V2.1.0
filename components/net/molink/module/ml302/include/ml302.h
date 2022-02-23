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
 * @file        ml302.h
 *
 * @brief       ml302 module api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-14   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ML302_H__
#define __ML302_H__

#include "mo_object.h"

#ifdef ML302_USING_GENERAL_OPS
#include "ml302_general.h"
#endif

#ifdef ML302_USING_NETSERV_OPS
#include "ml302_netserv.h"
#endif

#ifdef ML302_USING_PING_OPS
#include "ml302_ping.h"
#endif

#ifdef ML302_USING_IFCONFIG_OPS
#include "ml302_ifconfig.h"
#endif

#ifdef ML302_USING_NETCONN_OPS
#include "ml302_netconn.h"
#endif

#ifdef ML302_USING_MQTTC_OPS
#include "ml302_mqttc.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <os_event.h>

#ifdef MOLINK_USING_ML302

#ifndef ML302_NAME
#define ML302_NAME "ml302"
#endif

#ifndef ML302_DEVICE_NAME
#define ML302_DEVICE_NAME "uart2"
#endif

#ifndef ML302_RECV_BUFF_LEN
#define ML302_RECV_BUFF_LEN 512
#endif

#ifndef ML302_NETCONN_NUM
#define ML302_NETCONN_NUM 6
#endif

#define ML302_MQTTC_NUM 1   /* The ML302 supports only one MQTT client */

typedef struct mo_ml302
{
    mo_object_t parent;
#ifdef ML302_USING_NETCONN_OPS
    mo_netconn_t netconn[ML302_NETCONN_NUM];
	
	os_int32_t curr_connect;
    os_event_t netconn_evt;
    os_mutex_t netconn_lock;
    void      *netconn_data;
#endif /* ML302_USING_NETCONN_OPS */
#ifdef ML302_USING_MQTTC_OPS
    mo_mqttc_t mqttc[ML302_MQTTC_NUM];
    os_mutex_t mqttc_lock;
#endif /* ML302_USING_MQTTC_OPS */
} mo_ml302_t;

mo_object_t *module_ml302_create(const char *name, void *parser_config);
os_err_t     module_ml302_destroy(mo_object_t *self);

#endif /* MOLINK_USING_ML302 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ML302_H__ */

