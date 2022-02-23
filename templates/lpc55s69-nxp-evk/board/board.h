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
 * @file        board.h
 *
 * @brief       Board resource definition
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include "clock_config.h"
#include "pin_mux.h"
#include <device.h>
#include <drv_cfg.h>
#include <drv_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define LPC_FLASH_START_ADRESS      ((uint32_t)0x00000000)
#define LPC_FLASH_SIZE              (608 * 1024)//0x9DE00
#define LPC_FLASH_PAGE_SIZE         (0x200)
#define LPC_FLASH_BLOCK_SIZE        LPC_FLASH_PAGE_SIZE
#define LPC_FLASH_END_ADDRESS       ((uint32_t)(LPC_FLASH_START_ADRESS + LPC_FLASH_SIZE))

#define LPC_SRAM1_SIZE              (272 * 1024)
#define LPC_SRAM1_START             (0x20000000)
#define LPC_SRAM1_END               (LPC_SRAM1_START + LPC_SRAM1_SIZE)

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$ARM_LIB_HEAP$$ZI$$Base;
#define HEAP_BEGIN (&Image$$ARM_LIB_HEAP$$ZI$$Base)
#elif __ICCARM__
#pragma section = "HEAP"
#define HEAP_BEGIN (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN (&__bss_end)
#endif

#define HEAP_END LPC_SRAM1_END

extern const struct push_button key_table[];
extern const int                key_table_size;

extern const led_t led_table[];
extern const int   led_table_size;

os_err_t board_power_on(void);
#ifdef __cplusplus
}
#endif

#endif
