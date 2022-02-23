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
 * @file        mo_mqttc.h
 *
 * @brief       module link kit mqtt client api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-04   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_MQTTC_H__
#define __MO_MQTTC_H__

#include "mo_type.h"
#include "mo_ipaddr.h"
#include "mo_object.h"

#include <os_mq.h>

#ifdef MOLINK_USING_MQTTC_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MQTTC_MAX_MESSAGE_HANDLERS
#define MQTTC_MAX_MESSAGE_HANDLERS (5) /* redefinable - how many subscriptions do you want? */
#endif

#define MQTTC_WILL_FALG_ENABLE  (1)
#define MQTTC_WILL_FALG_DISABLE (0)

/**
 ***********************************************************************************************************************
 * @enum        mqttc_stat_t
 *
 * @brief       molink mqtt client state
 ***********************************************************************************************************************
 */
typedef enum mqttc_stat
{
    MQTTC_STAT_NULL = 0,   /* The MQTT client has not been created */
    MQTTC_STAT_INIT,       /* The MQTT client was created but not connected */
    MQTTC_STAT_CONNECT,    /* The MQTT client connect OK*/
    MQTTC_STAT_DISCONNECT, /* The MQTT client was disconnected but not destroy */
    MQTTC_STAT_UNDEFINED   /* The MQTT client undefined status */
} mqttc_stat_t;

/**
 ***********************************************************************************************************************
 * @enum        mqttc_qos_t
 *
 * @brief       molink mqtt client QoS levels.
 ***********************************************************************************************************************
 */
typedef enum mqttc_qos
{
    MQTTC_QOS_0 = 0,     /* At most once delivery */
    MQTTC_QOS_1,         /* At least once delivery */
    MQTTC_QOS_2,         /* Exactly once delivery */
    MQTTC_QOS_UNDEFINED, /* must not be used */
} mqttc_qos_t;

/**
 ***********************************************************************************************************************
 * @struct      mqttc_string_t
 *
 * @brief       molink mqtt client string
 ***********************************************************************************************************************
 */
typedef struct mqttc_string
{
    char     *data;
    os_size_t len;
} mqttc_string_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_mqttc_will_opts_t
 *
 * @brief       molink mqtt client "Last Will and Testament" (LWT) options
 ***********************************************************************************************************************
 */
typedef struct mqttc_will_opts
{
    mqttc_string_t topic_name; /* The LWT topic to which the LWT message will be published */
    mqttc_string_t message;    /* The LWT payload */
    mqttc_qos_t    qos;        /* The quality of service setting for the LWT message */
    os_uint8_t     retained;   /* The retained flag for the LWT message */
} mqttc_will_opts_t;

/**
 ***********************************************************************************************************************
 * @struct      mqttc_conn_opts_t
 *
 * @brief       molink mqtt client connect options
 ***********************************************************************************************************************
 */
typedef struct mqttc_conn_opts
{
    mqttc_string_t client_id;    /* The MQTT client identifier string */

    os_uint8_t  mqtt_version;    /* The Version of MQTT to be used.  3 = 3.1 4 = 3.1.1 */
    os_uint32_t keep_alive;      /* The keep alive time, unit second */
    os_uint8_t  clean_session;   /* The flag of clean_session option */
    os_uint8_t  will_flag;       /* The flag of will option */

    mqttc_will_opts_t will_opts; /* The options of will */

    mqttc_string_t username;
    mqttc_string_t password;
} mqttc_conn_opts_t;

/**
 ***********************************************************************************************************************
 * @struct      mqttc_create_opts_t
 *
 * @brief       molink mqtt client create options
 ***********************************************************************************************************************
 */
typedef struct mqttc_create_opts
{
    mqttc_string_t address;         /* The MQTT server address */
    os_uint16_t    port;            /* The MQTT server port */
    os_uint32_t    command_timeout; /* The MQTT client ACK timeout, unit ms*/
    os_size_t      max_msgs;        /* The max number of messages.*/
} mqttc_create_opts_t;

/**
 ***********************************************************************************************************************
 * @struct      mqttc_msg_t
 *
 * @brief       molink mqtt client message
 ***********************************************************************************************************************
 */
typedef struct mqttc_msg
{
    mqttc_qos_t qos;       /* The quality of service setting for the message */

    os_uint8_t  retained;  /* The retained flag for the message */
    os_uint8_t  dup;       /* The dup flag for the message*/

    void     *payload;     /* The data of the message */ 
    os_size_t payload_len; /* The length of data */
} mqttc_msg_t;

/**
 ***********************************************************************************************************************
 * @struct      mqttc_msg_data_t
 *
 * @brief       molink mqtt client message data
 ***********************************************************************************************************************
 */
typedef struct mqttc_msg_data
{
    mqttc_msg_t    message;
    mqttc_string_t topic_name;
} mqttc_msg_data_t;

/**
 ***********************************************************************************************************************
 * @struct      mqttc_msg_handler_t
 *
 * @brief       molink mqtt client message handler
 ***********************************************************************************************************************
 */
typedef void (*mqttc_msg_handler_t)(mqttc_msg_data_t*);

/**
 ***********************************************************************************************************************
 * @struct      mo_mqttc_t
 *
 * @brief       molink mqtt client
 ***********************************************************************************************************************
 */
typedef struct mo_mqttc
{
    mo_object_t *module;         /* The module used by the MQTT client */

    os_uint32_t mqttc_id;        /* The MQTT client identity */

    mqttc_stat_t stat;           /* The MQTT client status */

    char       *address;         /* The MQTT server address */
    os_uint16_t port;            /* The MQTT server port */

    os_uint32_t keep_alive;      /* The keep alive time, unit second */
    os_uint32_t command_timeout; /* The MQTT client ACK timeout, unit ms*/
    os_uint8_t  clean_session;   /* The flag of clean_session option */

    struct mqtt_msg_handlers
    {
        const char  *topic_filter;
        mqttc_msg_handler_t handler;
    } msg_handlers[MQTTC_MAX_MESSAGE_HANDLERS];  /* The MQTT client handlers */
    
    mqttc_msg_handler_t default_handler;         /* The MQTT client default handlers */

    os_mq_t mq;                                  /* The queue used to receive the data */

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_t mutex;
    os_task_t *task;             /* The thread that receives data and executes handler */
#endif

} mo_mqttc_t;

/**
 ***********************************************************************************************************************
 * @struct      mo_mqttc_ops_t
 *
 * @brief       molink mqtt client ops table
 ***********************************************************************************************************************
 */
typedef struct mo_mqttc_ops
{
    mo_mqttc_t *(*create)(mo_object_t *module, mqttc_create_opts_t *create_opts);
    os_err_t    (*connect)(mo_mqttc_t *client, mqttc_conn_opts_t *conn_opts);
    os_err_t    (*publish)(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_t *msg);
    os_err_t    (*subscribe)(mo_mqttc_t *client, const char *topic_filter, mqttc_qos_t qos);
    os_err_t    (*unsubscribe)(mo_mqttc_t *client, const char *topic_filter);
    os_err_t    (*disconnect)(mo_mqttc_t *client);
    os_err_t    (*destroy)(mo_mqttc_t *client);
} mo_mqttc_ops_t;

mo_mqttc_t *mo_mqttc_create(mo_object_t *module, mqttc_create_opts_t *create_opts);
os_err_t    mo_mqttc_connect(mo_mqttc_t *client, mqttc_conn_opts_t *conn_opts);
os_err_t    mo_mqttc_publish(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_t *msg);
os_err_t    mo_mqttc_set_msg_handler(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_handler_t handler);
os_err_t    mo_mqttc_subscribe(mo_mqttc_t *client,
                               const char *topic_filter,
                               mqttc_qos_t qos,
                               mqttc_msg_handler_t handler);
os_err_t    mo_mqttc_unsubscribe(mo_mqttc_t *client, const char *topic_filter);
os_err_t    mo_mqttc_disconnect(mo_mqttc_t *client);
os_bool_t   mo_mqttc_isconnect(mo_mqttc_t *client);
os_err_t    mo_mqttc_destroy(mo_mqttc_t *client);
os_err_t    mo_mqttc_yield(mo_mqttc_t *client, os_uint32_t timeout_ms);

#if defined(MOLINK_USING_MQTTC_TASK)
os_err_t    mo_mqttc_start_task(mo_mqttc_t *client);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_MQTTC_OPS */

#endif /* __MO_MQTTC_H__ */
