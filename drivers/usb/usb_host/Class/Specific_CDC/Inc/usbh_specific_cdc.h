/**
  ******************************************************************************
  * @file    usbh_specific_cdc.h
  * @author  MCD Application Team
  * @brief   This file contains all the prototypes for the usbh_SPECIFIC_CDC.c
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

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_SPECIFIC_CDC_H
#define __USBH_SPECIFIC_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "proto_usbh_core.h"


/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_SPECIFIC_CDC_CLASS
* @{
*/

/** @defgroup USBH_SPECIFIC_CDC_CORE
* @brief This file is the Header file for usbh_core.c
* @{
*/




/*Communication Class codes*/
#define USB_SPECIFIC_CDC_CLASS                               0xCCU
#define COMMUNICATION_INTERFACE_CLASS_CODE                      0x02U

/*Data Interface Class Codes*/
#define DATA_INTERFACE_CLASS_CODE                               0x0AU

/*Communication sub class codes*/
#define RESERVED                                                0x00U
#define DIRECT_LINE_CONTROL_MODEL                               0x01U
#define ABSTRACT_CONTROL_MODEL                                  0x02U
#define TELEPHONE_CONTROL_MODEL                                 0x03U
#define MULTICHANNEL_CONTROL_MODEL                              0x04U
#define CAPI_CONTROL_MODEL                                      0x05U
#define ETHERNET_NETWORKING_CONTROL_MODEL                       0x06U
#define ATM_NETWORKING_CONTROL_MODEL                            0x07U


/*Communication Interface Class Control Protocol Codes*/
#define NO_CLASS_SPECIFIC_PROTOCOL_CODE                         0x00U
#define COMMON_AT_COMMAND                                       0x01U
#define VENDOR_SPECIFIC                                         0xFFU


#define CS_INTERFACE                                            0x24U
#define SPECIFIC_CDC_PAGE_SIZE_64                                        0x40U

/*Class-Specific Request Codes*/
#define SPECIFIC_CDC_SEND_ENCAPSULATED_COMMAND                           0x00U
#define SPECIFIC_CDC_GET_ENCAPSULATED_RESPONSE                           0x01U
#define SPECIFIC_CDC_SET_COMM_FEATURE                                    0x02U
#define SPECIFIC_CDC_GET_COMM_FEATURE                                    0x03U
#define SPECIFIC_CDC_CLEAR_COMM_FEATURE                                  0x04U

#define SPECIFIC_CDC_SET_AUX_LINE_STATE                                  0x10U
#define SPECIFIC_CDC_SET_HOOK_STATE                                      0x11U
#define SPECIFIC_CDC_PULSE_SETUP                                         0x12U
#define SPECIFIC_CDC_SEND_PULSE                                          0x13U
#define SPECIFIC_CDC_SET_PULSE_TIME                                      0x14U
#define SPECIFIC_CDC_RING_AUX_JACK                                       0x15U

#define SPECIFIC_CDC_SET_LINE_CODING                                     0x20U
#define SPECIFIC_CDC_GET_LINE_CODING                                     0x21U
#define SPECIFIC_CDC_SET_CONTROL_LINE_STATE                              0x22U
#define SPECIFIC_CDC_SEND_BREAK                                          0x23U

#define SPECIFIC_CDC_SET_RINGER_PARMS                                    0x30U
#define SPECIFIC_CDC_GET_RINGER_PARMS                                    0x31U
#define SPECIFIC_CDC_SET_OPERATION_PARMS                                 0x32U
#define SPECIFIC_CDC_GET_OPERATION_PARMS                                 0x33U
#define SPECIFIC_CDC_SET_LINE_PARMS                                      0x34U
#define SPECIFIC_CDC_GET_LINE_PARMS                                      0x35U
#define SPECIFIC_CDC_DIAL_DIGITS                                         0x36U
#define SPECIFIC_CDC_SET_UNIT_PARAMETER                                  0x37U
#define SPECIFIC_CDC_GET_UNIT_PARAMETER                                  0x38U
#define SPECIFIC_CDC_CLEAR_UNIT_PARAMETER                                0x39U
#define SPECIFIC_CDC_GET_PROFILE                                         0x3AU

#define SPECIFIC_CDC_SET_ETHERNET_MULTICAST_FILTERS                      0x40U
#define SPECIFIC_CDC_SET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x41U
#define SPECIFIC_CDC_GET_ETHERNET_POWER_MANAGEMENT_PATTERN FILTER        0x42U
#define SPECIFIC_CDC_SET_ETHERNET_PACKET_FILTER                          0x43U
#define SPECIFIC_CDC_GET_ETHERNET_STATISTIC                              0x44U

#define SPECIFIC_CDC_SET_ATM_DATA_FORMAT                                 0x50U
#define SPECIFIC_CDC_GET_ATM_DEVICE_STATISTICS                           0x51U
#define SPECIFIC_CDC_SET_ATM_DEFAULT_VC                                  0x52U
#define SPECIFIC_CDC_GET_ATM_VC_STATISTICS                               0x53U


/* wValue for SetControlLineState*/
#define SPECIFIC_CDC_ACTIVATE_CARRIER_SIGNAL_RTS                         0x0002U
#define SPECIFIC_CDC_DEACTIVATE_CARRIER_SIGNAL_RTS                       0x0000U
#define SPECIFIC_CDC_ACTIVATE_SIGNAL_DTR                                 0x0001U
#define SPECIFIC_CDC_DEACTIVATE_SIGNAL_DTR                               0x0000U

#define LINE_CODING_STRUCTURE_SIZE                              0x07U
/**
  * @}
  */

/** @defgroup USBH_SPECIFIC_CDC_CORE_Exported_Types
* @{
*/

/* States for SPECIFIC_SPECIFIC_CDC State Machine */
typedef enum
{
  SPECIFIC_CDC_IDLE = 0U,
  SPECIFIC_CDC_SEND_DATA,
  SPECIFIC_CDC_SEND_DATA_WAIT,
  SPECIFIC_CDC_RECEIVE_DATA,
  SPECIFIC_CDC_RECEIVE_DATA_WAIT,
}
Specific_CDC_DataStateTypeDef;

typedef enum
{
  SPECIFIC_CDC_IDLE_STATE = 0U,
  SPECIFIC_CDC_SET_LINE_CODING_STATE,
  SPECIFIC_CDC_GET_LAST_LINE_CODING_STATE,
  SPECIFIC_CDC_TRANSFER_DATA,
  SPECIFIC_CDC_ERROR_STATE,
}
Specific_CDC_StateTypeDef;

typedef struct
{
    uint16_t PID;
    uint16_t PID_Mask;
    uint16_t VID;
    uint16_t VID_Mask;
    uint8_t  cdc_interface_idx;
}
USBH_ID_To_CDC_Interface;

/*Line coding structure*/
typedef union _SPECIFIC_CDC_LineCodingStructure
{
  uint8_t Array[LINE_CODING_STRUCTURE_SIZE];

  struct
  {

    uint32_t             dwDTERate;     /*Data terminal rate, in bits per second*/
    uint8_t              bCharFormat;   /*Stop bits
    0 - 1 Stop bit
    1 - 1.5 Stop bits
    2 - 2 Stop bits*/
    uint8_t              bParityType;   /* Parity
    0 - None
    1 - Odd
    2 - Even
    3 - Mark
    4 - Space*/
    uint8_t                bDataBits;     /* Data bits (5, 6, 7, 8 or 16). */
  } b;
}
Specific_CDC_LineCodingTypeDef;



/* Header Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Identifier (ID) of functional
      |                     |       |            | descriptor.
3     |  bSPECIFIC_CDCDC             | 2     |            |
      |                     |       |   Number   | USB Class Definitions for
      |                     |       |            | Communication Devices Specification
      |                     |       |            | release number in binary-coded
      |                     |       |            | decimal
------|---------------------|-------|------------|------------------------------
*/
typedef struct _FunctionalDescriptorHeader
{
  uint8_t     bLength;            /*Size of this descriptor.*/
  uint8_t     bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t     bDescriptorSubType; /* Header functional descriptor subtype as*/
  uint16_t    bSPECIFIC_CDCDC;             /* USB Class Definitions for Communication
                                    Devices Specification release number in
                                  binary-coded decimal. */
}
Specific_CDC_HeaderFuncDesc_TypeDef;
/* Call Management Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Call Management functional
      |                     |       |            |  descriptor subtype.
3     |  bmCapabilities     | 1     |   Bitmap   | The capabilities that this configuration
      |                     |       |            | supports:
      |                     |       |            | D7..D2: RESERVED (Reset to zero)
      |                     |       |            | D1: 0 - Device sends/receives call
      |                     |       |            | management information only over
      |                     |       |            | the Communication Class
      |                     |       |            | interface.
      |                     |       |            | 1 - Device can send/receive call
      |                     \       |            | management information over a
      |                     |       |            | Data Class interface.
      |                     |       |            | D0: 0 - Device does not handle call
      |                     |       |            | management itself.
      |                     |       |            | 1 - Device handles call
      |                     |       |            | management itself.
      |                     |       |            | The previous bits, in combination, identify
      |                     |       |            | which call management scenario is used. If bit
      |                     |       |            | D0 is reset to 0, then the value of bit D1 is
      |                     |       |            | ignored. In this case, bit D1 is reset to zero for
      |                     |       |            | future compatibility.
4     | bDataInterface      | 1     | Number     | Interface number of Data Class interface
      |                     |       |            | optionally used for call management.
------|---------------------|-------|------------|------------------------------
*/
typedef struct _CallMgmtFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes.*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Call Management functional descriptor subtype*/
  uint8_t    bmCapabilities;      /* bmCapabilities: D0+D1 */
  uint8_t    bDataInterface;      /*bDataInterface: 1*/
}
Specific_CDC_CallMgmtFuncDesc_TypeDef;
/* Abstract Control Management Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of functional descriptor, in bytes.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Abstract Control Management
      |                     |       |            |  functional  descriptor subtype.
3     |  bmCapabilities     | 1     |   Bitmap   | The capabilities that this configuration
      |                     |       |            | supports ((A bit value of zero means that the
      |                     |       |            | request is not supported.) )
                                                   D7..D4: RESERVED (Reset to zero)
      |                     |       |            | D3: 1 - Device supports the notification
      |                     |       |            | Network_Connection.
      |                     |       |            | D2: 1 - Device supports the request
      |                     |       |            | Send_Break
      |                     |       |            | D1: 1 - Device supports the request
      |                     \       |            | combination of Set_Line_Coding,
      |                     |       |            | Set_Control_Line_State, Get_Line_Coding, and the
                                                   notification Serial_State.
      |                     |       |            | D0: 1 - Device supports the request
      |                     |       |            | combination of Set_Comm_Feature,
      |                     |       |            | Clear_Comm_Feature, and Get_Comm_Feature.
      |                     |       |            | The previous bits, in combination, identify
      |                     |       |            | which requests/notifications are supported by
      |                     |       |            | a Communication Class interface with the
      |                     |       |            |   SubClass code of Abstract Control Model.
------|---------------------|-------|------------|------------------------------
*/
typedef struct _AbstractCntrlMgmtFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes.*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Abstract Control Management functional
                                  descriptor subtype*/
  uint8_t    bmCapabilities;      /* The capabilities that this configuration supports */
}
Specific_CDC_AbstCntrlMgmtFuncDesc_TypeDef;
/* Union Functional Descriptor
--------------------------------------------------------------------------------
Offset|  field              | Size  |    Value   |   Description
------|---------------------|-------|------------|------------------------------
0     |  bFunctionLength    | 1     |   number   |  Size of this descriptor.
1     |  bDescriptorType    | 1     |   Constant |  CS_INTERFACE (0x24)
2     |  bDescriptorSubtype | 1     |   Constant |  Union functional
      |                     |       |            |  descriptor subtype.
3     |  bMasterInterface   | 1     |   Constant | The interface number of the
      |                     |       |            | Communication or Data Class interface
4     | bSlaveInterface0    | 1     | Number     | nterface number of first slave or associated
      |                     |       |            | interface in the union.
------|---------------------|-------|------------|------------------------------
*/
typedef struct _UnionFunctionalDescriptor
{
  uint8_t    bLength;            /*Size of this functional descriptor, in bytes*/
  uint8_t    bDescriptorType;    /*CS_INTERFACE (0x24)*/
  uint8_t    bDescriptorSubType; /* Union functional descriptor SubType*/
  uint8_t    bMasterInterface;   /* The interface number of the Communication or
                                 Data Class interface,*/
  uint8_t    bSlaveInterface0;   /*Interface number of first slave*/
}
Specific_CDC_UnionFuncDesc_TypeDef;

typedef struct _USBH_SPECIFIC_CDCInterfaceDesc
{
  Specific_CDC_HeaderFuncDesc_TypeDef           Specific_CDC_HeaderFuncDesc;
  Specific_CDC_CallMgmtFuncDesc_TypeDef         Specific_CDC_CallMgmtFuncDesc;
  Specific_CDC_AbstCntrlMgmtFuncDesc_TypeDef    Specific_CDC_AbstCntrlMgmtFuncDesc;
  Specific_CDC_UnionFuncDesc_TypeDef            Specific_CDC_UnionFuncDesc;
}
Specific_CDC_InterfaceDesc_Typedef;


/* Structure for Specific_Specific_CDC process */
typedef struct
{
  uint8_t              NotifPipe;
  uint8_t              NotifEp;
  uint8_t              buff[8];
  uint16_t             NotifEpSize;
}
Specific_CDC_CommItfTypedef ;

typedef struct
{
  uint8_t              InPipe;
  uint8_t              OutPipe;
  uint8_t              OutEp;
  uint8_t              InEp;
  uint8_t              buff[8];
  uint16_t             OutEpSize;
  uint16_t             InEpSize;
}
Specific_CDC_DataItfTypedef ;

/* Structure for Specific_Specific_CDC process */
typedef struct _Specific_CDC_Process
{
  Specific_CDC_CommItfTypedef                CommItf;
  Specific_CDC_DataItfTypedef                DataItf;
  uint8_t                           *pTxData;
  uint8_t                           *pRxData;
  uint32_t                           TxDataLength;
  uint32_t                           RxDataLength;
  Specific_CDC_InterfaceDesc_Typedef         Specific_CDC_Desc;
  Specific_CDC_LineCodingTypeDef             LineCoding;
  Specific_CDC_LineCodingTypeDef             *pUserLineCoding;
  Specific_CDC_StateTypeDef                  state;
  Specific_CDC_DataStateTypeDef              data_tx_state;
  Specific_CDC_DataStateTypeDef              data_rx_state;
  uint8_t                           Rx_Poll;
}
Specific_CDC_HandleTypeDef;

/**
* @}
*/

/** @defgroup USBH_SPECIFIC_CDC_CORE_Exported_Defines
* @{
*/

/**
* @}
*/

/** @defgroup USBH_SPECIFIC_CDC_CORE_Exported_Macros
* @{
*/
/**
* @}
*/

/** @defgroup USBH_SPECIFIC_CDC_CORE_Exported_Variables
* @{
*/
extern USBH_ClassTypeDef  Specific_CDC_Class;
#define USBH_SPECIFIC_CDC_CLASS    &Specific_CDC_Class

/**
* @}
*/

/** @defgroup USBH_SPECIFIC_CDC_CORE_Exported_FunctionsPrototype
* @{
*/

USBH_StatusTypeDef  USBH_Specific_CDC_SetLineCoding(USBH_HandleTypeDef *phost,
                                           Specific_CDC_LineCodingTypeDef *linecoding);

USBH_StatusTypeDef  USBH_Specific_CDC_GetLineCoding(USBH_HandleTypeDef *phost,
                                           Specific_CDC_LineCodingTypeDef *linecoding);

USBH_StatusTypeDef  USBH_Specific_CDC_Transmit(USBH_HandleTypeDef *phost,
                                      uint8_t *pbuff,
                                      uint32_t length);

USBH_StatusTypeDef  USBH_Specific_CDC_Receive(USBH_HandleTypeDef *phost,
                                     uint8_t *pbuff,
                                     uint32_t length);


uint16_t            USBH_Specific_CDC_GetLastReceivedDataSize(USBH_HandleTypeDef *phost);

USBH_StatusTypeDef  USBH_Specific_CDC_Stop(USBH_HandleTypeDef *phost);

void USBH_Specific_CDC_LineCodingChanged(USBH_HandleTypeDef *phost);

void USBH_Specific_CDC_TransmitCallback(USBH_HandleTypeDef *phost);

void USBH_Specific_CDC_ReceiveCallback(USBH_HandleTypeDef *phost);

/**
* @}
*/

#ifdef __cplusplus
}
#endif

#endif /* __USBH_SPECIFIC_CDC_H */

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
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

