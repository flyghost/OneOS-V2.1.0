/*
 * Copyright (c) 2017 Baidu, Inc. All Rights Reserved.
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * README for this sample
 *
 * For demonstrating all the iotcore_client interface in one sample,
 * This sample program do the following steps:
 *  1. Publish one topic to the broker and subscribe the same topic.
 *  2. Publish the same topic every PUB_TIME_INTERVAL seconds.
 *  3. After publish UNSUB_AFTER_PUB_COUNT times, unsubscribe that topic.
 *
 * For implement the above target, you should config in IoTCore as follow:
 *  1. Add a device template, and add a topic in the template with PUB and
 *     SUB authority at the same time.
 *  2. Topic name like $iot/{deviceName}/user/xxx. Set the following macros
 *     TEST_SUB_TOPIC_FORMAT and TEST_PUB_TOPIC_FORMAT to "$iot/%s/user/xxx".
 *  3. Add a device in device list using the above device template.
 *  4. Acquire connection info in device infomation page, and set the info
 *     to relevant macro below.
 *    4.1 ENDPOINT can be found in device list page.
 *    4.2 If you chose authorize method as password, you should set macros
 *        as follow: ENDPOINT, IOTCORE_ID, DEVICE_KEY and DEVICE_SECRET.
 *    4.3 If you chose authorize method as certificate, you should set macros
 *        as follow: ENDPOINT, IOTCORE_ID, DEVICE_KEY. And
 *  5. If you chose authorize method as password, macro CONNECTION_TYPE
 *     can set to TCP or TLS. Variables client_cert and client_key can be
 *     ignored.
 *  6. If you chose authorize method as certificate, macro CONNECTION_TYPE
 *     can set to MUTUAL_TLS. And you should set variables client_cert and
 *     client_key according to the cert-and-keys.txt file downloaded when
 *     creating device.
 */

#include "iotcore_mqtt_client_sample.h"
#include "iotcore_mqtt_client.h"
#include "oneos_config.h"
#include "os_task.h"
#include <os_assert.h>

#include <stdio.h>
#include <time.h>
#include <azure_c_shared_utility/platform.h>
#include <azure_c_shared_utility/threadapi.h>
#include "azure_c_shared_utility/agenttime.h"
#ifdef BAIDUIOT_CLOUD_WITH_TLS
#include "baiduiot_certs.h"
#endif
#define DBG_EXT_TAG "baidu"
#define DBG_EXT_LVL DBG_EXT_INFO
#include <dlog.h>

#ifndef BAIDUIOT_CLOUD_WITH_TLS
#define CONNECTION_TYPE "TCP"
#else
#ifdef BAIDUIOT_PSK
#define CONNECTION_TYPE "TLS"
#endif
#ifdef BAIDUIOT_CA
#define CONNECTION_TYPE "MUTUAL_TLS"
#ifndef BAIDUIOT_DEVICE_SECRET
#define BAIDUIOT_DEVICE_SECRET "eZOuZyjbAFBSZQcS"      //Fake device secret
#endif
#endif
#if MBEDTLS_SSL_MAX_CONTENT_LEN < 0xE5B
#error "MBEDTLS_SSL_MAX_CONTENT_LEN is too short"
#endif
#endif

#define DEVICE_KEY    BAIDUIOT_DEVICE_KEY
#define DEVICE_SECRET BAIDUIOT_DEVICE_SECRET
#define IOTCORE_ID    BAIDUIOT_IOTCORE_ID
#define ENDPOINT      BAIDUIOT_SERVER_ADDRESS

#define PUB_TIME_INTERVAL     5
#define UNSUB_AFTER_PUB_COUNT 5
extern const char client_cert[];
extern const char client_key[];
// subscribe topic and publish topic is the same
// the demo can recive the same message it published, to prove the fuction of sub and pub working
static const char *TEST_SUB_TOPIC_FORMAT = "$iot/%s/user/test";
static const char *TEST_PUB_TOPIC_FORMAT = "$iot/%s/user/test";
static const char *TEST_MESSAGE          = "this is a message for test.";


const char *base_dp_upload_str = "{"
                                 "\"tags\": {%d},"
                                 "\"reported\" {%d}"
                                 "\"desired\": {%d}"
                                 "}";
static char analog_data[256];
static char SAMPLE_START = 0;

static char *generate_analog_data(void)
{
    static int id                         = 0;
    static int temperature_value          = 0;
    static int power_value                = 0;

    if (id != 2147483647)
    {
        id++;
    }
    else
    {
        id = 1;
    }
    ++temperature_value;
    ++power_value;
    temperature_value %= 40;
    power_value       %= 99;
    snprintf(analog_data, sizeof(analog_data), base_dp_upload_str, id, temperature_value, power_value);
    return analog_data;
}


static const char *QosToString(IOTCORE_MQTT_QOS qos_value)
{
    switch (qos_value)
    {
    case QOS_0_AT_MOST_ONCE:
        return "0_Deliver_At_Most_Once";
    case QOS_1_AT_LEAST_ONCE:
        return "1_Deliver_At_Least_Once";
    case QOS_2_EXACTLY_ONCE:
        return "2_Deliver_Exactly_Once";
    case QOS_FAILURE:
        return "Deliver_Failure";
    }
    return "";
}

void recv_callback(uint16_t                    packet_id,
                   IOTCORE_MQTT_QOS            qos,
                   const char                 *topic,
                   const IOTCORE_MQTT_PAYLOAD *msg,
                   int                         is_retained)
{
    char *recv_msg = (char *)malloc(msg->length + 1);
    memset(recv_msg, 0, msg->length + 1);
    memcpy(recv_msg, msg->message, msg->length);

    printf("****************** Incoming Msg ******************\r\n"
           "* Packet Id: %d\r\n* QOS: %s\r\n* Topic Name: %s\r\n* Is Retained: %s\r\n* App Msg: [%s]\r\n"
           "**************************************************\r\n",
           packet_id,
           QosToString(qos),
           topic,
           is_retained == 1 ? "true" : "false",
           recv_msg);
    free(recv_msg);
}

int pub_least_ack_process(MQTT_PUB_STATUS_TYPE status, void *context)
{
    if (status == MQTT_PUB_SUCCESS)
    {
        printf(" - received publish ack from mqtt server\r\n");
    }
    else
    {
        printf(" - fail to publish message to mqtt server\r\n");
    }

    return 0;
}

static int processSubAckFunction(IOTCORE_MQTT_QOS *qosReturn, size_t qosCount, void *context)
{
    printf(" - receive suback from hub server\r\n");
    for (int i = 0; i < qosCount; ++i)
    {
        printf(" - qos return: %d\r\n", qosReturn[i]);
    }

    int *flag = (int *)context;
    *flag     = 1;

    return 0;
}
#include "iotcore_mqtt_client.h"
static void iotcore_mqtt_client_run(void *param)
{
    if (platform_init() != 0)
    {
        printf("platform_init failed\r\n");
        return;
    }
    else
    {
        IOTCORE_INFO _info = {IOTCORE_ID, DEVICE_KEY, DEVICE_SECRET, ENDPOINT};

        MQTT_CONNECTION_TYPE conn_type;
        if (strcmp(CONNECTION_TYPE, "TCP") == 0)
        {
            conn_type = MQTT_CONNECTION_TCP;
        }
        else if (strcmp(CONNECTION_TYPE, "TLS") == 0)
        {
            conn_type = MQTT_CONNECTION_TLS;
        }
        else if (strcmp(CONNECTION_TYPE, "MUTUAL_TLS") == 0)
        {
            conn_type = MQTT_CONNECTION_MUTUAL_TLS;
        }
        else
        {
            printf("collect iotcore info error");
            return;
        }

        IOTCORE_RETRY_POLICY retry_policy = IOTCORE_RETRY_EXPONENTIAL_BACKOFF;

        size_t retry_timeout_limit_in_sec = 1000;

        IOTCORE_MQTT_CLIENT_HANDLE _client_handle = initialize_mqtt_client_handle(&_info,
                                                                                  NULL,
                                                                                  NULL,
                                                                                  conn_type,
                                                                                  recv_callback,
                                                                                  retry_policy,
                                                                                  retry_timeout_limit_in_sec);

        if (_client_handle == NULL)
        {
            printf("Error: fail to initialize IOTCORE_MQTT_CLIENT_HANDLE.\r\n");
            return;
        }

#if defined(BAIDUIOT_CLOUD_WITH_TLS)
        set_certificates(_client_handle, certificates);
#endif
#if defined(BAIDUIOT_CLOUD_WITH_TLS) && defined(BAIDUIOT_CA)
        set_client_cert(_client_handle, client_cert, client_key);
#endif
        int _result = iotcore_mqtt_doconnect(_client_handle, 60);

        if (_result != IOTCORE_ERR_OK)
        {
            printf("fail to establish connection with server.\r\n");
            return;
        }

        // construct test sub and pub topic
        int   _test_sub_topic_len_max = strlen(TEST_SUB_TOPIC_FORMAT) + strlen(DEVICE_KEY);
        int   _test_pub_topic_len_max = strlen(TEST_PUB_TOPIC_FORMAT) + strlen(DEVICE_KEY);
        char *_test_sub_topic         = (char *)malloc(_test_sub_topic_len_max);
        char *_test_pub_topic         = (char *)malloc(_test_pub_topic_len_max);
        if (_test_sub_topic == NULL || _test_pub_topic == NULL)
        {
            printf("Out of memory.\r\n");
            return;
        }
        memset(_test_sub_topic, 0, _test_sub_topic_len_max);
        memset(_test_pub_topic, 0, _test_pub_topic_len_max);
        sprintf(_test_sub_topic, TEST_SUB_TOPIC_FORMAT, DEVICE_KEY);
        sprintf(_test_pub_topic, TEST_PUB_TOPIC_FORMAT, DEVICE_KEY);

        int flag = 0;
        printf("subscibe a message.\r\n");
        subscribe_mqtt_topic(_client_handle, _test_sub_topic, QOS_1_AT_LEAST_ONCE, processSubAckFunction, &flag);

        _result = publish_mqtt_message(_client_handle,
                                       _test_pub_topic,
                                       QOS_1_AT_LEAST_ONCE,
                                       (const uint8_t *)TEST_MESSAGE,
                                       strlen(TEST_MESSAGE),
                                       pub_least_ack_process,
                                       _client_handle);

        int    _published_count = 0;
        time_t _start           = get_time(NULL);
        char* message = generate_analog_data();
        do
        {
            // publish mqtt message every 2 seconds
            if (get_time(NULL) - _start > PUB_TIME_INTERVAL)
            {
                printf("publish a message.\r\n");
                publish_mqtt_message(_client_handle,
                                     _test_pub_topic,
                                     QOS_1_AT_LEAST_ONCE,
                                     (const uint8_t *)message,
                                     strlen(message),
                                     pub_least_ack_process,
                                     _client_handle);
                _published_count++;
                _start = get_time(NULL);
            }

            // unsubscribe the topic after published 5 times, for testing unsubscribe_mqtt_topics function
            if (_published_count == UNSUB_AFTER_PUB_COUNT)
            {
                _published_count++;
                printf("unsubscribe mqtt topic\r\n");
                unsubscribe_mqtt_topics(_client_handle, _test_sub_topic);
            }

            iotcore_mqtt_dowork(_client_handle);
            ThreadAPI_Sleep(10);
        } while (!iotcore_get_mqtt_status(_client_handle).is_destroy_called &&
                 !iotcore_get_mqtt_status(_client_handle).is_disconnect_called &&
                 SAMPLE_START);

        iotcore_mqtt_disconnect(_client_handle);

        if (_test_pub_topic)
        {
            free(_test_pub_topic);
            _test_pub_topic = NULL;
        }
        if (_test_sub_topic)
        {
            free(_test_sub_topic);
            _test_sub_topic = NULL;
        }
        iotcore_mqtt_destroy(_client_handle);
        return;
    }

#ifdef _CRT_DBG_MAP_ALLOC
    _CrtDumpMemoryLeaks();
#endif
    // while(1);
}

os_task_t  *baidu_mqtt_test_thread = NULL;
static void baidu_mqtt_test(void)
{
    SAMPLE_START = OS_TRUE;
    if (baidu_mqtt_test_thread != NULL)
    {
        LOG_E(DBG_EXT_TAG, "baidu mqtt sample is running");
        return;
    }
    baidu_mqtt_test_thread =
        os_task_create("baidu_mqtt_test", iotcore_mqtt_client_run, OS_NULL, 8192, OS_TASK_PRIORITY_MAX / 2);
    if (NULL == baidu_mqtt_test_thread)
    {
        LOG_E(DBG_EXT_TAG, "create thread failed");
        OS_ASSERT(OS_NULL != baidu_mqtt_test_thread);
    }
    os_task_startup(baidu_mqtt_test_thread);
}

static void baidu_mqtt_stop(void)
{
    SAMPLE_START = OS_FALSE;
    baidu_mqtt_test_thread = NULL;
}
#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(baidu_mqtt_start, baidu_mqtt_test, "baidu");
SH_CMD_EXPORT(baidu_mqtt_stop, baidu_mqtt_stop, "baidu");
#endif
