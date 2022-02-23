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
#include "hc32f4a0.h"
#include "system_hc32f4a0.h"
#include "hc32f4a0_efm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SOC_MODEL         "HC32F4A0SITB"

#define HC32_FLASH_START_ADRESS       ((uint32_t)0x00000000)
#define HC32_FLASH_SIZE               (2048 * 1024)
#define HC32_FLASH_BLOCK_SIZE         (8 * 1024)
#define HC32_FLASH_BYTE_ALIGN_SIZE    (4)
#define HC32_FLASH_END_ADDRESS        ((uint32_t)(HC32_FLASH_START_ADRESS + HC32_FLASH_SIZE))
#define HC32_SECTOR_SIZE              (8 * 1024)

#define HC32_SRAM_SIZE        (516 * 1024)
#define HC32_SRAM_END         (0x20000000 + HC32_SRAM_SIZE)

/* uart1 */
#ifdef BSP_USING_UART1
#define UART1_TX_PORT      GPIO_PORT_A
#define UART1_TX_PIN       GPIO_PIN_09
#define UART1_TX_FUNC      GPIO_FUNC_20_USART1_TX
#define UART1_RX_PORT      GPIO_PORT_A
#define UART1_RX_PIN       GPIO_PIN_10
#define UART1_RX_FUNC      GPIO_FUNC_20_USART1_RX
#endif

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
extern int __bss_end__;
#define HEAP_BEGIN      ((void *)&__bss_end__)
#endif

#define HEAP_END        HC32_SRAM_END

#ifdef OS_USING_PUSH_BUTTON
extern const struct push_button key_table[];
extern const int                key_table_size;
#endif

#ifdef OS_USING_LED
extern const led_t led_table[];
extern const int   led_table_size;
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

void System_cfg(void);

#endif
