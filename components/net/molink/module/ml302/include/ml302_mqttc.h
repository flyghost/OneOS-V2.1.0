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
 * @file        ml302_netconn.h
 *
 * @brief       ml302 module link kit mqtt client api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-04   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ML302_MQTTC_H__
#define __ML302_MQTTC_H__

#include "mo_mqttc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ML302_USING_MQTTC_OPS

mo_mqttc_t *ml302_mqttc_create(mo_object_t *module, mqttc_create_opts_t *create_opts);
os_err_t    ml302_mqttc_connect(mo_mqttc_t *client, mqttc_conn_opts_t *conn_opts);
os_err_t    ml302_mqttc_publish(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_t *msg);
os_err_t    ml302_mqttc_subscribe(mo_mqttc_t *client, const char *topic_filter, mqttc_qos_t qos);
os_err_t    ml302_mqttc_unsubscribe(mo_mqttc_t *client, const char *topic_filter);
os_err_t    ml302_mqttc_disconnect(mo_mqttc_t *client);
os_err_t    ml302_mqttc_destroy(mo_mqttc_t *client);

#endif /* ML302_USING_MQTTC_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ML302_MQTTC_H__ */
