/**
  ******************************************************************************
  * @file    hk32f39a_crc.h
  * @author  Jane.L
  * @version V1.0.0
  * @brief   API files of CRC module.
  * @changelist
  ****************************************************************************** 
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HK32F39A_CRC_H
#define __HK32F39A_CRC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hk32f39a.h"

/** CRC_Exported_Functions  */

void CRC_ResetDR(void);
uint32_t CRC_CalcCRC(uint32_t Data);
uint32_t CRC_CalcBlockCRC(uint32_t pBuffer[], uint32_t BufferLength);
uint32_t CRC_GetCRC(void);
void CRC_SetIDRegister(uint8_t IDValue);
uint8_t CRC_GetIDRegister(void);

#ifdef __cplusplus
}
#endif

#endif /* __HK32F39A_CRC_H */


