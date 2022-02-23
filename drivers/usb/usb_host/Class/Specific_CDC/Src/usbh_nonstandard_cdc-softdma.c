/**
  ******************************************************************************
  * @file    usbh_nonstandard_cdc.c
  * @author  MCD Application Team
  * @brief   This file is the NonStandard_NonStandard_CDC Layer Handlers for USB Host NonStandard_NonStandard_CDC class.
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                NonStandard_NonStandard_CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as NonStandard_NonStandard_CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
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
#include "usbh_nonstandard_cdc.h"

#include <stdio.h>
#include <board.h>

#include <os_memory.h>
#include <bus/bus.h>
#include "usbh_nonstandard_cdc.h"
#include "drv_gpio.h"

typedef struct
{
    struct os_serial_device serial;

    USBH_HandleTypeDef *phost;
    
    HCD_HandleTypeDef *hhcd;
    
    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;
    
    os_uint8_t *rx_buff;
    os_uint32_t rx_index;
    os_uint32_t rx_size;

    const os_uint8_t *tx_buff;
    os_uint32_t tx_count;
    os_uint32_t tx_size;

    os_list_node_t list; 
    
}usbh_cdc_t;

static os_list_node_t usbh_cdc_list = OS_LIST_INIT(usbh_cdc_list);
/** @addtogroup USBH_LIB
* @{
*/

/** @addtogroup USBH_CLASS
* @{
*/

/** @addtogroup USBH_NonStandard_CDC_CLASS
* @{
*/

/** @defgroup USBH_NonStandard_CDC_CORE
* @brief    This file includes NonStandard_NonStandard_CDC Layer Handlers for USB Host NonStandard_NonStandard_CDC class.
* @{
*/

/** @defgroup USBH_NonStandard_CDC_CORE_Private_TypesDefinitions
* @{
*/
/**
* @}
*/


/** @defgroup USBH_NonStandard_CDC_CORE_Private_Defines
* @{
*/
#define USBH_NonStandard_NonStandard_CDC_BUFFER_SIZE                 1024
/**
* @}
*/


/** @defgroup USBH_NonStandard_CDC_CORE_Private_Macros
* @{
*/
/**
* @}
*/


/** @defgroup USBH_NonStandard_CDC_CORE_Private_Variables
* @{
*/
/**
* @}
*/


/** @defgroup USBH_NonStandard_CDC_CORE_Private_FunctionPrototypes
* @{
*/

static USBH_StatusTypeDef USBH_NonStandard_CDC_InterfaceInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_NonStandard_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_NonStandard_CDC_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_NonStandard_CDC_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_NonStandard_CDC_ClassRequest(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost,
                                        NonStandard_CDC_LineCodingTypeDef *linecoding);

static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
                                        NonStandard_CDC_LineCodingTypeDef *linecoding);

static void NonStandard_CDC_ProcessTransmission(USBH_HandleTypeDef *phost);

static void NonStandard_CDC_ProcessReception(USBH_HandleTypeDef *phost);

void USBH_NonStandard_CDC_TransmitCallback(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef  NonStandard_CDC_Class =
{
  "NonStandard_CDC",
  USB_NONSTANDARD_CDC_CLASS,
  USBH_NonStandard_CDC_InterfaceInit,
  USBH_NonStandard_CDC_InterfaceDeInit,
  USBH_NonStandard_CDC_ClassRequest,
  USBH_NonStandard_CDC_Process,
  USBH_NonStandard_CDC_SOFProcess,
  NULL,
};
/**
* @}
*/


USBH_ID_To_CDC_Interface specific_interface_array[] = 
{
    {0x0125, 0xffff, 0x2c7c, 0xffff, 2},
    {0x7523, 0xffff, 0x1a86, 0xffff, 0},
};

#include "driver.h"
static USBH_ID_To_Class specific_id_to_class_tab[] = 
{
    {0x0125, 0xffff, 0x2c7c, 0xffff, USB_NONSTANDARD_CDC_CLASS},
    {0x7523, 0xffff, 0x1a86, 0xffff, USB_NONSTANDARD_CDC_CLASS},
};
const int specific_class_table_size = ARRAY_SIZE(specific_id_to_class_tab);
#include "device.h"
#include <console.h>
os_err_t nonstandard_cdc_specific_id_init(void)
{
    uint8_t i;
    for (i=0; i<specific_class_table_size; i++)
        USBH_RegisterSpecificID(&specific_id_to_class_tab[i]);

    return OS_EOK;
}

OS_DEVICE_INIT(nonstandard_cdc_specific_id_init, OS_INIT_SUBLEVEL_HIGH);
/** @defgroup USBH_NonStandard_CDC_CORE_Private_Functions
* @{
*/
static int usbh_cdc_register(USBH_HandleTypeDef *phost);
static int usbh_cdc_unregister(USBH_HandleTypeDef *phost);
#if 0
usbh_cdc_t_info
{
    os_device_t dev;

    USBH_HandleTypeDef *phost;
    
    HCD_HandleTypeDef *hhcd;

    os_list_node_t list;

    os_uint8_t *rx_buff;
    
    os_uint32_t rx_size;
};

static os_list_node_t usbh_cdc_list = OS_LIST_INIT(usbh_cdc_list);
uint8_t temp_buffer[64];
static os_err_t usbh_cdc_init(os_device_t *dev)
{
#if 0
    struct stm32_usbh_cdc *usbh_cdc;
    os_uint32_t        data_bits;
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    usbh_cdc = os_container_of(serial, struct stm32_usbh_cdc, serial);

    
#endif
    usbh_cdc_t_info *usbh_cdc;
    
    dev->rx_size = 64;
    dev->rx_count = 0;
    
    usbh_cdc = os_container_of(dev, usbh_cdc_t_info, dev);
    
//    USBH_NonStandard_CDC_Receive(usbh_cdc->phost, temp_buffer, 64);

    return OS_EOK;
}

static os_err_t usbh_cdc_deinit(os_device_t *dev)
{
    #if 0
    OS_ASSERT(serial != OS_NULL);

    struct stm32_usbh_cdc *usbh_cdc = os_container_of(serial, struct stm32_usbh_cdc, serial);

    __HAL_UART_DISABLE_IT(usbh_cdc->husbh_cdc, UART_IT_IDLE);

    HAL_UART_AbortReceive(usbh_cdc->husbh_cdc);
    HAL_UART_AbortTransmit(usbh_cdc->husbh_cdc);

    soft_dma_stop(&usbh_cdc->sdma);
#endif
    return OS_EOK;
}

static os_size_t usbh_cdc_start_send(struct os_device *dev, os_off_t pos,const void *buffer, os_size_t size)
{
    usbh_cdc_t_info *usbh_cdc;
    HAL_StatusTypeDef  ret = HAL_OK;

    OS_ASSERT(dev != OS_NULL);

    usbh_cdc = os_container_of(dev, usbh_cdc_t_info, dev);

    USBH_NonStandard_CDC_Transmit(usbh_cdc->phost, (uint8_t *)buffer, size);
    
    return (ret == HAL_OK) ? size : 0;
} 


static os_size_t usbh_cdc_start_recv(struct os_device *dev, os_off_t pos, void *buffer, os_size_t size)
{
    HAL_StatusTypeDef  ret = HAL_OK;
    usbh_cdc_t_info *usbh_cdc;

    OS_ASSERT(dev != OS_NULL);

    usbh_cdc = os_container_of(dev, usbh_cdc_t_info, dev);

    USBH_NonStandard_CDC_Receive(usbh_cdc->phost, buffer, size);

    return (ret == HAL_OK) ? size : 0;
}


static os_err_t usbh_cdc_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
    OS_ASSERT(dev != OS_NULL);

    usbh_cdc_t_info *usbh_cdc;
    
    usbh_cdc = os_container_of(dev, usbh_cdc_t_info, dev);
    
    switch (cmd)
    {
    case OS_DEVICE_CTRL_RTC_GET_TIME:
        
        result = OS_EOK;
        break;
    }

    return result;
}


const static struct os_device_ops usbh_cdc_ops = {
    .init    = usbh_cdc_init,
    .deinit  = usbh_cdc_deinit,
    .read    = usbh_cdc_start_recv,
    .write   = usbh_cdc_start_send,
    .control = usbh_cdc_control,
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
    

    usbh_cdc_t_info *usbh_cdc = os_calloc(1, sizeof(usbh_cdc_t_info));

    OS_ASSERT(usbh_cdc);

    usbh_cdc->phost = phost;
    usbh_cdc->hhcd = (HCD_HandleTypeDef *)phost->pData;


    level = os_irq_lock();
    os_list_add(&usbh_cdc_list, &usbh_cdc->list);
    os_irq_unlock(level);

    usbh_cdc->dev.type = OS_DEVICE_TYPE_USBDEVICE;
    usbh_cdc->dev.ops  = &usbh_cdc_ops;
    
    return os_device_register(&usbh_cdc->dev, dev_name);

}

static int usbh_cdc_unregister(USBH_HandleTypeDef *phost)
{
    os_err_t    result  = 0;
    
    usbh_cdc_t_info *usbh_cdc;
    
    os_list_for_each_entry(usbh_cdc, &usbh_cdc_list, usbh_cdc_t_info, list)
    {
        if (usbh_cdc->phost == phost)
        {
//            (usbh_cdc->dev);
            USBH_DbgLog("usbh_cdc device unregister");
            result = os_device_unregister(&usbh_cdc->dev);
            os_list_del(&usbh_cdc->list);
            os_free(usbh_cdc);
            break;
        }
    }
    
    return result;
    
}

#endif

/**
  * @brief  USBH_NonStandard_CDC_InterfaceInit
  *         The function init the NonStandard_CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_NonStandard_CDC_InterfaceInit(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status;
  uint8_t interface = 0xFFU;
    uint8_t idx = 0U;
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle;

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
    USBH_DbgLog("Cannot Find the interface for Communication Interface Class.", phost->pActiveClass->Name);
    return USBH_FAIL;
  }

  status = USBH_SelectInterface(phost, interface);

  if (status != USBH_OK)
  {
    return USBH_FAIL;
  }

  phost->pActiveClass->pData = (NonStandard_CDC_HandleTypeDef *)USBH_malloc(sizeof(NonStandard_CDC_HandleTypeDef));
  NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (NonStandard_CDC_Handle == NULL)
  {
    USBH_DbgLog("Cannot allocate memory for NonStandard_NonStandard_CDC Handle");
    return USBH_FAIL;
  }

  /* Initialize NonStandard_CDC handler */
  USBH_memset(NonStandard_CDC_Handle, 0, sizeof(NonStandard_CDC_HandleTypeDef));

  /*Collect the notification endpoint address and length*/
  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U)
  {
    NonStandard_CDC_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    NonStandard_CDC_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }

  /*Allocate the length for host channel number in*/
  NonStandard_CDC_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, NonStandard_CDC_Handle->CommItf.NotifEp);

  /* Open pipe for Notification endpoint */
  USBH_OpenPipe(phost, NonStandard_CDC_Handle->CommItf.NotifPipe, NonStandard_CDC_Handle->CommItf.NotifEp,
                phost->device.address, phost->device.speed, USB_EP_TYPE_INTR,
                NonStandard_CDC_Handle->CommItf.NotifEpSize);

  phost->ll_ops->LL_SetToggle(phost, NonStandard_CDC_Handle->CommItf.NotifPipe, 0U);

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
    NonStandard_CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    NonStandard_CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }
  else
  {
    NonStandard_CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    NonStandard_CDC_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }

  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80U)
  {
    NonStandard_CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    NonStandard_CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  }
  else
  {
    NonStandard_CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    NonStandard_CDC_Handle->DataItf.OutEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  }
  
  if (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress & 0x80U)
  {
    NonStandard_CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress;
    NonStandard_CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].wMaxPacketSize;
  }
  else
  {
    NonStandard_CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].bEndpointAddress;
    NonStandard_CDC_Handle->DataItf.OutEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[2].wMaxPacketSize;
  }

  /*Allocate the length for host channel number out*/
  NonStandard_CDC_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, NonStandard_CDC_Handle->DataItf.OutEp);

  /*Allocate the length for host channel number in*/
  NonStandard_CDC_Handle->DataItf.InPipe = USBH_AllocPipe(phost, NonStandard_CDC_Handle->DataItf.InEp);

  /* Open channel for OUT endpoint */
  USBH_OpenPipe(phost, NonStandard_CDC_Handle->DataItf.OutPipe, NonStandard_CDC_Handle->DataItf.OutEp,
                phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                NonStandard_CDC_Handle->DataItf.OutEpSize);

  /* Open channel for IN endpoint */
  USBH_OpenPipe(phost, NonStandard_CDC_Handle->DataItf.InPipe, NonStandard_CDC_Handle->DataItf.InEp,
                phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                NonStandard_CDC_Handle->DataItf.InEpSize);

  NonStandard_CDC_Handle->state = NONSTANDARD_CDC_IDLE_STATE;

  phost->ll_ops->LL_SetToggle(phost, NonStandard_CDC_Handle->DataItf.OutPipe, 0U);
  phost->ll_ops->LL_SetToggle(phost, NonStandard_CDC_Handle->DataItf.InPipe, 0U);

    usbh_cdc_register(phost);

  return USBH_OK;
}



/**
  * @brief  USBH_NonStandard_CDC_InterfaceDeInit
  *         The function DeInit the Pipes used for the NonStandard_NonStandard_CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_NonStandard_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (NonStandard_CDC_Handle->CommItf.NotifPipe)
  {
    USBH_ClosePipe(phost, NonStandard_CDC_Handle->CommItf.NotifPipe);
    USBH_FreePipe(phost, NonStandard_CDC_Handle->CommItf.NotifPipe);
    NonStandard_CDC_Handle->CommItf.NotifPipe = 0U;     /* Reset the Channel as Free */
  }

  if (NonStandard_CDC_Handle->DataItf.InPipe)
  {
    USBH_ClosePipe(phost, NonStandard_CDC_Handle->DataItf.InPipe);
    USBH_FreePipe(phost, NonStandard_CDC_Handle->DataItf.InPipe);
    NonStandard_CDC_Handle->DataItf.InPipe = 0U;     /* Reset the Channel as Free */
  }

  if (NonStandard_CDC_Handle->DataItf.OutPipe)
  {
    USBH_ClosePipe(phost, NonStandard_CDC_Handle->DataItf.OutPipe);
    USBH_FreePipe(phost, NonStandard_CDC_Handle->DataItf.OutPipe);
    NonStandard_CDC_Handle->DataItf.OutPipe = 0U;    /* Reset the Channel as Free */
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
  * @brief  USBH_NonStandard_CDC_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for NonStandard_NonStandard_CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_NonStandard_CDC_ClassRequest(USBH_HandleTypeDef *phost)
{
    #if 0
  USBH_StatusTypeDef status;
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  /* Issue the get line coding request */
  status = GetLineCoding(phost, &NonStandard_CDC_Handle->LineCoding);
  if (status == USBH_OK)
  {
    phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
  }
  else if (status == USBH_NOT_SUPPORTED)
  {
    USBH_ErrLog("Control error: NonStandard_NonStandard_CDC: Device Get Line Coding configuration failed");
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
  * @brief  USBH_NonStandard_CDC_Process
  *         The function is for managing state machine for NonStandard_NonStandard_CDC data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_NonStandard_CDC_Process(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef req_status = USBH_OK;
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  switch (NonStandard_CDC_Handle->state)
  {

    case NONSTANDARD_CDC_IDLE_STATE:
      status = USBH_OK;
      break;

    case NONSTANDARD_CDC_SET_LINE_CODING_STATE:
      req_status = SetLineCoding(phost, NonStandard_CDC_Handle->pUserLineCoding);

      if (req_status == USBH_OK)
      {
        NonStandard_CDC_Handle->state = NONSTANDARD_CDC_GET_LAST_LINE_CODING_STATE;
      }

      else
      {
        if (req_status != USBH_BUSY)
        {
          NonStandard_CDC_Handle->state = NONSTANDARD_CDC_ERROR_STATE;
        }
      }
      break;


    case NONSTANDARD_CDC_GET_LAST_LINE_CODING_STATE:
      req_status = GetLineCoding(phost, &(NonStandard_CDC_Handle->LineCoding));

      if (req_status == USBH_OK)
      {
        NonStandard_CDC_Handle->state = NONSTANDARD_CDC_IDLE_STATE;

        if ((NonStandard_CDC_Handle->LineCoding.b.bCharFormat == NonStandard_CDC_Handle->pUserLineCoding->b.bCharFormat) &&
            (NonStandard_CDC_Handle->LineCoding.b.bDataBits == NonStandard_CDC_Handle->pUserLineCoding->b.bDataBits) &&
            (NonStandard_CDC_Handle->LineCoding.b.bParityType == NonStandard_CDC_Handle->pUserLineCoding->b.bParityType) &&
            (NonStandard_CDC_Handle->LineCoding.b.dwDTERate == NonStandard_CDC_Handle->pUserLineCoding->b.dwDTERate))
        {
          USBH_NonStandard_CDC_LineCodingChanged(phost);
        }
      }
      else
      {
        if (req_status != USBH_BUSY)
        {
          NonStandard_CDC_Handle->state = NONSTANDARD_CDC_ERROR_STATE;
        }
      }
      break;

    case NONSTANDARD_CDC_TRANSFER_DATA:
      NonStandard_CDC_ProcessTransmission(phost);
      NonStandard_CDC_ProcessReception(phost);
      break;

    case NONSTANDARD_CDC_ERROR_STATE:
      req_status = USBH_ClrFeature(phost, 0x00U);

      if (req_status == USBH_OK)
      {
        /*Change the state to waiting*/
        NonStandard_CDC_Handle->state = NONSTANDARD_CDC_IDLE_STATE;
      }
      break;

    default:
      break;

  }

  return status;
}

/**
  * @brief  USBH_NonStandard_CDC_SOFProcess
  *         The function is for managing SOF callback
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_NonStandard_CDC_SOFProcess(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);

  return USBH_OK;
}


/**
  * @brief  USBH_NonStandard_CDC_Stop
  *         Stop current NonStandard_NonStandard_CDC Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef  USBH_NonStandard_CDC_Stop(USBH_HandleTypeDef *phost)
{
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    NonStandard_CDC_Handle->state = NONSTANDARD_CDC_IDLE_STATE;

    USBH_ClosePipe(phost, NonStandard_CDC_Handle->CommItf.NotifPipe);
    USBH_ClosePipe(phost, NonStandard_CDC_Handle->DataItf.InPipe);
    USBH_ClosePipe(phost, NonStandard_CDC_Handle->DataItf.OutPipe);
  }
  return USBH_OK;
}
/**
  * @brief  This request allows the host to find out the currently
  *         configured line coding.
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost, NonStandard_CDC_LineCodingTypeDef *linecoding)
{

  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS | \
                                         USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = NONSTANDARD_CDC_GET_LINE_CODING;
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
                                        NonStandard_CDC_LineCodingTypeDef *linecoding)
{
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS |
                                         USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = NONSTANDARD_CDC_SET_LINE_CODING;
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
USBH_StatusTypeDef USBH_NonStandard_CDC_SetLineCoding(USBH_HandleTypeDef *phost,
                                          NonStandard_CDC_LineCodingTypeDef *linecoding)
{
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    NonStandard_CDC_Handle->state = NONSTANDARD_CDC_SET_LINE_CODING_STATE;
    NonStandard_CDC_Handle->pUserLineCoding = linecoding;

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
USBH_StatusTypeDef  USBH_NonStandard_CDC_GetLineCoding(USBH_HandleTypeDef *phost,
                                           NonStandard_CDC_LineCodingTypeDef *linecoding)
{
    #if 0
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((phost->gState == HOST_CLASS) || (phost->gState == HOST_CLASS_REQUEST))
  {
    *linecoding = NonStandard_CDC_Handle->LineCoding;
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
uint16_t USBH_NonStandard_CDC_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
  uint32_t dataSize;
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    dataSize = phost->ll_ops->LL_GetLastXferSize(phost, NonStandard_CDC_Handle->DataItf.InPipe);
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
USBH_StatusTypeDef  USBH_NonStandard_CDC_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((NonStandard_CDC_Handle->state == NONSTANDARD_CDC_IDLE_STATE) || (NonStandard_CDC_Handle->state == NONSTANDARD_CDC_TRANSFER_DATA))
  {
    NonStandard_CDC_Handle->pTxData = pbuff;
    NonStandard_CDC_Handle->TxDataLength = length;
    NonStandard_CDC_Handle->state = NONSTANDARD_CDC_TRANSFER_DATA;
    NonStandard_CDC_Handle->data_tx_state = NONSTANDARD_CDC_SEND_DATA;
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
USBH_StatusTypeDef  USBH_NonStandard_CDC_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((NonStandard_CDC_Handle->state == NONSTANDARD_CDC_IDLE_STATE) || (NonStandard_CDC_Handle->state == NONSTANDARD_CDC_TRANSFER_DATA))
  {
    NonStandard_CDC_Handle->pRxData = pbuff;
    NonStandard_CDC_Handle->RxDataLength = length;
    NonStandard_CDC_Handle->state = NONSTANDARD_CDC_TRANSFER_DATA;
    NonStandard_CDC_Handle->data_rx_state = NONSTANDARD_CDC_RECEIVE_DATA;
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
static void NonStandard_CDC_ProcessTransmission(USBH_HandleTypeDef *phost)
{
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

  switch (NonStandard_CDC_Handle->data_tx_state)
  {
    case NONSTANDARD_CDC_SEND_DATA:
      if (NonStandard_CDC_Handle->TxDataLength > NonStandard_CDC_Handle->DataItf.OutEpSize)
      {
        USBH_BulkSendData(phost,
                          NonStandard_CDC_Handle->pTxData,
                          NonStandard_CDC_Handle->DataItf.OutEpSize,
                          NonStandard_CDC_Handle->DataItf.OutPipe,
                          1U);
      }
      else
      {
        USBH_BulkSendData(phost,
                          NonStandard_CDC_Handle->pTxData,
                          (uint16_t)NonStandard_CDC_Handle->TxDataLength,
                          NonStandard_CDC_Handle->DataItf.OutPipe,
                          1U);
      }

      NonStandard_CDC_Handle->data_tx_state = NONSTANDARD_CDC_SEND_DATA_WAIT;
      break;

    case NONSTANDARD_CDC_SEND_DATA_WAIT:

      URB_Status = phost->ll_ops->LL_GetURBState(phost, NonStandard_CDC_Handle->DataItf.OutPipe);

      /* Check the status done for transmission */
      if (URB_Status == USBH_URB_DONE)
      {
        if (NonStandard_CDC_Handle->TxDataLength > NonStandard_CDC_Handle->DataItf.OutEpSize)
        {
          NonStandard_CDC_Handle->TxDataLength -= NonStandard_CDC_Handle->DataItf.OutEpSize;
          NonStandard_CDC_Handle->pTxData += NonStandard_CDC_Handle->DataItf.OutEpSize;
        }
        else
        {
          NonStandard_CDC_Handle->TxDataLength = 0U;
        }

        if (NonStandard_CDC_Handle->TxDataLength > 0U)
        {
          NonStandard_CDC_Handle->data_tx_state = NONSTANDARD_CDC_SEND_DATA;
        }
        else
        {
          NonStandard_CDC_Handle->data_tx_state = NONSTANDARD_CDC_IDLE;
          USBH_NonStandard_CDC_TransmitCallback(phost);
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
          NonStandard_CDC_Handle->data_tx_state = NONSTANDARD_CDC_SEND_DATA;

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

static void NonStandard_CDC_ProcessReception(USBH_HandleTypeDef *phost)
{
  NonStandard_CDC_HandleTypeDef *NonStandard_CDC_Handle = (NonStandard_CDC_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  uint32_t length;

  switch (NonStandard_CDC_Handle->data_rx_state)
  {

    case NONSTANDARD_CDC_RECEIVE_DATA:

      USBH_BulkReceiveData(phost,
                           NonStandard_CDC_Handle->pRxData,
                           NonStandard_CDC_Handle->DataItf.InEpSize,
                           NonStandard_CDC_Handle->DataItf.InPipe);

      NonStandard_CDC_Handle->data_rx_state = NONSTANDARD_CDC_RECEIVE_DATA_WAIT;

      break;

    case NONSTANDARD_CDC_RECEIVE_DATA_WAIT:

      URB_Status = phost->ll_ops->LL_GetURBState(phost, NonStandard_CDC_Handle->DataItf.InPipe);

      /*Check the status done for reception*/
      if (URB_Status == USBH_URB_DONE)
      {
        length = phost->ll_ops->LL_GetLastXferSize(phost, NonStandard_CDC_Handle->DataItf.InPipe);

        if (((NonStandard_CDC_Handle->RxDataLength - length) > 0U) && (length > NonStandard_CDC_Handle->DataItf.InEpSize))
        {
          NonStandard_CDC_Handle->RxDataLength -= length ;
          NonStandard_CDC_Handle->pRxData += length;
          NonStandard_CDC_Handle->data_rx_state = NONSTANDARD_CDC_RECEIVE_DATA;
        }
        else
        {
          NonStandard_CDC_Handle->data_rx_state = NONSTANDARD_CDC_IDLE;
          USBH_NonStandard_CDC_ReceiveCallback(phost);
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
__weak void USBH_NonStandard_CDC_TransmitCallback(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    USBH_DbgLog("Transmit Finished");

    usbh_cdc_t * usbh_cdc;
    
    os_list_for_each_entry(usbh_cdc, &usbh_cdc_list, usbh_cdc_t, list)
    {
        if (usbh_cdc->phost == phost)
        {
            usbh_cdc->tx_size = 0;
            os_hw_serial_isr_txdone((struct os_serial_device *)usbh_cdc);
            break;
        }
    }
}

/**
* @brief  The function informs user that data have been sent
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_NonStandard_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
    /* Prevent unused argument(s) compilation warning */
    USBH_DbgLog("Receive Finished");

    usbh_cdc_t * usbh_cdc;
    
    os_list_for_each_entry(usbh_cdc, &usbh_cdc_list, usbh_cdc_t, list)
    {
        if (usbh_cdc->phost == phost)
        {
            usbh_cdc->rx_index = USBH_NonStandard_CDC_GetLastReceivedDataSize(phost);
//            os_hw_serial_isr_rxdone((struct os_serial_device *)usbh_cdc);
            /* soft dma idle irq */
            soft_dma_timeout_irq(&usbh_cdc->sdma);
            break;
        }
    }
}

/**
* @brief  The function informs user that Settings have been changed
*  @param  pdev: Selected device
* @retval None
*/
__weak void USBH_NonStandard_CDC_LineCodingChanged(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);
}

static os_uint32_t usbh_cdc_sdma_int_get_index(soft_dma_t *dma)
{
    usbh_cdc_t *usbh_cdc = os_container_of(dma, usbh_cdc_t, sdma);
    
    return usbh_cdc->rx_index;
}

static os_err_t usbh_cdc_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    usbh_cdc_t *usbh_cdc = os_container_of(dma, usbh_cdc_t, sdma);

    usbh_cdc->rx_buff  = buff;
    usbh_cdc->rx_index = 0;
    usbh_cdc->rx_size  = size;

    USBH_NonStandard_CDC_Receive(usbh_cdc->phost, buff, size);
    
    return OS_EOK;
}

static os_uint32_t usbh_cdc_sdma_int_stop(soft_dma_t *dma)
{
    usbh_cdc_t *usbh_cdc = os_container_of(dma, usbh_cdc_t, sdma);

//    usbh_cdc_interrupt_disable(usbh_cdc->info->husbh_cdc, USART_INT_RBNE);
//    usbh_cdc_interrupt_disable(usbh_cdc->info->husbh_cdc, USART_INT_IDLE);
    
    return usbh_cdc_sdma_int_get_index(dma);
}

/* sdma callback */
static void usbh_cdc_sdma_callback(soft_dma_t *dma)
{
    usbh_cdc_t *usbh_cdc = os_container_of(dma, usbh_cdc_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)usbh_cdc);
}

static void usbh_cdc_sdma_init(usbh_cdc_t *usbh_cdc, dma_ring_t *ring)
{
    soft_dma_t *dma = &usbh_cdc->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));


    dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(115200);
    dma->hard_info.flag         = HARD_DMA_FLAG_TIMEOUT_IRQ;

    dma->ops.get_index          = usbh_cdc_sdma_int_get_index;
    dma->ops.dma_init           = OS_NULL;
    dma->ops.dma_start          = usbh_cdc_sdma_int_start;
    dma->ops.dma_stop           = usbh_cdc_sdma_int_stop;



    dma->cbs.dma_half_callback      = usbh_cdc_sdma_callback;
    dma->cbs.dma_full_callback      = usbh_cdc_sdma_callback;
    dma->cbs.dma_timeout_callback   = usbh_cdc_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&usbh_cdc->sdma, OS_TRUE);
}

//static int __usbh_cdc_init(const usbh_cdc_t_info *usbh_cdc_info, struct serial_configure *cfg)
//{
//    return OS_EOK;       
//}

static os_err_t usbh_cdc_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    usbh_cdc_t *usbh_cdc;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    usbh_cdc = os_container_of(serial, usbh_cdc_t, serial);

//    __usbh_cdc_init(usbh_cdc->info, cfg);

    /* SOFT DMA Init */
    usbh_cdc_sdma_init(usbh_cdc, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t usbh_cdc_deinit(struct os_serial_device *serial)
{
    usbh_cdc_t *usbh_cdc;

    OS_ASSERT(serial != OS_NULL);

    usbh_cdc = os_container_of(serial, usbh_cdc_t, serial);

    /* rx */
//    usbh_cdc_interrupt_disable(usbh_cdc->info->husbh_cdc, USART_INT_RBNE);
//    usbh_cdc_interrupt_disable(usbh_cdc->info->husbh_cdc, USART_INT_IDLE);    

    /* usbh_cdc dma deinit */
//    if (usbh_cdc->info->dma_periph != OS_NULL)
//    {
//        /* soft dma deinit */
//        soft_dma_stop(&usbh_cdc->sdma);
//        
//        /* hard dma deinit */
//        dma_deinit((uint32_t)usbh_cdc->info->dma_periph, usbh_cdc->info->dma_channel);
//    }

    /* tx */
//    usbh_cdc_interrupt_disable(usbh_cdc->info->husbh_cdc, USART_INT_TBE);
    
    usbh_cdc->tx_buff  = OS_NULL;
    usbh_cdc->tx_count = 0;
    usbh_cdc->tx_size  = 0;

    return 0;
}

static int usbh_cdc_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    usbh_cdc_t *usbh_cdc;

    HAL_StatusTypeDef  ret = HAL_OK;
    
    OS_ASSERT(serial != OS_NULL);

    usbh_cdc = os_container_of(serial, usbh_cdc_t, serial);

    usbh_cdc->tx_buff  = buff;
    usbh_cdc->tx_count = 0;
    usbh_cdc->tx_size  = size;

    USBH_NonStandard_CDC_Transmit(usbh_cdc->phost, (uint8_t *)buff, size);
    
    return (ret == HAL_OK) ? size : 0;
}

static int usbh_cdc_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    usbh_cdc_t *usbh_cdc;
    HAL_StatusTypeDef  ret;
    
    OS_ASSERT(serial != OS_NULL);

    usbh_cdc = os_container_of(serial, usbh_cdc_t, serial);

    usbh_cdc->tx_buff  = buff;
    usbh_cdc->tx_count = 0;
    usbh_cdc->tx_size  = size;
    
    USBH_NonStandard_CDC_Transmit(usbh_cdc->phost, (uint8_t *)buff, size);
    
    return (ret == HAL_OK) ? size : 0;
}

static const struct os_uart_ops usbh_cdc_ops = {
    .init         = usbh_cdc_init,
    .deinit       = usbh_cdc_deinit,
    
    .start_send   = usbh_cdc_start_send,
    .poll_send    = usbh_cdc_poll_send,
};


#if 0

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
    

    usbh_cdc_t_info *usbh_cdc = os_calloc(1, sizeof(usbh_cdc_t_info));

    OS_ASSERT(usbh_cdc);

    usbh_cdc->phost = phost;
    usbh_cdc->hhcd = (HCD_HandleTypeDef *)phost->pData;


    level = os_irq_lock();
    os_list_add(&usbh_cdc_list, &usbh_cdc->list);
    os_irq_unlock(level);

    usbh_cdc->dev.type = OS_DEVICE_TYPE_USBDEVICE;
    usbh_cdc->dev.ops  = &usbh_cdc_ops;
    
    return os_device_register(&usbh_cdc->dev, dev_name);

}

#endif

static int usbh_cdc_register(USBH_HandleTypeDef *phost)
{   
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t    result  = 0;
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
    
    usbh_cdc_t *usbh_cdc = os_calloc(1, sizeof(usbh_cdc_t));

    OS_ASSERT(usbh_cdc);

    usbh_cdc->phost = phost;
    usbh_cdc->hhcd = (HCD_HandleTypeDef *)phost->pData;
    
    struct os_serial_device *serial = &usbh_cdc->serial;

    serial->ops    = &usbh_cdc_ops;
    serial->config = config;

    level = os_irq_lock();
    os_list_add_tail(&usbh_cdc_list, &usbh_cdc->list);
    os_irq_unlock(level);

    result = os_hw_serial_register(serial, dev_name, NULL);

    OS_ASSERT(result == OS_EOK);

    return result;    
}

static int usbh_cdc_unregister(USBH_HandleTypeDef *phost)
{
    os_err_t    result  = 0;
    
    usbh_cdc_t *usbh_cdc;
    
    os_list_for_each_entry(usbh_cdc, &usbh_cdc_list, usbh_cdc_t, list)
    {
        if (usbh_cdc->phost == phost)
        {
//            (usbh_cdc->dev);
            USBH_DbgLog("usbh_cdc device unregister");
            result = os_hw_serial_unregister(&usbh_cdc->serial);
            os_list_del(&usbh_cdc->list);
            os_free(usbh_cdc);
            break;
        }
    }
    
    return result;
    
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
