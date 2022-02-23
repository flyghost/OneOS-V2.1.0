/**
  ******************************************************************************
  * @file    usb_mem.c
  * @author  veiko.c
  * @version V1.0.0
  * @date    2019-10-14
  * @brief   Utility functions for memory transfers to/from PMA
  ****************************************************************************** 
  */

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Extern variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * Function Name  : UserToPMABufferCopy
  * Description    : Copy a buffer from user memory area to packet memory area (PMA)
  * Input          : - pbUsrBuf: pointer to user memory area.
  *                  - wPMABufAddr: address into PMA.
  *                  - wNBytes: no. of bytes to be copied.
  * Output         : None.
  * Return         : None	.
  **/
void UserToPMABufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = (wNBytes + 1) >> 1;   /* n = (wNBytes + 1) / 2 */
  uint32_t i, temp1, temp2;
  uint16_t *pdwVal;
  pdwVal = (uint16_t *)(wPMABufAddr * 2 + PMAAddr);
  for (i = n; i != 0; i--)
  {
    temp1 = (uint16_t) * pbUsrBuf;
    pbUsrBuf++;
    temp2 = temp1 | (uint16_t) * pbUsrBuf << 8;
    *pdwVal++ = temp2;
    pdwVal++;
    pbUsrBuf++;
  }
}

/**
  * Function Name  : PMAToUserBufferCopy
  * Description    : Copy a buffer from user memory area to packet memory area (PMA)
  * Input          : - pbUsrBuf    = pointer to user memory area.
  *                  - wPMABufAddr = address into PMA.
  *                  - wNBytes     = no. of bytes to be copied.
  * Output         : None.
  * Return         : None.
  **/
void PMAToUserBufferCopy(uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  uint32_t n = (wNBytes + 1) >> 1;/* /2*/
  uint32_t i;
  uint32_t *pdwVal;
  pdwVal = (uint32_t *)(wPMABufAddr * 2 + PMAAddr);
  for (i = n; i != 0; i--)
  {
    *(uint16_t*)pbUsrBuf++ = *pdwVal++;
    pbUsrBuf++;
  }
}

/******************* (C) COPYRIGHT   HKMicroChip *****END OF FILE****/
