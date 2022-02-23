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

#include <stm32f4xx.h>
#include <drv_cfg.h>
#include <sram_port.h>

#ifdef __cplusplus
extern "C" {
#endif


#define STM32_SRAM1_SIZE  (128)
#define STM32_SRAM1_START (0x20000000)
#define STM32_SRAM1_END   (STM32_SRAM1_START + STM32_SRAM1_SIZE * 1024)

#define STM32_SRAM2_SIZE  (64)
#define STM32_SRAM2_START (0x10000000)
#define STM32_SRAM2_END   (STM32_SRAM2_START + STM32_SRAM2_SIZE * 1024)

#define STM32_FLASH_START_ADRESS ((uint32_t)0x08000000)
#define STM32_FLASH_SIZE         (1024 * 1024)
#define STM32_FLASH_END_ADDRESS  ((uint32_t)(STM32_FLASH_START_ADRESS + STM32_FLASH_SIZE))

//#define HEAP_USE_RAM1
#ifdef HEAP_USE_RAM1

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section = "CSTACK"
#define HEAP_BEGIN (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define HEAP_BEGIN ((void *)&__bss_end)
#endif

#define HEAP_SIZE   (64 * 1024)
#define HEAP_END    ((os_size_t)HEAP_BEGIN + HEAP_SIZE)

#define DMA_HEAP_BEGIN HEAP_END
#define DMA_HEAP_END   STM32_SRAM1_END
#define DMA_HEAP_SIZE  (DMA_HEAP_END - (os_size_t)DMA_HEAP_BEGIN)

#else

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define DMA_HEAP_BEGIN ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section = "CSTACK"
#define DMA_HEAP_BEGIN (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define DMA_HEAP_BEGIN ((void *)&__bss_end)
#endif

#define DMA_HEAP_END STM32_SRAM1_END

#define DMA_HEAP_SIZE   (DMA_HEAP_END - (os_size_t)DMA_HEAP_BEGIN)

#define HEAP_BEGIN  STM32_SRAM2_START
#define HEAP_END    STM32_SRAM2_END

#endif

extern const struct push_button key_table[];
extern const int                key_table_size;

extern const led_t led_table[];
extern const int   led_table_size;

#ifdef __cplusplus
}
#endif

#endif
