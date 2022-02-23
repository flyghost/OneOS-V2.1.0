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
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BOARD_H__
#define __BOARD_H__

#include <gd32vf103.h>
#include <drv_cfg.h>
//
#ifdef __cplusplus
extern "C" {
#endif

#define GD32_FLASH_START_ADRESS ((uint32_t)0x08000000)
#define GD32_FLASH_SIZE         (128 * 1024)
#define GD32_FLASH_END_ADDRESS  ((uint32_t)(GD32_FLASH_START_ADRESS + GD32_FLASH_SIZE))

#define GD32_SRAM1_SIZE  (32)
#define GD32_SRAM1_START (0x20000000)
#define GD32_SRAM1_END   (GD32_SRAM1_START + GD32_SRAM1_SIZE * 1024)

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

#define HEAP_END GD32_SRAM1_END

#ifdef OS_USING_PUSH_BUTTON
    extern const struct push_button key_table[];
    extern const int                key_table_size;
#endif

#ifdef OS_USING_LED
    extern const led_t led_table[];
    extern const int   led_table_size;
#endif

#ifdef __cplusplus
}
#endif
//
#endif
