/**
  ******************************************************************************
  * @file    hk32f39a_rcc.h
  * @version V1.0.0
  * @date    2019-08-05
  * @brief   This file contains all the functions prototypes for the RCC firmware 
  *          library.
  ****************************************************************************** 
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_RCC_H
#define __HK32F39A_RCC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"

/** @defgroup RCC_Exported_Types  */

typedef struct
{
  uint32_t SYSCLK_Frequency;  /*!< returns SYSCLK clock frequency expressed in Hz */
  uint32_t HCLK_Frequency;    /*!< returns HCLK clock frequency expressed in Hz */
  uint32_t PCLK1_Frequency;   /*!< returns PCLK1 clock frequency expressed in Hz */
  uint32_t PCLK2_Frequency;   /*!< returns PCLK2 clock frequency expressed in Hz */
  uint32_t ADCCLK_Frequency;  /*!< returns ADCCLK clock frequency expressed in Hz */
}RCC_ClocksTypeDef;



/** @defgroup HSE_configuration   */

#define RCC_HSE_OFF                      ((uint32_t)0x00000000)
#define RCC_HSE_ON                       ((uint32_t)0x00010000)
#define RCC_HSE_Bypass                   ((uint32_t)0x00040000)
#define IS_RCC_HSE(HSE) (((HSE) == RCC_HSE_OFF) || ((HSE) == RCC_HSE_ON) || \
                         ((HSE) == RCC_HSE_Bypass))



/** @defgroup PLL_entry_clock_source   */

#define RCC_PLLSource_HSI8M_Div2           ((uint32_t)0x00000000)
#define RCC_PLLSource_HSI56M		       ((uint32_t)0x00010008)
#define RCC_PLLSource_HSE		           ((uint32_t)0x00010000)

//#define RCC_PLLSource_HSE_Div2           ((uint32_t)0x00030000)
#define IS_RCC_PLL_SOURCE(SOURCE) (((SOURCE) == RCC_PLLSource_HSI8M_Div2) || \
                                   ((SOURCE) == RCC_PLLSource_HSI8M) || \
                                   ((SOURCE) == RCC_PLLSource_HSI56M) || \
                                   ((SOURCE) == RCC_PLLSource_HSE))

/** @defgroup PLL_Predivision_factor  */ 
#define RCC_PLLPREDIV_1                    ((uint32_t)0x00000000)  
#define RCC_PLLPREDIV_2                    ((uint32_t)0x00000001)
#define RCC_PLLPREDIV_3                    ((uint32_t)0x00000002)
#define RCC_PLLPREDIV_4                    ((uint32_t)0x00000003)
#define RCC_PLLPREDIV_5                    ((uint32_t)0x00000004)
#define RCC_PLLPREDIV_6                    ((uint32_t)0x00000005)
#define RCC_PLLPREDIV_7                    ((uint32_t)0x00000006)
#define RCC_PLLPREDIV_8                    ((uint32_t)0x00000007)
#define RCC_PLLPREDIV_9                    ((uint32_t)0x00000008)
#define RCC_PLLPREDIV_10                   ((uint32_t)0x00000009)
#define RCC_PLLPREDIV_11                   ((uint32_t)0x0000000a)
#define RCC_PLLPREDIV_12                   ((uint32_t)0x0000000b)
#define RCC_PLLPREDIV_13                   ((uint32_t)0x0000000c)
#define RCC_PLLPREDIV_14                   ((uint32_t)0x0000000d)
#define RCC_PLLPREDIV_15                   ((uint32_t)0x0000000e)
#define RCC_PLLPREDIV_16                   ((uint32_t)0x0000000f)
#define IS_RCC_PLL_PREDIV(PREDIV) (((PREDIV) == RCC_PLLPREDIV_1) || ((PREDIV) == RCC_PLLPREDIV_2)   || \
                              ((PREDIV) == RCC_PLLPREDIV_3) || ((PREDIV) == RCC_PLLPREDIV_4)   || \
                              ((PREDIV) == RCC_PLLPREDIV_5) || ((PREDIV) == RCC_PLLPREDIV_6)   || \
                              ((PREDIV) == RCC_PLLPREDIV_7) || ((PREDIV) == RCC_PLLPREDIV_8)   || \
                              ((PREDIV) == RCC_PLLPREDIV_9) || ((PREDIV) == RCC_PLLPREDIV_10) || \
                              ((PREDIV) == RCC_PLLPREDIV_11) || ((PREDIV) == RCC_PLLPREDIV_12) || \
                              ((PREDIV) == RCC_PLLPREDIV_13) || ((PREDIV) == RCC_PLLPREDIV_14) || \
                              ((PREDIV) == RCC_PLLPREDIV_15) || ((PREDIV) == RCC_PLLPREDIV_16))



/** @defgroup PLL_multiplication_factor   */

 #define RCC_PLLMul_2                    ((uint32_t)0x00000000)
 #define RCC_PLLMul_3                    ((uint32_t)0x00040000)
 #define RCC_PLLMul_4                    ((uint32_t)0x00080000)
 #define RCC_PLLMul_5                    ((uint32_t)0x000C0000)
 #define RCC_PLLMul_6                    ((uint32_t)0x00100000)
 #define RCC_PLLMul_7                    ((uint32_t)0x00140000)
 #define RCC_PLLMul_8                    ((uint32_t)0x00180000)
 #define RCC_PLLMul_9                    ((uint32_t)0x001C0000)
 #define RCC_PLLMul_10                   ((uint32_t)0x00200000)
 #define RCC_PLLMul_11                   ((uint32_t)0x00240000)
 #define RCC_PLLMul_12                   ((uint32_t)0x00280000)
 #define RCC_PLLMul_13                   ((uint32_t)0x002C0000)
 #define RCC_PLLMul_14                   ((uint32_t)0x00300000)
 #define RCC_PLLMul_15                   ((uint32_t)0x00340000)
 #define RCC_PLLMul_16                   ((uint32_t)0x00380000)
 #define IS_RCC_PLL_MUL(MUL) (((MUL) == RCC_PLLMul_2) || ((MUL) == RCC_PLLMul_3)   || \
                              ((MUL) == RCC_PLLMul_4) || ((MUL) == RCC_PLLMul_5)   || \
                              ((MUL) == RCC_PLLMul_6) || ((MUL) == RCC_PLLMul_7)   || \
                              ((MUL) == RCC_PLLMul_8) || ((MUL) == RCC_PLLMul_9)   || \
                              ((MUL) == RCC_PLLMul_10) || ((MUL) == RCC_PLLMul_11) || \
                              ((MUL) == RCC_PLLMul_12) || ((MUL) == RCC_PLLMul_13) || \
                              ((MUL) == RCC_PLLMul_14) || ((MUL) == RCC_PLLMul_15) || \
                              ((MUL) == RCC_PLLMul_16))


/** @defgroup System_clock_source   */

#define RCC_SYSCLKSource_HSI8M           ((uint32_t)0x00000000)
#define RCC_SYSCLKSource_HSE  	         ((uint32_t)0x00000001)
#define RCC_SYSCLKSource_PLLCLK          ((uint32_t)0x00000002)
#define RCC_SYSCLKSource_LSE        	 ((uint32_t)0x80000000)
#define RCC_SYSCLKSource_LSI     	     ((uint32_t)0x80000001)
#define RCC_SYSCLKSource_HSI56M          ((uint32_t)0x80000002)
#define RCC_SYSCLKSource_HSI28M          ((uint32_t)0x80000003)
#define RCC_SYSCLKSource_EXTCLK          ((uint32_t)0x80000004)

#define IS_RCC_SYSCLK_SOURCE(SOURCE) (((SOURCE) == RCC_SYSCLKSource_HSI8M) || \
                                      ((SOURCE) == RCC_SYSCLKSource_HSE) || \
                                      ((SOURCE) == RCC_SYSCLKSource_PLLCLK) || \
                                      ((SOURCE) == RCC_SYSCLKSource_LSE) || \
									  ((SOURCE) == RCC_SYSCLKSource_LSI) || \
									  ((SOURCE) == RCC_SYSCLKSource_HSI56M) || \
									  ((SOURCE) == RCC_SYSCLKSource_HSI28M) || \
                                      ((SOURCE) == RCC_SYSCLKSource_EXTCLK))

/** @defgroup AHB_clock_source   */

#define RCC_SYSCLK_Div1                  ((uint32_t)0x00000000)
#define RCC_SYSCLK_Div2                  ((uint32_t)0x00000080)
#define RCC_SYSCLK_Div4                  ((uint32_t)0x00000090)
#define RCC_SYSCLK_Div8                  ((uint32_t)0x000000A0)
#define RCC_SYSCLK_Div16                 ((uint32_t)0x000000B0)
#define RCC_SYSCLK_Div64                 ((uint32_t)0x000000C0)
#define RCC_SYSCLK_Div128                ((uint32_t)0x000000D0)
#define RCC_SYSCLK_Div256                ((uint32_t)0x000000E0)
#define RCC_SYSCLK_Div512                ((uint32_t)0x000000F0)
#define IS_RCC_HCLK(HCLK) (((HCLK) == RCC_SYSCLK_Div1) || ((HCLK) == RCC_SYSCLK_Div2) || \
                           ((HCLK) == RCC_SYSCLK_Div4) || ((HCLK) == RCC_SYSCLK_Div8) || \
                           ((HCLK) == RCC_SYSCLK_Div16) || ((HCLK) == RCC_SYSCLK_Div64) || \
                           ((HCLK) == RCC_SYSCLK_Div128) || ((HCLK) == RCC_SYSCLK_Div256) || \
                           ((HCLK) == RCC_SYSCLK_Div512))


/** @defgroup APB1_APB2_clock_source   */

#define RCC_HCLK_Div1                    ((uint32_t)0x00000000)
#define RCC_HCLK_Div2                    ((uint32_t)0x00000400)
#define RCC_HCLK_Div4                    ((uint32_t)0x00000500)
#define RCC_HCLK_Div8                    ((uint32_t)0x00000600)
#define RCC_HCLK_Div16                   ((uint32_t)0x00000700)
#define IS_RCC_PCLK(PCLK) (((PCLK) == RCC_HCLK_Div1) || ((PCLK) == RCC_HCLK_Div2) || \
                           ((PCLK) == RCC_HCLK_Div4) || ((PCLK) == RCC_HCLK_Div8) || \
                           ((PCLK) == RCC_HCLK_Div16))


/** @defgroup RCC_Interrupt_clear   */

#define RCC_IT_LSIRDY_CLEAR                    ((uint32_t)0x010000)
#define RCC_IT_LSERDY_CLEAR                    ((uint32_t)0x020000)
#define RCC_IT_HSIRDY_CLEAR                    ((uint32_t)0x040000)
#define RCC_IT_HSERDY_CLEAR                    ((uint32_t)0x080000)
#define RCC_IT_PLLRDY_CLEAR                    ((uint32_t)0x100000)
#define RCC_IT_CSS_CLEAR                       ((uint32_t)0x800000)

#define RCC_IT_LSIRDY                    ((uint8_t)0x01)
#define RCC_IT_LSERDY                    ((uint8_t)0x02)
#define RCC_IT_HSIRDY                    ((uint8_t)0x04)
#define RCC_IT_HSERDY                    ((uint8_t)0x08)
#define RCC_IT_PLLRDY                    ((uint8_t)0x10)
#define RCC_IT_CSS                       ((uint8_t)0x80)

 #define IS_RCC_IT(IT) ((((IT) & (uint8_t)0xE0) == 0x00) && ((IT) != 0x00))
 #define IS_RCC_GET_IT(IT) (((IT) == RCC_IT_LSIRDY) || ((IT) == RCC_IT_LSERDY) || \
                            ((IT) == RCC_IT_HSIRDY) || ((IT) == RCC_IT_HSERDY) || \
                            ((IT) == RCC_IT_PLLRDY) || ((IT) == RCC_IT_CSS))
 #define IS_RCC_CLEAR_IT(IT) ((((IT) & (uint8_t)0x60) == 0x00) && ((IT) != 0x00))


/** @defgroup USB_Device_clock_source   */

#define RCC_USBCLKSource_PLLCLK_1Div5   ((uint16_t)0x000)
#define RCC_USBCLKSource_PLLCLK_1Div    ((uint16_t)0x001)
#define RCC_USBCLKSource_PLLCLK_2Div	((uint16_t)0x100)
#define RCC_USBCLKSource_PLLCLK_2Div5	((uint16_t)0x200)
#define RCC_USBCLKSource_PLLCLK_3Div 	((uint16_t)0x300)
#define RCC_USBCLKSource_PLLCLK_3Div5 	((uint16_t)0x400)
#define RCC_USBCLKSource_PLLCLK_4Div 	((uint16_t)0x500)



 #define IS_RCC_USBCLK_SOURCE(SOURCE) (((SOURCE) == RCC_USBCLKSource_PLLCLK_1Div) || \
									 	((SOURCE) == RCC_USBCLKSource_PLLCLK_1Div5) || \
									 	((SOURCE) == RCC_USBCLKSource_PLLCLK_2Div) || \
									 	((SOURCE) == RCC_USBCLKSource_PLLCLK_2Div5) || \
									 	((SOURCE) == RCC_USBCLKSource_PLLCLK_3Div) || \
									 	((SOURCE) == RCC_USBCLKSource_PLLCLK_3Div5) || \
                                      	((SOURCE) == RCC_USBCLKSource_PLLCLK_4Div))


  

/** @defgroup ADC_clock_source   */

#define RCC_PCLK2_Div2                   ((uint32_t)0x00000000)
#define RCC_PCLK2_Div4                   ((uint32_t)0x00004000)
#define RCC_PCLK2_Div6                   ((uint32_t)0x00008000)
#define RCC_PCLK2_Div8                   ((uint32_t)0x0000C000)

#define IS_RCC_ADCCLK(ADCCLK) (((ADCCLK) == RCC_PCLK2_Div2) || ((ADCCLK) == RCC_PCLK2_Div4) || \
                               ((ADCCLK) == RCC_PCLK2_Div6) || ((ADCCLK) == RCC_PCLK2_Div8))


/** @defgroup LSE_configuration   */

#define RCC_LSE_OFF                      ((uint8_t)0x00)
#define RCC_LSE_ON                       ((uint8_t)0x01)
#define RCC_LSE_Bypass                   ((uint8_t)0x04)
#define IS_RCC_LSE(LSE) (((LSE) == RCC_LSE_OFF) || ((LSE) == RCC_LSE_ON) || \
                         ((LSE) == RCC_LSE_Bypass))


/** @defgroup RTC_clock_source   */

#define RCC_RTCCLKSource_LSE             ((uint32_t)0x00000100)
#define RCC_RTCCLKSource_LSI             ((uint32_t)0x00000200)
#define RCC_RTCCLKSource_HSE_Div128      ((uint32_t)0x00000300)
#define IS_RCC_RTCCLK_SOURCE(SOURCE) (((SOURCE) == RCC_RTCCLKSource_LSE) || \
                                      ((SOURCE) == RCC_RTCCLKSource_LSI) || \
                                      ((SOURCE) == RCC_RTCCLKSource_HSE_Div128))


/** @defgroup AHB_peripheral   */

#define RCC_AHBPeriph_DMA1               ((uint32_t)0x00000001)
#define RCC_AHBPeriph_DMA2               ((uint32_t)0x00000002)
#define RCC_AHBPeriph_SRAM               ((uint32_t)0x00000004)
#define RCC_AHBPeriph_FLITF              ((uint32_t)0x00000010)
#define RCC_AHBPeriph_CRC                ((uint32_t)0x00000040)
#define RCC_AHBPeriph_FSMC               ((uint32_t)0x00000100)
#define RCC_AHBPeriph_SDIO               ((uint32_t)0x00000400)

#define IS_RCC_AHB_PERIPH(PERIPH) ((((PERIPH) & 0xFFFFFAA8) == 0x00) && ((PERIPH) != 0x00))
/** @defgroup AHB_peripheral2   */

#define RCC_AHBPeriph2_COALU               	((uint32_t)0x00000001)
#define RCC_AHBPeriph2_AES                 	((uint32_t)0x00000002)
#define RCC_AHBPeriph2_HASH               	((uint32_t)0x00000004)
#define RCC_AHBPeriph2_TRNG              	((uint32_t)0x00000008)
#define RCC_AHBPeriph2_DCMI                	((uint32_t)0x00000010)
#define RCC_AHBPeriph2_QSPI              	((uint32_t)0x00000020)
#define IS_RCC_AHB_PERIPH2(PERIPH) ((((PERIPH) & 0xFFFFFF30) == 0x00) && ((PERIPH) != 0x00))



/** @defgroup APB2_peripheral   */

#define RCC_APB2Periph_AFIO              ((uint32_t)0x00000001)
#define RCC_APB2Periph_GPIOA             ((uint32_t)0x00000004)
#define RCC_APB2Periph_GPIOB             ((uint32_t)0x00000008)
#define RCC_APB2Periph_GPIOC             ((uint32_t)0x00000010)
#define RCC_APB2Periph_GPIOD             ((uint32_t)0x00000020)
#define RCC_APB2Periph_GPIOE             ((uint32_t)0x00000040)
#define RCC_APB2Periph_GPIOF             ((uint32_t)0x00000080)
#define RCC_APB2Periph_GPIOG             ((uint32_t)0x00000100)
#define RCC_APB2Periph_ADC1              ((uint32_t)0x00000200)
#define RCC_APB2Periph_ADC2              ((uint32_t)0x00000400)
#define RCC_APB2Periph_TIM1              ((uint32_t)0x00000800)
#define RCC_APB2Periph_SPI1              ((uint32_t)0x00001000)
#define RCC_APB2Periph_TIM8              ((uint32_t)0x00002000)
#define RCC_APB2Periph_USART1            ((uint32_t)0x00004000)
#define RCC_APB2Periph_ADC3              ((uint32_t)0x00008000)

#define IS_RCC_APB2_PERIPH(PERIPH) ((((PERIPH) & 0xFFFF0002) == 0x00) && ((PERIPH) != 0x00))


/** @defgroup APB2_peripheral2   */

#define RCC_APB2Periph2_USART6           ((uint32_t)0x00000001)
#define RCC_APB2Periph2_VC	             ((uint32_t)0x00000002)
#define RCC_APB2Periph2_SAIA             ((uint32_t)0x00000004)
#define RCC_APB2Periph2_SAIB             ((uint32_t)0x00000008)
#define RCC_APB2Periph2_PDMA             ((uint32_t)0x00000010)
#define RCC_APB2Periph2_PDMB             ((uint32_t)0x00000020)

#define IS_RCC_APB2_PERIPH2(PERIPH) ((((PERIPH) & 0xFFFFFFC0) == 0x00) && ((PERIPH) != 0x00))

/** @defgroup APB1_peripheral   */

#define RCC_APB1Periph_TIM2              ((uint32_t)0x00000001)
#define RCC_APB1Periph_TIM3              ((uint32_t)0x00000002)
#define RCC_APB1Periph_TIM4              ((uint32_t)0x00000004)
#define RCC_APB1Periph_TIM5              ((uint32_t)0x00000008)
#define RCC_APB1Periph_TIM6              ((uint32_t)0x00000010)
#define RCC_APB1Periph_TIM7              ((uint32_t)0x00000020)
#define RCC_APB1Periph_WWDG              ((uint32_t)0x00000800)
#define RCC_APB1Periph_SPI2              ((uint32_t)0x00004000)
#define RCC_APB1Periph_SPI3              ((uint32_t)0x00008000)
#define RCC_APB1Periph_USART2            ((uint32_t)0x00020000)
#define RCC_APB1Periph_USART3            ((uint32_t)0x00040000)
#define RCC_APB1Periph_USART4             ((uint32_t)0x00080000)
#define RCC_APB1Periph_USART5             ((uint32_t)0x00100000)
#define RCC_APB1Periph_I2C1              ((uint32_t)0x00200000)
#define RCC_APB1Periph_I2C2              ((uint32_t)0x00400000)
#define RCC_APB1Periph_USB               ((uint32_t)0x00800000)
#define RCC_APB1Periph_CAN1              ((uint32_t)0x02000000)
#define RCC_APB1Periph_CAN2              ((uint32_t)0x04000000)
#define RCC_APB1Periph_BKP               ((uint32_t)0x08000000)
#define RCC_APB1Periph_PWR               ((uint32_t)0x10000000)
#define RCC_APB1Periph_DAC               ((uint32_t)0x20000000)
#define IS_RCC_APB1_PERIPH(PERIPH) ((((PERIPH) & 0xC10137C0) == 0x00) && ((PERIPH) != 0x00))



/** @defgroup Clock_source_to_output_on_MCO_pin   */

#define RCC_MCO_NoClock                  ((uint8_t)0x00)
#define RCC_MCO_SYSCLK                   ((uint8_t)0x04)
#define RCC_MCO_HSI8M                    ((uint8_t)0x05)
#define RCC_MCO_HSE                      ((uint8_t)0x06)
#define RCC_MCO_PLLCLK_Div2              ((uint8_t)0x07)
#define RCC_MCO_LSI			             ((uint8_t)0x0C)
#define RCC_MCO_LSE		                 ((uint8_t)0x0D)
#define RCC_MCO_HSI28M              	 ((uint8_t)0x0E)

 #define IS_RCC_MCO(MCO) (((MCO) == RCC_MCO_NoClock) || ((MCO) == RCC_MCO_HSI8M) || \
                          ((MCO) == RCC_MCO_SYSCLK)  || ((MCO) == RCC_MCO_HSE) || \
                          ((MCO) == RCC_MCO_LSI)  || ((MCO) == RCC_MCO_LSE) || \
                          ((MCO) == RCC_MCO_PLLCLK_Div2)|| ((MCO) == RCC_MCO_HSI28M))



/*MCO_PRE define   */
#define RCC_MCO_Div1                ((uint8_t)0x00)
#define RCC_MCO_Div2                ((uint8_t)0x01)
#define RCC_MCO_Div4                ((uint8_t)0x02)
#define RCC_MCO_Div8                ((uint8_t)0x03)
#define RCC_MCO_Div16               ((uint8_t)0x04)
#define RCC_MCO_Div32               ((uint8_t)0x05)
#define RCC_MCO_Div64               ((uint8_t)0x06)
#define RCC_MCO_Div128              ((uint8_t)0x07)
#define RCC_MCOPRE_MASK             ((uint8_t)0x07)


/** @defgroup RCC_Flag   */

#define RCC_FLAG_HSI8MRDY                ((uint8_t)0x21)
#define RCC_FLAG_HSERDY                  ((uint8_t)0x31)
#define RCC_FLAG_PLLRDY                  ((uint8_t)0x39)
#define RCC_FLAG_LSERDY                  ((uint8_t)0x41)
#define RCC_FLAG_LSIRDY                  ((uint8_t)0x61)
#define RCC_FLAG_PINRST                  ((uint8_t)0x7A)
#define RCC_FLAG_PORRST                  ((uint8_t)0x7B)
#define RCC_FLAG_SFTRST                  ((uint8_t)0x7C)
#define RCC_FLAG_IWDGRST                 ((uint8_t)0x7D)
#define RCC_FLAG_WWDGRST                 ((uint8_t)0x7E)
#define RCC_FLAG_LPWRRST                 ((uint8_t)0x7F)
#define RCC_FLAG_HSI28MRDY               ((uint8_t)0x93)
#define RCC_FLAG_HSI56MRDY               ((uint8_t)0x91)



 #define IS_RCC_FLAG(FLAG) (((FLAG) == RCC_FLAG_HSI8MRDY) || ((FLAG) == RCC_FLAG_HSERDY) || \
                            ((FLAG) == RCC_FLAG_PLLRDY) || ((FLAG) == RCC_FLAG_LSERDY) || \
                            ((FLAG) == RCC_FLAG_LSIRDY) || ((FLAG) == RCC_FLAG_PINRST) || \
                            ((FLAG) == RCC_FLAG_PORRST) || ((FLAG) == RCC_FLAG_SFTRST) || \
                            ((FLAG) == RCC_FLAG_IWDGRST)|| ((FLAG) == RCC_FLAG_WWDGRST)|| \
                            ((FLAG) == RCC_FLAG_HSI28MRDY)|| ((FLAG) == RCC_FLAG_HSI56MRDY)|| \
                            ((FLAG) == RCC_FLAG_LPWRRST))


#define IS_RCC_CALIBRATION_VALUE(VALUE) ((VALUE) <= 0x1F)


#define  RCC_SAIA_EXT_CLK   RCC_CFGR6_SAIA_EXT_CLK
#define  RCC_SAIA_HSE_CLK   RCC_CFGR6_SAIA_HSE_CLK
#define  RCC_SAIA_PCLK2_Div2	RCC_CFGR6_SAIA_PCLK2_Div2
#define  RCC_SAIA_PCLK2_Div4	RCC_CFGR6_SAIA_PCLK2_Div4
#define  RCC_SAIA_PCLK2_Div6	RCC_CFGR6_SAIA_PCLK2_Div6
#define  RCC_SAIA_PCLK2_Div8	RCC_CFGR6_SAIA_PCLK2_Div8
#define  RCC_SAIA_PCLK2_Div12	RCC_CFGR6_SAIA_PCLK2_Div12
#define  RCC_SAIA_PCLK2_Div16	RCC_CFGR6_SAIA_PCLK2_Div16

#define  RCC_SAIB_EXT_CLK   RCC_CFGR6_SAIB_EXT_CLK
#define  RCC_SAIB_HSE_CLK   RCC_CFGR6_SAIB_HSE_CLK
#define  RCC_SAIB_PCLK2_Div2	RCC_CFGR6_SAIB_PCLK2_Div2
#define  RCC_SAIB_PCLK2_Div4	RCC_CFGR6_SAIB_PCLK2_Div4
#define  RCC_SAIB_PCLK2_Div6	RCC_CFGR6_SAIB_PCLK2_Div6
#define  RCC_SAIB_PCLK2_Div8	RCC_CFGR6_SAIB_PCLK2_Div8
#define  RCC_SAIB_PCLK2_Div12	RCC_CFGR6_SAIB_PCLK2_Div12
#define  RCC_SAIB_PCLK2_Div16	RCC_CFGR6_SAIB_PCLK2_Div16

 #define IS_SAI_FLAG(SAI_CLK) (((SAI_CLK) == RCC_SAIA_EXT_CLK) || ((SAI_CLK) == RCC_SAIA_HSE_CLK) || \
                            ((SAI_CLK) == RCC_SAIA_PCLK2_Div2) || ((SAI_CLK) == RCC_SAIA_PCLK2_Div4) || \
                            ((SAI_CLK) == RCC_SAIA_PCLK2_Div6) || ((SAI_CLK) == RCC_SAIA_PCLK2_Div8) || \
                            ((SAI_CLK) == RCC_SAIA_PCLK2_Div12) || ((SAI_CLK) == RCC_SAIA_PCLK2_Div16) || \
                            ((SAI_CLK) == RCC_SAIB_EXT_CLK)|| ((SAI_CLK) == RCC_SAIB_HSE_CLK)|| \
                            ((SAI_CLK) == RCC_SAIB_PCLK2_Div2)|| ((SAI_CLK) == RCC_SAIB_PCLK2_Div4)|| \
														((SAI_CLK) == RCC_SAIB_PCLK2_Div6)|| ((SAI_CLK) == RCC_SAIB_PCLK2_Div8)|| \
                            ((SAI_CLK) == RCC_SAIB_PCLK2_Div12)|| ((SAI_CLK) == RCC_SAIB_PCLK2_Div16))

void RCC_DeInit(void);
void RCC_HSEConfig(uint32_t RCC_HSE);
ErrorStatus RCC_WaitForStartUp(uint8_t RDYFlag);
void RCC_AdjustHSICalibrationValue(uint8_t HSICalibrationValue);
void RCC_HSI8MCmd(FunctionalState NewState);
void RCC_HSI28MCmd(FunctionalState NewState);
void RCC_HSI56MCmd(FunctionalState NewState);
void RCC_PLLConfig(uint32_t RCC_PLLSource, uint32_t RCC_PLLPre_DIV, uint32_t RCC_PLLMul);
void RCC_PLLCmd(FunctionalState NewState);
void RCC_SYSCLKConfig(uint32_t RCC_SYSCLKSource);
uint8_t RCC_GetSYSCLKSource(void);
void RCC_HCLKConfig(uint32_t RCC_SYSCLK);
void RCC_PCLK1Config(uint32_t RCC_HCLK);
void RCC_PCLK2Config(uint32_t RCC_HCLK);
void RCC_ITConfig(uint8_t RCC_IT, FunctionalState NewState);
void RCC_USBCLKConfig(uint32_t RCC_USBCLKSource);
void RCC_ADCCLKConfig(uint32_t RCC_PCLK2);
void RCC_LSEConfig(uint8_t RCC_LSE);
void RCC_LSICmd(FunctionalState NewState);
void RCC_RTCCLKConfig(uint32_t RCC_RTCCLKSource);
void RCC_RTCCLKCmd(FunctionalState NewState);
void RCC_GetClocksFreq(RCC_ClocksTypeDef* RCC_Clocks);
void RCC_AHBPeriphClockCmd(uint32_t RCC_AHBPeriph, FunctionalState NewState);
void RCC_AHBPeriph2ClockCmd(uint32_t RCC_AHBPeriph, FunctionalState NewState);

void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState);
void RCC_APB2Periph2ClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState);

void RCC_APB1PeriphClockCmd(uint32_t RCC_APB1Periph, FunctionalState NewState);
void RCC_APB2PeriphResetCmd(uint32_t RCC_APB2Periph, FunctionalState NewState);
void RCC_APB2Periph2ResetCmd(uint32_t RCC_APB2Periph, FunctionalState NewState);

void RCC_APB1PeriphResetCmd(uint32_t RCC_APB1Periph, FunctionalState NewState);
void RCC_BackupResetCmd(FunctionalState NewState);
void RCC_ClockSecuritySystemCmd(FunctionalState NewState);
void RCC_MCOConfig(uint8_t RCC_MCO, uint8_t RCC_MCOPRE);
FlagStatus RCC_GetFlagStatus(uint8_t RCC_FLAG);
void RCC_ClearFlag(void);
ITStatus RCC_GetITStatus(uint8_t RCC_IT);
void RCC_ClearITPendingBit(uint8_t RCC_IT);
void RCC_SAICLKConfig(uint32_t RCC_CLK);
uint32_t RCC_GetSaiCLKFreq(SAI_Block_TypeDef      *sai_mode);

#ifdef __cplusplus
}
#endif

#endif /* __HK32F39A_RCC_H */
 



