/**
  ******************************************************************************
  * @file    usbh_hid_mouse.c
  * @author  MCD Application Team
  * @brief   This file is the application layer for USB Host HID Mouse Handling.
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
- "stm32xxxxx_{eval}{discovery}{adafruit}_lcd.c"
- "stm32xxxxx_{eval}{discovery}_sdram.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid_mouse.h"
#include "usbh_hid_parser.h"


/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_MOUSE
  * @brief    This file includes HID Layer Handlers for USB Host HID class.
  * @{
  */

/** @defgroup USBH_HID_MOUSE_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_MOUSE_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_HID_MOUSE_Private_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_MOUSE_Private_FunctionPrototypes
  * @{
  */
static USBH_StatusTypeDef USBH_HID_MouseDecode(USBH_HandleTypeDef *phost);

/**
  * @}
  */


/** @defgroup USBH_HID_MOUSE_Private_Variables
  * @{
  */
HID_MOUSE_Info_TypeDef    mouse_info;
uint32_t                  mouse_report_data[2];
uint32_t                  mouse_rx_report_buf[2];

/* Structures defining how to access items in a HID mouse report */
/* Access button 1 state. */
static const HID_Report_ItemTypedef prop_b1 =
{
  (uint8_t *)(void *)mouse_report_data + 0, /*data*/
  1,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min value device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access button 2 state. */
static const HID_Report_ItemTypedef prop_b2 =
{
  (uint8_t *)(void *)mouse_report_data + 0, /*data*/
  1,     /*size*/
  1,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min value device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access button 3 state. */
static const HID_Report_ItemTypedef prop_b3 =
{
  (uint8_t *)(void *)mouse_report_data + 0, /*data*/
  1,     /*size*/
  2,     /*shift*/
  0,     /*count (only for array items)*/
  0,     /*signed?*/
  0,     /*min value read can return*/
  1,     /*max value read can return*/
  0,     /*min vale device can report*/
  1,     /*max value device can report*/
  1      /*resolution*/
};

/* Access x coordinate change. */
static const HID_Report_ItemTypedef prop_x =
{
  (uint8_t *)(void *)mouse_report_data + 1, /*data*/
  8,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  1,     /*signed?*/
  0,     /*min value read can return*/
  0xFFFF,/*max value read can return*/
  0,     /*min vale device can report*/
  0xFFFF,/*max value device can report*/
  1      /*resolution*/
};

/* Access y coordinate change. */
static const HID_Report_ItemTypedef prop_y =
{
  (uint8_t *)(void *)mouse_report_data + 2, /*data*/
  8,     /*size*/
  0,     /*shift*/
  0,     /*count (only for array items)*/
  1,     /*signed?*/
  0,     /*min value read can return*/
  0xFFFF,/*max value read can return*/
  0,     /*min vale device can report*/
  0xFFFF,/*max value device can report*/
  1      /*resolution*/
};


/**
  * @}
  */
#include <device.h>
#include <os_memory.h>
#include <os_errno.h>
struct os_device *mouse_dev;

#define OS_DEVICE_CTRL_MOUSE_SET_CALLBACK   IOC_USBDEVICE(0)        /* Set Callback. */
static USBH_StatusTypeDef(* mouse_data_in_callback)(HID_MOUSE_Info_TypeDef *mouse_info);

static os_err_t os_mouse_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t result = OS_ERROR;
//    OS_ASSERT(dev != OS_NULL);

//    struct os_device *mouse = (struct os_device *)dev;
    
    switch (cmd)
    {
        case OS_DEVICE_CTRL_MOUSE_SET_CALLBACK:
            // set callback
            mouse_data_in_callback = (USBH_StatusTypeDef(* )(HID_MOUSE_Info_TypeDef *mouse_info))args;
            break;
        default:
            break;
    }

    return result;
}

const static struct os_device_ops mouse_ops = {
    .control = os_mouse_control,
};

os_err_t usbh_mouse_dev_register(USBH_HandleTypeDef *phost, const char *name, void *data)
{
    os_err_t          ret = osOK;
    
    mouse_dev = os_calloc(1, sizeof(struct os_device));
    memset(mouse_dev, 0, sizeof(struct os_device));
    mouse_dev->type = OS_DEVICE_TYPE_USBDEVICE;
    mouse_dev->ops = &mouse_ops;
    
    ret = os_device_register(mouse_dev, name);

    return ret;
}

os_err_t usbh_mouse_dev_unregister()
{
    os_err_t          ret = osOK;
    
    ret = os_device_unregister(mouse_dev);
    os_free(mouse_dev);
//    mouse_dev = NULL;

    return ret;
}


/** @defgroup USBH_HID_MOUSE_Private_Functions
  * @{
  */

/**
  * @brief  USBH_HID_MouseInit
  *         The function init the HID mouse.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_HID_MouseInit(USBH_HandleTypeDef *phost)
{
    uint32_t i;
    HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

    mouse_info.x = 0U;
    mouse_info.y = 0U;
    mouse_info.buttons[0] = 0U;
    mouse_info.buttons[1] = 0U;
    mouse_info.buttons[2] = 0U;

    for (i = 0U; i < (sizeof(mouse_report_data) / sizeof(uint32_t)); i++)
    {
        mouse_report_data[i] = 0U;
        mouse_rx_report_buf[i] = 0U;
    }

    if (HID_Handle->length > sizeof(mouse_report_data))
    {
        HID_Handle->length = sizeof(mouse_report_data);
    }
    HID_Handle->pData = (uint8_t *)(void *)mouse_rx_report_buf;
    USBH_HID_FifoInit(&HID_Handle->fifo, phost->device.Data, HID_QUEUE_SIZE * sizeof(mouse_report_data));

    usbh_mouse_dev_register(phost, "mouse", NULL);
    return USBH_OK;
}


USBH_StatusTypeDef USBH_HID_MouseDeInit(USBH_HandleTypeDef *phost)
{
    usbh_mouse_dev_unregister();
    return USBH_OK;
}

USBH_StatusTypeDef USBH_HID_Mouse_Callback(USBH_HandleTypeDef *phost)
{
    if (mouse_data_in_callback != NULL)
    {
        mouse_data_in_callback(USBH_HID_GetMouseInfo(phost));
    }
    return USBH_OK;
}


/**
  * @brief  USBH_HID_GetMouseInfo
  *         The function return mouse information.
  * @param  phost: Host handle
  * @retval mouse information
  */
HID_MOUSE_Info_TypeDef *USBH_HID_GetMouseInfo(USBH_HandleTypeDef *phost)
{
  if (USBH_HID_MouseDecode(phost) == USBH_OK)
  {
    return &mouse_info;
  }
  else
  {
    return NULL;
  }
}

/**
  * @brief  USBH_HID_MouseDecode
  *         The function decode mouse data.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_HID_MouseDecode(USBH_HandleTypeDef *phost)
{
  HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;

  if (HID_Handle->length == 0U)
  {
    return USBH_FAIL;
  }
  /*Fill report */
  if (USBH_HID_FifoRead(&HID_Handle->fifo, &mouse_report_data, HID_Handle->length) ==  HID_Handle->length)
  {
    /*Decode report */
    mouse_info.x = (int8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_x, 0U);
    mouse_info.y = (int8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_y, 0U);

    mouse_info.buttons[0] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_b1, 0U);
    mouse_info.buttons[1] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_b2, 0U);
    mouse_info.buttons[2] = (uint8_t)HID_ReadItem((HID_Report_ItemTypedef *) &prop_b3, 0U);

    return USBH_OK;
  }
  return   USBH_FAIL;
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
