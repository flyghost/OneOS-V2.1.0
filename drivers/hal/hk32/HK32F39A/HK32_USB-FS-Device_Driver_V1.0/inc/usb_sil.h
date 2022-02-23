/**
  ******************************************************************************
  * @file    usb_sil.h
  * @author  veiko.c
  * @version V1.0.0
  * @date    2019-10-14
  * @brief   Simplified Interface Layer function prototypes.
  ****************************************************************************** 
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_SIL_H
#define __USB_SIL_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

uint32_t USB_SIL_Init(void);
uint32_t USB_SIL_Write(uint8_t bEpAddr, uint8_t* pBufferPointer, uint32_t wBufferSize);
uint32_t USB_SIL_Read(uint8_t bEpAddr, uint8_t* pBufferPointer);

/* External variables --------------------------------------------------------*/

#endif /* __USB_SIL_H */

/******************* (C) COPYRIGHT   HKMicroChip *****END OF FILE****/
