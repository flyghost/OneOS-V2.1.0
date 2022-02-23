/**
  ******************************************************************************
  * @file    usbd_printer.c
  * @author  MCD Application Team
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB Printer Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as Printer Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                Printer Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as Printer device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  *
  *           These aspects may be enriched or modified for a specific user application.
  *
  *            This driver doesn't implement the following aspects of the specification
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *
  *  @endverbatim
  *
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

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbd_printer.h"
#include "proto_usbd_ctlreq.h"

//#include "usbd_printer_dev.h"
#include "driver.h"
#include "device.h"
#include <console.h>

static uint8_t USBD_Printer_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_Printer_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_Printer_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_Printer_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_Printer_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_Printer_EP0_RxReady(USBD_HandleTypeDef *pdev);

static uint8_t *USBD_Printer_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_Printer_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_Printer_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_Printer_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t *USBD_Printer_GetDeviceQualifierDescriptor(uint16_t *length);


static int8_t Printer_Interface_Init(USBD_HandleTypeDef *pdev);
static int8_t Printer_Interface_DeInit(USBD_HandleTypeDef *pdev);
static int8_t Printer_Interface_Control(USBD_HandleTypeDef *pdev, uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t Printer_Interface_Receive(USBD_HandleTypeDef *pdev, uint8_t* pbuf, uint32_t Len);
static int8_t Printer_Interface_ReceiveCplt(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint32_t *Len, uint8_t epnum);
uint8_t Printer_Interface_Transmit(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint16_t Len);
static int8_t Printer_Interface_TransmitCplt(USBD_HandleTypeDef *pdev, uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_Printer_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_Printer_Private_Variables
  * @{
  */


/* Printer interface class callbacks structure */
USBD_ClassTypeDef  USBD_Printer =
{
  USBD_Printer_Init,
  USBD_Printer_DeInit,
  USBD_Printer_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_Printer_EP0_RxReady,
  USBD_Printer_DataIn,
  USBD_Printer_DataOut,
  NULL,
  NULL,
  NULL,
  USBD_Printer_GetHSCfgDesc,
  USBD_Printer_GetFSCfgDesc,
  USBD_Printer_GetOtherSpeedCfgDesc,
  USBD_Printer_GetDeviceQualifierDescriptor,
};

#define USB_PRINTER_CONFIG_DESC_LEN     32U
/* USB printing device class code */
#define USB_CLASS_PRINTER               0x07U

/* printing device subclass code */
#define USB_SUBCLASS_PRINTER            0x01U
/* printing device protocol code */
#define PROTOCOL_UNIDIRECTIONAL_ITF     0x01U
#define PROTOCOL_BI_DIRECTIONAL_ITF     0x02U
#define PROTOCOL_1284_4_ITF             0x03U
#define PROTOCOL_VENDOR                 0xFFU
enum _usbx_type {
    USB_EP_ATTR_CTL  = 0x0U,                     /*!< USB control transfer type */
    USB_EP_ATTR_ISO  = 0x1U,                     /*!< USB Isochronous transfer type */
    USB_EP_ATTR_BULK = 0x2U,                     /*!< USB Bulk transfer type */
    USB_EP_ATTR_INT  = 0x3U                      /*!< USB Interrupt transfer type */
};
#define USE_USB_HS
#define USE_ULPI_PHY
#ifdef USE_USB_HS
    #ifdef USE_ULPI_PHY
        #define PRINTER_DATA_PACKET_SIZE           512
    #else
        #define PRINTER_DATA_PACKET_SIZE           64
    #endif

    #define PRINTER_IN_PACKET                      PRINTER_DATA_PACKET_SIZE
    #define PRINTER_OUT_PACKET                     PRINTER_DATA_PACKET_SIZE
#else
    #define PRINTER_DATA_PACKET_SIZE               64
    #define PRINTER_IN_PACKET                      PRINTER_DATA_PACKET_SIZE
    #define PRINTER_OUT_PACKET                     PRINTER_DATA_PACKET_SIZE
#endif /* USE_USB_HS */
__ALIGN_BEGIN const usb_printer_desc_config_set printer_config_desc __ALIGN_END = 
{
    .config = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_config), 
             .bDescriptorType = USB_DESC_TYPE_CONFIGURATION
         },
        .wTotalLength         = USB_PRINTER_CONFIG_DESC_LEN,
        .bNumInterfaces       = 0x01U,
        .bConfigurationValue  = 0x01U,
        .iConfiguration       = 0x00U,
        .bmAttributes         = 0xA0U,
        .bMaxPower            = 0x32U
    },

    .printer_itf = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_itf), 
             .bDescriptorType =  USB_DESC_TYPE_INTERFACE 
         },
        .bInterfaceNumber     = 0x00U,
        .bAlternateSetting    = 0x00U,
        .bNumEndpoints        = 0x02U,
        .bInterfaceClass      = USB_CLASS_PRINTER,
        .bInterfaceSubClass   = USB_SUBCLASS_PRINTER,
        .bInterfaceProtocol   = PROTOCOL_BI_DIRECTIONAL_ITF,
        .iInterface           = 0x00U
    },

    .printer_epin = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_ep), 
             .bDescriptorType = USB_DESC_TYPE_ENDPOINT 
         },
        .bEndpointAddress     = PRINTER_IN_EP,
        .bmAttributes         = USB_EP_ATTR_BULK,
        .wMaxPacketSize       = PRINTER_IN_PACKET,
        .bInterval            = 0x00U
    },

    .printer_epout = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_ep), 
             .bDescriptorType = USB_DESC_TYPE_ENDPOINT 
         },
        .bEndpointAddress     = PRINTER_OUT_EP,
        .bmAttributes         = USB_EP_ATTR_BULK,
        .wMaxPacketSize       = PRINTER_OUT_PACKET,
        .bInterval            = 0x00U
    },
};

/* USB Printer device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_Printer_CfgHSDesc[USB_PRINTER_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_PRINTER_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  0x00,
  0x01,                                       /* bNumInterfaces: 2 interface */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                       /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                       /* bmAttributes: Bus Powered according to user configuration */
#endif
  USBD_MAX_POWER,                             /* MaxPower 100 mA */

  /*---------------------------------------------------------------------------*/

  /* Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
  0x00,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x01,                                       /* bNumEndpoints: One endpoints used */
  0x02,                                       /* bInterfaceClass: Communication Interface Class */
  0x02,                                       /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                       /* bInterfaceProtocol: Common AT commands */
  0x00,                                       /* iInterface: */

  /* Header Functional Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x00,                                       /* bDescriptorSubtype: Header Func Desc */
  0x10,                                       /* bcdPrinter: spec release number */
  0x01,

  /* Call Management Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x01,                                       /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                       /* bmCapabilities: D0+D1 */
  0x01,                                       /* bDataInterface: 1 */

  /* ACM Functional Descriptor */
  0x04,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                       /* bmCapabilities */

  /* Union Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x06,                                       /* bDescriptorSubtype: Union func desc */
  0x00,                                       /* bMasterInterface: Communication class interface */
  0x01,                                       /* bSlaveInterface0: Data Class Interface */

  /* Endpoint 2 Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_CMD_EP,                                 /* bEndpointAddress */
  0x03,                                       /* bmAttributes: Interrupt */
  LOBYTE(PRINTER_CMD_PACKET_SIZE),                /* wMaxPacketSize: */
  HIBYTE(PRINTER_CMD_PACKET_SIZE),
  PRINTER_HS_BINTERVAL,                           /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /* Data class interface descriptor */
  0x09,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
  0x01,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x02,                                       /* bNumEndpoints: Two endpoints used */
  0x0A,                                       /* bInterfaceClass: Printer */
  0x00,                                       /* bInterfaceSubClass: */
  0x00,                                       /* bInterfaceProtocol: */
  0x00,                                       /* iInterface: */

  /* Endpoint OUT Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(PRINTER_DATA_HS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(PRINTER_DATA_HS_MAX_PACKET_SIZE),
  0x00,                                       /* bInterval: ignore for Bulk transfer */

  /* Endpoint IN Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(PRINTER_DATA_HS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(PRINTER_DATA_HS_MAX_PACKET_SIZE),
  0x00                                        /* bInterval: ignore for Bulk transfer */
};


/* USB Printer device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_Printer_CfgFSDesc[USB_PRINTER_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                /* bDescriptorType: Configuration */
  USB_PRINTER_CONFIG_DESC_SIZ,                    /* wTotalLength:no of returned bytes */
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interface */
  0x01,                                       /* bConfigurationValue: Configuration value */
  0x00,                                       /* iConfiguration: Index of string descriptor describing the configuration */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                       /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                       /* bmAttributes: Bus Powered according to user configuration */
#endif
  USBD_MAX_POWER,                             /* MaxPower 100 mA */

  /*---------------------------------------------------------------------------*/

  /* Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x01,                                       /* bNumEndpoints: One endpoints used */
  0x02,                                       /* bInterfaceClass: Communication Interface Class */
  0x02,                                       /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                       /* bInterfaceProtocol: Common AT commands */
  0x00,                                       /* iInterface: */

  /* Header Functional Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x00,                                       /* bDescriptorSubtype: Header Func Desc */
  0x10,                                       /* bcdPrinter: spec release number */
  0x01,

  /* Call Management Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x01,                                       /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                       /* bmCapabilities: D0+D1 */
  0x01,                                       /* bDataInterface: 1 */

  /* ACM Functional Descriptor */
  0x04,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                       /* bmCapabilities */

  /* Union Functional Descriptor */
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x06,                                       /* bDescriptorSubtype: Union func desc */
  0x00,                                       /* bMasterInterface: Communication class interface */
  0x01,                                       /* bSlaveInterface0: Data Class Interface */

  /* Endpoint 2 Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_CMD_EP,                                 /* bEndpointAddress */
  0x03,                                       /* bmAttributes: Interrupt */
  LOBYTE(PRINTER_CMD_PACKET_SIZE),                /* wMaxPacketSize: */
  HIBYTE(PRINTER_CMD_PACKET_SIZE),
  PRINTER_FS_BINTERVAL,                           /* bInterval: */
  /*---------------------------------------------------------------------------*/

  /* Data class interface descriptor */
  0x09,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
  0x01,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x02,                                       /* bNumEndpoints: Two endpoints used */
  0x0A,                                       /* bInterfaceClass: Printer */
  0x00,                                       /* bInterfaceSubClass: */
  0x00,                                       /* bInterfaceProtocol: */
  0x00,                                       /* iInterface: */

  /* Endpoint OUT Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(PRINTER_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(PRINTER_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                       /* bInterval: ignore for Bulk transfer */

  /* Endpoint IN Descriptor */
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  LOBYTE(PRINTER_DATA_FS_MAX_PACKET_SIZE),        /* wMaxPacketSize: */
  HIBYTE(PRINTER_DATA_FS_MAX_PACKET_SIZE),
  0x00                                        /* bInterval: ignore for Bulk transfer */
};

__ALIGN_BEGIN static uint8_t USBD_Printer_OtherSpeedCfgDesc[USB_PRINTER_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09,                                       /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION,
  USB_PRINTER_CONFIG_DESC_SIZ,
  0x00,
  0x02,                                       /* bNumInterfaces: 2 interfaces */
  0x01,                                       /* bConfigurationValue: */
  0x04,                                       /* iConfiguration: */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                       /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                       /* bmAttributes: Bus Powered according to user configuration */
#endif
  USBD_MAX_POWER,                             /* MaxPower 100 mA */

  /*Interface Descriptor */
  0x09,                                       /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x01,                                       /* bNumEndpoints: One endpoints used */
  0x02,                                       /* bInterfaceClass: Communication Interface Class */
  0x02,                                       /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                       /* bInterfaceProtocol: Common AT commands */
  0x00,                                       /* iInterface: */

  /* Header Functional Descriptor */
  0x05,                                       /* bLength: Endpoint Descriptor size */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x00,                                       /* bDescriptorSubtype: Header Func Desc */
  0x10,                                       /* bcdPrinter: spec release number */
  0x01,

  /*Call Management Functional Descriptor*/
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x01,                                       /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                       /* bmCapabilities: D0+D1 */
  0x01,                                       /* bDataInterface: 1 */

  /*ACM Functional Descriptor*/
  0x04,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x02,                                       /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                       /* bmCapabilities */

  /*Union Functional Descriptor*/
  0x05,                                       /* bFunctionLength */
  0x24,                                       /* bDescriptorType: CS_INTERFACE */
  0x06,                                       /* bDescriptorSubtype: Union func desc */
  0x00,                                       /* bMasterInterface: Communication class interface */
  0x01,                                       /* bSlaveInterface0: Data Class Interface */

  /*Endpoint 2 Descriptor*/
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_CMD_EP,                                 /* bEndpointAddress */
  0x03,                                       /* bmAttributes: Interrupt */
  LOBYTE(PRINTER_CMD_PACKET_SIZE),                /* wMaxPacketSize: */
  HIBYTE(PRINTER_CMD_PACKET_SIZE),
  PRINTER_FS_BINTERVAL,                           /* bInterval: */

  /*---------------------------------------------------------------------------*/

  /*Data class interface descriptor*/
  0x09,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                    /* bDescriptorType: */
  0x01,                                       /* bInterfaceNumber: Number of Interface */
  0x00,                                       /* bAlternateSetting: Alternate setting */
  0x02,                                       /* bNumEndpoints: Two endpoints used */
  0x0A,                                       /* bInterfaceClass: Printer */
  0x00,                                       /* bInterfaceSubClass: */
  0x00,                                       /* bInterfaceProtocol: */
  0x00,                                       /* iInterface: */

  /*Endpoint OUT Descriptor*/
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_OUT_EP,                                 /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  0x40,                                       /* wMaxPacketSize: */
  0x00,
  0x00,                                       /* bInterval: ignore for Bulk transfer */

  /*Endpoint IN Descriptor*/
  0x07,                                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                     /* bDescriptorType: Endpoint */
  PRINTER_IN_EP,                                  /* bEndpointAddress */
  0x02,                                       /* bmAttributes: Bulk */
  0x40,                                       /* wMaxPacketSize: */
  0x00,
  0x00                                        /* bInterval */
};


struct usbd_printer_class
{
//    struct os_usbd_printer_dev printer_dev;

    USBD_HandleTypeDef *pdev;

    os_uint8_t *buff;
    os_size_t   size;
    os_size_t   count;

    os_list_node_t list;
};

static os_list_node_t usbd_printer_list = OS_LIST_INIT(usbd_printer_list);
#if 0
static os_err_t usbd_printer_init(struct os_usbd_printer_dev *printer_dev, struct usbd_printer_configure *cfg)
{
    return OS_EOK;
}

static int usbd_printer_start_recv(struct os_usbd_printer_dev *printer_dev, os_uint8_t *buff, os_size_t size)
{
    USBD_StatusTypeDef  ret = USBD_OK;
    struct usbd_printer_class *printer_class;

    OS_ASSERT(printer_dev != OS_NULL);

    printer_class = os_container_of(printer_dev, struct usbd_printer_class, printer_dev);

    printer_class->buff = buff;
    printer_class->size = size;

    USBD_Printer_ItfTypeDef * printer_fops = (USBD_Printer_ItfTypeDef *)(printer_class->pdev->pUserData);
    
    ret = (USBD_StatusTypeDef)printer_fops->Receive(printer_class->pdev, (uint8_t *)buff, size);

    return (ret == USBD_OK) ? size : 0;
}

static int usbd_printer_stop_recv(struct os_usbd_printer_dev *printer_dev)
{   
    return USBD_OK;
}

static int usbd_printer_start_send(struct os_usbd_printer_dev *printer_dev, const os_uint8_t *buff, os_size_t size)
{
    struct usbd_printer_class *printer_class;
    USBD_StatusTypeDef  ret = USBD_OK;
    
    OS_ASSERT(printer_dev != OS_NULL);

    printer_class = os_container_of(printer_dev, struct usbd_printer_class, printer_dev);

    USBD_Printer_ItfTypeDef * printer_fops = (USBD_Printer_ItfTypeDef *)(printer_class->pdev->pUserData);
    
    ret = (USBD_StatusTypeDef)printer_fops->Transmit(printer_class->pdev, (uint8_t *)buff, size);
    
    return (ret == USBD_OK) ? size : 0;
}

static int usbd_printer_stop_send(struct os_usbd_printer_dev *printer_dev)
{
    return USBD_OK;
}

static int usbd_printer_poll_send(struct os_usbd_printer_dev *printer_dev, const os_uint8_t *buff, os_size_t size)
{
    struct usbd_printer_class *printer_class;
    USBD_StatusTypeDef  ret = USBD_OK;
    
    OS_ASSERT(printer_dev != OS_NULL);

    printer_class = os_container_of(printer_dev, struct usbd_printer_class, printer_dev);

    USBD_Printer_ItfTypeDef * printer_fops = (USBD_Printer_ItfTypeDef *)(printer_class->pdev->pUserData);
    
    ret = (USBD_StatusTypeDef)printer_fops->Transmit(printer_class->pdev, (uint8_t *)buff, size);
    
    return (ret == USBD_OK) ? size : 0;
}

static const struct os_usbd_printer_ops usbd_printer_ops = 
{
    .init         = usbd_printer_init,

    .start_send   = usbd_printer_start_send,
    .stop_send    = usbd_printer_stop_send,

    .start_recv   = usbd_printer_start_recv,
    .stop_recv    = usbd_printer_stop_recv,
    
    .poll_send    = usbd_printer_poll_send,
};
#endif
int usbd_printer_register(USBD_HandleTypeDef *pdev)
{
    #if 0
    os_base_t   level;
    char* dev_name = NULL;

    if (pdev->id == PCD_USB_OTG_HS)
    {
        dev_name = "usbd_printer_hs";
    }
    else
    {
        dev_name = "usbd_printer_fs";
    }

    struct usbd_printer_class *printer_class = os_calloc(1, sizeof(struct usbd_printer_class));
//    struct usbd_printer_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    OS_ASSERT(printer_class);
    
    struct os_usbd_printer_dev *printer_dev = &printer_class->printer_dev;

    printer_class->pdev = pdev;

    level = os_irq_lock();
    os_list_add(&usbd_printer_list, &printer_class->list);
    os_irq_unlock(level);
    
//    printer_dev->ops  = &usbd_printer_ops;
//    printer_dev->config = config;
    
//    return os_hw_usbd_printer_register(printer_dev, dev_name, NULL);
#else
    return OS_EOK;
#endif

}

int usbd_printer_unregister(USBD_HandleTypeDef *pdev)
{
    #if 0
    os_err_t    result  = 0;
    
    struct usbd_printer_class *usbd_printer;
    
    os_list_for_each_entry(usbd_printer, &usbd_printer_list, struct usbd_printer_class, list)
    {
        if (usbd_printer->pdev == pdev)
        {
            USBD_DbgLog("usbd_printer device unregister");
//            result = os_hw_usbd_printer_unregister(&usbd_printer->printer_dev);
            os_list_del(&usbd_printer->list);
            os_free(usbd_printer);
            break;
        }
    }
    
    return result;
    #else
    return OS_EOK;
    #endif
    
}

USBD_User_Ops printer_user_ops = 
{
    usbd_printer_register,
    usbd_printer_unregister,
};

USBD_Printer_ItfTypeDef USBD_Interface_fops =
{
  Printer_Interface_Init,
  Printer_Interface_DeInit,
  Printer_Interface_Control,
  Printer_Interface_Receive,
  Printer_Interface_ReceiveCplt,
  Printer_Interface_Transmit,
  Printer_Interface_TransmitCplt
};

/**
  * @brief  USBD_Printer_Init
  *         Initialize the Printer interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_Printer_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_Printer_HandleTypeDef *hprinter;

    pdev->user_ops = &printer_user_ops;

if (USBD_Printer_RegisterInterface(pdev, &USBD_Interface_fops) != USBD_OK)
  {
    Error_Handler();
  }    

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_EMEM;
  }

  hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Open EP IN */
    (void)pdev->ll_ops->LL_OpenEP(pdev, PRINTER_IN_EP, USBD_EP_TYPE_BULK,
                         PRINTER_DATA_HS_IN_PACKET_SIZE);

    pdev->ep_in[PRINTER_IN_EP & 0xFU].is_used = 1U;

    /* Open EP OUT */
    (void)pdev->ll_ops->LL_OpenEP(pdev, PRINTER_OUT_EP, USBD_EP_TYPE_BULK,
                         PRINTER_DATA_HS_OUT_PACKET_SIZE);

    pdev->ep_out[PRINTER_OUT_EP & 0xFU].is_used = 1U;

    /* Set bInterval for Printer CMD Endpoint */
    pdev->ep_in[PRINTER_CMD_EP & 0xFU].bInterval = PRINTER_HS_BINTERVAL;
  }
  else
  {
    /* Open EP IN */
    (void)pdev->ll_ops->LL_OpenEP(pdev, PRINTER_IN_EP, USBD_EP_TYPE_BULK,
                         PRINTER_DATA_FS_IN_PACKET_SIZE);

    pdev->ep_in[PRINTER_IN_EP & 0xFU].is_used = 1U;

    /* Open EP OUT */
    (void)pdev->ll_ops->LL_OpenEP(pdev, PRINTER_OUT_EP, USBD_EP_TYPE_BULK,
                         PRINTER_DATA_FS_OUT_PACKET_SIZE);

    pdev->ep_out[PRINTER_OUT_EP & 0xFU].is_used = 1U;

    /* Set bInterval for CMD Endpoint */
    pdev->ep_in[PRINTER_CMD_EP & 0xFU].bInterval = PRINTER_FS_BINTERVAL;
  }

  /* Open Command IN EP */
  (void)pdev->ll_ops->LL_OpenEP(pdev, PRINTER_CMD_EP, USBD_EP_TYPE_INTR, PRINTER_CMD_PACKET_SIZE);
  pdev->ep_in[PRINTER_CMD_EP & 0xFU].is_used = 1U;

  /* Init  physical Interface components */
  ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->Init(pdev);

  /* Init Xfer states */
  hprinter->TxState = 0U;
  hprinter->RxState = 0U;

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)pdev->ll_ops->LL_PrepareReceive(pdev, PRINTER_OUT_EP, hprinter->RxBuffer,
                                 PRINTER_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)pdev->ll_ops->LL_PrepareReceive(pdev, PRINTER_OUT_EP, hprinter->RxBuffer,
                                 PRINTER_DATA_FS_OUT_PACKET_SIZE);
  }

    /* release mailbox */
    pdev->hotplus_status = USBD_PLUG_IN;
    if(OS_EOK == os_mb_send(pdev->usbd_mb, (os_ubase_t)pdev, OS_NO_WAIT))
    {
        LOG_W("printer", "task1 send OK");
    }
  
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_DeInit
  *         DeInitialize the Printer layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_Printer_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
    UNUSED(cfgidx);

    /* Close EP IN */
    (void)pdev->ll_ops->LL_CloseEP(pdev, PRINTER_IN_EP);
    pdev->ep_in[PRINTER_IN_EP & 0xFU].is_used = 0U;

    /* Close EP OUT */
    (void)pdev->ll_ops->LL_CloseEP(pdev, PRINTER_OUT_EP);
    pdev->ep_out[PRINTER_OUT_EP & 0xFU].is_used = 0U;

    /* Close Command IN EP */
    (void)pdev->ll_ops->LL_CloseEP(pdev, PRINTER_CMD_EP);
    pdev->ep_in[PRINTER_CMD_EP & 0xFU].is_used = 0U;
    pdev->ep_in[PRINTER_CMD_EP & 0xFU].bInterval = 0U;

    /* DeInit  physical Interface components */
    if (pdev->pClassData != NULL)
    {
        if (pdev->pUserData != NULL)
        {
            ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->DeInit(pdev);
        }
//        (void)USBD_free(pdev->pClassData);
//        pdev->pClassData = NULL;
    }

    return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_Setup
  *         Handle the Printer specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
#if 0
static uint8_t USBD_Printer_Setup(USBD_HandleTypeDef *pdev,
                              USBD_SetupReqTypedef *req)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;
  uint16_t len;
  uint8_t ifalt = 0U;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  if (hprinter == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      if (req->wLength != 0U)
      {
        if ((req->bmRequest & 0x80U) != 0U)
        {
          ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->Control(pdev, req->bRequest,
                                                            (uint8_t *)hprinter->data,
                                                            req->wLength);

          len = MIN(PRINTER_REQ_MAX_DATA_SIZE, req->wLength);
          (void)USBD_CtlSendData(pdev, (uint8_t *)hprinter->data, len);
        }
        else
        {
          hprinter->CmdOpCode = req->bRequest;
          hprinter->CmdLength = (uint8_t)req->wLength;

          (void)USBD_CtlPrepareRx(pdev, (uint8_t *)hprinter->data, req->wLength);
        }
      }
      else
      {
        ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->Control(pdev, req->bRequest,
                                                          (uint8_t *)req, 0U);
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}
#else
static uint8_t USBD_Printer_Setup(USBD_HandleTypeDef *pdev,
                              USBD_SetupReqTypedef *req)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;
  uint16_t len;
  uint8_t ifalt = 0U;
  uint16_t status_info = 0U;
  USBD_StatusTypeDef ret = USBD_OK;

  if (hprinter == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS:
      if (req->wLength != 0U)
      {
        if ((req->bmRequest & 0x80U) != 0U)
        {
          ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->Control(pdev, req->bRequest,
                                                            (uint8_t *)hprinter->data,
                                                            req->wLength);

          len = MIN(PRINTER_REQ_MAX_DATA_SIZE, req->wLength);
          (void)USBD_CtlSendData(pdev, (uint8_t *)hprinter->data, len);
        }
        else
        {
          hprinter->CmdOpCode = req->bRequest;
          hprinter->CmdLength = (uint8_t)req->wLength;

          (void)USBD_CtlPrepareRx(pdev, (uint8_t *)hprinter->data, req->wLength);
        }
      }
      else
      {
        ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->Control(pdev, req->bRequest,
                                                          (uint8_t *)req, 0U);
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}
#endif
/**
  * @brief  USBD_Printer_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_Printer_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_Printer_HandleTypeDef *hprinter;

  uint32_t maxpacket;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  maxpacket = pdev->ll_ops->LL_GetEPMaxPacket(pdev, epnum);

  if ((pdev->ep_in[epnum].total_length > 0U) &&
      ((pdev->ep_in[epnum].total_length % maxpacket) == 0U))
  {
    /* Update the packet total length */
    pdev->ep_in[epnum].total_length = 0U;

    /* Send ZLP */
    (void)pdev->ll_ops->LL_Transmit(pdev, epnum, NULL, 0U);
  }
  else
  {
    hprinter->TxState = 0U;

    if (((USBD_Printer_ItfTypeDef *)pdev->pUserData)->TransmitCplt != NULL)
    {
      ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->TransmitCplt(pdev, hprinter->TxBuffer, &hprinter->TxLength, epnum);
    }
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_Printer_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  /* Get the received data length */
  hprinter->RxLength = pdev->ll_ops->LL_GetRxDataSize(pdev, epnum);

  /* USB data will be immediately processed, this allow next USB traffic being
  NAKed till the end of the application Xfer */

  ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->ReceiveCplt(pdev, hprinter->RxBuffer, &hprinter->RxLength, epnum);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_EP0_RxReady
  *         Handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_Printer_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;

  if (hprinter == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if ((pdev->pUserData != NULL) && (hprinter->CmdOpCode != 0xFFU))
  {
    ((USBD_Printer_ItfTypeDef *)pdev->pUserData)->Control(pdev, hprinter->CmdOpCode,
                                                      (uint8_t *)hprinter->data,
                                                      (uint16_t)hprinter->CmdLength);
    hprinter->CmdOpCode = 0xFFU;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_Printer_GetFSCfgDesc(uint16_t *length)
{
    #if 0
  *length = (uint16_t)sizeof(USBD_Printer_CfgFSDesc);

  return USBD_Printer_CfgFSDesc;
    #else
    *length = (uint16_t)sizeof(printer_config_desc);

  return (uint8_t*)&printer_config_desc;
    #endif
}

/**
  * @brief  USBD_Printer_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_Printer_GetHSCfgDesc(uint16_t *length)
{
//  *length = (uint16_t)sizeof(USBD_Printer_CfgHSDesc);
    *length = (uint16_t)sizeof(printer_config_desc);

  return (uint8_t*)&printer_config_desc;
}

/**
  * @brief  USBD_Printer_GetOtherSpeedCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_Printer_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_Printer_OtherSpeedCfgDesc);

  return USBD_Printer_OtherSpeedCfgDesc;
}

/**
  * @brief  USBD_Printer_GetDeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_Printer_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_Printer_DeviceQualifierDesc);

  return USBD_Printer_DeviceQualifierDesc;
}

/**
  * @brief  USBD_Printer_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t USBD_Printer_RegisterInterface(USBD_HandleTypeDef *pdev,
                                   USBD_Printer_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData = fops;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t USBD_Printer_SetTxBuffer(USBD_HandleTypeDef *pdev,
                             uint8_t *pbuff, uint32_t length)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;

  if (hprinter == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hprinter->TxBuffer = pbuff;
  hprinter->TxLength = length;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t USBD_Printer_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;

  if (hprinter == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  hprinter->RxBuffer = pbuff;

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_Printer_TransmitPacket
  *         Transmit packet on IN endpoint
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_Printer_TransmitPacket(USBD_HandleTypeDef *pdev)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;
  USBD_StatusTypeDef ret = USBD_BUSY;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (hprinter->TxState == 0U)
  {
    /* Tx Transfer in progress */
    hprinter->TxState = 1U;

    /* Update the packet total length */
    pdev->ep_in[PRINTER_IN_EP & 0xFU].total_length = hprinter->TxLength;

    /* Transmit next packet */
    (void)pdev->ll_ops->LL_Transmit(pdev, PRINTER_IN_EP, hprinter->TxBuffer, hprinter->TxLength);

    ret = USBD_OK;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_Printer_ReceivePacket
  *         prepare OUT Endpoint for reception
  * @param  pdev: device instance
  * @retval status
  */
uint8_t USBD_Printer_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef *)pdev->pClassData;

  if (pdev->pClassData == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    /* Prepare Out endpoint to receive next packet */
    (void)pdev->ll_ops->LL_PrepareReceive(pdev, PRINTER_OUT_EP, hprinter->RxBuffer,
                                 PRINTER_DATA_HS_OUT_PACKET_SIZE);
  }
  else
  {
    /* Prepare Out endpoint to receive next packet */
    (void)pdev->ll_ops->LL_PrepareReceive(pdev, PRINTER_OUT_EP, hprinter->RxBuffer,
                                 PRINTER_DATA_FS_OUT_PACKET_SIZE);
  }

  return (uint8_t)USBD_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the Printer media low layer over the USB HS IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Printer_Interface_Init(USBD_HandleTypeDef *pdev)
{
    return (USBD_OK);
}

/**
  * @brief  DeInitializes the Printer media low layer
  * @param  None
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Printer_Interface_DeInit(USBD_HandleTypeDef *pdev)
{
    return (USBD_OK);
}

/**
  * @brief  Manage the Printer class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t Printer_Interface_Control(USBD_HandleTypeDef *pdev, uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  
  switch(cmd)
  {
  case PRINTER_SEND_ENCAPSULATED_COMMAND:

    break;

  case PRINTER_GET_ENCAPSULATED_RESPONSE:

    break;

  case PRINTER_SET_COMM_FEATURE:

    break;

  case PRINTER_GET_COMM_FEATURE:

    break;

  case PRINTER_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
  case PRINTER_SET_LINE_CODING:

    break;

  case PRINTER_GET_LINE_CODING:

    break;

  case PRINTER_SET_CONTROL_LINE_STATE:

    break;

  case PRINTER_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  
}

/**
  * @brief Data received over USB OUT endpoint are sent over Printer interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on Printer interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAILL
  */
static int8_t Printer_Interface_Receive(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint32_t Len)
{
    USBD_Printer_SetRxBuffer(pdev, &Buf[0]);
    USBD_Printer_ReceivePacket(pdev);
    return (USBD_OK);
}

static int8_t Printer_Interface_ReceiveCplt(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint32_t *Len, uint8_t epnum)
{
    struct usbd_printer_class *usbd;
    uint8_t result = USBD_OK;
//    int temp = *Len;

    os_list_for_each_entry(usbd, &usbd_printer_list, struct usbd_printer_class, list)
    {
        if (pdev == usbd->pdev)
        {
//            os_hw_usbd_printer_isr_rxdone(&usbd->printer_dev, temp);
            break;
        }
    }

    return result;
}

/**
  * @brief  Data to send over USB IN endpoint are sent over Printer interface
  *         through this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t Printer_Interface_Transmit(USBD_HandleTypeDef *pdev, uint8_t* Buf, uint16_t Len)
{
    uint8_t result = USBD_OK;

    USBD_Printer_HandleTypeDef *hprinter = (USBD_Printer_HandleTypeDef*)pdev->pClassData;
    if (hprinter->TxState != 0){
    return USBD_BUSY;
    }
    USBD_Printer_SetTxBuffer(pdev, Buf, Len);
    result = USBD_Printer_TransmitPacket(pdev);
    return result;
}

/**
  * @brief  Printer_TransmitCplt
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */

static int8_t Printer_Interface_TransmitCplt(USBD_HandleTypeDef *pdev, uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
    struct usbd_printer_class *usbd;
    uint8_t result = USBD_OK;
    
    /* transmit done */

    os_list_for_each_entry(usbd, &usbd_printer_list, struct usbd_printer_class, list)
    {
        if (pdev == usbd->pdev)
        {
//            os_hw_usbd_printer_isr_txdone(&usbd->printer_dev);
            break;
        }
    }
    return result;
}
