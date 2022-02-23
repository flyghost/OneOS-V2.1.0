/**
  ******************************************************************************
  * @file    hk32f39a_pwr.h
  * @author  Thomas.W
  * @version V1.0  
  * @brief   Header file of PWR module
  * @changelist  
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_PWR_H
#define __HK32F39A_PWR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"

#define PWR_PVDLevel_2V2          ((uint32_t)0x00000000)
#define PWR_PVDLevel_2V3          ((uint32_t)0x00000020)
#define PWR_PVDLevel_2V4          ((uint32_t)0x00000040)
#define PWR_PVDLevel_2V5          ((uint32_t)0x00000060)
#define PWR_PVDLevel_2V6          ((uint32_t)0x00000080)
#define PWR_PVDLevel_2V7          ((uint32_t)0x000000A0)
#define PWR_PVDLevel_2V8          ((uint32_t)0x000000C0)
#define PWR_PVDLevel_2V9          ((uint32_t)0x000000E0)
#define IS_PWR_PVD_LEVEL(LEVEL) (((LEVEL) == PWR_PVDLevel_2V2) || ((LEVEL) == PWR_PVDLevel_2V3)|| \
                                 ((LEVEL) == PWR_PVDLevel_2V4) || ((LEVEL) == PWR_PVDLevel_2V5)|| \
                                 ((LEVEL) == PWR_PVDLevel_2V6) || ((LEVEL) == PWR_PVDLevel_2V7)|| \
                                 ((LEVEL) == PWR_PVDLevel_2V8) || ((LEVEL) == PWR_PVDLevel_2V9))

/** @defgroup Regulator_state_is_STOP_mode 
  * @{
  */

#define PWR_Regulator_ON          ((uint32_t)0x00000000)
#define PWR_Regulator_LowPower    ((uint32_t)0x00000001)
#define IS_PWR_REGULATOR(REGULATOR) (((REGULATOR) == PWR_Regulator_ON) || \
                                     ((REGULATOR) == PWR_Regulator_LowPower))

/** @defgroup STOP_mode_entry 
  * @{
  */

#define PWR_STOPEntry_WFI         ((uint8_t)0x01)
#define PWR_STOPEntry_WFE         ((uint8_t)0x02)
#define IS_PWR_STOP_ENTRY(ENTRY) (((ENTRY) == PWR_STOPEntry_WFI) || ((ENTRY) == PWR_STOPEntry_WFE))
 
/** @defgroup PWR_Flag 
  * @{
  */

#define PWR_FLAG_WU               ((uint32_t)0x00000001)
#define PWR_FLAG_SB               ((uint32_t)0x00000002)
#define PWR_FLAG_PVDO             ((uint32_t)0x00000004)
#define PWR_FLAG_LDORDY           ((uint32_t)0x00000008)
#define IS_PWR_GET_FLAG(FLAG) (((FLAG) == PWR_FLAG_WU) || ((FLAG) == PWR_FLAG_SB) || \
                               ((FLAG) == PWR_FLAG_PVDO)|| ((FLAG) == PWR_FLAG_LDORDY))

#define IS_PWR_CLEAR_FLAG(FLAG) (((FLAG) == PWR_FLAG_WU) || ((FLAG) == PWR_FLAG_SB))

#define PWR_FLAG_SWUF                ((uint32_t)0x00000001)
#define PWR_FLAG_SHUTF               ((uint32_t)0x00000002)
#define PWR_FLAG_CSWUF               ((uint32_t)0x00000004)
#define PWR_FLAG_CSHUTF              ((uint32_t)0x00000008)

#define PWR_WUPOL1_MSK                ((uint32_t)0x00000001)
#define PWR_WUPOL2_MSK                ((uint32_t)0x00000002)
#define PWR_WUPOL3_MSK                ((uint32_t)0x00000004)
typedef enum {RISING = 0, FALLING = !RISING} WakePolarity;

#define PWR_CSR2_CSWUF     (0x1<<2) 
#define PWR_CSR2_CSHUTF    (0x1<<3) 
#define PWR_CSR2_BKPPDS    (0x1<<6) 
#define PWR_CSR2_SHDS      (0x1<<7) 


void PWR_DeInit(void);
void PWR_BackupAccessCmd(FunctionalState NewState);
void PWR_PVDCmd(FunctionalState NewState);
void PWR_PVDLevelConfig(uint32_t PWR_PVDLevel);
void PWR_WakeUpPin1Cmd(FunctionalState NewState);
void PWR_WakeUpPin2Cmd(FunctionalState NewState);
void PWR_WakeUpPin3Cmd(FunctionalState NewState);
void PWR_EnterSTOPMode(uint32_t PWR_Regulator, uint8_t PWR_STOPEntry);
void PWR_EnterSTANDBYMode(void);
FlagStatus PWR_GetFlagStatus(uint32_t PWR_FLAG);
void PWR_ClearFlag(uint32_t PWR_FLAG);

void PWR_EWUP_RTCCmd(FunctionalState NewState);
void PWR_SHDS_Cmd(FunctionalState NewState);
void PWR_BKPPDS_Cmd(FunctionalState NewState);
void PWR_CSR2_ClearFlag(uint32_t PWR_FLAG);
FlagStatus PWR_CSR2_GetFlagStatus(uint32_t PWR_FLAG);
void PWR_WakeUpPin1Polarity(WakePolarity eWakePolarity);
void PWR_WakeUpPin2Polarity(WakePolarity eWakePolarity);
void PWR_WakeUpPin3Polarity(WakePolarity eWakePolarity);
void PWR_PORPDR_CFG_Cmd(FunctionalState NewState);
void PWR_DAC1_ALP_Cmd(FunctionalState NewState);
void PWR_DAC2_ALP_Cmd(FunctionalState NewState);
void PWR_EnterShutDownMode(FunctionalState RTC_PD_IN_SHUTDOWN);

#ifdef __cplusplus
}
#endif

#endif

