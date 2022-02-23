/**
  ******************************************************************************
  * @file    hk32f39a_pwr.c
  * @author  Thomas.W
  * @version V1.0  
  * @brief   API file of PWR module
  * @changelist  
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_pwr.h"
#include "hk32f39a_rcc.h"

/** @defgroup PWR_Private_Defines
  * @{
  */

/* --------- PWR registers bit address in the alias region ---------- */
#define PWR_OFFSET               (PWR_BASE - PERIPH_BASE)

/* --- CR Register ---*/

/* Alias word address of DBP bit */
#define CR_OFFSET                (PWR_OFFSET + 0x00)
#define DBP_BitNumber            0x08
#define CR_DBP_BB                (PERIPH_BB_BASE + (CR_OFFSET * 32) + (DBP_BitNumber * 4))

/* Alias word address of PVDE bit */
#define PVDE_BitNumber           0x04
#define CR_PVDE_BB               (PERIPH_BB_BASE + (CR_OFFSET * 32) + (PVDE_BitNumber * 4))

/* --- CSR Register ---*/

/* Alias word address of EWUP bit */
#define CSR_OFFSET               (PWR_OFFSET + 0x04)
#define EWUP1_BitNumber           0x08
#define EWUP2_BitNumber           0x09
#define EWUP3_BitNumber           0x0A
#define CSR_EWUP1_BB              (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (EWUP1_BitNumber * 4))
#define CSR_EWUP2_BB              (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (EWUP2_BitNumber * 4))
#define CSR_EWUP3_BB              (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (EWUP3_BitNumber * 4))

/* ------------------ PWR registers bit mask ------------------------ */

/* CR register bit mask */
#define CR_DS_MASK               ((uint32_t)0xFFFFFFFC)
#define CR_PLS_MASK              ((uint32_t)0xFFFFFF1F)

/* --- CSR2 Register ---*/
/* Alias word address of EWUP_RTC/SHDS/BKPPDS bit */
#define CSR2_OFFSET               (PWR_OFFSET + 0x30)
#define BKPPDS_BitNumber           	0x06
#define SHDS_BitNumber           		0x07
#define EWUP_RTC_BitNumber          0x0F
#define CSR2_BKPPDS_BB              (PERIPH_BB_BASE + (CSR2_OFFSET * 32) + (BKPPDS_BitNumber * 4))
#define CSR2_SHDS_BB              (PERIPH_BB_BASE + (CSR2_OFFSET * 32) + (SHDS_BitNumber * 4))
#define CSR2_EWUP_RTC_BB              (PERIPH_BB_BASE + (CSR2_OFFSET * 32) + (EWUP_RTC_BitNumber * 4))

/* --- DAC_LP_CTL Register ---*/
#define DAC_LP_CTL_OFFSET               (PWR_OFFSET + 0x40)
#define DAC1_ALP_BitNumber           	0x0
#define DAC2_ALP_BitNumber           	0x8
#define DAC1_ALP_BB              (PERIPH_BB_BASE + (DAC_LP_CTL_OFFSET * 32) + (DAC1_ALP_BitNumber * 4))
#define DAC2_ALP_BB              (PERIPH_BB_BASE + (DAC_LP_CTL_OFFSET * 32) + (DAC2_ALP_BitNumber * 4))
  
/**
  * @brief  Deinitializes the PWR peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void PWR_DeInit(void)
{
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, ENABLE);
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_PWR, DISABLE);
}

/**
  * @brief  Enables or disables access to the RTC and backup registers.
  * @param  NewState: new state of the access to the RTC and backup registers.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void PWR_BackupAccessCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CR_DBP_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables the Power Voltage Detector(PVD).
  * @param  NewState: new state of the PVD.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void PWR_PVDCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CR_PVDE_BB = (uint32_t)NewState;
}

/**
  * @brief  Configures the voltage threshold detected by the Power Voltage Detector(PVD).
  * @param  PWR_PVDLevel: specifies the PVD detection level
  *   This parameter can be one of the following values:
  *     @arg PWR_PVDLevel_2V2: PVD detection level set to 2.2V
  *     @arg PWR_PVDLevel_2V3: PVD detection level set to 2.3V
  *     @arg PWR_PVDLevel_2V4: PVD detection level set to 2.4V
  *     @arg PWR_PVDLevel_2V5: PVD detection level set to 2.5V
  *     @arg PWR_PVDLevel_2V6: PVD detection level set to 2.6V
  *     @arg PWR_PVDLevel_2V7: PVD detection level set to 2.7V
  *     @arg PWR_PVDLevel_2V8: PVD detection level set to 2.8V
  *     @arg PWR_PVDLevel_2V9: PVD detection level set to 2.9V
  * @retval None
  */
void PWR_PVDLevelConfig(uint32_t PWR_PVDLevel)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_PWR_PVD_LEVEL(PWR_PVDLevel));
  tmpreg = PWR->CR;
  /* Clear PLS[7:5] bits */
  tmpreg &= CR_PLS_MASK;
  /* Set PLS[7:5] bits according to PWR_PVDLevel value */
  tmpreg |= PWR_PVDLevel;
  /* Store the new value */
  PWR->CR = tmpreg;
}

/**
  * @brief  Enables or disables the WakeUp Pin functionality.
  * @param  NewState: new state of the WakeUp Pin functionality.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void PWR_WakeUpPin1Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR_EWUP1_BB = (uint32_t)NewState;
}


void PWR_WakeUpPin2Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR_EWUP2_BB = (uint32_t)NewState;
}

void PWR_WakeUpPin3Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR_EWUP3_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables the RTC wakeup functionality in shutdown mode.
  * @param  NewState: This parameter can be: ENABLE or DISABLE
  * @retval None
  */
void PWR_EWUP_RTCCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR2_EWUP_RTC_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables shutdown mode.
  * @param  NewState: This parameter can be: ENABLE or DISABLE
  * @retval None
  */
void PWR_SHDS_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR2_SHDS_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables BKP and RTC domain power in shutdown mode.
  * @param  NewState: This parameter can be: ENABLE or DISABLE
  					ENABLE: Automatically power off BKP and RTC power when enter shutdowm mode
  					DISABLE: Don't power off BKP and RTC power when enter shutdowm mode
  * @retval None
  */
void PWR_BKPPDS_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR2_BKPPDS_BB = (uint32_t)NewState;
}




/**
  * @brief  Enters STOP mode.
  * @param  PWR_Regulator: specifies the regulator state in STOP mode.
  *   This parameter can be one of the following values:
  *     @arg PWR_Regulator_ON: STOP mode with regulator ON
  *     @arg PWR_Regulator_LowPower: STOP mode with regulator in low power mode
  * @param  PWR_STOPEntry: specifies if STOP mode in entered with WFI or WFE instruction.
  *   This parameter can be one of the following values:
  *     @arg PWR_STOPEntry_WFI: enter STOP mode with WFI instruction
  *     @arg PWR_STOPEntry_WFE: enter STOP mode with WFE instruction
  * @retval None
  */
void PWR_EnterSTOPMode(uint32_t PWR_Regulator, uint8_t PWR_STOPEntry)
{
  uint32_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_PWR_REGULATOR(PWR_Regulator));
  assert_param(IS_PWR_STOP_ENTRY(PWR_STOPEntry));
  
  /* Select the regulator state in STOP mode ---------------------------------*/
  tmpreg = PWR->CR;
  /* Clear PDDS and LPDS bits */
  tmpreg &= CR_DS_MASK;
  /* Set LPDS bit according to PWR_Regulator value */
  tmpreg |= PWR_Regulator;
  /* Store the new value */
  PWR->CR = tmpreg;
  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP;
  
  /* Select STOP mode entry --------------------------------------------------*/
  if(PWR_STOPEntry == PWR_STOPEntry_WFI)
  {   
    /* Request Wait For Interrupt */
    __WFI();
  }
  else
  {
    /* Request Wait For Event */
  	__SEV();
    __WFE();
	__WFE();
  }
  
  /* Reset SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR &= (uint32_t)~((uint32_t)SCB_SCR_SLEEPDEEP);  
}

/**
  * @brief  Enters STANDBY mode.
  * @param  None
  * @retval None
  */
void PWR_EnterSTANDBYMode(void)
{
  /* Clear Wake-up flag */
  PWR->CR |= PWR_CR_CWUF;
  /* Select STANDBY mode */
  PWR->CR |= PWR_CR_PDDS;
  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP;
/* This option is used to ensure that store operations are completed */
#if defined ( __CC_ARM   )
  __force_stores();
#endif
  /* Request Wait For Interrupt */
  __WFI();
}

/**
  * @brief  Enters SHUTDOWN mode.
  * @param  None
  * @retval None
  */
void PWR_EnterShutDownMode(FunctionalState RTC_PD_IN_SHUTDOWN)
{
  /* Clear Wake-up flag */
  PWR->CR |= PWR_CR_CWUF;
  /* Select STANDBY mode */
  PWR->CR |= PWR_CR_PDDS;
  /* CLEAR SHUTDOWN flags */ 
  PWR->CSR2 |= PWR_CSR2_CSWUF ;
  PWR->CSR2 |= PWR_CSR2_CSHUTF;
  /* Enable Shutdown */ 
  PWR->CSR2 |= PWR_CSR2_SHDS;        // enable shutdown 
  if (RTC_PD_IN_SHUTDOWN) 
    PWR->CSR2 |= PWR_CSR2_BKPPDS ;   // power down BKP and RTC domain in Shutdown Mode  

  /* Set SLEEPDEEP bit of Cortex System Control Register */
  SCB->SCR |= SCB_SCR_SLEEPDEEP;
/* This option is used to ensure that store operations are completed */
#if defined ( __CC_ARM   )
  __force_stores();
#endif
  /* Request Wait For Interrupt */
  __WFI();
}




/**
  * @brief  Checks whether the specified PWR flag is set or not.
  * @param  PWR_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg PWR_FLAG_WU: Wake Up flag
  *     @arg PWR_FLAG_SB: StandBy flag
  *     @arg PWR_FLAG_PVDO: PVD Output
  *     @arg PWR_FLAG_LDORDY: Internal CPU LDO ready flag
  * @retval The new state of PWR_FLAG (SET or RESET).
  */
FlagStatus PWR_GetFlagStatus(uint32_t PWR_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  assert_param(IS_PWR_GET_FLAG(PWR_FLAG));
  
  if ((PWR->CSR & PWR_FLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  /* Return the flag status */
  return bitstatus;
}

/**
  * @brief  Clears the PWR's pending flags.
  * @param  PWR_FLAG: specifies the flag to clear.
  *   This parameter can be one of the following values:
  *     @arg PWR_FLAG_WU: Wake Up flag
  *     @arg PWR_FLAG_SB: StandBy flag
  * @retval None
  */
void PWR_ClearFlag(uint32_t PWR_FLAG)
{
  /* Check the parameters */
  assert_param(IS_PWR_CLEAR_FLAG(PWR_FLAG));
         
  PWR->CR |=  PWR_FLAG << 2;
}

/**
  * @brief  Clears the PWR CSR2 pending flags.
  * @param  PWR_FLAG: specifies the flag to clear.
  *   This parameter can be one of the following values:
  *     @arg PWR_FLAG_CSHUTF: Clear shutdown flag
  *     @arg PWR_FLAG_CSWUF: Clear shutdown wakeup flag
  * @retval None
  */
void PWR_CSR2_ClearFlag(uint32_t PWR_FLAG)
{
  /* Check the parameters */         
  PWR->CSR2 |=  PWR_FLAG;
}

/**
  * @brief  Checks whether the specified PWR flag in CSR2 is set or not.
  * @param  PWR_FLAG: specifies the flag to check.
  *   This parameter can be one of the following values:
  *     @arg PWR_FLAG_SHUTF: shutdown flag, has been enter shutdown mode
  *     @arg PWR_FLAG_SWUF: shutdown wakeup flag, has been wakeup by RTC or wakeup pin from shutdown mode  
  * @retval The new state of PWR_FLAG (SET or RESET).
  */
FlagStatus PWR_CSR2_GetFlagStatus(uint32_t PWR_FLAG)
{
  FlagStatus bitstatus = RESET;
  /* Check the parameters */
  if ((PWR->CSR2 & PWR_FLAG) != (uint32_t)RESET)
  {
    bitstatus = SET;
  }
  else
  {
    bitstatus = RESET;
  }
  /* Return the flag status */
  return bitstatus;
}

/**
  * @brief  Config wakeup pin1 polarity.
  * @param  WakePolarity: specifies wakeup pin polarity.
  *   This parameter can be one of the following values:
  *     @arg RISING or FALLING
  * @retval None.
  */ 
void PWR_WakeUpPin1Polarity(WakePolarity eWakePolarity)
{
  /* Check the parameters */
  if(eWakePolarity == RISING)
  {
  		PWR->WUP_POL &=  ~PWR_WUPOL1_MSK;
  }
  else
  {
  		PWR->WUP_POL |=  PWR_WUPOL1_MSK;  		
  }

}
/**
  * @brief  Config wakeup pin2 polarity.
  * @param  WakePolarity: specifies wakeup pin polarity.
  *   This parameter can be one of the following values:
  *     @arg RISING or FALLING
  * @retval None.
  */ 
void PWR_WakeUpPin2Polarity(WakePolarity eWakePolarity)
{
  /* Check the parameters */
  if(eWakePolarity == RISING)
  {
  		PWR->WUP_POL &=  ~PWR_WUPOL2_MSK;
  }
  else
  {
  		PWR->WUP_POL |=  PWR_WUPOL2_MSK;  		
  }
}
/**
  * @brief  Config wakeup pin3 polarity.
  * @param  WakePolarity: specifies wakeup pin polarity.
  *   This parameter can be one of the following values:
  *     @arg RISING or FALLING
  * @retval None.
  */ 
void PWR_WakeUpPin3Polarity(WakePolarity eWakePolarity)
{
  /* Check the parameters */
  if(eWakePolarity == RISING)
  {
  		PWR->WUP_POL &=  ~PWR_WUPOL3_MSK;
  }
  else
  {
  		PWR->WUP_POL |=  PWR_WUPOL3_MSK;  		
  }
}

/**
  * @brief  Enables or disables PDR when enter standby or shutdown mode.
  * @param  NewState: This parameter can be: ENABLE or DISABLE
  				ENABLE: power off PDR when enter standby or shutdown mode.
  				DISABLE: Don't power off PDR when enter standby or shutdown mode.
  * @retval None
  */

void PWR_PORPDR_CFG_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  if(NewState == ENABLE)
 	{
 			PWR->PORPDR_CFG = 0x0000;
 			PWR->PORPDR_CFG = 0x5500;
 			PWR->PORPDR_CFG = 0xAA00;
 			PWR->PORPDR_CFG = 0x5A00;
 			PWR->PORPDR_CFG = 0xA500;
 			PWR->PORPDR_CFG = 0xC800;
 			PWR->PORPDR_CFG = 0x8C00;
 			PWR->PORPDR_CFG = 0x6900;
 			PWR->PORPDR_CFG = 0x9601; 			
	}
	else
	{
			PWR->PORPDR_CFG = 0x0;
	}	  
}

/**
  * @brief  Enables or disables DAC1 output status when enter stop or standby mode
  * @param  NewState: This parameter can be: ENABLE or DISABLE
  					ENABLE: DAC1 output keep the last value.
  					DISABLE:DAC1 output will be high-Z, and no drive  
  * @retval None
  */
void PWR_DAC1_ALP_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) DAC1_ALP_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables DAC2 output status when enter stop or standby mode
  * @param  NewState: This parameter can be: ENABLE or DISABLE
  					ENABLE: DAC2 output keep the last value.
  					DISABLE:DAC2 output will be high-Z, and no drive  
  * @retval None
  */
void PWR_DAC2_ALP_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) DAC2_ALP_BB = (uint32_t)NewState;
}


