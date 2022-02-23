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
 * @file        ope_math.h
 * 
 * @brief       Basic math library 
 * 
 * @details     Provides basic mathematical functions
 * 
 * @revision
 * Date         Author          Notes
 * 2021-05-08   HuSong          First Version
 ***********************************************************************************************************************
 */

#ifndef __OPE_MATH_H__
#define __OPE_MATH_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ope_types.h"
#include "ope_error.h"
#include "ope_log.h"

/* Determine whether the double data is zero, returns 1 if zero, or zero otherwise */
#define  CHECK_ZERO_DOUBLE(x) ((fabs(x) < 0.000000000001) ? 1 : 0)

/**
 ***********************************************************************************************************************
 * @enum        ope_sort_mode_t
 * 
 * @brief       The way we sort it, there are two states:
 *                  - OPE_ASCEND      Ascending sort
 *                  - OPE_DEASCEND    Descending sort 
 ***********************************************************************************************************************
 */
typedef enum
{
    OPE_ASCEND = 0,
    OPE_DEASCEND
}ope_sort_mode_t;

/**
 ***********************************************************************************************************************
 * @enum        ope_continuity_mode_t
 * 
 * @brief       How matrix data is stored in memory, there are two states:
 *                  - OPE_ROW         Store in rows consecutively
 *                  - OPE_COL         Store in columns consecutively
 ***********************************************************************************************************************
 */
typedef enum
{
    OPE_ROW = 0,
    OPE_COL
}ope_continuity_mode_t;

ope_int32_t ope_vsum_i8(ope_int8_t *src, ope_uint32_t len);
ope_float_t ope_vmean_i8(ope_int8_t* src, ope_uint32_t len);
ope_double_t ope_vmean_lf(ope_double_t* src, ope_uint32_t len);
ope_float_t ope_vvar_i8(ope_int8_t *src, ope_uint32_t len);
ope_int8_t ope_vmin_i8(ope_int8_t* src, ope_uint8_t len, ope_int8_t* index);
ope_int8_t ope_vmax_i8(ope_int8_t* src, ope_uint32_t len);
ope_uint32_t ope_vmax_ui32(ope_uint32_t *src, ope_uint32_t len);
ope_err_t ope_vinvert_i8(ope_int8_t *src, ope_int8_t *dst, ope_uint32_t len);
ope_err_t ope_vpow_lf(ope_double_t *src,
                      ope_uint32_t  len,
                      ope_double_t  x,
                      ope_double_t *dst);
ope_err_t ope_vsort_f(ope_float_t *src, 
                      ope_float_t *dst, 
                      ope_uint8_t len, 
                      ope_sort_mode_t sort_mode,
                      ope_uint8_t *index);
ope_err_t ope_madd_lf(ope_double_t  *src1,
                      ope_double_t  *src2,
                      ope_uint32_t   row,
                      ope_uint32_t   col,
                      ope_double_t  *dst);
ope_err_t ope_msub_lf(ope_double_t  *src1,
                      ope_double_t  *src2,
                      ope_uint32_t   row,
                      ope_uint32_t   col,
                      ope_double_t  *dst);
ope_err_t ope_mprod_f_col(ope_float_t  *src1,
                          ope_uint32_t  src1_row,
                          ope_uint32_t  src1_col,
                          ope_float_t  *src2,
                          ope_uint32_t  src2_row,
                          ope_uint32_t  src2_col,
                          ope_float_t  *dst);
ope_err_t ope_mprod_lf_col(ope_double_t  *src1,
                           ope_uint32_t   src1_row,
                           ope_uint32_t   src1_col,
                           ope_double_t  *src2,
                           ope_uint32_t   src2_row,
                           ope_uint32_t   src2_col,
                           ope_double_t  *dst);
ope_err_t ope_mprod_lf_row(ope_double_t  *src1,
                           ope_uint32_t   src1_row,
                           ope_uint32_t   src1_col,
                           ope_double_t  *src2,
                           ope_uint32_t   src2_row,
                           ope_uint32_t   src2_col,
                           ope_double_t  *dst);
ope_err_t ope_mtrans_lf_col(ope_double_t *src,
                            ope_uint32_t  row,
                            ope_uint32_t  col,
                            ope_double_t *dst);
ope_err_t ope_mtrans_lf_row(ope_double_t *src,
                            ope_uint32_t  row,
                            ope_uint32_t  col,
                            ope_double_t *dst);
ope_err_t ope_mprodt_lf_row(ope_double_t  *src1,
                            ope_uint32_t   src1_row,
                            ope_uint32_t   src1_col,
                            ope_double_t  *src2,
                            ope_uint32_t   src2_row,
                            ope_uint32_t   src2_col,
                            ope_double_t  *dst);
ope_err_t ope_minv_lf_row(ope_double_t *src,
                          ope_double_t *dst,
                          ope_uint32_t  n,
                          ope_void_t   *tmp_buf);

#endif