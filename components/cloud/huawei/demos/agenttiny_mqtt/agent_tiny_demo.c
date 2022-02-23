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
 * @brief       Cloud platform command execution file
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include "agent_tiny_demo.h"
//#include "los_base.h"
#include "los_task_adapter.h"       //#include "los_task.ph"
#include "los_typedef_adapter.h"    //#include "los_typedef.h"
//#include "los_sys.h"
#include "atiny_mqtt/mqtt_client.h"
#include "osdepends/atiny_osdep.h"
#include "log/atiny_log.h"
#include "agent_tiny_data_def.h"
#include "cJSON.h"
#include "agent_tiny_report.h"
#include "agent_tiny_cmd_ioctl.h"

#define DEFAULT_SERVER_IPV4 HUAWEI_IOT_MQTT_SERVER_ADDRESS
#ifdef WITH_DTLS
#if (MQTT_DEMO_USE_PSK == 1)
#define DEFAULT_SERVER_PORT      "8883"
#define AGENT_TINY_DEMO_PASSWORD HUAWEI_CLOUD_PSK
#define AGENT_TINY_DEMO_PSK_ID   HUAWEI_CLOUD_PSK
#elif (MQTT_DEMO_USE_CERT == 1)
#define DEFAULT_SERVER_PORT "8883"
// Device CA certificate string
static char g_mqtt_client_ca_crt[] = "-----BEGIN CERTIFICATE-----\r\n"
                                     "MIIDbjCCAlYCCQCsQGYmkvVpXzANBgkqhkiG9w0BAQsFADB5MQswCQYDVQQGEwJD\r\n"
                                     "TjELMAkGA1UECAwCR0QxCzAJBgNVBAcMAlNaMQ8wDQYDVQQKDAZIdWF3ZWkxDDAK\r\n"
                                     "BgNVBAsMA0lvVDERMA8GA1UEAwwIemhhbmdzYW4xHjAcBgkqhkiG9w0BCQEWDzEy\r\n"
                                     "MzQ1NjdAMTYzLmNvbTAeFw0yMDEyMjIwODE2NDRaFw0yMjA1MDYwODE2NDRaMHkx\r\n"
                                     "CzAJBgNVBAYTAkNOMQswCQYDVQQIDAJHRDELMAkGA1UEBwwCU1oxDzANBgNVBAoM\r\n"
                                     "Bkh1YXdlaTEMMAoGA1UECwwDSW9UMREwDwYDVQQDDAh6aGFuZ3NhbjEeMBwGCSqG\r\n"
                                     "SIb3DQEJARYPMTIzNDU2N0AxNjMuY29tMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8A\r\n"
                                     "MIIBCgKCAQEAtM8UnZb2j5FdCTWUlAlr5xHWtHANUhDbxjTvpRBZ2iCkhB07LUOn\r\n"
                                     "eUgmGw84iNE/RHxRqgqLdmpfwA3MnCaPIEFmvP4mVXZoDBBPs8CfyAosA903DSYT\r\n"
                                     "BwClB0Y/fwb5rB6a2HirzXCYa259Ly5HM3HFhuQdJfH1a5f1pnMOEzakKWkUnY5V\r\n"
                                     "UoeEN2ZYRf69walEz0XdDUs8FzdBJ19gKU2AOOdzbZREEtBf/qZSvFDZf7GP3xlD\r\n"
                                     "b/wgCHssufyAB39BeNuqRWnDfSk77BTyjTebSojGqOSdIl3ZdBOAFpBY0FatQO7o\r\n"
                                     "1yjvollO7nyErJyF2lFT6cR5fkP2d4qH1QIDAQABMA0GCSqGSIb3DQEBCwUAA4IB\r\n"
                                     "AQAcTDBcEV3r8ZoMJeDnnJkvuoANWHM9k7pLpAEjJBrSxNFlM7vht1UZl4zHRkbO\r\n"
                                     "K59xxzUYuk7alV9ZpiVc2bZxzYoFnY0jlIUvutbd/QO/APbQ54WZuL/4Kfvow2st\r\n"
                                     "IJYCrI8W2VrDtwe2I0/o6FXh5ArJj2UZcOOSbB8HsiQjDx7p7TZOnPJIMCbhcG1y\r\n"
                                     "vIm72NHxy2F0RLyHY5CaUwBlg4ArXFpjs1FNtCmJrx1qOAcoXtDvVMpiCfsz74cG\r\n"
                                     "XVe3uYxy0VabU0UpwpBQfwsbBDrhWIo+0RWPZx1vfwBDh42zQDVGm6swZlTTeOe1\r\n"
                                     "8gYXyTKs0OPIHkIQQZiP3ii0\r\n"
                                     "-----END CERTIFICATE-----\r\n";
// Device private key string
static char g_mqtt_client_pk_crt[] = "-----BEGIN RSA PRIVATE KEY-----\r\n"
                                     "MIIEpAIBAAKCAQEAtM8UnZb2j5FdCTWUlAlr5xHWtHANUhDbxjTvpRBZ2iCkhB07\r\n"
                                     "LUOneUgmGw84iNE/RHxRqgqLdmpfwA3MnCaPIEFmvP4mVXZoDBBPs8CfyAosA903\r\n"
                                     "DSYTBwClB0Y/fwb5rB6a2HirzXCYa259Ly5HM3HFhuQdJfH1a5f1pnMOEzakKWkU\r\n"
                                     "nY5VUoeEN2ZYRf69walEz0XdDUs8FzdBJ19gKU2AOOdzbZREEtBf/qZSvFDZf7GP\r\n"
                                     "3xlDb/wgCHssufyAB39BeNuqRWnDfSk77BTyjTebSojGqOSdIl3ZdBOAFpBY0Fat\r\n"
                                     "QO7o1yjvollO7nyErJyF2lFT6cR5fkP2d4qH1QIDAQABAoIBAQCe5SVe4J5JyRuT\r\n"
                                     "zkyGVElJRfoDlD2Ak2QwqbC4xMfR08IAgJfSzZPOF8O1IAk0437QNtilsKKjLBR9\r\n"
                                     "eSMJTzGUzST2jfsuO+JwR51jgLJqX9YSh0OsAHvUOZLN88Q1Y/HievrlphaFcp/g\r\n"
                                     "Obc10VutMBzA0JN1F0/wgBjXcGE8K9sNLz9lKkA6UZKBsEDX74kMLQN2WUYQ6KfA\r\n"
                                     "gGE6NgP3LpeedjI/YObHVgp3OThL0vUrTB+gXxWxAFytJk/yGnMTPt60U/G67TRf\r\n"
                                     "Fk5aPSu2foSxHD85jgmqFAqKxeGSQsZ2WsnG+47CTJBJ1geUgI7q2awtFdC8RLaM\r\n"
                                     "z+XF4HI5AoGBAOwa6eAjhIMQYgV/tZX1sihhUonCLEV0vH8xx+xfiVNVCCJG5L77\r\n"
                                     "5jkpZ792p5FT0UpRML7o0sXdEzLgzsMtkvDbuXutdoMblrIa+xS70NucDMgnV9LT\r\n"
                                     "eTKeZqqhtfJZHKJx1dFZySbjbk0mlaopVy/BhHdYC89rfepeWCBPmsWTAoGBAMQL\r\n"
                                     "W4KCefOFg63NIRss6DjVqpJ2jnuFZZWAFN6fmqycAT2ilLVBQsS+TqbkBh5Pnnb7\r\n"
                                     "gWYc9e5FCOwZaOp8CRl65qpIutCii0Sa9yU4shVhAk8AO3e3B1H6pVlx3Vfd/LgP\r\n"
                                     "5OCyD3scGcefkiymB+Zt8Mbjn7tw2r1haRdogt33AoGAZGp556MTe66fEdmkADhz\r\n"
                                     "4iqqMP7NV2jztmEUt1pWjIOQpwthxDxcSRlsD4lGcU+lpqbrRXa+FfsVbEEiju42\r\n"
                                     "HYxF0e0Ph2lXaT6CZOoQRSaUZRgeiQL5++ZUoeOO/5iYzhn+J0C5hXma8uMjKjDU\r\n"
                                     "7PGwBhYDR99euSwQ7QUOPVMCgYEApkPzomZCQ3Ryz05vy3p0oO8lsP33rW7fTsHH\r\n"
                                     "8V9vcQEGVF20vVzR4cZ9MjGltsNT5Bk+D3p0YVYPUR3jyyrTFNcVFkqEljmk39SF\r\n"
                                     "0v2Ym2Ug7huGU+vYu4BWB9v4G3NbyEdCmAar9n9IpfMBp/hQLvADCss2R2RZP9CD\r\n"
                                     "rqSmiNUCgYBw88/Zmn5BXp5mrNl/VCAkNPkazwq+SuNdc1gNfBD20OFG3hMHwwXY\r\n"
                                     "bLQCxfDl5zJzFa6wjTNs/5O+/oSdKmQn6Lg/GRJfQ4/1ETE6ec07CENbAP+0N7ld\r\n"
                                     "vv5okQfdLA3WILLVEwtct/+FG5OTVx1woepX6a8tlxURN2tACRAo3Q==\r\n"
                                     "-----END RSA PRIVATE KEY-----\r\n";
// Server CA certificate string
static char g_mqtt_server_ca_crt[] = "-----BEGIN CERTIFICATE-----\r\n"
                                     "MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh\r\n"
                                     "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n"
                                     "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD\r\n"
                                     "QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT\r\n"
                                     "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\r\n"
                                     "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG\r\n"
                                     "9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB\r\n"
                                     "CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97\r\n"
                                     "nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt\r\n"
                                     "43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P\r\n"
                                     "T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4\r\n"
                                     "gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO\r\n"
                                     "BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR\r\n"
                                     "TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw\r\n"
                                     "DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr\r\n"
                                     "hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg\r\n"
                                     "06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF\r\n"
                                     "PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls\r\n"
                                     "YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk\r\n"
                                     "CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=\r\n"
                                     "-----END CERTIFICATE-----\r\n";
// Device private key password
static char g_mqtt_client_pk_pwd[] = "1234321";
#endif /* MQTT_DEMO_USE_PSK */
#else
#define AGENT_TINY_DEMO_PASSWORD HUAWEI_CLOUD_MQTT_PASSWORD
#ifndef DEFAULT_SERVER_PORT
#define DEFAULT_SERVER_PORT "1883"
#endif
#endif /* WITH_DTLS */

#ifndef AGENT_TINY_DEMO_PASSWORD
#define AGENT_TINY_DEMO_PASSWORD "test_id"
#endif

#define AGENT_TINY_DEMO_PRODUCTID HUAWEI_IOT_MQTT_PRODUCT_ID
#define AGENT_TINY_DEMO_DEVICEID  HUAWEI_IOT_MQTT_PRODUCT_ID "_" HUAWEI_IOT_MQTT_DEVICE_ID

#define MAX_LEN 2048

os_mq_t              *g_report_mq = NULL;
static mqtt_client_s *g_phandle   = NULL;

#if (MQTT_DEMO_USE_PSK == 1)
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
 * @brief           This function is used to initialize Huawei cloud connection.
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
static void hw_cloud_init(void)
{
    if (g_report_mq == NULL)
    {
        g_report_mq = os_mq_create("report", sizeof(app_data_t), MQ_MAX_MSG);
    }
    OS_ASSERT(g_report_mq != NULL);
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to validate the structure app_data_t is valid.
 *
 * @param[in]       *data             The app_data_t object pointer
 *
 * @return          structure app_data_t is valid？
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
 * @brief           This function is equipment data reporting task.
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
void app_data_request_task(void)
{
    app_data_t mq_app_data;
    os_size_t  mq_len;
    int        ret = 0;

    OS_ASSERT(g_report_mq != NULL);
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

        if (g_phandle)
        {
            ret = atiny_mqtt_data_send(g_phandle, mq_app_data.buff, mq_app_data.len, MQTT_QOS_LEAST_ONCE);
            if (ret != ATINY_OK)
            {
                ATINY_LOG(LOG_ERROR, "report fail(%d), data: %s\n", ret, mq_app_data.buff);
            }
        }
        else
        {
            ATINY_LOG(LOG_ERR, "g_phandle null");
        }
        atiny_free(mq_app_data.buff);
        mq_app_data.buff = NULL;
        os_mutex_unlock(mq_app_data.mutex);
    }
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to create equipment data reporting task.
 *
 * @param[in]       void
 *
 * @return          Task creation results
 * @retval          0               success.
 * @retval          other           fail.
 ***********************************************************************************************************************
 */
static UINT32 creat_request_task()
{
    UINT32           uwRet = LOS_OK;
    TSK_INIT_PARAM_S task_init_param;
    UINT32           TskHandle;
    void            *TaskPtr;

    task_init_param.usTaskPrio   = 1;
    task_init_param.pcName       = "app_data_request_task";
    task_init_param.pfnTaskEntry = (TSK_ENTRY_FUNC)app_data_request_task;
    task_init_param.uwStackSize  = 0x1000;

    uwRet = LOS_TaskCreate(&TskHandle, &task_init_param, &TaskPtr);
    if (LOS_OK != uwRet)
    {
        return uwRet;
    }
    return uwRet;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is huawei cloud connection portal.
 *
 * @param[in]       void
 *
 * @return          void
 ***********************************************************************************************************************
 */
void agent_tiny_entry(void)
{
    UINT32             uwRet = LOS_OK;
    mqtt_param_s       atiny_params;
    mqtt_device_info_s device_info;

    atiny_params.server_ip   = DEFAULT_SERVER_IPV4;
    atiny_params.server_port = DEFAULT_SERVER_PORT;
#ifdef WITH_DTLS
#if (MQTT_DEMO_USE_PSK == 1)
    atiny_params.info.security_type    = MQTT_SECURITY_TYPE_PSK;
    atiny_params.info.u.psk.psk_id     = (unsigned char *)AGENT_TINY_DEMO_PSK_ID;
    atiny_params.info.u.psk.psk_id_len = strlen(AGENT_TINY_DEMO_PSK_ID);
    atiny_params.info.u.psk.psk        = get_psk_value(AGENT_TINY_DEMO_PSK_ID);
    atiny_params.info.u.psk.psk_len    = (strlen(AGENT_TINY_DEMO_PSK_ID) + 1) / 2;
#elif (MQTT_DEMO_USE_CERT == 1)
    atiny_params.info.security_type          = MQTT_SECURITY_TYPE_CA;
    atiny_params.info.u.ca.server_ca         = (uint8_t *)g_mqtt_server_ca_crt;
    atiny_params.info.u.ca.server_ca_len     = sizeof(g_mqtt_server_ca_crt);
    atiny_params.info.u.ca.client_ca         = (uint8_t *)g_mqtt_client_ca_crt;
    atiny_params.info.u.ca.client_ca_len     = sizeof(g_mqtt_client_ca_crt);
    atiny_params.info.u.ca.client_pk         = (uint8_t *)g_mqtt_client_pk_crt;
    atiny_params.info.u.ca.client_pk_len     = sizeof(g_mqtt_client_pk_crt);
    atiny_params.info.u.ca.client_pk_pwd     = (uint8_t *)g_mqtt_client_pk_pwd;
    atiny_params.info.u.ca.client_pk_pwd_len = sizeof(g_mqtt_client_pk_pwd);
#else
    atiny_params.info.security_type = MQTT_SECURITY_TYPE_NONE;
#endif /* MQTT_DEMO_USE_PSK */
#else
    atiny_params.info.security_type = MQTT_SECURITY_TYPE_NONE;
#endif /* WITH_DTLS */

    atiny_params.cmd_ioctl = agent_tiny_ioct_init();

    if (ATINY_OK != atiny_mqtt_init(&atiny_params, &g_phandle))
    {
        return;
    }
    uwRet = creat_report_task();
    uwRet |= creat_request_task();
    if (LOS_OK != uwRet)
    {
        return;
    }

    device_info.codec_mode        = MQTT_CODEC_MODE_JSON;
    device_info.sign_type         = MQTT_SIGN_TYPE_HMACSHA256_NO_CHECK_TIME;
    device_info.password          = AGENT_TINY_DEMO_PASSWORD;
    device_info.connection_type   = MQTT_STATIC_CONNECT;
    device_info.u.s_info.deviceid = AGENT_TINY_DEMO_DEVICEID;
    (void)atiny_mqtt_bind(&device_info, g_phandle);
    return;
}
/**
 ***********************************************************************************************************************
 * @brief           This function is used to start huawei cloud connection by shell.
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

    if (argc < 2)
    {
        atiny_printf("input error!\r\n"
                     "example:\r\n"
                     "hw_cloud start\r\n");
        return (-1);
    }

    if (!strncmp((const char *)argv[1], "start", strlen("start")))
    {
        start = TRUE;
    }
    else
    {
        atiny_printf("input error!\r\n"
                     "example:\r\n"
                     "hw_cloud start\r\n");
        return (-1);
    }

    if (connect_task != NULL)
    {
        atiny_printf("hw_cloud is already running\r\n");
        return (-1);
    }

    if (start == TRUE)
    {
        hw_cloud_init();
        if (connect_task == NULL)
        {
            connect_task = os_task_create("hw_cloud", (entry)agent_tiny_entry, NULL, 8096, SHELL_TASK_PRIORITY - 2);
        }
        if (connect_task != NULL)
        {
            os_task_startup(connect_task);
        }
    }
    else if (start == FALSE)
    {
    }

    return 0;
}

#ifdef OS_USING_SHELL

#include <shell.h>

SH_CMD_EXPORT(hw_cloud, hw_cloud, "start huawei cloud connect");

#endif /* OS_USING_SHELL */
