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
 * @file        onepos_position_module.c
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-29   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <string.h>  
#include <os_mq.h>
#include <os_memory.h>                      
#include <os_errno.h>

#include "onepos_log.h"
#include "onepos_state.h"
#include "onepos_timer.h"
#include "onepos_common.h"
#include "onepos_data_cache.h"
#include "onepos_position_module.h"

#include "ope_position.h"

static os_mq_t                 *gs_onepos_position_mq;
static onepos_timer_t          *gs_onepos_position_timer;
static ope_ble_rssi_ap_t       *gs_position_internal_buff      = OS_NULL;
static os_uint32_t             *gs_position_internal_size_list = OS_NULL;



/**
  ***********************************************************************************************************************
  * @brief           onepos position task
  ***********************************************************************************************************************
  */
void onepos_position_task(void)
{   
    os_err_t init_successful                        = OS_EOK;
    os_err_t init_error                             = OS_ERROR;
    
    onepos_data_cache_t           *data_cache       = OS_NULL;
    ope_general_ble_rssi_config_t *algorithm_handle = OS_NULL;
    os_int32_t  data_num                            = -1;
    os_uint32_t single_node_max_size                = 0;

    os_err_t    ret;
    os_err_t    algo_ret                            = OPE_EERROR;

    ope_ble_rssi_ap_frame_t        assembled_frame;
    onepos_state_position_res_t    position_result;
    ope_position_coordinate_result algo_position_result;
        
    //onepos_state_config_param_t config_param;
    //memset(&config_param, 0, sizeof(onepos_state_config_param_t));

    /* 0.init the ble_algo handle */
    algorithm_handle = ope_general_ble_rssi_config(CACHE_MEM_NUM);    
    if(OS_NULL == algorithm_handle)
    {
        ONEPOS_LOG_E("ope_general_ble_rssi_config() failed!");
        os_mq_send(gs_onepos_position_mq, &init_error, sizeof(init_error), OS_WAIT_FOREVER);
    }
    onepos_state_set_one_handle(POS_ENGINE, (void*)algorithm_handle);
    
    /* 1.get the cache handle */
    onepos_state_get_one_handle(DATA_CACHE, (void**)&data_cache);
    if(data_cache != OS_NULL)
    {
        gs_position_internal_buff = (ope_ble_rssi_ap_t*)os_malloc(data_cache->queue->node_num_max * data_cache->queue->node_size_max);

        if (gs_position_internal_buff == OS_NULL)
        {
            ONEPOS_LOG_E("position_internal_buff malloc failed!");
            os_mq_send(gs_onepos_position_mq, &init_error, sizeof(init_error), OS_WAIT_FOREVER);
        }
                
        gs_position_internal_size_list = (os_uint32_t*)os_malloc(data_cache->queue->node_num_max * sizeof(os_uint32_t));
        if (gs_position_internal_size_list == OS_NULL)
        {
            ONEPOS_LOG_E("position_internal_buff malloc failed!");
            os_mq_send(gs_onepos_position_mq, &init_error, sizeof(init_error), OS_WAIT_FOREVER);
        }
    }
    else
    {
        ONEPOS_LOG_E("data_cache acquisition failed!");
        os_mq_send(gs_onepos_position_mq, &init_error, sizeof(init_error), OS_WAIT_FOREVER);    
    }

    /* 2.send position_task startup successful message */
    os_mq_send(gs_onepos_position_mq, &init_successful, sizeof(init_successful), OS_WAIT_FOREVER);
    
    while(1)
    {
        /* 3.get position task right */
        onepos_get_positioning_thread_right();
        
  
        /* 4.redefine position interval */
        onepos_timer_set_delaytime(gs_onepos_position_timer, onepos_ble_get_position_interval());


        /* 5.start the timer */
        onepos_timer_start(gs_onepos_position_timer);

        
        /* 6.get data from cache */       
        ret = onepos_data_cache_fetch_all(data_cache, 
                                          gs_position_internal_buff, 
                                          &data_num, 
                                          &single_node_max_size, 
                                          gs_position_internal_size_list);

        if(OS_EOK == ret)
        {
            assembled_frame.ap_num = data_num;
            assembled_frame.ap = gs_position_internal_buff;
            algo_ret = ope_general_ble_rssi_location(assembled_frame, algorithm_handle, &algo_position_result);


            /* 7.save the positioning result */                                 /* | change_pos | */
            if(OPE_EOK == algo_ret)
            {
                position_result.lat =  algo_position_result.lat;
                position_result.lon =  algo_position_result.lon;
                onepos_state_set_result(&position_result);
            }
            else
            {
                ONEPOS_LOG_I("positioning failed, algo_ret = %d.", algo_ret);
            }            
        }                   
        else if(POS_WEMPTY == ret)
        {
            /* | change_pos | 如果 data_num == 0:证明没读到数据, 此时应该输出什么结果？*/  
            ONEPOS_LOG_I("no new cache data to fetch.");
        }
        else
        {
            ONEPOS_LOG_W("error occured when fetching cache data, ret = %d.", ret);
        }

        
        /* 8.wait for the timer to end */
        ret = onepos_timer_wait(gs_onepos_position_timer);
        if(OS_EOK != ret)
        {
            ONEPOS_LOG_W("gs_onepos_position_timer innormal ret = %d : -2->OS_ETIMEOUT, -15->POS_ENULL.", ret);
        }

        
        /* 9.stop the timer */
        onepos_timer_stop(gs_onepos_position_timer);    

        
        /* 10.release the position task right */
        onepos_release_positioning_thread_right();  
    }   
}


/**
  ***********************************************************************************************************************
  * @brief            create a position module
  *
  * @return           the position task handle
  * @retval           OS_NULL      failed
  ***********************************************************************************************************************
  */
os_task_t *onepos_position_module_create(void)
{
    os_err_t     msg;
    os_size_t    recv_size;
    os_task_t   *position_task      = OS_NULL;  
    gs_onepos_position_mq           = OS_NULL;
    gs_onepos_position_timer        = OS_NULL;
    
    /* 1.create the timer */
    gs_onepos_position_timer = onepos_timer_create("onepos_position_timer", onepos_ble_get_position_interval());
    if(OS_NULL == gs_onepos_position_timer)
    {
        ONEPOS_LOG_E("OS_NULL == gs_onepos_position_timer.");
        goto exit;
    }


    /* 2.create the msg queue */
    gs_onepos_position_mq = os_mq_create("onepos_position_mq", sizeof(os_err_t), 1);
    if(OS_NULL == gs_onepos_position_mq)
    {
        ONEPOS_LOG_E("OS_NULL == gs_onepos_position_mq.");
        goto exit;
    } 


    /* 3.create position task */
    position_task = os_task_create("onepos_position_task", (void*)onepos_position_task, NULL, 2048, 25);   
    if(OS_NULL == position_task)
    {
        ONEPOS_LOG_E("OS_NULL == position_task.");
        goto exit;
    } 

    
    /* 4.strt up the position task */                                                                    
    os_task_startup(position_task);


    /* 5.blocking the message queue */
    if(OS_EOK != os_mq_recv(gs_onepos_position_mq, &msg, sizeof(os_err_t), OS_TICK_PER_SECOND, &recv_size) 
       || OS_EOK != msg)
    {
        ONEPOS_LOG_E(" os_mq_recv(gs_onepos_position_mq, &msg, sizeof(os_err_t), OS_TICK_PER_SECOND, &recv_size): error.");
        goto exit;
    }

    ONEPOS_LOG_I("onepos_position_module_create() succeed."); //onepos_debug
    return position_task;
       
exit:
    
    if(OS_NULL != position_task)
    {
        os_task_destroy(position_task);
        position_task = OS_NULL;
        onepos_state_set_one_handle_null(POSITION_TASK, (void*)position_task);
    }    
    
    if(OS_NULL != gs_onepos_position_timer)
    {
        onepos_timer_destroy(gs_onepos_position_timer);
    }

    if(OS_NULL != gs_onepos_position_mq)
    {
        os_mq_destroy(gs_onepos_position_mq);
    }   

    if(OS_NULL != gs_position_internal_buff)
    {
        os_free(gs_position_internal_buff);
    }
    
    if(OS_NULL != gs_position_internal_size_list)
    {
        os_free(gs_position_internal_size_list);
    } 
    
    return OS_NULL;
}


/**
  ***********************************************************************************************************************
  * @brief            destroy the position module 
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_ESTATE   
  ***********************************************************************************************************************
  */
os_err_t onepos_position_module_destroy(void)
{   
    os_task_t  *position_task   = OS_NULL;
    ope_general_ble_rssi_config_t *algorithm_handle = OS_NULL;
    
    if(OS_EOK != onepos_state_get_one_handle(POSITION_TASK, (void**)&position_task) 
       || OS_NULL == position_task)
    {
        ONEPOS_LOG_E("OS_EOK != onepos_state_get_one_handle(POSITION_TASK, (void**)&position_task): error.");
        return POS_ESTATE;
    }
       
    if(OS_EOK != onepos_state_get_one_handle(POS_ENGINE, (void**)&algorithm_handle) 
       || OS_NULL == algorithm_handle)
    {
        ONEPOS_LOG_E("OS_EOK != onepos_state_get_one_handle(POS_ENGINE, (void**)&algorithm_handle): error.");
        return POS_ESTATE;
    }

    /* Once the positioning thread right obtained, means one round positioning is over. It is time to end.*/
    if(onepos_get_positioning_thread_right() != OS_EOK)
    {
        return POS_ESTATE;
    }
    
    os_task_destroy(position_task);
    os_mq_destroy(gs_onepos_position_mq);
    onepos_timer_destroy(gs_onepos_position_timer);
    os_free(gs_position_internal_buff);
    os_free(gs_position_internal_size_list);        
    ope_general_ble_rssi_exit(algorithm_handle);
    
    position_task = OS_NULL;
    onepos_state_set_one_handle_null(POSITION_TASK, (void*)position_task);   

    algorithm_handle = OS_NULL;
    onepos_state_set_one_handle_null(POS_ENGINE, (void*)algorithm_handle);  
    
    if(OS_EOK != onepos_release_positioning_thread_right())
    {
        return POS_ESTATE;
    }
    
    ONEPOS_LOG_I("onepos_position_module_destroy() succeed.");   //onepos_debug
    return OS_EOK;
}

