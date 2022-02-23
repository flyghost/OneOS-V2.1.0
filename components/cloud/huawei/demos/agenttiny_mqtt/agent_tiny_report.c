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
 * @brief       Cloud platform command execution file
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include "agent_tiny_demo.h"
#include "los_task_adapter.h"
#include "atiny_mqtt/mqtt_client.h"
#include "cJSON.h"
#include "atiny_error.h"
#include "log/atiny_log.h"
#include "los_typedef_adapter.h"
#include "agent_tiny_data_def.h"

static app_data_t g_app_data;
extern os_mq_t   *g_report_mq;

/**
 ***********************************************************************************************************************
 * @brief           This function is used to initialize agent tiny report.
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
static void agent_tiny_report_init(void)
{
    if (g_report_mq == NULL)
    {
        g_report_mq = os_mq_create("report", sizeof(app_data_t), MQ_MAX_MSG);
    }
    OS_ASSERT(g_report_mq != NULL);

    memset(&g_app_data, 0, sizeof(app_data_t));
    g_app_data.mutex = os_mutex_create("rep_lock", OS_FALSE);
    OS_ASSERT(g_app_data.mutex != NULL);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to generate profile JSON file.
 *
 * @param[in]       profile_data    JSON file pointer address.
 *                  name            Property name
 *                  value           Property value
 *
 * @return          Generate result
 * @retval          0               Successful.
 * @retval          Other           Fail.
 ***********************************************************************************************************************
 */
static int create_profile_data(cJSON **profile_data, const char *name, int value)
{
    cJSON *item = NULL;
    cJSON *tmp  = NULL;

    OS_ASSERT(profile_data != NULL && name != NULL);

    int ret = ATINY_ERR;

    item = cJSON_CreateObject();
    if (item == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateObject null");
        return ATINY_ERR;
    }

    tmp = cJSON_CreateNumber(value);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateNumber null");
        goto EXIT;
    }
    cJSON_AddItemToObject(item, name, tmp);
    *profile_data = item;
    ret           = ATINY_OK;

EXIT:
    if (ret != ATINY_OK)
    {
        cJSON_Delete(item);
    }
    return ret;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to generate service JSON file.
 *
 * @param[in]       profile_data    JSON file pointer address.
 *                  num             Number of profile_data arrays.
 *
 * @return          JSON file pointer
 * @retval          0               Fail.
 * @retval          Other           JSON file pointer.
 ***********************************************************************************************************************
 */
static cJSON *create_service_data(cJSON *profile_data[], uint32_t num)
{
    cJSON      *service_data = NULL;
    int         ret          = ATINY_ERR;
    const char *serviceid    = "Agriculture";
    uint32_t    i;

    service_data = cJSON_CreateArray();
    if (service_data == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateArray null");
        goto EXIT;
    }

    for (i = 0; i < num; i++)
    {
        cJSON *item = NULL;
        cJSON *tmp  = NULL;

        item = cJSON_CreateObject();
        if (item == NULL)
        {
            ATINY_LOG(LOG_ERR, "cJSON_CreateObject null");
            goto EXIT;
        }
        cJSON_AddItemToArray(service_data, item);

        tmp = cJSON_CreateString(serviceid);
        if (tmp == NULL)
        {
            ATINY_LOG(LOG_ERR, "cJSON_CreateString null");
            goto EXIT;
        }
        cJSON_AddItemToObject(item, MQTT_SERVICEID, tmp);

        cJSON_AddItemToObject(item, MQTT_SERVICE_DATA, profile_data[i]);
        profile_data[i] = NULL;
    }

    ret = ATINY_OK;

EXIT:
    if (ret != ATINY_OK)
    {
        if (service_data)
        {
            cJSON_Delete(service_data);
            service_data = NULL;
        }
    }

    for (i = 0; i < num; i++)
    {
        if (profile_data[i])
        {
            cJSON_Delete(profile_data[i]);
        }
    }

    return service_data;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to JSON file to string.
 *
 * @param[in]       profile_data    JSON file pointer address.
 *                  num             Number of profile_data arrays.
 *
 * @return          String address
 * @retval          0               Fail.
 * @retval          Other           String address.
 ***********************************************************************************************************************
 */
static char *create_json_data(cJSON *profile_data[], uint32_t num)
{
    cJSON *tmp          = NULL;
    char  *ret          = NULL;
    cJSON *service_data = NULL;
    cJSON *root         = NULL;

    service_data = create_service_data(profile_data, num);
    if (service_data == NULL)
    {
        goto EXIT;
    }

    root = cJSON_CreateObject();
    if (root == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateObject null");
        goto EXIT;
    }

    tmp = cJSON_CreateString(MQTT_DEVICE_REQ);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateString MQTT_DEVICE_REQ null");
        goto EXIT;
    }
    cJSON_AddItemToObject(root, MQTT_MSG_TYPE, tmp);

    tmp = cJSON_CreateNumber(MQTT_NO_MORE_DATA);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateNumber MQTT_NO_MORE_DATA null");
        goto EXIT;
    }
    cJSON_AddItemToObject(root, MQTT_HAS_MORE, tmp);

    cJSON_AddItemToObject(root, MQTT_DATA, service_data);
    service_data = NULL;
    ret          = cJSON_Print(root);
    if (ret == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_Print fail");
    }

EXIT:
    if (root)
    {
        cJSON_Delete(root);
    }
    if (service_data)
    {
        cJSON_Delete(service_data);
    }
    return ret;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to simulated acquisition of ambient temperature.
 *
 * @param[in]       void
 *
 * @return          Ambient temperature.
 ***********************************************************************************************************************
 */
static int get_agriculture_temperature(void)
{
    static int Temperature = 0; /* Agriculture server attribute(Temperature) which defined in IoT platform */
    ++Temperature;
    return (Temperature);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to simulated acquisition of ambient humidity.
 *
 * @param[in]       void
 *
 * @return          Ambient humidity.
 ***********************************************************************************************************************
 */
static int get_agriculture_humidity(void)
{
    static int Humidity = 0; /* Agriculture server attribute(Humidity) which defined in IoT platform */
    Humidity += 2;
    return (Humidity);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to simulated acquisition of ambient luminance.
 *
 * @param[in]       void
 *
 * @return          Ambient luminance.
 ***********************************************************************************************************************
 */
static int get_agriculture_luminance(void)
{
    static int Luminance = 0; /* Agriculture server attribute(luminance) which defined in IoT platform */
    Luminance += 3;
    return (Luminance);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to generate report data.
 *
 * @param[in]       void
 *
 * @return          Message string address.
 * @retval          0               Fail.
 * @retval          Other           String address.
 ***********************************************************************************************************************
 */
static char *mqtt_json_message()
{
    cJSON *profile_data[3] = {0};
    char  *msg             = NULL;
    int    ret             = create_profile_data(&profile_data[0], "Temperature", get_agriculture_temperature());
    ret |= create_profile_data(&profile_data[1], "Humidity", get_agriculture_humidity());
    ret |= create_profile_data(&profile_data[2], "Luminance", get_agriculture_luminance());
    if (ret == ATINY_OK)
    {
        msg = create_json_data(profile_data, 3);
    }
    return msg;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is data monitoring and reporting task.
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
static void agent_tiny_report_task(void)
{
    agent_tiny_report_init();
    while (1)
    {
        int   ret;
        char *msg = mqtt_json_message();
        if (msg != NULL)
        {
            ret = os_mutex_lock(g_app_data.mutex, OS_WAIT_FOREVER);
            if (ret != LOS_OK)
            {
                ATINY_LOG(LOG_ERROR, "get mutex lock fail:%d\n", ret);
                continue;
            }
            g_app_data.type = APP_MESSAGE_JSON;
            g_app_data.buff = msg;
            g_app_data.len  = strlen((const char *)msg);
            os_mutex_unlock(g_app_data.mutex);
            ret = os_mq_send(g_report_mq, &g_app_data, sizeof(app_data_t), OS_WAIT_FOREVER);
            if (ret != LOS_OK)
            {
                ATINY_LOG(LOG_ERROR, "message queue send fail:%d\n", ret);
            }
        }
        (void)LOS_TaskDelay(1 * 1000);
    }
}
/**
 ***********************************************************************************************************************
 * @brief           This function is create data monitoring and reporting task.
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
UINT32 creat_report_task(void)
{
    UINT32           uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;
    UINT32           TskHandle;
    void            *TaskPtr;

    task_init_param.usTaskPrio   = 1;
    task_init_param.pcName       = "app_req";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)agent_tiny_report_task;
    task_init_param.uwStackSize  = 0x1000;

    uwRet = LOS_TaskCreate(&TskHandle, &task_init_param, &TaskPtr);
    if (LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}
