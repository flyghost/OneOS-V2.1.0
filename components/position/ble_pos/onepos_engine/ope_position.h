/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        ope_algorithm.h
 * 
 * @brief       Basic algorithm library 
 * 
 * @details     Provide the basic algorithm function required for positioning
 * 
 * @revision
 * Date         Author          Notes
 * 2021-05-01   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __OPE_POSITION_H__
#define __OPE_POSITION_H__

#include "ope_algorithm.h"

/**
 ***********************************************************************************************************************
 * @struct      ope_location_statue_t
 *      
 * @brief       Positioning state
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_uint32_t location_num;          /* Number of location */
    ope_uint32_t success_num;           /* Number of successful positioning */
    ope_uint32_t defeat_num;            /* Number of location failures */
    ope_uint32_t continue_success_num;  /* Successive number of successful positioning */
    ope_uint32_t continue_defeat_num;   /* Number of consecutive positioning failures */
    ope_uint32_t error_num;             /* Number of running errors */
    ope_uint32_t location_time;         /* Positioning time-consuming */
}ope_location_statue_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_position_coordinate_result
 *      
 * @brief       Positioning results
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_double_t lat;
    ope_double_t lon;
}ope_position_coordinate_result;

/**
 ***********************************************************************************************************************
 * @struct      ope_general_ble_rssi_config_t
 *      
 * @brief       Universal Bluetooth RSSI configuration parameter
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_ble_rssi_beacon_data_scope_filter_param_t beacon_data_scope_filter_param;
    ope_ble_rssi_beacon_data_reduction_param_t    beacon_data_reduction_param;
    ope_ble_rssi_statistics_beacon_selector_param beacon_selector_param;
    ope_ble_rssi_to_distance_param_t rssi_distance_param;
    ope_ble_rssi_kalman_param_t      kalman_param;
    ope_ble_rssi_trilateration_calc_param_t trilateration_calc_param;
    ope_location_statue_t            location_statue; /* Positioning state */
    ope_ble_rssi_selector_buf_t      selector_buf;    /* Beacon information within the selector */
    ope_position_coordinate_result   prev_result;     /* The positioning result of the previous frame */
    ope_float_t charac_weight[3];     /* weight coefficient */
    ope_uint32_t ap_num_max;          /* Maximum number of AP's allowed */
    ope_uint32_t valid_history_mark;  /* Historical data valid flag, 0: invalid, 1: valid */
    ope_void_t *tmp_buf;              /* Intermediate data cache space */
}ope_general_ble_rssi_config_t;

ope_general_ble_rssi_config_t *ope_general_ble_rssi_config(ope_uint32_t ap_num_max);
ope_err_t ope_general_ble_rssi_location(ope_ble_rssi_ap_frame_t         ap_frame,
                                        ope_general_ble_rssi_config_t  *config,
                                        ope_position_coordinate_result *position_result);
ope_err_t ope_general_ble_rssi_exit(ope_general_ble_rssi_config_t *config);

#endif /* __OPE_POSITION_H__ */