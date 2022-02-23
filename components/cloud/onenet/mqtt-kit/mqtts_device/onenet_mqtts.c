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
 * @file        onenet_mqtts.c
 * 
 * @brief       Supply functions to OneNET-MQTTS connect, subscribe & receive, publish message from queue and reconnect.  
 * 
 * @details     
 * 
 * @revision
 * Date         Author          Notes
 * 2020-06-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_types.h>
#include <os_mq.h>
#include <os_errno.h>
#include <os_task.h>
#include <os_assert.h>
#include <os_clock.h>
#include <os_memory.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include "MQTTClient.h"
#include "MQTTOneOS.h"
#include "oneos_config.h"
#include "token.h"
#include "onenet_mqtts.h"
#include "onenet_device_sample.h"
#include "ca_certificate.h"

#define DBG_EXT_TAG "onenet_mqtts"
#define DBG_EXT_LVL DBG_EXT_INFO
#include "dlog.h"

#ifndef U32_DIFF
#define U32_DIFF(a, b) (((a) >= (b)) ? ((a) - (b)) : (((a) + ((b) ^ 0xFFFFFFFF) + 1)))
#endif

#define ONENET_DEVICE_REGISTER_SERVER_IP      "api.heclouds.com" /*register server*/
#define ONENET_DEVICE_REGISTER_SERVER_PORT    80

#define ONENET_MQTTS_DEVICE_TLS_SERVER_IP     "mqttstls.heclouds.com" /*cloud TLS server*/
#define ONENET_MQTTS_DEVICE_TLS_SERVER_PORT   8883

#define ONENET_MQTTS_DEVICE_SERVER_IP         "mqtts.heclouds.com" /*cloud TCP server*/
#define ONENET_MQTTS_DEVICE_SERVER_PORT       1883

#define SUBSCRIBE_DEVICE_ALL_TOPIC            "$sys/%s/%s/#" /*all device topic*/ /*$sys/{pid}/{device-name}/#*/
#define PUBLISH_DATA_TOPIC                    "$sys/%s/%s/dp/post/json"          /*publish data*/
#define SUBSCRIBE_PUBDATA_ACCEPTED_TOPIC      "$sys/%s/%s/dp/post/json/accepted" /*onenet accepted publish data,       \
                                                                                 msg with id*/
#define SUBSCRIBE_PUBDATA_REJECTED_TOPIC      "$sys/%s/%s/dp/post/json/rejected" /*onenet rejected publish data,       \
                                                                                 msg with id and error code*/
#define SUBSCRIBE_PUBDATA_RESULT_TOPIC        "$sys/%s/%s/dp/post/json/+"        /*onenet publish data result,         \
                                                                                 accepted or rejected*/
#define SUBSCRIBE_CMD_TOPIC                   "$sys/%s/%s/cmd/#"                 /*onenet cmd, msg with cmd data*/
#define SUBSCRIBE_CMD_REQUEST_TOPIC           "$sys/%s/%s/cmd/request/+"         /*onenet cmd request*/
#define SUBSCRIBE_CMD_RESPONSE_RESULT_TOPIC   "$sys/%s/%s/cmd/response/+/+"      /*onenet cmd request device response  \
                                                                                 result*/
#define SUBSCRIBE_CMD_RESPONSE_ACCEPTED_TOPIC "$sys/%s/%s/cmd/response/%s/accepted" /*onenet cmd request device response\
                                                                                 success*/ /*{cmdid}/accepted*/
#define SUBSCRIBE_CMD_RESPONSE_REJECTED_TOPIC "$sys/%s/%s/cmd/response/%s/rejected" /*onenet cmd request device response\
                                                                                 failed*/  /*{cmdid}/rejected*/
#define PUBLISH_SUBCMD_TOPIC                  "$sys/%s/%s/cmd/response/%s"       /*acho onenet cmd request*/ /*response \
                                                                                 /{cmdid}*/
#define PUBLISH_IMAGE_GET_TOPIC               "$sys/%s/%s/image/get"             /*$sys/{pid}/{device-name}/image/get*/
#define PUBLISH_IMAGE_UPDATE_TOPIC            "$sys/%s/%s/image/update"          /*$sys/{pid}/{device-name}/image       \
                                                                                 /update*/
#define SUBSCRIBE_IMAGE_TOPIC                 "$sys/%s/%s/image/#"               /*rejected and accepted*/

#define MQTT_COMMAND_TIMEOUT                  5000 /*or 30000ms*/
#define MQTT_SEND_BUFF_SIZE                   1024
#define MQTT_READ_BUFF_SIZE                   1024
#define USER_MESSAGE_HANDLERS_NUM             5 /* must equal or less then MAX_MESSAGE_HANDLERS defined in MQTTClient.h*/
#define USER_MQTT_SUBTOPIC_LEN_MAX            128

typedef enum
{
    ONENET_STATE_RESET   = 0, /*disconnect*/
    ONENET_STATE_CONNECT = 1, /*connecting*/
    ONENET_STATE_SERVICE = 2, /*connected*/
} onenet_state_t;

/**
 ***********************************************************************************************************************
 * @struct      onenet_mqtts
 *      
 * @brief       Base structure of OneNET-MQTTS object
 ***********************************************************************************************************************
 */
typedef struct
{
    Network    network;
    MQTTClient client;
} onenet_mqtts_t;

typedef struct
{
    char     sub_topic[USER_MQTT_SUBTOPIC_LEN_MAX];
    enum QoS sub_qos;
    void (*callback)(MessageData *);
} subscribe_message_handlers_t;

/**
 ***********************************************************************************************************************
 * @struct      onenet_info
 *      
 * @brief       The information of OneNET-MQTTS object
 ***********************************************************************************************************************
 */
typedef struct
{
    char                         *ip;
    int                          port;
    char                         pro_id[10];
    char                         access_key[48];
    char                         dev_name[64 + 1];
    char                         dev_id[16];
    char                         key[48];
    unsigned int                 keepheart_interval;
    unsigned short               device_register; /*1:register complete  0:register incomplete*/
    subscribe_message_handlers_t subscribe_message_handlers[USER_MESSAGE_HANDLERS_NUM];
} onenet_info_t;

typedef struct register_network register_network_t;
struct register_network
{
    int  socket;
    int  (*read)(register_network_t *, unsigned char *, int, int);
    int  (*write)(register_network_t *, unsigned char *, int, int);
    void (*disconnect)(register_network_t *);
};

static int onenet_mqtts_client_publish(char *, char *);
unsigned char mqtt_sendbuf[MQTT_SEND_BUFF_SIZE];
unsigned char mqtt_readbuf[MQTT_READ_BUFF_SIZE];

unsigned char          g_mqtts_device_tostop = OS_FALSE;
onenet_mqtts_t         g_onenet_mqtts        = {0};
onenet_state_t         g_onenet_state        = ONENET_STATE_RESET;
MQTTPacket_connectData g_mqtt_connect_data   = MQTTPacket_connectData_initializer;
#ifdef ONENET_MQTTS_USING_TLS
onenet_info_t g_onenet_info = {ONENET_MQTTS_DEVICE_TLS_SERVER_IP, 
                              ONENET_MQTTS_DEVICE_TLS_SERVER_PORT, 
                              "", "", "", "", "", 0, 0};
#else
onenet_info_t g_onenet_info = {ONENET_MQTTS_DEVICE_SERVER_IP, 
                              ONENET_MQTTS_DEVICE_SERVER_PORT, 
                              "", "", "", "", "", 0, 0};
#endif

struct os_mq mqtts_mq;
os_uint8_t   mqtts_msg_pool[MESSAGE_QUEUE_POOL_SIZE];

/**
 ***********************************************************************************************************************
 * @brief           Main interface for call event callback function of the OneNET-MQTTS. 
 * 
 * @details         
 * 
 * @attention       User could register themselves callback functions with vary events in this function.
 * 
 * @param[in]       onenet_event    The OneNET-MQTTS events, enum type defined in "onenet_mqtts.h".
 * 
 * @return          No Return.         
 ***********************************************************************************************************************
 */
void onenet_event_callback(onenet_event_t onenet_event)
{
    switch (onenet_event)
    {
    case ONENET_EVENT_START:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_start!");
        break;
    case ONENET_EVENT_DEVICE_REGISTER_OK:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_device_register_ok!");
        break;
    case ONENET_EVENT_DEVICE_REGISTER_FAIL:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_device_register_fail!");
        break;
    case ONENET_EVENT_MQTTS_DEVICE_CONNECTTING:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_mqtts_device_connectting!");
        break;
    case ONENET_EVENT_MQTTS_DEVICE_CONNECT_SUCCESS:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_mqtts_device_connect_success!");
        break;
    case ONENET_EVENT_MQTTS_DEVICE_CONNECT_FAIL:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_mqtts_device_connect_fail!");
        break;
    case ONENET_EVENT_MQTTS_DEVICE_DISCONNECT:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_mqtts_device_disconnect!");
        break;
    case ONENET_EVENT_KEEP_HEARTBEAT_SUCCESS:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_keep_heartbeat_success!");
        break;
    case ONENET_EVENT_SEND_DATA:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_send_data!");
        break;
    case ONENET_EVENT_SUBSCRIBE_SUCCESS:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_subscribe_success!");
        break;
    case ONENET_EVENT_SEND_UNSSUBSCRIBE:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_send_unsubscribe!");
        break;
    case ONENET_EVENT_PUBLISH_SUCCESS:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_publish_success!");
        break;
    case ONENET_EVENT_RECV_CMD:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_recv_cmd!");
        break;
    case ONENET_EVENT_CHECK_MQTTS_DEVICE_STATUS:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_check_mqtts_device_status!");
        break;
    case ONENET_EVENT_CHECK_NETWORK:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_check_network!");
        break;
    case ONENET_EVENT_FAULT_PROCESS:
        LOG_I(DBG_EXT_TAG, "onenet event callback: onenet_event_fault_process!");
        break;
    default:
        break;
    }
}

static void onenet_message_queue_init(void)
{
    os_err_t rc;

    rc = os_mq_init(&mqtts_mq,
                    "mqtts_mq",
                    &mqtts_msg_pool[0],
                    sizeof(mqtts_msg_pool),
                    sizeof(mq_msg_t));
    if (rc != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "init message queue failed.");
    }

    OS_ASSERT(OS_EOK == rc);
}

/**
 ***********************************************************************************************************************
 * @brief           This function will get work information of the device.
 *  
 * @details         
 * 
 * @attention       The information fill in "onenet_device_sample.h".
 * 
 * @param[]         None            
 *  
 * @return          The getting status.
 * @retval          OS_TRUE         Get information success.
 * @retval          OS_FALSE        Get information failure.
 ***********************************************************************************************************************
 */
int onenet_get_device_info(void)
{
    char pro_id[]     = USER_PRODUCT_ID;
    char access_key[] = USER_ACCESS_KEY;
    char dev_name[]   = USER_DEVICE_NAME;

    memset(g_onenet_info.pro_id, 0, sizeof(g_onenet_info.pro_id));
    strcpy(g_onenet_info.pro_id, pro_id);

    memset(g_onenet_info.access_key, 0, sizeof(g_onenet_info.access_key));
    strcpy(g_onenet_info.access_key, access_key);

    if (strlen(dev_name) > 0 && strlen(dev_name) < 65)
    {
        memset(g_onenet_info.dev_name, 0, sizeof(g_onenet_info.dev_name));
        strcpy(g_onenet_info.dev_name, dev_name);
    }
    else
    {
        return OS_FALSE;
    }

    g_onenet_info.keepheart_interval = USER_KEEPALIVE_INTERVAL;

#ifndef ONENET_MQTTS_USING_AUTO_REGISTER
    char dev_id[] = USER_DEVICE_ID;
    char key[]    = USER_KEY;

    memset(g_onenet_info.dev_id, 0, sizeof(g_onenet_info.dev_id));
    strcpy(g_onenet_info.dev_id, dev_id);
    memset(g_onenet_info.key, 0, sizeof(g_onenet_info.key));
    strcpy(g_onenet_info.key, key);
#endif

    return OS_TRUE;
}

static void submessage_arrived_default_handler(MessageData *data)
{
    os_kprintf("Recv submessage, Message arrived on topic %.*s: %.*s\r\n",
               data->topicName->lenstring.len,
               data->topicName->lenstring.data,
               data->message->payloadlen,
               (char *)data->message->payload);
}

static void submessage_pubdata_accepted_arrived_handler(MessageData *data)
{
    /* publish and save data to onenet success */
    os_kprintf("Recv submessage pubdata accepted, Message arrived on topic %.*s: %.*s\r\n",
               data->topicName->lenstring.len,
               data->topicName->lenstring.data,
               data->message->payloadlen,
               (char *)data->message->payload);
}

static void submessage_pubdata_rejected_arrived_handler(MessageData *data)
{
    /* publish and save data to onenet failed */
    os_kprintf("Recv submessage pubdata rejected, Message arrived on topic %.*s: %.*s\r\n",
               data->topicName->lenstring.len,
               data->topicName->lenstring.data,
               data->message->payloadlen,
               (char *)data->message->payload);
    /* find wrong, judge if need send again */
}

static void submessage_cmd_request_arrived_handler(MessageData *data)
{
    /* recv onenet cmd, user can design to do own thing */
    char  cmd_type = 0;
    char *data_ptr = NULL;
    int   i;

    os_kprintf("Recv submessage cmd request, Message arrived on topic %.*s: ",
               data->topicName->lenstring.len,
               data->topicName->lenstring.data);
    if ((data->message->payloadlen) > 0)
    {
        for (i = 0; i < (data->message->payloadlen); i++)
        {
            os_kprintf("0x%x ", *((char *)data->message->payload + i));
        }
        os_kprintf("\r\n");
    }

    /* response cmd first */
    data_ptr = strstr(data->topicName->lenstring.data, "request/"); /* recv onenet publish cmd, find cmdid */
    if (data_ptr)
    {
        char  cmd_resp_topic[128] = {0};
        char  cmdid[40]           = {0};
        char *cmd_resp_msg        = NULL;
        char  str[]               = "{OK}";

        data_ptr = strchr(data_ptr, '/');
        data_ptr++;
        memcpy(cmdid, data_ptr, 36);
        cmdid[36] = 0;
        snprintf(cmd_resp_topic,
                 sizeof(cmd_resp_topic),
                 PUBLISH_SUBCMD_TOPIC,
                 g_onenet_info.pro_id,
                 g_onenet_info.dev_name,
                 cmdid);
        cmd_resp_msg = str;

        onenet_mqtts_client_publish(cmd_resp_topic, cmd_resp_msg); /* response cmd */
    }

    cmd_type = *((char *)data->message->payload);
    switch (cmd_type)
    {
    /* do own thing here */
    case 0x01:
        os_kprintf("Recv submessage cmd: cmd_type=0x01\r\n");
        break;
    case 0x02:
        os_kprintf("Recv submessage cmd: cmd_type=0x02\r\n");
        break;
    case 0x03:
        os_kprintf("Recv submessage cmd: cmd_type=0x03\r\n");
        break;
    default:
        break;
    }
}

static void submessage_cmd_response_result_arrived_handler(MessageData *data)
{
    /* recv device cmd reponse result, success or failed */
    os_kprintf("Recv cmd response result, Message arrived on topic %.*s: %x\r\n",
               data->topicName->lenstring.len,
               data->topicName->lenstring.data,
               *((char *)data->message->payload));
}

static void submessage_image_arrived_handler(MessageData *data)
{
    /* about all image topic */
    os_kprintf("Recv submessage image, Message arrived on topic %.*s: %.*s\r\n",
               data->topicName->lenstring.len,
               data->topicName->lenstring.data,
               data->message->payloadlen,
               (char *)data->message->payload);
}

static void mqtt_subscribe_message_handlers_init(void)
{
    subscribe_message_handlers_t subscribe_message_handler_def[USER_MESSAGE_HANDLERS_NUM];
    char                         subtopic_buf[USER_MQTT_SUBTOPIC_LEN_MAX] = {0};
    int                          subtopic_buf_len                         = sizeof(subtopic_buf);

    /* subtopic must be right and subscribe_topic_def[0~USER_MESSAGE_HANDLERS_NUM][64] can not be NULL, \
       user can decrease num of USER_MESSAGE_HANDLERS_NUM according to own needs */
    memset(subscribe_message_handler_def, 0, sizeof(subscribe_message_handlers_t) * USER_MESSAGE_HANDLERS_NUM);

    strcpy(subscribe_message_handler_def[0].sub_topic, SUBSCRIBE_PUBDATA_ACCEPTED_TOPIC);
    subscribe_message_handler_def[0].sub_qos  = QOS1;
    subscribe_message_handler_def[0].callback = submessage_pubdata_accepted_arrived_handler;

    strcpy(subscribe_message_handler_def[1].sub_topic, SUBSCRIBE_PUBDATA_REJECTED_TOPIC);
    subscribe_message_handler_def[1].sub_qos  = QOS1;
    subscribe_message_handler_def[1].callback = submessage_pubdata_rejected_arrived_handler;

    strcpy(subscribe_message_handler_def[2].sub_topic, SUBSCRIBE_CMD_REQUEST_TOPIC);
    subscribe_message_handler_def[2].sub_qos  = QOS1;
    subscribe_message_handler_def[2].callback = submessage_cmd_request_arrived_handler;

    strcpy(subscribe_message_handler_def[3].sub_topic, SUBSCRIBE_CMD_RESPONSE_RESULT_TOPIC);
    subscribe_message_handler_def[3].sub_qos  = QOS1;
    subscribe_message_handler_def[3].callback = submessage_cmd_response_result_arrived_handler;

    strcpy(subscribe_message_handler_def[4].sub_topic, SUBSCRIBE_IMAGE_TOPIC);
    subscribe_message_handler_def[4].sub_qos  = QOS1;
    subscribe_message_handler_def[4].callback = submessage_image_arrived_handler;

    for (int i = 0; i < USER_MESSAGE_HANDLERS_NUM; i++)
    {
        memset(subtopic_buf, 0, subtopic_buf_len);
        snprintf(subtopic_buf,
                 subtopic_buf_len,
                 subscribe_message_handler_def[i].sub_topic,
                 g_onenet_info.pro_id,
                 g_onenet_info.dev_name);

        memset(g_onenet_info.subscribe_message_handlers[i].sub_topic,
               0,
               sizeof(g_onenet_info.subscribe_message_handlers[i].sub_topic));
        strncpy(&g_onenet_info.subscribe_message_handlers[i].sub_topic[0], subtopic_buf, subtopic_buf_len - 1);
        g_onenet_info.subscribe_message_handlers[i].sub_qos  = subscribe_message_handler_def[i].sub_qos;
        g_onenet_info.subscribe_message_handlers[i].callback = subscribe_message_handler_def[i].callback;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initialize a OneNET-MQTTS working process. 
 * 
 * @details         Include: Network initialization, Client initialization and Subscribe initialization.
 * 
 * @attention       
 * 
 * @param[]         None.
 * 
 * @return          No Return.         
 ***********************************************************************************************************************
 */
void onenet_mqtts_init(void)
{
    MQTTNetworkInit(&g_onenet_mqtts.network, g_onenet_info.ip, g_onenet_info.port, g_onenet_mqtts_ca_certificate);

    MQTTClientInit(&g_onenet_mqtts.client,
                   &g_onenet_mqtts.network,
                   MQTT_COMMAND_TIMEOUT,
                   mqtt_sendbuf,
                   MQTT_SEND_BUFF_SIZE,
                   mqtt_readbuf,
                   MQTT_READ_BUFF_SIZE);

    mqtt_subscribe_message_handlers_init();
}

static int set_onenet_state(onenet_state_t new_state)
{
    int ret = ERR;

    switch (new_state)
    {
    case ONENET_STATE_RESET:
        g_onenet_state = ONENET_STATE_RESET;
        ret            = OK;
        break;
    case ONENET_STATE_CONNECT:
        if (g_onenet_state == ONENET_STATE_RESET || g_onenet_state == ONENET_STATE_SERVICE)
        {
            g_onenet_state = ONENET_STATE_CONNECT;
            ret            = OK;
        }
        break;
    case ONENET_STATE_SERVICE:
        if (g_onenet_state == ONENET_STATE_CONNECT)
        {
            g_onenet_state = ONENET_STATE_SERVICE;
            ret            = OK;
        }
        break;
    default:
        break;
    }

    return ret;
}

#ifdef ONENET_MQTTS_USING_AUTO_REGISTER
static unsigned char get_device_resigter_state(void)
{
    return g_onenet_info.device_register;
}

static void set_device_resigter_state(void)
{
    g_onenet_info.device_register = 1;
}
#endif

static int register_network_connect(register_network_t *n, char *addr, int port)
{
    struct sockaddr_in sAddr;
    struct hostent    *host_entry = NULL;
    int                ret        = -1;

    if ((host_entry = gethostbyname(addr)) == NULL)
    {
        LOG_E(DBG_EXT_TAG, "dns parse error!");
        goto exit;
    }

    memset(&sAddr, 0, sizeof(struct sockaddr_in));
    sAddr.sin_family = AF_INET;
    sAddr.sin_port   = htons(port);
    sAddr.sin_addr   = *(struct in_addr *)host_entry->h_addr_list[0];

    if ((n->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        goto exit;

    if ((ret = connect(n->socket, (struct sockaddr *)&sAddr, sizeof(sAddr))) < 0)
    {
        closesocket(n->socket);
        n->socket = -1;
        goto exit;
    }

exit:
    return ret;
}

static int oneos_read(register_network_t *n, unsigned char *buffer, int len, int timeout_ms)
{
    os_tick_t      xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    os_tick_t      tick_pre     = 0;
    os_tick_t      tick_now     = 0;
    int            recvLen      = 0;
    struct timeval tv;
    int            rc = 0;
    tv.tv_sec  = 0;
    tv.tv_usec = timeout_ms * 1000;
    tick_pre = os_tick_get();

    do
    {
        tv.tv_sec  = 0;
        tv.tv_usec = xTicksToWait * (1000 / OS_TICK_PER_SECOND) * 1000;

        tick_now = os_tick_get();

        setsockopt(n->socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));
        rc = recv(n->socket, buffer, len, 0);
        if (-1 == rc)
        {
            if (errno != EINTR && errno != EWOULDBLOCK && errno != EAGAIN)
            {
                recvLen = -1;
                break;
            }
        }
        else if (0 == rc)
        {
            recvLen = -1;
            break;
        }
        else if (rc > 0)
        {
            recvLen = rc;
            break;
        }
    } 
    while (recvLen == 0 && (U32_DIFF(tick_now, tick_pre) < xTicksToWait));

    return recvLen;
}

static int oneos_write(register_network_t *n, unsigned char *buffer, int len, int timeout_ms)
{
    os_tick_t      xTicksToWait = os_tick_from_ms(timeout_ms); /* convert milliseconds to ticks */
    os_tick_t      tick_pre     = 0;
    os_tick_t      tick_now     = 0;
    struct timeval tv;
    int            sentLen = 0;
    int            rc      = 0;

    tv.tv_sec  = 0;
    tv.tv_usec = timeout_ms * 1000;
    tick_pre = os_tick_get();

    do
    {
        tick_now = os_tick_get();
        setsockopt(n->socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
        rc = send(n->socket, buffer + sentLen, len - sentLen, 0);
        if (rc > 0)
        {
            sentLen += rc;
        }
        else if (rc < 0)
        {
            sentLen = rc;
            break;
        }
    }
    while (sentLen < len && ((U32_DIFF(tick_now, tick_pre)) < xTicksToWait));

    return sentLen;
}

static void oneos_disconnect(register_network_t *n)
{
    if (-1 == n->socket)
    {
        LOG_E(DBG_EXT_TAG, "register_network n->socket = -1");
        return;
    }

    closesocket(n->socket);
    n->socket = -1;
    LOG_I(DBG_EXT_TAG, "register_network close success");
}

static void register_network_init(register_network_t *n)
{
    n->socket     = -1;
    n->read       = oneos_read;
    n->write      = oneos_write;
    n->disconnect = oneos_disconnect;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will get connect status of the device which connect to the OneNET with 
 *                  OneNET-MQTTS protocol.
 *  
 * @details         
 * 
 * @attention       
 * 
 * @param[]         None            
 *  
 * @return          The connect status.
 * @retval          1               Is connected.
 * @retval          0               No connect.
 ***********************************************************************************************************************
 */
int onenet_mqtts_device_is_connected(void)
{
    return MQTTIsConnected(&g_onenet_mqtts.client);
}

/**
 ***********************************************************************************************************************
 * @brief           This function will automatic register the device to OneNET with OneNET-MQTTS protocol. 
 * 
 * @details         The device connecting register sever, send register message, receive response message and 
 *                  disconnect with register sever.
 * 
 * @attention       Serial is a device name defined by user, need a unique identification, supports Numbers, letters, 
 *                  characters '_' and '-', not longer than 64.
 * 
 * @param[in]       access_key      Get in OneNET web, when User has registered with MQTT.
 * @param[in]       pro_id          Get in OneNET web, when User has registered with MQTT. 
 * @param[in]       serial          The device name, user defined.
 * @param[out]      dev_id          OneNET sever return automatic, identical with the OneNET web.
 * @param[out]      key             OneNET sever return automatic, identical with the OneNET web.
 * 
 * @return          Return register status.
 * @retval          OS_TRUE         Register success.
 * @retval          OS_FALSE        Register failed.
 ***********************************************************************************************************************
 */
int onenet_mqtts_device_register(const char *access_key,
                                 const char *pro_id, 
                                 const char *serial, 
                                 char       *dev_id, 
                                 char       *key)
{
    int                ret                    = OS_FALSE;
    register_network_t register_network;
    char               version[]              = "2018-10-31";
    unsigned int       expiration_time        = 1956499200; /* 2032-1-1 0:0:0 can set by user */
    char               authorization_buf[144] = {0};
    unsigned short     send_len               = 11 + strlen(serial);
    int                timeout                = 5;
    char              *send_ptr               = NULL; 
    char              *data_ptr               = NULL;
    char               recv_buf[1024]         = {0};
    int                rc                     = ERR;

    if (NULL == access_key || NULL == pro_id || NULL == serial || NULL == dev_id || NULL == key)
    {
        return OS_FALSE;
    }

    send_ptr = os_malloc(send_len + 240);
    if (NULL == send_ptr)
    {
        return OS_FALSE;
    }

    register_network_init(&register_network);
    rc = register_network_connect(&register_network,
                                  ONENET_DEVICE_REGISTER_SERVER_IP,
                                  ONENET_DEVICE_REGISTER_SERVER_PORT);
    if (ERR == rc)
    {
        onenet_event_callback(ONENET_EVENT_CHECK_NETWORK);
        LOG_E(DBG_EXT_TAG, "register server establish network failed");
        goto exit;
    }
    LOG_I(DBG_EXT_TAG, "register server establish network sucess");

    onenet_authorization(version,
                         (char *)pro_id,
                         expiration_time,
                         (char *)access_key,
                         NULL,
                         authorization_buf,
                         sizeof(authorization_buf),
                         1);

    snprintf(send_ptr,
             240 + send_len,
             "POST /mqtt/v1/devices/reg HTTP/1.1\r\n"
             "Authorization:%s\r\n"
             "Host:ota.heclouds.com\r\n"
             "Content-Type:application/json\r\n"
             "Content-Length:%d\r\n\r\n"
             "{\"name\":\"%s\"}",
             authorization_buf,
             11 + strlen(serial),
             serial);

    rc = register_network.write(&register_network, (unsigned char *)send_ptr, strlen(send_ptr), timeout * 1000);
    if (rc == strlen(send_ptr))
    {
        rc = register_network.read(&register_network, (unsigned char *)recv_buf, sizeof(recv_buf), timeout * 1000);
        if (rc > 0)
        {
            data_ptr = strstr(recv_buf, "device_id");
        }
        if (data_ptr)
        {
            char name[16];
            int  pid = 0;

            if (sscanf(data_ptr,
                       "device_id\" : \"%[^\"]\",\n    \"name\" : \"%[^\"]\",\n    \"pid\" : %d,\n    \"key\" : "
                       "\"%[^\"]\"",
                       dev_id,
                       name,
                       &pid,
                       key) == 4)
            {
                LOG_I(DBG_EXT_TAG, "create device: %s, %s, %d, %s", dev_id, name, pid, key);
                ret = OS_TRUE;
            }
        }
    }
    else
    {
        ret = OS_FALSE;
    }

    register_network.disconnect(&register_network);

exit:
    os_free(send_ptr);
    return ret;
}

static int onenet_mqtts_client_connect(const char *username, const char *password, const char *clientid)
{
    int connack_rc = 0;

    g_mqtt_connect_data.MQTTVersion       = 4;
    g_mqtt_connect_data.clientID.cstring  = (char *)clientid;
    g_mqtt_connect_data.keepAliveInterval = g_onenet_info.keepheart_interval;
    g_mqtt_connect_data.cleansession      = 1; /*must be 1*/
    g_mqtt_connect_data.willFlag          = 0; /*must be 0*/
    g_mqtt_connect_data.username.cstring  = (char *)username;
    g_mqtt_connect_data.password.cstring  = (char *)password;

    connack_rc = MQTTConnect(&g_onenet_mqtts.client, &g_mqtt_connect_data);
    switch (connack_rc)
    {
    case 0:
        LOG_I(DBG_EXT_TAG, "Tips: device connect to onenet success");
        return OS_TRUE;
    case 1:
        LOG_E(DBG_EXT_TAG, "WARN: device unable to connect: protocol wrong");
        break;
    case 2:
        LOG_E(DBG_EXT_TAG, "WARN: device unable to connect: illegal clientid");
        break;
    case 3:
        LOG_E(DBG_EXT_TAG, "WARN: device unable to connect: server wrong");
        break;
    case 4:
        LOG_E(DBG_EXT_TAG, "WARN: device unable to connect: username or password wrong");
        break;
    case 5:
        LOG_E(DBG_EXT_TAG, "WARN: device unable to connect: illegal link, such as illegag token");
        break;
    default:
        LOG_E(DBG_EXT_TAG, "ERR: device error connect.");
        break;
    }

    return OS_FALSE;
}

static int onenet_device_link(const char *dev_name, const char *pro_id, const char *key)
{
    char         version[]              = "2018-10-31";
    unsigned int expiration_time        = 1956499200;
    char         authorization_buf[160] = {0};

    if (NULL == dev_name || NULL == pro_id || NULL == key)
    {
        return OS_FALSE;
    }

    onenet_authorization(version,
                         (char *)pro_id,
                         expiration_time,
                         (char *)key,
                         (char *)dev_name,
                         authorization_buf,
                         sizeof(authorization_buf),
                         0);

    return onenet_mqtts_client_connect(pro_id, authorization_buf, dev_name);
}

/**
 ***********************************************************************************************************************
 * @brief           This function will let device connecting OneNET with OneNET-MQTTS protocol.
 *  
 * @details         Include network level and MQTT protocol level.
 * 
 * @attention       Network connecting failure will return immediately, MQTT protocol connecting failure will disconnt
 *                  the network before return.
 * 
 * @param[]         None            
 *  
 * @return          The connect status.
 * @retval          OS_TRUE         Device connect OneNET success.
 * @retval          OS_FALSE        Device connect OneNET failure.
 ***********************************************************************************************************************
 */
int onenet_mqtts_device_link(void)
{
    int rc  = 0;
    int ret = OS_FALSE;

    if (0 != MQTTIsConnected(&g_onenet_mqtts.client))
    {
        return OS_TRUE;
    }

    rc = g_onenet_mqtts.network.connect(&g_onenet_mqtts.network);
    if (ERR == rc)
    {
        LOG_E(DBG_EXT_TAG, "establish network failed, check IP and PORT");
        onenet_event_callback(ONENET_EVENT_CHECK_NETWORK);
        return OS_FALSE;
    }
    LOG_I(DBG_EXT_TAG, "establish network sucess");

    if (OS_TRUE == onenet_device_link(g_onenet_info.dev_name, g_onenet_info.pro_id, g_onenet_info.key))
    {
        onenet_event_callback(ONENET_EVENT_MQTTS_DEVICE_CONNECT_SUCCESS);
        ret = OS_TRUE;
    }
    else
    {
        g_onenet_mqtts.network.disconnect(&g_onenet_mqtts.network);
        LOG_I(DBG_EXT_TAG, "abolish network, will cycle again");
        onenet_event_callback(ONENET_EVENT_MQTTS_DEVICE_CONNECT_FAIL);
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will let device disconnect the OneNET.
 *  
 * @details         
 * 
 * @attention       Only disconnect MQTT protocol level.
 * 
 * @param[]         None            
 *  
 * @return          No return.
 ***********************************************************************************************************************
 */
void onenet_mqtts_device_disconnect(void)
{
    if (0 != MQTTIsConnected(&g_onenet_mqtts.client))
    {
        MQTTDisconnect(&g_onenet_mqtts.client);
    }
}

static int onenet_mqtts_client_subscribe(const char *topicString, enum QoS sub_qos, messageHandler submessage_handler)
{
    int rc        = ERR;
    int max_tries = 3;

    if (NULL == topicString)
    {
        return OS_FALSE;
    }

    if (NULL == submessage_handler)
    {
       submessage_handler = submessage_arrived_default_handler;
    }
 
    while (max_tries-- > 0)
    {
        rc = MQTTSubscribe(&g_onenet_mqtts.client, topicString, sub_qos, submessage_handler);
        if (rc != OK)
        {
            LOG_E(DBG_EXT_TAG, "mqtt subscribe %s failed", topicString);
        }
        else
        {
            break;
        }
    }

    if (rc == OK)
    {
        LOG_I(DBG_EXT_TAG, "mqtt subscribe %s success", topicString);
        return OS_TRUE;
    }
    else
    {
        if (0 != MQTTIsConnected(&g_onenet_mqtts.client))
        {
           MQTTDisconnect(&g_onenet_mqtts.client);
        }
        return OS_FALSE;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function will let device subscribe MQTT message from OneNET.
 *  
 * @details         The subscribe Topics initialized in onenet_mqtts_init(), and register callback function with Topics.
 * 
 * @attention       The max number of subscribe Topics was set 5.
 * 
 * @param[]         None            
 *  
 * @return          The subscribe result.
 * @retval          OS_TRUE         Subscribe success.
 * @retval          OS_FALSE        Subscribe failure.
 ***********************************************************************************************************************
 */
int onenet_mqtts_device_subscribe(void)
{
    int            ret                = OS_FALSE;
    int            i                  = 0;
    char          *subtopic_filter    = NULL;
    enum QoS       sub_qos            = QOS1;
    messageHandler submessage_handler = NULL;

    for (i = 0; i < USER_MESSAGE_HANDLERS_NUM; i++)
    {
        subtopic_filter    = g_onenet_info.subscribe_message_handlers[i].sub_topic;
        sub_qos            = g_onenet_info.subscribe_message_handlers[i].sub_qos;
        submessage_handler = g_onenet_info.subscribe_message_handlers[i].callback;

        if (OS_TRUE == onenet_mqtts_client_subscribe(subtopic_filter, sub_qos, submessage_handler))
        {
            ret = OS_TRUE;
        }
        else
        {
            return OS_FALSE;
        }
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will let device unsubscribe MQTT message from OneNET.
 *  
 * @details         
 * 
 * @attention       
 * 
 * @param[in]       topicFilter     The topic that should unsubscribe.      
 *  
 * @return          The unsubscribe result.
 * @retval          OS_TRUE         Unsubscribe success.
 * @retval          OS_FALSE        Unsubscribe failure.
 ***********************************************************************************************************************
 */
int onenet_mqtts_client_unsubscribe(const char *topicFilter)
{
    int rc = 0;

    rc = MQTTUnsubscribe(&g_onenet_mqtts.client, topicFilter);
    if (OK == rc)
    {
        LOG_I(DBG_EXT_TAG, "mqtt unsubscribe %s success", topicFilter);
        return OS_TRUE;
    }
    else
    {
        LOG_E(DBG_EXT_TAG, "mqtt unsubscribe %s fail", topicFilter);
        return OS_FALSE;
    }
}

static int onenet_mqtts_device_yield(void)
{
    int rc = ERR;

    rc = MQTTYield(&g_onenet_mqtts.client, 500);
    if (OK == rc)
    {
        return OS_TRUE;
    }
    else
    {
        return OS_FALSE; /* here mqtts device has been disconnected */
    }
}

static int onenet_mqtts_client_publish(char *pubtopic, char *pub_msg)
{
    MQTTMessage message;
    int         rc = ERR;

    if (NULL == pub_msg)
    {
        return OS_FALSE;
    }

    message.qos        = QOS1; /*onenet publish must be QOS0 or QOS1*/
    message.retained   = 0;
    message.dup        = 0;
    message.payload    = pub_msg;
    message.payloadlen = strlen(pub_msg);
    rc                 = MQTTPublish(&g_onenet_mqtts.client, pubtopic, &message);
    if (OK != rc)
    {
        LOG_E(DBG_EXT_TAG, "mqtt publish error");
        return OS_FALSE;
    }
    LOG_I(DBG_EXT_TAG, "send mqtt success: %s", pub_msg);

    return OS_TRUE;
}


/* method 1: publish data to onenet periodically, details in onenet_mqtts_device_entry() */
#if 0
static int onenet_mqtts_device_publish_cycle(os_tick_t *last_publish_tick)
{
    int         ret                        = OS_TRUE;
    os_tick_t   tick_now                   = 0;
    os_tick_t   tick_pre                   = 0;
    os_tick_t   tick_diff                  = 0;
    os_uint32_t publish_timeout            = USER_PUBLISH_INTERVAL;
    char       *pro_id                     = USER_PRODUCT_ID;
    char       *dev_name                   = USER_DEVICE_NAME;
    char        pubtopic_buf[64]           = {0};
    int         pubtopic_buf_len           = sizeof(pubtopic_buf);
    char       *pubtopic_filter            = NULL;
    char        pub_buf[PUB_DATA_BUFF_LEN] = {0};
    static int  data_id                    = 0;
    int         temperature_value          = 0;
    int         power_value                = 0;
    char       *publish_message_str        = NULL;
    /*char str[]="{\"id\":123,\"dp\":{\"temperature\":[{\"v\":50,}],\"power\":[{\"v\":6,}]}}";*/

    snprintf(pubtopic_buf, pubtopic_buf_len, PUBLISH_DATA_TOPIC, pro_id, dev_name);
    pubtopic_filter = pubtopic_buf;

    if (NULL == last_publish_tick)
    {
        return OS_FALSE;
    }

    tick_now  =  os_tick_get();
    tick_pre  = *last_publish_tick;
    tick_diff =  U32_DIFF(tick_now, tick_pre);

    if (tick_diff > os_tick_from_ms(publish_timeout * 1000))
    {
        if (data_id != 2147483647)
        {
            data_id++;
        }
        else
        {
            data_id = 1;
        }
        temperature_value = rand() % 40;
        power_value       = rand() % 99;
        snprintf(pub_buf, sizeof(pub_buf), base_dp_upload_str, data_id, temperature_value, power_value);

        publish_message_str = pub_buf;
        if (NULL == publish_message_str)
        {
            return OS_FALSE;
        }

        if (OS_TRUE == onenet_mqtts_client_publish(pubtopic_filter, publish_message_str))
        {
            *last_publish_tick = tick_now;
            onenet_event_callback(ONENET_EVENT_PUBLISH_SUCCESS);
            ret = OS_TRUE;
        }
        else
        {
            /* here mqtts device has been disconnected */
            onenet_event_callback(ONENET_EVENT_MQTTS_DEVICE_DISCONNECT);
            ret = OS_FALSE;
        }
    }

    return ret;
}
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function will let device publish MQTT message to OneNET.
 *  
 * @details         The publish message obtained from message queue which need initialized before publish process.
 * 
 * @attention       The QOS of message is QOS1, OneNET supported QOS0,QOS1.
 * 
 * @param[]         None.      
 *  
 * @return          The publish result.
 * @retval          OS_TRUE         Publish success.
 * @retval          OS_FALSE        Publish failure.
 ***********************************************************************************************************************
 */
int onenet_mqtts_device_publish(void)
{
    int       ret                          = OS_TRUE;
    char     *pro_id                       = USER_PRODUCT_ID;
    char     *dev_name                     = USER_DEVICE_NAME;
    char      pubtopic_buf[64]             = {0};
    int       pubtopic_buf_len             = sizeof(pubtopic_buf);
    char     *pubtopic_filter              = NULL;
    char      pubmsg_buf[PUB_MSG_BUFF_LEN] = {0};
    mq_msg_t  mq_msg;
    os_size_t recv_len = sizeof(mq_msg);

    memset(&mq_msg, 0x00, sizeof(mq_msg));
    if (os_mq_recv(&mqtts_mq, (void *)&mq_msg, sizeof(mq_msg_t), OS_NO_WAIT, &recv_len) !=
        OS_EOK) /* get data from messagequeue, without wait*/
    {
        return OS_TRUE;
    }

    switch (mq_msg.topic_type)
    {
    case DATA_POINT_TOPIC:
        snprintf(pubtopic_buf, pubtopic_buf_len, PUBLISH_DATA_TOPIC, pro_id, dev_name);
        memcpy(pubmsg_buf, mq_msg.data_buf, mq_msg.data_len);
        break;
    case DEVICE_IMAGE_GET_TOPIC:
        snprintf(pubtopic_buf, pubtopic_buf_len, PUBLISH_IMAGE_GET_TOPIC, pro_id, dev_name);
        memset(pubmsg_buf, 0, sizeof(pubmsg_buf));
        break;
    case DEVICE_IMAGE_UPDATE_TOPIC:
        snprintf(pubtopic_buf, pubtopic_buf_len, PUBLISH_IMAGE_UPDATE_TOPIC, pro_id, dev_name);
        memcpy(pubmsg_buf, mq_msg.data_buf, mq_msg.data_len);
        break;
    default:
        break;
    }
    pubtopic_filter = pubtopic_buf;

    if (OS_TRUE == onenet_mqtts_client_publish(pubtopic_filter, pubmsg_buf))
    {
        onenet_event_callback(ONENET_EVENT_PUBLISH_SUCCESS);
        ret = OS_TRUE;
    }
    else
    {
        ret = OS_FALSE;
    }

    return ret;
}

static int onenet_mqtts_device_entry(void)
{
    char      device_subscribe_flag = 0;
    g_mqtts_device_tostop = OS_FALSE;
/*  os_tick_t last_publish_tick = 0;*/ /* method 1: publish data to onenet periodically, 
                                       use in "onenet_mqtts_device_publish_cycle()" */

    if (OS_TRUE != onenet_get_device_info())
    {
        onenet_event_callback(ONENET_EVENT_FAULT_PROCESS);
        return OS_FALSE;
    }
    else
    {
        onenet_event_callback(ONENET_EVENT_START);
    }

    while (!g_mqtts_device_tostop)
    {
        switch (g_onenet_state)
        {
        case ONENET_STATE_RESET:
            onenet_mqtts_init();
            onenet_message_queue_init(); /* create mqtts_message_queue */
            set_onenet_state(ONENET_STATE_CONNECT);

            break;

        case ONENET_STATE_CONNECT:
#ifdef ONENET_MQTTS_USING_AUTO_REGISTER
            if (OS_FALSE == get_device_resigter_state())
            {
                if (OS_FALSE == onenet_mqtts_device_register(g_onenet_info.access_key,
                                                             g_onenet_info.pro_id,
                                                             g_onenet_info.dev_name,
                                                             g_onenet_info.dev_id,
                                                             g_onenet_info.key))
                {
                    onenet_event_callback(ONENET_EVENT_DEVICE_REGISTER_FAIL);
                }
                else
                {
                    set_device_resigter_state();
                    onenet_event_callback(ONENET_EVENT_DEVICE_REGISTER_OK);
                    continue;
                }
            }
            else
            {
                if (OS_TRUE == onenet_mqtts_device_link())
                {
                    set_onenet_state(ONENET_STATE_SERVICE);
/*                  last_publish_tick = os_tick_get();*/  /* method 1: publish data to onenet periodically, use in
                                                          "onenet_mqtts_device_publish_cycle()" */
                    device_subscribe_flag = 0;
                    continue;
                }
            }
#else
            if (OS_TRUE == onenet_mqtts_device_link())
            {
                set_onenet_state(ONENET_STATE_SERVICE);
/*              last_publish_tick = os_tick_get();*/  /* method 1: publish data to onenet periodically, use in
                                                      "onenet_mqtts_device_publish_cycle()" */
                device_subscribe_flag = 0;
                continue;
            }
#endif
            break;

        case ONENET_STATE_SERVICE:
            if (OS_TRUE == onenet_mqtts_device_is_connected())
            {
                /* do the subscribe onenet topic once after start or restart mqtts device */
                if (0 == device_subscribe_flag)
                {
                    if (OS_TRUE == onenet_mqtts_device_subscribe())
                    {
                        device_subscribe_flag = 1;
                        onenet_event_callback(ONENET_EVENT_SUBSCRIBE_SUCCESS);
                    }
                    else
                    {
                        /* here mqtts device has been disconnected */
                        continue;
                    }
                }

                /* method 1: publish data to onenet periodically */
                /* if (CM_FALSE == onenet_mqtts_device_publish_cycle(&last_publish_tick))*/ /* get data while publish */
                /*continue;*/

                /* method 2: publish data while mqtts_messagequeue is not empty*/
                if (OS_FALSE == onenet_mqtts_device_publish()) /* get data from messagequeue */
                {
                    continue;
                }

                /* receive_process and heartbeat send */
                if (OS_FALSE == onenet_mqtts_device_yield())
                {
                    continue;
                }
            }
            else
            {
                g_onenet_mqtts.network.disconnect(&g_onenet_mqtts.network);
                set_onenet_state(ONENET_STATE_CONNECT);
                os_task_msleep(5000);
            }
            break;

        default:
            break;
        }

        os_task_msleep(200);
    }

    onenet_mqtts_device_disconnect();
    g_onenet_mqtts.network.disconnect(&g_onenet_mqtts.network);
    onenet_event_callback(ONENET_EVENT_MQTTS_DEVICE_DISCONNECT);

    return OK;
}

/**
 ***********************************************************************************************************************
 * @brief           The MSH input command, this function will exit a OneNET-MQTTS process.
 *  
 * @details         Reversed command with onenet_mqtts_device_start.
 * 
 *                  Use command：onenet_mqtts_device_end.
 * 
 * @attention       
 * 
 * @param[]         None.
 *  
 * @return          None.
 ***********************************************************************************************************************
 */
void onenet_mqtts_device_end(void)
{
    g_mqtts_device_tostop = OS_TRUE;
    os_task_msleep(3000);

    if (g_onenet_state == ONENET_STATE_RESET)
    {
        set_onenet_state(ONENET_STATE_RESET);
    }
    else
    {
        set_onenet_state(ONENET_STATE_CONNECT);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           The MSH input command, this function will send MQTT message into message queue that will publish 
 *                  in next step.
 *  
 * @details         The command have 3 parameters: 1.onenet_mqtts_publish;
 *                                                 2.0-data point
 *                                                   or 1-get mirror
 *                                                   or 2-update mirror;
 *                                                 3.the data will publish.
 *                  example: onenet_mqtts_publish 0 {"id":101,"dp":{"humi":[{"v":32,}],"temp":[{"v":25,}]}}
 * 
 * @attention       
 * 
 * @param[in]       argc            Number of transfer parameters, must be 3.
 * @param[in]       argv[]          Data of transfer parameters.      
 *  
 * @return          None.
 ***********************************************************************************************************************
 */
void onenet_mqtts_publish(int argc, char *argv[])
{
    /* input msg data must be json type, otherwise msg will be reject by onenet */
    if (argc != 3)
    {
        LOG_E(DBG_EXT_TAG, "Input topic type and message that you want to publish, length less than 59,\
                  example1: onenet_mqtts_publish 0 {\"id\":101,\"dp\":{\"humi\":[{\"v\":40,}],\"temp\":[{\"v\":25,}]}}");
        return;
    }

    os_err_t rc;
    char    *topic_type    = argv[1];
    char    *pub_msg       = argv[2];
    int      pubtopic_type = 0;
    int      pubmsg_len    = 0;
    mq_msg_t mq_msg;

    pubtopic_type = atoi(topic_type);
    pubmsg_len    = strlen(pub_msg);

    memset(&mq_msg, 0x00, sizeof(mq_msg));
    mq_msg.topic_type = pubtopic_type;
    memcpy(mq_msg.data_buf, pub_msg, pubmsg_len);
    mq_msg.data_len = pubmsg_len;

    if (OS_FALSE == onenet_mqtts_device_is_connected())
    {
        LOG_E(DBG_EXT_TAG, "onenet mqtts device is disconnected.");
        return;
    }

    rc = os_mq_send(&mqtts_mq, (void *)&mq_msg, sizeof(mq_msg_t), 0);
    if (rc != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG, "mqtts_device_messagequeue_send ERR");
    }
}

static void onenet_mqtts_device_thread_func(void *arg)
{
    onenet_mqtts_device_entry();
}

/**
 ***********************************************************************************************************************
 * @brief           The MSH input command, this function will starting a OneNET-MQTTS process.
 *  
 * @details         Include: MQTT task initialization, network, MQTT client, publish message queue initialization,
 *                  device register, network, client connect, specifical topics subscribe, topics publish and 
 *                  keeping HeartBeat.
 * 
 *                  Use command：onenet_mqtts_device_start
 * 
 * @attention       
 * 
 * @param[]         None.
 *  
 * @return          None.
 ***********************************************************************************************************************
 */
#define ONENET_MQTTS_DEVICE_THREAD_STACK_SIZE 8192
os_task_t *onenet_mqtts_device_thread = NULL;
void onenet_mqtts_device_start(void)
{
    onenet_mqtts_device_thread = os_task_create("onenet_mqtts_device",
                                                onenet_mqtts_device_thread_func,
                                                OS_NULL,
                                                ONENET_MQTTS_DEVICE_THREAD_STACK_SIZE,
                                                OS_TASK_PRIORITY_MAX / 2);

    if (NULL == onenet_mqtts_device_thread)
    {
        LOG_E(DBG_EXT_TAG, "onenet mqtts device create thread failed");
        OS_ASSERT(OS_NULL != onenet_mqtts_device_thread);
    }

    os_task_startup(onenet_mqtts_device_thread);
}

#ifdef OS_USING_SHELL
#include <shell.h>

SH_CMD_EXPORT(onenet_mqtts_device_start, onenet_mqtts_device_start, "start onenet mqtts device");
SH_CMD_EXPORT(onenet_mqtts_device_end, onenet_mqtts_device_end, "end onenet mqtts device");
SH_CMD_EXPORT(onenet_mqtts_publish, onenet_mqtts_publish, "publish message to onenet specified topic");
#endif
