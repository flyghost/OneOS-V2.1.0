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
 * @file        onepos_interface.h
 *
 * @brief       control interface of onepos
 *
 * Date         Author          Notes
 * 2020-07-08   OneOs Team      First Version
 ***********************************************************************************************************************
 */
#include <sys/time.h>
#include <os_assert.h>
#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "onepos_net_pos_if.h"

#define ONEPOS_LOG_TAG "onepos.net_pos"
#define ONEPOS_LOG_LVL ONEPOS_LOG_INFO
#include <onepos_log.h>

static onepos_pos_t      g_sys_net_pos_pos = {0,};
static onepos_net_pos_t  g_net_pos         = DEF_NET_POS_INIT(&g_sys_net_pos_pos);

static void net_pos_msg_come_cb_func(void *handle, const char *topic_name, void *payload, size_t payload_len);

#ifdef NET_POS_SUPP_REMOTE_CONF
static void net_pos_cfg_come_cb_func(void *handle, const char *topic_name, void *payload, size_t payload_len);
#endif

static os_err_t  net_pos_init_sub_topic(onepos_net_pos_t* pos)
{
    os_err_t    result                           = OS_EOK;
    prot_sub_t  sub_topic[NET_POS_SUB_TOPIC_MAX] = {0,};

    OS_ASSERT(OS_NULL != pos);

    memset(sub_topic, 0, sizeof(prot_sub_t) * NET_POS_SUB_TOPIC_MAX);

    snprintf(sub_topic[POS_MSG_SUB_TOPIC].topic_name, ONEPOS_MQTT_TOPIC_STRLEN, "%s%s",
            POS_TOPIC_PRE, NET_POS_MSG_SUB_TOPIC_SUFF);
    sub_topic[POS_MSG_SUB_TOPIC].topic_cb = net_pos_msg_come_cb_func;

    #ifdef NET_POS_SUPP_REMOTE_CONF
    snprintf(sub_topic[CONF_SUB_TOPIC].topic_name, ONEPOS_MQTT_TOPIC_STRLEN, "%s%s",
            POS_TOPIC_PRE, NET_POS_CONF_SUB_TOPIC_SUFF);
    sub_topic[CONF_SUB_TOPIC].topic_cb = net_pos_cfg_come_cb_func;
    #endif

    result = onepos_prot_add_topic(pos->prot, NET_POS_SUB_TOPIC_MAX, sub_topic);

    return result;
}

static os_err_t  net_pos_deinit_sub_topic(onepos_net_pos_t* pos)
{
    os_err_t    result                           = OS_EOK;
    prot_sub_t  sub_topic[NET_POS_SUB_TOPIC_MAX] = {0,};

    OS_ASSERT(OS_NULL != pos);

    memset(sub_topic, 0, sizeof(prot_sub_t) * NET_POS_SUB_TOPIC_MAX);

    snprintf(sub_topic[POS_MSG_SUB_TOPIC].topic_name, ONEPOS_MQTT_TOPIC_STRLEN, "%s%s",
            POS_TOPIC_PRE, NET_POS_MSG_SUB_TOPIC_SUFF);

    #ifdef NET_POS_SUPP_REMOTE_CONF
    snprintf(sub_topic[CONF_SUB_TOPIC].topic_name, ONEPOS_MQTT_TOPIC_STRLEN, "%s%s",
            POS_TOPIC_PRE, NET_POS_CONF_SUB_TOPIC_SUFF);
    #endif

    result =  onepos_prot_remove_topic(pos->prot, NET_POS_SUB_TOPIC_MAX, sub_topic);

    return result;
}

void net_pos_init_pub_topic(onepos_net_pos_t* pos)
{
    char    *temp     = OS_NULL;
    
    OS_ASSERT(OS_NULL != pos);
    
    os_mutex_recursive_lock(pos->lock, OS_WAIT_FOREVER);

    temp = pos->pub_topic[POS_MSG_PUB_TOPIC];
    
    memset((void*)(temp), 0, ONEPOS_MQTT_TOPIC_STRLEN);
    snprintf(temp, ONEPOS_MQTT_TOPIC_STRLEN, "%s%s",
             POS_TOPIC_PRE, NET_POS_MSG_PUB_TOPIC_SUFF);

    temp = pos->pub_topic[CONF_PUB_TOPIC];
    
    memset((void*)temp, 0, ONEPOS_MQTT_TOPIC_STRLEN);
    snprintf(temp, ONEPOS_MQTT_TOPIC_STRLEN, "%s%s",
             POS_TOPIC_PRE, NET_POS_CONF_PUB_TOPIC_SUFF);

    os_mutex_recursive_unlock(pos->lock);

}

/**
 ***********************************************************************************************************************
 * @brief           onepos timers call back func
 *
 * @param[in]       parameter       input param(no using)
 ***********************************************************************************************************************
 */
static void onepos_timer_func(void *parameter)
{
    onepos_net_pos_t *pos = (onepos_net_pos_t*)parameter;

    OS_ASSERT(OS_NULL != parameter);

    os_sem_post(pos->notice);
}

/**
 ***********************************************************************************************************************
 * @brief           get onepos server status
 *
 * @return          onepos server is runing or not
 * @retval          ONEPOS_RUNING        onepos server is runing
 * @retval          ONEPOS_CLOSING       onepos server is closed
 ***********************************************************************************************************************
 */
onepos_serv_sta_t onepos_get_server_sta(void)
{
    onepos_serv_sta_t sta = ONEPOS_CLOSING;

    sta = g_net_pos.status;

    return sta;
}
/**
 ***********************************************************************************************************************
 * @brief           set onepos server status
 *
 * @param[in]       sta                 server status to set
                    ONEPOS_RUNING       set server runing
                    ONEPOS_CLOSING      set server closing
 ***********************************************************************************************************************
 */
static os_bool_t onepos_set_server_sta(onepos_serv_sta_t sta)
{
    os_bool_t ret = OS_TRUE;

    if (ONEPOS_CLOSING == sta || (sta > ONEPOS_CLOSING && sta < ONEPOS_MAX_STA))
    {
        g_net_pos.status = sta;
        if (ONEPOS_WILL_CLOSE != sta)
            onepos_rep_net_pos_sta(&g_net_pos);
    }
    else
        ret = OS_FALSE;

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           get onepos server type
 *
 * @return          onepos server is runing or not
 * @retval          ONEPOS_SIG_RUN        onepos server is circulation
 * @retval          ONEPOS_CIRC_RUN       onepos server is single
 ***********************************************************************************************************************
 */
onepos_serv_type_t onepos_get_server_type(void)
{
    onepos_serv_type_t type = ONEPOS_INVAILD_TYPE;

    type                    = g_net_pos.type;

    return type;
}

/**
 ***********************************************************************************************************************
 * @brief           set onepos server type
 *
 * @param[in]       type                  server type to set
                    ONEPOS_SIG_RUN        onepos server is circulation
                    ONEPOS_CIRC_RUN       onepos server is single
 ***********************************************************************************************************************
 */
os_bool_t onepos_set_server_type(onepos_serv_type_t type)
{
    os_bool_t          ret      = OS_TRUE;
    onepos_serv_type_t sev_type = type;

    if (sev_type > ONEPOS_INVAILD_TYPE && sev_type < ONEPOS_MAX_TYPE)
    {
        if (ONEPOS_CLOSING == onepos_get_server_sta())
        {
            g_net_pos.type = sev_type;
            onepos_rep_net_pos_sta(&g_net_pos);
        }
        else
        {
            ONEPOS_LOG_I("only support set position server type while the server closing");
            ret = OS_TRUE;
        }
    }
    else
    {
        ONEPOS_LOG_I("set position server type is error,  should be(0/1)");
        ret = OS_FALSE;
    }
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           get position error
 *
 * @return           position error
 ***********************************************************************************************************************
 */
os_uint32_t onepos_get_sev_pos_err(void)
{
    return g_net_pos.pos_err;
}

/**
 ***********************************************************************************************************************
 * @brief           set position error
 *
 * @param[in]       pos_err            position error to set
 ***********************************************************************************************************************
 */
os_bool_t onepos_set_pos_err(os_uint32_t pos_err)
{
    g_net_pos.pos_err = pos_err;
    onepos_rep_net_pos_sta(&g_net_pos);
    return OS_TRUE;
}

/**
 ***********************************************************************************************************************
 * @brief           get position interval of currently used
 *
 * @return          position interval
 ***********************************************************************************************************************
 */
os_uint32_t onepos_get_pos_interval(void)
{
    os_uint16_t interval = 0;
    interval             = g_net_pos.interval;
    return interval;
}

/**
 ***********************************************************************************************************************
 * @brief           set position interval
 *
 * @param[in]       interval            position interval to set
 ***********************************************************************************************************************
 */

os_bool_t onepos_set_pos_interval(os_int32_t interval)
{
    os_tick_t ticks = 0;

    if (ONEPOS_MIN_INTERVAL <= interval && ONEPOS_MAX_INTERVAL >= interval)
    {
        if (interval == g_net_pos.interval)
        {
            ONEPOS_LOG_I("The interval is already is %ds", g_net_pos.interval);
            return OS_TRUE;
        }

        if (ONEPOS_RUNING == onepos_get_server_sta())
        {
            os_mutex_recursive_lock(g_net_pos.lock, OS_WAIT_FOREVER);

            g_net_pos.interval = interval;
            onepos_rep_net_pos_sta(&g_net_pos);
            ticks = OS_TICK_PER_SECOND * g_net_pos.interval;
            os_timer_stop(g_net_pos.timer);
            os_timer_set_timeout_ticks(g_net_pos.timer, ticks);
            os_timer_start(g_net_pos.timer);

            os_mutex_recursive_unlock(g_net_pos.lock);
        }
        else
        {
            g_net_pos.interval = interval;
        }
    }
    else
    {
        ONEPOS_LOG_I("set position interval error, should be(%us ~ %us)", ONEPOS_MIN_INTERVAL, OS_UINT16_MAX);
        return OS_FALSE;
    }
    return OS_TRUE;
}

static os_bool_t copy_position(onepos_pos_t *dst_info, onepos_pos_t *src_info)
{
    os_bool_t ret = OS_FALSE;
    if (dst_info && src_info)
    {
        memcpy(dst_info, src_info, sizeof(onepos_pos_t));
        ret = OS_TRUE;
    }
    else
    {
        ONEPOS_LOG_E("dst_info is NULL");
    }

    return ret;
}

void onepos_info_print(onepos_pos_t *pos_result)
{
    char      lat[22] = {0,};
    char      lon[22] = {0,};
    struct tm TM = {0};

    if (pos_result)
    {
        gmtime_r((time_t *)&pos_result->time, &TM);
        snprintf(lat, 22, "%-20lf", pos_result->lat_coordinate);
        snprintf(lon, 22, "%-20lf", pos_result->lon_coordinate);
        os_kprintf("$OPPOS %04d/%02d/%02d %02d:%02d:%02d %s %s\r\n",
               TM.tm_year + 1900,
               TM.tm_mon + 1,
               TM.tm_mday,
               TM.tm_hour + 8,
               TM.tm_min,
               TM.tm_sec,
               lat,
               lon);
    }
    else
    {
        ONEPOS_LOG_E("position result is NULL!");
    }
}

/**
 ***********************************************************************************************************************
 * @brief           onepos server exit
 ***********************************************************************************************************************
 */
static void onepos_net_pos_exit(onepos_net_pos_t* pos)
{
    OS_ASSERT(OS_NULL != pos);

    if (pos->timer != OS_NULL)
    {
        os_timer_destroy(pos->timer);
        pos->timer = OS_NULL;
    }
    if (pos->notice != OS_NULL)     
    {
        os_sem_destroy(pos->notice);
        pos->notice = OS_NULL;
    }

    if(pos->prot != OS_NULL)
    {
        net_pos_deinit_sub_topic(pos);
		
        pos_app_nonuse_onepos_prot(pos->prot);
        pos->prot     = OS_NULL;
    }

    if(pos->pub_topic != OS_NULL)
    {
        os_free(pos->pub_topic);
        pos->pub_topic = OS_NULL;
    }

    if (pos->lock != OS_NULL)
    {
        os_mutex_destroy(pos->lock);
        pos->lock = OS_NULL;
    }
    
    if (pos->task != OS_NULL)
    {
//        os_task_suspend(pos->task);
        os_task_destroy(pos->task);
        pos->task = OS_NULL;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           onepos task function
 *
 * @param[in]       parameter       input param(no using)
 ***********************************************************************************************************************
 */
static void net_pos_circ_run_task_func(void *parameter)
{
    onepos_net_pos_t    *pos         = (onepos_net_pos_t*)parameter;

    OS_ASSERT(OS_NULL != parameter);
    
    while (1)
    {
        /* receive stop cmd, will stop */
       if (ONEPOS_WILL_CLOSE == onepos_get_server_sta())
       {
           /* wait latest position rec pos */
           os_task_msleep(ONEPOS_WAIT_MQTT_READY);
           onepos_set_server_sta(ONEPOS_CLOSING);
           ONEPOS_LOG_D("exit onepos network position sever");
           onepos_net_pos_exit(pos);
       }

        os_sem_wait(pos->notice, OS_WAIT_FOREVER);
        os_timer_start(pos->timer);

        ONEPOS_LOG_D("start once position");

        if (!PROT_IS_CONN(pos->prot))
        {
            ONEPOS_LOG_W("onepos protocol is not ready, will delay %u ms, then exit this time position.", ONEPOS_MQTT_RECONN_DELAY);
            os_task_msleep(ONEPOS_MQTT_RECONN_DELAY);
            onepos_prot_connect(pos->prot);
            continue;  
        }
        if (!net_pos_func(pos))
        {
            ONEPOS_LOG_E("this location is error!");
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           onepos server circularly run function of task
 ***********************************************************************************************************************
 */
static void onepos_net_pos_circ_run(onepos_net_pos_t *pos, onepos_pos_t *pos_src)
{
    onepos_prot_t *prot = OS_NULL;

    OS_ASSERT(OS_NULL != pos);
    OS_ASSERT(OS_NULL != pos_src);

    /* check the server is runing or not */
    if (OS_NULL != os_task_find("net_pos"))
    {
        ONEPOS_LOG_I("Pls start server after last server stop.");
        return;
    }
    
    /* clear task while exit sever of os_task_destroy */
    if(OS_NULL != pos->task)
    {
        pos->task = OS_NULL;
    }

    // memset(pos, 0, sizeof(onepos_net_pos_t));
    if(pos->task)
    {
        pos->task = OS_NULL;
    }
    /* init positon result */
    pos->pos_result = pos_src;

    /* malloc publish topic */
    pos->pub_topic = (onepos_prot_pub_tpc_t*)os_calloc(NET_POS_PUB_TOPIC_MAX, sizeof(onepos_prot_pub_tpc_t));
    if (pos->pub_topic == OS_NULL)
    {
        ONEPOS_LOG_I("malloc onepos protocol error.");
        goto __exit ;
    }

    pos->notice = os_sem_create("net_pos_sem", 0, 1);
    if (pos->notice == OS_NULL)
    {
        ONEPOS_LOG_E("create network position notice failed.");
        goto __exit;
    }

    pos->lock = os_mutex_create("net_pos_lock", 1);
    if (pos->lock == OS_NULL)
    {
        ONEPOS_LOG_E("create network position lock failed");
        goto __exit;
    }

    pos->timer = os_timer_create("net_pos_timer",
                                   onepos_timer_func,
                                   (void*)&g_net_pos,
                                   (OS_TICK_PER_SECOND * onepos_get_pos_interval()),
                                   OS_TIMER_FLAG_ONE_SHOT);

    if (pos->timer == OS_NULL)
    {
        ONEPOS_LOG_E("create network position timer failed");
        goto __exit;
    }

    /* onepos protocol */
    prot = pos_app_using_onepos_prot();
    if(prot)
    {
        os_mutex_recursive_lock(pos->lock, OS_WAIT_FOREVER);
        pos->prot = prot;
        os_mutex_recursive_unlock(pos->lock);
    }
    else
    {
        ONEPOS_LOG_E("net_pos using onepos protocol failed");
        goto __exit;
    }

    net_pos_init_pub_topic(pos);
    net_pos_init_sub_topic(pos);
    
    pos->task = os_task_create("net_pos",
                                 net_pos_circ_run_task_func,
                                 (void*)pos,
                                 ONEPOS_NET_POS_TASK_STA_SIZE,
                                 ONEPOS_NET_POS_TASK_PRIO);
    /* start server */
    if (pos->task == OS_NULL)
    {
        ONEPOS_LOG_E("create network position task failed");
        goto __exit;
    }
    
    onepos_set_server_sta(ONEPOS_RUNING);
    
    /* start timer for period timing */
    os_timer_start(pos->timer);
    os_task_startup(pos->task);
    
    return;

    __exit:
        onepos_net_pos_exit(pos);
        
        return ;
}

/**
 ***********************************************************************************************************************
 * @brief           onepos run once
 ***********************************************************************************************************************
 */
static void onepos_net_pos_sgl_run(onepos_net_pos_t *pos, onepos_pos_t *pos_src)
{
    onepos_prot_t *prot = OS_NULL;
    
    OS_ASSERT(OS_NULL != pos);
    OS_ASSERT(OS_NULL != pos_src);

    /* clear task while exit sever of os_task_destroy */
    if(OS_NULL != pos->task)
    {
        pos->task = OS_NULL;
    }

    /* init positon result */
    pos->pos_result = pos_src;

    /* malloc publish topic */
    pos->pub_topic = (onepos_prot_pub_tpc_t*)os_calloc(NET_POS_PUB_TOPIC_MAX, sizeof(onepos_prot_pub_tpc_t));
    if (pos->pub_topic == OS_NULL)
    {
        ONEPOS_LOG_I("malloc onepos protocol error.");
        goto __exit ;
    }

    pos->lock = os_mutex_create("net_pos_lock", 1);
    if (pos->lock == OS_NULL)
    {
        ONEPOS_LOG_E("create network position lock failed");
        goto __exit;
    }

    /* onepos protocol */
    prot = pos_app_using_onepos_prot();
    if(prot)
    {
        os_mutex_recursive_lock(pos->lock, OS_WAIT_FOREVER);
        pos->prot = prot;
        os_mutex_recursive_unlock(pos->lock);
    }
    else
    {
        ONEPOS_LOG_E("net_pos using onepos protocol failed");
        goto __exit;
    }

    net_pos_init_pub_topic(pos);
    net_pos_init_sub_topic(pos);

    if (!PROT_IS_CONN(pos->prot))
    {
        ONEPOS_LOG_W("onepos protocol is not ready");
        onepos_prot_connect(pos->prot);
    }

    onepos_set_server_sta(ONEPOS_SIG_RUNING); 

    if (!net_pos_func(pos))
    {
        ONEPOS_LOG_E("this location is error!");
    }
    else
    {
        /* wait this position rec pos */
        os_task_msleep(ONEPOS_WAIT_MQTT_READY * 20);    
    }

    __exit:
        onepos_set_server_sta(ONEPOS_CLOSING);
        onepos_net_pos_exit(pos);

        return ;
}

/**
 ***********************************************************************************************************************
 * @brief           parse position result
 *
 * @param[in]       onepos_pos_t            to save the position result
 * @param[out]      data_item               position info(json format)
 *
 * @return          parse result
 * @retval          OS_EOK          parse is successful
 * @retval          OS_ERROR        paese is failed
 ***********************************************************************************************************************
 */
static os_err_t net_pos_parse_pos(onepos_pos_t *pos_src, cJSON *data_item)
{
    cJSON              *item        = OS_NULL;
    os_err_t            ret         = OS_EOK;
    static os_uint64_t  latest_time = 0;

    if (pos_src && data_item)
    {
        item = cJSON_GetObjectItem(data_item, "at");
        if(item)
        {
            if (latest_time >= item->valuedouble)
            {
                ONEPOS_LOG_E("error time stamp");
                return OS_ERROR;
            }
            else
            {
                latest_time = item->valuedouble;
            }

            pos_src->time = (os_uint32_t)(item->valuedouble / 1000);
        }

        item = cJSON_GetObjectItem(data_item, "pos");
        if (item)
        {
            sscanf(item->valuestring, "%lf,%lf", &pos_src->lat_coordinate,
                &pos_src->lon_coordinate);
        }
        if (!IS_VAILD_LAT(pos_src->lat_coordinate))
        {
            ONEPOS_LOG_D("error lat_coordinate");
            return OS_ERROR;
        }

        if (!IS_VAILD_LON(pos_src->lon_coordinate))
        {
            ONEPOS_LOG_D("error lon_coordinate");
            return OS_ERROR;
        }
    }
    else
    {
        ONEPOS_LOG_E("input param is error!");
        ret = OS_ERROR;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           parse receive message
 *
 * @param[in]       pos_src    to save position result
 * @param[in]       data       received message
 * 
 * @return          parse result
 * @retval          OS_TRUE         parse is successful
 * @retval          OS_FALSE        paese is failed
 ***********************************************************************************************************************
 */
static os_bool_t net_pos_parse_rec_data(onepos_pos_t *pos_src, char *data)
{
    cJSON    *root      = OS_NULL;
    cJSON    *item      = OS_NULL;
    os_bool_t result    = OS_TRUE;

    if (!pos_src)
    {
        ONEPOS_LOG_E("error position data string: %s", data);
        return OS_FALSE;
    }

    root = cJSON_Parse(data);
    if (!root)
    {
        ONEPOS_LOG_E("error position data string: %s", data);
        return OS_FALSE;
    }

    item = cJSON_GetObjectItem(root, "err_code");
    if (ONEPOS_COMM_SUCC == item->valueint)
    {
        result = (net_pos_parse_pos(pos_src, root)) == OS_EOK ? OS_TRUE : OS_FALSE;
    } 
    else if (ONEPOS_NULL_POSITION == item->valueint)
    {
        ONEPOS_LOG_I("position result is NULL!");
        result = OS_FALSE;
    } 
    else if (ONEPOS_OVER_LIMIT == item->valueint)
    {
        ONEPOS_LOG_I("position server over call limit!");
        result = OS_FALSE;
    }
    else
    {
        ONEPOS_LOG_E("errcode is error!!!");
    }
    cJSON_Delete(root);

    return result;
}

#if defined(NET_POS_SUPP_REMOTE_CONF)
/**
 ***********************************************************************************************************************
 * @brief           parse receive config message
 *
 * @param[out]      config_id       save config id
 * @param[in]       data            received message
 *
 * @return          os_bool_t       parse is succ
 * @retval          OS_FALSE        parse is failed
 * @retval          OS_TRUE         parse is succ
 ***********************************************************************************************************************
 */
static os_bool_t net_pos_parse_conf_msg(os_int32_t *config_id, const char *data)
{
    cJSON       *root = OS_NULL;
    cJSON       *item = OS_NULL;
    os_bool_t    result = OS_TRUE;

    root = cJSON_Parse((const char*)data);
    if (!root)
    {
        ONEPOS_LOG_E("error config string: %s", data);
        result = OS_FALSE;
        goto __exit;
    }

    item = cJSON_GetObjectItem(root, "config_id");
    if (config_id && item)
    {
        *config_id = item->valueint;
        if ((os_int32_t)*config_id < 0u || *config_id > OS_UINT32_MAX)
        {
            ONEPOS_LOG_E("error config id is : %d!", *config_id);
            result = OS_FALSE;
            goto __exit;
        }
        else
            ONEPOS_LOG_D("config id is : %d!", *config_id);

        item = cJSON_GetObjectItem(root, "interval");
        if (item && onepos_set_pos_interval(item->valueint))
        {
            ONEPOS_LOG_D("interval is : %d!", item->valueint);
        }
        else
        {
            ONEPOS_LOG_E("error interval is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }
        
        item = cJSON_GetObjectItem(root, "pos_error");
        if (item && onepos_set_pos_err(item->valueint))
        {
            ONEPOS_LOG_D("pos_error is : %d!", item->valueint);
        }
        else
        {
            ONEPOS_LOG_E("error pos_error is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }

        item = cJSON_GetObjectItem(root, "rept_type");
        if (item && IS_VAILD_SEV_TYPE(item->valueint) &&
            onepos_set_server_type((onepos_serv_type_t)item->valueint))

        {
          ONEPOS_LOG_D("rept_type is : %d!", item->valueint);
        }
        else
        {
            ONEPOS_LOG_E("error rept_type is : %d!", item->valueint);
            result = OS_FALSE;
            goto __exit;
        }

        item = cJSON_GetObjectItem(root, "pos_on");
        if (item)
        {
            ONEPOS_LOG_D("pos_on is : %d!", item->valueint);
            if (0 == item->valueint)
            {
                onepos_start_server();
            }
            else if (1 == item->valueint)
            {
                onepos_stop_server();
            }
            else
            {
                ONEPOS_LOG_E("error pos_on is : %d!", item->valueint);
                result = OS_FALSE;
                goto __exit;
            }
        }
        else
        {
            result = OS_FALSE;
            goto __exit;
        }
    }

    else
    {
        result = OS_FALSE;
    }

    __exit:
        cJSON_Delete(root);
        return result;
}

/**
 ***********************************************************************************************************************
 * @brief           reply config message
 *
 * @param[in]       conf_id       config id
 * @param[in]       conf_ret        config result
 ***********************************************************************************************************************
 */
void net_pos_conf_reply(onepos_net_pos_t *pos, os_uint32_t conf_id, os_bool_t conf_ret)
{
    cJSON     *reply_json = OS_NULL;
    char      *reply_str  = OS_NULL;

    onepos_conf_err_code_t err_code = conf_ret ? ONEPOS_CONFIG_SUCC : ONEPOS_CONFIG_FAIL;

    OS_ASSERT(OS_NULL != pos);

    reply_json = cJSON_CreateObject();
    if (reply_json)
    {
        cJSON_AddItemToObject(reply_json, "config_id",
                              cJSON_CreateNumber(conf_id));
        cJSON_AddItemToObject(reply_json, "err_code",
                              cJSON_CreateNumber((double)err_code));
        reply_str = cJSON_Print(reply_json);
        cJSON_Delete(reply_json);
        if (reply_str)
        {
            if(OS_EOK != onepos_mqtt_msg_publish(pos->prot, pos->pub_topic[CONF_PUB_TOPIC], reply_str, strlen(reply_str)))
            {
                ONEPOS_LOG_E("response config : %d failed", conf_id);
            }
            clean_net_pos_msg(reply_str);
        }
        else
        {
            ONEPOS_LOG_E("config message reply string is null!");
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           will call this function while config message arrived
 *
 * @param[in]       data            subscribed message info.
 ***********************************************************************************************************************
 */
static void net_pos_cfg_come_cb_func(void *handle, const char *topic_name, void *payload, size_t payload_len)
{
    os_bool_t  conf_ret = OS_FALSE;
    os_int32_t conf_id  = -1;

#if defined(ONEPOS_NET_POS_DEBUG)
    os_kprintf("rec_conf: %s\r\n", (char*)payload);
#endif

    net_pos_parse_conf_msg(&conf_id, (const char*)payload);

    net_pos_conf_reply(&g_net_pos, conf_id, conf_ret);
    onepos_rep_net_pos_sta(&g_net_pos);
}
#endif

/**
 ***********************************************************************************************************************
 * @brief           report status information of device to onepos platform
 ***********************************************************************************************************************
 */
void onepos_rep_net_pos_sta(onepos_net_pos_t *pos) 
{
    char rep_str[128] = {0,};

    OS_ASSERT(OS_NULL != pos);

    /* if onepos protocol is not ready will not report status */
    if(OS_NULL == pos->prot)
    {
        ONEPOS_LOG_I("could not report onepos network position status while server closing");
        return ;
    }

        memset(rep_str, 0, 128);

        sprintf(rep_str,
                "{\"payload\":{\"pos_on\":%u,\"interval\":%u,"
                "\"rept_type\":%u}}",
        onepos_get_server_sta() == ONEPOS_CLOSING ? 1u : 0u,
        onepos_get_pos_interval(),
        onepos_get_server_type());

        onepos_mqtt_msg_publish(pos->prot, pos->pub_topic[CONF_PUB_TOPIC], rep_str, strlen(rep_str));
}

/**
 ***********************************************************************************************************************
 * @brief           will call this function while position result message arrived
 *
 * @param[in]       data            message info
 ***********************************************************************************************************************
 */
static void net_pos_msg_come_cb_func(void *handle, const char *topic_name, void *payload, size_t payload_len)
{
    onepos_pos_t pos_data = {0,};

    memset(&pos_data, 0, sizeof(onepos_pos_t));

#if defined(ONEPOS_NET_POS_DEBUG)
    onepos_msg_dbg_show("rec_msg", payload, payload_len);
#endif
    if(net_pos_parse_rec_data(&pos_data, (char*)payload))
    {
        #if defined(ONEPOS_NET_POS_DEBUG)
        onepos_info_print(&pos_data);
        #endif

        os_mutex_recursive_lock(g_net_pos.lock, OS_WAIT_FOREVER);
        memcpy(g_net_pos.pos_result, &pos_data, sizeof(onepos_pos_t));
        os_mutex_recursive_unlock(g_net_pos.lock);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           init device of onepos used
 *
 * @return          result of init device
 * @retval          OS_EOK          init device successful
 * @retval          OS_ERROR        init device failed
 ***********************************************************************************************************************
 */
static os_err_t onepos_net_pos_init_dev(void)
{
    os_err_t ret = OS_EOK;

#if defined(ONEPOS_WIFI_POS)
    if (OS_EOK != onepos_init_wifi_dev())

    {
        ONEPOS_LOG_E("init wifi device is error");
        ret = OS_ERROR;
    }
#endif

#if defined(ONEPOS_CELL_POS)
    if ((OS_EOK == ret) && (OS_EOK != onepos_init_cell_dev()))
    {
        ONEPOS_LOG_E("init cell module is error");
        ret = OS_ERROR;
    }
#endif

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           free network postion publish message space
 *
 * @param[in]       msg_str       message space to free
 ***********************************************************************************************************************
 */
void clean_net_pos_msg(char *msg_str)
{
    if (msg_str)
    {
        os_free(msg_str);
        msg_str = OS_NULL;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           init device of onepos network postion used
 *
 * @return          result of init device
 * @retval          OS_EOK          init device successful
 * @retval          OS_ERROR        init device failed
 ***********************************************************************************************************************
 */
os_bool_t onepos_net_pos_get_dev_sta(void)
{
  os_err_t ret = OS_EOK;

#if defined(ONEPOS_WIFI_POS)
    if (OS_TRUE != onepos_get_wifi_sta())
    {
        ONEPOS_LOG_E("wifi device status is not ready");
        ret = OS_ERROR;
    }
#endif

#if defined(ONEPOS_CELL_POS)
    if ((OS_EOK == ret) && (OS_EOK != onepos_get_cell_sta()))
    {
        ONEPOS_LOG_E("cell module is not ready");
        ret = OS_ERROR;
    }
#endif

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           run onepos position
 *
 * @param[in]       pos             onepos network position
 *
 * @return          location succ or not
 * @retval          os_true         location successful
 * @retval          os_false        location failed
 ***********************************************************************************************************************
 */
os_bool_t net_pos_func(onepos_net_pos_t *pos)
{
    char      *pub_msg      = OS_NULL;
    os_bool_t  ret          = OS_TRUE;    
    cJSON     *json_src     = OS_NULL;
    
    OS_ASSERT(OS_NULL != pos);

    json_src = cJSON_CreateObject();
    if(!json_src)
    {
        ONEPOS_LOG_E("creat onepos pulish massage json is NULL");
        return OS_FALSE;
    }
#if defined(ONEPOS_WIFI_POS)
    if(OS_EOK != onepos_wifi_pos_pub_msg(json_src))
    {
        ONEPOS_LOG_E("onepos wifi massage is ERROR.");
        ret = OS_FALSE;
        goto __exit;
    }
#endif
#if defined(ONEPOS_CELL_POS)
    if(OS_EOK != cell_pos_pub_msg(json_src))
    {
        ONEPOS_LOG_E("onepos cell massage is ERROR.");
        ret = OS_FALSE;
        goto __exit;
    }

#endif

    pub_msg = cJSON_Print(json_src);

    if (pub_msg) 
    {

#if defined(ONEPOS_NET_POS_DEBUG)
    #define     TEMP_DEBUG_BUFF_LEN    (64)
    char        temp[TEMP_DEBUG_BUFF_LEN + 1]  = {0,};
    os_uint32_t cnt        = 0;

    while(cnt < strlen(pub_msg))
    {
        memset(temp, 0, sizeof(temp));

        if((strlen(pub_msg)) - cnt > TEMP_DEBUG_BUFF_LEN)
        {
            memcpy(temp, (pub_msg + cnt), TEMP_DEBUG_BUFF_LEN);
            temp[TEMP_DEBUG_BUFF_LEN] = '\0';
            cnt += TEMP_DEBUG_BUFF_LEN;
            os_kprintf("%s", temp);
        }
        else
        {
            memcpy(temp, (pub_msg + cnt), strlen(pub_msg) - cnt);
            temp[strlen(pub_msg) - cnt + 1] = '\0';
            cnt = strlen(pub_msg);
            os_kprintf("%s\r\n", temp);
        }
    }
#endif
        if (OS_EOK != onepos_mqtt_msg_publish(pos->prot, pos->pub_topic[POS_MSG_PUB_TOPIC], pub_msg, strlen(pub_msg)))
        {
            ONEPOS_LOG_E("mqtt publish message is ERR!");
            ret = OS_FALSE;
        }
		goto __exit;
    }
    else 
    {
        ONEPOS_LOG_E("pub_msg is NULL!");
    }

__exit:
    if(OS_NULL != json_src)
        cJSON_Delete(json_src);
    if(OS_NULL != pub_msg)
    {
        clean_net_pos_msg(pub_msg);
        pub_msg = OS_NULL;
    }
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           start onepos server
 ***********************************************************************************************************************
 */
void onepos_start_server(void)
{
    if (ONEPOS_CLOSING == onepos_get_server_sta())
    {
        if(OS_EOK != onepos_net_pos_init_dev())
        {
            ONEPOS_LOG_E("network position device init failed");
            return;
        }
        
        if (ONEPOS_SIG_RUN == onepos_get_server_type())
        {
            onepos_net_pos_sgl_run(&g_net_pos, &g_sys_net_pos_pos);
        }
        else
        {
            onepos_net_pos_circ_run(&g_net_pos, &g_sys_net_pos_pos);
        }
    }
}

/**
 ***********************************************************************************************************************
 * @brief           stop onepos server
 ***********************************************************************************************************************
 */
void onepos_stop_server(void)
{
    if (ONEPOS_RUNING == onepos_get_server_sta())
        onepos_set_server_sta(ONEPOS_WILL_CLOSE);
}

/**
 ***********************************************************************************************************************
 * @brief           get the lateset position of system
 *
 * @param[in]       src_info        save position data
 *
 * @return          os_bool_t       get position succ or fail
 * @retval          OS_FALSE        get failed
 * @retval          OS_TRUE         get successful
 ***********************************************************************************************************************
 */
os_bool_t onepos_get_latest_position(onepos_pos_t *src_info)
{
    os_bool_t         ret  = OS_FALSE;

    if (src_info)
    {
        if(ONEPOS_CLOSING == onepos_get_server_sta())
        {
            ret = copy_position(src_info, g_net_pos.pos_result);
        }
        else
        {
            os_mutex_recursive_lock(g_net_pos.lock, OS_WAIT_FOREVER);
            ret = copy_position(src_info, g_net_pos.pos_result);
            os_mutex_recursive_unlock(g_net_pos.lock);
        }
    }
    else
    {
        ONEPOS_LOG_E("src_info is NULL!");
        ret = OS_FALSE;
    }

    return ret;
}
