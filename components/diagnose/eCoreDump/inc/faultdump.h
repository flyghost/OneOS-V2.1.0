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
 * @file        faultdump.h
 *
 * @brief       This file provides interface for faultdump.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ECD_FAULT_DUMP_H__
#define __ECD_FAULT_DUMP_H__

#include <stdint.h>
#include "ecoredump.h"

/**
 ***********************************************************************************************************************
 * @brief           Generate coredump and save to flash.
 *
 * @param           None.
 *
 * @return          Whether save operation success.
 * @retval          0   success.
 * @retval          -1  failed.
 ***********************************************************************************************************************
 */
int ecd_faultdump(void);

/**
 ***********************************************************************************************************************
 * @brief           Generate coredump and save to flash.
 *
 * @param           None.
 *
 * @return          Number of corefiles saved on flash.
 ***********************************************************************************************************************
 */
int ecd_get_fault_file_count(void);

/**
 ***********************************************************************************************************************
 * @brief           Generate coredump and save to flash.
 *
 * @param[out]      data        return the corefile start.
 * @param[out]      len         return the length of tht corefile.
 * @param[in]       index       corefile index.
 *
 * @return          on Return.
 ***********************************************************************************************************************
 */
void ecd_get_fault_file(const uint8_t ** core_data, uint32_t *len, int index);

// Only support xip flash.

// Implement below function on your platform.
int32_t ecd_flash_op_init(void ** ctx);
uint32_t ecd_flash_op_get_page_size(void * ctx);
const uint8_t * ecd_flash_op_get_start_addr(void * ctx);
uint32_t ecd_flash_op_get_total_size(void * ctx);
int32_t ecd_flash_op_write_page(void * ctx, uint32_t page_addr, uint8_t *buff, uint32_t page_nr);

void ecd_log_err(char * format, ...);

#endif
