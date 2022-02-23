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
 * @file        usbdevice.c
 *
 * @brief       This file provides functions for usb device.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <drv_cfg.h>
#include <os_list.h>

#ifdef OS_USING_USB_DEVICE

#define USB_DEVICE_CONTROLLER_NAME "usbd"

#ifdef OS_USB_DEVICE_COMPOSITE
const static char* ustring[] =
{
    "Language",
    "OS Team.",
    "OS Composite Device",
    "320219198301",
    "Configuration",
    "Interface",
    USB_STRING_OS
};

static struct udevice_descriptor compsit_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength; */
    USB_DESC_TYPE_DEVICE,     /* type; */
    USB_BCD_VERSION,          /* bcdUSB; */
    USB_CLASS_MISC,           /* bDeviceClass; */
    0x02,                     /* bDeviceSubClass; */
    0x01,                     /* bDeviceProtocol; */
    0x40,                     /* bMaxPacketSize0; */
    _VENDOR_ID,               /* idVendor; */
    _PRODUCT_ID,              /* idProduct; */
    USB_BCD_DEVICE,           /* bcdDevice; */
    USB_STRING_MANU_INDEX,    /* iManufacturer; */
    USB_STRING_PRODUCT_INDEX, /* iProduct; */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber; */
    USB_DYNAMIC,              /* bNumConfigurations; */
};

static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),         /* bLength */
    USB_DESC_TYPE_DEVICEQUALIFIER, /* bDescriptorType */
    0x0200,                        /* bcdUSB */
    USB_CLASS_MISC,                /* bDeviceClass */
    0x02,                          /* bDeviceSubClass */
    0x01,                          /* bDeviceProtocol */
    64,                            /* bMaxPacketSize0 */
    0x01,                          /* bNumConfigurations */
    0,
};
#endif

struct usb_os_comp_id_descriptor usb_comp_id_desc =
{
    {
        USB_DYNAMIC,
        0x0100,
        0x04,
        USB_DYNAMIC,
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    },
};
static os_list_node_t class_list;
int                   os_usbd_class_list_init(void)
{
    os_list_init(&class_list);
    return 0;
}
OS_DEVICE_INIT(os_usbd_class_list_init,OS_INIT_SUBLEVEL_HIGH);

os_err_t os_usbd_class_register(udclass_t udclass)
{
#ifndef OS_USB_DEVICE_COMPOSITE
    if (!os_list_empty(&class_list))
    {
        os_kprintf("[D/USBD] If you want to use usb composite device please define OS_USB_DEVICE_COMPOSITE\r\n");
        return OS_ERROR;
    }
#endif
    os_list_add(&class_list, &udclass->list);
    return OS_EOK;
}

os_err_t os_usb_device_init(void)
{
    os_device_t    *udc;
    udevice_t       udevice;
    uconfig_t       cfg;
    ufunction_t     func;
    os_list_node_t *i;
    udclass_t       udclass;

    if (os_list_empty(&class_list))
    {
        os_kprintf("[D/USBD] No class register on usb device\r\n");
        return OS_ERROR;
    }
    /* Create and startup usb device task */
    os_usbd_core_init();

    /* Create a device object */
    udevice = os_usbd_device_new();

    udc = os_device_find(USB_DEVICE_CONTROLLER_NAME);
    if (udc == OS_NULL)
    {
        os_kprintf("can't find usb device controller %s\r\n", USB_DEVICE_CONTROLLER_NAME);
        return OS_ERROR;
    }

    /* Set usb controller driver to the device */
    os_usbd_device_set_controller(udevice, (udcd_t)udc);

    /* Create a configuration object */
    cfg = os_usbd_config_new();

    os_usbd_device_set_os_comp_id_desc(udevice, &usb_comp_id_desc);

    for (i = class_list.next; i != &class_list; i = i->next)
    {
        /* Get a class creater */
        udclass = os_list_entry(i, struct udclass, list);
        /* Create a function object */
        func = udclass->os_usbd_function_create(udevice);
        /* Add the function to the configuration */
        os_usbd_config_add_function(cfg, func);
    }
    /* Set device descriptor to the device */
#ifdef OS_USB_DEVICE_COMPOSITE
    os_usbd_device_set_descriptor(udevice, &compsit_desc);
    os_usbd_device_set_string(udevice, ustring);
    if (udevice->dcd->device_is_hs)
    {
        os_usbd_device_set_qualifier(udevice, &dev_qualifier);
    }
#else
    os_usbd_device_set_descriptor(udevice, func->dev_desc);
#endif

    /* Add the configuration to the device */
    os_usbd_device_add_config(udevice, cfg);

    /* Initialize usb device controller */
    os_device_open(udc);

    /* Set default configuration to 1 */
    os_usbd_set_config(udevice, 1);

    return OS_EOK;
}
#endif
