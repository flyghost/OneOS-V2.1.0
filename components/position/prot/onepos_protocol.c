/**
 ***********************************************************************************************************************
 * Copyright (c)2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *use this file except in compliance with the License. You may obtain a copy of
 *the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file        onepos_protocol.c
 *
 * @brief       collect information of position to offer called by upper
 *
 * @details
 *
 * @revision
 * Date         Author          Notes
 * 2020-07-07   OneOs Team      First Version   
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <os_task.h>
#include <os_errno.h>
#include <os_memory.h>
#include <os_assert.h>
#include "onepos_protocol.h"

#define ONEPOS_LOG_TAG "onepos.prot"
#define ONEPOS_LOG_LVL ONEPOS_LOG_INFO
#include <onepos_log.h>

static onepos_prot_t *g_onepos_prot = OS_NULL;
/**
 ***********************************************************************************************************************
 * @brief           Lock the recursive lock that protects the execution of the onepos protocol.
 * @param[in]       prot          A pointer to onepos protocol
 *
 * @return          @see os_mutex_recursive_lock.
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_lock(onepos_prot_t* prot)
{
    return os_mutex_recursive_lock(&prot->prot_lock, OS_WAIT_FOREVER);
}

/**
 ***********************************************************************************************************************
 * @brief           Unlock the recursive lock that protects the execution of the onepos protocol.
 * @param[in]       prot          A pointer to onepos protocol
 *
 * @return          @see os_mutex_recursive_unlock.
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_unlock(onepos_prot_t* prot)
{
    return os_mutex_recursive_unlock(&prot->prot_lock);
}

/**
 ***********************************************************************************************************************
 * @brief           add subscription topic onepos protocol
 *
 * @param[in]       prot             onepos protocol
 * @param[in]       num              number of topic
 * @param[in]       topics           topics to add
 *
 * @return          os_err_t         add succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_add_topic(onepos_prot_t* prot, const os_uint32_t num, prot_sub_t *topics)
{
    os_err_t    ret         = OS_EOK;

    OS_ASSERT(OS_NULL != prot);
    OS_ASSERT(OS_NULL != topics);

    if(num == 0)
    {
        ONEPOS_LOG_W("add 0 topic, addr : 0x%x", topics);
        return OS_EOK;
    }
     
    /* add topic */
    for(os_uint32_t j = 0; j < num; j ++)
    {
        onepos_prot_sub_tps_node_t *tpc = os_malloc(sizeof(onepos_prot_sub_tps_node_t));
        if (tpc == OS_NULL)
        {
            ONEPOS_LOG_E("malloc onepos_prot_sub_tps_node_t error");
            return OS_ENOMEM;
        }

		memcpy(tpc->node.topic_name, topics[j].topic_name, ONEPOS_MQTT_TOPIC_STRLEN);
        tpc->node.topic_cb   = topics[j].topic_cb;

        onepos_prot_lock(prot);
        os_list_add(&prot->sub_tpcs_list, &tpc->list);
        onepos_prot_unlock(prot);    
    }

    /* subscribe topic */
    for(os_uint32_t j = 0; 0 == ret && j < num; j ++)
    {
        ret = cms_mqtt_subscribe(prot->cms_mqtt, topics[j].topic_name, ONEPOS_MQTT_COMM_QOS, topics[j].topic_cb);    
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           remove subscription topic onepos protocol
 *
 * @param[in]       prot             onepos protocol
 * @param[in]       num              number of topic
 * @param[in]       topics           topics to add
 *
 * @return          os_err_t         remove succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_remove_topic(onepos_prot_t* prot, const os_uint32_t num, prot_sub_t *topics)
{
    os_err_t                    ret    = OS_EOK;
    onepos_prot_sub_tps_node_t *node   = OS_NULL;
    onepos_prot_sub_tps_node_t *tmp    = OS_NULL;

    OS_ASSERT(OS_NULL != prot);
    OS_ASSERT(OS_NULL != topics);

    if(num == 0)
    {
        ONEPOS_LOG_W("remove 0 topic, adrr : 0x%x", topics);
        return OS_EOK;
    }
    
    OS_ASSERT(OS_NULL != prot);
    
    /* unsubscribe topic */
    for(os_uint32_t j = 0; j < num; j ++)
    {
        os_list_for_each_entry_safe(node, tmp, &(prot->sub_tpcs_list), onepos_prot_sub_tps_node_t, list)
        {
            if (node && 0 == strcmp(node->node.topic_name, topics[j].topic_name))
            {
                onepos_prot_lock(prot);
                os_list_del(&node->list);
                onepos_prot_unlock(prot);
    
                os_free(node);
    
                ret = cms_mqtt_unsubscribe(prot->cms_mqtt, topics[j].topic_name);    
            }
        }
    }


    return ret;  
}

/**
 ***********************************************************************************************************************
 * @brief           position app using onepos protocol
 *
 * @return          onepos_prot_t    
 ***********************************************************************************************************************
 */
onepos_prot_t* pos_app_using_onepos_prot(void)
{
    onepos_prot_t *prot = OS_NULL;

    /* onepos prot is not  */
    if(OS_NULL == g_onepos_prot)
    {
        prot = onepos_prot_creat("global_prot");
        if(prot)
        {
            if(OS_EOK == onepos_prot_init(prot))
            {
                onepos_prot_lock(prot);
                prot->user_num ++;
                g_onepos_prot = prot;
                onepos_prot_unlock(prot);
            }
            else
            {
                onepos_prot_destroy(prot);
                prot = OS_NULL;
                ONEPOS_LOG_E("init onepos protocol error.");
            }
        }
        else
        {
            ONEPOS_LOG_E("creat onepos protocol error.");
        }
    }
    else
    {
        onepos_prot_lock(g_onepos_prot);
        prot = g_onepos_prot;
        prot->user_num ++;
        onepos_prot_unlock(g_onepos_prot);         
    }

    return prot;
}

/**
 ***********************************************************************************************************************
 * @brief           position app nousing global onepos protocol
 * 
 *
 * @param[in]       prot             onepos protocol
 * 
 * @return          os_err_t    
 ***********************************************************************************************************************
 */
os_err_t pos_app_nonuse_onepos_prot(onepos_prot_t *prot)
{
    os_err_t       result   = OS_EOK;

    OS_ASSERT(prot != OS_NULL);
    OS_ASSERT(prot == g_onepos_prot);

    onepos_prot_lock(prot);
    prot->user_num --;
    onepos_prot_unlock(prot);

    if(0 == prot->user_num)
    {
		onepos_prot_disconnect(prot);
        result = onepos_prot_deinit(prot);
        onepos_prot_destroy(prot);

        g_onepos_prot = OS_NULL;
    }
    else
    {
        ONEPOS_LOG_I("there others using this protocol, should remove privately-owned topic before this.");         
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           onepos protocol creat
 *
 * @param[in]       prot_name        onepos protocol name
 *
 * @return          onepos_prot_t    
 ***********************************************************************************************************************
 */
onepos_prot_t* onepos_prot_creat(const char* prot_name)
{
    onepos_prot_t *prot = OS_NULL;

    OS_ASSERT(OS_NULL != prot_name);

    prot = (onepos_prot_t*)os_calloc(1, sizeof(onepos_prot_t));
    if(OS_NULL == prot)
    {
        ONEPOS_LOG_E("failed to creat onepos_prot.");
        goto __exit;
    }
    else
    {
        os_snprintf(prot->name, OS_NAME_MAX, "%s_prot", prot_name); 
        return prot;
    }

    __exit:
        if(OS_NULL != prot)
        {
            os_free(prot);
            prot = (onepos_prot_t*)OS_NULL;
        }
        return (onepos_prot_t*)OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief           onepos protocol destroy
 *
 * @param[in]       prot            onepos protocol
 *
 * @return          os_err_t        destroy succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_destroy(onepos_prot_t* prot)
{
    OS_ASSERT(OS_NULL != prot);

    if(OS_NULL != prot)
    {
        os_free(prot);
        prot = (onepos_prot_t*)OS_NULL;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           init onepos protocol connection data
 *
 * @param[in]       prot            onepos protocol
 * @param[in]       dev_id          onepos device id
 * @param[in]       passwd          onepos device password
 *
 * @return          os_err_t        init succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_init_conn_data(onepos_prot_t* prot)
{
    OS_ASSERT(OS_NULL != prot);

    prot->conn_param.keep_alive_s    = ONEPOS_MQTT_COMM_ALIVE_TIME;
    prot->conn_param.client_id       = "oneos";
    prot->conn_param.username        = OS_NULL;
    prot->conn_param.password        = OS_NULL;
    
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           init onepos protocol
 *
 * @param[in]       prot            onepos protocol
 * @param[in]       dev_id          onepos device id
 * @param[in]       passwd          onepos device password
 *
 * @return          os_err_t        init succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_init(onepos_prot_t* prot)
{
	void            *handle = OS_NULL;
    os_err_t         result = OS_EOK;
    cms_mqtt_param   param  = {0};

    OS_ASSERT(OS_NULL != prot);
    
    memset(prot, 0, sizeof(onepos_prot_t));
    memset(&param, 0, sizeof(cms_mqtt_param));

    param.sendbuf_size = ONEPOS_COMM_SEND_BUFF_LEN;
    param.recvbuf_size = ONEPOS_COMM_REC_BUFF_LEN;
    param.timeout_ms   = ONEPOS_PLATFORM_COMM_TIMEOUT;

    os_mutex_init(&prot->prot_lock, "onepos_prot", OS_TRUE);

    /* Init connect data */
    onepos_prot_lock(prot);
    result = onepos_init_conn_data(prot);
    onepos_prot_unlock(prot);
    
    if(OS_EOK != result)
    {
        ONEPOS_LOG_E("failed to init network.");
        return OS_ERROR;
    }

    /* Init handle */
    handle = cms_mqtt_init(atoi(ONEPOS_CMS_SERVER_ID), &param);
    if(handle)
    {
        onepos_prot_lock(prot);
        prot->cms_mqtt           = handle;
        os_list_init(&(prot->sub_tpcs_list));
        onepos_prot_unlock(prot);
    }
    else
    {
        result = OS_ENOMEM;
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           onepos protocol disconnect
 *
 * @param[in]       prot             onepos protocol
 *
 * @return          os_err_t        disconnect succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_disconnect(onepos_prot_t* prot)
{
    os_err_t                    ret    = OS_EOK;
    onepos_prot_sub_tps_node_t *node   = OS_NULL;
    onepos_prot_sub_tps_node_t *tmp    = OS_NULL;

	OS_ASSERT(OS_NULL != prot);

    if(PROT_IS_CONN(prot))
    {
        os_list_for_each_entry_safe(node, tmp, &(prot->sub_tpcs_list), onepos_prot_sub_tps_node_t, list)
        {
            if (node)
            {
                ret = cms_mqtt_unsubscribe(prot->cms_mqtt, node->node.topic_name);    
            }

            if(ret != 0)
            {
                break;
            }
        }
    }

    cms_mqtt_disconnect(prot->cms_mqtt);

	return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           onepos protocol connect
 *
 * @param[in]       prot             onepos protocol
 *
 * @return          os_err_t         connect succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_connect(onepos_prot_t* prot)
{
    os_err_t                    ret    = OS_EOK;
    onepos_prot_sub_tps_node_t *node   = OS_NULL;
    onepos_prot_sub_tps_node_t *tmp    = OS_NULL;

    OS_ASSERT(OS_NULL != prot);

    if(cms_connect_success != cms_mqtt_connect(prot->cms_mqtt, &(prot->conn_param))) 
    {
        ONEPOS_LOG_E("protocol connect failed");
        goto __exit;
    }

    ONEPOS_LOG_D("protocol connected");

    if(PROT_IS_CONN(prot))
    {
        os_list_for_each_entry_safe(node, tmp, &(prot->sub_tpcs_list), onepos_prot_sub_tps_node_t, list)
        {
            if (node)
            {
                ret = cms_mqtt_subscribe(prot->cms_mqtt, node->node.topic_name, ONEPOS_MQTT_COMM_QOS, node->node.topic_cb);    
            }

            if(ret != 0)
            {
                goto __exit;
            }
        }
    }

    return OS_EOK;

    __exit:
        onepos_prot_disconnect(prot);
        return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief           deinit onepos protocol
 *
 * @param[in]       prot             onepos protocol
 *
 * @return          os_err_t        deinit succ or failed
 ***********************************************************************************************************************
 */
os_err_t onepos_prot_deinit(onepos_prot_t* prot)
{
    onepos_prot_sub_tps_node_t *node;
    onepos_prot_sub_tps_node_t *tmp;

    OS_ASSERT(OS_NULL != prot);
    
    onepos_prot_lock(prot);
    cms_mqtt_deinit(prot->cms_mqtt);
    prot->cms_mqtt = OS_NULL;

    os_list_for_each_entry_safe(node, tmp, &(prot->sub_tpcs_list), onepos_prot_sub_tps_node_t, list)
    {
        if (node)
        {
            os_list_del(&node->list);
            os_free(node);
        }
    }

    prot->sub_tpcs_list.next = OS_NULL;
    prot->sub_tpcs_list.prev = OS_NULL;

    onepos_prot_unlock(prot);

    os_mutex_deinit(&prot->prot_lock);

    return OS_EOK;
}


/**
 ***********************************************************************************************************************
 * @brief          publish massage to topic of mqtt
 *
 * @param[in]       prot             onepos protocol
 * @param[in]       topic            mqtt topic name
 * @param[in]       msg              message will be published
 * @param[in]       len              length message will be published
 *
 * @return          os_err_t
 * @retval          OS_EOK       publish message to the topic is successfully
 * @retval          OS_ERROR     publish message to the topic is failed
 ***********************************************************************************************************************
 */
os_err_t onepos_mqtt_msg_publish(onepos_prot_t *prot, const char *topic, char *buff, os_size_t len)
{
    os_err_t                ret         = OS_EOK;
    os_int32_t              rc          = 0;
    cms_mqtt_message        message;

    OS_ASSERT(OS_NULL != prot);

    message.qos        = ONEPOS_MQTT_COMM_QOS;
    message.payload    = buff;
    message.payloadlen = len;

    if (PROT_IS_CONN(prot) != OS_FALSE)
    {
#if defined(ONEPOS_PROTO_DEBUG)
        os_kprintf("prot : %s\r\n topic : %s\r\n len : %d\r\n", prot->name, topic, len);
#endif
        rc = cms_mqtt_publish(prot->cms_mqtt, topic, &message);
        if (cms_connect_success != rc)
        {
            ret = OS_ERROR;
            ONEPOS_LOG_E("Return code from MQTT publish is %d", rc);
        }
    }
    else
    {
        ONEPOS_LOG_W("onepos protocol is not concting.");
        ret = OS_ERROR;
    }

    return ret;
}
