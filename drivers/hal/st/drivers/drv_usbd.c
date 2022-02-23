/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        drv_usbd.c
 *
 * @brief       This file implements USB driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include "board.h"

#include "oneos_config.h"
#include "proto_usbd_def.h"
#include "proto_usbd_core.h"
#include <os_mb.h>

#ifdef USB_USING_DEVICE_HS
#include "usbd_desc_hs.h"
#endif

#ifdef USB_USING_DEVICE_FS
#include "usbd_desc_fs.h"
#endif

#if (defined(OS_USBD_CDC_CLASS_HS) || defined(OS_USBD_CDC_CLASS_FS))
#include "usbd_cdc.h"
#elif (defined(OS_USBD_PRINTER_CLASS_HS) || defined(OS_USBD_PRINTER_CLASS_FS))
#include "usbd_printer.h"
#endif


USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status);

/**
  * @brief  Setup stage callback
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Data In stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;

  if ( hpcd->Init.speed == PCD_SPEED_FULL)
  {
    speed = USBD_SPEED_FULL;
  }
#ifdef PCD_SPEED_HIGH
  else if ( hpcd->Init.speed == PCD_SPEED_HIGH)
  {
    speed = USBD_SPEED_HIGH;
  }
#endif
  else
  {
    Error_Handler();
  }
    /* Set Speed. */
  USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);

  /* Reset Device. */
  USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Suspend callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
    USBD_HandleTypeDef *pdev = (USBD_HandleTypeDef*)hpcd->pData;
  /* Inform USB library that core enters in suspend Mode. */
  USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
  __HAL_PCD_GATE_PHYCLOCK(hpcd);
    
    pdev->hotplus_status = USBD_PLUG_OUT;
    if(OS_EOK != os_mb_send(pdev->usbd_mb, (os_ubase_t)pdev, OS_NO_WAIT))
    {
        LOG_W("cdc", "mailbox send failed");
    }
    
  /* Enter in STOP mode. */
  /* USER CODE BEGIN 2 */
  if (hpcd->Init.low_power_enable)
  {
    /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register. */
    SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
  }
  /* USER CODE END 2 */
}

/**
  * @brief  Resume callback.
  * When Low power mode is enabled the debug cannot be used (IAR, Keil doesn't support it)
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */
  USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#else
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
static void PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#else
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */
{
  USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
#if (USE_HAL_PCD_REGISTER_CALLBACKS == 1U)
    /* Register USB PCD CallBacks */
    HAL_PCD_RegisterCallback(pdev->pData, HAL_PCD_SOF_CB_ID, PCD_SOFCallback);
    HAL_PCD_RegisterCallback(pdev->pData, HAL_PCD_SETUPSTAGE_CB_ID, PCD_SetupStageCallback);
    HAL_PCD_RegisterCallback(pdev->pData, HAL_PCD_RESET_CB_ID, PCD_ResetCallback);
    HAL_PCD_RegisterCallback(pdev->pData, HAL_PCD_SUSPEND_CB_ID, PCD_SuspendCallback);
    HAL_PCD_RegisterCallback(pdev->pData, HAL_PCD_RESUME_CB_ID, PCD_ResumeCallback);
    HAL_PCD_RegisterCallback(pdev->pData, HAL_PCD_CONNECT_CB_ID, PCD_ConnectCallback);
    HAL_PCD_RegisterCallback(pdev->pData, HAL_PCD_DISCONNECT_CB_ID, PCD_DisconnectCallback);

    HAL_PCD_RegisterDataOutStageCallback(pdev->pData, PCD_DataOutStageCallback);
    HAL_PCD_RegisterDataInStageCallback(pdev->pData, PCD_DataInStageCallback);
    HAL_PCD_RegisterIsoOutIncpltCallback(pdev->pData, PCD_ISOOUTIncompleteCallback);
    HAL_PCD_RegisterIsoInIncpltCallback(pdev->pData, PCD_ISOINIncompleteCallback);
#endif /* USE_HAL_PCD_REGISTER_CALLBACKS */

    if (pdev->id == DEVICE_HS) 
    {
        HAL_PCDEx_SetRxFiFo(pdev->pData, 0x200);
        HAL_PCDEx_SetTxFiFo(pdev->pData, 0, 0x80);
        HAL_PCDEx_SetTxFiFo(pdev->pData, 1, 0x174);
    }
    else
    {
        HAL_PCDEx_SetRxFiFo(pdev->pData, 0x80);
        HAL_PCDEx_SetTxFiFo(pdev->pData, 0, 0x40);
        HAL_PCDEx_SetTxFiFo(pdev->pData, 1, 0x80);
    }
    return USBD_OK;
}

/**
  * @brief  De-Initializes the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_DeInit(pdev->pData);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Starts the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_Start(pdev->pData);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Stops the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_Stop(pdev->pData);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Opens an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  ep_type: Endpoint type
  * @param  ep_mps: Endpoint max packet size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Closes an endpoint of the low level driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Flushes an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Sets a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Clears a Stall condition on an endpoint of the Low Level Driver.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Returns Stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Stall (1: Yes, 0: No)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;

  if((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assigns a USB address to the device.
  * @param  pdev: Device handle
  * @param  dev_addr: Device address
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Transmits data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be sent
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Prepares an endpoint for reception.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data to be received
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;

  hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);

  usb_status =  USBD_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Returns the last transferred packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Received Data Size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

/**
  * @brief  Returns max packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Received Data Size
  */
uint32_t USBD_LL_GetEPMaxPacket(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    PCD_HandleTypeDef *hpcd = pdev->pData;
    
    if (hpcd != NULL)
    {
        return hpcd->IN_ep[ep_addr].maxpacket;
    }
    else
    {
        return 0;
    }
}

/**
  * @brief  Delays routine for the USB device library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/**
  * @brief  Returns the USB status depending on the HAL status:
  * @param  hal_status: HAL status
  * @retval USB status
  */
USBD_StatusTypeDef USBD_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
  USBD_StatusTypeDef usb_status = USBD_OK;

  switch (hal_status)
  {
    case HAL_OK :
      usb_status = USBD_OK;
    break;
    case HAL_ERROR :
      usb_status = USBD_FAIL;
    break;
    case HAL_BUSY :
      usb_status = USBD_BUSY;
    break;
    case HAL_TIMEOUT :
      usb_status = USBD_FAIL;
    break;
    default :
      usb_status = USBD_FAIL;
    break;
  }
  return usb_status;
}
#include <board.h>
#include "drv_common.h"

struct stm32_usbd
{
    USBD_HandleTypeDef husbd;
    
    os_list_node_t list;
    
    uint8_t interface_type;
};


static os_list_node_t stm32_usbd_list = OS_LIST_INIT(stm32_usbd_list);


USBD_LL_Ops usbd_ops = 
{
    USBD_LL_Init          ,
    USBD_LL_DeInit        ,
    USBD_LL_Start         ,
    USBD_LL_Stop          ,
    USBD_LL_OpenEP        ,
    USBD_LL_CloseEP       ,
    USBD_LL_FlushEP       ,
    USBD_LL_StallEP       ,
    USBD_LL_ClearStallEP  ,
    USBD_LL_IsStallEP     ,
    USBD_LL_SetUSBAddress ,
    USBD_LL_Transmit      ,
    USBD_LL_PrepareReceive,
    USBD_LL_GetRxDataSize ,
    USBD_LL_GetEPMaxPacket,
    USBD_LL_Delay         ,
};

/**
  * Init USB device Library, add supported class and start the library
  * @retval None
  */

void MX_USB_DEVICE_Init(USBD_HandleTypeDef *pdev, uint8_t id)
{
    USBD_ClassTypeDef *Register_Class = NULL;
    
    /* Init Device Library, add supported class and start the library. */
#ifdef USB_USING_DEVICE_HS
    if (id == PCD_USB_OTG_HS)
    {
        if (USBD_Init(pdev, &HS_Desc, id) != USBD_OK)
        {
            Error_Handler();
        }
#ifdef OS_USBD_CDC_CLASS_HS
        Register_Class = &USBD_CDC;
#endif
        
#ifdef OS_USBD_PRINTER_CLASS_HS
        Register_Class = &USBD_Printer;
#endif
    }

#endif
    
#ifdef USB_USING_DEVICE_FS
    if (id == PCD_USB_OTG_FS)
    {
        if (USBD_Init(pdev, &FS_Desc, id) != USBD_OK)
        {
            Error_Handler();
        }
#ifdef OS_USBD_CDC_CLASS_FS
        Register_Class = &USBD_CDC;
#endif
        
#ifdef OS_USBD_PRINTER_CLASS_FS
        Register_Class = &USBD_PRINTER;
#endif
    }
#endif
    if (Register_Class == NULL)
    {
        Error_Handler();
    }
    if (USBD_RegisterClass(pdev, Register_Class) != USBD_OK)
    {
        Error_Handler();
    }

#if defined(STM32H747xx)
    HAL_PWREx_EnableUSBVoltageDetector();
#endif
}







#define MB_MAX_MAILS    10
#define MB_POLL_SIZE    (MB_MAX_MAILS * sizeof(os_uint32_t))

static char usbd_hotplug_mb_pool[MB_POLL_SIZE];
static os_mb_t usbd_hotplug_mb;

static void usbd_hotplug_detect(void *parameter)
{
    USBD_HandleTypeDef * pdev;
    os_ubase_t mb_value;
    while(1)
    {
        if (OS_EOK == os_mb_recv(&usbd_hotplug_mb, &mb_value, OS_WAIT_FOREVER))
        {
            pdev = (USBD_HandleTypeDef *)mb_value;
            
            if(pdev->hotplus_status == USBD_PLUG_IN)
            {
                if (pdev->user_ops != NULL)
                {
                    pdev->user_ops->Register(pdev);
                }
            }
            else if (pdev->hotplus_status == USBD_PLUG_OUT)
            {
                /* unregister device */
                if (pdev->user_ops != NULL)
                {
                    pdev->user_ops->Unregister(pdev);
                }
            }
        }
    }

}

int usbd_dectect_task_create(USBD_HandleTypeDef *pdev)
{
    os_task_t *task;
    
    //create mailbox queue
    if(OS_EOK != os_mb_init(&usbd_hotplug_mb, "mailbox_static", &usbd_hotplug_mb_pool[0], MB_POLL_SIZE))
    {
        LOG_W("USBD", "msgqueue_static_sample msgqueue init ERR");
        return OS_ERROR;
    }

    pdev->usbd_mb = &usbd_hotplug_mb;
    
    task = os_task_create("usbd_hotplug_detect", usbd_hotplug_detect, NULL, 1024, 3);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}


static int stm32_usbd_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    void * hclass = NULL;    
    
    struct stm32_usbd * usb_device = os_calloc(1, sizeof(struct stm32_usbd));
    
    if (usb_device ==NULL)
    {
        return OS_ERROR;
    }
    
    struct stm32_pcd_info * usbd_info = (struct stm32_pcd_info *)dev->info;

#ifdef USB_USING_DEVICE_HS
    if (strcmp(usbd_info->interface_type, "PCD_USB_OTG_HS") == 0)
    {
    
#if defined(OS_USBD_PRINTER_CLASS_HS)
        hclass =  os_calloc(1, sizeof(USBD_Printer_HandleTypeDef));
#elif defined(OS_USBD_CDC_CLASS_HS)
        hclass =  os_calloc(1, sizeof(USBD_CDC_HandleTypeDef));
#endif
        usb_device->interface_type = PCD_USB_OTG_HS;
    }
#endif

#ifdef USB_USING_DEVICE_FS
    if (strcmp(usbd_info->interface_type, "PCD_USB_OTG_FS") == 0)
    {
    
#if defined(OS_USBD_PRINTER_CLASS_HS)
        hclass =  os_calloc(1, sizeof(USBD_Printer_HandleTypeDef));
#elif defined(OS_USBD_CDC_CLASS_FS)
        hclass =  os_calloc(1, sizeof(USBD_CDC_HandleTypeDef));
#endif
        usb_device->interface_type = PCD_USB_OTG_FS;
    }
#endif
    
    if (hclass == NULL)
    {
       return OS_ERROR;
    }
    
    usb_device->husbd.pClassData = hclass;
    usb_device->husbd.pData = usbd_info->instance;
    usbd_info->instance->pData = &usb_device->husbd;
    usb_device->husbd.ll_ops = &usbd_ops;

    level = os_irq_lock();
    os_list_add_tail(&stm32_usbd_list, &usb_device->list);
    os_irq_unlock(level);
    
    usbd_dectect_task_create(&usb_device->husbd);

    MX_USB_DEVICE_Init(&usb_device->husbd, usb_device->interface_type);


    return OS_EOK;
}

OS_DRIVER_INFO stm32_usbd_driver = {
    .name   = "PCD_HandleTypeDef",
    .probe  = stm32_usbd_probe,
};

OS_DRIVER_DEFINE(stm32_usbd_driver,DEVICE,OS_INIT_SUBLEVEL_LOW);

void usbd_start(USBD_HandleTypeDef *phost)
{
    if (USBD_Start(phost) != USBD_OK)
    {
        Error_Handler();
    }
}

os_err_t usbd_start_init(void)
{
    struct stm32_usbd *usbd;
    
    os_list_for_each_entry(usbd, &stm32_usbd_list, struct stm32_usbd, list)
    {
        usbd_start(&usbd->husbd);
    }
    
    return OS_EOK;
}

OS_APP_INIT(usbd_start_init, OS_INIT_SUBLEVEL_HIGH);

