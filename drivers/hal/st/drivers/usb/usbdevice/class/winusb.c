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
 * @file        winusb.c
 *
 * @brief       This file provides functions for winusb.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <arch_interrupt.h>
#include <os_task.h>
#include <os_list.h>
#include <drv_cfg.h>
#include <usb/usb_device.h>
#include "winusb.h"
struct winusb_device
{
    struct os_device parent;
    void (*cmd_handler)(os_uint8_t *buffer, os_size_t size);
    os_uint8_t cmd_buff[256];
    uep_t      ep_out;
    uep_t      ep_in;
};

typedef struct winusb_device *winusb_device_t;

OS_ALIGN(4)
static struct udevice_descriptor dev_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength; */
    USB_DESC_TYPE_DEVICE,     /* type; */
    USB_BCD_VERSION,          /* bcdUSB; */
    0x00,                     /* bDeviceClass; */
    0x00,                     /* bDeviceSubClass; */
    0x00,                     /* bDeviceProtocol; */
    0x40,                     /* bMaxPacketSize0; */
    _VENDOR_ID,               /* idVendor; */
    _PRODUCT_ID,              /* idProduct; */
    USB_BCD_DEVICE,           /* bcdDevice; */
    USB_STRING_MANU_INDEX,    /* iManufacturer; */
    USB_STRING_PRODUCT_INDEX, /* iProduct; */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber; */
    USB_DYNAMIC,              /* bNumConfigurations; */
};

// FS and HS needed
OS_ALIGN(4)
static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),         /* bLength */
    USB_DESC_TYPE_DEVICEQUALIFIER, /* bDescriptorType */
    0x0200,                        /* bcdUSB */
    0xFF,                          /* bDeviceClass */
    0x00,                          /* bDeviceSubClass */
    0x00,                          /* bDeviceProtocol */
    64,                            /* bMaxPacketSize0 */
    0x01,                          /* bNumConfigurations */
    0,
};

OS_ALIGN(4)
struct winusb_descriptor _winusb_desc =
{
#ifdef OS_USB_DEVICE_COMPOSITE
    /* Interface Association Descriptor */
    {
        USB_DESC_LENGTH_IAD,
        USB_DESC_TYPE_IAD,
        USB_DYNAMIC,
        0x01,
        0xFF,
        0x00,
        0x00,
        0x00,
    },
#endif
    /* Interface descriptor */
    {
        USB_DESC_LENGTH_INTERFACE, /* bLength; */
        USB_DESC_TYPE_INTERFACE,   /* type; */
        USB_DYNAMIC,               /* bInterfaceNumber; */
        0x00,                      /* bAlternateSetting; */
        0x02,                      /* bNumEndpoints */
        0xFF,                      /* bInterfaceClass; */
        0x00,                      /* bInterfaceSubClass; */
        0x00,                      /* bInterfaceProtocol; */
        0x00,                      /* iInterface; */
    },
    /* Endpoint descriptor */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_OUT,
        USB_EP_ATTR_BULK,
        USB_DYNAMIC,
        0x00,
    },
    /* Endpoint descriptor */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_IN,
        USB_EP_ATTR_BULK,
        USB_DYNAMIC,
        0x00,
    },
};

OS_ALIGN(4)
const static char *_ustring[] =
{
    "Language",
    "OS Team.",
    "OS Win USB",
    "32021919830108",
    "Configuration",
    "Interface",
    USB_STRING_OS /* Must be */
};

OS_ALIGN(4)
struct usb_os_proerty winusb_proerty[] =
{
    USB_OS_PROERTY_DESC(USB_OS_PROERTY_TYPE_REG_SZ, "DeviceInterfaceGUID", OS_WINUSB_GUID),
};

OS_ALIGN(4)
struct usb_os_function_comp_id_descriptor winusb_func_comp_id_desc =
{
    .bFirstInterfaceNumber = USB_DYNAMIC,
    .reserved1             = 0x01,
    .compatibleID          = {'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00},
    .subCompatibleID       = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    .reserved2             = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

static os_err_t _ep_out_handler(ufunction_t func, os_size_t size)
{
    winusb_device_t winusb_device = (winusb_device_t)func->user_data;

    winusb_device->parent.rx_count = size;
    os_device_recv_notify(&winusb_device->parent);

    return OS_EOK;
}

static os_err_t _ep_in_handler(ufunction_t func, os_size_t size)
{
    winusb_device_t winusb_device = (winusb_device_t)func->user_data;

#if 0
    struct os_device_cb_info *info = &winusb_device->parent.cb_table[OS_DEVICE_CB_TYPE_TX];
    if (info->cb != OS_NULL)
    {
        info->data = winusb_device->ep_in->buffer;
        info->cb(&winusb_device->parent, info);
    }
#endif
    
    os_device_send_notify(&winusb_device->parent);
    
    return OS_EOK;
}
static ufunction_t cmd_func = OS_NULL;
static os_err_t    _ep0_cmd_handler(udevice_t device, os_size_t size)
{
    winusb_device_t winusb_device;

    if (cmd_func != OS_NULL)
    {
        winusb_device = (winusb_device_t)cmd_func->user_data;
        cmd_func      = OS_NULL;
        if (winusb_device->cmd_handler != OS_NULL)
        {
            winusb_device->cmd_handler(winusb_device->cmd_buff, size);
        }
    }
    dcd_ep0_send_status(device->dcd);
    return OS_EOK;
}
static os_err_t _ep0_cmd_read(ufunction_t func, ureq_t setup)
{
    winusb_device_t winusb_device = (winusb_device_t)func->user_data;
    cmd_func                      = func;
    os_usbd_ep0_read(func->device, winusb_device->cmd_buff, setup->wLength, _ep0_cmd_handler);
    return OS_EOK;
}
static os_err_t _interface_handler(ufunction_t func, ureq_t setup)
{
    switch (setup->bRequest)
    {
    case 'A':
        switch (setup->wIndex)
        {
        case 0x05:
            usbd_os_proerty_descriptor_send(func,
                                            setup,
                                            winusb_proerty,
                                            sizeof(winusb_proerty) / sizeof(winusb_proerty[0]));
            break;
        }
        break;
    case 0x0A: /* customer */
        _ep0_cmd_read(func, setup);
        break;
    }

    return OS_EOK;
}
static os_err_t _function_enable(ufunction_t func)
{
    OS_ASSERT(func != OS_NULL);
    return OS_EOK;
}
static os_err_t _function_disable(ufunction_t func)
{
    OS_ASSERT(func != OS_NULL);
    return OS_EOK;
}

static struct ufunction_ops ops =
{
    _function_enable,
    _function_disable,
    OS_NULL,
};

static os_err_t _winusb_descriptor_config(winusb_desc_t winusb, os_uint8_t cintf_nr, os_uint8_t device_is_hs)
{
#ifdef OS_USB_DEVICE_COMPOSITE
    winusb->iad_desc.bFirstInterface = cintf_nr;
#endif
    winusb->ep_out_desc.wMaxPacketSize             = device_is_hs ? 512 : 64;
    winusb->ep_in_desc.wMaxPacketSize              = device_is_hs ? 512 : 64;
    winusb_func_comp_id_desc.bFirstInterfaceNumber = cintf_nr;
    return OS_EOK;
}

static os_size_t win_usb_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    if (((ufunction_t)dev->user_data)->device->state != USB_STATE_CONFIGURED)
    {
        return 0;
    }
    winusb_device_t winusb_device           = (winusb_device_t)dev;
    winusb_device->ep_out->buffer           = buffer;
    winusb_device->ep_out->request.buffer   = buffer;
    winusb_device->ep_out->request.size     = size;
    winusb_device->ep_out->request.req_type = UIO_REQUEST_READ_FULL;
    os_usbd_io_request(((ufunction_t)dev->user_data)->device, winusb_device->ep_out, &winusb_device->ep_out->request);
    return size;
}
static os_size_t win_usb_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    if (((ufunction_t)dev->user_data)->device->state != USB_STATE_CONFIGURED)
    {
        return 0;
    }
    winusb_device_t winusb_device          = (winusb_device_t)dev;
    winusb_device->ep_in->buffer           = (void *)buffer;
    winusb_device->ep_in->request.buffer   = winusb_device->ep_in->buffer;
    winusb_device->ep_in->request.size     = size;
    winusb_device->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(((ufunction_t)dev->user_data)->device, winusb_device->ep_in, &winusb_device->ep_in->request);
    return size;
}
static os_err_t win_usb_control(os_device_t *dev, int cmd, void *args)
{
    winusb_device_t winusb_device = (winusb_device_t)dev;
    if (OS_DEVICE_CTRL_CONFIG == cmd)
    {
        winusb_device->cmd_handler = (void (*)(os_uint8_t *, os_size_t))args;
    }
    return OS_EOK;
}

const static struct os_device_ops winusb_device_ops =
{
    OS_NULL,
    OS_NULL,
    OS_NULL,
    win_usb_read,
    win_usb_write,
    win_usb_control,
};

static os_err_t os_usb_winusb_init(ufunction_t func)
{
    winusb_device_t winusb_device = (winusb_device_t)func->user_data;
    winusb_device->parent.type    = OS_DEVICE_TYPE_MISCELLANEOUS;
    winusb_device->parent.ops     = &winusb_device_ops;
    winusb_device->parent.user_data = func;

    return os_device_register(&winusb_device->parent, "winusb");
}

ufunction_t os_usbd_function_winusb_create(udevice_t device)
{
    ufunction_t     func;
    winusb_device_t winusb_device;

    uintf_t       winusb_intf;
    ualtsetting_t winusb_setting;
    winusb_desc_t winusb_desc;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Set usb device string description */
    os_usbd_device_set_string(device, _ustring);

    /* Create a cdc function */
    func = os_usbd_function_new(device, &dev_desc, &ops);
    os_usbd_device_set_qualifier(device, &dev_qualifier);

    /* Allocate memory for cdc vcom data */
    winusb_device = (winusb_device_t)os_calloc(1, sizeof(struct winusb_device));
    memset((void *)winusb_device, 0, sizeof(struct winusb_device));
    func->user_data = (void *)winusb_device;
    /* Create an interface object */
    winusb_intf = os_usbd_interface_new(device, _interface_handler);

    /* Create an alternate setting object */
    winusb_setting = os_usbd_altsetting_new(sizeof(struct winusb_descriptor));

    /* Config desc in alternate setting */
    os_usbd_altsetting_config_descriptor(winusb_setting, &_winusb_desc, (os_off_t) & ((winusb_desc_t)0)->intf_desc);

    /* Configure the hid interface descriptor */
    _winusb_descriptor_config(winusb_setting->desc, winusb_intf->intf_num, device->dcd->device_is_hs);

    /* Create endpoint */
    winusb_desc           = (winusb_desc_t)winusb_setting->desc;
    winusb_device->ep_out = os_usbd_endpoint_new(&winusb_desc->ep_out_desc, _ep_out_handler);
    winusb_device->ep_in  = os_usbd_endpoint_new(&winusb_desc->ep_in_desc, _ep_in_handler);

    /* Add the int out and int in endpoint to the alternate setting */
    os_usbd_altsetting_add_endpoint(winusb_setting, winusb_device->ep_out);
    os_usbd_altsetting_add_endpoint(winusb_setting, winusb_device->ep_in);

    /* Add the alternate setting to the interface, then set default setting */
    os_usbd_interface_add_altsetting(winusb_intf, winusb_setting);
    os_usbd_set_altsetting(winusb_intf, 0);

    /* Add the interface to the mass storage function */
    os_usbd_function_add_interface(func, winusb_intf);

    os_usbd_os_comp_id_desc_add_os_func_comp_id_desc(device->os_comp_id_desc, &winusb_func_comp_id_desc);
    /* Initilize winusb */
    os_usb_winusb_init(func);
    return func;
}

struct udclass winusb_class = {.os_usbd_function_create = os_usbd_function_winusb_create};

int os_usbd_winusb_class_register(void)
{
    os_usbd_class_register(&winusb_class);
    return 0;
}
OS_PREV_INIT(os_usbd_winusb_class_register);
