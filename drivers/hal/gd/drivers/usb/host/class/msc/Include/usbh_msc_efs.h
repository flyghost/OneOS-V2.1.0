/**
  ******************************************************************************
  * @file    usbh_msc_efs.h
  * @author  MCU SD
  * @version V1.0.0
  * @date    26-Dec-2014
  * @brief   Header file for usbh_msc_fs_interface.c
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GD32F10X_USBH_MSC_EFS_H
#define __GD32F10X_USBH_MSC_EFS_H

/** @addtogroup GD32F10x_Firmware
  * @{
  */

/** @addtogroup USB_OTG_FS
  * @{
  */

/** @addtogroup USB_OTG_FS_Host_Class_Library
  * @{
  */
  
/** @defgroup USBH_MSC_EFS
  * @{
  */

/** @defgroup USBH_MSC_EFS_Exported_Types
  * @{
  */ 
struct hwInterface
{
    /*FILE *imageFile;*/
    int32_t sectorCount;
};

typedef struct hwInterface hwInterface;

/**
  * @}
  */

/** @defgroup USBH_MSC_EFS_Exported_Defines
  * @{
  */
#define EFS_ERROR       -1
#define EFS_PASS        0

/**
  * @}
  */

/** @defgroup USBH_MSC_EFS_Exported_FunctionsPrototype
  * @{
  */
int8_t if_initInterface(hwInterface* file,char* opts);
int8_t if_readBuf(hwInterface* file,uint32_t address,uint8_t* buf);
int8_t if_writeBuf(hwInterface* file,uint32_t address,uint8_t* buf);

/**
  * @}
  */

#endif  /*__GD32F10X_USBH_MSC_EFS_H*/


/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

/******************* (C) COPYRIGHT 2014 GIGADEVICE *****END OF FILE****/
