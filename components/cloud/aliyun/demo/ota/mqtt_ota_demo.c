/*
 * Copyright (C) 2015-2018 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "infra_compat.h"
#include "mqtt_api.h"
#include "ota_api.h"
#include "wrappers.h"
#include "os_task.h"
#define DBG_EXT_TAG "ali.ota.demo"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"

OS_USED static char g_product_key[IOTX_PRODUCT_KEY_LEN + 1]       = PKG_USING_ALI_IOTKIT_PRODUCT_KEY;
OS_USED static char g_product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = PKG_USING_ALI_IOTKIT_PRODUCT_SECRET;
OS_USED static char g_device_name[IOTX_DEVICE_NAME_LEN + 1]       = PKG_USING_ALI_IOTKIT_DEVICE_NAME;
OS_USED static char g_device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = PKG_USING_ALI_IOTKIT_DEVICE_SECRET;

#define OTA_MQTT_MSGLEN (2048)

OS_WEAK int HAL_ali_ota_write(char *buf, int len)
{
    return 1;
}

void event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    uintptr_t               packet_id  = (uintptr_t)msg->msg;
    iotx_mqtt_topic_info_pt topic_info = (iotx_mqtt_topic_info_pt)msg->msg;

    switch (msg->event_type)
    {
    case IOTX_MQTT_EVENT_UNDEF:
        LOG_I(DBG_EXT_TAG, "undefined event occur.");
        break;

    case IOTX_MQTT_EVENT_DISCONNECT:
        LOG_I(DBG_EXT_TAG, "MQTT disconnect.");
        break;

    case IOTX_MQTT_EVENT_RECONNECT:
        LOG_I(DBG_EXT_TAG, "MQTT reconnect.");
        break;

    case IOTX_MQTT_EVENT_SUBCRIBE_SUCCESS:
        LOG_I(DBG_EXT_TAG, "subscribe success, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_SUBCRIBE_TIMEOUT:
        LOG_I(DBG_EXT_TAG, "subscribe wait ack timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_SUBCRIBE_NACK:
        LOG_I(DBG_EXT_TAG, "subscribe nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_UNSUBCRIBE_SUCCESS:
        LOG_I(DBG_EXT_TAG, "unsubscribe success, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_UNSUBCRIBE_TIMEOUT:
        LOG_I(DBG_EXT_TAG, "unsubscribe timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_UNSUBCRIBE_NACK:
        LOG_I(DBG_EXT_TAG, "unsubscribe nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_SUCCESS:
        LOG_I(DBG_EXT_TAG, "publish success, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_TIMEOUT:
        LOG_I(DBG_EXT_TAG, "publish timeout, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_NACK:
        LOG_I(DBG_EXT_TAG, "publish nack, packet-id=%u", (unsigned int)packet_id);
        break;

    case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
        LOG_I(DBG_EXT_TAG, "topic message arrived but without any related handle: topic=%.*s, topic_msg=%.*s",
                  topic_info->topic_len,
                  topic_info->ptopic,
                  topic_info->payload_len,
                  topic_info->payload);
        break;

    default:
        LOG_I(DBG_EXT_TAG, "Should NOT arrive here.");
        break;
    }
}

static int _ota_mqtt_client(void)
{
#define OTA_BUF_LEN (128)

    int               rc = 0, ota_over = 0;
    void *            pclient = NULL, *h_ota = NULL;
    iotx_conn_info_pt pconn_info;
    iotx_mqtt_param_t mqtt_params;
    char *            msg_buf = NULL, *msg_readbuf = NULL;
    char              buf_ota[OTA_BUF_LEN];
    
    msg_buf = (char *)HAL_Malloc(OTA_MQTT_MSGLEN);
    if (NULL == msg_buf)
    {
        LOG_E(DBG_EXT_TAG, "not enough memory");
        rc = -1;
        goto do_exit;
    }
    
    msg_readbuf = (char *)HAL_Malloc(OTA_MQTT_MSGLEN);
    if (NULL == msg_readbuf)
    {
        LOG_E(DBG_EXT_TAG, "not enough memory");
        rc = -1;
        goto do_exit;
    }

#ifdef SUPPORT_TLS
    IOT_Ioctl(IOTX_IOCTL_SET_MQTT_DOMAIN, "x509.itls.cn-shanghai.aliyuncs.com");
    IOT_Ioctl(IOTX_IOCTL_SET_HTTP_DOMAIN, "iot-auth.cn-shanghai.aliyuncs.com");
#else
    IOT_Ioctl(IOTX_IOCTL_SET_PRODUCT_KEY, g_product_key);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_NAME, g_device_name);
    IOT_Ioctl(IOTX_IOCTL_SET_DEVICE_SECRET, g_device_secret);
#endif

    /* Device AUTH */
    if (0 != IOT_SetupConnInfo(g_product_key, g_device_name, g_device_secret, (void **)&pconn_info))
    {
        LOG_E(DBG_EXT_TAG, "AUTH request failed!");
        rc = -1;
        goto do_exit;
    }

    /* Initialize MQTT parameter */
    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.port      = pconn_info->port;
    mqtt_params.host      = pconn_info->host_name;
    mqtt_params.client_id = pconn_info->client_id;
    mqtt_params.username  = pconn_info->username;
    mqtt_params.password  = pconn_info->password;
    mqtt_params.pub_key   = pconn_info->pub_key;

    mqtt_params.request_timeout_ms    = 2000;
    mqtt_params.clean_session         = 0;
    mqtt_params.keepalive_interval_ms = 60000;
    mqtt_params.read_buf_size         = OTA_MQTT_MSGLEN;
    mqtt_params.write_buf_size        = OTA_MQTT_MSGLEN;

    mqtt_params.handle_event.h_fp     = event_handle;
    mqtt_params.handle_event.pcontext = NULL;

    /* Construct a MQTT client with specify parameter */
    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient)
    {
        LOG_E(DBG_EXT_TAG, "MQTT construct failed");
        rc = -1;
        goto do_exit;
    }
    h_ota = IOT_OTA_Init(g_product_key, g_device_name, pclient);
    if (NULL == h_ota)
    {
        rc = -1;
        LOG_E(DBG_EXT_TAG, "initialize OTA failed");
        goto do_exit;
    }

    /* if (0 != IOT_OTA_ReportVersion(h_ota, "iotx_ver_1.1.0")) { */
    /* rc = -1; */
    /* LOG_I("report OTA version failed"); */
    /* goto do_exit; */
    /* } */

    HAL_SleepMs(1000);

    do
    {
        uint32_t firmware_valid;
        LOG_I(DBG_EXT_TAG, "wait ota upgrade command....");

        /* handle the MQTT packet received from TCP or SSL connection */
        IOT_MQTT_Yield(pclient, 200);

        if (IOT_OTA_IsFetching(h_ota))
        {
            uint32_t last_percent = 0, percent = 0;
            char     md5sum[33];
            char     version[128] = {0};
            int      len, size_downloaded, size_file;
            do
            {

                len = IOT_OTA_FetchYield(h_ota, buf_ota, OTA_BUF_LEN, 1);
                LOG_I(DBG_EXT_TAG, "IOT_OTA_FetchYield result: %d", len);
                if (len > 0)
                {
                    if (1 != HAL_ali_ota_write(buf_ota, len))
                    {
                        LOG_I(DBG_EXT_TAG, "write data to file failed");
                        IOT_OTA_Ioctl(h_ota, IOT_OTAG_RESET_STATE, NULL, 0);
                        rc = -1;
                        break;
                    }
                    LOG_I(DBG_EXT_TAG, "write ota data....");
                }
                else
                {
                    HAL_SleepMs(500);
                    continue;
                }

                /* get OTA information */
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FETCHED_SIZE, &size_downloaded, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_FILE_SIZE, &size_file, 4);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_MD5SUM, md5sum, 33);
                IOT_OTA_Ioctl(h_ota, IOT_OTAG_VERSION, version, 128);

                last_percent = percent;
                percent      = (size_downloaded * 100) / size_file;
                if (percent - last_percent > 0)
                {
                    IOT_OTA_ReportProgress(h_ota, (IOT_OTA_Progress_t)percent, NULL);
                    IOT_OTA_ReportProgress(h_ota, (IOT_OTA_Progress_t)percent, "hello");
                }

                HAL_SleepMs(200);
                IOT_MQTT_Yield(pclient, 100);
            } while (!IOT_OTA_IsFetchFinish(h_ota));

            IOT_OTA_Ioctl(h_ota, IOT_OTAG_CHECK_FIRMWARE, &firmware_valid, 4);
            if (0 == firmware_valid)
            {
                LOG_E(DBG_EXT_TAG, "The firmware is invalid");
            }
            else
            {
                LOG_I(DBG_EXT_TAG, "The firmware is valid");
            }

            ota_over = 1;
        }
        HAL_SleepMs(2000);
    } while (!ota_over);

    HAL_SleepMs(200);

do_exit:

    if (NULL != h_ota)
    {
        IOT_OTA_Deinit(h_ota);
    }

    if (NULL != pclient)
    {
        IOT_MQTT_Destroy(&pclient);
    }

    if (NULL != msg_buf)
    {
        HAL_Free(msg_buf);
    }

    if (NULL != msg_readbuf)
    {
        HAL_Free(msg_readbuf);
    }

    return rc;
}

static void ota_example_main(void *param)
{
    LOG_I(DBG_EXT_TAG, "ota demo start");
    IOT_SetLogLevel(IOT_LOG_DEBUG);

    _ota_mqtt_client();

    IOT_DumpMemoryStats(IOT_LOG_DEBUG);

    LOG_I(DBG_EXT_TAG, "ota demo stop");
    return;
}

static int ota_example_start(int argc, char *argv[])
{
    os_task_t *mqtt_test_hl = os_task_create("ali_ota_test", 
                                             ota_example_main, 
                                             NULL, 
                                             4096 * 2, 
                                             OS_TASK_PRIORITY_MAX / 2);
    
    if(NULL == mqtt_test_hl)
    {
        LOG_E(DBG_EXT_TAG, "create task error");  
        return -1;
    }
    else
    {    
        os_task_startup(mqtt_test_hl);
        return 0;
    }
}

#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(ali_ota_demo, ota_example_start, "ali mqtt ota demo start");
#endif /* OS_USING_SHELL */
