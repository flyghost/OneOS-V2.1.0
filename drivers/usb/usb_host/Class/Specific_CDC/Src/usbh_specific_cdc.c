/**
  ******************************************************************************
  * @file    usbh_specific_cdc.c
  * @author  MCD Application Team
  * @brief   This file is the Specific_Specific_CDC Layer Handlers for USB Host Specific_Specific_CDC class.
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                Specific_Specific_CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as Specific_Specific_CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
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
- "stm32xxxxx_{eval}{discovery}{adafruit}_sd.c"
- "stm32xxxxx_{eval}{discovery}{adafruit}_lcd.c"
- "stm32xxxxx_{eval}{discovery}_sdram.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbh_specific_cdc.h"
#include "usbh_cdc_dev.h"
#include "driver.h"
#include "device.h"
#include <serial.h>
/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_Specific_CDC_CLASS
* @{
*/

/** @defgroup USBH_Specific_CDC_CORE
* @brief    This file includes Specific_Specific_CDC Layer Handlers for USB Host Specific_Specific_CDC class.
* @{
*/

/** @defgroup USBH_Specific_CDC_CORE_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBH_Specific_CDC_CORE_Private_Defines
* @{
*/
#define USBH_Specific_Specific_CDC_BUFFER_SIZE                 1024
/**
* @}
*/


/** @defgroup USBH_Specific_CDC_CORE_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup USBH_Specific_CDC_CORE_Private_Variables
* @{
*/
/**
* @}
*/


/** @defgroup USBH_Specific_CDC_CORE_Private_FunctionPrototypes
* @{
*/

static USBH_StatusTypeDef USBH_Specific_CDC_InterfaceInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_Specific_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_Specific_CDC_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_Specific_CDC_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_Specific_CDC_ClassRequest(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost,
                                        Specific_CDC_LineCodingTypeDef *linecoding);

static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
                                        Specific_CDC_LineCodingTypeDef *linecoding);

static void Specific_CDC_ProcessTransmission(USBH_HandleTypeDef *phost);

static void Specific_CDC_ProcessReception(USBH_HandleTypeDef *phost);

void USBH_Specific_CDC_TransmitCallback(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef  Specific_CDC_Class =
{
  "Specific_CDC",
  USB_SPECIFIC_CDC_CLASS,
  USBH_Specific_CDC_InterfaceInit,
  USBH_Specific_CDC_InterfaceDeInit,
  USBH_Specific_CDC_ClassRequest,
  USBH_Specific_CDC_Process,
  USBH_Specific_CDC_SOFProcess,
  NULL,
};
/**
* @}
*/

USBH_ID_To_CDC_Interface specific_interface_array[] = 
{
    {0x0125, 0xffff, 0x2c7c, 0xffff, 2},
    {0x0800, 0xffff, 0x2c7c, 0xffff, 2},
    {0x7523, 0xffff, 0x1a86, 0xffff, 0},
};


static USBH_ID_To_Class specific_id_to_class_tab[] = 
{
    {0x0125, 0xffff, 0x2c7c, 0xffff, USB_SPECIFIC_CDC_CLASS},
    {0x7523, 0xffff, 0x1a86, 0xffff, USB_SPECIFIC_CDC_CLASS},
    {0x0800, 0xffff, 0x2c7c, 0xffff, USB_SPECIFIC_CDC_CLASS},
};
const int specific_class_table_size = ARRAY_SIZE(specific_id_to_class_tab);

os_err_t specific_cdc_specific_id_init(void)
{
    uint8_t i;
    for (i=0; i<specific_class_table_size; i++)
        USBH_RegisterSpecificID(&specific_id_to_class_tab[i]);

    return OS_EOK;
}

OS_PREV_INIT(specific_cdc_specific_id_init, OS_INIT_SUBLEVEL_HIGH);
/** @defgroup USBH_Specific_CDC_CORE_Private_Functions
* @{
*/


struct usbh_specific_cdc_class
{
    struct os_usbh_cdc_dev cdc_dev;

    USBH_HandleTypeDef *phost;
    
//    HCD_HandleTypeDef *hhcd;

    os_bool_t   rx_isr_enabled;

    os_uint8_t *buff;
    os_size_t   size;
    os_size_t   count;

    os_list_node_t list;
};

static os_list_node_t usbh_cdc_list = OS_LIST_INIT(usbh_cdc_list);

static os_err_t usbh_cdc_init(struct os_usbh_cdc_dev *cdc_dev, struct usbh_cdc_configure *cfg)
{
    return OS_EOK;
}

static int usbh_cdc_start_recv(struct os_usbh_cdc_dev *cdc_dev, os_uint8_t *buff, os_size_t size)
{
    USBH_StatusTypeDef  ret = USBH_OK;
    struct usbh_specific_cdc_class *cdc_class;

    OS_ASSERT(cdc_dev != OS_NULL);

    cdc_class = os_container_of(cdc_dev, struct usbh_specific_cdc_class, cdc_dev);

    cdc_class->buff = buff;
    cdc_class->size = size;

    USBH_Specific_CDC_Receive(cdc_class->phost, buff, size);

    return (ret == USBH_OK) ? size : 0;
}

static int usbh_cdc_stop_recv(struct os_usbh_cdc_dev *cdc_dev)
{
    struct usbh_specific_cdc_class *cdc_class;

    OS_ASSERT(cdc_dev != OS_NULL);

    cdc_class = os_container_of(cdc_dev, struct usbh_specific_cdc_class, cdc_dev);
    
    cdc_class = cdc_class;
    
    return USBH_OK;
}

static int usbh_cdc_start_send(struct os_usbh_cdc_dev *cdc_dev, const os_uint8_t *buff, os_size_t size)
{
    struct usbh_specific_cdc_class *cdc_class;
    USBH_StatusTypeDef  ret = USBH_OK;
    
    OS_ASSERT(cdc_dev != OS_NULL);

    cdc_class = os_container_of(cdc_dev, struct usbh_specific_cdc_class, cdc_dev);

    USBH_Specific_CDC_Transmit(cdc_class->phost, (uint8_t *)buff, size);
    
    return (ret == USBH_OK) ? size : 0;
}

static int usbh_cdc_stop_send(struct os_usbh_cdc_dev *cdc_dev)
{
    
//    UNUSED(cdc_dev);
    return USBH_OK;
}

static int usbh_cdc_poll_send(struct os_usbh_cdc_dev *cdc_dev, const os_uint8_t *buff, os_size_t size)
{
    struct usbh_specific_cdc_class *cdc_class;
    USBH_StatusTypeDef  ret = USBH_OK;
    
    OS_ASSERT(cdc_dev != OS_NULL);

    cdc_class = os_container_of(cdc_dev, struct usbh_specific_cdc_class, cdc_dev);

    USBH_Specific_CDC_Transmit(cdc_class->phost, (uint8_t *)buff, size);
    
    return (ret == USBH_OK) ? size : 0;
}

static const struct os_usbh_cdc_ops usbh_cdc_ops = 
{
    .init         = usbh_cdc_init,

    .start_send   = usbh_cdc_start_send,
    .stop_send    = usbh_cdc_stop_send,

    .start_recv   = usbh_cdc_start_recv,
    .stop_recv    = usbh_cdc_stop_recv,
    
    .poll_send    = usbh_cdc_poll_send,
};

static int usbh_cdc_register(USBH_HandleTypeDef *phost)
{
    os_base_t   level;
    char* dev_name = NULL;

    if (phost->id == OTG_HS)
    {
        dev_name = "usbh_cdc_hs";
    }
    else
    {
        dev_name = "usbh_cdc_fs";
    }

    struct usbh_specific_cdc_class *cdc_class = os_calloc(1, sizeof(struct usbh_specific_cdc_class));
    struct usbh_cdc_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    OS_ASSERT(cdc_class);
    
    struct os_usbh_cdc_dev *cdc_dev = &cdc_class->cdc_dev;

    cdc_class->phost = phost;
//    cdc_class->hhcd = phost->pData;


    level = os_irq_lock();
    os_list_add(&usbh_cdc_list, &cdc_class->list);
    os_irq_unlock(level);
    
    cdc_dev->ops  = &usbh_cdc_ops;
    cdc_dev->config = config;
    
    return os_hw_usbh_cdc_register(cdc_dev, dev_name, NULL);

}

static int usbh_cdc_unregister(USBH_HandleTypeDef *phost)
{
    os_err_t    result  = 0;
    
    struct usbh_specific_cdc_class *usbh_cdc;
    
    os_list_for_each_entry(usbh_cdc, &usbh_cdc_list, struct usbh_specific_cdc_class, list)
    {
        if (usbh_cdc->phost == phost)
        {
            USBH_DbgLog("usbh_cdc device unregister");
            result = os_hw_usbh_cdc_unregister(&usbh_cdc->cdc_dev);
            os_list_del(&usbh_cdc->list);
            os_free(usbh_cdc);
            break;
        }
    }
    
    return result;
    
}

/**
  * @brief  USBH_Specific_CDC_InterfaceInit
  *         The function init the Specific_CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_Specific_CDC_InterfaceInit(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status;
  uint8_t interface = 0xFFU;
    uint8_t idx = 0U;
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle;

    for(idx=0; idx<ARRAY_SIZE(specific_interface_array); idx++)
    {
        if (((phost->device.DevDesc.idProduct & specific_interface_array[idx].PID_Mask) 
                == (specific_interface_array[idx].PID & specific_interface_array[idx].PID_Mask))
                && ((phost->device.DevDesc.idVendor & specific_interface_array[idx].VID_Mask) 
                == (specific_interface_array[idx].VID & specific_interface_array[idx].VID_Mask))
                )
        {
            interface = specific_interface_array[idx].cdc_interface_idx;
            break;
        }
    }
    
    if (interface == 0xFFU)
    {
      interface = USBH_FindInterface(phost, COMMUNICATION_INTERFACE_CLASS_CODE,
                                     ABSTRACT_CONTROL_MODEL, COMMON_AT_COMMAND);
    }
    
  if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) /* No Valid Interface */
  {
    USBH_DbgLog("Cannot Find the interface for %s Class.", phost->pActiveClass->Name);
    return USBH_FAIL;
  }

  status = USBH_SelectInterface(phost, interface);

  if (status != USBH_OK)
  {
    return USBH_FAIL;
  }

  phost->pActiveClass->pData = (Specific_CDC_HandleTypeDef *)USBH_malloc(sizeof(Specific_CDC_HandleTypeDef));
  Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (Specific_CDC_Handle == NULL)
  {
    USBH_DbgLog("Cannot allocate memory for Specific_Specific_CDC Handle");
    return USBH_FAIL;
  }

  /* Initialize Specific_CDC handler */
  USBH_memset(Specific_CDC_Handle, 0, sizeof(Specific_CDC_HandleTypeDef));

  /*Collect the notification endpoint address and length*/
  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U)
  {
    Specific_CDC_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    Specific_CDC_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }

  /*Allocate the length for host channel number in*/
  Specific_CDC_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, Specific_CDC_Handle->CommItf.NotifEp);

  /* Open pipe for Notification endpoint */
  USBH_OpenPipe(phost, Specific_CDC_Handle->CommItf.NotifPipe, Specific_CDC_Handle->CommItf.NotifEp,
                phost->device.address, phost->device.speed, USB_EP_TYPE_INTR,
                Specific_CDC_Handle->CommItf.NotifEpSize);

  phost->ll_ops->LL_SetToggle(phost, Specific_CDC_Handle->CommItf.NotifPipe, 0U);

//  interface = USBH_FindInterface(phost, DATA_INTERFACE_CLASS_CODE,
//                                 RESERVED, NO_CLASS_SPECIFIC_PROTOCOL_CODE);

//  if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) /* No Valid Interface */
//  {
//    USBH_DbgLog("Cannot Find the interface for Data Interface Class.", phost->pActiveClass->Name);
//    return USBH_FAIL;
//  }

  /*Collect the class specific endpoint address and length*/
  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U)
  {
    Specific_CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    Specific_CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }
  else
  {
    Specific_CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    Specific_CDC_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }

  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80U)
  {
    Specific_CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    Specific_CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  }
  else
  {
    Specific_CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    Specific_CDC_Handle->DataItf.OutEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  }
  
  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress & 0x80U)
  {
    Specific_CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress;
    Specific_CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].wMaxPacketSize;
  }
  else
  {
    Specific_CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress;
    Specific_CDC_Handle->DataItf.OutEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].wMaxPacketSize;
  }

  /*Allocate the length for host channel number out*/
  Specific_CDC_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, Specific_CDC_Handle->DataItf.OutEp);

  /*Allocate the length for host channel number in*/
  Specific_CDC_Handle->DataItf.InPipe = USBH_AllocPipe(phost, Specific_CDC_Handle->DataItf.InEp);

  /* Open channel for OUT endpoint */
  USBH_OpenPipe(phost, Specific_CDC_Handle->DataItf.OutPipe, Specific_CDC_Handle->DataItf.OutEp,
                phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                Specific_CDC_Handle->DataItf.OutEpSize);

  /* Open channel for IN endpoint */
  USBH_OpenPipe(phost, Specific_CDC_Handle->DataItf.InPipe, Specific_CDC_Handle->DataItf.InEp,
                phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                Specific_CDC_Handle->DataItf.InEpSize);

  Specific_CDC_Handle->state = SPECIFIC_CDC_IDLE_STATE;

  phost->ll_ops->LL_SetToggle(phost, Specific_CDC_Handle->DataItf.OutPipe, 0U);
  phost->ll_ops->LL_SetToggle(phost, Specific_CDC_Handle->DataItf.InPipe, 0U);

    usbh_cdc_register(phost);

  return USBH_OK;
}



/**
  * @brief  USBH_Specific_CDC_InterfaceDeInit
  *         The function DeInit the Pipes used for the Specific_Specific_CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_Specific_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (Specific_CDC_Handle->CommItf.NotifPipe)
  {
    USBH_ClosePipe(phost, Specific_CDC_Handle->CommItf.NotifPipe);
    USBH_FreePipe(phost, Specific_CDC_Handle->CommItf.NotifPipe);
    Specific_CDC_Handle->CommItf.NotifPipe = 0U;     /* Reset the Channel as Free */
  }

  if (Specific_CDC_Handle->DataItf.InPipe)
  {
    USBH_ClosePipe(phost, Specific_CDC_Handle->DataItf.InPipe);
    USBH_FreePipe(phost, Specific_CDC_Handle->DataItf.InPipe);
    Specific_CDC_Handle->DataItf.InPipe = 0U;     /* Reset the Channel as Free */
  }

  if (Specific_CDC_Handle->DataItf.OutPipe)
  {
    USBH_ClosePipe(phost, Specific_CDC_Handle->DataItf.OutPipe);
    USBH_FreePipe(phost, Specific_CDC_Handle->DataItf.OutPipe);
    Specific_CDC_Handle->DataItf.OutPipe = 0U;    /* Reset the Channel as Free */
  }

  if (phost->pActiveClass->pData)
  {
    USBH_free(phost->pActiveClass->pData);
    phost->pActiveClass->pData = 0U;
  }

    usbh_cdc_unregister(phost);

  return USBH_OK;
}

/**
  * @brief  USBH_Specific_CDC_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for Specific_Specific_CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_Specific_CDC_ClassRequest(USBH_HandleTypeDef *phost)
{
    #if 0
    USBH_StatusTypeDef status;
    Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

    /* Issue the get line coding request */
    status = GetLineCoding(phost, &Specific_CDC_Handle->LineCoding);
    if (status == USBH_OK)
    {
        phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
    }
    else if (status == USBH_NOT_SUPPORTED)
    {
        USBH_ErrLog("Control error: Specific_Specific_CDC: Device Get Line Coding configuration failed");
    }
    else
    {
        /* .. */
    }

    return status;
    #else
    return USBH_OK;
    #endif
}


/**
  * @brief  USBH_Specific_CDC_Process
  *         The function is for managing state machine for Specific_Specific_CDC data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_Specific_CDC_Process(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef req_status = USBH_OK;
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  switch (Specific_CDC_Handle->state)
  {

    case SPECIFIC_CDC_IDLE_STATE:
      status = USBH_OK;
      break;

    case SPECIFIC_CDC_SET_LINE_CODING_STATE:
      req_status = SetLineCoding(phost, Specific_CDC_Handle->pUserLineCoding);

      if (req_status == USBH_OK)
      {
        Specific_CDC_Handle->state = SPECIFIC_CDC_GET_LAST_LINE_CODING_STATE;
      }

      else
      {
        if (req_status != USBH_BUSY)
        {
          Specific_CDC_Handle->state = SPECIFIC_CDC_ERROR_STATE;
        }
      }
      break;


    case SPECIFIC_CDC_GET_LAST_LINE_CODING_STATE:
      req_status = GetLineCoding(phost, &(Specific_CDC_Handle->LineCoding));

      if (req_status == USBH_OK)
      {
        Specific_CDC_Handle->state = SPECIFIC_CDC_IDLE_STATE;

        if ((Specific_CDC_Handle->LineCoding.b.bCharFormat == Specific_CDC_Handle->pUserLineCoding->b.bCharFormat) &&
            (Specific_CDC_Handle->LineCoding.b.bDataBits == Specific_CDC_Handle->pUserLineCoding->b.bDataBits) &&
            (Specific_CDC_Handle->LineCoding.b.bParityType == Specific_CDC_Handle->pUserLineCoding->b.bParityType) &&
            (Specific_CDC_Handle->LineCoding.b.dwDTERate == Specific_CDC_Handle->pUserLineCoding->b.dwDTERate))
        {
          USBH_Specific_CDC_LineCodingChanged(phost);
        }
      }
      else
      {
        if (req_status != USBH_BUSY)
        {
          Specific_CDC_Handle->state = SPECIFIC_CDC_ERROR_STATE;
        }
      }
      break;

    case SPECIFIC_CDC_TRANSFER_DATA:
      Specific_CDC_ProcessTransmission(phost);
      Specific_CDC_ProcessReception(phost);
      break;

    case SPECIFIC_CDC_ERROR_STATE:
      req_status = USBH_ClrFeature(phost, 0x00U);

      if (req_status == USBH_OK)
      {
        /*Change the state to waiting*/
        Specific_CDC_Handle->state = SPECIFIC_CDC_IDLE_STATE;
      }
      break;

    default:
      break;

  }

  return status;
}

/**
  * @brief  USBH_Specific_CDC_SOFProcess
  *         The function is for managing SOF callback
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_Specific_CDC_SOFProcess(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
//  UNUSED(phost);

  return USBH_OK;
}


/**
  * @brief  USBH_Specific_CDC_Stop
  *         Stop current Specific_Specific_CDC Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_Specific_CDC_Stop(USBH_HandleTypeDef *phost)
{
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    Specific_CDC_Handle->state = SPECIFIC_CDC_IDLE_STATE;

    USBH_ClosePipe(phost, Specific_CDC_Handle->CommItf.NotifPipe);
    USBH_ClosePipe(phost, Specific_CDC_Handle->DataItf.InPipe);
    USBH_ClosePipe(phost, Specific_CDC_Handle->DataItf.OutPipe);
  }
  return USBH_OK;
}
/**
  * @brief  This request allows the host to find out the currently
  *         configured line coding.
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost, Specific_CDC_LineCodingTypeDef *linecoding)
{

  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS | \
                                         USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = SPECIFIC_CDC_GET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0U;
  phost->Control.setup.b.wIndex.w = 0U;
  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

  return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
}


/**
  * @brief  This request allows the host to specify typical asynchronous
  * line-character formatting properties
  * This request applies to asynchronous byte stream data class interfaces
  * and endpoints
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
                                        Specific_CDC_LineCodingTypeDef *linecoding)
{
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS |
                                         USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = SPECIFIC_CDC_SET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0U;

  phost->Control.setup.b.wIndex.w = 0U;

  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

  return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef USBH_Specific_CDC_SetLineCoding(USBH_HandleTypeDef *phost,
                                          Specific_CDC_LineCodingTypeDef *linecoding)
{
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    Specific_CDC_Handle->state = SPECIFIC_CDC_SET_LINE_CODING_STATE;
    Specific_CDC_Handle->pUserLineCoding = linecoding;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
  }

  return USBH_OK;
}

/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_Specific_CDC_GetLineCoding(USBH_HandleTypeDef *phost,
                                           Specific_CDC_LineCodingTypeDef *linecoding)
{
    #if 0
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((phost->gState == HOST_CLASS) || (phost->gState == HOST_CLASS_REQUEST))
  {
    *linecoding = Specific_CDC_Handle->LineCoding;
    return USBH_OK;
  }
  else
  {
    return USBH_FAIL;
  }
    #else
  return USBH_OK;
    #endif
}

/**
  * @brief  This function return last received data size
  * @param  None
  * @retval None
  */
uint16_t USBH_Specific_CDC_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
  uint32_t dataSize;
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    dataSize = phost->ll_ops->LL_GetLastXferSize(phost, Specific_CDC_Handle->DataItf.InPipe);
  }
  else
  {
    dataSize =  0U;
  }

  return (uint16_t)dataSize;
}

/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef  USBH_Specific_CDC_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((Specific_CDC_Handle->state == SPECIFIC_CDC_IDLE_STATE) || (Specific_CDC_Handle->state == SPECIFIC_CDC_TRANSFER_DATA))
  {
    Specific_CDC_Handle->pTxData = pbuff;
    Specific_CDC_Handle->TxDataLength = length;
    Specific_CDC_Handle->state = SPECIFIC_CDC_TRANSFER_DATA;
    Specific_CDC_Handle->data_tx_state = SPECIFIC_CDC_SEND_DATA;
    Status = USBH_OK;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
  }
  return Status;
}


/**
* @brief  This function prepares the state before issuing the class specific commands
* @param  None
* @retval None
*/
USBH_StatusTypeDef  USBH_Specific_CDC_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((Specific_CDC_Handle->state == SPECIFIC_CDC_IDLE_STATE) || (Specific_CDC_Handle->state == SPECIFIC_CDC_TRANSFER_DATA))
  {
    Specific_CDC_Handle->pRxData = pbuff;
    Specific_CDC_Handle->RxDataLength = length;
    Specific_CDC_Handle->state = SPECIFIC_CDC_TRANSFER_DATA;
    Specific_CDC_Handle->data_rx_state = SPECIFIC_CDC_RECEIVE_DATA;
    Status = USBH_OK;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
  }
  return Status;
}

/**
* @brief  The function is responsible for sending data to the device
*  @param  pdev: Selected device
* @retval None
*/
static void Specific_CDC_ProcessTransmission(USBH_HandleTypeDef *phost)
{
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

  switch (Specific_CDC_Handle->data_tx_state)
  {
    case SPECIFIC_CDC_SEND_DATA:
      if (Specific_CDC_Handle->TxDataLength > Specific_CDC_Handle->DataItf.OutEpSize)
      {
        USBH_BulkSendData(phost,
                          Specific_CDC_Handle->pTxData,
                          Specific_CDC_Handle->DataItf.OutEpSize,
                          Specific_CDC_Handle->DataItf.OutPipe,
                          1U);
      }
      else
      {
        USBH_BulkSendData(phost,
                          Specific_CDC_Handle->pTxData,
                          (uint16_t)Specific_CDC_Handle->TxDataLength,
                          Specific_CDC_Handle->DataItf.OutPipe,
                          1U);
      }

      Specific_CDC_Handle->data_tx_state = SPECIFIC_CDC_SEND_DATA_WAIT;
      break;

    case SPECIFIC_CDC_SEND_DATA_WAIT:

      URB_Status = phost->ll_ops->LL_GetURBState(phost, Specific_CDC_Handle->DataItf.OutPipe);

      /* Check the status done for transmission */
      if (URB_Status == USBH_URB_DONE)
      {
        if (Specific_CDC_Handle->TxDataLength > Specific_CDC_Handle->DataItf.OutEpSize)
        {
          Specific_CDC_Handle->TxDataLength -= Specific_CDC_Handle->DataItf.OutEpSize;
          Specific_CDC_Handle->pTxData += Specific_CDC_Handle->DataItf.OutEpSize;
        }
        else
        {
          Specific_CDC_Handle->TxDataLength = 0U;
        }

        if (Specific_CDC_Handle->TxDataLength > 0U)
        {
          Specific_CDC_Handle->data_tx_state = SPECIFIC_CDC_SEND_DATA;
        }
        else
        {
          Specific_CDC_Handle->data_tx_state = SPECIFIC_CDC_IDLE;
          USBH_Specific_CDC_TransmitCallback(phost);
        }

#if (USBH_USE_OS == 1U)
        phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
        (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
      }
      else
      {
        if (URB_Status == USBH_URB_NOTREADY)
        {
          Specific_CDC_Handle->data_tx_state = SPECIFIC_CDC_SEND_DATA;

#if (USBH_USE_OS == 1U)
          phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
          (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
          (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
        }
      }
      break;

    default:
      break;
  }
}
/**
* @brief  This function responsible for reception of data from the device
*  @param  pdev: Selected device
* @retval None
*/

static void Specific_CDC_ProcessReception(USBH_HandleTypeDef *phost)
{
  Specific_CDC_HandleTypeDef *Specific_CDC_Handle = (Specific_CDC_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  uint32_t length;

  switch (Specific_CDC_Handle->data_rx_state)
  {

    case SPECIFIC_CDC_RECEIVE_DATA:

      USBH_BulkReceiveData(phost,
                           Specific_CDC_Handle->pRxData,
                           Specific_CDC_Handle->DataItf.InEpSize,
                           Specific_CDC_Handle->DataItf.InPipe);

      Specific_CDC_Handle->data_rx_state = SPECIFIC_CDC_RECEIVE_DATA_WAIT;

      break;

    case SPECIFIC_CDC_RECEIVE_DATA_WAIT:

      URB_Status = phost->ll_ops->LL_GetURBState(phost, Specific_CDC_Handle->DataItf.InPipe);

      /*Check the status done for reception*/
      if (URB_Status == USBH_URB_DONE)
      {
        length = phost->ll_ops->LL_GetLastXferSize(phost, Specific_CDC_Handle->DataItf.InPipe);

        if (((Specific_CDC_Handle->RxDataLength - length) > 0U) && (length > Specific_CDC_Handle->DataItf.InEpSize))
        {
          Specific_CDC_Handle->RxDataLength -= length ;
          Specific_CDC_Handle->pRxData += length;
          Specific_CDC_Handle->data_rx_state = SPECIFIC_CDC_RECEIVE_DATA;
        }
        else
        {
          Specific_CDC_Handle->data_rx_state = SPECIFIC_CDC_IDLE;
          USBH_Specific_CDC_ReceiveCallback(phost);
        }

#if (USBH_USE_OS == 1U)
        phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
        (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, NULL);
#endif
#endif
      }
      break;

    default:
      break;
  }
}

/**
* @brief  The function informs user that data have been received
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_Specific_CDC_TransmitCallback(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    USBH_DbgLog("Transmit Finished");

    struct usbh_specific_cdc_class *usbh_cdc;
    
    os_list_for_each_entry(usbh_cdc, &usbh_cdc_list, struct usbh_specific_cdc_class, list)
    {
        if (usbh_cdc->phost == phost)
        {
            os_hw_usbh_cdc_isr_txdone(&usbh_cdc->cdc_dev);
            break;
        }
    }
}

/**
* @brief  The function informs user that data have been sent
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_Specific_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    USBH_DbgLog("Receive Finished");

    struct usbh_specific_cdc_class *usbh_cdc;
    
    os_list_for_each_entry(usbh_cdc, &usbh_cdc_list, struct usbh_specific_cdc_class, list)
    {
        if (usbh_cdc->phost == phost)
        {
            usbh_cdc->count = USBH_Specific_CDC_GetLastReceivedDataSize(phost);
            os_hw_usbh_cdc_isr_rxdone(&usbh_cdc->cdc_dev, usbh_cdc->count);
            break;
        }
    }
}

/**
* @brief  The function informs user that Settings have been changed
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_Specific_CDC_LineCodingChanged(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
//  UNUSED(phost);
}

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


/**
* @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
