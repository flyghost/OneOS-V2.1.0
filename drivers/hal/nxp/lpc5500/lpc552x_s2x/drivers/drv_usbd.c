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
 * @brief       This file implements USB driver for lpc
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include "board.h"
#include "drv_usbd.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_composite.h"
#include "usb_device_config.h"
#include "usb_device_descriptor.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DBG_TAG "drv.usbd"
#include <drv_log.h>
#include <dlog.h>

typedef struct lpc_usb
{
    os_device_t parent;
    usb_device_handle deviceHandle;
    usb_device_composite_struct_t *s_UsbDeviceComposite;
}lpc_usb_t;

static lpc_usb_t lpc_usb = {0};

#ifdef OS_USB_DEVICE_HID
#ifdef OS_USB_DEVICE_HID_MOUSE
usb_status_t USB_DeviceInterface0HidMouseInit(usb_device_composite_struct_t *deviceComposite)
{
    lpc_usb.s_UsbDeviceComposite = deviceComposite;
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceHidMousecallback(void)
{
    if (lpc_usb.parent.ops != OS_NULL)
        os_device_recv_notify(&lpc_usb.parent);
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceInterface0HidMouseCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Error;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            if (lpc_usb.s_UsbDeviceComposite->attach)
            {
                return USB_DeviceHidMousecallback();
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            break;
        default:
            break;
    }

    return error;
}

usb_status_t USB_DeviceInterface0HidMouseSetConfiguration(class_handle_t handle, uint8_t configuration_idx)
{
   return USB_DeviceHidMousecallback();
}

usb_status_t USB_DeviceInterface0HidMouseSetInterface(class_handle_t handle, uint8_t alternateSetting)
{
   return USB_DeviceHidMousecallback();
}

os_size_t lpc_usb_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    usb_status_t status;

    status = USB_DeviceHidRecv(lpc_usb.s_UsbDeviceComposite->interface0HidMouseHandle, USB_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN, (os_uint8_t *)buffer, USB_INTERFACE_0_HID_MOUSE_INPUT_REPORT_LENGTH);
    if (status != kStatus_USB_Success)
    {
        LOG_E(DBG_TAG, "USB read failed!");
        return OS_ERROR;
    }

    return OS_EOK;
}

os_size_t lpc_usb_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    usb_status_t status;
    
    status = USB_DeviceHidSend(lpc_usb.s_UsbDeviceComposite->interface0HidMouseHandle, USB_INTERFACE_0_HID_MOUSE_SETTING_0_EP_1_INTERRUPT_IN, (os_uint8_t *)buffer, USB_INTERFACE_0_HID_MOUSE_INPUT_REPORT_LENGTH);
    if (status != kStatus_USB_Success)
    {
        LOG_E(DBG_TAG, "USB write failed!");
        return OS_ERROR;
    }

    return OS_EOK;
}

usb_status_t lpc_usb_device_start()
{
    return USB_DeviceRun(lpc_usb.s_UsbDeviceComposite->deviceHandle);
}

usb_status_t lpc_usb_device_stop()
{
    return USB_DeviceStop(lpc_usb.s_UsbDeviceComposite->deviceHandle);
}
#endif
#endif

static os_err_t lpc_usb_init(os_device_t *device)
{
    lpc_usb_device_stop();
    
    return OS_EOK;
}

static os_err_t  lpc_usb_control(os_device_t *dev, os_int32_t cmd, void *args)
{
    usb_status_t status;
    
    switch (cmd)
    {
    case LPC_USB_START:
        status = lpc_usb_device_start();
        break;

    case LPC_USB_STOP:
        status = lpc_usb_device_stop();
        break;
    }
    
    return OS_EOK;
}

const static struct os_device_ops usbd_ops = {
    .init = lpc_usb_init,
    .read = lpc_usb_read,
    .write = lpc_usb_write,
    .control = lpc_usb_control,
};

static int lpc_usbd_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    lpc_usb.parent.type = OS_DEVICE_TYPE_USBDEVICE;
    lpc_usb.parent.ops = &usbd_ops;
    lpc_usb.parent.user_data = &lpc_usb,
    
    os_device_register((os_device_t *)&lpc_usb, "usbd");
    return OS_EOK;
}

OS_DRIVER_INFO lpc_usbd_driver = {
    .name   = "USB_Type",
    .probe  = lpc_usbd_probe,
};

OS_DRIVER_DEFINE(lpc_usbd_driver, DEVICE, OS_INIT_SUBLEVEL_LOW);

#ifdef OS_USB_DEVICE_HID_MOUSE
#include <shell.h>
#include <os_memory.h>

os_err_t usb_mouse_callback(os_device_t *dev, struct os_device_cb_info *info)
{
    return OS_EOK;
}

int USB_DeviceHidMouse_test(void)
{
    static int8_t x = 0U;
    static int8_t y = 0U;

    os_uint8_t *buffer = (os_uint8_t *)os_calloc(1, 8);

    os_device_t *device = os_device_find("usbd");
    
    OS_ASSERT(device != OS_NULL);
    os_device_open(device);
    
    struct os_device_cb_info cb_info = 
    {
        .type = OS_DEVICE_CB_TYPE_RX,
        .cb   = usb_mouse_callback,
    };

    os_device_control(device, OS_DEVICE_CTRL_SET_CB, &cb_info);
    
    os_device_control(device, LPC_USB_START, OS_NULL);
    enum
    {
        RIGHT,
        DOWN,
        LEFT,
        UP
    };
    static uint8_t dir = RIGHT;

    for (int i = 0; i < 800; i++)
    {
        switch (dir)
        {
            case RIGHT:
                /* Move right. Increase X value. */
                buffer[1] = 1U;
                buffer[2] = 0U;
                x++;
                if (x > 99U)
                {
                    dir++;
                }
                break;
            case DOWN:
                /* Move down. Increase Y value. */
                buffer[1] = 0U;
                buffer[2] = 1U;
                y++;
                if (y > 99U)
                {
                    dir++;
                }
                break;
            case LEFT:
                /* Move left. Decrease X value. */
                buffer[1] = (uint8_t)(0xFFU);
                buffer[2] = 0U;
                x--;
                if (x < 1U)
                {
                    dir++;
                }
                break;
            case UP:
                /* Move up. Decrease Y value. */
                buffer[1] = 0U;
                buffer[2] = (uint8_t)(0xFFU);
                y--;
                if (y < 1U)
                {
                    dir = RIGHT;
                }
                break;
            default:
                break;
        }
        os_task_msleep(10);
        os_device_write_block(device, 0, buffer, 4); 
    }
    os_device_control(device, LPC_USB_STOP, OS_NULL);
    
    return OS_EOK;
}

SH_CMD_EXPORT(usbd_test, USB_DeviceHidMouse_test, "usbd_hid_test");
#endif