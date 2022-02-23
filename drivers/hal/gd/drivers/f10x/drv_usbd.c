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

#include <os_task.h>
#include <os_irq.h>
#include "board.h"

//#define DBG_ENABLE
#define DBG_SECTION_NAME "USBD"
#define DBG_LEVEL        DBG_LOG
#define DBG_COLOR
#include <drv_log.h>

static PCD_HandleTypeDef *_stm_pcd;
static struct udcd        _stm_udc;
static struct ep_id       _ep_pool[] = {
    {0x0, USB_EP_ATTR_CONTROL, USB_DIR_INOUT, 64, ID_ASSIGNED},
    {0x1, USB_EP_ATTR_BULK, USB_DIR_IN, 64, ID_UNASSIGNED},
    {0x1, USB_EP_ATTR_BULK, USB_DIR_OUT, 64, ID_UNASSIGNED},
    {0x2, USB_EP_ATTR_INT, USB_DIR_IN, 64, ID_UNASSIGNED},
    {0x2, USB_EP_ATTR_INT, USB_DIR_OUT, 64, ID_UNASSIGNED},
    {0x3, USB_EP_ATTR_BULK, USB_DIR_IN, 64, ID_UNASSIGNED},
#if !defined(SERIES_STM32F1)
    {0x3, USB_EP_ATTR_BULK, USB_DIR_OUT, 64, ID_UNASSIGNED},
#endif
    {0xFF, USB_EP_ATTR_TYPE_MASK, USB_DIR_MASK, 0, ID_ASSIGNED},
};

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *pcd)
{
    os_interrupt_enter();

    /* open ep0 OUT and IN */
    HAL_PCD_EP_Open(pcd, 0x00, 0x40, EP_TYPE_CTRL);
    HAL_PCD_EP_Open(pcd, 0x80, 0x40, EP_TYPE_CTRL);
    os_usbd_reset_handler(&_stm_udc);

    os_interrupt_leave();
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
    os_interrupt_enter();
    os_usbd_ep0_setup_handler(&_stm_udc, (struct urequest *)hpcd->Setup);
    os_interrupt_leave();
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    os_interrupt_enter();
    
    if (epnum == 0)
    {
        os_usbd_ep0_in_handler(&_stm_udc);
    }
    else
    {
        os_usbd_ep_in_handler(&_stm_udc, 0x80 | epnum, hpcd->IN_ep[epnum].xfer_count);
    }

    os_interrupt_leave();
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
    os_interrupt_enter();
    
    os_usbd_connect_handler(&_stm_udc);

    os_interrupt_leave();
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
    os_interrupt_enter();
    
    os_usbd_sof_handler(&_stm_udc);

    os_interrupt_leave();
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    os_interrupt_enter();
    
    os_usbd_disconnect_handler(&_stm_udc);

    os_interrupt_leave();
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    os_interrupt_enter();
    
    if (epnum != 0)
    {
        os_usbd_ep_out_handler(&_stm_udc, epnum, hpcd->OUT_ep[epnum].xfer_count);
    }
    else
    {
        os_usbd_ep0_out_handler(&_stm_udc, hpcd->OUT_ep[0].xfer_count);
    }

    os_interrupt_leave();
}

void HAL_PCDEx_SetConnectionState(PCD_HandleTypeDef *hpcd, uint8_t state)
{
    os_interrupt_enter();
    
    if (state == 1)
    {
#if defined(SERIES_STM32F1)
        os_pin_mode(BSP_USB_CONNECT_PIN, PIN_MODE_OUTPUT);
        os_pin_write(BSP_USB_CONNECT_PIN, BSP_USB_PULL_UP_STATUS);
#endif
    }
    else
    {
#if defined(SERIES_STM32F1)
        os_pin_mode(BSP_USB_CONNECT_PIN, PIN_MODE_OUTPUT);
        os_pin_write(BSP_USB_CONNECT_PIN, !BSP_USB_PULL_UP_STATUS);
#endif
    }

    os_interrupt_leave();
}

static os_err_t _ep_set_stall(os_uint8_t address)
{
    HAL_PCD_EP_SetStall(_stm_pcd, address);
    return OS_EOK;
}

static os_err_t _ep_clear_stall(os_uint8_t address)
{
    HAL_PCD_EP_ClrStall(_stm_pcd, address);
    return OS_EOK;
}

static os_err_t _set_address(os_uint8_t address)
{
    HAL_PCD_SetAddress(_stm_pcd, address);
    return OS_EOK;
}

static os_err_t _set_config(os_uint8_t address)
{
    return OS_EOK;
}

static os_err_t _ep_enable(uep_t ep)
{
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);
    HAL_PCD_EP_Open(_stm_pcd, ep->ep_desc->bEndpointAddress, ep->ep_desc->wMaxPacketSize, ep->ep_desc->bmAttributes);
    return OS_EOK;
}

static os_err_t _ep_disable(uep_t ep)
{
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);
    HAL_PCD_EP_Close(_stm_pcd, ep->ep_desc->bEndpointAddress);
    return OS_EOK;
}

static os_size_t _ep_read(os_uint8_t address, void *buffer)
{
    os_size_t size = 0;
    OS_ASSERT(buffer != OS_NULL);
    return size;
}

static os_size_t _ep_read_prepare(os_uint8_t address, void *buffer, os_size_t size)
{
    HAL_PCD_EP_Receive(_stm_pcd, address, buffer, size);
    return size;
}

static os_size_t _ep_write(os_uint8_t address, void *buffer, os_size_t size)
{
    HAL_PCD_EP_Transmit(_stm_pcd, address, buffer, size);
    return size;
}

static os_err_t _ep0_send_status(void)
{
    HAL_PCD_EP_Transmit(_stm_pcd, 0x00, NULL, 0);
    return OS_EOK;
}

static os_err_t _suspend(void)
{
    return OS_EOK;
}

static os_err_t _wakeup(void)
{
    return OS_EOK;
}

static os_err_t _init(os_device_t *device)
{
    PCD_HandleTypeDef *pcd = (PCD_HandleTypeDef *)device->user_data;
    
    /* Set LL Driver parameters */
    pcd->Init.dev_endpoints = 8;
    pcd->Init.ep0_mps       = DEP0CTL_MPS_64;

    /* Initialize LL Driver */
    HAL_PCD_DeInit(pcd);
    HAL_PCD_Init(pcd);
    
#if !defined(SERIES_STM32F3)
    HAL_PCDEx_SetRxFiFo(pcd, 0x80);
    HAL_PCDEx_SetTxFiFo(pcd, 0, 0x40);
    HAL_PCDEx_SetTxFiFo(pcd, 1, 0x40);
    HAL_PCDEx_SetTxFiFo(pcd, 2, 0x40);
    HAL_PCDEx_SetTxFiFo(pcd, 3, 0x40);
#else
    HAL_PCDEx_PMAConfig(pcd, 0x00, PCD_SNG_BUF, 0x18);
    HAL_PCDEx_PMAConfig(pcd, 0x80, PCD_SNG_BUF, 0x58);
    HAL_PCDEx_PMAConfig(pcd, 0x81, PCD_SNG_BUF, 0x98);
    HAL_PCDEx_PMAConfig(pcd, 0x01, PCD_SNG_BUF, 0x118);
    HAL_PCDEx_PMAConfig(pcd, 0x82, PCD_SNG_BUF, 0xD8);
    HAL_PCDEx_PMAConfig(pcd, 0x02, PCD_SNG_BUF, 0x158);
    HAL_PCDEx_PMAConfig(pcd, 0x83, PCD_SNG_BUF, 0x198);
#endif
    HAL_PCD_Start(pcd);
    return OS_EOK;
}

const static struct udcd_ops _udc_ops = {
    _set_address,
    _set_config,
    _ep_set_stall,
    _ep_clear_stall,
    _ep_enable,
    _ep_disable,
    _ep_read_prepare,
    _ep_read,
    _ep_write,
    _ep0_send_status,
    _suspend,
    _wakeup,
};

#ifdef OS_USING_DEVICE_OPS
const static struct os_device_ops _ops = {
    _init,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
    OS_NULL,
};
#endif

static int stm32_usbd_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    _stm_pcd = (PCD_HandleTypeDef *)dev->info;
    
    memset((void *)&_stm_udc, 0, sizeof(struct udcd));
    _stm_udc.parent.type = OS_DEVICE_TYPE_USBDEVICE;
#ifdef OS_USING_DEVICE_OPS
    _stm_udc.parent.ops = &_ops;
#else
    _stm_udc.parent.init = _init;
#endif
    _stm_udc.parent.user_data = _stm_pcd;
    _stm_udc.ops              = &_udc_ops;
    /* Register endpoint infomation */
    _stm_udc.ep_pool = _ep_pool;
    _stm_udc.ep0.id  = &_ep_pool[0];

#ifdef PCD_SPEED_HIGH
    if (_stm_pcd->Init.speed == PCD_SPEED_HIGH)
        _stm_udc.device_is_hs = OS_TRUE;
#endif

    os_device_register((os_device_t *)&_stm_udc, "usbd", 0);
    os_usb_device_init();
    return OS_EOK;
}

OS_DRIVER_INFO stm32_usbd_driver = {
    .name   = "PCD_HandleTypeDef",
    .probe  = stm32_usbd_probe,
};

OS_DRIVER_DEFINE(stm32_usbd_driver, "3");

