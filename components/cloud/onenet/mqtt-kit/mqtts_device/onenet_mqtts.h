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
 * @file        onenet_mqtts.h
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

#ifndef _ONENET_MQTTS_H_
#define _ONENET_MQTTS_H_

#define OK  0
#define ERR -1

#ifndef ONENET_MQTTS_PUBLISH_DATA_BUFFER_LENGTH
#define PUB_DATA_BUFF_LEN 128  /* publish payload buffer length */
#else
#define PUB_DATA_BUFF_LEN  ONENET_MQTTS_PUBLISH_DATA_BUFFER_LENGTH
#endif

#define PUB_MSG_BUFF_LEN PUB_DATA_BUFF_LEN

#define MESSAGE_QUEUE_POOL_SIZE ( 10 * sizeof(mq_msg_t) )

/**
 *********************************************************************************************************************** 
 * @enum        onenet_event
 * 
 * @brief       OneNET-MQTTS events type
 ***********************************************************************************************************************
 */
typedef enum
{
    ONENET_EVENT_START = 0,
    ONENET_EVENT_DEVICE_REGISTER_OK,
    ONENET_EVENT_DEVICE_REGISTER_FAIL,
    ONENET_EVENT_MQTTS_DEVICE_CONNECTTING,
    ONENET_EVENT_MQTTS_DEVICE_CONNECT_SUCCESS,
    ONENET_EVENT_MQTTS_DEVICE_CONNECT_FAIL,
    ONENET_EVENT_MQTTS_DEVICE_DISCONNECT,
    ONENET_EVENT_KEEP_HEARTBEAT_SUCCESS,
    ONENET_EVENT_SEND_DATA,
    ONENET_EVENT_SUBSCRIBE_SUCCESS,
    ONENET_EVENT_SEND_UNSSUBSCRIBE,
    ONENET_EVENT_PUBLISH_SUCCESS,
    ONENET_EVENT_RECV_CMD,
    ONENET_EVENT_CHECK_MQTTS_DEVICE_STATUS,
    ONENET_EVENT_CHECK_NETWORK,
    ONENET_EVENT_FAULT_PROCESS,
} onenet_event_t;

typedef enum
{
    DATA_POINT_TOPIC          = 0,
    DEVICE_IMAGE_GET_TOPIC    = 1,
    DEVICE_IMAGE_UPDATE_TOPIC = 2,
    CHILD_DEVICE_TOPIC        = 3,
} mqtts_pubtopic_type_t;

typedef struct
{
    int  topic_type;    /*0:data point, 1:device image get, 2:device image update, 3:child device*/
    char data_buf[PUB_DATA_BUFF_LEN]; /*publish data*/ /*can increase by user*/
    int  data_len;      /*publish data length*/
} mq_msg_t;

extern void onenet_event_callback(onenet_event_t);
extern int  onenet_get_device_info(void);
extern void onenet_mqtts_init(void);
extern int  onenet_mqtts_device_is_connected(void);
extern int  onenet_mqtts_device_register(const char *, const char *, const char *, char *, char *);
extern int  onenet_mqtts_device_link(void);
extern void onenet_mqtts_device_disconnect(void);
extern int  onenet_mqtts_device_subscribe(void);
extern int  onenet_mqtts_client_unsubscribe(const char *);
extern int  onenet_mqtts_device_publish(void);
extern void onenet_mqtts_publish(int, char *[]);
extern void onenet_mqtts_device_start(void);
extern void onenet_mqtts_device_end(void);

#endif /* _ONENET_MQTTS_H_ */
