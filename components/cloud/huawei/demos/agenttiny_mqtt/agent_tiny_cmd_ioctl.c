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
 * @file        agent_tiny_cmd_ioctl.c
 *
 * @brief       Cloud platform command execution file
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_mq.h>
#include <time.h>
#include <os_assert.h>
#include "agent_tiny_cmd_ioctl.h"
#include "agent_tiny_data_def.h"
#include "log/atiny_log.h"
#include "cJSON.h"

#ifndef array_size
#define array_size(a) (sizeof(a) / sizeof(*(a)))
#endif

extern os_mq_t   *g_report_mq;
static app_data_t g_app_data;

/**
 ***********************************************************************************************************************
 * @brief           This function is agent tiny report initialization.
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
    g_app_data.mutex = os_mutex_create("cmd_lock", OS_FALSE);
    OS_ASSERT(g_app_data.mutex != NULL);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is get local device time.
 *
 * @param[in]       time_buf        Return time string address.
 *                  len             Time_buf length.
 *
 * @return          Function execution result.
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
static int demo_get_time(char *time_buf, int32_t len)
{
    const int32_t min_time_len = 11;
    if ((time_buf == NULL) || (len < min_time_len))
    {
        ATINY_LOG(LOG_ERR, "invalid param len %d", len);
        return ATINY_ERR;
    }
#ifdef OS_USING_RTC
    time_t     now    = time(OS_NULL);
    struct tm *now_tm = localtime(&now);
    atiny_snprintf(time_buf,
                   len,
                   "%04d%02d%02d%02d%02d%02d",
                   now_tm->tm_year + 1900,
                   now_tm->tm_mon + 1,
                   now_tm->tm_mday,
                   now_tm->tm_hour,
                   now_tm->tm_min,
                   now_tm->tm_sec);
#else
    const char demo_time[] = "2020010112";
    memcpy(time_buf, demo_time, len);
#endif
    return ATINY_OK;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used parsing platform commands (JSON format), developers can redefine.
 *
 * @param[in]       serviceid       ServiceId field buffer.
 *                  cmd             Cmd field buffer.
 *                  paras           Paras field buffer (JSON format).
 *                  has_more        Has_more field.
 *                  mid             Message id.
 *
 * @return          Function execution result.
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
static int proc_rcv_msg(const char *serviceid, const char *cmd, cJSON *paras, int has_more, int mid)
{
    atiny_printf("proc_rcv_msg call");
    if (serviceid == NULL || cmd == NULL || paras == NULL)
    {
        atiny_printf("Parameter error!\r\n");
        return ATINY_ARG_INVALID;
    }
    atiny_printf("serviceid: %s\r\n", serviceid);
    atiny_printf("cmd: %s\r\n", cmd);
    if (paras->type == cJSON_String || paras->type == cJSON_Raw)
    {
        if (paras->string == NULL || paras->valuestring == NULL)
        {
            atiny_printf("Parameter error!\r\n");
            return ATINY_ARG_INVALID;
        }
        atiny_printf("paras: %s\r\n", paras->string);
        atiny_printf("value: %s\r\n", paras->valuestring);
    }
    else if (paras->type == cJSON_Number)
    {
        if (paras->string == NULL)
        {
            atiny_printf("Parameter error!\r\n");
            return ATINY_ARG_INVALID;
        }
        atiny_printf("paras: %lf\r\n", paras->string);
        atiny_printf("value: %lf\r\n", paras->valuedouble);
    }
    else if (paras->type == cJSON_Array || paras->type == cJSON_Object)
    {
        if (paras->string == NULL || paras->child == NULL || paras->child->string == NULL)
        {
            atiny_printf("Parameter error!\r\n");
            return ATINY_ARG_INVALID;
        }
        atiny_printf("paras: %s\r\n", paras->string);
        atiny_printf("child: %s\r\n", paras->child->string);
        if (paras->child->type == cJSON_String || paras->child->type == cJSON_Raw)
        {
            if (paras->child->valuestring == NULL)
            {
                atiny_printf("Parameter error!\r\n");
                return ATINY_ARG_INVALID;
            }
            atiny_printf("value: %s\r\n", paras->child->valuestring);
        }
        else if (paras->type == cJSON_Number)
        {
            atiny_printf("value: %lf\r\n", paras->valuedouble);
        }
    }
    else
    {
        if (paras->string == NULL)
        {
            atiny_printf("Parameter error!\r\n");
            return ATINY_ARG_INVALID;
        }
        atiny_printf("paras: %s\r\n", paras->string);
    }
    atiny_printf("has_more: %d\r\n", has_more);
    atiny_printf("mid: %d\r\n", mid);
    return ATINY_OK;
}
/**
 ***********************************************************************************************************************
 * @brief           This function used to perform result message encapsulation.
 *
 * @param[in]       mid             Message id.
 *                  errcode         Error code, generally, 0 is success, others are failure.
 *                  has_more        Is the message complete
 *                  body            Main message body
 *
 * @return          Function execution result.
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
static int send_msg_resp(int mid, int errcode, int has_more, cJSON *body)
{
    cJSON *tmp     = NULL;
    int    ret     = ATINY_ERR;
    cJSON *msg     = NULL;
    char  *str_msg = NULL;

    msg = cJSON_CreateObject();
    if (msg == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateObject null");
        goto EXIT;
    }

    tmp = cJSON_CreateString(MQTT_DEVICE_RSP);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateString  null");
        goto EXIT;
    }
    cJSON_AddItemToObject(msg, MQTT_MSG_TYPE, tmp);

    tmp = cJSON_CreateNumber(mid);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateNumber null");
        goto EXIT;
    }
    cJSON_AddItemToObject(msg, MQTT_MID, tmp);

    tmp = cJSON_CreateNumber(errcode);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateNumber null");
        goto EXIT;
    }
    cJSON_AddItemToObject(msg, MQTT_ERR_CODE, tmp);

    tmp = cJSON_CreateNumber(has_more);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateNumber null");
        goto EXIT;
    }
    cJSON_AddItemToObject(msg, MQTT_HAS_MORE, tmp);

    cJSON_AddItemToObject(msg, MQTT_BODY, body);
    body = NULL;

    str_msg = cJSON_Print(msg);
    if (str_msg == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateNumber null");
        goto EXIT;
    }
    ret = os_mutex_lock(g_app_data.mutex, OS_WAIT_FOREVER);
    if (ret != LOS_OK)
    {
        atiny_free(str_msg);
        ATINY_LOG(LOG_ERROR, "get mutex lock fail:%d\n", ret);
        goto EXIT;
    }
    g_app_data.type = APP_MESSAGE_JSON;
    g_app_data.buff = str_msg;
    g_app_data.len  = strlen((const char *)str_msg);
    os_mutex_unlock(g_app_data.mutex);
    ret = os_mq_send(g_report_mq, &g_app_data, sizeof(app_data_t), OS_WAIT_FOREVER);
    if (ret != LOS_OK)
    {
        atiny_free(str_msg);
        ATINY_LOG(LOG_ERROR, "message queue send fail:%d\n", ret);
        goto EXIT;
    }

EXIT:
    cJSON_Delete(msg);
    if (body)
    {
        cJSON_Delete(body);
    }
    return ret;
}
/**
 ***********************************************************************************************************************
 * @brief           This function used to generate message response body (JSON format).
 *
 * @param[in]       void
 *
 * @return          response body address.
 * @retval          0               fail.
 * @retval          other           response body address.
 ***********************************************************************************************************************
 */
static cJSON *get_resp_body(void)
{
    cJSON      *body = NULL;
    cJSON      *tmp;
    const char *body_para = "body_para";

    body = cJSON_CreateObject();
    if (body == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateObject");
        return NULL;
    }

    tmp = cJSON_CreateString(body_para);
    if (tmp == NULL)
    {
        ATINY_LOG(LOG_ERR, "cJSON_CreateString null");
        goto EXIT;
    }
    cJSON_AddItemToObject(body, body_para, tmp);

    return body;

EXIT:
    cJSON_Delete(body);
    return NULL;
}

enum
{
    SERVERID_IDX,
    CMD_IDX,
    PARAS_IDX,
    HAS_MORE_IDX,
    MID_IDX,
};

/**
 ***********************************************************************************************************************
 * @brief           This function used to processing platform messages.
 *
 * @param[in]       msg             Message address
 *
 * @return          Function execution result..
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
static int handle_rcv_msg(cJSON *msg)
{
    cJSON      *items[5] = {0};
    const char *names[5] = {MQTT_SERVICEID, MQTT_CMD, MQTT_PARAS, MQTT_HAS_MORE, MQTT_MID};
    int         ret;
    cJSON      *body;
    uint32_t    i;

    for (i = 0; i < array_size(items); i++)
    {
        items[i] = cJSON_GetObjectItem(msg, names[i]);
        if (names[i] == NULL)
        {
            ATINY_LOG(LOG_ERR, "cJSON_GetObjectItem %s fail", names[i]);
            return ATINY_ERR;
        }
    }

    if ((items[SERVERID_IDX]->string == NULL) || (items[CMD_IDX]->string == NULL) ||
        (items[HAS_MORE_IDX]->valueint != MQTT_NO_MORE_DATA && items[HAS_MORE_IDX]->valueint != MQTT_MORE_DATA))
    {
        ATINY_LOG(LOG_ERR, "null or err para hasMore %d", items[3]->valueint);
        return ATINY_ERR;
    }

    ret  = proc_rcv_msg(items[SERVERID_IDX]->valuestring,
                       items[CMD_IDX]->valuestring,
                       items[PARAS_IDX],
                       items[HAS_MORE_IDX]->valueint,
                       items[MID_IDX]->valueint);
    body = get_resp_body();
    if (body == NULL)
    {
        ATINY_LOG(LOG_ERR, "get_resp_body fail");
        return ATINY_ERR;
    }

    return send_msg_resp(items[MID_IDX]->valueint,
                         (ret == ATINY_OK) ? MQTT_ERR_CODE_OK : MQTT_ERR_CODE_ERR,
                         MQTT_NO_MORE_DATA,
                         body);
}

/**
 ***********************************************************************************************************************
 * @brief           This function used to processing platform messages.
 *
 * @param[in]       msg             Message address
 *                  len             Message length
 *
 * @return          Function execution result.
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
static int demo_rcv_msg(const uint8_t *msg, int32_t len)
{
    cJSON *parse_msg = NULL;
    int    ret       = ATINY_ERR;
    if ((msg == NULL) || len <= 0)
    {
        ATINY_LOG(LOG_ERR, "invalid param len %ld", len);
        return ATINY_ERR;
    }

    ATINY_LOG(LOG_INFO, "recv msg %s", msg);
    parse_msg = cJSON_Parse((const char *)msg);
    if (parse_msg != NULL)
    {
        ret = handle_rcv_msg(parse_msg);
    }
    else
    {
        ATINY_LOG(LOG_ERR, "cJSON_Parse fail");
    }

    if (parse_msg)
    {
        cJSON_Delete(parse_msg);
    }

    return ret;
}
/**
 ***********************************************************************************************************************
 * @brief           This function used to processing platform messages.
 *
 * @param[in]       cmd             Command type
 *                  arg             Message buffer address
 *                  len             Message buffer length
 *
 * @return          Function execution result.
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
int demo_cmd_ioctl(mqtt_cmd_e cmd, void *arg, int32_t len)
{
    int result = ATINY_ERR;

    switch (cmd)
    {
    case MQTT_GET_TIME:
        result = demo_get_time(arg, len);
        break;
    case MQTT_RCV_MSG:
        result = demo_rcv_msg(arg, len);
        break;
    default:
        break;
    }
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function is agent tiny ioct initialization.
 *
 * @param[in]       void
 *
 * @return          Mqtt message processing callback function address.
 * @retval          0               fail.
 * @retval          other           callback function address.
 ***********************************************************************************************************************
 */
cmd_ioct_func agent_tiny_ioct_init(void)
{
    agent_tiny_report_init();
    return (demo_cmd_ioctl);
}
