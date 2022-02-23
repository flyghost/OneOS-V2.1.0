/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        board.h
 *
 * \@brief       Board resource definition
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <drv_cfg.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SOC_MODEL         "FM33LG016"

#define FM_FLASH_START_ADRESS      ((os_uint32_t)0x0)
#define FM_FLASH_SIZE              (64 * 1024)
#define FM_FLASH_BLOCK_SIZE        (512)
#define FM_FLASH_PAGE_SIZE         (512)
#define FM_FLASH_BYTE_ALIGN_SIZE   (4)
#define FM_FLASH_END_ADDRESS       ((os_uint32_t)(FM_FLASH_START_ADRESS + FM_FLASH_SIZE))

#define FM_SRAM1_SIZE  (16)
#define FM_SRAM1_START (0x20000000)
#define FM_SRAM1_END   (FM_SRAM1_START + FM_SRAM1_SIZE * 1024)

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section = "HEAP"
#define HEAP_BEGIN (__segment_end("HEAP"))
#else
extern int __bss_end;
#define HEAP_BEGIN (&__bss_end)
#endif

#define HEAP_END FM_SRAM1_END

#ifdef OS_USING_LED
extern const led_t led_table[];
extern const int led_table_size;
#endif

#ifdef OS_USING_PUSH_BUTTON
extern const struct push_button key_table[];
extern const int key_table_size;
#endif

#ifdef OS_USING_SN
extern const int board_no_pin_tab[];
extern const int board_no_pin_tab_size;

extern const int slot_no_pin_tab[];
extern const int slot_no_pin_tab_size;
#endif

#ifdef __cplusplus
}
#endif

#endif
