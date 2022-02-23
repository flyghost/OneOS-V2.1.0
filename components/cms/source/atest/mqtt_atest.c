/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        mqtt_atest.c
 *
 * @brief       This is mqtt test file based atest.
 *
 * @revision
 * Date         Author          Notes
 * 2020-10-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "oneos_config.h"
#if defined(CMS_CONNECT_MQTT)
#include "cms_con_mqtt.h"
#include <shell.h>
#include <atest.h>
#include <os_errno.h>
#include <os_assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void *mqtt_handle = NULL;
static int   scode       = 300;
char         topic_str[] = "svr/300/req";

#define MQTT_ATEST_BUFF_DEEP 1000
static void atest_cms_mqtt_init(void)
{
    if (mqtt_handle != NULL)
        return;
    cms_mqtt_param param = {5000, MQTT_ATEST_BUFF_DEEP, MQTT_ATEST_BUFF_DEEP * 2};
    mqtt_handle          = cms_mqtt_init(scode, &param);
    tp_assert_true(mqtt_handle != NULL);
}

static void atest_cms_mqtt_deinit(void)
{
    cms_mqtt_deinit(mqtt_handle);
    mqtt_handle = NULL;
}

static void atest_cms_mqtt_connect(void)
{
    if (cms_mqtt_get_state(mqtt_handle) == cms_con_state_connect)
        return;

    cms_mqtt_connect_param param;
    memset(&param, 0, sizeof(cms_mqtt_connect_param));
    param.client_id    = "12345678";
    param.username     = "oneos";
    param.password     = "123456";
    param.keep_alive_s = 300;
    int ret            = cms_mqtt_connect(mqtt_handle, &param);
    tp_assert_true(ret == 0);
    tp_assert_true(cms_mqtt_get_state(mqtt_handle) == cms_con_state_connect);
}

static void atest_cms_mqtt_disconnect(void)
{
    tp_assert_true(cms_mqtt_get_state(mqtt_handle) == cms_con_state_connect);
    cms_mqtt_disconnect(mqtt_handle);
    tp_assert_true(cms_mqtt_get_state(mqtt_handle) == cms_con_state_disconnect);
}

static void message_handler(void *handle, const char *topic_name, void *payload, size_t payload_len)
{
    printf("%s\r\n", __func__);
    printf("topic:%s\r\n", topic_name);
    uint8_t *buf = (uint8_t *)payload;
    for (int i = 0; i < payload_len; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\r\n");
}

static void atest_cms_mqtt_subscribe(void)
{
    int rc = cms_mqtt_subscribe(mqtt_handle, topic_str, mqtt_qos1, message_handler);
    tp_assert_true(rc == CMS_ERROR_SUCCESS);
}

static void atest_cms_mqtt_unsubscribe(void)
{
    int rc = cms_mqtt_unsubscribe(mqtt_handle, topic_str);
    tp_assert_true(rc == CMS_ERROR_SUCCESS);
}

char        long_buf[MQTT_ATEST_BUFF_DEEP] = {0};
static void atest_cms_mqtt_publish(void)
{
    cms_mqtt_message mqtt_message;

    memset(long_buf, '0', MQTT_ATEST_BUFF_DEEP - 100);

    mqtt_message.qos        = mqtt_qos0;
    mqtt_message.payload    = (void *)long_buf;
    mqtt_message.payloadlen = MQTT_ATEST_BUFF_DEEP - 100;
    int rc                  = cms_mqtt_publish(mqtt_handle, topic_str, &mqtt_message);
    tp_assert_true(rc == CMS_ERROR_SUCCESS);

    memset(long_buf, '1', MQTT_ATEST_BUFF_DEEP - 100);
    mqtt_message.qos        = mqtt_qos1;
    mqtt_message.payload    = (void *)long_buf;
    mqtt_message.payloadlen = MQTT_ATEST_BUFF_DEEP - 100;
    rc                      = cms_mqtt_publish(mqtt_handle, topic_str, &mqtt_message);
    tp_assert_true(rc == CMS_ERROR_SUCCESS);
}

static void atest_cms_mqtt_pingreq(void)
{
    int rc = cms_mqtt_ping_request(mqtt_handle);
    tp_assert_true(rc == CMS_ERROR_SUCCESS);
}

static void atest_cms_mqtt_get_state(void)
{
    int rc = cms_mqtt_get_state(mqtt_handle);
    if (rc == cms_con_state_connect)
        printf("mqtt is connected!\r\n");
    else
        printf("mqtt is not connected!\r\n");
}

static void cms_mqtt_all(void)
{
    ATEST_UNIT_RUN(atest_cms_mqtt_init);
    ATEST_UNIT_RUN(atest_cms_mqtt_connect);
    ATEST_UNIT_RUN(atest_cms_mqtt_subscribe);
    ATEST_UNIT_RUN(atest_cms_mqtt_publish);
    ATEST_UNIT_RUN(atest_cms_mqtt_pingreq);
    ATEST_UNIT_RUN(atest_cms_mqtt_unsubscribe);
    ATEST_UNIT_RUN(atest_cms_mqtt_disconnect);
    ATEST_UNIT_RUN(atest_cms_mqtt_deinit);
}

ATEST_TC_EXPORT(cms.mqtt.total, cms_mqtt_all, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.init, atest_cms_mqtt_init, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.deinit, atest_cms_mqtt_deinit, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.connect, atest_cms_mqtt_connect, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.disconnect, atest_cms_mqtt_disconnect, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.subscribe, atest_cms_mqtt_subscribe, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.unsubscribe, atest_cms_mqtt_unsubscribe, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.publish, atest_cms_mqtt_publish, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.ping, atest_cms_mqtt_pingreq, NULL, NULL, TC_PRIORITY_LOW);
ATEST_TC_EXPORT(cms.mqtt.state, atest_cms_mqtt_get_state, NULL, NULL, TC_PRIORITY_LOW);
#endif
