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
#include "hc32f19x.h"
#include "system_hc32f19x.h"
#include "hc_flash.h"
#include "hc_sysctrl.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SOC_MODEL         "HC32F196KCTA"

#define HC32_FLASH_START_ADRESS       ((uint32_t)0x00000000)
#define HC32_FLASH_SIZE               (256 * 1024)
#define HC32_FLASH_BLOCK_SIZE         (512)
#define HC32_FLASH_BYTE_ALIGN_SIZE    (1)
#define HC32_FLASH_END_ADDRESS        ((uint32_t)(HC32_FLASH_START_ADRESS + HC32_FLASH_SIZE))
#define HC32_SECTOR_SIZE              (512)

#define HC32_SRAM_SIZE        (32 * 1024)
#define HC32_SRAM_END         (0x20000000 + HC32_SRAM_SIZE)

/* lpuart0 */
#ifdef BSP_USING_LPUART0
#define LPUART0_TX_PORT    GpioPortB
#define LPUART0_TX_PIN     GpioPin0
#define LPUART0_TX_AF      GpioAf3
#define LPUART0_RX_PORT    GpioPortC
#define LPUART0_RX_PIN     GpioPin5
#define LPUART0_RX_AF      GpioAf1
#endif

/* lpuart1 */
#ifdef BSP_USING_LPUART1
#define LPUART1_TX_PORT    GpioPortA
#define LPUART1_TX_PIN     GpioPin0
#define LPUART1_TX_AF      GpioAf2
#define LPUART1_RX_PORT    GpioPortA
#define LPUART1_RX_PIN     GpioPin1
#define LPUART1_RX_AF      GpioAf2
#endif

/* uart0 */
#ifdef BSP_USING_UART0
#define UART0_TX_PORT      GpioPortA
#define UART0_TX_PIN       GpioPin9
#define UART0_TX_AF        GpioAf1
#define UART0_RX_PORT      GpioPortA
#define UART0_RX_PIN       GpioPin10
#define UART0_RX_AF        GpioAf1
#endif

/* uart1 */
#ifdef BSP_USING_UART1
#define UART1_TX_PORT      GpioPortA
#define UART1_TX_PIN       GpioPin2
#define UART1_TX_AF        GpioAf1
#define UART1_RX_PORT      GpioPortA
#define UART1_RX_PIN       GpioPin3
#define UART1_RX_AF        GpioAf1
#endif

/* uart2 */
#ifdef BSP_USING_UART2
#define UART2_TX_PORT      GpioPortC
#define UART2_TX_PIN       GpioPin3
#define UART2_TX_AF        GpioAf5
#define UART2_RX_PORT      GpioPortC
#define UART2_RX_PIN       GpioPin2
#define UART2_RX_AF        GpioAf4
#endif

/* uart3 */
#ifdef BSP_USING_UART3
#define UART3_TX_PORT      GpioPortC
#define UART3_TX_PIN       GpioPin7
#define UART3_TX_AF        GpioAf6
#define UART3_RX_PORT      GpioPortC
#define UART3_RX_PIN       GpioPin6
#define UART3_RX_AF        GpioAf6
#endif

#ifdef BSP_USING_I2C0
#define I2C0_SCL_PORT      GpioPortB
#define I2C0_SCL_PIN       GpioPin8
#define I2C0_SDA_PORT      GpioPortB
#define I2C0_SDA_PIN       GpioPin9
#define I2C0_GPIO_AF       GpioAf1
#endif
#ifdef BSP_USING_I2C1
#define I2C1_SCL_PORT      GpioPortB
#define I2C1_SCL_PIN       GpioPin10
#define I2C1_SDA_PORT      GpioPortB
#define I2C1_SDA_PIN       GpioPin11
#define I2C1_GPIO_AF       GpioAf1
#endif

#ifdef OS_USING_DAC
#define DAC_PORT           GpioPortA
#define DAC_PIN            GpioPin4
#endif

#ifdef OS_USING_SPI

#ifdef BSP_USING_SPI0
#define SPI0_CS_PORT       GpioPortE
#define SPI0_CS_PIN        GpioPin12
#define SPI0_SCK_PORT      GpioPortE
#define SPI0_SCK_PIN       GpioPin13
#define SPI0_MISO_PORT     GpioPortE
#define SPI0_MISO_PIN      GpioPin14
#define SPI0_MOSI_PORT     GpioPortE
#define SPI0_MOSI_PIN      GpioPin15
#define SPI0_GPIO_AF       GpioAf2
#endif
#ifdef BSP_USING_SPI1
#define SPI1_CS_PORT       GpioPortB
#define SPI1_CS_PIN        GpioPin12
#define SPI1_SCK_PORT      GpioPortB
#define SPI1_SCK_PIN       GpioPin13
#define SPI1_MISO_PORT     GpioPortB
#define SPI1_MISO_PIN      GpioPin14
#define SPI1_MOSI_PORT     GpioPortB
#define SPI1_MOSI_PIN      GpioPin15
#define SPI1_GPIO_AF       GpioAf1
#endif

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

void SystemClkInit_RCH(en_sysctrl_rch_freq_t enRchFreq);
void SystemClkInit_PLL48M_byRCH(void);
void System_cfg(void);

#endif
