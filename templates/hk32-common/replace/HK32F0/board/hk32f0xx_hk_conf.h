/**
  ******************************************************************************
  * @file    hk32f0xx_hk_conf.h
  * @brief   hk32f0xx  configuration file.
  *          The file is the unique include file that the application programmer
	*          is using in the C source code.it is a patch file 
  ******************************************************************************
**/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F0xx_HK_CONF_H
#define __HK32F0xx_HK_CONF_H

 
/* Exported constants --------------------------------------------------------*/

/* ########################## Module Selection ############################## */
/**
  * @brief This is the list of modules to be used in the hk driver with HK32F0XX 
  */
 
 
// 如果用到此功能，则把一下宏定义注释去掉即可

/*	#define HK_ADC_MODULE_ENABLED */ 
/*	#define HK_CAN_MODULE_ENABLED */ 
/*	#define HK_CRC_MODULE_ENABLED */  
/*	#define HK_DBGMCUC_MODULE_ENABLED*/  
/*	#define HK_DMA_MODULE_ENABLED */ 
/*	#define HK_EXTI_MODULE_ENABLED */ 
/*	#define HK_FLASH_MODULE_ENABLED */ 
/*	#define HK_GPIO_MODULE_ENABLED */ 
/*	#define HK_SYSCFG_MODULE_ENABLED */ 
/*	#define HK_I2C_MODULE_ENABLED */ 
/*	#define HK_IWDG_MODULE_ENABLED */ 
/*	#define HK_PWR_MODULE_ENABLED */ 
/*	#define HK_RCC_MODULE_ENABLED */ 
/*	#define HK_RTC_MODULE_ENABLED */ 
/*	#define HK_SPI_MODULE_ENABLED */ 
/*	#define HK_TIM_MODULE_ENABLED */ 
/*	#define HK_USART_MODULE_ENABLED */ 
/*	#define HK_WWDG_MODULE_ENABLED */ 
/*	#define HK_MISC_MODULE_ENABLED */ 
/*	#define HK_DIVSQRT_MODULE_ENABLED  */ //除法开方模块

	#define HK_ADC_MODULE_ENABLED 
	#define HK_CAN_MODULE_ENABLED 
	#define HK_CRC_MODULE_ENABLED   
	#define HK_DBGMCU_MODULE_ENABLED  
	#define HK_DMA_MODULE_ENABLED 
	#define HK_EXTI_MODULE_ENABLED 
	#define HK_FLASH_MODULE_ENABLED 
	#define HK_GPIO_MODULE_ENABLED  
	#define HK_SYSCFG_MODULE_ENABLED 
	#define HK_I2C_MODULE_ENABLED  
	#define HK_IWDG_MODULE_ENABLED  
	#define HK_PWR_MODULE_ENABLED  
	#define HK_RCC_MODULE_ENABLED  
	#define HK_RTC_MODULE_ENABLED  
	#define HK_SPI_MODULE_ENABLED  
	#define HK_TIM_MODULE_ENABLED  
	#define HK_USART_MODULE_ENABLED 
	#define HK_WWDG_MODULE_ENABLED 
	#define HK_MISC_MODULE_ENABLED 
	#define HK_DIVSQRT_MODULE_ENABLED   //除法开方模块
	
/* Includes ------------------------------------------------------------------*/
	#ifdef HK_ADC_MODULE_ENABLED
 	#include "hk32f0xx_adc.h"
 	#endif
	#ifdef HK_CRC_MODULE_ENABLED
	#include "hk32f0xx_crc.h"
	#endif
	#ifdef HK_DBGMCU_MODULE_ENABLED
	#include "hk32f0xx_dbgmcu.h"
	#endif
	#ifdef HK_DMA_MODULE_ENABLED
	#include "hk32f0xx_dma.h"
	#endif
	#ifdef HK_EXTI_MODULE_ENABLED
	#include "hk32f0xx_exti.h"
	#endif
	#ifdef HK_FLASH_MODULE_ENABLED
	#include "hk32f0xx_flash.h"	
	#endif
	#ifdef HK_GPIO_MODULE_ENABLED
	#include "hk32f0xx_gpio.h"
	#endif
	#ifdef HK_SYSCFG_MODULE_ENABLED
	#include "hk32f0xx_syscfg.h"
	#endif
	#ifdef HK_I2C_MODULE_ENABLED
	#include "hk32f0xx_i2c.h"
	#endif
	#ifdef HK_IWDG_MODULE_ENABLED
	#include "hk32f0xx_iwdg.h"
	#endif
	#ifdef HK_PWR_MODULE_ENABLED
	#include "hk32f0xx_pwr.h"
	#endif
	#ifdef HK_RCC_MODULE_ENABLED
	#include "hk32f0xx_rcc.h"
	#endif
	#ifdef HK_RTC_MODULE_ENABLED
	#include "hk32f0xx_rtc.h"
	#endif
	#ifdef HK_SPI_MODULE_ENABLED
	#include "hk32f0xx_spi.h"
	#endif
	#ifdef HK_TIM_MODULE_ENABLED
	#include "hk32f0xx_tim.h"
	#endif
	#ifdef HK_USART_MODULE_ENABLED
	#include "hk32f0xx_usart.h"
	#endif
	#ifdef HK_WWDG_MODULE_ENABLED
	#include "hk32f0xx_wwdg.h"
 	#endif
 	#ifdef HK_MISC_MODULE_ENABLED
	#include "hk32f0xx_misc.h"  /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */
	#endif
	#ifdef HK_DIVSQRT_MODULE_ENABLED
	#include "hk32f0xx_divsqrt.h"
	#endif

	/* Exported macro ------------------------------------------------------------*/
	#ifdef  USE_FULL_ASSERT
	/**
		* @brief  The assert_param macro is used for function's parameters check.
		* @param  expr: If expr is false, it calls assert_failed function
		*         which reports the name of the source file and the source
		*         line number of the call that failed. 
		*         If expr is true, it returns no value.
		* @retval None
		*/
		#define assert_param(expr) ((expr) ? (void)0U : assert_failed((char *)__FILE__, __LINE__))
	/* Exported functions ------------------------------------------------------- */
		void assert_failed(char* file, uint32_t line);
	#else
		#define assert_param(expr) ((void)0U)
	#endif /* USE_FULL_ASSERT */  

		
#endif /* __HK32F0XX_CONF_H */
