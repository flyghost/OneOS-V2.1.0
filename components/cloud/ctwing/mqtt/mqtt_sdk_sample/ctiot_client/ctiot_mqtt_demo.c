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
 * @file        ctiot_mqtt_demo.c
 * 
 * @brief       demo for ctiot mqtt
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-12-21   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include "ctiot_mqtt_client.h"
#include "ctiot_log.h"
#include "ctiot_os.h"
#include "ctiot_device_info.h"
#ifdef CTIOT_MQTT_USING_TLS
#include "ctiot_cert.h"
#endif

#ifdef OS_USING_SHELL
#include <shell.h>
#endif


#ifndef array_size
#define array_size(a) (sizeof(a)/sizeof(*(a)))
#endif

#ifdef NEED_DN_DATA
CTIOT_CB_FUNC ctiotCbFunc;
#endif

mqtt_client_s *g_phandle = NULL;
static int to_stop = 0;

#if (DEVICE_TRANSPARENT == 0)
static void ctiot_mqtt_event_upload(void)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    char * payload = NULL;
    EVENT_REPORT_SERVICE_EVENTREPORT para1;

    memset(&para1, 0, sizeof(EVENT_REPORT_SERVICE_EVENTREPORT));
    para1.property_temperaturedata = 11.11;
    para1.property_humiditydata = 22.22;
    para1.property_motordata = 1;

    if(ctiot_mqtt_isconnected(g_phandle) == false)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mqtt device is not connected");
        return;
    }

    ret = ctiot_mqtt_encode_event_report_service_eventreport(&para1, &payload);
    CTIOT_LOG(LOG_INFO, "ctiot mqtt encode event report message result:%d", ret);
    if (ret != CTIOT_SUCCESS)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mqtt encode event report message failed");
        return;
    }

    ret = ctiot_mqtt_msg_publish("service_eventreport", MQTT_QOS_MOST_ONCE, payload);
    OS_PUT_MEM(payload);
    CTIOT_LOG(LOG_INFO, "ctiot mqtt publish event report message result: %d", ret);
}
#endif

static void ctiot_mqtt_data_upload(void)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    char * payload = NULL;
    DATA_REPORT_SERVICE_DATAREPORT para1;

    memset(&para1, 0, sizeof(DATA_REPORT_SERVICE_DATAREPORT));
    para1.property_temperaturedata = rand() % 40;
    para1.property_humiditydata = rand() % 99;
    para1.property_motordata = 1;

    if(ctiot_mqtt_isconnected(g_phandle) == false)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mqtt device is not connected");
        return;
    }

    ret = ctiot_mqtt_encode_data_report_service_datareport(&para1, &payload);
    CTIOT_LOG(LOG_INFO, "ctiot mqtt encode data report message result:%d", ret);
    if (ret != CTIOT_SUCCESS)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mqtt encode data report message failed");
        return;
    }

    ret = ctiot_mqtt_msg_publish("service_datareport", MQTT_QOS_MOST_ONCE, payload);
    OS_PUT_MEM(payload);
    CTIOT_LOG(LOG_INFO, "ctiot mqtt publish data report message result : %d", ret);
}

#ifdef NEED_DN_DATA
static void ctiot_mqtt_cmd_dn_service_cmddn(void *p)
{
#if (DEVICE_TRANSPARENT == 1 )
    char *payload = (char *)p;
    
    CTIOT_LOG(LOG_INFO, "ctiot mqtt transparent recv service cmd message: %s", payload);
#else
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    CMD_DN_SERVICE_CMDDN *para = (CMD_DN_SERVICE_CMDDN *)p;
    char* payload = NULL;
    CMD_RESPONSE_SERVICE_CMDDNRESPONSE para1;
	
    memset(&para1, 0, sizeof(CMD_RESPONSE_SERVICE_CMDDNRESPONSE));
    para1.taskId = para->taskId;
    para1.property_temperaturedata = 33.33;
    para1.property_humiditydata = 44.44;
    para1.property_motordata = 1;

    ret = ctiot_mqtt_encode_cmd_response_service_cmddnresponse(&para1, &payload);
    CTIOT_LOG(LOG_INFO, "ctiot mqtt encode service cmd response message result: %d", ret);
    if (ret != CTIOT_SUCCESS)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mqtt encode service cmd response message failed");
        return;
    }
    ret = ctiot_mqtt_msg_publish("service_cmddnresponse", MQTT_QOS_LEAST_ONCE, payload);
    OS_PUT_MEM(payload);
    CTIOT_LOG(LOG_INFO, "ctiot mqtt publish service cmd response message result: %d", ret);
#endif
}
#endif

static void ctiot_mqtt_demo_entry(void)
{
    int ret = CTIOT_SUCCESS;
    mqtt_param_s ctiot_params;
    mqtt_device_info_s device_info;
    int count = 0;
    to_stop = 0;

#ifdef CTIOT_MQTT_USING_TLS
    ctiot_params.server_ip = DEVICE_TLSADDRESS;
    ctiot_params.server_port = DEVICE_TLSPORT;
    ctiot_params.info.security_type = MQTT_SECURITY_TYPE_CA;
    ctiot_params.info.u.ca.ca_crt = ctiot_root_ca_cert;
    ctiot_params.info.u.ca.ca_len = strlen(ctiot_root_ca_cert);
#else
    ctiot_params.server_ip = DEVICE_TCPADDRESS;
    ctiot_params.server_port = DEVICE_TCPPORT;
    ctiot_params.info.security_type = MQTT_SECURITY_TYPE_NONE;
#endif

#ifdef NEED_DN_DATA
    ctiotCbFunc.ctiot_mqtt_cmd_dn_service_cmddn = ctiot_mqtt_cmd_dn_service_cmddn;

    if (CTIOT_OK != ctiot_mqtt_init(&ctiot_params, &ctiotCbFunc, &g_phandle))
#else
    if (CTIOT_OK != ctiot_mqtt_init(&ctiot_params, NULL, &g_phandle))
#endif
    {
        return;
    }

    device_info.codec_mode = MQTT_CODEC_MODE_JSON;
    device_info.connection_type = MQTT_STATIC_CONNECT;
    device_info.password = DEVICE_TOKEN;
    device_info.u.s_info.deviceid = DEVICE_ID;
    CTIOT_LOG(LOG_INFO, "deviceID: %s, password: %s", device_info.u.s_info.deviceid, device_info.password);

    if (ctiot_mqtt_login(&device_info, g_phandle) != CTIOT_OK)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mqtt login failed");
        return;
    }

#if (DEVICE_TRANSPARENT == 0)
    ctiot_mqtt_event_upload();
#endif

    while(!to_stop)
    {
        count++;
        ret = ctiot_handleMessagr(g_phandle);
        if (ret == CTIOT_SUCCESS)
        {
            if (count%10 == 0)
            {
                ctiot_mqtt_data_upload();
            }
        }
        else
        {
            break;
        }
    }

    ctiot_mqtt_logout(g_phandle);
}

static void ctiot_mqtt_demo_task_func(void *arg)
{
#ifdef OS_USING_SHELL
    CTIOT_LOG(LOG_INFO, "Memory used before");
    sh_exec("show_mem");
#endif


    CTIOT_LOG(LOG_INFO, "ctiot mqtt demo start!");
    if (ctiot_init_heap(1024, 30) != 0)
    {
        CTIOT_LOG(LOG_ERR, "ctiot mqtt init heap failed, demo terminated");
        return;
    }
    ctiot_mqtt_demo_entry();
    ctiot_free_all_heap();


    OS_SLEEP(1000);
#ifdef OS_USING_SHELL
    CTIOT_LOG(LOG_INFO, "Memory used after");
    sh_exec("show_mem");
#endif
}

#ifdef CTIOT_MQTT_USING_TLS
#define CTIOT_MQTT_DEMO_TASK_STACK_SIZE 8192
#else
#define CTIOT_MQTT_DEMO_TASK_STACK_SIZE 4096
#endif

CTIOT_Thread_t ctiot_mqtt_demo_task = NULL;
static void ctiot_mqtt_demo_start(void)
{
    ctiot_thread_create(&ctiot_mqtt_demo_task,
                        ctiot_mqtt_demo_task_func,
                        OS_NULL,
                        OS_TASK_PRIORITY_MAX / 2,
                        CTIOT_MQTT_DEMO_TASK_STACK_SIZE);
}

static void ctiot_mqtt_demo_end(void)
{
    to_stop = 1;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(ctiot_mqtt_demo_start, ctiot_mqtt_demo_start, "ctiot mqtt device connect entry");
SH_CMD_EXPORT(ctiot_mqtt_demo_end, ctiot_mqtt_demo_end, "end ctiot mqtt demo");
#endif /* OS_USING_SHELL */
