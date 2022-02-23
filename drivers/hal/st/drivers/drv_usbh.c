#include <board.h>

#include "oneos_config.h"
#include "proto_usbh_core.h"
#include "drv_common.h"
#include "proto_usbh_def.h"


#ifdef ENABLE_USBH_AUDIO_CLASS
#include "usbh_audio.h"
#endif

#ifdef ENABLE_USBH_CDC_CLASS
#include "usbh_cdc.h"
#endif

#ifdef ENABLE_USBH_MSC_CLASS
#include "usbh_msc.h"
#endif

#ifdef ENABLE_USBH_HID_CLASS
#include "usbh_hid.h"
#endif

#ifdef ENABLE_USBH_MTP_CLASS
#include "usbh_mtp.h"
#endif

#ifdef ENABLE_USBH_SPECIFIC_CDC_CLASS
#include "usbh_specific_cdc.h"
#endif


/* Private function prototypes -----------------------------------------------*/
USBH_StatusTypeDef USBH_Get_USB_Status(HAL_StatusTypeDef hal_status);


static os_list_node_t stm32_usbh_list = OS_LIST_INIT(stm32_usbh_list);


/*******************************************************************************
                       LL Driver Callbacks (HCD -> USB Host Library)
*******************************************************************************/

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_IncTimer(hhcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_Connect(hhcd->pData);
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_Disconnect(hhcd->pData);
}

/**
  * @brief  Notify URB state change callback.
  * @param  hhcd: HCD handle
  * @param  chnum: channel number
  * @param  urb_state: state
  * @retval None
  */
void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state)
{
  /* To be used with OS to sync URB state with the global state machine */
#if (USBH_USE_OS == 1)
  USBH_LL_NotifyURBChange(hhcd->pData);
#endif
}
/**
* @brief  Port Port Enabled callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_PortEnabled_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_PortEnabled(hhcd->pData);
}

/**
  * @brief  Port Port Disabled callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
void HAL_HCD_PortDisabled_Callback(HCD_HandleTypeDef *hhcd)
{
  USBH_LL_PortDisabled(hhcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Host Library --> HCD)
*******************************************************************************/

/**
  * @brief  Initialize the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_Init(USBH_HandleTypeDef *phost)
{
    USBH_LL_SetTimer(phost, HAL_HCD_GetCurrentFrame(phost->pData));
    return USBH_OK;
}

/**
  * @brief  De-Initialize the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_DeInit(USBH_HandleTypeDef *phost)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBH_StatusTypeDef usb_status = USBH_OK;

  hal_status = HAL_HCD_DeInit(phost->pData);

  usb_status = USBH_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Start the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_Start(USBH_HandleTypeDef *phost)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBH_StatusTypeDef usb_status = USBH_OK;

  hal_status = HAL_HCD_Start(phost->pData);

  usb_status = USBH_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Stop the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_Stop(USBH_HandleTypeDef *phost)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBH_StatusTypeDef usb_status = USBH_OK;

  hal_status = HAL_HCD_Stop(phost->pData);

  usb_status = USBH_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Return the USB host speed from the low level driver.
  * @param  phost: Host handle
  * @retval USBH speeds
  */
USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef *phost)
{
    USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

    switch (HAL_HCD_GetCurrentSpeed(phost->pData))
    {
        case 0 :
            speed = USBH_SPEED_HIGH;
            USBH_UsrLog("speed: USBH_SPEED_HIGH");
            break;

        case 1 :
            speed = USBH_SPEED_FULL;
            USBH_UsrLog("speed: USBH_SPEED_FULL");
            break;

        case 2 :
            speed = USBH_SPEED_LOW;
            USBH_UsrLog("speed: USBH_SPEED_LOW");
            break;

        default:
            speed = USBH_SPEED_FULL;
            USBH_UsrLog("speed(default): USBH_SPEED_FULL");
            break;
    }
    return  speed;
}

/**
  * @brief  Reset the Host port of the low level driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_ResetPort(USBH_HandleTypeDef *phost)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBH_StatusTypeDef usb_status = USBH_OK;

  hal_status = HAL_HCD_ResetPort(phost->pData);

  usb_status = USBH_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Return the last transfered packet size.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @retval Packet size
  */
uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef *phost, uint8_t pipe)
{
  return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
  * @brief  Open a pipe of the low level driver.
  * @param  phost: Host handle
  * @param  pipe_num: Pipe index
  * @param  epnum: Endpoint number
  * @param  dev_address: Device USB address
  * @param  speed: Device Speed
  * @param  ep_type: Endpoint type
  * @param  mps: Endpoint max packet size
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_OpenPipe(USBH_HandleTypeDef *phost, uint8_t pipe_num, uint8_t epnum,
                                    uint8_t dev_address, uint8_t speed, uint8_t ep_type, uint16_t mps)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBH_StatusTypeDef usb_status = USBH_OK;

  hal_status = HAL_HCD_HC_Init(phost->pData, pipe_num, epnum,
                               dev_address, speed, ep_type, mps);

  usb_status = USBH_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Close a pipe of the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_ClosePipe(USBH_HandleTypeDef *phost, uint8_t pipe)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBH_StatusTypeDef usb_status = USBH_OK;

  hal_status = HAL_HCD_HC_Halt(phost->pData, pipe);

  usb_status = USBH_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  USBH_LL_GetFreePipe
  * @param  phost: Host Handle
  *         Get a free Pipe number for allocation to a device endpoint
  * @retval idx: Free Pipe number
  */
static uint16_t USBH_LL_GetFreePipe(USBH_HandleTypeDef *phost)
{
  uint8_t idx = 0U;

  for (idx = 0U ; idx < 11U ; idx++)
  {
    if ((phost->Pipes[idx] & 0x8000U) == 0U)
    {
      return (uint16_t)idx;
    }
  }

  return 0xFFFFU;
}

/**
  * @brief  USBH_Alloc_Pipe
  *         Allocate a new Pipe
  * @param  phost: Host Handle
  * @param  ep_addr: End point for which the Pipe to be allocated
  * @retval Pipe number
  */
uint8_t USBH_LL_AllocPipe(USBH_HandleTypeDef *phost, uint8_t ep_addr)
{
  uint16_t pipe;

  pipe =  USBH_LL_GetFreePipe(phost);

  if (pipe != 0xFFFFU)
  {
    phost->Pipes[pipe & 0xFU] = 0x8000U | ep_addr;
  }

  return (uint8_t)pipe;
}


/**
  * @brief  USBH_Free_Pipe
  *         Free the USB Pipe
  * @param  phost: Host Handle
  * @param  idx: Pipe number to be freed
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_LL_FreePipe(USBH_HandleTypeDef *phost, uint8_t idx)
{
  if (idx < 11U)
  {
    phost->Pipes[idx] &= 0x7FFFU;
  }

  return USBH_OK;
}




/**
  * @brief  Submit a new URB to the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  *         This parameter can be a value from 1 to 15
  * @param  direction : Channel number
  *          This parameter can be one of the these values:
  *           0 : Output
  *           1 : Input
  * @param  ep_type : Endpoint Type
  *          This parameter can be one of the these values:
  *            @arg EP_TYPE_CTRL: Control type
  *            @arg EP_TYPE_ISOC: Isochrounous type
  *            @arg EP_TYPE_BULK: Bulk type
  *            @arg EP_TYPE_INTR: Interrupt type
  * @param  token : Endpoint Type
  *          This parameter can be one of the these values:
  *            @arg 0: PID_SETUP
  *            @arg 1: PID_DATA
  * @param  pbuff : pointer to URB data
  * @param  length : Length of URB data
  * @param  do_ping : activate do ping protocol (for high speed only)
  *          This parameter can be one of the these values:
  *           0 : do ping inactive
  *           1 : do ping active
  * @retval Status
  */
USBH_StatusTypeDef USBH_LL_SubmitURB(USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t direction,
                                     uint8_t ep_type, uint8_t token, uint8_t *pbuff, uint16_t length,
                                     uint8_t do_ping)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBH_StatusTypeDef usb_status = USBH_OK;

  hal_status = HAL_HCD_HC_SubmitRequest(phost->pData, pipe, direction ,
                                        ep_type, token, pbuff, length,
                                        do_ping);
  usb_status =  USBH_Get_USB_Status(hal_status);

  return usb_status;
}

/**
  * @brief  Get a URB state from the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  *         This parameter can be a value from 1 to 15
  * @retval URB state
  *          This parameter can be one of the these values:
  *            @arg URB_IDLE
  *            @arg URB_DONE
  *            @arg URB_NOTREADY
  *            @arg URB_NYET
  *            @arg URB_ERROR
  *            @arg URB_STALL
  */
USBH_URBStateTypeDef USBH_LL_GetURBState(USBH_HandleTypeDef *phost, uint8_t pipe)
{
  return (USBH_URBStateTypeDef)HAL_HCD_HC_GetURBState (phost->pData, pipe);
}

/**
  * @brief  Drive VBUS.
  * @param  phost: Host handle
  * @param  state : VBUS state
  *          This parameter can be one of the these values:
  *           0 : VBUS Inactive
  *           1 : VBUS Active
  * @retval Status
  */
USBH_StatusTypeDef USBH_LL_DriverVBUS(USBH_HandleTypeDef *phost, uint8_t state)
{

  /* USER CODE BEGIN 0 */

  /* USER CODE END 0*/

  if (phost->id == OTG_FS)
  {
    if (state == 0)
    {
      /* Drive high Charge pump */
      /* ToDo: Add IOE driver control */
      /* USER CODE BEGIN DRIVE_HIGH_CHARGE_FOR_FS */

      /* USER CODE END DRIVE_HIGH_CHARGE_FOR_FS */
    }
    else
    {
      /* Drive low Charge pump */
      /* ToDo: Add IOE driver control */
      /* USER CODE BEGIN DRIVE_LOW_CHARGE_FOR_FS */

      /* USER CODE END DRIVE_LOW_CHARGE_FOR_FS */
    }
  }
  if (phost->id == OTG_HS)
  {
    if (state == 0)
    {
      /* Drive high Charge pump */
      /* ToDo: Add IOE driver control */
      /* USER CODE BEGIN DRIVE_HIGH_CHARGE_FOR_HS */

      /* USER CODE END DRIVE_HIGH_CHARGE_FOR_HS */
    }
    else
    {
      /* Drive low Charge pump */
      /* ToDo: Add IOE driver control */
      /* USER CODE BEGIN DRIVE_LOW_CHARGE_FOR_HS */

      /* USER CODE END DRIVE_LOW_CHARGE_FOR_HS */
    }
  }
  HAL_Delay(200);
  return USBH_OK;
}

/**
  * @brief  Set toggle for a pipe.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @param  toggle: toggle (0/1)
  * @retval Status
  */
USBH_StatusTypeDef USBH_LL_SetToggle(USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t toggle)
{
  HCD_HandleTypeDef *pHandle;
  pHandle = phost->pData;

  if(pHandle->hc[pipe].ep_is_in)
  {
    pHandle->hc[pipe].toggle_in = toggle;
  }
  else
  {
    pHandle->hc[pipe].toggle_out = toggle;
  }

  return USBH_OK;
}

/**
  * @brief  Return the current toggle of a pipe.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @retval toggle (0/1)
  */
uint8_t USBH_LL_GetToggle(USBH_HandleTypeDef *phost, uint8_t pipe)
{
  uint8_t toggle = 0;
  HCD_HandleTypeDef *pHandle;
  pHandle = phost->pData;

  if(pHandle->hc[pipe].ep_is_in)
  {
    toggle = pHandle->hc[pipe].toggle_in;
  }
  else
  {
    toggle = pHandle->hc[pipe].toggle_out;
  }
  return toggle;
}

/**
  * @brief  Delay routine for the USB Host Library
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBH_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}


/**
  * @brief  USBH_LL_CtlSendSetup
  *         Sends the Setup Packet to the Device
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer from which the Data will be send to Device
  * @param  pipe_num: Pipe Number
  * @retval USBH Status
  */
  
USBH_StatusTypeDef USBH_LL_CtlSendSetup(USBH_HandleTypeDef *phost,
                                     uint8_t *buff,
                                     uint8_t pipe_num)
{
    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    0U,                    /* Direction : OUT  */
                    USBH_EP_CONTROL,      /* EP type          */
                    USBH_PID_SETUP,       /* Type setup       */
                    buff,                 /* data buffer      */
                    USBH_SETUP_PKT_SIZE,  /* data length      */
                    0U);
    return USBH_OK;

}

/**
  * @brief  USBH_LL_CtlSendData
  *         Sends a data Packet to the Device
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer from which the Data will be sent to Device
  * @param  length: Length of the data to be sent
  * @param  pipe_num: Pipe Number
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_LL_CtlSendData(USBH_HandleTypeDef *phost,
                                    uint8_t *buff,
                                    uint16_t length,
                                    uint8_t pipe_num,
                                    uint8_t do_ping)
{

    if (phost->device.speed != USBH_SPEED_HIGH)
    {
        do_ping = 0U;
    }

    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    0U,                   /* Direction : OUT  */
                    USBH_EP_CONTROL,      /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    length,               /* data length      */
                    do_ping);             /* do ping (HS Only)*/

    return USBH_OK;
}


/**
  * @brief  USBH_LL_CtlReceiveData
  *         Receives the Device Response to the Setup Packet
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer in which the response needs to be copied
  * @param  length: Length of the data to be received
  * @param  pipe_num: Pipe Number
  * @retval USBH Status.
  */
USBH_StatusTypeDef USBH_LL_CtlReceiveData(USBH_HandleTypeDef *phost,
                                       uint8_t *buff,
                                       uint16_t length,
                                       uint8_t pipe_num)
{
    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    1U,                    /* Direction : IN   */
                    USBH_EP_CONTROL,      /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    length,               /* data length      */
                    0U);
    return USBH_OK;

}


/**
  * @brief  USBH_LL_BulkSendData
  *         Sends the Bulk Packet to the device
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer from which the Data will be sent to Device
  * @param  length: Length of the data to be sent
  * @param  pipe_num: Pipe Number
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_LL_BulkSendData(USBH_HandleTypeDef *phost,
                                     uint8_t *buff,
                                     uint16_t length,
                                     uint8_t pipe_num,
                                     uint8_t do_ping)
{
    if (phost->device.speed != USBH_SPEED_HIGH)
    {
        do_ping = 0U;
    }

    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    0U,                    /* Direction : IN   */
                    USBH_EP_BULK,         /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    length,               /* data length      */
                    do_ping);             /* do ping (HS Only)*/

    return USBH_OK;
}


/**
  * @brief  USBH_LL_BulkReceiveData
  *         Receives IN bulk packet from device
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer in which the received data packet to be copied
  * @param  length: Length of the data to be received
  * @param  pipe_num: Pipe Number
  * @retval USBH Status.
  */
USBH_StatusTypeDef USBH_LL_BulkReceiveData(USBH_HandleTypeDef *phost,
                                        uint8_t *buff,
                                        uint16_t length,
                                        uint8_t pipe_num)
{
    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    1U,                    /* Direction : IN   */
                    USBH_EP_BULK,         /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    length,               /* data length      */
                    0U);

    return USBH_OK;
}


/**
  * @brief  USBH_LL_InterruptReceiveData
  *         Receives the Device Response to the Interrupt IN token
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer in which the response needs to be copied
  * @param  length: Length of the data to be received
  * @param  pipe_num: Pipe Number
  * @retval USBH Status.
  */
USBH_StatusTypeDef USBH_LL_InterruptReceiveData(USBH_HandleTypeDef *phost,
                                             uint8_t *buff,
                                             uint8_t length,
                                             uint8_t pipe_num)
{
    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    1U,                   /* Direction : IN   */
                    USBH_EP_INTERRUPT,    /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    (uint16_t)length,     /* data length      */
                    0U);

    return USBH_OK;
}

/**
  * @brief  USBH_LL_InterruptSendData
  *         Sends the data on Interrupt OUT Endpoint
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer from where the data needs to be copied
  * @param  length: Length of the data to be sent
  * @param  pipe_num: Pipe Number
  * @retval USBH Status.
  */
USBH_StatusTypeDef USBH_LL_InterruptSendData(USBH_HandleTypeDef *phost,
                                          uint8_t *buff,
                                          uint8_t length,
                                          uint8_t pipe_num)
{
    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    0U,                   /* Direction : OUT   */
                    USBH_EP_INTERRUPT,    /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    (uint16_t)length,     /* data length      */
                    0U);

    return USBH_OK;
}

/**
  * @brief  USBH_LL_IsocReceiveData
  *         Receives the Device Response to the Isochronous IN token
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer in which the response needs to be copied
  * @param  length: Length of the data to be received
  * @param  pipe_num: Pipe Number
  * @retval USBH Status.
  */
USBH_StatusTypeDef USBH_LL_IsocReceiveData(USBH_HandleTypeDef *phost,
                                        uint8_t *buff,
                                        uint32_t length,
                                        uint8_t pipe_num)
{
    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    1U,                   /* Direction : IN   */
                    USBH_EP_ISO,          /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    (uint16_t)length,     /* data length      */
                    0U);

    return USBH_OK;
}

/**
  * @brief  USBH_LL_IsocSendData
  *         Sends the data on Isochronous OUT Endpoint
  * @param  phost: Host Handle
  * @param  buff: Buffer pointer from where the data needs to be copied
  * @param  length: Length of the data to be sent
  * @param  pipe_num: Pipe Number
  * @retval USBH Status.
  */
USBH_StatusTypeDef USBH_LL_IsocSendData(USBH_HandleTypeDef *phost,
                                     uint8_t *buff,
                                     uint32_t length,
                                     uint8_t pipe_num)
{

    USBH_LL_SubmitURB(phost,                      /* Driver handle    */
                    pipe_num,             /* Pipe index       */
                    0U,                   /* Direction : OUT   */
                    USBH_EP_ISO,          /* EP type          */
                    USBH_PID_DATA,        /* Type Data        */
                    buff,                 /* data buffer      */
                    (uint16_t)length,     /* data length      */
                    0U);

    return USBH_OK;
}


USBH_LL_Ops usbh_ops = 
{
    USBH_LL_Init,
    USBH_LL_DeInit,
    USBH_LL_Start,
    USBH_LL_Stop,
    USBH_LL_GetSpeed,
    USBH_LL_ResetPort,
    USBH_LL_GetLastXferSize,
    USBH_LL_OpenPipe,
    USBH_LL_ClosePipe,
    USBH_LL_AllocPipe,
    USBH_LL_FreePipe,
    USBH_LL_GetFreePipe,
    USBH_LL_SubmitURB,
    USBH_LL_GetURBState,
    USBH_LL_DriverVBUS,
    USBH_LL_SetToggle,
    USBH_LL_GetToggle,
    USBH_Delay,
    USBH_LL_CtlSendSetup,
    USBH_LL_CtlSendData,
    USBH_LL_CtlReceiveData,
    USBH_LL_BulkSendData,
    USBH_LL_BulkReceiveData,
    USBH_LL_InterruptSendData,
    USBH_LL_InterruptReceiveData,
    USBH_LL_IsocSendData,
    USBH_LL_IsocReceiveData,
};

/**
  * @brief  Retuns the USB status depending on the HAL status:
  * @param  hal_status: HAL status
  * @retval USB status
  */
USBH_StatusTypeDef USBH_Get_USB_Status(HAL_StatusTypeDef hal_status)
{
  USBH_StatusTypeDef usb_status = USBH_OK;

  switch (hal_status)
  {
    case HAL_OK :
      usb_status = USBH_OK;
    break;
    case HAL_ERROR :
      usb_status = USBH_FAIL;
    break;
    case HAL_BUSY :
      usb_status = USBH_BUSY;
    break;
    case HAL_TIMEOUT :
      usb_status = USBH_FAIL;
    break;
    default :
      usb_status = USBH_FAIL;
    break;
  }
  return usb_status;
}





static void USBH_UserProcess (USBH_HandleTypeDef *phost, uint8_t id)
{
  /* USER CODE BEGIN CALL_BACK_2 */
  switch(id)
  {
  case HOST_USER_SELECT_CONFIGURATION:
  break;

  case HOST_USER_DISCONNECTION:

  break;

  case HOST_USER_CLASS_ACTIVE:

  break;

  case HOST_USER_CONNECTION:

  break;

  default:
  break;
  }
  /* USER CODE END CALL_BACK_2 */
}

void MX_USB_HOST_Init(USBH_HandleTypeDef *phost, uint8_t id)
{
    /* Init host Library, add supported class and start the library. */
    if (USBH_Init(phost, USBH_UserProcess, id) != USBH_OK)
    {
        Error_Handler();
    }

#ifdef ENABLE_USBH_AUDIO_CLASS
    if (USBH_RegisterClass(phost, USBH_AUDIO_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
#endif

#ifdef ENABLE_USBH_CDC_CLASS
    if (USBH_RegisterClass(phost, USBH_CDC_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
#endif

#ifdef ENABLE_USBH_MSC_CLASS
    if (USBH_RegisterClass(phost, USBH_MSC_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
#endif

#ifdef ENABLE_USBH_HID_CLASS
    if (USBH_RegisterClass(phost, USBH_HID_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
#endif

#ifdef ENABLE_USBH_MTP_CLASS
    if (USBH_RegisterClass(phost, USBH_MTP_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
#endif

#ifdef ENABLE_USBH_SPECIFIC_CDC_CLASS
    if (USBH_RegisterClass(phost, USBH_SPECIFIC_CDC_CLASS) != USBH_OK)
    {
        Error_Handler();
    }
#endif
    
    /* USER CODE BEGIN USB_HOST_Init_PreTreatment */

}

void usbh_start(USBH_HandleTypeDef *phost)
{
    if (USBH_Start(phost) != USBH_OK)
    {
        Error_Handler();
    }
}

struct stm32_usbh
{
    USBH_HandleTypeDef husbh;
    
    os_list_node_t list;
    
    uint8_t host_type;
};

static int stm32_usb_host_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    
    struct stm32_usbh * usb_host = os_calloc(1, sizeof(struct stm32_usbh));
    
    struct stm32_hcd_info * usbh_info = (struct stm32_hcd_info *)dev->info;
    
    usb_host->husbh.pData = usbh_info->instance;
    usb_host->host_type    = usbh_info->host_type;
    usbh_info->instance->pData = &usb_host->husbh;
    usb_host->husbh.ll_ops = &usbh_ops;

    level = os_irq_lock();
    os_list_add_tail(&stm32_usbh_list, &usb_host->list);
    os_irq_unlock(level);
    
    MX_USB_HOST_Init(&usb_host->husbh, usb_host->host_type);
    
    return OS_EOK;
}

OS_DRIVER_INFO stm32_usb_host_driver = {
    .name   = "HCD_HandleTypeDef",
    .probe  = stm32_usb_host_probe,
};
OS_DRIVER_DEFINE(stm32_usb_host_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);


os_err_t usbh_start_init(void)
{
    struct stm32_usbh *usbh;
    
    os_list_for_each_entry(usbh, &stm32_usbh_list, struct stm32_usbh, list)
    {
        usbh_start(&usbh->husbh);
    }
    
    return OS_EOK;
}

OS_APP_INIT(usbh_start_init, OS_INIT_SUBLEVEL_HIGH);


