/**
  ******************************************************************************
  * @file    hk32f39a_comp.h
  * @author  Jane.L
  * @version V1.0.0
  * @brief   API files of volt comp module.
  * @changelist
  ****************************************************************************** 
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_COMP_H
#define __HK32F39A_COMP_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"

/** @addtogroup HK32F39A_StdPeriph_Driver
  * @{
  */

/** @addtogroup comp
  * @{
  */

/** @defgroup comp_Exported_Types
  * @{
  */

/** 
  * @brief  comp Init structure definition
  */

typedef struct
{
  uint32_t OutputPol;        /*!< Set comparator output polarity.
                                    This parameter can be a value of @ref COMP_OutputPolarity */
  uint32_t Mode;               /*!< Set comparator operating mode to adjust power and speed.
                                    Note: For the characteritics of comparator power modes
                                          (propagation delay and power consumption), refer to device datasheet.
                                    This parameter can be a value of @ref COMP_PowerMode */
  uint32_t HystVol;              /*!<Hysteresis voltage select.
                                    This parameter can be a value of @ref COMP_HystVol */
  uint32_t TriggerMode;        /*!< Set the comparator output triggering External Interrupt Line (EXTI).
                                    This parameter can be a value of @ref COMP_EXTI_TriggerMode */

}COMP_InitTypeDef;


/**
  * @}
  */
	/** @defgroup COMP_PowerMode COMP power mode
	  * @{
	  */
	/* Note: For the characteritics of comparator power modes					  */
	/*		 (propagation delay and power consumption), 						  */
	/*		 refer to device datasheet. 										  */
#define COMP_POWERMODE_UltraLowSPEED   ((uint32_t)0x00000000U)       /*!< COMP power mode to ultra low power 3uA*/
#define COMP_POWERMODE_LowSPEED        ((uint32_t)0x00000001U)       /*!< COMP power mode to low power 5uA*/
#define COMP_POWERMODE_MEDIUMSPEED     ((uint32_t)0x00000002U)       /*!< COMP power mode to midium power 40uA*/
#define COMP_POWERMODE_HighSPEED       ((uint32_t)0x00000003U)       /*!< COMP power mode to high power 100uA*/

/** @defgroup COMP_OutputPolarity 
  * @{
  */
/* When output polarity default, comparator output as following
   VINP>VINN output high
   VINP<VINN output low*/
#define COMP_OUTPUT_LEVEL_Defult           ((uint32_t)0x00000000U)
/* When output polarity inverted, comparator output as following 
   VINP>VINN output low
   VINP<VINN output high*/
#define COMP_OUTPUT_LEVEL_Invert           ((uint32_t)0x00000001U)

/** @defgroup COMP_HystVol 
  * @{
  */
#define COMP_HystVol_None           ((uint32_t)0x00000000U)
#define COMP_HystVol_30mV           ((uint32_t)0x00000001U)
/** @defgroup COMP_channel 
  * @{
  */
#define VCCH1            ((uint32_t)0x00000000U)
#define VCCH2            ((uint32_t)0x00000008U)
#define VCCH3            ((uint32_t)0x00000010U)
#define VCCH4            ((uint32_t)0x00000018U)
/** @defgroup COMP_interrupt trigger 
  * @{
  */
#define COMP_TRIGGERMODE_NONE            ((uint32_t)0x00000000U)
#define COMP_TRIGGERMODE_IT_RISING       ((uint32_t)0x00000001U)
#define COMP_TRIGGERMODE_IT_FALLING      ((uint32_t)0x00000002U)
#define COMP_TRIGGERMODE_EVENT_RISING    ((uint32_t)0x00000003U)
#define COMP_TRIGGERMODE_EVENT_FALLING   ((uint32_t)0x00000004U)


/** @defgroup COMP_ExtiLine COMP EXTI Lines
  * @{
  */
  #if 0
#define COMP_EXTI_LINE_COMP1           (EXTI_IMR_IM20)  /*!< EXTI line 20 connected to COMP1 output */
#define COMP_EXTI_LINE_COMP2           (EXTI_IMR_IM21)  /*!< EXTI line 21 connected to COMP2 output */
#define COMP_EXTI_LINE_COMP3           (EXTI_IMR_IM22)  /*!< EXTI line 22 connected to COMP3 output */
#define COMP_EXTI_LINE_COMP4           (EXTI_IMR_IM23)  /*!< EXTI line 23 connected to COMP4 output */
#endif

#define IS_COMP_POWERMODE(__POWERMODE__)    (((__POWERMODE__) == COMP_POWERMODE_UltraLowSPEED)  || \
                                             ((__POWERMODE__) == COMP_POWERMODE_LowSPEED) || \
                                             ((__POWERMODE__) == COMP_POWERMODE_MEDIUMSPEED) || \
                                             ((__POWERMODE__) == COMP_POWERMODE_HighSPEED)  )

#define IS_COMP_OUTPUTPOL(POL)  (((POL) == COMP_OUTPUT_LEVEL_Defult)  || \
                                 ((POL) == COMP_OUTPUT_LEVEL_Invert))

#define IS_COMP_HystVolt(hyst)  (((hyst) == COMP_HystVol_None)  || \
                                 ((hyst) == COMP_HystVol_30mV))

#define IS_COMP_TRIGGERMODE(__TRIGGERMODE__) (((__TRIGGERMODE__) == COMP_TRIGGERMODE_NONE)                 || \
                                              ((__TRIGGERMODE__) == COMP_TRIGGERMODE_IT_RISING)            || \
                                              ((__TRIGGERMODE__) == COMP_TRIGGERMODE_IT_FALLING)           || \
                                              ((__TRIGGERMODE__) == COMP_TRIGGERMODE_EVENT_RISING)         || \
                                              ((__TRIGGERMODE__) == COMP_TRIGGERMODE_EVENT_FALLING)   )

#define IS_COMP_VCCH(__VCCH__)    (((__VCCH__) == VCCH1)  || \
                                             ((__VCCH__) == VCCH2) || \
                                             ((__VCCH__) == VCCH3) || \
                                             ((__VCCH__) == VCCH4)  )


void VComp_DeInit(void);
void VComp_Init(uint32_t VComp_Channel,COMP_InitTypeDef *vcomp);
void VComp_StructInit(COMP_InitTypeDef *vcomp);
void VComp_Start(uint32_t VComp_Channel);
void VComp_Stop(uint32_t VComp_Channel);
void VCOMP_IRQHandler(uint32_t EXTI_Channel);



#endif /*__HK32F39A_DAC_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/******************* (C) COPYRIGHT   HKMicroChip *****END OF FILE****/
