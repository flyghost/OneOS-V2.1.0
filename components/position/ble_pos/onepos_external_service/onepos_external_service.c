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
 * @file        onepos_external_service.c
 *
 * @brief       onepos user interface
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-29   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <string.h>   
#include <os_sem.h>
#include <os_errno.h>

#include "onepos_log.h"
#include "onepos_state.h"
#include "onepos_common.h"
#include "onepos_position_module.h"
#include "onepos_external_service.h"
#include "onepos_hardware_control_module.h"

#include <oneos_config.h>


#ifndef ONEPOS_BLE_POSITION_INTERVAL_HIGH
    #define    ONEPOS_BLE_POSITION_INTERVAL_HIGH    2000
#endif /* ONEPOS_BLE_POSITION_INTERVAL_HIGH */


#ifndef ONEPOS_BLE_POSITION_INTERVAL_LOW
    #define    ONEPOS_BLE_POSITION_INTERVAL_LOW     500
#endif /* ONEPOS_BLE_POSITION_INTERVAL_LOW */


/**
  ***********************************************************************************************************************
  * @brief           get positioning interval 
  * @return          the positioning interval
  ***********************************************************************************************************************
  */
os_uint32_t onepos_ble_obtain_position_interval(void)
{
    return onepos_ble_get_position_interval();
}


/**
  ***********************************************************************************************************************
  * @brief            start ble position 
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           OS_ERROR  
  ***********************************************************************************************************************
  */
os_err_t onepos_ble_position_start(void)
{
    if(OS_TRUE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos positioning service has been initialized!");
        return POS_EINIT;
    }

    os_err_t           ret;
    os_task_t         *hardware_task = OS_NULL;
    os_task_t         *position_task = OS_NULL;
    
    
    onepos_state_position_res_t invaild_reslut;
    
    ret = onepos_state_init();
    memset(&invaild_reslut, 0, sizeof(onepos_state_position_res_t));
    onepos_state_set_result(&invaild_reslut);
  
    if(OS_EOK != ret)
    {
        ONEPOS_LOG_E("onepos_state_init error! ret = %d, -11->POS_ESEM  -12->POS_EINIT.", ret);
        goto exit;
    }
    
    /* create hardware module first */
    hardware_task = onepos_hardware_control_module_create();
    if(OS_NULL == hardware_task)
    {
        ONEPOS_LOG_E("onepos_hardware_control_module_create error!");
        goto exit;
    }  
    onepos_state_set_one_handle(HARDWARE_TASK, (void*)hardware_task);

    
    /* then create position module */
    position_task = onepos_position_module_create();
    if(OS_NULL == position_task)
    {
        ONEPOS_LOG_E("onepos_position_module_create error!");
        goto exit;
    }     
    onepos_state_set_one_handle(POSITION_TASK, (void*)position_task);
    
    ONEPOS_LOG_I("onepos_ble_position_start() succeed.");  //onepos_debug
    return OS_EOK;
    
exit:  
    if(OS_NULL != position_task)
    {
        onepos_position_module_destroy();
    }
    
    if(OS_NULL != hardware_task)
    {
        onepos_hardware_control_module_destroy();
    } 
    
    ret = onepos_state_deinit();
    if(OS_EOK != ret)
    {
        ONEPOS_LOG_W("onepos_state_deinit failed, ret = %d.", ret);
    }

    return OS_ERROR;
}


/**
  ***********************************************************************************************************************
  * @brief            exit ble position 
  *
  * @return           the operation result
  * @retval           OS_EOK
  * @retval           POS_EINIT
  * @retval           POS_EFREE    
  ***********************************************************************************************************************
  */
os_err_t onepos_ble_position_exit(void)
{
    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state is not initialized, onepos_ble_position_exit is unnecessary.");
        return POS_EINIT;
    }

    os_err_t ret;
    onepos_state_handle_t obtained_result;
    memset(&obtained_result, 0, sizeof(onepos_state_handle_t));
       
    ret = onepos_state_get_all_handle(&obtained_result);
    if(OS_EOK != ret)
    {
        ONEPOS_LOG_W("onepos_state_get_all_handle failed,ret = %d.", ret);
        return ret;
    }
    
    /* destroy position module first */
    if(OS_NULL != obtained_result.position_task)
    {
        onepos_position_module_destroy();
    }
    
    /* then destroy hardware module */
    if(OS_NULL != obtained_result.hardware_control_task)
    {
        onepos_hardware_control_module_destroy();
    }
    
    ret = onepos_state_deinit();
    if(OS_EOK != ret)
    {
        ONEPOS_LOG_W("onepos_state_deinit failed,ret = %d.", ret);
    }

    ONEPOS_LOG_I("onepos_ble_position_exit() ret = %d, 0:OS_EOK.", ret);   //onepos_debug
    return ret;
}


/**
  ***********************************************************************************************************************
  * @brief           calc scan interval based on position interval that is needed, then set the both interval time
  *                  
  * @param[in]       position_interval  (unit:1ms)
  *
  * @return          operation result
  ***********************************************************************************************************************
  */
os_err_t onepos_ble_position_change_position_interval(os_uint32_t position_interval)
{
    os_uint32_t scan_interval;

    if(OS_FALSE == onepos_state_is_initialized())
    {
        if(position_interval <= ONEPOS_BLE_POSITION_INTERVAL_HIGH && position_interval >= ONEPOS_BLE_POSITION_INTERVAL_LOW)
        {
            scan_interval = onepos_ble_position_calc_scan_interval(position_interval); 
            return onepos_state_set_position_scan_interval_off(position_interval, scan_interval);
        }
        else
        {
            ONEPOS_LOG_I("ONEPOS_BLE_POSITION_INTERVAL not in range, low_bound = %d, high bound = %d, will proceed dafault setup.",
                         ONEPOS_BLE_POSITION_INTERVAL_LOW,  ONEPOS_BLE_POSITION_INTERVAL_HIGH);
            return onepos_state_set_position_scan_interval_off(1000, 800);  
        };
    }
    else
    {
        if(position_interval <= ONEPOS_BLE_POSITION_INTERVAL_HIGH && position_interval >= ONEPOS_BLE_POSITION_INTERVAL_LOW)
        {
            scan_interval = onepos_ble_position_calc_scan_interval(position_interval);           
            return onepos_state_set_position_scan_interval_on(position_interval, scan_interval);
        }
        else
        {
            ONEPOS_LOG_I("ONEPOS_BLE_POSITION_INTERVAL not in range, low_bound = %d, high bound = %d, will proceed dafault setup.",
                         ONEPOS_BLE_POSITION_INTERVAL_LOW,  ONEPOS_BLE_POSITION_INTERVAL_HIGH);
            return onepos_state_set_position_scan_interval_on(1000, 800);  
        }
    }
}


/**
  ***********************************************************************************************************************
  * @brief           get positioning result
  *
  * @param[in]       res(provided by user)
  *
  * @return          the operation result
  * @retval          OS_EOK
  * @retval          POS_ENULL
  * @retval          POS_EINIT  
  ***********************************************************************************************************************
  */
os_err_t onepos_ble_position_get_res(onepos_position_res_t *res)
{
    if(OS_NULL == res)
    {
        return POS_ENULL;
    }

    onepos_state_position_res_t invaild_reslut;
    memset(&invaild_reslut, 0, sizeof(onepos_state_position_res_t));

    if(OS_FALSE == onepos_state_is_initialized())
    {
        ONEPOS_LOG_W("onepos state is not initialized! will return invaild_result.");
        onepos_state_set_result(&invaild_reslut);
        return POS_EINIT;
    }

    onepos_state_get_result((onepos_state_position_res_t *)res);
    
    return OS_EOK;
}

