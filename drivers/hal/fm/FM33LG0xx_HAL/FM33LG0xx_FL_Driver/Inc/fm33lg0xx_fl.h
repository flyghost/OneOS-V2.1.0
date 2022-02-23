/**
  *******************************************************************************************************
  * @file    fm33lg0xx_fl.h
  * @author  FMSH Application Team
  * @brief   Header file of FL Driver Library
  *******************************************************************************************************
  * @attention
  *
  * Copyright (c) [2019] [Fudan Microelectronics]
  * THIS SOFTWARE is licensed under the Mulan PSL v1.
  * can use this software according to the terms and conditions of the Mulan PSL v1.
  * You may obtain a copy of Mulan PSL v1 at:
  * http://license.coscl.org.cn/MulanPSL
  * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
  * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
  * PURPOSE.
  * See the Mulan PSL v1 for more details.
  *
  *******************************************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------------------------------*/
#ifndef __FM33LG0XX_FL_H
#define __FM33LG0XX_FL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief Select FM33LG0XX Device
  */
#if !defined  (FM33LG0XX)
#define FM33LG0XX
#endif /* FM33LG0XX */

/* Defines -------------------------------------------------------------------------------------------*/

/**
  * @brief List of drivers to be used.
  *
  * @note Uncomment following lines to disable specified driver.
  */

#ifndef MFANG

#define FL_ADC_DRIVER_ENABLED
#define FL_AES_DRIVER_ENABLED
#define FL_ATIM_DRIVER_ENABLED
#define FL_BSTIM16_DRIVER_ENABLED
#define FL_BSTIM32_DRIVER_ENABLED
#define FL_CAN_DRIVER_ENABLED
#define FL_CDIF_DRIVER_ENABLED
#define FL_CMU_DRIVER_ENABLED
#define FL_COMP_DRIVER_ENABLED
#define FL_CRC_DRIVER_ENABLED
#define FL_DAC_DRIVER_ENABLED
#define FL_DIVAS_DRIVER_ENABLED
#define FL_DMA_DRIVER_ENABLED
#define FL_EXTI_DRIVER_ENABLED
#define FL_FLASH_DRIVER_ENABLED
#define FL_GPIO_DRIVER_ENABLED
#define FL_GPTIM_DRIVER_ENABLED
#define FL_I2C_DRIVER_ENABLED
#define FL_IWDT_DRIVER_ENABLED
#define FL_LCD_DRIVER_ENABLED
#define FL_LPTIM16_DRIVER_ENABLED
#define FL_LPTIM32_DRIVER_ENABLED
#define FL_LPUART_DRIVER_ENABLED
#define FL_OPA_DRIVER_ENABLED
#define FL_PGL_DRIVER_ENABLED
#define FL_PMU_DRIVER_ENABLED
#define FL_RMU_DRIVER_ENABLED
#define FL_RNG_DRIVER_ENABLED
#define FL_RTCA_DRIVER_ENABLED
#define FL_RTCB_DRIVER_ENABLED
#define FL_SPI_DRIVER_ENABLED
#define FL_SVD_DRIVER_ENABLED
#define FL_U7816_DRIVER_ENABLED
#define FL_UART_DRIVER_ENABLED
#define FL_VAO_DRIVER_ENABLED
#define FL_VREF_DRIVER_ENABLED
#define FL_VREFP_DRIVER_ENABLED
#define FL_WWDT_DRIVER_ENABLED

#endif

/* Includes -------------------------------------------------------------------------------------------*/
#include "fm33xx.h"
#include "fm33_assert.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Macros ---------------------------------------------------------------------------------------------*/
/** @defgroup FL_Private_Macros FL Driver Library Private Macros
  * @{
  */

/**
  * @brief FM33LG0xx FL Driver Library version number
  */
#define __FM33LG0xx_FL_VERSION_MAIN   (0x02) /*!< [31:24] main version */
#define __FM33LG0xx_FL_VERSION_SUB1   (0x01) /*!< [23:16] sub1 version */
#define __FM33LG0xx_FL_VERSION_SUB2   (0x01) /*!< [15:0]  sub2 version */
#define __FM33LG0xx_FL_VERSION        ((__FM33LG0xx_FL_VERSION_MAIN  << 24)\
                                         |(__FM33LG0xx_FL_VERSION_SUB1 << 16)\
                                         |(__FM33LG0xx_FL_VERSION_SUB2))

/**
  * @brief Macros used by delay support functions
  */
#define FL_DELAY_US                (SystemCoreClock/1000000)
#define FL_DELAY_MS                (SystemCoreClock/1000)

/**
  * @}
  */

/* Types ----------------------------------------------------------------------------------------------*/
/** @defgroup FL_ET_Return FL Exported Return Type Defines
  * @{
  */

typedef enum
{
    FL_RESET = 0U,
    FL_SET = !FL_RESET
} FL_FlagStatus, FL_ITStatus;

typedef enum
{
    FL_DISABLE = 0U,
    FL_ENABLE = !FL_DISABLE
} FL_FunState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == FL_DISABLE) || ((STATE) == FL_ENABLE))

typedef enum
{
    FL_FAIL = 0U,
    FL_PASS = !FL_FAIL
} FL_ErrorStatus;

/**
  * @}
  */

/* Exported Functions ---------------------------------------------------------------------------------*/
/** @defgroup FL_EF_DELAY   Exported FL Driver Library Delay Support Functions
  * @{
  */

void FL_DelayInit(void);
void FL_DelayUs(uint32_t count);
void FL_DelayMs(uint32_t count);
void FL_DelayUsStart(uint32_t count);
void FL_DelayMsStart(uint32_t count);
bool FL_DelayEnd(void);

/**
  * @}
  */

/** @defgroup FL_EF_INIT   Exported FL Driver Library Init Functions
  * @{
  */

void FL_Init(void);

/**
  * @}
  */

/* Post Includes --------------------------------------------------------------------------------------*/
/**
  * @brief Include peripheral's header file
  */

#if defined(USE_FULL_ASSERT)
#include "fm33_assert.h"
#endif /* USE_FULL_ASSERT */

#if defined(FL_ADC_DRIVER_ENABLED)
#include "fm33lg0xx_fl_adc.h"
#endif /* FL_ADC_DRIVER_ENABLED */

#if defined(FL_AES_DRIVER_ENABLED)
#include "fm33lg0xx_fl_aes.h"
#endif /* FL_AES_DRIVER_ENABLED */

#if defined(FL_ATIM_DRIVER_ENABLED)
#include "fm33lg0xx_fl_atim.h"
#endif /* FL_ATIM_DRIVER_ENABLED */

#if defined(FL_BSTIM16_DRIVER_ENABLED)
#include "fm33lg0xx_fl_bstim16.h"
#endif /* FL_BSTIM16_DRIVER_ENABLED */

#if defined(FL_BSTIM32_DRIVER_ENABLED)
#include "fm33lg0xx_fl_bstim32.h"
#endif /* FL_BSTIM32_DRIVER_ENABLED */

#if defined(FL_CAN_DRIVER_ENABLED)
#include "fm33lg0xx_fl_can.h"
#endif /* FL_CAN_DRIVER_ENABLED */

#if defined(FL_CDIF_DRIVER_ENABLED)
#include "fm33lg0xx_fl_cdif.h"
#endif /* FL_CDIF_DRIVER_ENABLED */

#if defined(FL_CMU_DRIVER_ENABLED)
#include "fm33lg0xx_fl_cmu.h"
#endif /* FL_CMU_DRIVER_ENABLED */

#if defined(FL_COMP_DRIVER_ENABLED)
#include "fm33lg0xx_fl_comp.h"
#endif /* FL_COMP_DRIVER_ENABLED */

#if defined(FL_CRC_DRIVER_ENABLED)
#include "fm33lg0xx_fl_crc.h"
#endif /* FL_CRC_DRIVER_ENABLED */

#if defined(FL_DAC_DRIVER_ENABLED)
#include "fm33lg0xx_fl_dac.h"
#endif /* FL_DAC_DRIVER_ENABLED */

#if defined(FL_DIVAS_DRIVER_ENABLED)
#include "fm33lg0xx_fl_divas.h"
#endif /* FL_DIVAS_DRIVER_ENABLED */

#if defined(FL_DMA_DRIVER_ENABLED)
#include "fm33lg0xx_fl_dma.h"
#endif /* FL_DMA_DRIVER_ENABLED */

#if defined(FL_EXTI_DRIVER_ENABLED)
#include "fm33lg0xx_fl_exti.h"
#endif /* FL_EXTI_DRIVER_ENABLED */

#if defined(FL_FLASH_DRIVER_ENABLED)
#include "fm33lg0xx_fl_flash.h"
#endif /* FL_FLASH_DRIVER_ENABLED */

#if defined(FL_GPIO_DRIVER_ENABLED)
#include "fm33lg0xx_fl_gpio.h"
#endif /* FL_GPIO_DRIVER_ENABLED */

#if defined(FL_GPTIM_DRIVER_ENABLED)
#include "fm33lg0xx_fl_gptim.h"
#endif /* FL_GPTIM_DRIVER_ENABLED */

#if defined(FL_I2C_DRIVER_ENABLED)
#include "fm33lg0xx_fl_i2c.h"
#endif /* FL_I2C_DRIVER_ENABLED */

#if defined(FL_IWDT_DRIVER_ENABLED)
#include "fm33lg0xx_fl_iwdt.h"
#endif /* FL_IWDT_DRIVER_ENABLED */

#if defined(FL_LCD_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lcd.h"
#endif /* FL_LCD_DRIVER_ENABLED */

#if defined(FL_LPTIM16_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lptim16.h"
#endif /* FL_LPTIM16_DRIVER_ENABLED */

#if defined(FL_LPTIM32_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lptim32.h"
#endif /* FL_LPTIM32_DRIVER_ENABLED */

#if defined(FL_LPUART_DRIVER_ENABLED)
#include "fm33lg0xx_fl_lpuart.h"
#endif /* FL_LPUART_DRIVER_ENABLED */

#if defined(FL_OPA_DRIVER_ENABLED)
#include "fm33lg0xx_fl_opa.h"
#endif /* FL_OPA_DRIVER_ENABLED */

#if defined(FL_PGL_DRIVER_ENABLED)
#include "fm33lg0xx_fl_pgl.h"
#endif /* FL_PGL_DRIVER_ENABLED */

#if defined(FL_PMU_DRIVER_ENABLED)
#include "fm33lg0xx_fl_pmu.h"
#endif /* FL_PMU_DRIVER_ENABLED */

#if defined(FL_RMU_DRIVER_ENABLED)
#include "fm33lg0xx_fl_rmu.h"
#endif /* FL_RMU_DRIVER_ENABLED */

#if defined(FL_RNG_DRIVER_ENABLED)
#include "fm33lg0xx_fl_rng.h"
#endif /* FL_RNG_DRIVER_ENABLED */

#if defined(FL_RTCA_DRIVER_ENABLED)
#include "fm33lg0xx_fl_rtca.h"
#endif /* FL_RTCA_DRIVER_ENABLED */

#if defined(FL_RTCB_DRIVER_ENABLED)
#include "fm33lg0xx_fl_rtcb.h"
#endif /* FL_RTCB_DRIVER_ENABLED */

#if defined(FL_SPI_DRIVER_ENABLED)
#include "fm33lg0xx_fl_spi.h"
#endif /* FL_SPI_DRIVER_ENABLED */

#if defined(FL_SVD_DRIVER_ENABLED)
#include "fm33lg0xx_fl_svd.h"
#endif /* FL_SVD_DRIVER_ENABLED */

#if defined(FL_U7816_DRIVER_ENABLED)
#include "fm33lg0xx_fl_u7816.h"
#endif /* FL_U7816_DRIVER_ENABLED */

#if defined(FL_UART_DRIVER_ENABLED)
#include "fm33lg0xx_fl_uart.h"
#endif /* FL_UART_DRIVER_ENABLED */

#if defined(FL_VAO_DRIVER_ENABLED)
#include "fm33lg0xx_fl_vao.h"
#endif /* FL_VAO_DRIVER_ENABLED */

#if defined(FL_VREF_DRIVER_ENABLED)
#include "fm33lg0xx_fl_vref.h"
#endif /* FL_VREF_DRIVER_ENABLED */

#if defined(FL_VREFP_DRIVER_ENABLED)
#include "fm33lg0xx_fl_vrefp.h"
#endif /* FL_VREFP_DRIVER_ENABLED */

#if defined(FL_WWDT_DRIVER_ENABLED)
#include "fm33lg0xx_fl_wwdt.h"
#endif /* FL_WWDT_DRIVER_ENABLED */

#ifdef __cplusplus
}
#endif

#endif /* __FM33LG0XX_FL_H */

/*************************(C) COPYRIGHT Fudan Microelectronics **** END OF FILE*************************/
