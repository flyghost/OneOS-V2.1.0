/**
  ******************************************************************************
  * @file    usbd_cdc.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_cdc.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_PRINTER_H
#define __USB_PRINTER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "proto_usbd_ioreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_cdc.c
  * @{
  */


/** @defgroup usbd_cdc_Exported_Defines
  * @{
  */
#define PRINTER_IN_EP                                   0x81U  /* EP1 for data IN */
#define PRINTER_OUT_EP                                  0x01U  /* EP1 for data OUT */
#define PRINTER_CMD_EP                                  0x82U  /* EP2 for PRINTER commands */

#ifndef PRINTER_HS_BINTERVAL
#define PRINTER_HS_BINTERVAL                            0x10U
#endif /* PRINTER_HS_BINTERVAL */

#ifndef PRINTER_FS_BINTERVAL
#define PRINTER_FS_BINTERVAL                            0x10U
#endif /* PRINTER_FS_BINTERVAL */

/* PRINTER Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define PRINTER_DATA_HS_MAX_PACKET_SIZE                 512U  /* Endpoint IN & OUT Packet size */
#define PRINTER_DATA_FS_MAX_PACKET_SIZE                 64U  /* Endpoint IN & OUT Packet size */
#define PRINTER_CMD_PACKET_SIZE                         8U  /* Control Endpoint Packet size */

#define USB_PRINTER_CONFIG_DESC_SIZ                     67U
#define PRINTER_DATA_HS_IN_PACKET_SIZE                  PRINTER_DATA_HS_MAX_PACKET_SIZE
#define PRINTER_DATA_HS_OUT_PACKET_SIZE                 PRINTER_DATA_HS_MAX_PACKET_SIZE

#define PRINTER_DATA_FS_IN_PACKET_SIZE                  PRINTER_DATA_FS_MAX_PACKET_SIZE
#define PRINTER_DATA_FS_OUT_PACKET_SIZE                 PRINTER_DATA_FS_MAX_PACKET_SIZE

#define PRINTER_REQ_MAX_DATA_SIZE                       0x7U
/*---------------------------------------------------------------------*/
/*  PRINTER definitions                                                    */
/*---------------------------------------------------------------------*/
#define PRINTER_SEND_ENCAPSULATED_COMMAND               0x00U
#define PRINTER_GET_ENCAPSULATED_RESPONSE               0x01U
#define PRINTER_SET_COMM_FEATURE                        0x02U
#define PRINTER_GET_COMM_FEATURE                        0x03U
#define PRINTER_CLEAR_COMM_FEATURE                      0x04U
#define PRINTER_SET_LINE_CODING                         0x20U
#define PRINTER_GET_LINE_CODING                         0x21U
#define PRINTER_SET_CONTROL_LINE_STATE                  0x22U
#define PRINTER_SEND_BREAK                              0x23U

/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */
typedef struct
{
  uint32_t bitrate;
  uint8_t  format;
  uint8_t  paritytype;
  uint8_t  datatype;
} USBD_Printer_LineCodingTypeDef;

typedef struct _USBD_Printer_Itf
{
  int8_t (* Init)(USBD_HandleTypeDef *pdev);
  int8_t (* DeInit)(USBD_HandleTypeDef *pdev);
  int8_t (* Control)(USBD_HandleTypeDef *pdev, uint8_t cmd, uint8_t *pbuf, uint16_t length);
  int8_t (* Receive)(USBD_HandleTypeDef *pdev, uint8_t *Buf, uint32_t Len);
  int8_t (* ReceiveCplt)(USBD_HandleTypeDef *pdev, uint8_t *Buf, uint32_t *Len, uint8_t epnum);  
  uint8_t (* Transmit)(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint16_t Len);
  int8_t (* TransmitCplt)(USBD_HandleTypeDef *pdev, uint8_t *Buf, uint32_t *Len, uint8_t epnum);
} USBD_Printer_ItfTypeDef;


typedef struct
{
  uint32_t data[PRINTER_DATA_HS_MAX_PACKET_SIZE / 4U];      /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint8_t  CmdLength;
  uint8_t  *RxBuffer;
  uint8_t  *TxBuffer;
  uint32_t RxLength;
  uint32_t TxLength;

  __IO uint32_t TxState;
  __IO uint32_t RxState;
} USBD_Printer_HandleTypeDef;

#pragma pack(1)

/* USB standard device request structure */
typedef struct _usb_req 
{
    uint8_t           bmRequestType;  /*!< type of request */
    uint8_t           bRequest;       /*!< request of setup packet */
    uint16_t          wValue;         /*!< value of setup packet */
    uint16_t          wIndex;         /*!< index of setup packet */
    uint16_t          wLength;        /*!< length of setup packet */
} usb_req;


/* USB setup packet define */
typedef union _usb_setup 
{
    uint8_t data[8];

    usb_req req;
} usb_setup;

/* USB descriptor defines */

typedef struct _usb_desc_header 
{
    uint8_t bLength;                      /*!< size of the descriptor */
    uint8_t bDescriptorType;              /*!< type of the descriptor */
} usb_desc_header;

typedef struct _usb_desc_dev 
{
    usb_desc_header header;               /*!< descriptor header, including type and size */

    uint16_t bcdUSB;                      /*!< BCD of the supported USB specification */
    uint8_t  bDeviceClass;                /*!< USB device class */
    uint8_t  bDeviceSubClass;             /*!< USB device subclass */
    uint8_t  bDeviceProtocol;             /*!< USB device protocol */
    uint8_t  bMaxPacketSize0;             /*!< size of the control (address 0) endpoint's bank in bytes */
    uint16_t idVendor;                    /*!< vendor ID for the USB product */
    uint16_t idProduct;                   /*!< unique product ID for the USB product */
    uint16_t bcdDevice;                   /*!< product release (version) number */
    uint8_t  iManufacturer;               /*!< string index for the manufacturer's name */
    uint8_t  iProduct;                    /*!< string index for the product name/details */
    uint8_t  iSerialNumber;               /*!< string index for the product's globally unique hexadecimal serial number */
    uint8_t  bNumberConfigurations;       /*!< total number of configurations supported by the device */
} usb_desc_dev;

typedef struct _usb_desc_config 
{
    usb_desc_header header;               /*!< descriptor header, including type and size */

    uint16_t wTotalLength;                /*!< size of the configuration descriptor header,and all sub descriptors inside the configuration */
    uint8_t  bNumInterfaces;              /*!< total number of interfaces in the configuration */
    uint8_t  bConfigurationValue;         /*!< configuration index of the current configuration */
    uint8_t  iConfiguration;              /*!< index of a string descriptor describing the configuration */
    uint8_t  bmAttributes;                /*!< configuration attributes */
    uint8_t  bMaxPower;                   /*!< maximum power consumption of the device while in the current configuration */
} usb_desc_config;

typedef struct _usb_desc_itf 
{
    usb_desc_header header;               /*!< descriptor header, including type and size */

    uint8_t bInterfaceNumber;             /*!< index of the interface in the current configuration */
    uint8_t bAlternateSetting;            /*!< alternate setting for the interface number */
    uint8_t bNumEndpoints;                /*!< total number of endpoints in the interface */
    uint8_t bInterfaceClass;              /*!< interface class ID */
    uint8_t bInterfaceSubClass;           /*!< interface subclass ID */
    uint8_t bInterfaceProtocol;           /*!< interface protocol ID */
    uint8_t iInterface;                   /*!< index of the string descriptor describing the interface */
} usb_desc_itf;

typedef struct _usb_desc_ep 
{
    usb_desc_header header;               /*!< descriptor header, including type and size. */

    uint8_t  bEndpointAddress;            /*!< logical address of the endpoint */
    uint8_t  bmAttributes;                /*!< endpoint attributes */
    uint16_t wMaxPacketSize;              /*!< size of the endpoint bank, in bytes */
    uint8_t  bInterval;                   /*!< polling interval in milliseconds for the endpoint if it is an INTERRUPT or ISOCHRONOUS type */
} usb_desc_ep;

typedef struct _usb_desc_LANGID 
{
    usb_desc_header header;               /*!< descriptor header, including type and size. */
    uint16_t wLANGID;                     /*!< LANGID code */
} usb_desc_LANGID;

typedef struct _usb_desc_str 
{
    usb_desc_header header;               /*!< descriptor header, including type and size. */
    uint16_t unicode_string[64];          /*!< unicode string data */
} usb_desc_str;

/* USB configuration descriptor structure */
typedef struct
{
    usb_desc_config         config;
    usb_desc_itf            printer_itf;
    usb_desc_ep             printer_epin;
    usb_desc_ep             printer_epout;
} usb_printer_desc_config_set;

#pragma pack()

/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef USBD_Printer;
#define USBD_PRINTER_CLASS &USBD_Printer
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t USBD_Printer_RegisterInterface(USBD_HandleTypeDef *pdev,
                                   USBD_Printer_ItfTypeDef *fops);

uint8_t USBD_Printer_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff,
                             uint32_t length);

uint8_t USBD_Printer_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
uint8_t USBD_Printer_ReceivePacket(USBD_HandleTypeDef *pdev);
uint8_t USBD_Printer_TransmitPacket(USBD_HandleTypeDef *pdev);
int usbd_printer_register(USBD_HandleTypeDef *pdev);
int usbd_printer_unregister(USBD_HandleTypeDef *pdev);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_PRINTER_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
