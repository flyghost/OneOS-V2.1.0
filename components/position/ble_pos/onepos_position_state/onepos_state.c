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
 * @file        onepos_state.c
 *
 * @brief       onepos service status control
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-27   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <dlog.h>
#include <string.h>
#include <os_sem.h>
#include <os_memory.h>
#include <os_errno.h> 

#include "onepos_log.h"
#include "onepos_state.h"
#include "onepos_common.h"
#include "onepos_external_service.h"

#include <oneos_config.h>

#define  INITIALIZED               0X5A555AAA
#define  UNINITIALIZED             0X12121212

static os_uint32_t    gs_position_interval = ONEPOS_BLE_POSITION_INTERVAL;          /*  unit:  1ms */
static onepos_state_t gs_onepos_state_config;


/**
  ***********************************************************************************************************************
  * @brief           Confirm the initialization status
  *
  * @return          Whether the positioning state has been initialized
  ***********************************************************************************************************************
  */
os_bool_t onepos_state_is_initialized(void)
{
    if(gs_onepos_state_config.state_control.state_init_mark == INITIALIZED)
    {
        return OS_TRUE;
    }
    
    return OS_FALSE;
}


/**
  ***********************************************************************************************************************
  * @brief           get positioning interval
  * @return          the positioning interval
  ***********************************************************************************************************************
  */
os_uint32_t onepos_ble_get_position_interval(void)
{
    return gs_position_interval;
}


/**
  ***********************************************************************************************************************
  * @brief            set position scan interval in gs_onepos_state_config.config_param
  *                   *when the position_service is on
  * @param[in]        postition_interval(unit:      1ms) 
  * @param[in]        scan_interval     (unit:  0.625ms) 
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  ***********************************************************************************************************************
  */
os_err_t onepos_state_set_position_scan_interval_on(os_uint32_t postition_interval, os_uint32_t scan_interval)
{

    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not to set scan interval.");
        return POS_EINIT;
    }

    gs_position_interval = postition_interval;
    
    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   
    gs_onepos_state_config.config_param.scan_interval     = scan_interval;
    os_sem_post(gs_onepos_state_config.state_control.state_reader);
    
    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief            set position scan interval in gs_onepos_state_config.config_param
  *                   *when the position_service is off
  * @param[in]        postition_interval(unit:     1ms) 
  * @param[in]        scan_interval     (unit: 0.625ms) 
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  ***********************************************************************************************************************
  */
os_err_t onepos_state_set_position_scan_interval_off(os_uint32_t postition_interval, os_uint32_t scan_interval)
{

    if(OS_TRUE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has been initialized, can not to set scan interval.");
        return POS_EINIT;
    }

    gs_position_interval = postition_interval; 
    gs_onepos_state_config.config_param.scan_interval     = scan_interval;
     
    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief           calc scan interval based on position interval that is set by user
  *
  * @param[in]       position_interval                (unit:1ms)   
  *
  * @return          The value of the scan interval   (unit:0.625ms)
  ***********************************************************************************************************************
  */
os_uint32_t onepos_ble_position_calc_scan_interval(os_uint32_t position_interval)
{
    /* Calculation strategy can be changed */
    return (position_interval * 1000 ) / 625 / 2;    
}


/**
  ***********************************************************************************************************************
  * @brief           calc scan window based on scan interval
  *
  * @param[in]       scan interval                    (unit:0.625ms)   
  *
  * @return          The value of the scan window     (unit:0.625ms)
  ***********************************************************************************************************************
  */
os_uint32_t onepos_ble_position_calc_scan_window(os_uint32_t scan_interval)
{
    /* According to the power consumption target, calculation strategy can be changed */
    return scan_interval; 
}


/**
  ***********************************************************************************************************************
  * @brief           initialize the positioning state
  *
  * @param[in]       config_param   
  *
  * @return          the operation result
  * @retval          OS_EOK
  * @retval          POS_EINIT
  * @retval          POS_ESEM
  ***********************************************************************************************************************
  */
os_err_t onepos_state_init(void)
{
    os_sem_t   *sem;
    os_uint32_t scan_interval;          /* unit: 0.625ms */
    os_uint32_t scan_window;            /* unit: 0.625ms */
    
    if(OS_TRUE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has been initialized, no need to init again.");
        return POS_EINIT;
    }

    scan_interval = onepos_ble_position_calc_scan_interval(gs_position_interval);
    scan_window   = onepos_ble_position_calc_scan_window(scan_interval);
    
    gs_onepos_state_config.config_param.scan_interval         = scan_interval;
    gs_onepos_state_config.config_param.scan_window           = scan_window;
    gs_onepos_state_config.config_param.frame_beacon_mun_max  = 0;
    gs_onepos_state_config.config_param.history_valid_flag    = 0;
    gs_onepos_state_config.config_param.history_valid_thr     = 0;
    gs_onepos_state_config.config_param.position_interval_min = 0;
    
    gs_onepos_state_config.state_control.state_reader   = OS_NULL;
    gs_onepos_state_config.state_control.running_state  = OS_NULL;
    gs_onepos_state_config.handle.ble_algo              = OS_NULL;               
    gs_onepos_state_config.handle.data_cache            = OS_NULL;
    gs_onepos_state_config.handle.position_task         = OS_NULL;
    gs_onepos_state_config.handle.hardware_control_task = OS_NULL;


    sem = os_sem_create("onepos_state_reader", 1, SEM_VALUE_TO_MUTEX);
    if(OS_NULL == sem)
    {
        return POS_ESEM;
    }
    gs_onepos_state_config.state_control.state_reader = sem;


    sem = os_sem_create("onepos_running_state", 1, SEM_VALUE_TO_MUTEX);
    if(OS_NULL == sem)
    {
        os_sem_destroy(gs_onepos_state_config.state_control.state_reader);
        return POS_ESEM;
    }
    gs_onepos_state_config.state_control.running_state = sem;   


    gs_onepos_state_config.state_control.state_init_mark = INITIALIZED;

    return OS_EOK;  
}


/**
  ***********************************************************************************************************************
  * @brief           uninitialize the positioning state
  *
  * @return          the operation result
  * @retval          OS_EOK
  * @retval          POS_EFREE
  * @retval          POS_EINIT
  ***********************************************************************************************************************
  */
os_err_t onepos_state_deinit(void)
{
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, no need to deinit!");
        return POS_EINIT;
    }

    if(OS_NULL != gs_onepos_state_config.handle.hardware_control_task 
       || OS_NULL != gs_onepos_state_config.handle.position_task
       || OS_NULL != gs_onepos_state_config.handle.data_cache
       || OS_NULL != gs_onepos_state_config.handle.ble_algo)
    {
        return POS_EFREE;
    }


    if(OS_NULL != gs_onepos_state_config.state_control.state_reader)
    {
        os_sem_destroy(gs_onepos_state_config.state_control.state_reader);
        gs_onepos_state_config.state_control.state_reader = OS_NULL;
    }


    if(OS_NULL != gs_onepos_state_config.state_control.running_state)
    {
        os_sem_destroy(gs_onepos_state_config.state_control.running_state);
        gs_onepos_state_config.state_control.running_state = OS_NULL;
    }  


    gs_onepos_state_config.state_control.state_init_mark = UNINITIALIZED;
    
    return OS_EOK;  
}


/**
  ***********************************************************************************************************************
  * @brief            get gs_onepos_state_config.config_param content
  *
  * @param[out]       config_param
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_ENULL
  ***********************************************************************************************************************
  */
os_err_t onepos_state_get_config_param(onepos_state_config_param_t *config_param)
{
    if(OS_NULL == config_param)
    {
        return POS_ENULL;
    }
    
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not to get param.");
        return POS_EINIT;
    }

    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   
    memcpy(config_param, &gs_onepos_state_config.config_param, sizeof(onepos_state_config_param_t));
    os_sem_post(gs_onepos_state_config.state_control.state_reader);
    
    return OS_EOK;  
}


/**
  ***********************************************************************************************************************
  * @brief            get one control handle from gs_onepos_state_config.handle
  *
  * @param[in]        handle_type
  * @param[out]       handle               storage addr(provided by the user)
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_ENULL
  * @retval           OS_ERROR 
  ***********************************************************************************************************************
  */
os_err_t onepos_state_get_one_handle(onepos_state_handle_type_t handle_type, void **handle)
{
    if(OS_NULL == handle || handle_type <= STATE_HANDLE_HEAD || handle_type >= STATE_HANDLE_BUTTOM)
    {
        return POS_ENULL;
    }
    
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not get one handle.");
        return POS_EINIT;
    }
 
    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   
    
    switch (handle_type)
    {
        case POS_ENGINE:
            *handle = gs_onepos_state_config.handle.ble_algo;
            break;
        case DATA_CACHE:
            *handle = gs_onepos_state_config.handle.data_cache;
            break; 
        case POSITION_TASK:
            *handle = gs_onepos_state_config.handle.position_task;
            break;
        case HARDWARE_TASK:
            *handle = gs_onepos_state_config.handle.hardware_control_task;
            break; 
        default:
            os_sem_post(gs_onepos_state_config.state_control.state_reader);
            return OS_ERROR;
    }

    os_sem_post(gs_onepos_state_config.state_control.state_reader);
   
    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief            get all control handle from gs_onepos_state_config.handle
  *
  * @param[out]       handles_head           storage addr(provided by the user)
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_ENULL
  ***********************************************************************************************************************
  */
os_err_t onepos_state_get_all_handle(void *handles_head)
{
    if(OS_NULL == handles_head)
    {
        return POS_ENULL;
    }
    
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not get all the handles.");
        return POS_EINIT;
    }

    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   
    memcpy(handles_head, &gs_onepos_state_config.handle, sizeof(onepos_state_handle_t));
    os_sem_post(gs_onepos_state_config.state_control.state_reader);
    
    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief            set one control handle in gs_onepos_state_config.handle
  *
  * @param[in]        handle_type
  * @param[in]        handle
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_ENULL
  * @retval           OS_ERROR
  ***********************************************************************************************************************
  */
os_err_t onepos_state_set_one_handle(onepos_state_handle_type_t handle_type, void *handle)
{
    if(OS_NULL == handle || handle_type <= STATE_HANDLE_HEAD || handle_type >= STATE_HANDLE_BUTTOM)
    {
        return POS_ENULL;
    }
    
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not set one handle.");
        return POS_EINIT;
    }

    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   

    switch (handle_type)
    {
        case POS_ENGINE:
            gs_onepos_state_config.handle.ble_algo = handle;            
            break;
        case DATA_CACHE:
            gs_onepos_state_config.handle.data_cache = handle;            
            break; 
        case POSITION_TASK:
            gs_onepos_state_config.handle.position_task = handle;            
            break;
        case HARDWARE_TASK:
            gs_onepos_state_config.handle.hardware_control_task = handle;            
            break; 
        default:
            os_sem_post(gs_onepos_state_config.state_control.state_reader);
            return OS_ERROR;
    }

    os_sem_post(gs_onepos_state_config.state_control.state_reader);

    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief            set one control handle NULL in gs_onepos_state_config.handle
  *
  * @param[in]        handle_type
  * @param[in]        handle
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_ENULL
  * @retval           OS_ERROR
  ***********************************************************************************************************************
  */
os_err_t onepos_state_set_one_handle_null(onepos_state_handle_type_t handle_type, void *handle)
{
    if(OS_NULL != handle || handle_type <= STATE_HANDLE_HEAD || handle_type >= STATE_HANDLE_BUTTOM)
    {
        return POS_ENULL;
    }
    
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not make one handle NULL.");
        return POS_EINIT;
    }

    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   

    switch (handle_type)
    {
        case POS_ENGINE:
            gs_onepos_state_config.handle.ble_algo = handle;
            break;
        case DATA_CACHE:
            gs_onepos_state_config.handle.data_cache = handle;
            break; 
        case POSITION_TASK:
            gs_onepos_state_config.handle.position_task = handle;            
            break;
        case HARDWARE_TASK:
            gs_onepos_state_config.handle.hardware_control_task = handle;            
            break; 
        default:
            os_sem_post(gs_onepos_state_config.state_control.state_reader);
            return OS_ERROR;
    }

    os_sem_post(gs_onepos_state_config.state_control.state_reader);

    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief            get positioning result from gs_onepos_state_config.position_res
  *
  * @param[out]       res(provided by user)
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_ENULL
  ***********************************************************************************************************************
  */
os_err_t onepos_state_get_result(onepos_state_position_res_t *res)
{
    if(OS_NULL == res)
    {
        return POS_ENULL;
    }
    
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not get result.");
        return POS_EINIT;
    }

    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   
    memcpy(res, &gs_onepos_state_config.position_res, sizeof(onepos_state_position_res_t));
    os_sem_post(gs_onepos_state_config.state_control.state_reader);
    
    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief            set positioning result in gs_onepos_state_config.position_res
  *
  * @param[in]        res
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_ENULL
  ***********************************************************************************************************************
  */
os_err_t onepos_state_set_result(onepos_state_position_res_t *res)
{
    if(OS_NULL == res)
    {
        return POS_ENULL;
    }
    
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state has not been initialized, can not set result.");
        return POS_EINIT;
    }

    os_sem_wait(gs_onepos_state_config.state_control.state_reader, OS_WAIT_FOREVER);   
    memcpy(&gs_onepos_state_config.position_res, res, sizeof(onepos_state_position_res_t));
    os_sem_post(gs_onepos_state_config.state_control.state_reader);
    
    return OS_EOK;   
}


/**
  ***********************************************************************************************************************
  * @brief            get the right of the onepos_position thread(task)
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  ***********************************************************************************************************************
  */
os_err_t onepos_get_positioning_thread_right(void)
{
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos positioning service not started yet,can not obtain position_thread right!");
        return POS_EINIT;
    }

    os_sem_wait(gs_onepos_state_config.state_control.running_state, OS_WAIT_FOREVER);  
    return OS_EOK; 
}


/**
  ***********************************************************************************************************************
  * @brief            release the right of the onepos_position thread(task)
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  ***********************************************************************************************************************
  */
os_err_t onepos_release_positioning_thread_right(void)
{
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos positioning service not started yet,can not release position_thread right!");
        return POS_EINIT;
    }

    os_sem_post(gs_onepos_state_config.state_control.running_state); 
    return OS_EOK; 
}

