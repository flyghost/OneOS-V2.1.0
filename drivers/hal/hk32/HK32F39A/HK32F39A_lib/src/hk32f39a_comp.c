/**
  ******************************************************************************
  * @file    hk32f39a_comp.c
  * @author  Jane.L
  * @version V1.0.0
  * @brief   API files of Volt comp module.
  * @changelist
  ****************************************************************************** 
  */

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_comp.h"

/** @addtogroup HK32F39A_StdPeriph_Driver
  * @{
  */

/** @defgroup COMP 
  * @brief comp driver modules
  * @{
  */ 

/** @defgroup comp_Private_TypesDefinitions
  * @{
  */

/**
  * @}
  */

/** @defgroup comp_Private_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup comp_Private_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup comp_Private_Variables
  * @{
  */
#define CSR_CLEAR_MASK              ((uint32_t)0x0000001F)
#define CSR_CLEAR_status            ((uint32_t)0x00000000)

/**
  * @}
  */

/** @defgroup comp_Private_FunctionPrototypes
  * @{
  */

/**
  * @}
  */

/** @defgroup comp_Private_Functions
  * @{
  */

/**
  * @brief  Deinitializes the comp peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void VComp_DeInit(void)
{
  /* Enable  reset state */
   COMP->CSR = CSR_CLEAR_status;
}

/**
  * @brief  Initializes the voltage comp peripheral according to the specified 
  *         parameters in the DAC_InitStruct.
  * @param  VComp_Channel: the selected VC channel. 
  *   This parameter can be one of the following values:
  *     @arg VCCH1: VC Channel1 selected
  *     @arg VCCH2: VC Channel2 selected
  *     @arg VCCH3: VC Channel3 selected
  *     @arg VCCH4: VC Channel4 selected
  * @param  COMP_InitTypeDef: pointer to a COMP_InitTypeDef structure that
  *        contains the configuration information for the specified VC channel.
  * @retval None
  */
void VComp_Init(uint32_t VComp_Channel,COMP_InitTypeDef *vcomp)
{
	uint32_t tmpreg1 = 0, tmpreg2 = 0;
	/* Check the Vcomp parameters */
	 assert_param(IS_COMP_OUTPUTPOL(vcomp->OutputPol));
	 assert_param(IS_COMP_POWERMODE(vcomp->Mode));
	 assert_param(IS_COMP_HystVolt(vcomp->HystVol));
	 assert_param(IS_COMP_TRIGGERMODE(vcomp->TriggerMode));

	 tmpreg1 = COMP->CSR;
  /* Clear mode, pol, hystbits */
	 tmpreg1 &= ~(CSR_CLEAR_MASK << VComp_Channel);
  /* Configure for the selected comp channel: mode output polarity, hyst, */
  /* Set power mode and TENx bits according to mode value */
  /* Set outut polarity bits according to OutputPol value */
  /* Set hystesis bits according to HystVol value */ 
	 tmpreg2 = (vcomp->Mode | vcomp->OutputPol | vcomp->HystVol); 
	/* Calculate CSR register value depending on Channel */
	tmpreg1 |= tmpreg2 << VComp_Channel;
	 /* Write to comp register CSR */
	COMP->CSR = tmpreg1;
}


/**
  * @brief  Fills each vcomp_InitStruct member with its default value.
  * @param  vcomp_InitStruct : pointer to a vcomp_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void VComp_StructInit(COMP_InitTypeDef *vcomp)
{
/*--------------- Reset DAC init structure parameters values -----------------*/
  /* Initialize the power mode member */
  vcomp->Mode = COMP_POWERMODE_UltraLowSPEED;
  /* Initialize the output polarity member */
  vcomp->OutputPol = COMP_OUTPUT_LEVEL_Defult;
  /* Initialize the output voltage hystesis member */
  vcomp->HystVol = COMP_HystVol_None;
}

/**
  * @brief  Enables the specified VC channel.
  * @param  VComp_Channel: the selected VC channel. 
  *   This parameter can be one of the following values:
  *     @arg VCCH1: Channel1 selected
  *     @arg VCCH2: Channel2 selected
  *     @arg VCCH3: Channel3 selected
  *     @arg VCCH4: Channel4 selected
  * @retval None
  */
void VComp_Start(uint32_t VComp_Channel)
{
  /* Check the parameters */
  assert_param(IS_COMP_VCCH(VComp_Channel));
  /* Enable the selected DAC channel */
  COMP->CSR |= (VComp_EN << VComp_Channel);

}

/**
  * @brief disables the specified VC channel.
  * @param  VComp_Channel: the selected VC channel. 
  *   This parameter can be one of the following values:
  *     @arg VCCH1: Channel1 selected
  *     @arg VCCH2: Channel2 selected
  *     @arg VCCH3: Channel3 selected
  *     @arg VCCH4: Channel4 selected
  * @retval None
  */
void VComp_Stop(uint32_t VComp_Channel)
{
  /* Check the parameters */
  assert_param(IS_COMP_VCCH(VComp_Channel));
  /* Disable the selected VC channel */
  COMP->CSR &= ~(VComp_EN << VComp_Channel);
}

/**
  * @brief  Comparator IRQ handler.
  * @param  hcomp  COMP handle
  * @param  EXTI_Channel
            0x0010 0000 for VC1
            0x0020 0000 for VC2
            0x0040 0000 for VC3
            0x0080 0000 for VC4
            bit20 for VC1,
            bit21 for VC2,
            bit22 for VC3,
            bit23 for VC4, 
  * @retval None
  */
void VCOMP_IRQHandler(uint32_t EXTI_Channel)
{
  /* Get the EXTI line corresponding to the selected COMP instance */
  uint32_t exti_line = EXTI_Channel;
  
  /* Check COMP EXTI flag */
  if(READ_BIT(EXTI->PR, exti_line) != RESET)
  {
    /* Check whether comparator is in independent or window mode */
      /* Clear COMP EXTI line pending bit */
      WRITE_REG(EXTI->PR, exti_line);
  }
}

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
