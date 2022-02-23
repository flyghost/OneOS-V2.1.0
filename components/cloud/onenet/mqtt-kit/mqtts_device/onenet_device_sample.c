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
 * @file        onenet_device_sample.c
 * 
 * @brief       Demonstrate a sample which publish messages into message queue periodic.  
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <os_types.h>
#include <os_mq.h>
#include <os_errno.h>
#include <os_task.h>
#include <os_assert.h>
#include <oneos_config.h>
#include "onenet_mqtts.h"

#define DBG_EXT_TAG "onenet_device.sample"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"

const char *base_dp_upload_str = "{"
                                 "\"id\": %d,"
                                 "\"dp\": {"
                                 "\"temperature\": [{"
                                 "\"v\": %d,"
                                 "}],"
                                 "\"power\": [{"
                                 "\"v\": %d"
                                 "}]"
                                 "}"
                                 "}";

extern struct os_mq mqtts_mq;

static void generate_onenet_publish_data_cycle_thread_func(void *arg)
{
    os_err_t rc;
    char     pub_buf[PUB_DATA_BUFF_LEN] = {0};
    char    *pub_msg                    = NULL;
    int      pub_msg_len                = 0;
    mq_msg_t mq_msg;
    int      id                         = 0;
    int      temperature_value          = 0;
    int      power_value                = 0;

    while (1)
    {
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
        snprintf(pub_buf, sizeof(pub_buf), base_dp_upload_str, id, temperature_value, power_value);

        pub_msg     = pub_buf;
        pub_msg_len = strlen(pub_msg);

        memset(&mq_msg, 0x00, sizeof(mq_msg));
        mq_msg.topic_type = DATA_POINT_TOPIC;
        memcpy(mq_msg.data_buf, pub_msg, pub_msg_len);
        mq_msg.data_len = pub_msg_len;

        rc = os_mq_send(&mqtts_mq, (void *)&mq_msg, sizeof(mq_msg_t), 0);
        if (rc != OS_EOK)
        {
            LOG_E(DBG_EXT_TAG, "mqtts_device_messagequeue_send ERR");
        }

        os_task_msleep(10 * 1000);
    }
}

#define GENERATE_ONENET_PUBLISH_DATA_CYCLE_THREAD_STACK_SIZE 1024
os_task_t  *generate_onenet_publish_data_cycle_thread = NULL;
static void generate_onenet_publish_data_cycle(void)
{
    generate_onenet_publish_data_cycle_thread = os_task_create("generate_pubdata",
                                                               generate_onenet_publish_data_cycle_thread_func,
                                                               OS_NULL,
                                                               GENERATE_ONENET_PUBLISH_DATA_CYCLE_THREAD_STACK_SIZE,
                                                               OS_TASK_PRIORITY_MAX / 2);

    if (NULL == generate_onenet_publish_data_cycle_thread)
    {
        LOG_E(DBG_EXT_TAG, "onenet mqtts client create thread failed");
        OS_ASSERT(OS_NULL != generate_onenet_publish_data_cycle_thread);
    }
    os_task_startup(generate_onenet_publish_data_cycle_thread);

}
static void stop_onenet_publish_data_cycle(void)
{
    os_task_destroy(generate_onenet_publish_data_cycle_thread);
    LOG_I(DBG_EXT_TAG, "onenet publish_data_cycle thread stop");
    return;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(generate_onenet_publish_data_cycle,
              generate_onenet_publish_data_cycle,
              "publish message cycle to onenet specified topic");
SH_CMD_EXPORT(stop_onenet_publish_data_cycle,
              stop_onenet_publish_data_cycle,
              "stop publishing message cycle to onenet specified topic");
#endif
