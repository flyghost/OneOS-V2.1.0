/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "coap_api.h"
#include "wrappers.h"
#include "os_task.h"
#define DBG_EXT_TAG "ali.coap.demo"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"

OS_USED static char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = PKG_USING_ALI_IOTKIT_PRODUCT_KEY;
OS_USED static char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = PKG_USING_ALI_IOTKIT_PRODUCT_SECRET;
OS_USED static char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = PKG_USING_ALI_IOTKIT_DEVICE_NAME;
OS_USED static char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = PKG_USING_ALI_IOTKIT_DEVICE_SECRET;

#define IOTX_PRE_NOSEC_SERVER_URI "coap://pre.coap.cn-shanghai.link.aliyuncs.com:5683"
#define IOTX_PRE_PSK_SERVER_URI   "coap-psk://pre.coap.cn-shanghai.link.aliyuncs.com:5683"

/* online url */
#define IOTX_ONLINE_PSK_SERVER_URL "coap-psk://%s.coap.cn-shanghai.link.aliyuncs.com:5682"

char m_coap_client_running = 1;
char m_coap_reconnect      = 0;

static void iotx_response_handler(void *arg, void *p_response)
{
    int                   len       = 0;
    unsigned char *       p_payload = NULL;
    iotx_coap_resp_code_t resp_code;
    IOT_CoAP_GetMessageCode(p_response, &resp_code);
    IOT_CoAP_GetMessagePayload(p_response, &p_payload, &len);
    LOG_I(DBG_EXT_TAG, "Message response code: 0x%x\r\n", resp_code);
    LOG_I(DBG_EXT_TAG, "Len: %d, Payload: %s\r\n", len, p_payload);
}

static void iotx_post_data_to_server(void *param)
{
    char                 path[IOTX_URI_MAX_LEN + 1] = {0};
    iotx_coap_context_t *p_ctx                      = (iotx_coap_context_t *)param;
    iotx_message_t       message;

    HAL_Snprintf(path, IOTX_URI_MAX_LEN, "/topic/%s/%s/user/update/", g_product_key, g_device_name);

    memset(&message, 0, sizeof(iotx_message_t));
    message.p_payload     = (unsigned char *)"{\"name\":\"hello world\"}";
    message.payload_len   = strlen("{\"name\":\"hello world\"}");
    message.resp_callback = iotx_response_handler;
    message.msg_type      = IOTX_MESSAGE_CON;
    message.content_type  = IOTX_CONTENT_TYPE_JSON;

    IOT_CoAP_SendMessage(p_ctx, path, &message);
}

static int iotx_get_devinfo(iotx_coap_device_info_t *p_devinfo)
{
    if (NULL == p_devinfo)
    {
        return IOTX_ERR_INVALID_PARAM;
    }

    memset(p_devinfo, 0x00, sizeof(iotx_coap_device_info_t));

    /* get device info */
    memcpy(p_devinfo->product_key, g_product_key, strlen(g_product_key));
    memcpy(p_devinfo->device_name, g_device_name, strlen(g_device_name));
    memcpy(p_devinfo->device_secret, g_device_secret, strlen(g_device_secret));
    memset(p_devinfo->device_id, 0, IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 2);
    HAL_Snprintf(p_devinfo->device_id,
                 IOTX_PRODUCT_KEY_LEN + IOTX_DEVICE_NAME_LEN + 2,
                 "%s.%s",
                 p_devinfo->product_key,
                 p_devinfo->device_name);

    LOG_I(DBG_EXT_TAG, "The Product Key  : %s\r\n", p_devinfo->product_key);
    LOG_I(DBG_EXT_TAG, "The Device Name  : %s\r\n", p_devinfo->device_name);
    LOG_I(DBG_EXT_TAG, "The Device Secret: %s\r\n", p_devinfo->device_secret);
    LOG_I(DBG_EXT_TAG, "The Device ID    : %s\r\n", p_devinfo->device_id);
    return IOTX_SUCCESS;
}

void coap_example_main(void *param)
{
    int                     count = 0;
    iotx_coap_config_t      config;
    iotx_coap_device_info_t deviceinfo;

    /* set device info use HAL function */
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    IOT_Ioctl(IOTX_IOCTL_GET_DEVICE_SECRET, g_device_secret);

    IOT_SetLogLevel(IOT_LOG_DEBUG);

    LOG_I(DBG_EXT_TAG, "coap demo start\r\n");
    memset(&config, 0x00, sizeof(iotx_coap_config_t));

    char url[256] = {0};
    snprintf(url, sizeof(url), IOTX_ONLINE_PSK_SERVER_URL, g_product_key);
    config.p_url = url;

    iotx_get_devinfo(&deviceinfo);
    config.p_devinfo    = (iotx_coap_device_info_t *)&deviceinfo;
    config.wait_time_ms = 3000;

    iotx_coap_context_t *p_ctx = NULL;

reconnect:
    p_ctx = IOT_CoAP_Init(&config);
    if (NULL != p_ctx)
    {
        IOT_CoAP_DeviceNameAuth(p_ctx);
        do
        {
            if (IOT_TRUE == IOT_CoAP_Check_Auth(p_ctx) && (100 == count || 0 == count))
            {
                iotx_post_data_to_server((void *)p_ctx);
                count = 1;
            }
            count++;
            IOT_CoAP_Yield(p_ctx);
        } while (m_coap_client_running);

        IOT_CoAP_Deinit(&p_ctx);
    }
    else
    {
        LOG_E(DBG_EXT_TAG, "IoTx CoAP init failed\r\n");
    }
    if (m_coap_reconnect)
    {
        m_coap_reconnect = 0;
        goto reconnect;
    }

    IOT_SetLogLevel(IOT_LOG_NONE);
    LOG_I(DBG_EXT_TAG, "coap demo end\r\n");
    return;
}
static int coap_example_start(int argc, char *argv[])
{
    os_task_t *coap_test_hl = os_task_create("ali_coap_test", 
                                             coap_example_main, 
                                             NULL, 
                                             4096 * 2, 
                                             OS_TASK_PRIORITY_MAX / 2);

    if(NULL == coap_test_hl)
    {
        LOG_E(DBG_EXT_TAG, "create task error");  
        return -1;
    }
    else
    {    
        m_coap_client_running = 1;
        os_task_startup(coap_test_hl);
        return 0;
    }
}
static int coap_example_stop(int argc, char *argv[])
{
    m_coap_client_running = 0;
    return 0;
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(ali_coap_demo, coap_example_start, "ali coap demo start");
SH_CMD_EXPORT(ali_coap_demo_stop, coap_example_stop, "ali coap demo stop");
#endif /* OS_USING_SHELL */
