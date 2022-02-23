/*----------------------------------------------------------------------------
 * Copyright (c) <2016-2018>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/
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
 * @file        agent_tiny_demo.c
 *
 * @brief       Handshake, connection and monitoring tasks between terminal and platform
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_mq.h>
#include <os_assert.h>
#include <string.h>
#include <stdlib.h>
#include "liblwm2m.h"
#include "agent_tiny_demo.h"
#include "agent_tiny_data_def.h"

#if defined WITH_AT_FRAMEWORK
#include "at_frame/at_api.h"
#endif

#define DEFAULT_SERVER_IP HUAWEI_IOT_LWM2M_SERVER_ADDRESS /* cloud platform ip */
#define LWM2M_LIFE_TIME   5000                            /* device life time */

char *g_endpoint_name = HUAWEI_IOT_DEVICE_LWM2M_ID; /* Equipment identification code defined on the platform */

#ifdef WITH_DTLS

char *g_endpoint_name_s    = HUAWEI_IOT_DEVICE_LWM2M_ID; /* Equipment identification code defined on the platform */
char *g_endpoint_name_iots = HUAWEI_IOT_DEVICE_LWM2M_ID;
char *g_endpoint_name_bs   = HUAWEI_IOT_DEVICE_LWM2M_ID;

#endif

static void               *g_phandle     = NULL;
static void               *g_report_task = NULL;
static atiny_device_info_t g_device_info;
static atiny_param_t       g_atiny_params;

/**
 ***********************************************************************************************************************
 * @brief           This function is used to PSK string serialization.
 *
 * @param[in]       psk             PSK string address
 *
 * @return          result of serialize PSK
 * @retval          NULL            invalid.
 * @retval          Other           serialize PSK address.
 ***********************************************************************************************************************
 */
#ifdef WITH_DTLS
static unsigned char *get_psk_value(const char *psk)
{
    int psk_len       = strlen(psk);
    int psk_value_len = (psk_len + 1) / 2;

    if (psk_value_len < 4 || psk_value_len > 16)
    {
        ATINY_LOG(LOG_ERROR, "psk length error(%d), range:4-16\n", psk_len);
        return NULL;
    }

    unsigned char *psk_value = (unsigned char *)atiny_malloc(psk_value_len);
    if (psk == NULL)
        return NULL;
    memset(psk_value, 0, psk_value_len);

    for (int i = 0; i < psk_len; i++)
    {
        unsigned char tmp;
        if ('0' <= psk[i] && psk[i] <= '9')
        {
            tmp = psk[i] - '0';
        }
        else if ('A' <= psk[i] && psk[i] <= 'F')
        {
            tmp = psk[i] - 'A' + 0x0A;
        }
        else if ('a' <= psk[i] && psk[i] <= 'f')
        {
            tmp = psk[i] - 'a' + 0x0A;
        }
        else
        {
            ATINY_LOG(LOG_ERROR, "psk error! Huawei IoT cloud only support hex keys.\n");
            atiny_free(psk_value);
            return NULL;
        }

        if (i % 2)
        {
            psk_value[i / 2] += tmp;
        }
        else
        {
            psk_value[i / 2] += (tmp << 4);
        }
    }
    return psk_value;
}
#endif
/**
 ***********************************************************************************************************************
 * @brief           This function is used to validate the structure app_data_t is valid.
 *
 * @param[in]       *data             The app_data_t object pointer
 *
 * @return          structure app_data_t is valid?
 * @retval          true            valid.
 * @retval          false           invalid.
 ***********************************************************************************************************************
 */
static inline BOOL app_data_invalid(app_data_t *data)
{
    if (data == NULL || data->mutex == NULL || (data->type <= APP_MESSAGE_NULL && data->type >= APP_MESSAGE_MAX))
        return true;
    return false;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to data reporting callback function, users can redefine.
 *
 * @param[in]       type            data type.
 *                  cookie          cookie.
 *                  status          data sending status.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void __attribute__((weak)) ack_callback(atiny_report_type_e type, int cookie, data_send_status_e status)
{
    ATINY_LOG(LOG_DEBUG, "type:%d cookie:%d status:%d\n", type, cookie, status);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is terminal report task, listens for all messages that need to be reported.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void app_data_post(void)
{
    extern os_mq_t *g_report_mq;    // message queue for terminal data reporting
    data_report_t   report_data;
    app_data_t      mq_app_data;
    os_size_t       mq_len;
    int             ret = 0;
    int             cnt = 0;

    OS_ASSERT(g_report_mq != NULL);

    report_data.callback = ack_callback;
    report_data.type     = APP_DATA;
    while (1)
    {
        ret = os_mq_recv(g_report_mq, &mq_app_data, sizeof(app_data_t), OS_WAIT_FOREVER, &mq_len);
        if (mq_len != sizeof(app_data_t))
        {
            ATINY_LOG(LOG_ERROR, "message length error: expect(%d), actual(%u)\n", sizeof(app_data_t), mq_len);
            continue;
        }
        if (app_data_invalid(&mq_app_data))
        {
            ATINY_LOG(LOG_ERROR, "message is invalid\n");
            continue;
        }
        if (LOS_OK != os_mutex_lock(mq_app_data.mutex, OS_WAIT_FOREVER))
        {
            ATINY_LOG(LOG_ERROR, "get mutex lock fail\n");
            continue;
        }

        report_data.buf    = mq_app_data.buff;
        report_data.len    = mq_app_data.len;
        report_data.cookie = cnt++;
        ret                = atiny_data_report(g_phandle, &report_data);
        os_mutex_unlock(mq_app_data.mutex);

        if (ret != LOS_OK)
        {
            ATINY_LOG(LOG_ERR, "data report ret: %d\n", ret);
            continue;
        }
        if (APP_MESSAGE_REQUEST == mq_app_data.type)
            ret = atiny_data_change(g_phandle, DEVICE_MEMORY_FREE);
        if (ret != LOS_OK)
            ATINY_LOG(LOG_ERR, "data change ret: %d\n", ret);
    }
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to create terminal report task.
 *
 * @param[in]       None.
 *
 * @return          Create task results
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
UINT32 creat_report_task(void)
{
    UINT32           uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;
    UINT32           TskHandle;

    task_init_param.usTaskPrio   = 1;
    task_init_param.pcName       = "app_post";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)app_data_post;
    task_init_param.uwStackSize  = 0x1000;

    uwRet = LOS_TaskCreate(&TskHandle, &task_init_param, &g_report_task);
    if (LOS_OK != uwRet)
    {
        return uwRet;
    }
    return LOS_OK;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is terminal connection task entry.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void agent_tiny_entry(void)
{
    UINT32                  uwRet = LOS_OK;
    atiny_param_t          *atiny_params;
    atiny_security_param_t *iot_security_param = NULL;
    atiny_security_param_t *bs_security_param  = NULL;

    atiny_device_info_t *device_info = &g_device_info;

#ifdef WITH_DTLS
    device_info->endpoint_name = g_endpoint_name_s;
#else
    device_info->endpoint_name      = g_endpoint_name;
#endif
#ifdef CONFIG_FEATURE_FOTA
    device_info->manufacturer = "Lwm2mFota";
    device_info->dev_type     = "Lwm2mFota";
#else
    device_info->manufacturer       = "Agent_Tiny";
#endif
    atiny_params                        = &g_atiny_params;
    atiny_params->server_params.binding = "UQ";
    // atiny_params->server_params.life_time = LWM2M_LIFE_TIME;
    atiny_params->server_params.life_time   = 20;
    atiny_params->server_params.storing_cnt = 0;

    atiny_params->server_params.bootstrap_mode = BOOTSTRAP_FACTORY;
    atiny_params->server_params.hold_off_time  = 10;

    // pay attention: index 0 for iot server, index 1 for bootstrap server.
    iot_security_param = &(atiny_params->security_params[0]);
    bs_security_param  = &(atiny_params->security_params[1]);

    iot_security_param->server_ip = DEFAULT_SERVER_IP;
    bs_security_param->server_ip  = DEFAULT_SERVER_IP;

#ifdef WITH_DTLS
    iot_security_param->server_port = "5684";
    bs_security_param->server_port  = "5684";

    iot_security_param->psk_Id  = g_endpoint_name_iots;
    iot_security_param->psk     = (char *)get_psk_value(HUAWEI_CLOUD_PSK);
    iot_security_param->psk_len = (strlen(HUAWEI_CLOUD_PSK) + 1) / 2;

    bs_security_param->psk_Id  = g_endpoint_name_bs;
    bs_security_param->psk     = (char *)get_psk_value(HUAWEI_CLOUD_PSK);
    bs_security_param->psk_len = (strlen(HUAWEI_CLOUD_PSK) + 1) / 2;
    if (iot_security_param->psk == NULL || bs_security_param->psk == NULL)
    {
        ATINY_LOG(LOG_ERROR, "psk error! exit connection.\n");
    }
#else
    iot_security_param->server_port = "5683";
    bs_security_param->server_port  = "5683";

    iot_security_param->psk_Id  = NULL;
    iot_security_param->psk     = NULL;
    iot_security_param->psk_len = 0;

    bs_security_param->psk_Id  = NULL;
    bs_security_param->psk     = NULL;
    bs_security_param->psk_len = 0;
#endif

    if (ATINY_OK != atiny_init(atiny_params, &g_phandle))
    {
        return;
    }

    uwRet = creat_report_task();
    if (LOS_OK != uwRet)
    {
        return;
    }

    (void)atiny_bind(device_info, g_phandle);
#ifdef WITH_DTLS
    if (iot_security_param->psk != NULL)
    {
        atiny_free(iot_security_param->psk);
    }
    if (bs_security_param->psk != NULL)
    {
        atiny_free(bs_security_param->psk);
    }
#endif
}
/**
 ***********************************************************************************************************************
 * @brief           This function is exit terminal connection task and releasing resources.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void agent_tiny_exit(void)
{
    LOS_TaskDestroy(g_report_task);
    atiny_deinit(g_phandle);
}
