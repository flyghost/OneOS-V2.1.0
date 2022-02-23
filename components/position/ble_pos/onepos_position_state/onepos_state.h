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
 * @file        onepos_state.h
 *
 * @brief       onepos service status control
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-27   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ONEPOS_STATE_H__
#define __ONEPOS_STATE_H__

#include <os_types.h>    //
#include <os_sem.h>      //
#include <os_task.h>     //
#include "onepos_data_cache.h"
#include "ope_position.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef struct
{
    os_uint32_t scan_interval;                       /*  uint: 0.625ms */
    os_uint32_t scan_window;                         /*  uint: 0.625ms */
    os_uint32_t position_interval_min;               /*  uint:     1ms */
    os_uint32_t frame_beacon_mun_max;     
    os_bool_t   history_valid_flag;
    double      history_valid_thr;
}onepos_state_config_param_t;


typedef struct
{
    os_sem_t   *state_reader;
    os_sem_t   *running_state;
    os_uint32_t state_init_mark;                      
}onepos_state_control_t;


/* The order must be the same as onepos_state_handle_type_t */
typedef struct
{
    ope_general_ble_rssi_config_t  *ble_algo;             
    onepos_data_cache_t            *data_cache;                  
    os_task_t                      *position_task;
    os_task_t                      *hardware_control_task;
}onepos_state_handle_t;


typedef struct
{
    double lat;
    double lon;
}onepos_state_position_res_t;


#pragma pack(4)
typedef struct
{
    onepos_state_config_param_t config_param;  
    onepos_state_control_t      state_control; 
    onepos_state_handle_t       handle;        
    onepos_state_position_res_t position_res;  
}onepos_state_t;
#pragma pack()


typedef enum
{
    STATE_HANDLE_HEAD   = 0,
    POS_ENGINE          = 1,
    DATA_CACHE          = 2,
    POSITION_TASK       = 3,
    HARDWARE_TASK       = 4,
    /*add handle above STATE_HANDLE_BUTTOM */
    STATE_HANDLE_BUTTOM
}onepos_state_handle_type_t;


extern os_uint32_t onepos_ble_get_position_interval(void);
extern os_bool_t   onepos_state_is_initialized(void);
extern os_uint32_t onepos_ble_position_calc_scan_interval(os_uint32_t position_interval);
extern os_uint32_t onepos_ble_position_calc_scan_window(os_uint32_t scan_interval);
extern os_err_t    onepos_state_init(void);
extern os_err_t    onepos_state_deinit(void);
extern os_err_t    onepos_state_get_config_param(onepos_state_config_param_t *config_param);
extern os_err_t    onepos_state_set_position_scan_interval_on(os_uint32_t postition_interval, os_uint32_t scan_interval);
extern os_err_t    onepos_state_set_position_scan_interval_off(os_uint32_t postition_interval, os_uint32_t scan_interval);
extern os_err_t    onepos_state_get_one_handle(onepos_state_handle_type_t handle_type, void **handle);
extern os_err_t    onepos_state_get_all_handle(void *handles_head);
extern os_err_t    onepos_state_set_one_handle(onepos_state_handle_type_t handle_type, void *handle);
extern os_err_t    onepos_state_set_one_handle_null(onepos_state_handle_type_t handle_type, void *handle);
extern os_err_t    onepos_state_get_result(onepos_state_position_res_t *res);
extern os_err_t    onepos_state_set_result(onepos_state_position_res_t *res);
extern os_err_t    onepos_get_positioning_thread_right(void);
extern os_err_t    onepos_release_positioning_thread_right(void);


#ifdef __cplusplus
}
#endif

#endif /*__ONEPOS_STATE_H__ */

