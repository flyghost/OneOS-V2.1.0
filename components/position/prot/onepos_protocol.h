/**
 ***********************************************************************************************************************
 * Copyright (c)2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        onepos_protocol.h
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __ONEPOS_PROTOCOL_H__
#define __ONEPOS_PROTOCOL_H__
#include <os_list.h>
#include <os_mutex.h>
#include <cms_con_mqtt.h>
#include "onepos_common.h"

typedef void (*onepos_prot_cb_handler)(void *handle, const char *topic_name, void *payload, size_t payload_len);

/**
 ***********************************************************************************************************************
 * @def         ONEPOS_PLATFORM_COMM_TIMEOUT
 *
 * @brief       Communication timeout with the onepos platform(unit : millsecond)
 ***********************************************************************************************************************
 */
#define ONEPOS_PLATFORM_COMM_TIMEOUT 5000
/**
 ***********************************************************************************************************************
 * @def         ONEPOS_MQTT_COMM_ALIVE_TIME
 * @brief       keep alive time while mqtt communication
 ***********************************************************************************************************************
 */
#define ONEPOS_MQTT_COMM_ALIVE_TIME 50u
/**
 ***********************************************************************************************************************
 * @def         ONEPOS_MQTT_COMM_QOS
 *
 * @brief       Quality of Service while mqtt communication
 *
 * @param       mqtt_qos0       At most once
 * @param       mqtt_qos1       At least once
 ***********************************************************************************************************************
 */
#define ONEPOS_MQTT_COMM_QOS mqtt_qos0

#define ONEPOS_MQTT_RECONN_DELAY 1000   

#define ONEPOS_CMS_SERVER_ID     "1002"

#define POS_TOPIC_PRE "svr/"ONEPOS_CMS_SERVER_ID

#define ONEPOS_MQTT_TOPIC_STRLEN        50

typedef char onepos_prot_pub_tpc_t [ONEPOS_MQTT_TOPIC_STRLEN];

/**
 ***********************************************************************************************************************
 * @enum        onepos_conf_err_code_t
 *
 * @brief       onepos config error code
 ***********************************************************************************************************************
 */
typedef enum
{
    ONEPOS_CONFIG_SUCC = 0,
    ONEPOS_CONFIG_FAIL = 10001
} onepos_conf_err_code_t;

/**
 ***********************************************************************************************************************
 * @enum        onepos_comm_err_code_t
 *
 * @brief       onepos Supported Communication Error Code
 ***********************************************************************************************************************
 */
typedef enum
{
    ONEPOS_COMM_SUCC     = 0,
    ONEPOS_NULL_POSITION = 10000,
    ONEPOS_COMM_FAIL     = 10010,
    ONEPOS_OVER_LIMIT    = 11000,
    /* Add others communication error code */
} onepos_comm_err_code_t;

typedef struct onepos_prot_sub
{
    char                   topic_name[ONEPOS_MQTT_TOPIC_STRLEN];
    onepos_prot_cb_handler topic_cb;
}prot_sub_t;
typedef struct onepos_prot_sub_tps_node
{
    os_list_node_t list;
    prot_sub_t     node;
}onepos_prot_sub_tps_node_t;

typedef struct onepos_prot 
{
    char                    name[OS_NAME_MAX];
    os_uint32_t             user_num;
    os_mutex_t              prot_lock;
    void                   *cms_mqtt;
    os_list_node_t          sub_tpcs_list;
    cms_mqtt_connect_param  conn_param; 
} onepos_prot_t;

#define PROT_IS_CONN(prot) (cms_con_state_connect != cms_mqtt_get_state(prot->cms_mqtt) ? OS_FALSE : OS_TRUE)

#define ONEPOS_WAIT_MQTT_COMM_BUSY    20        // ms
#define ONEPOS_WAIT_MQTT_READY        50        // ms
#define ONEPOS_WAIT_PUB_MSG_TIMEOUT   30        // ms

extern os_err_t onepos_prot_lock(onepos_prot_t* prot);
extern os_err_t onepos_prot_unlock(onepos_prot_t* prot);
extern os_err_t onepos_prot_deinit(onepos_prot_t* prot);
extern os_err_t onepos_prot_connect(onepos_prot_t* prot);
extern os_err_t onepos_prot_destroy(onepos_prot_t* prot);
extern os_err_t onepos_prot_disconnect(onepos_prot_t* prot);
extern onepos_prot_t* onepos_prot_creat(const char* prot_name);
extern os_err_t onepos_prot_init(onepos_prot_t* prot);
extern onepos_prot_t* pos_app_using_onepos_prot(void);
extern os_err_t pos_app_nonuse_onepos_prot(onepos_prot_t* prot);
extern os_err_t onepos_init_conn_data(onepos_prot_t* prot);
extern os_err_t onepos_prot_add_topic(onepos_prot_t* prot, const os_uint32_t num, prot_sub_t *topics);
extern os_err_t onepos_prot_remove_topic(onepos_prot_t* prot, const os_uint32_t num, prot_sub_t *topics);
extern os_err_t onepos_mqtt_msg_publish(onepos_prot_t *prot, const char *topic, char *buff, os_size_t len);
#endif /* __ONEPOS_PROTOCOL_H__ */
