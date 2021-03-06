/**************************************************************************
* @file hk32f0xx.h
* @version V1.0.1
* @date    2019-08-15
* @brief CMSIS Cortex-M Peripheral Access Layer for HKMicroChip devices
*
*****************************************************************************/
/** @addtogroup hk32f0xx
  * @{
  */
    
#ifndef __HK32F0xx_H
#define __HK32F0xx_H

#ifdef __cplusplus
 extern "C" {
#endif /* __cplusplus */
   
/** @addtogroup Library_configuration_section
  * @{
  */
  

/* Uncomment the line below according to the target HK32 device used in your
   application 
  */

#if !defined (HK32F030x4) && !defined (HK32F030x6) &&  !defined (HK32F030x8) &&\
    !defined (HK32F031x4) && !defined (HK32F031x6) &&                       \
    !defined (HK32F04Ax4) && !defined (HK32F04Ax6) && !defined (HK32F04Ax8) 

#endif
   
/*  Tip: To avoid modifying this file each time you need to switch between these
        devices, you can define the device in your toolchain compiler preprocessor.
  */

/**
  * @brief CMSIS Device version number V2.3.3
  */
#define __HK32F0_DEVICE_VERSION_MAIN   (0x02) /*!< [31:24] main version */
#define __HK32F0_DEVICE_VERSION_SUB1   (0x03) /*!< [23:16] sub1 version */
#define __HK32F0_DEVICE_VERSION_SUB2   (0x03) /*!< [15:8]  sub2 version */
#define __HK32F0_DEVICE_VERSION_RC     (0x00) /*!< [7:0]  release candidate */ 
#define __HK32F0_DEVICE_VERSION        ((__HK32F0_DEVICE_VERSION_MAIN << 24)\
                                        |(__HK32F0_DEVICE_VERSION_SUB1 << 16)\
                                        |(__HK32F0_DEVICE_VERSION_SUB2 << 8 )\
                                        |(__HK32F0_DEVICE_VERSION_RC))
                                             
/**
  * @}
  */

/** @addtogroup Device_Included
  * @{
  */

#if defined(HK32F030x4)
  #include "hk32f030x4x6x8.h"
#elif defined(HK32F030x6)
  #include "hk32f030x4x6x8.h"
#elif defined(HK32F030x8)
  #include "hk32f030x4x6x8.h"
#elif defined(HK32F031x4)
  #include "hk32f031x4x6.h"
#elif defined(HK32F031x6)
  #include "hk32f031x4x6.h"
#elif defined(HK32F04Ax)
  #include "hk32f04ax4x6x8.h"  
#else
 #error "Please select first the target HK32F0xx device used in your application (in hk32f0xx.h file)"
#endif

/**
  * @}
  */

/** @addtogroup Exported_types
  * @{
  */ 
typedef enum 
{
  RESET = 0, 
  SET = !RESET
} FlagStatus, ITStatus;

typedef enum 
{
  DISABLE = 0, 
  ENABLE = !DISABLE
} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum 
{
  ERROR = 0, 
  SUCCESS = !ERROR
} ErrorStatus;

/**
  * @}
  */
/*****************  Bit definition for SYSCFG_xxx ISR Wrapper register  ****************/
#define SYSCFG_ITLINE0_SR_EWDG                ((uint32_t)0x00000001) /*!< EWDG interrupt */
#define SYSCFG_ITLINE1_SR_PVDOUT              ((uint32_t)0x00000001) /*!< Power voltage detection -> exti[31] Interrupt */
#define SYSCFG_ITLINE1_SR_VDDIO2              ((uint32_t)0x00000002) /*!< VDDIO2 -> exti[16] Interrupt */
#define SYSCFG_ITLINE2_SR_RTC_WAKEUP          ((uint32_t)0x00000001) /*!< RTC WAKEUP -> exti[20] Interrupt */
#define SYSCFG_ITLINE2_SR_RTC_TSTAMP          ((uint32_t)0x00000002) /*!< RTC Time Stamp -> exti[19] interrupt */
#define SYSCFG_ITLINE2_SR_RTC_ALRA            ((uint32_t)0x00000003) /*!< RTC Alarm -> exti[17] interrupt .... */
#define SYSCFG_ITLINE3_SR_FLASH_ITF           ((uint32_t)0x00000001) /*!< Flash ITF Interrupt */
#define SYSCFG_ITLINE4_SR_CRS                 ((uint32_t)0x00000001) /*!< CRS interrupt */
#define SYSCFG_ITLINE4_SR_CLK_CTRL            ((uint32_t)0x00000002) /*!< CLK CTRL interrupt */
#define SYSCFG_ITLINE5_SR_EXTI0               ((uint32_t)0x00000001) /*!< External Interrupt 0 */
#define SYSCFG_ITLINE5_SR_EXTI1               ((uint32_t)0x00000002) /*!< External Interrupt 1 */
#define SYSCFG_ITLINE6_SR_EXTI2               ((uint32_t)0x00000001) /*!< External Interrupt 2 */
#define SYSCFG_ITLINE6_SR_EXTI3               ((uint32_t)0x00000002) /*!< External Interrupt 3 */
#define SYSCFG_ITLINE7_SR_EXTI4               ((uint32_t)0x00000001) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI5               ((uint32_t)0x00000002) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI6               ((uint32_t)0x00000004) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI7               ((uint32_t)0x00000008) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI8               ((uint32_t)0x00000010) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI9               ((uint32_t)0x00000020) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI10              ((uint32_t)0x00000040) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI11              ((uint32_t)0x00000080) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI12              ((uint32_t)0x00000100) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI13              ((uint32_t)0x00000200) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI14              ((uint32_t)0x00000400) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE7_SR_EXTI15              ((uint32_t)0x00000800) /*!< External Interrupt 15 to 4 */
#define SYSCFG_ITLINE8_SR_TSC_EOA             ((uint32_t)0x00000001) /*!< Touch control EOA Interrupt */
#define SYSCFG_ITLINE8_SR_TSC_MCE             ((uint32_t)0x00000002) /*!< Touch control MCE Interrupt */
#define SYSCFG_ITLINE9_SR_DMA1_CH1            ((uint32_t)0x00000001) /*!< DMA1 Channel 1 Interrupt */
#define SYSCFG_ITLINE10_SR_DMA1_CH2           ((uint32_t)0x00000001) /*!< DMA1 Channel 2 Interrupt */
#define SYSCFG_ITLINE10_SR_DMA1_CH3           ((uint32_t)0x00000002) /*!< DMA2 Channel 3 Interrupt */
#define SYSCFG_ITLINE11_SR_DMA1_CH4           ((uint32_t)0x00000001) /*!< DMA1 Channel 4 Interrupt */
#define SYSCFG_ITLINE11_SR_DMA1_CH5           ((uint32_t)0x00000002) /*!< DMA1 Channel 5 Interrupt */
#define SYSCFG_ITLINE12_SR_ADC                ((uint32_t)0x00000001) /*!< ADC Interrupt */
#define SYSCFG_ITLINE12_SR_COMP1              ((uint32_t)0x00000002) /*!< COMP1 Interrupt -> exti[21] */
#define SYSCFG_ITLINE12_SR_COMP2              ((uint32_t)0x00000004) /*!< COMP2 Interrupt -> exti[22] */
#define SYSCFG_ITLINE13_SR_TIM1_BRK           ((uint32_t)0x00000001) /*!< TIM1 BRK Interrupt */
#define SYSCFG_ITLINE13_SR_TIM1_UPD           ((uint32_t)0x00000002) /*!< TIM1 UPD Interrupt */
#define SYSCFG_ITLINE13_SR_TIM1_TRG           ((uint32_t)0x00000004) /*!< TIM1 TRG Interrupt */
#define SYSCFG_ITLINE13_SR_TIM1_CCU           ((uint32_t)0x00000008) /*!< TIM1 CCU Interrupt */
#define SYSCFG_ITLINE14_SR_TIM1_CC            ((uint32_t)0x00000001) /*!< TIM1 CC Interrupt */
#define SYSCFG_ITLINE15_SR_TIM2_GLB           ((uint32_t)0x00000001) /*!< TIM2 GLB Interrupt */
#define SYSCFG_ITLINE16_SR_TIM3_GLB           ((uint32_t)0x00000001) /*!< TIM3 GLB Interrupt */
#define SYSCFG_ITLINE17_SR_DAC                ((uint32_t)0x00000001) /*!< DAC Interrupt */
#define SYSCFG_ITLINE17_SR_TIM6_GLB           ((uint32_t)0x00000002) /*!< TIM6 GLB Interrupt */
#define SYSCFG_ITLINE19_SR_TIM14_GLB          ((uint32_t)0x00000001) /*!< TIM14 GLB Interrupt */
#define SYSCFG_ITLINE20_SR_TIM15_GLB          ((uint32_t)0x00000001) /*!< TIM15 GLB Interrupt */
#define SYSCFG_ITLINE21_SR_TIM16_GLB          ((uint32_t)0x00000001) /*!< TIM16 GLB Interrupt */
#define SYSCFG_ITLINE22_SR_TIM17_GLB          ((uint32_t)0x00000001) /*!< TIM17 GLB Interrupt */
#define SYSCFG_ITLINE23_SR_I2C1_GLB           ((uint32_t)0x00000001) /*!< I2C1 GLB Interrupt -> exti[23] */
#define SYSCFG_ITLINE24_SR_I2C2_GLB           ((uint32_t)0x00000001) /*!< I2C2 GLB Interrupt */
#define SYSCFG_ITLINE25_SR_SPI1               ((uint32_t)0x00000001) /*!< SPI1 Interrupt */
#define SYSCFG_ITLINE26_SR_SPI2               ((uint32_t)0x00000001) /*!< SPI2  Interrupt */
#define SYSCFG_ITLINE27_SR_USART1_GLB         ((uint32_t)0x00000001) /*!< USART1 GLB Interrupt -> exti[25] */
#define SYSCFG_ITLINE28_SR_USART2_GLB         ((uint32_t)0x00000001) /*!< USART2 GLB Interrupt -> exti[26] */
#define SYSCFG_ITLINE30_SR_CAN                ((uint32_t)0x00000001) /*!< CAN Interrupt */
#define SYSCFG_ITLINE30_SR_CEC                ((uint32_t)0x00000002) /*!< CEC Interrupt */

/** @addtogroup Exported_macros
  * @{
  */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define READ_REG(REG)         ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))




  
/** @addtogroup Library_configuration_section
  * @{
  */
  
/**
  * @brief HK32 Family
  */
#if !defined (HK32F0)
#define HK32F0
#endif /* HK32F0 */


/*
 * @brief In the following line adjust the value of External High Speed oscillator (HSE)
   used in your application 
   
   Tip: To avoid modifying this file each time you need to use different HSE, you
        can define the HSE value in your toolchain compiler preprocessor.
  */
#if !defined  (HSE_VALUE)     
#define HSE_VALUE    ((uint32_t)8000000) /*!< Value of the External oscillator in Hz*/
#endif /* HSE_VALUE */

/**
 * @brief In the following line adjust the External High Speed oscillator (HSE) Startup 
   Timeout value 
   */
#if !defined  (HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT   ((uint16_t)0xFFFF) /*!< Time out for HSE start up */
#endif /* HSE_STARTUP_TIMEOUT */

/**
 * @brief In the following line adjust the Internal High Speed oscillator (HSI) Startup 
   Timeout value 
   */
#if !defined  (HSI_STARTUP_TIMEOUT)
#define HSI_STARTUP_TIMEOUT   ((uint16_t)0xFFFF) /*!< Time out for HSI start up */
#endif /* HSI_STARTUP_TIMEOUT */

#if !defined  (HSI_VALUE) 
#define HSI_VALUE  ((uint32_t)8000000) /*!< Value of the Internal High Speed oscillator in Hz.
                                             The real value may vary depending on the variations
                                             in voltage and temperature.  */
#endif /* HSI_VALUE */

#if !defined  (HSI14_VALUE) 
#define HSI14_VALUE ((uint32_t)14000000) /*!< Value of the Internal High Speed oscillator for ADC in Hz.
                                             The real value may vary depending on the variations
                                             in voltage and temperature.  */
#endif /* HSI14_VALUE */

#if !defined  (HSI72_VALUE) 
#define HSI48_VALUE ((uint32_t)72000000) /*!< Value of the Internal High Speed oscillator for USB in Hz.
                                             The real value may vary depending on the variations
                                             in voltage and temperature.  */
#endif /* HSI72_VALUE */

#if !defined  (LSI_VALUE) 
#define LSI_VALUE  ((uint32_t)40000)    /*!< Value of the Internal Low Speed oscillator in Hz
                                             The real value may vary depending on the variations
                                             in voltage and temperature.  */
#endif /* LSI_VALUE */

#if !defined  (LSE_VALUE) 
#define LSE_VALUE  ((uint32_t)32768)    /*!< Value of the External Low Speed oscillator in Hz */
#endif /* LSE_VALUE */

/**
 * @brief HK32F0xx Standard Peripheral Library version number V1.4.0
   */
#define __HK32F0XX_STDPERIPH_VERSION_MAIN   (0x01) /*!< [31:24] main version */
#define __HK32F0XX_STDPERIPH_VERSION_SUB1   (0x05) /*!< [23:16] sub1 version */
#define __HK32F0XX_STDPERIPH_VERSION_SUB2   (0x00) /*!< [15:8]  sub2 version */
#define __HK32F0XX_STDPERIPH_VERSION_RC     (0x00) /*!< [7:0]  release candidate */ 
#define __HK32F0XX_STDPERIPH_VERSION        ((__HK32F0XX_STDPERIPH_VERSION_MAIN << 24)\
                                             |(__HK32F0XX_STDPERIPH_VERSION_SUB1 << 16)\
                                             |(__HK32F0XX_STDPERIPH_VERSION_SUB2 << 8)\
                                             |(__HK32F0XX_STDPERIPH_VERSION_RC))

#ifdef USE_STDPERIPH_DRIVER
  #include "hk32f0xx_hk_conf.h"
#endif


/*#endif USE_STDPERIPH_DRIVER*/
#endif /*#endif __HK32F0xx_H*/


