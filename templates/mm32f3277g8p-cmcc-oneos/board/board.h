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


#include <drv_cfg.h>
#include <HAL_device.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MM32_FLASH_START_ADRESS (0x08000000)
#define MM32_FLASH_SIZE         (0x00080000)
#define MM32_FLASH_BLOCK_SIZE   (0x00000400)
#define MM32_FLASH_END_ADDRESS  ((uint32_t)(MM32_FLASH_START_ADRESS + MM32_FLASH_SIZE))

#define MM32_SRAM1_START        (0x20000000)
#define MM32_SRAM1_SIZE         (0x00020000)
#define MM32_SRAM1_END          (MM32_SRAM1_START + MM32_SRAM1_SIZE)

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

#define HEAP_END MM32_SRAM1_END




#if defined(OS_USING_PUSH_BUTTON)
extern const struct push_button key_table[];
extern const int key_table_size;
#endif

#if defined(OS_USING_LED)
extern const led_t led_table[];
extern const int   led_table_size;
#endif

#if defined(OS_USING_BUZZER)
extern const buzzer_t buzzer_table[];
extern const int   buzzer_table_size;
#endif

extern os_uint32_t SystemCoreClock;

#ifdef __cplusplus
}
#endif

#endif
