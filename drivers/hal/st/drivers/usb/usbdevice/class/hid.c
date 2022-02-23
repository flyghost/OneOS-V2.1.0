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
 * @file        hid.c
 *
 * @brief       This file provides functions for hid.
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

#include "usb/usb_common.h"
#include "usb/usb_device.h"

#include "hid.h"

#ifdef OS_USB_DEVICE_HID

struct hid_s
{
    struct os_device  parent;
    struct ufunction *func;
    uep_t             ep_in;
    uep_t             ep_out;
    int               status;
    os_uint16_t       protocol;
    os_uint8_t        report_buf[MAX_REPORT_SIZE];
    struct os_mq      hid_mq;
};

/* CustomHID_ConfigDescriptor */
OS_ALIGN(4)
const os_uint8_t _report_desc[]=
{
#ifdef OS_USB_DEVICE_HID_KEYBOARD
    USAGE_PAGE(1),      0x01,
    USAGE(1),           0x06,
    COLLECTION(1),      0x01,
    REPORT_ID(1),       HID_REPORT_ID_KEYBOARD1,

    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0xE0,
    USAGE_MAXIMUM(1),   0xE7,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_SIZE(1),     0x01,
    REPORT_COUNT(1),    0x08,
    INPUT(1),           0x02,
    REPORT_COUNT(1),    0x01,
    REPORT_SIZE(1),     0x08,
    INPUT(1),           0x01,


    REPORT_COUNT(1),    0x05,
    REPORT_SIZE(1),     0x01,
    USAGE_PAGE(1),      0x08,
    USAGE_MINIMUM(1),   0x01,
    USAGE_MAXIMUM(1),   0x05,
    OUTPUT(1),          0x02,
    REPORT_COUNT(1),    0x01,
    REPORT_SIZE(1),     0x03,
    OUTPUT(1),          0x01,


    REPORT_COUNT(1),    0x06,
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,
    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    INPUT(1),           0x00,
    END_COLLECTION(0),
#if OS_USB_DEVICE_HID_KEYBOARD_NUMBER>1
    /* keyboard2 */
    USAGE_PAGE(1),      0x01,
    USAGE(1),           0x06,
    COLLECTION(1),      0x01,
    REPORT_ID(1),       HID_REPORT_ID_KEYBOARD2,

    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0xE0,
    USAGE_MAXIMUM(1),   0xE7,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_SIZE(1),     0x01,
    REPORT_COUNT(1),    0x08,
    INPUT(1),           0x02,
    REPORT_COUNT(1),    0x01,
    REPORT_SIZE(1),     0x08,
    INPUT(1),           0x01,

    REPORT_COUNT(1),    0x06,
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,
    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    INPUT(1),           0x00,
    END_COLLECTION(0),
#if OS_USB_DEVICE_HID_KEYBOARD_NUMBER>2
    USAGE_PAGE(1),      0x01,
    USAGE(1),           0x06,
    COLLECTION(1),      0x01,
    REPORT_ID(1),       HID_REPORT_ID_KEYBOARD3,

    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0xE0,
    USAGE_MAXIMUM(1),   0xE7,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_SIZE(1),     0x01,
    REPORT_COUNT(1),    0x08,
    INPUT(1),           0x02,
    REPORT_COUNT(1),    0x01,
    REPORT_SIZE(1),     0x08,
    INPUT(1),           0x01,

    REPORT_COUNT(1),    0x06,
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,
    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    INPUT(1),           0x00,
    END_COLLECTION(0),
#if OS_USB_DEVICE_HID_KEYBOARD_NUMBER>3
    USAGE_PAGE(1),      0x01,
    USAGE(1),           0x06,
    COLLECTION(1),      0x01,
    REPORT_ID(1),       HID_REPORT_ID_KEYBOARD4,

    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0xE0,
    USAGE_MAXIMUM(1),   0xE7,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_SIZE(1),     0x01,
    REPORT_COUNT(1),    0x08,
    INPUT(1),           0x02,
    REPORT_COUNT(1),    0x01,
    REPORT_SIZE(1),     0x08,
    INPUT(1),           0x01,

    REPORT_COUNT(1),    0x06,
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x65,
    USAGE_PAGE(1),      0x07,
    USAGE_MINIMUM(1),   0x00,
    USAGE_MAXIMUM(1),   0x65,
    INPUT(1),           0x00,
    END_COLLECTION(0),
#endif
#endif
#endif
#endif
/* Media Control */
#ifdef OS_USB_DEVICE_HID_MEDIA
    USAGE_PAGE(1),      0x0C,
    USAGE(1),           0x01,
    COLLECTION(1),      0x01,
    REPORT_ID(1),       HID_REPORT_ID_MEDIA,
    USAGE_PAGE(1),      0x0C,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    REPORT_SIZE(1),     0x01,
    REPORT_COUNT(1),    0x07,
    USAGE(1),           0xB5,             // Next Track
    USAGE(1),           0xB6,             // Previous Track
    USAGE(1),           0xB7,             // Stop
    USAGE(1),           0xCD,             // Play / Pause
    USAGE(1),           0xE2,             // Mute
    USAGE(1),           0xE9,             // Volume Up
    USAGE(1),           0xEA,             // Volume Down
    INPUT(1),           0x02,             // Input (Data, Variable, Absolute)
    REPORT_COUNT(1),    0x01,
    INPUT(1),           0x01,
    END_COLLECTION(0),
#endif

#ifdef OS_USB_DEVICE_HID_GENERAL
    USAGE_PAGE(1),      0x8c,
    USAGE(1),           0x01,
    COLLECTION(1),      0x01,
    REPORT_ID(1),       HID_REPORT_ID_GENERAL,

    REPORT_COUNT(1),    OS_USB_DEVICE_HID_GENERAL_IN_REPORT_LENGTH,
    USAGE(1),           0x03,
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0xFF,
    INPUT(1),           0x02,

    REPORT_COUNT(1),    OS_USB_DEVICE_HID_GENERAL_OUT_REPORT_LENGTH,
    USAGE(1),           0x04,
    REPORT_SIZE(1),     0x08,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0xFF,
    OUTPUT(1),          0x02,
    END_COLLECTION(0),
#endif
#ifdef OS_USB_DEVICE_HID_MOUSE
    USAGE_PAGE(1),      0x01,           /* Generic Desktop */
    USAGE(1),           0x02,           /* Mouse */
    COLLECTION(1),      0x01,           /* Application */
    USAGE(1),           0x01,           /* Pointer */
    COLLECTION(1),      0x00,           /* Physical */
    REPORT_ID(1),       HID_REPORT_ID_MOUSE,
    REPORT_COUNT(1),    0x03,
    REPORT_SIZE(1),     0x01,
    USAGE_PAGE(1),      0x09,           /* Buttons */
    USAGE_MINIMUM(1),   0x1,
    USAGE_MAXIMUM(1),   0x3,
    LOGICAL_MINIMUM(1), 0x00,
    LOGICAL_MAXIMUM(1), 0x01,
    INPUT(1),           0x02,
    REPORT_COUNT(1),    0x01,
    REPORT_SIZE(1),     0x05,
    INPUT(1),           0x01,
    REPORT_COUNT(1),    0x03,
    REPORT_SIZE(1),     0x08,
    USAGE_PAGE(1),      0x01,
    USAGE(1),           0x30,           /* X */
    USAGE(1),           0x31,           /* Y */
    USAGE(1),           0x38,           /* scroll */
    LOGICAL_MINIMUM(1), 0x81,
    LOGICAL_MAXIMUM(1), 0x7f,
    INPUT(1),           0x06,
    END_COLLECTION(0),
    END_COLLECTION(0),
#endif
}; /* CustomHID_ReportDescriptor */

OS_ALIGN(4)
static struct udevice_descriptor _dev_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength; */
    USB_DESC_TYPE_DEVICE,     /* type; */
    USB_BCD_VERSION,          /* bcdUSB; */
    USB_CLASS_HID,            /* bDeviceClass; */
    0x00,                     /* bDeviceSubClass; */
    0x00,                     /* bDeviceProtocol; */
    64,                       /* bMaxPacketSize0; */
    _VENDOR_ID,               /* idVendor; */
    _PRODUCT_ID,              /* idProduct; */
    USB_BCD_DEVICE,           /* bcdDevice; */
    USB_STRING_MANU_INDEX,    /* iManufacturer; */
    USB_STRING_PRODUCT_INDEX, /* iProduct; */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber; */
    USB_DYNAMIC,              /* bNumConfigurations; */
};

#if 0
/* FS and HS needed */
OS_ALIGN(4)
static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),          /* bLength */
    USB_DESC_TYPE_DEVICEQUALIFIER,  /* bDescriptorType */
    0x0200,                         /* bcdUSB */
    USB_CLASS_MASS_STORAGE,         /* bDeviceClass */
    0x06,                           /* bDeviceSubClass */
    0x50,                           /* bDeviceProtocol */
    64,                             /* bMaxPacketSize0 */
    0x01,                           /* bNumConfigurations */
    0,
};
#endif

/* Hid interface descriptor */
OS_ALIGN(4)
const static struct uhid_comm_descriptor _hid_comm_desc = {
#ifdef OS_USB_DEVICE_COMPOSITE
    /* Interface Association Descriptor */
    {
        USB_DESC_LENGTH_IAD,
        USB_DESC_TYPE_IAD,
        USB_DYNAMIC,
        0x01,
        0x03, /* bInterfaceClass: HID */
#if defined(OS_USB_DEVICE_HID_KEYBOARD) || defined(OS_USB_DEVICE_HID_MOUSE)
        USB_HID_SUBCLASS_BOOT, /* bInterfaceSubClass : 1=BOOT, 0=no boot */
#else
        USB_HID_SUBCLASS_NOBOOT,   /* bInterfaceSubClass : 1=BOOT, 0=no boot */
#endif
#if !defined(OS_USB_DEVICE_HID_KEYBOARD) || !defined(OS_USB_DEVICE_HID_MOUSE) || !defined(OS_USB_DEVICE_HID_MEDIA)
        USB_HID_PROTOCOL_NONE, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
#elif !defined(OS_USB_DEVICE_HID_MOUSE)
        USB_HID_PROTOCOL_KEYBOARD, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
#else
        USB_HID_PROTOCOL_MOUSE, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
#endif
        0x00,
    },
#endif
    /* Interface Descriptor */
    {
        USB_DESC_LENGTH_INTERFACE,
        USB_DESC_TYPE_INTERFACE,
        USB_DYNAMIC, /* bInterfaceNumber: Number of Interface */
        0x00,        /* bAlternateSetting: Alternate setting */
        0x02,        /* bNumEndpoints */
        0x03,        /* bInterfaceClass: HID */
#if defined(OS_USB_DEVICE_HID_KEYBOARD) || defined(OS_USB_DEVICE_HID_MOUSE)
        USB_HID_SUBCLASS_BOOT, /* bInterfaceSubClass : 1=BOOT, 0=no boot */
#else
        USB_HID_SUBCLASS_NOBOOT,   /* bInterfaceSubClass : 1=BOOT, 0=no boot */
#endif
#if !defined(OS_USB_DEVICE_HID_KEYBOARD) || !defined(OS_USB_DEVICE_HID_MOUSE) || !defined(OS_USB_DEVICE_HID_MEDIA)
        USB_HID_PROTOCOL_NONE, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
#elif !defined(OS_USB_DEVICE_HID_MOUSE)
        USB_HID_PROTOCOL_KEYBOARD, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
#else
        USB_HID_PROTOCOL_MOUSE, /* nInterfaceProtocol : 0=none, 1=keyboard, 2=mouse */
#endif
        0, /* iInterface: Index of string descriptor */
    },

    /* HID Descriptor */
    {
        HID_DESCRIPTOR_SIZE, /* bLength: HID Descriptor size */
        HID_DESCRIPTOR_TYPE, /* bDescriptorType: HID */
        0x0110,              /* bcdHID: HID Class Spec release number */
        0x00,                /* bCountryCode: Hardware target country */
        0x01,                /* bNumDescriptors: Number of HID class descriptors to follow */
        {
            {
                0x22,                 /* bDescriptorType */
                sizeof(_report_desc), /* wItemLength: Total length of Report descriptor */
            },
        },
    },

    /* Endpoint Descriptor IN */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_IN,
        USB_EP_ATTR_INT,
        0x40,
        0x01,
    },

    /* Endpoint Descriptor OUT */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_OUT,
        USB_EP_ATTR_INT,
        0x40,
        0x01,
    },
};

OS_ALIGN(4)
const static char *_ustring[] =
{
    "Language",
    "OS Team.",
    "OS HID-Device",
    "32021919830108",
    "Configuration",
    "Interface",
};

static void dump_data(os_uint8_t *data, os_size_t size)
{
    os_size_t i;
    for (i = 0; i < size; i++)
    {
        os_kprintf("%02x ", *data++);
        if ((i + 1) % 8 == 0)
        {
            os_kprintf("\r\n");
        }
        else if ((i + 1) % 4 == 0)
        {
            os_kprintf(" ");
        }
    }
}
static void dump_report(struct hid_report *report)
{
    os_kprintf("\nHID Recived:");
    os_kprintf("\nReport ID %02x \r\n", report->report_id);
    dump_data(report->report, report->size);
}

static os_err_t _ep_out_handler(ufunction_t func, os_size_t size)
{
    struct hid_s     *data;
    struct hid_report report;
    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    data = (struct hid_s *)func->user_data;

    if (size != 0)
    {
        memcpy((void *)&report, (void *)data->ep_out->buffer, size);
        report.size = size - 1;
        os_mq_send(&data->hid_mq, (void *)&report, sizeof(report), OS_NO_WAIT);
    }

    data->ep_out->request.buffer   = data->ep_out->buffer;
    data->ep_out->request.size     = EP_MAXPACKET(data->ep_out);
    data->ep_out->request.req_type = UIO_REQUEST_READ_BEST;
    os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);
    return OS_EOK;
}

static os_err_t _ep_in_handler(ufunction_t func, os_size_t size)
{
    struct hid_s *hid;
    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);

    hid = (struct hid_s *)func->user_data;

    os_device_send_notify(&hid->parent);

    return OS_EOK;
}

static os_err_t _hid_set_report_callback(udevice_t device, os_size_t size)
{
    OS_DEBUG_LOG(OS_DEBUG_USB, ("_hid_set_report_callback\r\n"));

    if (size != 0)
    {
    }

    dcd_ep0_send_status(device->dcd);

    return OS_EOK;
}

static os_err_t _interface_handler(ufunction_t func, ureq_t setup)
{
    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    struct hid_s *data = (struct hid_s *)func->user_data;

    if (setup->wIndex != 0)
        return OS_EIO;

    switch (setup->bRequest)
    {
    case USB_REQ_GET_DESCRIPTOR:
        if ((setup->wValue >> 8) == USB_DESC_TYPE_REPORT)
        {
            os_usbd_ep0_write(func->device, (void *)(&_report_desc[0]), sizeof(_report_desc));
        }
        else if ((setup->wValue >> 8) == USB_DESC_TYPE_HID)
        {

            os_usbd_ep0_write(func->device, (void *)(&_hid_comm_desc.hid_desc), sizeof(struct uhid_descriptor));
        }
        break;
    case USB_HID_REQ_GET_REPORT:
        if (setup->wLength == 0)
        {
            os_usbd_ep0_set_stall(func->device);
            break;
        }
        if ((setup->wLength == 0) || (setup->wLength > MAX_REPORT_SIZE))
            setup->wLength = MAX_REPORT_SIZE;
        os_usbd_ep0_write(func->device, data->report_buf, setup->wLength);
        break;
    case USB_HID_REQ_GET_IDLE:

        dcd_ep0_send_status(func->device->dcd);
        break;
    case USB_HID_REQ_GET_PROTOCOL:
        os_usbd_ep0_write(func->device, &data->protocol, 2);
        break;
    case USB_HID_REQ_SET_REPORT:

        if ((setup->wLength == 0) || (setup->wLength > MAX_REPORT_SIZE))
            os_usbd_ep0_set_stall(func->device);

        os_usbd_ep0_read(func->device, data->report_buf, setup->wLength, _hid_set_report_callback);
        break;
    case USB_HID_REQ_SET_IDLE:
        dcd_ep0_send_status(func->device->dcd);
        break;
    case USB_HID_REQ_SET_PROTOCOL:
        data->protocol = setup->wValue;

        dcd_ep0_send_status(func->device->dcd);
        break;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will run cdc function, it will be called on handle set configuration bRequest.
 *
 * @param[in]       func           The usb function object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _function_enable(ufunction_t func)
{
    struct hid_s *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    data = (struct hid_s *)func->user_data;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("hid function enable\r\n"));
    /*
    _vcom_reset_state(func);
    */
    if (data->ep_out->buffer == OS_NULL)
    {
        data->ep_out->buffer = os_calloc(1, HID_RX_BUFSIZE);
    }
    data->ep_out->request.buffer   = data->ep_out->buffer;
    data->ep_out->request.size     = EP_MAXPACKET(data->ep_out);
    data->ep_out->request.req_type = UIO_REQUEST_READ_BEST;

    os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will stop cdc function, it will be called on handle set configuration bRequest.
 *
 * @param[in]       func           The usb function object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _function_disable(ufunction_t func)
{
    struct hid_s *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    data = (struct hid_s *)func->user_data;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("hid function disable\r\n"));

    if (data->ep_out->buffer != OS_NULL)
    {
        os_free(data->ep_out->buffer);
        data->ep_out->buffer = OS_NULL;
    }

    return OS_EOK;
}

static struct ufunction_ops ops =
{
    _function_enable,
    _function_disable,
    OS_NULL,
};

static os_err_t _hid_descriptor_config(uhid_comm_desc_t hid, os_uint8_t cintf_nr)
{
#ifdef OS_USB_DEVICE_COMPOSITE
    hid->iad_desc.bFirstInterface = cintf_nr;
#endif

    return OS_EOK;
}

static os_size_t _hid_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct hid_s     *hiddev = (struct hid_s *)dev;
    struct hid_report report;
    
    if (hiddev->func->device->state == USB_STATE_CONFIGURED)
    {
        report.report_id = pos;
        memcpy((void *)report.report, (void *)buffer, size);
        report.size                     = size;
        hiddev->ep_in->request.buffer   = (void *)&report;
        hiddev->ep_in->request.size     = (size + 1) > 64 ? 64 : size + 1;
        hiddev->ep_in->request.req_type = UIO_REQUEST_WRITE;
        os_usbd_io_request(hiddev->func->device, hiddev->ep_in, &hiddev->ep_in->request);
        return size;
    }

    return 0;
}
OS_WEAK void HID_Report_Received(hid_report_t report)
{
    dump_report(report);
}
OS_ALIGN(OS_ALIGN_SIZE)
static os_uint8_t     hid_task_stack[512];
static struct os_task hid_task;

static void hid_task_entry(void *parameter)
{
    struct hid_report report;
    struct hid_s     *hiddev;
    os_size_t         recv_len;
    hiddev = (struct hid_s *)parameter;
    while (1)
    {
        if (os_mq_recv(&hiddev->hid_mq, &report, sizeof(report), OS_WAIT_FOREVER, &recv_len) != OS_EOK)
            continue;
        HID_Report_Received(&report);
    }
}

const static struct os_device_ops hid_device_ops =
{
    .write = _hid_write,
};

static os_uint8_t hid_mq_pool[(sizeof(struct hid_report) + sizeof(void *)) * 8];
static void       os_usb_hid_init(struct ufunction *func)
{
    struct hid_s *hiddev;
    hiddev = (struct hid_s *)func->user_data;
    memset(&hiddev->parent, 0, sizeof(hiddev->parent));

    hiddev->parent.ops = &hid_device_ops;
    hiddev->func = func;

    os_device_register(&hiddev->parent, "hidd");
    os_mq_init(&hiddev->hid_mq,
               "hiddmq",
               hid_mq_pool,
               sizeof(hid_mq_pool),
               sizeof(struct hid_report));

    os_task_init(&hid_task,
                 "hidd",
                 hid_task_entry,
                 hiddev,
                 hid_task_stack,
                 sizeof(hid_task_stack),
                 OS_USBD_TASK_PRIO);
    os_task_startup(&hid_task);
}

ufunction_t os_usbd_function_hid_create(udevice_t device)
{
    ufunction_t   func;
    struct hid_s *data;

    uintf_t          hid_intf;
    ualtsetting_t    hid_setting;
    uhid_comm_desc_t hid_desc;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Set usb device string description */
    os_usbd_device_set_string(device, _ustring);

    /* Create a cdc function */
    func = os_usbd_function_new(device, &_dev_desc, &ops);
    /* Not support hs */
    // os_usbd_device_set_qualifier(device, &_dev_qualifier);

    /* Allocate memory for cdc vcom data */
    data = (struct hid_s *)os_calloc(1, sizeof(struct hid_s));
    memset(data, 0, sizeof(struct hid_s));
    func->user_data = (void *)data;

    /* Create an interface object */
    hid_intf = os_usbd_interface_new(device, _interface_handler);

    /* Create an alternate setting object */
    hid_setting = os_usbd_altsetting_new(sizeof(struct uhid_comm_descriptor));

    /* Config desc in alternate setting */
    os_usbd_altsetting_config_descriptor(hid_setting, &_hid_comm_desc, (os_off_t) & ((uhid_comm_desc_t)0)->intf_desc);

    /* Configure the hid interface descriptor */
    _hid_descriptor_config(hid_setting->desc, hid_intf->intf_num);

    /* Create endpoint */
    hid_desc     = (uhid_comm_desc_t)hid_setting->desc;
    data->ep_out = os_usbd_endpoint_new(&hid_desc->ep_out_desc, _ep_out_handler);
    data->ep_in  = os_usbd_endpoint_new(&hid_desc->ep_in_desc, _ep_in_handler);

    /* Add the int out and int in endpoint to the alternate setting */
    os_usbd_altsetting_add_endpoint(hid_setting, data->ep_out);
    os_usbd_altsetting_add_endpoint(hid_setting, data->ep_in);

    /* Add the alternate setting to the interface, then set default setting */
    os_usbd_interface_add_altsetting(hid_intf, hid_setting);
    os_usbd_set_altsetting(hid_intf, 0);

    /* Add the interface to the mass storage function */
    os_usbd_function_add_interface(func, hid_intf);

    /* Initilize hid */
    os_usb_hid_init(func);
    return func;
}
struct udclass hid_class = {.os_usbd_function_create = os_usbd_function_hid_create};

int os_usbd_hid_class_register(void)
{
    os_usbd_class_register(&hid_class);
    return 0;
}
OS_DEVICE_INIT(os_usbd_hid_class_register,OS_INIT_SUBLEVEL_MIDDLE);
#endif /* OS_USB_DEVICE_HID */
