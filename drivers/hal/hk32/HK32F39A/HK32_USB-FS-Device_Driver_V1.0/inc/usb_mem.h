/**
  ******************************************************************************
  * @file    usb_mem.h
  * @author  veiko.c
  * @version V1.0.0
  * @date    2019-10-14
  * @brief   Utility prototypes functions for memory/PMA transfers
  ****************************************************************************** 
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_MEM_H
#define __USB_MEM_H

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void UserToPMABufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);
void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes);
/* External variables --------------------------------------------------------*/

#endif  /*__USB_MEM_H*/

/******************* (C) COPYRIGHT   HKMicroChip *****END OF FILE****/
