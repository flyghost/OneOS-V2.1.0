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

#ifndef __OPE_ALGORITHM_H__
#define __OPE_ALGORITHM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ope_types.h"
#include "ope_error.h"
#include "ope_log.h"
#include "ope_math.h"

#define OPE_MAC_LEN     12 /* The length of the MAC address */
#define OPE_ALIGN       8  /* Byte alignment */
#define OPE_PI          3.141592653  /* PI */
#define OPE_COORDINATE_INVALID  0xffffffff  /* Coordinates is invalid */

/* Address byte alignment function */
#define OPE_CALC_ALIGN(addr)  \
    (((ope_uint32_t)addr + OPE_ALIGN - 1) & (~(OPE_ALIGN - 1)))

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_ap_t
 *      
 * @brief       Single AP information
 ***********************************************************************************************************************
 */
typedef struct 
{
    ope_char_t   mac[OPE_MAC_LEN];  /* MAC address */
    ope_int8_t   rssi;              /* RSSI */
    ope_double_t lat;               /* latitude */
    ope_double_t lon;               /* longitude */
}ope_ble_rssi_ap_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_ap_frame_t
 *      
 * @brief       AP information for a single frame
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_uint8_t ap_num;             /* The number of AP messages */
    ope_ble_rssi_ap_t *ap;          /* The AP information */
}ope_ble_rssi_ap_frame_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_beacon_t
 *      
 * @brief       Single beacon information
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_char_t   mac[OPE_MAC_LEN];  /* MAC address */
    ope_uint8_t  rssi_num;          /* The number of RSSI */
    ope_int8_t  *rssi;              /* RSSI */
    ope_double_t lat;               /* latitude */
    ope_double_t lon;               /* longitude */
}ope_ble_rssi_beacon_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_beacon_frame_t
 *      
 * @brief       Beacon information for a single frame
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_uint8_t beacon_num;         /* The number of beacon messages */
    ope_ble_rssi_beacon_t *beacon;  /* The beacon information */
}ope_ble_rssi_beacon_frame_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_beacon_data_reduction_param_t
 *      
 * @brief       The paramters to the ope_ble_rssi_beacon_data_reduction function
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_double_t lat_max;           /* The maximum latitude */
    ope_double_t lat_min;           /* The minimum latitude */
    ope_double_t lon_max;           /* The maximum longitude */
    ope_double_t lon_min;           /* The minimum longitude */
}ope_ble_rssi_beacon_data_reduction_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_selector_beacon_t
 *      
 * @brief       Single beacon information within the selector
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_char_t   mac[OPE_MAC_LEN];  /* MAC address */
    ope_uint8_t  undetected_num;    /* The number of consecutive undetected times */
    ope_int8_t  *rssi;              /* RSSI */
    ope_double_t lat;               /* latitude */
    ope_double_t lon;               /* longitude */
}ope_ble_rssi_selector_beacon_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_selector_buf_t
 *      
 * @brief       Beacon information within the selector
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_uint8_t selector_beacon_num;            /* The number of beacon */
    ope_uint8_t selector_beacon_num_max;        /* The maximum number of beacons allowed */
    ope_uint8_t selector_beacon_rssi_num_max;   /* The maximum number of active RSSI that a single beacon can store */
    ope_uint8_t selector_invalid_num;           /* Limits on the number of times beacons are invalid */
    ope_ble_rssi_selector_beacon_t *selector_beacon;   /* Cached beacons */
}ope_ble_rssi_selector_buf_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_statistics_beacon_selector_param
 *      
 * @brief       The paramters to the ope_ble_rssi_statistics_beacon_selector function
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_uint8_t select_num_max;                 /* The maximum number of selected beacons */
    ope_uint8_t eigenvalue_selector_num_max;    /* The maximum number of eigenvalues to be selected */
}ope_ble_rssi_statistics_beacon_selector_param;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_kalman_param_t
 *      
 * @brief       The paramters to the ope_ble_rssi_kalman functions
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_double_t b[16];  /* Input matrix(4x4), row continuous */
    ope_double_t u[4];   /* System input(4x1), row continuous */
    ope_double_t k[8];   /* Kalman gain(4x2), row continuous */
    ope_double_t p[16];  /* The covariance matrix of the system states(4x4), row continuous */
    ope_double_t x[4];   /* Estimate the result(4x1), row continuous */
    ope_double_t a[16];  /* State transition matrix(4x4), row continuous */
    ope_double_t q[16];  /* Prediction of noise covariance matrix(4x4), row continuous */
    ope_double_t h[8];   /* Measurement matrix(2x4), row continuous */
    ope_double_t r[4];   /* Observe the noise covariance matrix(2x2), row continuous */
}ope_ble_rssi_kalman_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_beacon_data_scope_filter_param_t
 *      
 * @brief       The paramters to the ope_ble_rssi_beacon_data_scope_filter function
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_int8_t   rssi_range;           /* Maximum range of fluctuation */
    ope_double_t rssi_ratio_thr;       /* Percentage of smaller RSSI */             
}ope_ble_rssi_beacon_data_scope_filter_param_t;


/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_to_distance_param_t
 *      
 * @brief       The paramters to the ope_ble_rssi_to_distanc function
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_double_t initial_value;    /* Environmental factors of logarithmic modeln */
    ope_double_t env_factor;       /* RSSI at one meter of logarithmic model */
}
ope_ble_rssi_to_distance_param_t;

/**
 ***********************************************************************************************************************
 * @struct      ope_ble_rssi_trilateration_calc_param_t
 *      
 * @brief       The paramters to the ope_ble_rssi_trilateration_calc function
 ***********************************************************************************************************************
 */
typedef struct
{
    ope_double_t  threshold;       /* Iteration exit threshold */
    ope_uint32_t  search_num_max;  /* Upper limit number of iterations */
}ope_ble_rssi_trilateration_calc_param_t;

ope_err_t ope_ble_rssi_beacon_data_reduction(ope_ble_rssi_ap_frame_t ap_frame,
                                             ope_ble_rssi_beacon_frame_t *beacon_frame,
                                             ope_ble_rssi_beacon_data_reduction_param_t param,
                                             ope_int8_t  *rssi_addr,
                                             ope_int32_t *rssi_num_all,
                                             ope_void_t  *tmp_buf);

ope_err_t ope_ble_rssi_statistics_beacon_selector(ope_ble_rssi_beacon_frame_t *beacon_frame,
                                                  ope_ble_rssi_selector_buf_t *selector_buf,
                                                  ope_float_t  *charac_weight,
                                                  ope_ble_rssi_statistics_beacon_selector_param param,
                                                  ope_void_t   *tmp_buf,
                                                  ope_double_t *lat,
                                                  ope_double_t *lon,
                                                  ope_double_t *rssi,
                                                  ope_uint8_t  *select_num);

ope_err_t ope_ble_rssi_selector_buf_init(ope_ble_rssi_selector_buf_t *selector_buf,
                                         ope_void_t  *buf_addr,
                                         ope_uint8_t  selector_beacon_num_max,
                                         ope_uint8_t  selector_beacon_rssi_num_max,
                                         ope_uint8_t  selector_invalid_num);

ope_err_t ope_ble_rssi_kalman_init(ope_double_t  p_x, 
                                   ope_double_t  p_y,
                                   ope_double_t  v_x,
                                   ope_double_t  v_y,
                                   ope_double_t *x,
                                   ope_ble_rssi_kalman_param_t *param);

ope_err_t ope_ble_rssi_kalman_update(ope_double_t *z,
                                     ope_ble_rssi_kalman_param_t *param,
                                     ope_double_t *x,
                                     ope_void_t   *tmp_buf);
                                     
ope_err_t latlon_to_xy( ope_double_t *lat,
                        ope_double_t *lon,
                        ope_uint8_t   length,
                        ope_double_t *x,
                        ope_double_t *y, 
                        ope_double_t *lam_prime);

ope_err_t xy_to_latlon(ope_double_t *x,
                       ope_double_t *y,
                       ope_double_t *lam_prime,
                       ope_uint8_t   length,
                       ope_double_t *lat,
                       ope_double_t *lon);

ope_err_t ope_ble_rssi_beacon_data_scope_filter(ope_ble_rssi_beacon_frame_t *beacon_frame,
                                                ope_ble_rssi_beacon_data_scope_filter_param_t param);

ope_err_t ope_ble_rssi_to_distance(ope_double_t *rssi, 
                                   ope_uint8_t   length,
                                   ope_ble_rssi_to_distance_param_t param, 
                                   ope_double_t *dist);

ope_err_t ope_ble_rssi_trilateration_calc(ope_double_t *x,
                                          ope_double_t *y, 
                                          ope_double_t *dist,
                                          ope_uint32_t  length,
                                          ope_ble_rssi_trilateration_calc_param_t param,
                                          ope_void_t   *tmp_buf,
                                          ope_double_t *pos_x, 
                                          ope_double_t *pos_y);                            

#endif /* __OPE_ALGORITHM_H__ */