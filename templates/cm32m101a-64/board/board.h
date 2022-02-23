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
#include "cm32m101a.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CM32_FLASH_START_ADRESS       ((uint32_t)0x08000000)
#define CM32_FLASH_SIZE               (128 * 1024)
#define CM32_FLASH_BLOCK_SIZE         (2048)
#define CM32_FLASH_BYTE_ALIGN_SIZE	  (4)
#define CM32_FLASH_END_ADDRESS        ((uint32_t)(CM32_FLASH_START_ADRESS + CM32_FLASH_SIZE))
#define CM32_SECTOR_SIZE              (2048)

#define CM32_SRAM_SIZE        (32)
#define CM32_SRAM_END         (0x20000000 + CM32_SRAM_SIZE * 1024)

/* lpuart0 */
#ifdef BSP_USING_LPUART0
#define LPUART0_RX_GPIO    GPIOC
#define LPUART0_TX_GPIO    GPIOC
#define LPUART0_CLK        RCC_APB2_PERIPH_USART1
#define LPUART0_GPIO_CLK   RCC_APB2_PERIPH_GPIOC
#define LPUART0_RxPin      GPIO_PIN_5
#define LPUART0_TxPin      GPIO_PIN_4
#define LPUART0_Rx_GPIO_AF GPIO_AF2_LPUART
#define LPUART0_Tx_GPIO_AF GPIO_AF2_LPUART

#define LPUART0_GPIO_APBxClkCmd   RCC_EnableAPB2PeriphClk
#endif

/* usart1 */
#ifdef BSP_USING_UART1
#define USART1_RX_GPIO    GPIOA
#define USART1_TX_GPIO    GPIOA
#define USART1_CLK        RCC_APB2_PERIPH_USART1
#define USART1_GPIO_CLK   RCC_APB2_PERIPH_GPIOA
#define USART1_RX_PIN     GPIO_PIN_10
#define USART1_TX_PIN     GPIO_PIN_9
#define USART1_RX_GPIO_AF GPIO_AF4_USART1
#define USART1_TX_GPIO_AF GPIO_AF4_USART1
#endif

/* usart2 */
#ifdef BSP_USING_UART2
#define USART2_RX_GPIO    GPIOA
#define USART2_TX_GPIO    GPIOA
#define USART2_CLK        RCC_APB1_PERIPH_USART2
#define USART2_GPIO_CLK   RCC_APB2_PERIPH_GPIOA
#define USART2_RX_PIN     GPIO_PIN_3
#define USART2_TX_PIN     GPIO_PIN_2
#define USART2_RX_GPIO_AF GPIO_AF4_USART2
#define USART2_TX_GPIO_AF GPIO_AF4_USART2
#endif

/* usart3 */
#ifdef BSP_USING_UART3
#define USART3_RX_GPIO    GPIOC
#define USART3_TX_GPIO    GPIOC
#define USART3_CLK        RCC_APB1_PERIPH_USART3
#define USART3_GPIO_CLK   RCC_APB2_PERIPH_GPIOC
#define USART3_RX_PIN     GPIO_PIN_11
#define USART3_TX_PIN     GPIO_PIN_10
#define USART3_RX_GPIO_AF GPIO_AF5_USART3
#define USART3_TX_GPIO_AF GPIO_AF5_USART3
#endif

/* uart4 */
#ifdef BSP_USING_UART4
#define UART4_RX_GPIO    GPIOB
#define UART4_TX_GPIO    GPIOB
#define UART4_CLK        RCC_APB2_PERIPH_UART4
#define UART4_GPIO_CLK   RCC_APB2_PERIPH_GPIOB
#define UART4_RX_PIN     GPIO_PIN_15
#define UART4_TX_PIN     GPIO_PIN_14
#define UART4_RX_GPIO_AF GPIO_AF6_UART4
#define UART4_TX_GPIO_AF GPIO_AF6_UART4
#endif

/* uart5 */
#ifdef BSP_USING_UART5
#define UART5_RX_GPIO    GPIOB
#define UART5_TX_GPIO    GPIOB
#define UART5_CLK        RCC_APB2_PERIPH_UART5
#define UART5_GPIO_CLK   RCC_APB2_PERIPH_GPIOB
#define UART5_RX_PIN     GPIO_PIN_5
#define UART5_TX_PIN     GPIO_PIN_4
#define UART5_RX_GPIO_AF GPIO_AF7_UART5
#define UART5_TX_GPIO_AF GPIO_AF6_UART5
#endif

#ifdef BSP_USING_I2C1
#define I2C1_SCL_PIN      GPIO_PIN_6
#define I2C1_SCL_PORT     GPIOB
#define AF_I2C1_SCL       GPIO_AF1_I2C1
#define I2C1_SDA_PIN      GPIO_PIN_7
#define I2C1_SDA_PORT     GPIOB
#define AF_I2C1_SDA       GPIO_AF1_I2C1
#endif

#ifdef BSP_USING_I2C2
#define I2C2_SCL_PIN      GPIO_PIN_10
#define I2C2_SCL_PORT     GPIOB
#define AF_I2C2_SCL       GPIO_AF6_I2C2
#define I2C2_SDA_PIN      GPIO_PIN_11
#define I2C2_SDA_PORT     GPIOB
#define AF_I2C2_SDA       GPIO_AF6_I2C2
#endif

#ifdef BSP_USING_SPI1
#define SPI1_CS_PORT      GPIOA
#define SPI1_CS_PIN       GPIO_PIN_8
#define SPI1_CS_AF        GPIO_AF5_SPI1
#define SPI1_SCK_PORT     GPIOA
#define SPI1_SCK_PIN      GPIO_PIN_5
#define SPI1_SCK_AF       GPIO_AF0_SPI1
#define SPI1_MISO_PORT    GPIOA
#define SPI1_MISO_PIN     GPIO_PIN_6
#define SPI1_MISO_AF      GPIO_AF0_SPI1
#define SPI1_MOSI_PORT    GPIOA
#define SPI1_MOSI_PIN     GPIO_PIN_7
#define SPI1_MOSI_AF      GPIO_AF0_SPI1
#endif

#ifdef BSP_USING_SPI2
#define SPI2_CS_PORT      GPIOC
#define SPI2_CS_PIN       GPIO_PIN_6
#define SPI2_CS_AF        GPIO_AF5_SPI2
#define SPI2_SCK_PORT     GPIOC
#define SPI2_SCK_PIN      GPIO_PIN_7
#define SPI2_SCK_AF       GPIO_AF5_SPI2
#define SPI2_MISO_PORT    GPIOC
#define SPI2_MISO_PIN     GPIO_PIN_8
#define SPI2_MISO_AF      GPIO_AF5_SPI2
#define SPI2_MOSI_PORT    GPIOC
#define SPI2_MOSI_PIN     GPIO_PIN_9
#define SPI2_MOSI_AF      GPIO_AF5_SPI2
#endif

#if defined(__CC_ARM) || defined(__CLANG_ARM)
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define HEAP_BEGIN      ((void *)&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="CSTACK"
#define HEAP_BEGIN      (__segment_end("CSTACK"))
#else
extern int __bss_end;
#define HEAP_BEGIN      ((void *)&__bss_end)
#endif

#define HEAP_END        CM32_SRAM_END

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

#endif
