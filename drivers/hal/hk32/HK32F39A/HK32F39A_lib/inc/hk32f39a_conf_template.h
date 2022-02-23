/**
  ******************************************************************************
  * @file    hk32f39a_conf.h
  * @author  Thomas.W
  * @version V1.0  
  * @brief   Library configuration file.
  * @changelist  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_CONF_H
#define __HK32F39A_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ########################## Module Selection ############################## */

#define RTE_DEVICE_ADC
#define RTE_DEVICE_BKP
#define RTE_DEVICE_CAN
#define RTE_DEVICE_CRC
#define RTE_DEVICE_DAC
#define RTE_DEVICE_DBGMCU
#define RTE_DEVICE_EXTI
#define RTE_DEVICE_FLASH
#define RTE_DEVICE_FSMC
#define RTE_DEVICE_GPIO
#define RTE_DEVICE_I2C
#define RTE_DEVICE_IWDG
#define RTE_DEVICE_PWR
#define RTE_DEVICE_RCC
#define RTE_DEVICE_RTC
#define RTE_DEVICE_SDIO
#define RTE_DEVICE_SPI
#define RTE_DEVICE_TIM
#define RTE_DEVICE_USART
#define RTE_DEVICE_WWDG
#define RTE_DEVICE_COMP
#define CORTEX_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_NOR_MODULE_ENABLED
#define HAL_NAND_MODULE_ENABLED
#define HAL_SRAM_MODULE_ENABLED
/* ########################## The specifical module in HK32F39A ####################*/
#define RTE_DEVICE_TFT
#define HAL_SAI_MODULE_ENABLED
#define HAL_DCMI_MODULE_ENABLED
#define HAL_TRNG_MODULE_ENABLED
#define HAL_AES_MODULE_ENABLED
#define HAL_HASH_MODULE_ENABLED
#define HAL_QSPI_MODULE_ENABLED
#define HAL_RNG_MODULE_ENABLED
#define HAL_CRYP_MODULE_ENABLED
/* ########################## Oscillator Values adaptation ####################*/
/**
 * @brief In the following line adjust the value of External High Speed oscillator (HSE)
   used in your application   
   range 4M-32M*/ 
#define HSE_VALUE    	((uint32_t)8000000) /*!< Value of the External oscillator in Hz */
#define SAI_EXT_CLK		((uint32_t)49152000)
/* @brief In the following line adjust the External High Speed oscillator (HSE) Startup 
   Timeout value    */
#define HSE_STARTUP_TIMEOUT   ((uint16_t)0xFFFF) /*!< Time out for HSE start up */
#define HSI_STARTUP_TIMEOUT   ((uint16_t)0xFFFF) /*!< Time out for HSI start up */

#define HSI_VALUE    	((uint32_t)8000000) /*!< Value of the Internal oscillator in Hz*/
#define HSI8M_VALUE    	((uint32_t)8000000) /*!< Value of the Internal oscillator in Hz*/
#define HSI28M_VALUE    ((uint32_t)28000000) /*!< Value of the Internal oscillator in Hz*/
#define HSI56M_VALUE    ((uint32_t)56000000) /*!< Value of the Internal oscillator in Hz*/

#define LSE_VALUE    	((uint32_t)32768) /*!< Value of the LSE in Hz*/
#define LSI_VALUE    	((uint32_t)40000) /*!< Value of the LSI in Hz*/

#define EXTCLK_VALUE	((uint32_t)56000000) /*!< Value of the Internal oscillator in Hz*/


/* ########################### System Configuration ######################### */
/**
  * @brief This is the HAL system configuration section
  */
#define  VDD_VALUE                    3300U /*!< Value of VDD in mv */
#define  TICK_INT_PRIORITY            0x0FU /*!< tick interrupt priority */
#define  PREFETCH_ENABLE              1U

/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */
#ifdef  USE_FULL_ASSERT

/**
  * @brief  The assert_param macro is used for function's parameters check.
  * @param  expr: If expr is false, it calls assert_failed function which reports 
  *         the name of the source file and the source line number of the call 
  *         that failed. If expr is true, it returns no value.
  * @retval None
  */
  #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8_t *)__FILE__, __LINE__))
/* Exported functions ------------------------------------------------------- */
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0)
#endif /* USE_FULL_ASSERT */

/* Includes ------------------------------------------------------------------*/
/**
  * @brief Include module's header file
  */
#ifdef RTE_DEVICE_ADC
#include "hk32f39a_adc.h"
#endif
#ifdef RTE_DEVICE_BKP
#include "hk32f39a_bkp.h"
#endif
#ifdef RTE_DEVICE_CAN
#include "hk32f39a_can.h"
#endif
#ifdef RTE_DEVICE_CRC
#include "hk32f39a_crc.h"
#endif
#ifdef RTE_DEVICE_DAC
#include "hk32f39a_dac.h"
#endif
#ifdef RTE_DEVICE_DBGMCU
#include "hk32f39a_dbgmcu.h"
#endif
#ifdef RTE_DEVICE_EXTI
#include "hk32f39a_exti.h"
#endif
#ifdef RTE_DEVICE_FLASH
#include "hk32f39a_flash.h"
#endif
#ifdef RTE_DEVICE_FSMC
#include "hk32f39a_fsmc.h"
#endif
#ifdef RTE_DEVICE_GPIO
#include "hk32f39a_gpio.h"
#endif
#ifdef RTE_DEVICE_I2C
#include "hk32f39a_i2c.h"
#endif
#ifdef RTE_DEVICE_IWDG
#include "hk32f39a_iwdg.h"
#endif
#ifdef RTE_DEVICE_PWR
#include "hk32f39a_pwr.h"
#endif
#ifdef RTE_DEVICE_RCC
#include "hk32f39a_rcc.h"
#endif
#ifdef RTE_DEVICE_RTC
#include "hk32f39a_rtc.h"
#endif
#ifdef RTE_DEVICE_SDIO
#include "hk32f39a_sdio.h"
#endif
#ifdef RTE_DEVICE_SPI
#include "hk32f39a_spi.h"
#endif
#ifdef RTE_DEVICE_TIM
#include "hk32f39a_tim.h"
#endif
#ifdef RTE_DEVICE_USART
#include "hk32f39a_usart.h"
#endif
#ifdef RTE_DEVICE_WWDG
#include "hk32f39a_wwdg.h"
#endif
#ifdef RTE_DEVICE_COMP
#include "hk32f39a_comp.h"
#endif
	
#ifdef RTE_DEVICE_CACHE
#include "hk32f39a_cache.h"
#endif

#ifdef RTE_DEVICE_TFT
#include "hk32f39a_tft.h"
#endif

#ifdef CORTEX_MODULE_ENABLED
 #include "hk32f39a_cortex.h"
#endif /* CORTEX_MODULE_ENABLED */

#ifdef HAL_DMA_MODULE_ENABLED
  #include "hk32f39a_hal_dma.h"
	#include "hk32f39a_hal_dma_ex.h"
#endif /* HAL_DMA_MODULE_ENABLED */

#ifdef HAL_SAI_MODULE_ENABLED
 #include "hk32f39a_sai.h"
 #include "hk32f39a_sai_ex.h"
#endif /* HAL_SAI_MODULE_ENABLED */

#ifdef HAL_DCMI_MODULE_ENABLED
 #include "hk32f39a_dcmi.h"
 #include "hk32f39a_dcmi_ex.h"
#endif /* HAL_SAI_MODULE_ENABLED */

#ifdef HAL_SRAM_MODULE_ENABLED
 #include "hk32f39a_hal_sram.h"
#endif /* HAL_SRAM_MODULE_ENABLED */
	
#ifdef HAL_NOR_MODULE_ENABLED
 #include "hk32f39a_hal_nor.h"
#endif /* HAL_NOR_MODULE_ENABLED */

#ifdef HAL_NAND_MODULE_ENABLED
 #include "hk32f39a_hal_nand.h"
#endif /* HAL_NAND_MODULE_ENABLED */   

#ifdef HAL_QSPI_MODULE_ENABLED
 #include "hk32f39a_hal_qspi.h"
#endif /*HAL_QSPI_MODULE_ENABLED */ 

#ifdef HAL_RNG_MODULE_ENABLED
 #include "hk32f39a_hal_rng.h"
#endif /*HAL_RNG_MODULE_ENABLED */ 

#ifdef HAL_CRYP_MODULE_ENABLED
 #include "hk32f39a_hal_cryp.h"
 #include "hk32f39a_hal_cryp_ex.h"
#endif /*HAL_CRYP_MODULE_ENABLED */ 

#ifdef HAL_HASH_MODULE_ENABLED
 #include "hk32f39a_hal_hash.h"
#endif /*HAL_RNG_MODULE_ENABLED */

#ifdef __cplusplus
}
#endif

#endif 
