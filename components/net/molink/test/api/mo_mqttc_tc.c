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
 * @file        mo_mqttc_tc.c
 *
 * @brief       Molink modules mqtt client API test cases
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <atest.h>
#include <shell.h>
#include <os_types.h>
#include <device.h>
#include <serial.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <mo_api.h>

#define MO_LOG_TAG "molink.mqttc.tc"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define TEST_HOST      "121.89.166.244"
#define TEST_PORT      (6726)

#define TEST_CLIENT_ID "OneOS_MQTTClient"
#define TEST_USERNAME  "username"
#define TEST_PASSWORD  "password"
#define TEST_TOPIC     "oneostest"
#define TEST_TOPIC_1   "oneostest1"
#define TEST_TOPIC_2   "oneostest2"
#define TEST_TOPIC_3   "oneostest3"

#define TEST_MQTTC_MAX_NUM (1)
#define TEST_MQTTC_SUB_NUM (5)
#define TEST_MQTTC_PUB_NUM (100)

static mo_object_t *test_module  = OS_NULL;
static os_sem_t    *test_sem     = OS_NULL;
static os_sem_t    *test_sem1    = OS_NULL;
static os_sem_t    *test_sem2    = OS_NULL;
static os_sem_t    *test_sem3    = OS_NULL;

os_err_t sh_exec(const char *cmd);

static void test_mqttc_create_and_destroy(void)
{
    sh_exec("mem");

    mqttc_create_opts_t create_opts = {
        .address = {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)},
        .port = TEST_PORT};

    mo_mqttc_t *mqttc[TEST_MQTTC_MAX_NUM] = {0};

    for (int i = 0; i < TEST_MQTTC_MAX_NUM; i++)
    {
        mqttc[i] = mo_mqttc_create(test_module, &create_opts);
        tp_assert_not_null(mqttc);
    }

    mo_mqttc_t *mqttc_tmp = mo_mqttc_create(test_module, &create_opts);
    tp_assert_null(mqttc_tmp);

    for (int i = 0; i < TEST_MQTTC_MAX_NUM; i++)
    {
        tp_assert_true(mo_mqttc_destroy(mqttc[i]) == OS_EOK);
    }

    mqttc_tmp = mo_mqttc_create(test_module, &create_opts);
    tp_assert_not_null(mqttc_tmp);

    tp_assert_true(mo_mqttc_destroy(mqttc_tmp) == OS_EOK);

    sh_exec("mem");
}

static void test_mqttc_connect_and_disconnect(void)
{
    mqttc_create_opts_t create_opts = {
        .address = {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)},
        .port = TEST_PORT};

    mo_mqttc_t *mqttc = mo_mqttc_create(test_module, &create_opts);
    tp_assert_not_null(mqttc);

    mqttc_conn_opts_t conn_opts = {
        .client_id = {
            .data = TEST_CLIENT_ID,
            .len  = strlen(TEST_CLIENT_ID),
        },
        .mqtt_version  = 4,
        .keep_alive    = 60,
        .clean_session = 1,
        .will_flag     = 0,
    };

    os_err_t result = mo_mqttc_connect(mqttc, &conn_opts);
    tp_assert_true(result == OS_EOK);

    tp_assert_true(mo_mqttc_isconnect(mqttc));

    result = mo_mqttc_disconnect(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_destroy(mqttc);
    tp_assert_true(result == OS_EOK);
}

static void test_mqttc_handler1(mqttc_msg_data_t *data)
{
    INFO("Message arrived on topic %.*s: %.*s",
               data->topic_name.len,
               data->topic_name.data,
               data->message.payload_len,
               data->message.payload);
}

static void test_mqttc_subscribe_and_unsubscribe(void)
{
    mqttc_create_opts_t create_opts = {
        .address = {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)},
        .port = TEST_PORT};
        
    mo_mqttc_t *mqttc = mo_mqttc_create(test_module, &create_opts);
    tp_assert_not_null(mqttc);

    mqttc_conn_opts_t conn_opts = {
        .client_id =
            {
                .data = TEST_CLIENT_ID,
                .len  = strlen(TEST_CLIENT_ID),
            },
        .mqtt_version  = 4,
        .keep_alive    = 60,
        .clean_session = 1,
        .will_flag     = 0,
    };

    os_err_t result = mo_mqttc_connect(mqttc, &conn_opts);
    tp_assert_true(result == OS_EOK);

    for (int i = 0; i < TEST_MQTTC_SUB_NUM; i++)
    {
        result = mo_mqttc_subscribe(mqttc, TEST_TOPIC, MQTTC_QOS_1, test_mqttc_handler1);
        tp_assert_true(result == OS_EOK);

        os_task_msleep(3000);

        result = mo_mqttc_unsubscribe(mqttc, TEST_TOPIC);
        tp_assert_true(result == OS_EOK);
    }

    result = mo_mqttc_disconnect(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_destroy(mqttc);
    tp_assert_true(result == OS_EOK);
}

static void test_mqttc_publish(void)
{
    mqttc_create_opts_t create_opts = {
        .address = {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)},
        .port = TEST_PORT};

    mo_mqttc_t *mqttc = mo_mqttc_create(test_module, &create_opts);
    tp_assert_not_null(mqttc);

    mqttc_conn_opts_t conn_opts = {
        .client_id =
            {
                .data = TEST_CLIENT_ID,
                .len  = strlen(TEST_CLIENT_ID),
            },
        .mqtt_version  = 4,
        .keep_alive    = 60,
        .clean_session = 1,
        .will_flag     = 0,
    };

    os_err_t result = mo_mqttc_connect(mqttc, &conn_opts);
    tp_assert_true(result == OS_EOK);

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        char payload[64] = {0};
        sprintf(payload, "message number %d", i);

        mqttc_msg_t msg = {
            .qos = MQTTC_QOS_1,
            .retained = 0,
            .payload = payload,
            .payload_len = strlen(payload)
        };

        result = mo_mqttc_publish(mqttc, TEST_TOPIC, &msg);
        tp_assert_true(result == OS_EOK);
    }

    result = mo_mqttc_disconnect(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_destroy(mqttc);
    tp_assert_true(result == OS_EOK);
}

static void test_mqttc_publish_and_subscribe(void)
{
    mqttc_create_opts_t create_opts = {
        .address = {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)},
        .port = TEST_PORT};

    mo_mqttc_t *mqttc = mo_mqttc_create(test_module, &create_opts);
    tp_assert_not_null(mqttc);

    mqttc_conn_opts_t conn_opts = {
        .client_id =
            {
                .data = TEST_CLIENT_ID,
                .len  = strlen(TEST_CLIENT_ID),
            },
        .mqtt_version  = 4,
        .keep_alive    = 60,
        .clean_session = 1,
        .will_flag     = 0,
    };

    os_err_t result = mo_mqttc_connect(mqttc, &conn_opts);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_subscribe(mqttc, TEST_TOPIC, MQTTC_QOS_1, test_mqttc_handler1);
    tp_assert_true(result == OS_EOK);

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        char payload[64] = {0};
        sprintf(payload, "message number %d", i);

        mqttc_msg_t msg = {
            .qos = MQTTC_QOS_1,
            .retained = 0,
            .payload = payload,
            .payload_len = strlen(payload)
        };

        result = mo_mqttc_publish(mqttc, TEST_TOPIC, &msg);
        tp_assert_true(result == OS_EOK);

        result = mo_mqttc_yield(mqttc, 1000);
        tp_assert_true(result == OS_EOK);
    }

    result = mo_mqttc_disconnect(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_destroy(mqttc);
    tp_assert_true(result == OS_EOK);
}

static void test_mqttc_handler_with_sem(mqttc_msg_data_t *data)
{
    INFO("Message arrived on topic %.*s: %.*s",
               data->topic_name.len,
               data->topic_name.data,
               data->message.payload_len,
               data->message.payload);

    os_sem_post(test_sem);
}

static void test_mqttc_start_task(void)
{
    test_sem = os_sem_create("socket_test", 0, OS_IPC_FLAG_PRIO);

    mqttc_create_opts_t create_opts = {
        .address = {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)},
        .port = TEST_PORT};

    mo_mqttc_t *mqttc = mo_mqttc_create(test_module, &create_opts);
    tp_assert_not_null(mqttc);

    mqttc_conn_opts_t conn_opts = {
        .client_id =
            {
                .data = TEST_CLIENT_ID,
                .len  = strlen(TEST_CLIENT_ID),
            },
        .mqtt_version  = 4,
        .keep_alive    = 60,
        .clean_session = 1,
        .will_flag     = 0,
    };

    os_err_t result = mo_mqttc_connect(mqttc, &conn_opts);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_start_task(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_subscribe(mqttc, TEST_TOPIC, MQTTC_QOS_1, test_mqttc_handler_with_sem);
    tp_assert_true(result == OS_EOK);

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        char payload[64] = {0};
        sprintf(payload, "message number %d", i);

        mqttc_msg_t msg = {
            .qos = MQTTC_QOS_1,
            .retained = 0,
            .payload = payload,
            .payload_len = strlen(payload)
        };

        result = mo_mqttc_publish(mqttc, TEST_TOPIC, &msg);
        tp_assert_true(result == OS_EOK);
    }

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        result = os_sem_wait(test_sem, OS_IPC_WAITING_FOREVER);
        tp_assert_true(result == OS_EOK);
    }

    os_sem_destroy(test_sem);

    result = mo_mqttc_disconnect(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_destroy(mqttc);
    tp_assert_true(result == OS_EOK);
}

static void test_mqttc_handler_with_sem1(mqttc_msg_data_t *data)
{
    INFO("Message arrived on topic %.*s: %.*s",
               data->topic_name.len,
               data->topic_name.data,
               data->message.payload_len,
               data->message.payload);

    os_sem_post(test_sem1);
}

static void test_mqttc_handler_with_sem2(mqttc_msg_data_t *data)
{
    INFO("Message arrived on topic %.*s: %.*s",
               data->topic_name.len,
               data->topic_name.data,
               data->message.payload_len,
               data->message.payload);

    os_sem_post(test_sem2);
}

static void test_mqttc_handler_with_sem3(mqttc_msg_data_t *data)
{
    INFO("Message arrived on topic %.*s: %.*s",
               data->topic_name.len,
               data->topic_name.data,
               data->message.payload_len,
               data->message.payload);

    os_sem_post(test_sem3);
}

static void test_mqttc_muti_topic(void)
{
    test_sem1 = os_sem_create("socket_test1", 0, OS_IPC_FLAG_PRIO);
    test_sem2 = os_sem_create("socket_test2", 0, OS_IPC_FLAG_PRIO);
    test_sem3 = os_sem_create("socket_test3", 0, OS_IPC_FLAG_PRIO);

    mqttc_create_opts_t create_opts = {
        .address = {
            .data = TEST_HOST, 
            .len = strlen(TEST_HOST)},
        .port = TEST_PORT};

    mo_mqttc_t *mqttc = mo_mqttc_create(test_module, &create_opts);
    tp_assert_not_null(mqttc);

    mqttc_conn_opts_t conn_opts = {
        .client_id =
            {
                .data = TEST_CLIENT_ID,
                .len  = strlen(TEST_CLIENT_ID),
            },
        .mqtt_version  = 4,
        .keep_alive    = 60,
        .clean_session = 1,
        .will_flag     = 0,
    };

    os_err_t result = mo_mqttc_connect(mqttc, &conn_opts);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_start_task(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_subscribe(mqttc, TEST_TOPIC_1, MQTTC_QOS_1, test_mqttc_handler_with_sem1);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_subscribe(mqttc, TEST_TOPIC_2, MQTTC_QOS_1, test_mqttc_handler_with_sem2);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_subscribe(mqttc, TEST_TOPIC_3, MQTTC_QOS_1, test_mqttc_handler_with_sem3);
    tp_assert_true(result == OS_EOK);

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        char payload[64] = {0};
        sprintf(payload, "message number %d", i);

        mqttc_msg_t msg = {
            .qos = MQTTC_QOS_1,
            .retained = 0,
            .payload = payload,
            .payload_len = strlen(payload)
        };

        result = mo_mqttc_publish(mqttc, TEST_TOPIC_1, &msg);
        tp_assert_true(result == OS_EOK);

        result = mo_mqttc_publish(mqttc, TEST_TOPIC_2, &msg);
        tp_assert_true(result == OS_EOK);

        result = mo_mqttc_publish(mqttc, TEST_TOPIC_3, &msg);
        tp_assert_true(result == OS_EOK);
    }

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        result = os_sem_wait(test_sem1, OS_IPC_WAITING_FOREVER);
        tp_assert_true(result == OS_EOK);
    }

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        result = os_sem_wait(test_sem2, OS_IPC_WAITING_FOREVER);
        tp_assert_true(result == OS_EOK);
    }

    for (int i = 0; i < TEST_MQTTC_PUB_NUM; i++)
    {
        result = os_sem_wait(test_sem3, OS_IPC_WAITING_FOREVER);
        tp_assert_true(result == OS_EOK);
    }

    os_sem_destroy(test_sem1);
    os_sem_destroy(test_sem2);
    os_sem_destroy(test_sem3);

    result = mo_mqttc_disconnect(mqttc);
    tp_assert_true(result == OS_EOK);

    result = mo_mqttc_destroy(mqttc);
    tp_assert_true(result == OS_EOK);
}

static void test_case(void)
{
    ATEST_UNIT_RUN(test_mqttc_create_and_destroy);
    os_task_msleep(3000);
    ATEST_UNIT_RUN(test_mqttc_connect_and_disconnect);
    os_task_msleep(3000);
    ATEST_UNIT_RUN(test_mqttc_subscribe_and_unsubscribe);
    os_task_msleep(3000);
    ATEST_UNIT_RUN(test_mqttc_publish);
    os_task_msleep(3000);
    ATEST_UNIT_RUN(test_mqttc_publish_and_subscribe);
    os_task_msleep(3000);
    ATEST_UNIT_RUN(test_mqttc_start_task);
    os_task_msleep(3000);
    ATEST_UNIT_RUN(test_mqttc_muti_topic);
}

static os_err_t test_init(void)
{
    test_module = mo_get_default();

    if (OS_NULL == test_module)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t test_cleanup(void)
{
    os_task_msleep(100);

    return OS_EOK;
}

ATEST_TC_EXPORT(components.net.molink.api.mqttc.tc, test_case, test_init, test_cleanup, TC_PRIORITY_MIDDLE);
