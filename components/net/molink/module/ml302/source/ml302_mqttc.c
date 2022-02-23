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
 * @file        ml302_mqttc.c
 *
 * @brief       ml302 module link kit mqtt client api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-04   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_mqttc_priv.h"

#include "ml302_mqttc.h"
#include "ml302.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "ml302.mqttc"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#define ML302_MQTTC_ADDRESS_LEN_MAX   (128)
#define ML302_MQTTC_CLIENT_ID_LEN_MAX (128)
#define ML302_MQTTC_USERNAME_LEN_MAX  (128)
#define ML302_MQTTC_PASSWORD_LEN_MAX  (128)
#define ML302_MQTTC_TOPIC_LEN_MAX     (512)
#define ML302_MQTTC_MSG_LEN_MAX       (2048)
#define ML302_MQTTC_CMD_TIMEOUT_DEF   (10 * 1000) /* 10s */
#define ML302_MQTTC_MSG_NUM_DEF       (10)

#ifdef ML302_USING_MQTTC_OPS

static os_bool_t ml302_check_create_opts(mqttc_create_opts_t *create_opts)
{
    if (create_opts->address.len == 0)
    {
        ERROR("The host address length is not valid");
        return OS_FALSE;
    }
    else if (create_opts->address.len > ML302_MQTTC_ADDRESS_LEN_MAX)
    {
        ERROR("The host address or IP string is too long and the longest length is %d.",
                  ML302_MQTTC_ADDRESS_LEN_MAX);
        return OS_FALSE;
    }
    
    return OS_TRUE;
}

static os_bool_t ml302_mqttc_check_state(mo_object_t *module)
{
    at_parser_t *parser = &module->parser;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff, 
                      .buff_size = sizeof(resp_buff), 
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    if (at_parser_exec_cmd(parser, &resp, "AT+MQTTCFG?") != OS_EOK)
    {
        ERROR("Check module %s mqtt client state failed!", module->name);
        return OS_FALSE;
    }

    if (OS_NULL == at_resp_get_line_by_kw(&resp, "+MQTTCFG:\"(null)\""))
    {
        ERROR("The MQTT client in the %s module has not been released, please reset the module and try again",
                  module->name);
        return OS_FALSE;
    }

    return OS_TRUE;
}

static mo_mqttc_t *ml302_mqttc_alloc(mo_object_t *module)
{
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    /* The ML302 supports only one MQTT client */
    if (ml302->mqttc[0].stat != MQTTC_STAT_NULL)
    {
        ERROR("Module %s alloc mqtt clinet failed!", module->name);
        return OS_NULL;
    }

    if (ml302_mqttc_check_state(module))
    {
        ml302->mqttc[0].mqttc_id = 0;
        return &ml302->mqttc[0];
    }
    
    return OS_NULL;
}

static os_err_t ml302_mqttc_do_create(mo_mqttc_t *client, mqttc_create_opts_t *create_opts)
{
    os_size_t max_msgs = create_opts->max_msgs == 0 ? ML302_MQTTC_MSG_NUM_DEF : create_opts->max_msgs;

    os_err_t result = os_data_queue_init(&client->msg_queue, max_msgs, 0, OS_NULL);
    if (result != OS_EOK)
    {
        ERROR("%s data queue init failed, no enough memory.", client->module->name);
        return OS_ENOMEM;
    }

    client->address = os_calloc(1, create_opts->address.len + 1);
    if (OS_NULL == client->address)
    {
        os_data_queue_deinit(&client->msg_queue);
        return OS_ENOMEM;
    }
    strncpy(client->address, create_opts->address.data, create_opts->address.len);

    client->port = create_opts->port;
    client->command_timeout =
        create_opts->command_timeout == 0 ? ML302_MQTTC_CMD_TIMEOUT_DEF : create_opts->command_timeout;

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_init(&client->mutex, client->module->name, OS_IPC_FLAG_PRIO, OS_FALSE);
#endif

    return OS_EOK;
}

mo_mqttc_t *ml302_mqttc_create(mo_object_t *module, mqttc_create_opts_t *create_opts)
{
    if (!ml302_check_create_opts(create_opts))
    {
        ERROR("Module %s create options is not valid", module->name);
        return OS_NULL;
    }
    
    mo_ml302_t *ml302 = os_container_of(module, mo_ml302_t, parent);

    os_mutex_lock(&ml302->mqttc_lock, OS_IPC_WAITING_FOREVER);

    mo_mqttc_t *client = ml302_mqttc_alloc(module);
    if (OS_NULL == client)
    {
        os_mutex_unlock(&ml302->mqttc_lock);
        return OS_NULL;
    }

    os_err_t result = ml302_mqttc_do_create(client, create_opts);
    if (result != OS_EOK)
    {
        memset(client, 0, sizeof(mo_mqttc_t));
        client = OS_NULL;
    }else
    {
        client->module = module;
        client->stat   = MQTTC_STAT_INIT;
    }

    os_mutex_unlock(&ml302->mqttc_lock);

    return client;
}

static os_bool_t ml302_check_conn_opts(mqttc_conn_opts_t *conn_opts)
{
    if (conn_opts->client_id.len != 0 && conn_opts->client_id.len > ML302_MQTTC_CLIENT_ID_LEN_MAX)
    {
        ERROR("The client id string length %d is not valid.", conn_opts->client_id.len);
        return OS_FALSE;
    }

    if (conn_opts->username.len != 0 && conn_opts->username.len > ML302_MQTTC_USERNAME_LEN_MAX)
    {
        ERROR("The username string length %d is not valid.", conn_opts->username.len);
        return OS_FALSE;
    }

    if (conn_opts->password.len != 0 && conn_opts->password.len > ML302_MQTTC_PASSWORD_LEN_MAX)
    {
        ERROR("The password string length %d is not valid.", conn_opts->password.len);
        return OS_FALSE;
    }

    if (MQTTC_WILL_FALG_ENABLE == conn_opts->will_flag)
    {
        mqttc_will_opts_t will_opts = conn_opts->will_opts;

        if (will_opts.topic_name.len == 0 || will_opts.topic_name.len > ML302_MQTTC_TOPIC_LEN_MAX)
        {
            ERROR("The will topic string length %d is not valid", will_opts.topic_name.len);
            return OS_FALSE;
        }

        if (will_opts.message.len == 0 || will_opts.message.len > ML302_MQTTC_MSG_LEN_MAX)
        {
            ERROR("The will message length %d is not valid", will_opts.message.len);
            return OS_FALSE;
        }
    }
    
    return OS_TRUE;
}

os_err_t ml302_mqttc_connect(mo_mqttc_t *client, mqttc_conn_opts_t *conn_opts)
{
    mo_object_t *module = client->module;
    
    if (!ml302_check_conn_opts(conn_opts))
    {
        ERROR("Module %s connect options is not valid", module->name);
        return OS_ERROR;
    }

    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 1 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+MQTTTO=%d", client->command_timeout / 1000);
    if (result != OS_EOK)
    {
        ERROR("Module %s mqtt client %d configuration parameters failed", module->name, client->mqttc_id);
        return result;
    }

    resp.timeout = 150 * OS_TICK_PER_SECOND;
    /* AT+MQTTCFG=<host>,<port>,<id>,<keepAlive>,<user>,<passwd>,<clean>,<version>,<encrypt> */
    /* TODO: encrypt and version */
    result = at_parser_exec_cmd(parser,
                                &resp,
                                "AT+MQTTCFG=\"%s\",%d,\"%s\",%d,\"%s\",\"%s\",%d,\"\",0",
                                client->address,
                                client->port,
                                conn_opts->client_id.len > 0 ? conn_opts->client_id.data : "",
                                conn_opts->keep_alive,
                                conn_opts->username.len > 0 ? conn_opts->username.data : "",
                                conn_opts->password.len > 0 ? conn_opts->password.data : "",
                                conn_opts->clean_session);
    if (result != OS_EOK)
    {
        ERROR("Module %s mqtt client %d configuration parameters failed", module->name, client->mqttc_id);
        return result;
    }

    os_uint8_t user_falg = conn_opts->username.len > 0 ? 1 : 0;
    os_uint8_t pwd_flag  = conn_opts->password.len > 0 ? 1 : 0;

    mqttc_will_opts_t *will_opts = &conn_opts->will_opts;

    resp.line_num = 4;

    /* AT+MQTTOPEN=<usrFlag>,<pwdFlag>,<willFlag>,<willRetain>,<willQos>,<will-topic>,<will-mesg> */
    result = at_parser_exec_cmd(parser,\
                                &resp,
                                "AT+MQTTOPEN=%d,%d,%d,%d,%d,\"%s\",\"%s\"",
                                user_falg,
                                pwd_flag,
                                conn_opts->will_flag,
                                will_opts->retained,
                                will_opts->qos,
                                will_opts->topic_name.len > 0 ? will_opts->topic_name.data : "",
                                will_opts->message.len > 0 ? will_opts->message.data : "");
    if (result != OS_EOK)
    {
        ERROR("Module %s mqtt client %d connect failed!", module->name, client->mqttc_id);
        return result;
    }

    if (at_resp_get_line_by_kw(&resp, "+MQTTOPEN:OK") == OS_NULL)
    {
        ERROR("Module %s mqtt client %d connect failed!", module->name, client->mqttc_id);
    }
    else
    {
        client->stat = MQTTC_STAT_CONNECT;
    }

    return result;
}

os_err_t ml302_mqttc_publish(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_t *msg)
{
    mo_object_t *module = client->module;

    if (strlen(topic_filter) > ML302_MQTTC_TOPIC_LEN_MAX)
    {
        ERROR("Module %s publish topic string length %d is not valid", module->name, strlen(topic_filter));
        return OS_ERROR;
    }
    
    if (msg->payload_len > ML302_MQTTC_MSG_LEN_MAX)
    {
        ERROR("Module %s publish message length %d is not valid", module->name, msg->payload_len);
        return OS_ERROR;
    }

    at_parser_t *parser = &module->parser;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .line_num  = 2,
                      .timeout   = 5 * OS_TICK_PER_SECOND};

    at_parser_exec_lock(parser);

    at_parser_set_end_mark(parser, ">", 1);

    /* AT+MQTTPUB=<topic>,<qos>,<retain>,<dup>,<message> */
    os_err_t result = at_parser_exec_cmd(parser,
                                         &resp,
                                         "AT+MQTTPUB=\"%s\",%d,%d,%d,%d",
                                         topic_filter,
                                         msg->qos,
                                         msg->retained,
                                         msg->dup,
                                         msg->payload_len);
    if (result != OS_EOK)
    {
        goto __exit;
    }

    if (at_parser_send(parser, msg->payload, msg->payload_len) <= 0)
    {
        ERROR("Module %s publish %d bytes data failed!", module->name, msg->payload_len);
    }

    resp.line_num = 0;

    /* Handle publish results data */
    result = at_parser_exec_cmd(parser, &resp, "");
    if (result != OS_EOK)
    {
        ERROR("Module %s publish check result data failed!", module->name);
    }

__exit:
    at_parser_set_end_mark(parser, OS_NULL, 0);

    at_parser_exec_unlock(parser);

    return result;
}

os_err_t ml302_mqttc_subscribe(mo_mqttc_t *client, const char *topic_filter, mqttc_qos_t qos)
{
    mo_object_t *module = client->module;

    if (strlen(topic_filter) > ML302_MQTTC_TOPIC_LEN_MAX)
    {
        ERROR("Module %s subscribe topic string length %d is not valid", module->name, strlen(topic_filter));
        return OS_ERROR;
    }

    at_parser_t *parser = &module->parser;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND};

    /* AT+MQTTSUB=<topic>,<qos> */
    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+MQTTSUB=\"%s\",%d", topic_filter, qos);
    if (result != OS_EOK)
    {
        ERROR("Module %s mqtt client %d subscribe topic %s failed!", module->name, client->mqttc_id, topic_filter);
    }

    return result;
}

os_err_t ml302_mqttc_unsubscribe(mo_mqttc_t *client, const char *topic_filter)
{
    mo_object_t *module = client->module;

    if (strlen(topic_filter) > ML302_MQTTC_TOPIC_LEN_MAX)
    {
        ERROR("Module %s unsubscribe topic string length %d is not valid", module->name, strlen(topic_filter));
        return OS_ERROR;
    }

    at_parser_t *parser = &module->parser;

    char resp_buff[128] = {0};

    at_resp_t resp = {.buff      = resp_buff, 
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 5 * OS_TICK_PER_SECOND};

    /* AT+MQTTUNSUB=<topic> */
    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+MQTTUNSUB=\"%s\"", topic_filter);
    if (result != OS_EOK)
    {
        ERROR("Module %s mqtt client %d unsubscribe topic %s failed!", module->name, client->mqttc_id, topic_filter);
    }

    return result;
}

static os_err_t ml302_mqttc_do_disconnect(mo_mqttc_t *client)
{
    mo_object_t *module = client->module;
    at_parser_t *parser = &module->parser;

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff, 
                      .buff_size = sizeof(resp_buff),
                      .timeout   = 10 * OS_TICK_PER_SECOND};

    os_err_t result = at_parser_exec_cmd(parser, &resp, "AT+MQTTDISC");
    if (result != OS_EOK)
    {
        ERROR("Module %s mqtt client %d disconnect failed!", module->name, client->mqttc_id);
    }

    return result;
}

os_err_t ml302_mqttc_disconnect(mo_mqttc_t *client)
{
    if (MQTTC_STAT_CONNECT == client->stat)
    {
        return ml302_mqttc_do_disconnect(client);
    }
    else
    {
        return OS_EOK;
    }
}

static os_err_t ml302_mqttc_do_destroy(mo_mqttc_t *client)
{
    at_parser_t *parser = &(client->module->parser);

    char resp_buff[AT_RESP_BUFF_SIZE_DEF] = {0};

    at_resp_t resp = {.buff      = resp_buff,
                      .buff_size = sizeof(resp_buff),
                      .timeout   = AT_RESP_TIMEOUT_DEF};

    return at_parser_exec_cmd(parser, &resp, "AT+MQTTDEL");
}

os_err_t ml302_mqttc_destroy(mo_mqttc_t *client)
{
    DEBUG("Module %s mqtt client in %d status", client->module->name, client->stat);

    if (MQTTC_STAT_CONNECT == client->stat)
    {
        ERROR("Module %s destroy failed, please disconnect first", client->module->name);
        return OS_ERROR;
    }

    os_err_t result = OS_EOK;

    if (client->stat != MQTTC_STAT_INIT)
    {
        result = ml302_mqttc_do_destroy(client);
        if (result != OS_EOK)
        {
            return result;
        }
    }

    mo_mqttc_data_queue_destroy(client);

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_deinit(&client->mutex);
#endif

    os_free(client->address);

    memset(client, 0, sizeof(mo_mqttc_t));

    return OS_EOK;
}

static void urc_ping_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    /* +MQTTPINGRSP: OK */
    if (strstr(data, "OK") != OS_NULL)
    {
        INFO("Receive ping response from the server");
    }
}

static void urc_reconnect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    os_int32_t retry_times = 0;

    sscanf(data, "+MQTTREC: %d", &retry_times);

    INFO("The %d times attempt to reconnect to the server", retry_times);
}

static void urc_disconnect_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);
    
    /* +MQTTDISC: OK */
    if (strstr(data, "OK") != OS_NULL)
    {
        mo_object_t *module = os_container_of(parser, mo_object_t, parser);
        mo_ml302_t * ml302  = os_container_of(module, mo_ml302_t, parent);

        mo_mqttc_t *client = &ml302->mqttc[0];

        mo_mqttc_data_queue_disconnect_notice(client);

        WARN("The connection to the server has been disconnected");
    }
}

static mqttc_msg_data_t *ml302_get_publish_msg(char *data)
{
    mqttc_msg_data_t *msg_data = (mqttc_msg_data_t *)os_calloc(1, sizeof(mqttc_msg_data_t));
    if (OS_NULL == data)
    {
        ERROR("Calloc mqtt message memory failed, no enough memory");
        return OS_NULL;
    }

    /* +MQTTPUBLISH: 0,2,0,1,”test”,23,{"action":"leave_hbcd"} */
    char *delim = ",";
    char *tmp = strtok((char *)data, delim);

    /* Get dup */
    sscanf(tmp, "+MQTTPUBLISH: %hhu", &msg_data->message.dup);

    /* Get qos */
    tmp = strtok(OS_NULL, delim);
    msg_data->message.qos = (mqttc_qos_t)atoi(tmp);

    /* Get retained */
    tmp = strtok(OS_NULL, delim);
    msg_data->message.retained = (os_uint8_t)atoi(tmp);

    /* Skim packID */
    tmp = strtok(OS_NULL, delim);

    /* Get topic */
    tmp = strtok(OS_NULL, delim);
    msg_data->topic_name.len = strlen(tmp) - 2;
    msg_data->topic_name.data = os_calloc(1, msg_data->topic_name.len + 1);
    if (OS_NULL == msg_data->topic_name.data)
    {
        ERROR("Calloc mqtt message topic memory %d bytes failed, no enough memory", msg_data->topic_name.len);
        os_free(msg_data);
        return OS_NULL;
    }
    strncpy(msg_data->topic_name.data, tmp + 1, msg_data->topic_name.len);

    /* Get message length */
    tmp = strtok(OS_NULL, delim);
    msg_data->message.payload_len = (os_size_t)atoi(tmp);

    /* Get message data */
    tmp = strtok(OS_NULL, delim);

    msg_data->message.payload = os_calloc(1, msg_data->message.payload_len);
    if (OS_NULL == msg_data->message.payload)
    {
        ERROR("Calloc mqtt message payload memory %d bytes failed, no enough memory", msg_data->topic_name.len);
        os_free(msg_data->topic_name.data);
        os_free(msg_data);
        return OS_NULL;
    }
    memcpy(msg_data->message.payload, tmp, msg_data->message.payload_len);

    return msg_data;
}

static void urc_publish_func(struct at_parser *parser, const char *data, os_size_t size)
{
    OS_ASSERT(OS_NULL != parser);
    OS_ASSERT(OS_NULL != data);

    mqttc_msg_data_t *msg = ml302_get_publish_msg((char *)data); /* Cast to string split */
    if (OS_NULL == msg)
    {
        return;
    }

    mo_object_t *module = os_container_of(parser, mo_object_t, parser);
    mo_ml302_t  *ml302  = os_container_of(module, mo_ml302_t, parent);
    mo_mqttc_t  *client = &ml302->mqttc[0];

    os_err_t result = mo_mqttc_data_queue_push_notice(client, msg);
    if (result != OS_EOK)
    {
        ERROR("Module %s mqtt client %d push message data failed!", module->name, client->mqttc_id);
        os_free(msg);
    }
}

static at_urc_t gs_urc_table[] = {
    {.prefix = "+MQTTPINGRSP:", .suffix = "\r\n", .func = urc_ping_func},
    {.prefix = "+MQTTREC:",     .suffix = "\r\n", .func = urc_reconnect_func},
    {.prefix = "+MQTTDISC:",    .suffix = "\r\n", .func = urc_disconnect_func},
    {.prefix = "+MQTTPUBLISH:", .suffix = "\r\n", .func = urc_publish_func},
};

void ml302_mqttc_init(mo_ml302_t *module)
{
    /* Init module mqtt client array */
    memset(module->mqttc, 0, sizeof(module->mqttc));

    os_mutex_init(&module->mqttc_lock, module->parent.name, OS_IPC_FLAG_PRIO, OS_FALSE);

    /* Set mqtt client urc table */
    at_parser_t *parser = &(module->parent.parser);
    at_parser_set_urc_table(parser, gs_urc_table, sizeof(gs_urc_table) / sizeof(gs_urc_table[0]));
}

#endif /* ML302_USING_MQTTC_OPS */
