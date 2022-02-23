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
 * @file        cmiot_hal.h
 *
 * @brief       The hal header file
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMIOT_HAL_H__
#define __CMIOT_HAL_H__

#include "cmiot_type.h"
#include <fal_part.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CMIOT_BUF_MAX_LEN 32

typedef enum
{
    CMIOT_FLASH_OPERATION_READ,
    CMIOT_FLASH_OPERATION_WRITE,
    CMIOT_FLASH_OPERATION_ERASE,
    CMIOT_FLASH_OPERATION_END
} cmiot_flash_operation_t;

typedef struct
{
    cmiot_uint8  inited;
    cmiot_uint16 update_result;
    cmiot_char   mid[CMIOT_MID_MAXLEN + 1];
    cmiot_char   code[32];
    cmiot_char   device_id[CMIOT_DEVICEID_MAX_LEN + 1];
    cmiot_char   device_secret[CMIOT_DEVICESECRET_MAX_LEN + 1];
    cmiot_uint32 index;
    cmiot_uint32 index_max;
    cmiot_uint32 delta_id;
} CMIOT_ALIGN(1) cmiot_update_t;

typedef enum
{
    CMIOT_RESULT_FAIL = 0,
    CMIOT_RESULT_CV_SUCC,
    CMIOT_RESULT_CV_FAIL,
    CMIOT_RESULT_CV_NO_NEW,
    CMIOT_RESULT_RPT_SUCC,
    CMIOT_RESULT_RPT_FAIL,
    CMIOT_RESULT_PROGRESS,
    CMIOT_RESULT_END,
} cmiot_result_t;

typedef struct
{
    cmiot_uint8  result;
    cmiot_uint32 state;
} CMIOT_ALIGN(1) cmiot_action_result_t;

void            cmiot_hal_update_device(cmiot_char * mid,
                                        cmiot_uint16 mid_len,
                                        cmiot_char * device_id,
                                        cmiot_uint16 device_id_len,
                                        cmiot_char * device_secret,
                                        cmiot_uint16 device_secret_len);
cmiot_int32     cmiot_hal_flash_read(cmiot_uint8 type, cmiot_uint32 addr, cmiot_char *buf, cmiot_uint32 size);
cmiot_int32     cmiot_hal_flash_write(cmiot_uint8 type, cmiot_uint32 addr, cmiot_char *buf, cmiot_uint32 size);
cmiot_int32     cmiot_hal_flash_erase(cmiot_uint8 type, cmiot_uint32 addr, cmiot_uint32 size);
cmiot_uint32    cmiot_hal_get_true_blocksize(cmiot_uint8 type);
cmiot_uint32    cmiot_hal_get_true_pagesize(cmiot_uint8 type);
cmiot_uint32    cmiot_hal_get_info_addr(void);
cmiot_uint32    cmiot_hal_get_backup_addr(void);
cmiot_uint32    cmiot_hal_get_app_addr(void);
cmiot_uint32    cmiot_hal_get_delta_addr(void);
cmiot_uint32    cmiot_hal_get_delta_size(void);
cmiot_uint32    cmiot_hal_get_download_size(void);
cmiot_bool      cmiot_hal_write_delta(cmiot_uint16 index, cmiot_char *data, cmiot_uint16 len);
cmiot_bool      cmiot_hal_erase_sector(cmiot_uint8 type, cmiot_uint32 addr);
cmiot_update_t *cmiot_hal_init_update(void);
cmiot_update_t *cmiot_hal_get_update(void);
cmiot_bool      cmiot_hal_set_update(cmiot_update_t *cmiot_update);
cmiot_uint32    cmiot_hal_get_delta_id(void);
cmiot_char *    cmiot_hal_get_device_id(void);
cmiot_char *    cmiot_hal_get_device_secret(void);
cmiot_bool      cmiot_hal_set_download_index(cmiot_uint32 index);
cmiot_uint32    cmiot_hal_get_download_index(void);
void            cmiot_hal_reset_index_pos(void);

cmiot_char * cmiot_hal_get_mid(void);
cmiot_uint16 cmiot_hal_get_update_result(void);
fal_part_t * cmiot_hal_get_device(cmiot_uint8 type);

#ifdef __cplusplus
}
#endif

#endif
