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
 * @file        mo_mqttc.c
 *
 * @brief       module link kit mqtt client api
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-04   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mo_mqttc.h"
#include "mo_mqttc_priv.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MO_LOG_TAG "molink.mqttc"
#define MO_LOG_LVL  MO_LOG_INFO
#include "mo_log.h"

#ifndef MQTTC_TASK_STACK_SIZE
#define MQTTC_TASK_STACK_SIZE 1024
#endif

#ifndef MQTTC_TASK_PRIORITY
#define MQTTC_TASK_PRIORITY (20)
#endif

#ifndef MQTTC_TASK_TIMESLICE
#define MQTTC_TASK_TIMESLICE 5
#endif

#ifdef MOLINK_USING_MQTTC_OPS

static mo_mqttc_ops_t *get_mqttc_ops(mo_object_t *self)
{
    mo_mqttc_ops_t *ops = (mo_mqttc_ops_t *)self->ops_table[MODULE_OPS_MQTTC];

    if (OS_NULL == ops)
    {
        ERROR("Module %s does not support mqtt client operates", self->name);
    }

    return ops;
}


/**
 ***********************************************************************************************************************
 * @brief           This function create an instance of a molink mqtt client
 *
 * @param[in]       module          The descriptor of molink module instance 
 * @param[in]       create_opts     The create options of client. @ref mqttc_create_opts_t
 * 
 * @return          On success, return a molink mqtt client descriptor; 
 *                  On error, OS_NULL is returned.
 ***********************************************************************************************************************
 */
mo_mqttc_t *mo_mqttc_create(mo_object_t *module, mqttc_create_opts_t *create_opts)
{
    OS_ASSERT(OS_NULL != module);
    OS_ASSERT(OS_NULL != create_opts);

    mo_mqttc_ops_t *ops = get_mqttc_ops(module);

    if (OS_NULL == ops)
    {
        return OS_NULL;
    }

    if (OS_NULL == ops->create)
    {
        ERROR("Module %s does not support mqtt client create operate", module->name);
        return OS_NULL;
    }

    return ops->create(module, create_opts);
}

/**
 ***********************************************************************************************************************
 * @brief           This function open the mqtt client connection to the server
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * @param[in]       connect_opts    The connect options of client. @ref mqttc_conn_opts_t
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Connect failed
 * @retval          OS_ETIMEOUT     Connect timeout
 * @retval          OS_EOK          Connect successfully
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_connect(mo_mqttc_t *client, mqttc_conn_opts_t *connect_opts)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != client->module);
    OS_ASSERT(OS_NULL != connect_opts);

    mo_object_t *module = client->module;

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_lock(&client->mutex, OS_WAIT_FOREVER);
#endif

    if (client->stat != MQTTC_STAT_INIT)
    {
        ERROR("Module %s mqtt client %d connect state %d error!", module->name, client->mqttc_id, client->stat);
        return OS_ERROR;
    }

    mo_mqttc_ops_t *ops = get_mqttc_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->connect)
    {
        ERROR("Module %s does not support mqtt client connect operate", module->name);
        return OS_ERROR;
    }

    os_err_t result = ops->connect(client, connect_opts);

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_unlock(&client->mutex);
#endif

    return result;
}

static void mqttc_clean_session(mo_mqttc_t *client)
{
    for (int i = 0; i < MQTTC_MAX_MESSAGE_HANDLERS; i++)
    {
        client->msg_handlers[i].topic_filter = OS_NULL;
    }
}

static void mqttc_close_session(mo_mqttc_t *client)
{
    client->stat = MQTTC_STAT_DISCONNECT;
    if (client->clean_session)
    {
        mqttc_clean_session(client);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function tries to publish the message to a given topic
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * @param[in]       topic_filter    The topic associated with this message
 * @param[in]       msg             The message to be published. @ref mqttc_msg_t
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Publish failed
 * @retval          OS_ETIMEOUT     Publish timeout
 * @retval          OS_EOK          Publish successfully
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_publish(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_t *msg)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != topic_filter);
    OS_ASSERT(OS_NULL != msg);

    mo_object_t *module = client->module;

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_lock(&client->mutex, OS_WAIT_FOREVER);
#endif

    if (client->stat != MQTTC_STAT_CONNECT)
    {
        ERROR("Module %s mqtt client %d publish state %d error!", module->name, client->mqttc_id, client->stat);
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    mo_mqttc_ops_t *ops = get_mqttc_ops(module);

    if (OS_NULL == ops)
    {
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    if (OS_NULL == ops->publish)
    {
        ERROR("Module %s does not support mqtt client publish operate", module->name);
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    os_err_t result = ops->publish(client, topic_filter, msg);
    if (result != OS_EOK)
    {
        mqttc_close_session(client);
    }

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_unlock(&client->mutex);
#endif

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function set or remove a per topic message handler
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * @param[in]       topic_filter    The topic filter set the message handler for
 * @param[in]       handler         Pointer to the message handler function or NULL to remove. @ref mqttc_msg_handler_t
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Set failed
 * @retval          OS_EOK          Set successfully
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_set_msg_handler(mo_mqttc_t *client, const char *topic_filter, mqttc_msg_handler_t handler)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != topic_filter);

    os_err_t result = OS_ERROR;
    
    os_int32_t i = -1;

    /* first check for an existing matching slot */
    for (i = 0; i < MQTTC_MAX_MESSAGE_HANDLERS; i++)
    {
        if (client->msg_handlers[i].topic_filter != OS_NULL &&
            strcmp(client->msg_handlers[i].topic_filter, topic_filter) == 0)
        {
            if (OS_NULL == handler) /* remove existing */
            {
                client->msg_handlers[i].topic_filter = OS_NULL;
                client->msg_handlers[i].handler      = OS_NULL;
            }
            result = OS_EOK;
            break;
        }
    }
    /* if no existing, look for empty slot (unless we are removing) */
    if (handler != OS_NULL)
    {
        if (OS_ERROR == result)
        {
            for (i = 0; i < MQTTC_MAX_MESSAGE_HANDLERS; i++)
            {
                if (OS_NULL == client->msg_handlers[i].topic_filter)
                {
                    result = OS_EOK;
                    break;
                }
            }
        }
        if (i < MQTTC_MAX_MESSAGE_HANDLERS)
        {
            client->msg_handlers[i].topic_filter = topic_filter;
            client->msg_handlers[i].handler      = handler;
        }
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function attempts to subscribe a client to a single topic
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * @param[in]       topic_filter    The subscription topic
 * @param[in]       qos             The requested quality of service for the subscription.
 * @param[in]       handler         Pointer to the message handler function. @ref mqttc_msg_handler_t
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Subscribe failed
 * @retval          OS_ETIMEOUT     Subscribe timeout
 * @retval          OS_EOK          Subscribe successfully
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_subscribe(mo_mqttc_t *client, const char *topic_filter, mqttc_qos_t qos, mqttc_msg_handler_t handler)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != topic_filter);

    mo_object_t *module = client->module;

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_lock(&client->mutex, OS_WAIT_FOREVER);
#endif

    if (client->stat != MQTTC_STAT_CONNECT)
    {
        ERROR("Module %s mqtt client %d publish state %d error!", module->name, client->mqttc_id, client->stat);
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    mo_mqttc_ops_t *ops = get_mqttc_ops(module);

    if (OS_NULL == ops)
    {
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    if (OS_NULL == ops->subscribe)
    {
        ERROR("Module %s does not support mqtt client subscribe operate", module->name);
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    os_err_t result = ops->subscribe(client, topic_filter, qos);
    if (OS_EOK == result)
    {
        result = mo_mqttc_set_msg_handler(client, topic_filter, handler);
    }

    if (result != OS_EOK)
    {
        mqttc_close_session(client);
    }

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_unlock(&client->mutex);
#endif
    
    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function attempts to remove an existing subscription made by the specified client.
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * @param[in]       topic_filter    The topic for the subscription to be removed
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Remove failed
 * @retval          OS_ETIMEOUT     Remove timeout
 * @retval          OS_EOK          Remove successfully
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_unsubscribe(mo_mqttc_t *client, const char *topic_filter)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != topic_filter);

    mo_object_t *module = client->module;

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_lock(&client->mutex, OS_WAIT_FOREVER);
#endif

    if (client->stat != MQTTC_STAT_CONNECT)
    {
        ERROR("Module %s mqtt client %d publish state %d error!", module->name, client->mqttc_id, client->stat);
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    mo_mqttc_ops_t *ops = get_mqttc_ops(module);

    if (OS_NULL == ops)
    {
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    if (OS_NULL == ops->unsubscribe)
    {
        ERROR("Module %s does not support mqtt client unsubscribe operate", module->name);
        os_mutex_unlock(&client->mutex);
        return OS_ERROR;
    }

    os_err_t result = ops->unsubscribe(client, topic_filter);
    if (OS_EOK == result)
    {   
        /* remove the subscription message handler associated with this topic, if there is one */
        result = mo_mqttc_set_msg_handler(client, topic_filter, OS_NULL);
    }

    if (result != OS_EOK)
    {
        mqttc_close_session(client);
    }

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_unlock(&client->mutex);
#endif

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function attempts to disconnect the client from the MQTT server
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Disconnect failed
 * @retval          OS_ETIMEOUT     Disconnect timeout
 * @retval          OS_EOK          Disconnect successfully
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_disconnect(mo_mqttc_t *client)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != client->module);

    mo_object_t *module = client->module;

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_lock(&client->mutex, OS_WAIT_FOREVER);
#endif

    if (client->stat != MQTTC_STAT_CONNECT && client->stat != MQTTC_STAT_DISCONNECT)
    {
        ERROR("Module %s mqtt client %d connect state %d error!", module->name, client->mqttc_id, client->stat);
        return OS_ERROR;
    }

    mo_mqttc_ops_t *ops = get_mqttc_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->disconnect)
    {
        ERROR("Module %s does not support mqtt client disconnect operate", module->name);
        return OS_ERROR;
    }

    os_err_t result = ops->disconnect(client);

    mqttc_close_session(client);

#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_unlock(&client->mutex);
#endif

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function allows the client application to test whether or not 
 *                  a client is currently connected to the MQTT server.
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * 
 * @return          Boolean OS_TURE if the client is connected, otherwise OS_FALSE.
 ***********************************************************************************************************************
 */
os_bool_t mo_mqttc_isconnect(mo_mqttc_t *client)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != client->module);

    return (client->stat == MQTTC_STAT_CONNECT) ? OS_TRUE : OS_FALSE;
}

/**
 ***********************************************************************************************************************
 * @brief           This function destroy an instance of a molink mqtt client
 *
 * @param[in]       client          The descriptor of molink mqtt client instance 
 * 
 * @return          Returns the result of the operation
 * @retval          OS_ERROR        Destroy failed
 * @retval          OS_EOK          Destroy successfully
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_destroy(mo_mqttc_t *client)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != client->module);

    mo_object_t *module = client->module;

    if (client->stat == MQTTC_STAT_NULL)
    {
        ERROR("Module %s mqtt client %d connect state %d error!", module->name, client->mqttc_id, client->stat);
        return OS_ERROR;
    }

#if defined(MOLINK_USING_MQTTC_TASK)
    if (client->task != OS_NULL)
    {
        os_task_destroy(client->task);
    }
#endif

    mo_mqttc_ops_t *ops = get_mqttc_ops(module);

    if (OS_NULL == ops)
    {
        return OS_ERROR;
    }

    if (OS_NULL == ops->destroy)
    {
        ERROR("Module %s does not support mqtt client destroy operate", module->name);
        return OS_ERROR;
    }

    return ops->destroy(client);
}

/*
 * assume topic filter and name is in correct format
 * # can only be at end
 * + and # can only be next to separator
*/ 
static os_bool_t is_topic_matched(char* topicFilter, mqttc_string_t* topic_name)
{
    char* curf = topicFilter;
    char* curn = topic_name->data;
    char* curn_end = curn + topic_name->len;

    while (*curf && curn < curn_end)
    {
        if (*curn == '/' && *curf != '/')
            break;
        if (*curf != '+' && *curf != '#' && *curf != *curn)
            break;
        if (*curf == '+')
        {   // skip until we meet the next separator, or end of string
            char* nextpos = curn + 1;
            while (nextpos < curn_end && *nextpos != '/')
                nextpos = ++curn + 1;
        }
        else if (*curf == '#')
            curn = curn_end - 1;    // skip until end of string
        curf++;
        curn++;
    };

    return (curn == curn_end) && (*curf == '\0');
}

static os_err_t deliver_message(mo_mqttc_t *client, mqttc_msg_data_t *msg)
{
    os_err_t result = OS_ERROR;

    for (int i = 0; i < MQTTC_MAX_MESSAGE_HANDLERS; i++)
    {
        if (client->msg_handlers[i].topic_filter != OS_NULL &&
            (strncmp(msg->topic_name.data, client->msg_handlers[i].topic_filter, msg->topic_name.len) == 0 ||
             is_topic_matched((char *)client->msg_handlers[i].topic_filter, &msg->topic_name)))
        {
            if (client->msg_handlers[i].handler != OS_NULL)
            {
                client->msg_handlers[i].handler(msg);
                result = OS_EOK;
            }
        }
    }

    if (OS_ERROR == result && client->default_handler != OS_NULL)
    {
        client->default_handler(msg);
        result = OS_EOK;
    }
    
    return result;
}

static os_err_t mqttc_pop_message(mo_mqttc_t *client, os_tick_t timeout)
{
    mqttc_msg_data_t *msg = OS_NULL;
    os_size_t msg_len = 0;

    os_err_t result = os_mq_recv(&client->mq, (const void**)&msg, sizeof(msg), timeout, &msg_len);
    if (client->stat != MQTTC_STAT_CONNECT && (OS_EEMPTY == result || OS_ETIMEOUT == result))
    {
        DEBUG("Module %s mqtt client %d does not connect to server, pop message failed!",
                  module->name,
                  client->mqttc_id);
        return OS_ERROR;
    }

    switch(result)
    {
    case OS_ERROR:
        ERROR("Module %s mqtt client %d pop message error", client->module->name, client->mqttc_id);
        return result;
    case OS_ETIMEOUT:
        DEBUG("Module %s mqtt client %d pop message timeout", client->module->name, client->mqttc_id);
        return result;
    case OS_EEMPTY:
        DEBUG("Module %s mqtt client %d pop message empty", client->module->name, client->mqttc_id);
        return result;
    default:
        break;
    }

    if (deliver_message(client, msg) != OS_EOK)
    {
        WARN("Module %s mqtt client %d deliver message error", client->module->name, client->mqttc_id);
    }

    free(msg->message.payload);
    free(msg->topic_name.data);
    free(msg);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           When implementing a single-threaded client,
 *                  call this function periodically to allow processing of message
 *
 * @param[in]       client          The descriptor of molink mqtt client instance
 * @param[in]       timeout_ms      The length of time to wait for a message in milliseconds. 
 * 
 * @return          Returns the result of the operation
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_yield(mo_mqttc_t *client, os_uint32_t timeout_ms)
{
    os_err_t result = OS_EOK;

    os_tick_t now_tick = 0;
    os_tick_t pre_tick = os_tick_get();
    os_tick_t rem_tick = os_tick_from_ms(timeout_ms);

    do
    {
        if (mqttc_pop_message(client, rem_tick) == OS_ERROR)
        {
            result = OS_ERROR;
            break;
        }

        now_tick = os_tick_get();

        if (now_tick - pre_tick < rem_tick)
        {
            rem_tick -= now_tick - pre_tick;
        }
        else
        {
            break;
        }

    } while (1);
    
    return result;
}

static void mqttc_run(void *parameter)
{
    mo_mqttc_t *client = (mo_mqttc_t *)parameter;

    while (1)
    {
#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_lock(&client->mutex, OS_WAIT_FOREVER);
#endif
    mqttc_pop_message(client, os_tick_from_ms(500));
#if defined(MOLINK_USING_MQTTC_TASK)
    os_mutex_unlock(&client->mutex);
#endif
    }
}

#if defined(MOLINK_USING_MQTTC_TASK)
/**
 ***********************************************************************************************************************
 * @brief           MQTT start background task for a client.  After this, mo_mqttc_yield should not be called.
 *
 * @param[in]       client          The descriptor of molink mqtt client instance
 * 
 * @return          Returns the result of the operation
 ***********************************************************************************************************************
 */
os_err_t mo_mqttc_start_task(mo_mqttc_t *client)
{
    OS_ASSERT(client != OS_NULL);

    char task_name[OS_NAME_MAX + 1] = {0};

    snprintf(task_name, OS_NAME_MAX, "%smqc%d", client->module->name, client->mqttc_id);

    client->task = os_task_create(task_name, 
                                  mqttc_run,
                                  client,
                                  MQTTC_TASK_STACK_SIZE,
                                  MQTTC_TASK_PRIORITY,
                                  MQTTC_TASK_TIMESLICE);
    if (OS_NULL == client->task)
    {
        ERROR("Create Module %s mqtt client %d task failed!", client->module->name, client->mqttc_id);
        return OS_ERROR;
    }
    
    return os_task_startup(client->task);
}
#endif

os_err_t mo_mqttc_data_queue_push_notice(mo_mqttc_t *client, mqttc_msg_data_t *msg)
{
    OS_ASSERT(OS_NULL != client);
    OS_ASSERT(OS_NULL != msg);

    os_err_t result = OS_EOK;

    if (MQTTC_STAT_CONNECT == client->stat)
    {
        result = os_data_queue_push(&client->msg_queue, msg, sizeof(mqttc_msg_data_t), OS_WAIT_FOREVER);
    }
    else
    {
        result = OS_ERROR;
        ERROR("MQTT client id %d push state error", client->mqttc_id);
    }
    
    return result;
}

void mo_mqttc_data_queue_disconnect_notice(mo_mqttc_t *client)
{
    OS_ASSERT(OS_NULL != client);

    if (MQTTC_STAT_CONNECT == client->stat)
    {
        client->stat = MQTTC_STAT_DISCONNECT;
        os_data_queue_reset(&client->msg_queue);
    }
    else
    {
        ERROR("MQTT client id %d disconnect state error", client->mqttc_id);
    }
}

void mo_mqttc_data_queue_destroy(mo_mqttc_t *client)
{
    OS_ASSERT(OS_NULL != client);

    os_data_queue_t *msg_queue = &client->msg_queue;

    os_err_t          result    = OS_EOK;
    os_size_t         data_size = 0;
    mqttc_msg_data_t *msg_ptr  = OS_NULL;

    do
    {
        result = os_data_queue_pop(msg_queue, (const void **)&msg_ptr, &data_size, OS_IPC_WAITING_NO);
        if (result != OS_EOK)
        {
            /* All items in the queue have been released */

            break;
        }

        free(msg_ptr->message.payload);
        free(msg_ptr->topic_name.data);
        free(msg_ptr);

    } while (1);

    os_data_queue_deinit(msg_queue);
}

#endif /* MOLINK_USING_MQTTC_OPS */
