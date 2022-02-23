/**
  ******************************************************************************
  * @file    hk32f39a_bkp.c
  * @author  Thomas.W
  * @version V1.0  
  * @brief   API file of BKP module
  * @changelist  
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a_bkp.h"
#include "hk32f39a_rcc.h"

/* ------------ BKP registers bit address in the alias region --------------- */
#define BKP_OFFSET        (BKP_BASE - PERIPH_BASE)

/* --- CR Register ----*/

/* Alias word address of TPAL bit */
#define CR_OFFSET         (BKP_OFFSET + 0x30)
#define TPAL_BitNumber    0x01
#define CR_TPAL_BB        (PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPAL_BitNumber * 4))

/* Alias word address of TPE bit */
#define TPE_BitNumber     0x00
#define CR_TPE_BB         (PERIPH_BB_BASE + (CR_OFFSET * 32) + (TPE_BitNumber * 4))

/* --- CSR Register ---*/
/* Alias word address of TPIE bit */
#define CSR_OFFSET        (BKP_OFFSET + 0x34)
#define TPIE_BitNumber    0x02
#define CSR_TPIE_BB       (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TPIE_BitNumber * 4))

/* Alias word address of TIF bit */
#define TIF_BitNumber     0x09
#define CSR_TIF_BB        (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TIF_BitNumber * 4))

/* Alias word address of TEF bit */
#define TEF_BitNumber     0x08
#define CSR_TEF_BB        (PERIPH_BB_BASE + (CSR_OFFSET * 32) + (TEF_BitNumber * 4))


/* --- WUTCR Register ---*/
/* Alias word address of WUTE bit */
#define WUTCR_OFFSET      (BKP_OFFSET + 0x38)
#define WUTE_BitNumber    0x0F
#define WUTO0_BitNumber   0x08
#define WUTO1_BitNumber   0x09
#define WUTO2_BitNumber   0x0A
#define WUTO3_BitNumber   0x0B
#define WUTCR_WUTE_BB     (PERIPH_BB_BASE + (WUTCR_OFFSET * 32) + (WUTE_BitNumber * 4))
#define WUTCR_WUTO0_BB     (PERIPH_BB_BASE + (WUTCR_OFFSET * 32) + (WUTO0_BitNumber * 4))
#define WUTCR_WUTO1_BB     (PERIPH_BB_BASE + (WUTCR_OFFSET * 32) + (WUTO1_BitNumber * 4))
#define WUTCR_WUTO2_BB     (PERIPH_BB_BASE + (WUTCR_OFFSET * 32) + (WUTO2_BitNumber * 4))
#define WUTCR_WUTO3_BB     (PERIPH_BB_BASE + (WUTCR_OFFSET * 32) + (WUTO3_BitNumber * 4))

/* ---------------------- BKP registers bit mask ------------------------ */

/* RTCCR register bit mask */
#define RTCCR_CAL_MASK    ((uint16_t)0xFF80)
#define RTCCR_MASK        ((uint16_t)0xFC7F)

/* BKP_WUTCR register bit mask */
#define BKP_WUCKSEL_MASK  ((uint16_t)0xFFF8)


/**
  * @brief  Deinitializes the BKP peripheral registers to their default reset values.
  * @param  None
  * @retval None
  */
void BKP_DeInit(void)
{
  RCC_BackupResetCmd(ENABLE);
  RCC_BackupResetCmd(DISABLE);
}

/**
  * @brief  Configures the Tamper Pin active level.
  * @param  BKP_TamperPinLevel: specifies the Tamper Pin active level.
  *   This parameter can be one of the following values:
  *     @arg BKP_TamperPinLevel_High: Tamper pin active on high level
  *     @arg BKP_TamperPinLevel_Low: Tamper pin active on low level
  * @retval None
  */
void BKP_TamperPinLevelConfig(uint16_t BKP_TamperPinLevel)
{
  /* Check the parameters */
  assert_param(IS_BKP_TAMPER_PIN_LEVEL(BKP_TamperPinLevel));
  *(__IO uint32_t *) CR_TPAL_BB = BKP_TamperPinLevel;
}

/**
  * @brief  Enables or disables the Tamper Pin activation.
  * @param  NewState: new state of the Tamper Pin activation.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void BKP_TamperPinCmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CR_TPE_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables the Tamper Pin Interrupt.
  * @param  NewState: new state of the Tamper Pin Interrupt.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void BKP_ITConfig(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) CSR_TPIE_BB = (uint32_t)NewState;
}

/**
  * @brief  Enables or disables the wakeup timer.
  * @param  NewState: new state of the wakeup timer.
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void BKP_WakeupTimerConfig(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONAL_STATE(NewState));
  *(__IO uint32_t *) WUTCR_WUTE_BB = (uint32_t)NewState;
}


/**
  * @brief  Selct RTC wakeup timer Clock source.
  * @param  WakeupTimerClockSource:specifies the RTC wakeup timer Clock source.
  *   This parameter must be 
			 BKP_WUCKSEL_DIV16£ºSelect RTC clock div16 as wakeup timer clock
			 BKP_WUCKSEL_DIV8£ºSelect RTC clock div8 as wakeup timer clock
			 BKP_WUCKSEL_DIV4£ºSelect RTC clock div4 as wakeup timer clock
			 BKP_WUCKSEL_DIV2£ºSelect RTC clock div2 as wakeup timer clock
			 BKP_WUCKSEL_TRCLK£ºSelect RTC second pulse as wakeup timer clock 
			 BKP_WUCKSEL_TRCLKExtended£ºSelect RTC second pulse as wakeup timer clock, and extend 16bit wakeup timer to 17bit.
  * @retval None
  */
void BKP_SetRTCWakeupTimerClock(uint16_t WakeupTimerClockSource)
{
  uint16_t tmpreg = 0;
  /* Check the parameters */

  tmpreg = BKP->WUTCR;
  /* Clear WUCKSEL[0..2] bits */
  tmpreg &= BKP_WUCKSEL_MASK;
  
  /* Set WUCKSEL[0..2] bits according to WakeupTimerClockSource value */
  tmpreg |= WakeupTimerClockSource;
  /* Store the new value */
  BKP->WUTCR = tmpreg;		
}

/**
  * @brief  Enables or disables the wakeup timer output selection.
  * @param  WUTOPC13: wakeup timer output PC13 pin.
  					WUTOStandby:wakeup timer output in standby mode.
  					WUTOStop:wakeup timer output in stop mode.  					
  					WUTOTriggerADC:wakeup timer output to trigger ADC.  					
  *   This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void BKP_WakeupTimerOutSel(FunctionalState WUTOPC13,FunctionalState WUTOStandby,FunctionalState WUTOStop,FunctionalState WUTOTriggerADC)
{
  /* Check the parameters */
  *(__IO uint32_t *) WUTCR_WUTO0_BB = (uint32_t)WUTOPC13;
  *(__IO uint32_t *) WUTCR_WUTO1_BB = (uint32_t)WUTOStandby;
  *(__IO uint32_t *) WUTCR_WUTO2_BB = (uint32_t)WUTOStop;
  *(__IO uint32_t *) WUTCR_WUTO3_BB = (uint32_t)WUTOTriggerADC;
      
}


/**
  * @brief  Select the RTC output source to output on the Tamper pin.
  * @param  BKP_RTCOutputSource: specifies the RTC output source.
  *   This parameter can be one of the following values:
  *     @arg BKP_RTCOutputSource_None: no RTC output on the Tamper pin.
  *     @arg BKP_RTCOutputSource_CalibClock: output the RTC clock with frequency
  *                                          divided by 64 on the Tamper pin.
  *     @arg BKP_RTCOutputSource_Alarm: output the RTC Alarm pulse signal on
  *                                     the Tamper pin.
  *     @arg BKP_RTCOutputSource_Second: output the RTC Second pulse signal on
  *                                      the Tamper pin.  
  * @retval None
  */
void BKP_RTCOutputConfig(uint16_t BKP_RTCOutputSource)
{
  uint16_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_BKP_RTC_OUTPUT_SOURCE(BKP_RTCOutputSource));
  tmpreg = BKP->RTCCR;
  /* Clear CCO, ASOE and ASOS bits */
  tmpreg &= RTCCR_MASK;
  
  /* Set CCO, ASOE and ASOS bits according to BKP_RTCOutputSource value */
  tmpreg |= BKP_RTCOutputSource;
  /* Store the new value */
  BKP->RTCCR = tmpreg;
}

/**
  * @brief  Sets RTC Clock Calibration value.
  * @param  CalibrationValue: specifies the RTC Clock Calibration value.
  *   This parameter must be a number between 0 and 0x7F.
  * @retval None
  */
void BKP_SetRTCCalibrationValue(uint8_t CalibrationValue)
{
  uint16_t tmpreg = 0;
  /* Check the parameters */
  assert_param(IS_BKP_CALIBRATION_VALUE(CalibrationValue));
  tmpreg = BKP->RTCCR;
  /* Clear CAL[6:0] bits */
  tmpreg &= RTCCR_CAL_MASK;
  /* Set CAL[6:0] bits according to CalibrationValue value */
  tmpreg |= CalibrationValue;
  /* Store the new value */
  BKP->RTCCR = tmpreg;
}

/**
  * @brief  Writes user data to the specified Data Backup Register.
  * @param  BKP_DR: specifies the Data Backup Register.
  *   This parameter can be BKP_DRx where x:[0, 42]
  * @param  Data: data to write
  * @retval None
  */
void BKP_WriteBackupRegister(uint16_t BKP_DR, uint16_t Data)
{
  __IO uint32_t tmp = 0;

  /* Check the parameters */
  assert_param(IS_BKP_DR(BKP_DR));

  tmp = (uint32_t)BKP_BASE; 
  tmp += BKP_DR;

  *(__IO uint32_t *) tmp = Data;
}

/**
  * @brief  Reads data from the specified Data Backup Register.
  * @param  BKP_DR: specifies the Data Backup Register.
  *   This parameter can be BKP_DRx where x:[0, 42]
  * @retval The content of the specified Data Backup Register
  */
uint16_t BKP_ReadBackupRegister(uint16_t BKP_DR)
{
  __IO uint32_t tmp = 0;

  /* Check the parameters */
  assert_param(IS_BKP_DR(BKP_DR));

  tmp = (uint32_t)BKP_BASE; 
  tmp += BKP_DR;

  return (*(__IO uint16_t *) tmp);
}

/**
  * @brief  Checks whether the Tamper Pin Event flag is set or not.
  * @param  None
  * @retval The new state of the Tamper Pin Event flag (SET or RESET).
  */
FlagStatus BKP_GetFlagStatus(void)
{
  return (FlagStatus)(*(__IO uint32_t *) CSR_TEF_BB);
}

/**
  * @brief  Clears Tamper Pin Event pending flag.
  * @param  None
  * @retval None
  */
void BKP_ClearFlag(void)
{
  /* Set CTE bit to clear Tamper Pin Event flag */
  BKP->CSR |= BKP_CSR_CTE;
}

/**
  * @brief  Checks whether the Tamper Pin Interrupt has occurred or not.
  * @param  None
  * @retval The new state of the Tamper Pin Interrupt (SET or RESET).
  */
ITStatus BKP_GetITStatus(void)
{
  return (ITStatus)(*(__IO uint32_t *) CSR_TIF_BB);
}

/**
  * @brief  Clears Tamper Pin Interrupt pending bit.
  * @param  None
  * @retval None
  */
void BKP_ClearITPendingBit(void)
{
  /* Set CTI bit to clear Tamper Pin Interrupt pending bit */
  BKP->CSR |= BKP_CSR_CTI;
}
