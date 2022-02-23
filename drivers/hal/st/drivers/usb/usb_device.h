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
 * @file        usb_device.h
 *
 * @brief       This file provides struct/enum definition and usb device functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#include <os_task.h>
#include <string.h>
#include <device.h>
#include <os_errno.h>
#include <os_mq.h>
#include <os_assert.h>
#include <os_memory.h>
#include <os_event.h>
#include <ring_buff.h>
#include "usb/usb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Vendor ID */
#ifdef USB_VENDOR_ID
#define _VENDOR_ID USB_VENDOR_ID
#else
#define _VENDOR_ID 0x0EFF
#endif
/* Product ID */
#ifdef USB_PRODUCT_ID
#define _PRODUCT_ID USB_PRODUCT_ID
#else
#define _PRODUCT_ID 0x0001
#endif

#define USB_BCD_DEVICE  0x0200 /* USB Specification Release Number in Binary-Coded Decimal */
#define USB_BCD_VERSION 0x0200 /* USB 2.0 */
#define EP0_IN_ADDR     0x80
#define EP0_OUT_ADDR    0x00
#define EP_HANDLER(ep, func, size)                                                                                     \
    OS_ASSERT(ep != OS_NULL);                                                                                          \
    ep->handler(func, size)
#define EP_ADDRESS(ep)   ep->ep_desc->bEndpointAddress
#define EP_MAXPACKET(ep) ep->ep_desc->wMaxPacketSize
#define FUNC_ENABLE(func)                                                                                              \
    do                                                                                                                 \
    {                                                                                                                  \
        if (func->ops->enable != OS_NULL && func->enabled == OS_FALSE)                                                 \
        {                                                                                                              \
            if (func->ops->enable(func) == OS_EOK)                                                                     \
                func->enabled = OS_TRUE;                                                                               \
        }                                                                                                              \
    } while (0)
#define FUNC_DISABLE(func)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        if (func->ops->disable != OS_NULL && func->enabled == OS_TRUE)                                                 \
        {                                                                                                              \
            func->enabled = OS_FALSE;                                                                                  \
            func->ops->disable(func);                                                                                  \
        }                                                                                                              \
    } while (0)

struct ufunction;
struct udevice;
struct uendpoint;

typedef enum
{
    /* Request to read full count */
    UIO_REQUEST_READ_FULL,
    /* Request to read any count */
    UIO_REQUEST_READ_BEST,
    /* Request to write full count */
    UIO_REQUEST_WRITE,
} UIO_REQUEST_TYPE;

struct udcd_ops
{
    os_err_t (*set_address)(os_uint8_t address);
    os_err_t (*set_config)(os_uint8_t address);
    os_err_t (*ep_set_stall)(os_uint8_t address);
    os_err_t (*ep_clear_stall)(os_uint8_t address);
    os_err_t (*ep_enable)(struct uendpoint *ep);
    os_err_t (*ep_disable)(struct uendpoint *ep);
    os_size_t (*ep_read_prepare)(os_uint8_t address, void *buffer, os_size_t size);
    os_size_t (*ep_read)(os_uint8_t address, void *buffer);
    os_size_t (*ep_write)(os_uint8_t address, void *buffer, os_size_t size);
    os_err_t (*ep0_send_status)(void);
    os_err_t (*suspend)(void);
    os_err_t (*wakeup)(void);
};

struct ep_id
{
    os_uint8_t  addr;
    os_uint8_t  type;
    os_uint8_t  dir;
    os_uint16_t maxpacket;
    os_uint8_t  status;
};

typedef os_err_t (*udep_handler_t)(struct ufunction *func, os_size_t size);

struct uio_request
{
    os_list_node_t   list;
    UIO_REQUEST_TYPE req_type;
    os_uint8_t      *buffer;
    os_size_t        size;
    os_size_t        remain_size;
};
typedef struct uio_request *uio_request_t;

struct uendpoint
{
    os_list_node_t     list;
    uep_desc_t         ep_desc;
    os_list_node_t     request_list;
    struct uio_request request;
    os_uint8_t        *buffer;
    os_bool_t          stalled;
    struct ep_id      *id;
    udep_handler_t     handler;
    os_err_t (*rx_indicate)(struct udevice *dev, os_size_t size);
};
typedef struct uendpoint *uep_t;

struct udcd
{
    struct os_device       parent;
    const struct udcd_ops *ops;
    struct uendpoint       ep0;
    uep0_stage_t           stage;
    struct ep_id          *ep_pool;
    os_uint8_t             device_is_hs;
};
typedef struct udcd *udcd_t;

struct ualtsetting
{
    os_list_node_t list;
    uintf_desc_t   intf_desc;
    void          *desc;
    os_size_t      desc_size;
    os_list_node_t ep_list;
};
typedef struct ualtsetting *ualtsetting_t;

typedef os_err_t (*uintf_handler_t)(struct ufunction *func, ureq_t setup);

struct uinterface
{
    os_list_node_t  list;
    os_uint8_t      intf_num;
    ualtsetting_t   curr_setting;
    os_list_node_t  setting_list;
    uintf_handler_t handler;
};
typedef struct uinterface *uintf_t;

struct ufunction_ops
{
    os_err_t (*enable)(struct ufunction *func);
    os_err_t (*disable)(struct ufunction *func);
    os_err_t (*sof_handler)(struct ufunction *func);
};
typedef struct ufunction_ops *ufunction_ops_t;

struct ufunction
{
    os_list_node_t  list;
    ufunction_ops_t ops;
    struct udevice *device;
    udev_desc_t     dev_desc;
    void           *user_data;
    os_bool_t       enabled;

    os_list_node_t intf_list;
};
typedef struct ufunction *ufunction_t;

struct uconfig
{
    os_list_node_t            list;
    struct uconfig_descriptor cfg_desc;
    os_list_node_t            func_list;
};
typedef struct uconfig *uconfig_t;

struct udevice
{
    os_list_node_t            list;
    struct udevice_descriptor dev_desc;

    struct usb_qualifier_descriptor *dev_qualifier;
    usb_os_comp_id_desc_t            os_comp_id_desc;
    const char **                    str;

    udevice_state_t state;
    os_list_node_t  cfg_list;
    uconfig_t       curr_cfg;
    os_uint8_t      nr_intf;

    udcd_t dcd;
};
typedef struct udevice *udevice_t;

struct udclass
{
    os_list_node_t list;
    ufunction_t (*os_usbd_function_create)(udevice_t device);
};
typedef struct udclass *udclass_t;

enum udev_msg_type
{
    USB_MSG_SETUP_NOTIFY,
    USB_MSG_DATA_NOTIFY,
    USB_MSG_EP0_OUT,
    USB_MSG_EP_CLEAR_FEATURE,
    USB_MSG_SOF,
    USB_MSG_RESET,
    USB_MSG_PLUG_IN,
    /*
     * We don't need to add a "PLUG_IN" event because after the cable is
     * plugged in(before any SETUP) the classed have nothing to do. If the host
     * is ready, it will send RESET and we will have USB_MSG_RESET. So, a RESET
     * should reset and run the class while plug_in is not.
     */
    USB_MSG_PLUG_OUT,
};
typedef enum udev_msg_type udev_msg_type;

struct ep_msg
{
    os_size_t  size;
    os_uint8_t ep_addr;
};

struct udev_msg
{
    udev_msg_type type;
    udcd_t        dcd;
    union
    {
        struct ep_msg   ep_msg;
        struct urequest setup;
    } content;
};
typedef struct udev_msg *udev_msg_t;

int           os_usbd_class_list_init(void);
udevice_t     os_usbd_device_new(void);
uconfig_t     os_usbd_config_new(void);
ufunction_t   os_usbd_function_new(udevice_t device, udev_desc_t dev_desc, ufunction_ops_t ops);
uintf_t       os_usbd_interface_new(udevice_t device, uintf_handler_t handler);
uep_t         os_usbd_endpoint_new(uep_desc_t ep_desc, udep_handler_t handler);
ualtsetting_t os_usbd_altsetting_new(os_size_t desc_size);

os_err_t os_usbd_core_init(void);
os_err_t os_usb_device_init(void);
os_err_t os_usbd_event_signal(struct udev_msg *msg);
os_err_t os_usbd_device_set_controller(udevice_t device, udcd_t dcd);
os_err_t os_usbd_device_set_descriptor(udevice_t device, udev_desc_t dev_desc);
os_err_t os_usbd_device_set_string(udevice_t device, const char **ustring);
os_err_t os_usbd_device_set_qualifier(udevice_t device, struct usb_qualifier_descriptor *qualifier);
os_err_t os_usbd_device_set_os_comp_id_desc(udevice_t device, usb_os_comp_id_desc_t os_comp_id_desc);
os_err_t os_usbd_device_add_config(udevice_t device, uconfig_t cfg);
os_err_t os_usbd_config_add_function(uconfig_t cfg, ufunction_t func);
os_err_t os_usbd_class_register(udclass_t udclass);
os_err_t os_usbd_function_add_interface(ufunction_t func, uintf_t intf);
os_err_t os_usbd_interface_add_altsetting(uintf_t intf, ualtsetting_t setting);
os_err_t os_usbd_altsetting_add_endpoint(ualtsetting_t setting, uep_t ep);
os_err_t os_usbd_os_comp_id_desc_add_os_func_comp_id_desc(usb_os_comp_id_desc_t      os_comp_id_desc,
                                                          usb_os_func_comp_id_desc_t os_func_comp_id_desc);
os_err_t os_usbd_altsetting_config_descriptor(ualtsetting_t setting, const void *desc, os_off_t intf_pos);
os_err_t os_usbd_set_config(udevice_t device, os_uint8_t value);
os_err_t os_usbd_set_altsetting(uintf_t intf, os_uint8_t value);

udevice_t os_usbd_find_device(udcd_t dcd);
uconfig_t os_usbd_find_config(udevice_t device, os_uint8_t value);
uintf_t   os_usbd_find_interface(udevice_t device, os_uint8_t value, ufunction_t *pfunc);
uep_t     os_usbd_find_endpoint(udevice_t device, ufunction_t *pfunc, os_uint8_t ep_addr);
os_size_t os_usbd_io_request(udevice_t device, uep_t ep, uio_request_t req);
os_size_t os_usbd_ep0_write(udevice_t device, void *buffer, os_size_t size);
os_size_t
os_usbd_ep0_read(udevice_t device, void *buffer, os_size_t size, os_err_t (*rx_ind)(udevice_t device, os_size_t size));

int os_usbd_vcom_class_register(void);
int os_usbd_ecm_class_register(void);
int os_usbd_hid_class_register(void);
int os_usbd_msc_class_register(void);
int os_usbd_rndis_class_register(void);
int os_usbd_winusb_class_register(void);

#ifdef OS_USB_DEVICE_COMPOSITE
os_err_t os_usbd_function_set_iad(ufunction_t func, uiad_desc_t iad_desc);
#endif

os_err_t os_usbd_set_feature(udevice_t device, os_uint16_t value, os_uint16_t index);
os_err_t os_usbd_clear_feature(udevice_t device, os_uint16_t value, os_uint16_t index);
os_err_t os_usbd_ep_set_stall(udevice_t device, uep_t ep);
os_err_t os_usbd_ep_clear_stall(udevice_t device, uep_t ep);
os_err_t os_usbd_ep0_set_stall(udevice_t device);
os_err_t os_usbd_ep0_clear_stall(udevice_t device);
os_err_t os_usbd_ep0_setup_handler(udcd_t dcd, struct urequest *setup);
os_err_t os_usbd_ep0_in_handler(udcd_t dcd);
os_err_t os_usbd_ep0_out_handler(udcd_t dcd, os_size_t size);
os_err_t os_usbd_ep_in_handler(udcd_t dcd, os_uint8_t address, os_size_t size);
os_err_t os_usbd_ep_out_handler(udcd_t dcd, os_uint8_t address, os_size_t size);
os_err_t os_usbd_reset_handler(udcd_t dcd);
os_err_t os_usbd_connect_handler(udcd_t dcd);
os_err_t os_usbd_disconnect_handler(udcd_t dcd);
os_err_t os_usbd_sof_handler(udcd_t dcd);

OS_INLINE os_err_t dcd_set_address(udcd_t dcd, os_uint8_t address)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->set_address != OS_NULL);

    return dcd->ops->set_address(address);
}

OS_INLINE os_err_t dcd_set_config(udcd_t dcd, os_uint8_t address)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->set_config != OS_NULL);

    return dcd->ops->set_config(address);
}

OS_INLINE os_err_t dcd_ep_enable(udcd_t dcd, uep_t ep)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->ep_enable != OS_NULL);

    return dcd->ops->ep_enable(ep);
}

OS_INLINE os_err_t dcd_ep_disable(udcd_t dcd, uep_t ep)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->ep_disable != OS_NULL);

    return dcd->ops->ep_disable(ep);
}

OS_INLINE os_size_t dcd_ep_read_prepare(udcd_t dcd, os_uint8_t address, void *buffer, os_size_t size)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);

    if (dcd->ops->ep_read_prepare != OS_NULL)
    {
        return dcd->ops->ep_read_prepare(address, buffer, size);
    }
    else
    {
        return 0;
    }
}

OS_INLINE os_size_t dcd_ep_read(udcd_t dcd, os_uint8_t address, void *buffer)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);

    if (dcd->ops->ep_read != OS_NULL)
    {
        return dcd->ops->ep_read(address, buffer);
    }
    else
    {
        return 0;
    }
}

OS_INLINE os_size_t dcd_ep_write(udcd_t dcd, os_uint8_t address, void *buffer, os_size_t size)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->ep_write != OS_NULL);

    return dcd->ops->ep_write(address, buffer, size);
}

OS_INLINE os_err_t dcd_ep0_send_status(udcd_t dcd)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->ep0_send_status != OS_NULL);

    return dcd->ops->ep0_send_status();
}

OS_INLINE os_err_t dcd_ep_set_stall(udcd_t dcd, os_uint8_t address)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->ep_set_stall != OS_NULL);

    return dcd->ops->ep_set_stall(address);
}

OS_INLINE os_err_t dcd_ep_clear_stall(udcd_t dcd, os_uint8_t address)
{
    OS_ASSERT(dcd != OS_NULL);
    OS_ASSERT(dcd->ops != OS_NULL);
    OS_ASSERT(dcd->ops->ep_clear_stall != OS_NULL);

    return dcd->ops->ep_clear_stall(address);
}
OS_INLINE void usbd_os_proerty_descriptor_send(ufunction_t      func,
                                               ureq_t           setup,
                                               usb_os_proerty_t usb_os_proerty,
                                               os_uint8_t       number_of_proerty)
{
    struct usb_os_property_header header;
    static os_uint8_t            *data;
    os_uint8_t                   *pdata;
    os_uint8_t                    index, i;
    if (data == OS_NULL)
    {
        header.dwLength   = sizeof(struct usb_os_property_header);
        header.bcdVersion = 0x0100;
        header.wIndex     = 0x05;
        header.wCount     = number_of_proerty;
        for (index = 0; index < number_of_proerty; index++)
        {
            header.dwLength += usb_os_proerty[index].dwSize;
        }
        data = (os_uint8_t *)os_calloc(1, header.dwLength);
        OS_ASSERT(data != OS_NULL);
        pdata = data;
        memcpy((void *)pdata, (void *)&header, sizeof(struct usb_os_property_header));
        pdata += sizeof(struct usb_os_property_header);
        for (index = 0; index < number_of_proerty; index++)
        {
            memcpy((void *)pdata, (void *)&usb_os_proerty[index], 10);
            pdata += 10;
            for (i = 0; i < usb_os_proerty[index].wPropertyNameLength / 2; i++)
            {
                *pdata = usb_os_proerty[index].bPropertyName[i];
                pdata++;
                *pdata = 0;
                pdata++;
            }
            *((os_uint32_t *)pdata) = usb_os_proerty[index].dwPropertyDataLength;
            pdata += 4;
            for (i = 0; i < usb_os_proerty[index].dwPropertyDataLength / 2; i++)
            {
                *pdata = usb_os_proerty[index].bPropertyData[i];
                pdata++;
                *pdata = 0;
                pdata++;
            }
        }
    }
    os_usbd_ep0_write(func->device, data, setup->wLength);
}

#ifdef __cplusplus
}
#endif

#endif
