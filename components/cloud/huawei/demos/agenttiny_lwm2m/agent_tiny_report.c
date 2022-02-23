/*
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
 * @file        agent_tiny_report.c
 *
 * @brief       Terminal message reporting function
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <stdlib.h>
#include <os_mq.h>
#include <os_task.h>
#include <os_assert.h>
#include "agent_tiny_data_def.h"
#include "agent_tiny_demo.h"

typedef struct
{
    UINT8  message_id;
    UINT8  temperature;
    UINT8  humidity;
    UINT16 luminance;
} app_attr_data_test;

os_mq_t *         g_report_mq = NULL;    // message queue for terminal data reporting
static app_data_t g_app_data;

/**
 ***********************************************************************************************************************
 * @brief           This function is used to query terminal property value.
 *
 * @param[in]       *attr             The property structure object pointer
 *
 * @return          Are the parameters changed？
 * @retval          true            changed.
 * @retval          false           no change.
 ***********************************************************************************************************************
 */
static BOOL get_app_attr_data(app_attr_data_t *attr)
{
    attr->message_id = APP_MESSAGE_POST_ATTR_ID;
    attr->temperature += 1; /* Simulate temperature change */
    attr->humidity += 2;    /* Simulate humidity change */
    attr->luminance += 3;   /* Simulate luminance change */
    return true;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to convert terminal attribute values to network byte order.
 *
 * @param[in]       *net             Network byte order buffer address
 *                  *host            Host byte order buffer address
 *
 * @return          None
 ***********************************************************************************************************************
 */
static void app_attr_hton(void *net, void *host)
{
    app_attr_data_t *n = (app_attr_data_t *)net;
    app_attr_data_t *h = (app_attr_data_t *)host;
    n->message_id      = h->message_id;
    n->temperature     = h->temperature;
    n->humidity        = h->humidity;
    n->luminance       = htons(h->luminance);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to data reporting task, which will query device properties every 20 seconds
 *and report.
 *
 * @param[in]       *parameter       Task parameter pointer
 *
 * @return          None
 ***********************************************************************************************************************
 */
static void agent_tiny_request_task(void *parameter)
{
    app_attr_data_t attr;
    int             ret;

    while (1)
    {
        if (get_app_attr_data(&attr))
        {
            ret = os_mutex_lock(g_app_data.mutex, OS_WAIT_FOREVER);
            if (ret != LOS_OK)
            {
                ATINY_LOG(LOG_ERROR, "get mutex lock fail:%d\n", ret);
                continue;
            }
            app_attr_hton(g_app_data.buff, &attr);
            os_mutex_unlock(g_app_data.mutex);
            ret = os_mq_send(g_report_mq, &g_app_data, sizeof(app_data_t), OS_WAIT_FOREVER);
            if (ret != LOS_OK)
            {
                ATINY_LOG(LOG_ERROR, "message queue send fail:%d\n", ret);
            }
        }
        os_task_msleep(20 * 1000);
    }
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to start cloud platform connection task.
 *
 * @param[in]       *parameter       Task parameter pointer
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void agent_tiny_connect_task(void *parameter)
{
    agent_tiny_entry();
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to parameter initialization.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void agent_tiny_request_init(void)
{
    if (g_report_mq == NULL)
    {
        g_report_mq = os_mq_create("report", sizeof(app_data_t), MQ_MAX_MSG);
    }
    OS_ASSERT(g_report_mq != NULL);

    memset(&g_app_data, 0, sizeof(app_data_t));
    g_app_data.mutex = os_mutex_create("rep_lock", OS_FALSE);
    OS_ASSERT(g_app_data.mutex != NULL);

    g_app_data.buff = atiny_malloc(sizeof(app_attr_data_t));
    OS_ASSERT(g_app_data.buff != NULL);

    g_app_data.type = APP_MESSAGE_REQUEST;
    g_app_data.len  = sizeof(app_attr_data_t);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to parameter de initialization.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
static void agent_tiny_request_deinit(void)
{
    if (g_report_mq != NULL)
    {
        os_mq_destroy(g_report_mq);
        g_report_mq = NULL;
    }

    if (g_app_data.mutex != NULL)
    {
        os_mutex_destroy(g_app_data.mutex);
        g_app_data.mutex = NULL;
    }

    if (g_app_data.buff != NULL)
    {
        atiny_free(g_app_data.buff);
        g_app_data.buff = NULL;
    }

    g_app_data.type = APP_MESSAGE_NULL;
    g_app_data.len  = 0;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to start huawei cloud connection.
 *
 * @param[in]       argc             Number of shell parameters
 *                  argv             parameters address
 *
 * @return          Start connection result
 * @retval          0               Successful.
 * @retval          Other           Fail.
 ***********************************************************************************************************************
 */
static int hw_cloud(char argc, char **argv)
{
    os_uint8_t        start        = 0xFF;
    static os_task_t *connect_task = NULL;
    static os_task_t *report_task  = NULL;

    if (argc < 2)
    {
        atiny_printf("example:\r\n"
                     "hw_cloud start\r\n"
                     "hw_cloud stop\r\n");
        return (-1);
    }

    if (!strncmp((const char *)argv[1], "start", strlen("start")))
    {
        start = TRUE;
    }
    else if (!strncmp((const char *)argv[1], "stop", strlen("stop")))
    {
        start = FALSE;
    }
    else
    {
        atiny_printf("example:\r\n"
                     "hw_cloud start\r\n"
                     "hw_cloud stop\r\n");
        return (-1);
    }
    if (start == TRUE)
    {
        if (connect_task != NULL || report_task != NULL)
        {
            atiny_printf("hw_cloud  is already running\r\n");
            return (-1);
        }
        agent_tiny_request_init();
        connect_task = os_task_create("hw_cloud", agent_tiny_connect_task, NULL, 8096, SHELL_TASK_PRIORITY - 2);
        report_task  = os_task_create("report", agent_tiny_request_task, NULL, 8096, SHELL_TASK_PRIORITY - 1);
        OS_ASSERT(connect_task != NULL && report_task != NULL);
        os_task_startup(connect_task);
        os_task_startup(report_task);
    }
    else if (start == FALSE)
    {
        if (connect_task == NULL || report_task == NULL)
        {
            atiny_printf("hw_cloud  is already stop\r\n");
            return (-1);
        }
        os_task_destroy(report_task);
        os_task_tsleep(0);
        agent_tiny_exit();
        agent_tiny_request_deinit();
        connect_task = report_task = NULL;
    }
    return 0;
}

#ifdef OS_USING_SHELL

#include <shell.h>

SH_CMD_EXPORT(hw_cloud, hw_cloud, "start huawei cloud connect");

#endif /* OS_USING_SHELL */
