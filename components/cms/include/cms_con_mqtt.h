/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *use this file except in compliance with the License. You may obtain a copy of
 *the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file        cms_con_mqtt.h
 *
 * @brief       define the mqtt protocol interface of CMS connection component
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-09   OneOS Team      First version.
 ***********************************************************************************************************************/
#ifndef __CMS_CON_MQTT_H__
#define __CMS_CON_MQTT_H__
#include "cms_con_def.h"

#if defined(__cplusplus)
extern "C" {
#endif

typedef enum
{
    mqtt_qos0,
    mqtt_qos1,
    mqtt_sub_fail = 0x80
} cms_mqtt_qos;    //不支持QoS2

typedef struct
{
    cms_mqtt_qos qos;
    void *       payload;
    size_t       payloadlen;
} cms_mqtt_message;

typedef void (*cms_mqtt_message_handler)(void *handle, const char *topic_name, void *payload, size_t payload_len);

typedef struct
{
    uint16_t timeout_ms;   /* 消息超时时间 */
    size_t   sendbuf_size; /* mqtt实例发送区缓存大小 */
    size_t   recvbuf_size; /* mqtt实例接收区缓存大小 */
} cms_mqtt_param;

typedef struct
{
    char *   client_id;    /* 客户端ID */
    uint16_t keep_alive_s; /* 保活时间，单位秒，0为不保活 */
    char *   username;     /* 用户名 */
    char *   password;     /* 密码 */
} cms_mqtt_connect_param;

void *cms_mqtt_init(uint16_t scode, const cms_mqtt_param *param);

void cms_mqtt_deinit(void *handle);

int cms_mqtt_connect(void *handle, cms_mqtt_connect_param *param);

void cms_mqtt_disconnect(void *handle);

int cms_mqtt_subscribe(void *                   handle,
                       const char *             topic_filter,
                       cms_mqtt_qos             qos,
                       cms_mqtt_message_handler message_handler);

int cms_mqtt_unsubscribe(void *handle, const char *topic_filter);

int cms_mqtt_publish(void *handle, const char *topic, cms_mqtt_message *message);

int cms_mqtt_ping_request(void *handle);

int cms_mqtt_get_state(void *handle);

#if defined(__cplusplus)
}
#endif

#endif
