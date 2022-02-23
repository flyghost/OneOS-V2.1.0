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
 * @file        core.c
 *
 * @brief       This file provides functions for usb device.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include "usb/usb_common.h"
#include "usb/usb_device.h"

static os_list_node_t device_list;

static os_size_t os_usbd_ep_write(udevice_t device, uep_t ep, void *buffer, os_size_t size);
static os_size_t os_usbd_ep_read_prepare(udevice_t device, uep_t ep, void *buffer, os_size_t size);
static os_err_t  os_usbd_ep_assign(udevice_t device, uep_t ep);
os_err_t         os_usbd_ep_unassign(udevice_t device, uep_t ep);

/**
 ***********************************************************************************************************************
 * @brief           This function will handle get_device_descriptor bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _get_device_descriptor(struct udevice *device, ureq_t setup)
{
    os_size_t size;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_get_device_descriptor\r\n"));

    /* Device descriptor wLength should less than USB_DESC_LENGTH_DEVICE*/
    size = (setup->wLength > USB_DESC_LENGTH_DEVICE) ? USB_DESC_LENGTH_DEVICE : setup->wLength;

    /* Send device descriptor to endpoint 0 */
    os_usbd_ep0_write(device, (os_uint8_t *)&device->dev_desc, size);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle get_config_descriptor bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _get_config_descriptor(struct udevice *device, ureq_t setup)
{
    os_size_t   size;
    ucfg_desc_t cfg_desc;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_get_config_descriptor\r\n"));

    cfg_desc = &device->curr_cfg->cfg_desc;
    size     = (setup->wLength > cfg_desc->wTotalLength) ? cfg_desc->wTotalLength : setup->wLength;

    /* Send configuration descriptor to endpoint 0 */
    os_usbd_ep0_write(device, (os_uint8_t *)cfg_desc, size);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle get_string_descriptor bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Invalid bRequest.
 ***********************************************************************************************************************
 */
static os_err_t _get_string_descriptor(struct udevice *device, ureq_t setup)
{
    struct ustring_descriptor str_desc;
    os_uint8_t                index, i;
    os_uint32_t               len;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_get_string_descriptor\r\n"));

    str_desc.type = USB_DESC_TYPE_STRING;
    index         = setup->wValue & 0xFF;

    if (index == 0xEE)
    {
        index = USB_STRING_OS_INDEX;
    }

    if (index > USB_STRING_MAX)
    {
        os_kprintf("unknown string index\r\n");
        os_usbd_ep0_set_stall(device);
        return OS_ERROR;
    }
    else if (index == USB_STRING_LANGID_INDEX)
    {
        str_desc.bLength   = 4;
        str_desc.String[0] = 0x09;
        str_desc.String[1] = 0x04;
    }
    else
    {
        len              = strlen(device->str[index]);
        str_desc.bLength = len * 2 + 2;

        for (i = 0; i < len; i++)
        {
            str_desc.String[i * 2]     = device->str[index][i];
            str_desc.String[i * 2 + 1] = 0;
        }
    }

    if (setup->wLength > str_desc.bLength)
        len = str_desc.bLength;
    else
        len = setup->wLength;

    /* Send string descriptor to endpoint 0 */
    os_usbd_ep0_write(device, (os_uint8_t *)&str_desc, len);

    return OS_EOK;
}

static os_err_t _get_qualifier_descriptor(struct udevice *device, ureq_t setup)
{
    OS_DEBUG_LOG(OS_DEBUG_USB, ("_get_qualifier_descriptor\r\n"));

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    if (device->dev_qualifier && device->dcd->device_is_hs)
    {
        /* Send device qualifier descriptor to endpoint 0 */
        os_usbd_ep0_write(device, (os_uint8_t *)device->dev_qualifier, sizeof(struct usb_qualifier_descriptor));
    }
    else
    {
        os_usbd_ep0_set_stall(device);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle get_descriptor bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _get_descriptor(struct udevice *device, ureq_t setup)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    if (setup->request_type == USB_REQ_TYPE_DIR_IN)
    {
        switch (setup->wValue >> 8)
        {
        case USB_DESC_TYPE_DEVICE:
            _get_device_descriptor(device, setup);
            break;
        case USB_DESC_TYPE_CONFIGURATION:
            _get_config_descriptor(device, setup);
            break;
        case USB_DESC_TYPE_STRING:
            _get_string_descriptor(device, setup);
            break;
        case USB_DESC_TYPE_DEVICEQUALIFIER:
            /*
             * If a full-speed only device (with a device descriptor version number equal to 0200H) receives a
             * GetDescriptor() request for a device_qualifier, it must respond with a request error. The host must
             * not make a request for an other_speed_configuration descriptor unless it first successfully
             * retrieves the device_qualifier descriptor.
             */
            if (device->dcd->device_is_hs)
            {
                _get_qualifier_descriptor(device, setup);
            }
            else
            {
                os_usbd_ep0_set_stall(device);
            }
            break;
        case USB_DESC_TYPE_OTHERSPEED:
            _get_config_descriptor(device, setup);
            break;
        default:
            os_kprintf("unsupported descriptor request\r\n");
            os_usbd_ep0_set_stall(device);
            break;
        }
    }
    else
    {
        os_kprintf("request direction error\r\n");
        os_usbd_ep0_set_stall(device);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle get_interface bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _get_interface(struct udevice *device, ureq_t setup)
{
    os_uint8_t  value;
    uintf_t     intf;
    ufunction_t func;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_get_interface\r\n"));

    if (device->state != USB_STATE_CONFIGURED)
    {
        os_usbd_ep0_set_stall(device);
        return OS_ERROR;
    }

    /* Find the specified interface and its alternate setting */
    intf  = os_usbd_find_interface(device, setup->wIndex & 0xFF, &func);
    value = intf->curr_setting->intf_desc->bAlternateSetting;

    /* Send the interface alternate setting to endpoint 0 */
    os_usbd_ep0_write(device, &value, 1);

    if (intf->handler)
    {
        intf->handler(func, setup);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle set_interface bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _set_interface(struct udevice *device, ureq_t setup)
{
    ufunction_t          func;
    uintf_t              intf;
    uep_t                ep;
    struct os_list_node *i;
    ualtsetting_t        setting;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_set_interface\r\n"));

    if (device->state != USB_STATE_CONFIGURED)
    {
        os_usbd_ep0_set_stall(device);
        return OS_ERROR;
    }

    /* Find the specified interface */
    intf = os_usbd_find_interface(device, setup->wIndex & 0xFF, &func);

    /* Set alternate setting to the interface */
    os_usbd_set_altsetting(intf, setup->wValue & 0xFF);
    setting = intf->curr_setting;

    /* Start all endpoints of the interface alternate setting */
    for (i = setting->ep_list.next; i != &setting->ep_list; i = i->next)
    {
        ep = (uep_t)os_list_entry(i, struct uendpoint, list);
        dcd_ep_disable(device->dcd, ep);
        dcd_ep_enable(device->dcd, ep);
    }
    dcd_ep0_send_status(device->dcd);

    if (intf->handler)
    {
        intf->handler(func, setup);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle get_config bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _get_config(struct udevice *device, ureq_t setup)
{
    os_uint8_t value;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);
    OS_ASSERT(device->curr_cfg != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_get_config\r\n"));

    if (device->state == USB_STATE_CONFIGURED)
    {
        /* Get current configuration */
        value = device->curr_cfg->cfg_desc.bConfigurationValue;
    }
    else
    {
        value = 0;
    }
    /* Write the current configuration to endpoint 0 */
    os_usbd_ep0_write(device, &value, 1);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle set_config bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Invalid bRequest.
 ***********************************************************************************************************************
 */
static os_err_t _set_config(struct udevice *device, ureq_t setup)
{
    struct os_list_node *i, *j, *k;
    uconfig_t            cfg;
    uintf_t              intf;
    ualtsetting_t        setting;
    uep_t                ep;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_set_config\r\n"));

    if (setup->wValue > device->dev_desc.bNumConfigurations)
    {
        os_usbd_ep0_set_stall(device);
        return OS_ERROR;
    }

    if (setup->wValue == 0)
    {
        OS_DEBUG_LOG(OS_DEBUG_USB, ("address state\r\n"));
        device->state = USB_STATE_ADDRESS;

        goto _exit;
    }

    /* Set current configuration */
    os_usbd_set_config(device, setup->wValue);
    cfg = device->curr_cfg;

    for (i = cfg->func_list.next; i != &cfg->func_list; i = i->next)
    {
        /* Run all functiones and their endpoints in the configuration */
        ufunction_t func = (ufunction_t)os_list_entry(i, struct ufunction, list);
        for (j = func->intf_list.next; j != &func->intf_list; j = j->next)
        {
            intf    = (uintf_t)os_list_entry(j, struct uinterface, list);
            setting = intf->curr_setting;
            for (k = setting->ep_list.next; k != &setting->ep_list; k = k->next)
            {
                ep = (uep_t)os_list_entry(k, struct uendpoint, list);

                /* First disable then enable an endpoint */
                dcd_ep_disable(device->dcd, ep);
                dcd_ep_enable(device->dcd, ep);
            }
        }
        /* After enabled endpoints, then enable function */
        FUNC_ENABLE(func);
    }

    device->state = USB_STATE_CONFIGURED;

_exit:
    /* Issue status stage */
    dcd_ep0_send_status(device->dcd);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle set_address bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _set_address(struct udevice *device, ureq_t setup)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    /* Set address in device control driver */
    dcd_set_address(device->dcd, setup->wValue);

    /* Issue status stage */
    dcd_ep0_send_status(device->dcd);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_set_address\r\n"));

    device->state = USB_STATE_ADDRESS;

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle standard bRequest to interface that defined in function-specifics.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Invalid bRequest.
 ***********************************************************************************************************************
 */
static os_err_t _request_interface(struct udevice *device, ureq_t setup)
{
    uintf_t     intf;
    ufunction_t func;
    os_err_t    ret;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_request_interface\r\n"));

    intf = os_usbd_find_interface(device, setup->wIndex & 0xFF, &func);
    if (intf != OS_NULL)
    {
        ret = intf->handler(func, setup);
    }
    else
    {
        ret = OS_ERROR;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle standard bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Invalid bRequest.
 ***********************************************************************************************************************
 */
static os_err_t _standard_request(struct udevice *device, ureq_t setup)
{
    udcd_t      dcd;
    os_uint16_t value = 0;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    dcd = device->dcd;

    switch (setup->request_type & USB_REQ_TYPE_RECIPIENT_MASK)
    {
    case USB_REQ_TYPE_DEVICE:
        switch (setup->bRequest)
        {
        case USB_REQ_GET_STATUS:
            os_usbd_ep0_write(device, &value, 2);
            break;
        case USB_REQ_CLEAR_FEATURE:
            os_usbd_clear_feature(device, setup->wValue, setup->wIndex);
            dcd_ep0_send_status(dcd);
            break;
        case USB_REQ_SET_FEATURE:
            os_usbd_set_feature(device, setup->wValue, setup->wIndex);
            break;
        case USB_REQ_SET_ADDRESS:
            _set_address(device, setup);
            break;
        case USB_REQ_GET_DESCRIPTOR:
            _get_descriptor(device, setup);
            break;
        case USB_REQ_SET_DESCRIPTOR:
            os_usbd_ep0_set_stall(device);
            break;
        case USB_REQ_GET_CONFIGURATION:
            _get_config(device, setup);
            break;
        case USB_REQ_SET_CONFIGURATION:
            _set_config(device, setup);
            break;
        default:
            os_kprintf("unknown device request\r\n");
            os_usbd_ep0_set_stall(device);
            break;
        }
        break;
    case USB_REQ_TYPE_INTERFACE:
        switch (setup->bRequest)
        {
        case USB_REQ_GET_INTERFACE:
            _get_interface(device, setup);
            break;
        case USB_REQ_SET_INTERFACE:
            _set_interface(device, setup);
            break;
        default:
            if (_request_interface(device, setup) != OS_EOK)
            {
                os_kprintf("unknown interface request\r\n");
                os_usbd_ep0_set_stall(device);
                return OS_ERROR;
            }
            else
                break;
        }
        break;
    case USB_REQ_TYPE_ENDPOINT:
        switch (setup->bRequest)
        {
        case USB_REQ_GET_STATUS:
        {
            uep_t ep;

            ep    = os_usbd_find_endpoint(device, OS_NULL, setup->wIndex);
            value = ep->stalled;
            os_usbd_ep0_write(device, &value, 2);
        }
        break;
        case USB_REQ_CLEAR_FEATURE:
        {
            uep_t                ep;
            uio_request_t        req;
            struct os_list_node *node;

            ep = os_usbd_find_endpoint(device, OS_NULL, setup->wIndex);
            if (USB_EP_HALT == setup->wValue && ep->stalled == OS_TRUE)
            {
                os_usbd_clear_feature(device, setup->wValue, setup->wIndex);
                dcd_ep0_send_status(dcd);
                ep->stalled = OS_FALSE;

                for (node = ep->request_list.next; node != &ep->request_list; node = node->next)
                {
                    req = (uio_request_t)os_list_entry(node, struct uio_request, list);
                    os_usbd_io_request(device, ep, req);
                    OS_DEBUG_LOG(OS_DEBUG_USB, ("fired a request\r\n"));
                }

                os_list_init(&ep->request_list);
            }
        }
        break;
        case USB_REQ_SET_FEATURE:
        {
            uep_t ep;

            if (USB_EP_HALT == setup->wValue)
            {
                ep          = os_usbd_find_endpoint(device, OS_NULL, setup->wIndex);
                ep->stalled = OS_TRUE;
                os_usbd_set_feature(device, setup->wValue, setup->wIndex);
                dcd_ep0_send_status(dcd);
            }
        }
        break;
        case USB_REQ_SYNCH_FRAME:
            break;
        default:
            os_kprintf("unknown endpoint request\r\n");
            os_usbd_ep0_set_stall(device);
            break;
        }
        break;
    case USB_REQ_TYPE_OTHER:
        os_kprintf("unknown other type request\r\n");
        os_usbd_ep0_set_stall(device);
        break;
    default:
        os_kprintf("unknown type request\r\n");
        os_usbd_ep0_set_stall(device);
        break;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle function bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Invalid bRequest.
 ***********************************************************************************************************************
 */
static os_err_t _function_request(udevice_t device, ureq_t setup)
{
    uintf_t     intf;
    ufunction_t func;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    /* Verify bRequest wValue */
    if (setup->wIndex > device->curr_cfg->cfg_desc.bNumInterfaces)
    {
        os_usbd_ep0_set_stall(device);
        return OS_ERROR;
    }

    switch (setup->request_type & USB_REQ_TYPE_RECIPIENT_MASK)
    {
    case USB_REQ_TYPE_INTERFACE:
        intf = os_usbd_find_interface(device, setup->wIndex & 0xFF, &func);
        if (intf == OS_NULL)
        {
            os_kprintf("unkwown interface request\r\n");
            os_usbd_ep0_set_stall(device);
        }
        else
        {
            intf->handler(func, setup);
        }
        break;
    case USB_REQ_TYPE_ENDPOINT:
        break;
    default:
        os_kprintf("unknown function request type\r\n");
        os_usbd_ep0_set_stall(device);
        break;
    }

    return OS_EOK;
}

static os_err_t _vendor_request(udevice_t device, ureq_t setup)
{
    static os_uint8_t         *usb_comp_id_desc      = OS_NULL;
    static os_uint32_t         usb_comp_id_desc_size = 0;
    usb_os_func_comp_id_desc_t func_comp_id_desc;
    uintf_t                    intf;
    ufunction_t                func;
    switch (setup->bRequest)
    {
    case 'A':
        switch (setup->wIndex)
        {
        case 0x04:
            if (os_list_len(&device->os_comp_id_desc->func_desc) == 0)
            {
                os_usbd_ep0_set_stall(device);
                return OS_EOK;
            }
            if (usb_comp_id_desc == OS_NULL)
            {
                os_uint8_t *    pusb_comp_id_desc;
                os_list_node_t *p;
                usb_comp_id_desc_size = sizeof(struct usb_os_header_comp_id_descriptor) +
                                        (sizeof(struct usb_os_function_comp_id_descriptor) - sizeof(os_list_node_t)) *
                                            os_list_len(&device->os_comp_id_desc->func_desc);

                usb_comp_id_desc = (os_uint8_t *)os_calloc(1, usb_comp_id_desc_size);
                OS_ASSERT(usb_comp_id_desc != OS_NULL);
                device->os_comp_id_desc->head_desc.dwLength = usb_comp_id_desc_size;
                pusb_comp_id_desc                           = usb_comp_id_desc;
                memcpy((void *)pusb_comp_id_desc,
                       (void *)&device->os_comp_id_desc->head_desc,
                       sizeof(struct usb_os_header_comp_id_descriptor));
                pusb_comp_id_desc += sizeof(struct usb_os_header_comp_id_descriptor);

                for (p = device->os_comp_id_desc->func_desc.next; p != &device->os_comp_id_desc->func_desc; p = p->next)
                {
                    func_comp_id_desc = os_list_entry(p, struct usb_os_function_comp_id_descriptor, list);
                    memcpy(pusb_comp_id_desc,
                           (void *)&func_comp_id_desc->bFirstInterfaceNumber,
                           sizeof(struct usb_os_function_comp_id_descriptor) - sizeof(os_list_node_t));
                    pusb_comp_id_desc += sizeof(struct usb_os_function_comp_id_descriptor) - sizeof(os_list_node_t);
                }
            }
            os_usbd_ep0_write(device, (void *)usb_comp_id_desc, setup->wLength);
            break;
        case 0x05:
            intf = os_usbd_find_interface(device, setup->wValue & 0xFF, &func);
            if (intf != OS_NULL)
            {
                intf->handler(func, setup);
            }
            break;
        }

        break;
    }
    return OS_EOK;
}

static os_err_t _dump_setup_packet(ureq_t setup)
{
    OS_DEBUG_LOG(OS_DEBUG_USB, ("[\r\n"));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("  setup_request : 0x%x\r\n", setup->request_type));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("  value         : 0x%x\r\n", setup->wValue));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("  length        : 0x%x\r\n", setup->wLength));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("  index         : 0x%x\r\n", setup->wIndex));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("  request       : 0x%x\r\n", setup->bRequest));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("]\r\n"));

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle setup bRequest.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       setup           The setup bRequest.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Invalid bRequest.
 ***********************************************************************************************************************
 */
static os_err_t _setup_request(udevice_t device, ureq_t setup)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    _dump_setup_packet(setup);

    switch ((setup->request_type & USB_REQ_TYPE_MASK))
    {
    case USB_REQ_TYPE_STANDARD:
        _standard_request(device, setup);
        break;
    case USB_REQ_TYPE_CLASS:
        _function_request(device, setup);
        break;
    case USB_REQ_TYPE_VENDOR:
        _vendor_request(device, setup);
        break;
    default:
        os_kprintf("unknown setup request type\r\n");
        os_usbd_ep0_set_stall(device);
        return OS_ERROR;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will hanle data notify event.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       ep_msg          The endpoint message.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Invalid bRequest.
 ***********************************************************************************************************************
 */
static os_err_t _data_notify(udevice_t device, struct ep_msg *ep_msg)
{
    uep_t       ep;
    ufunction_t func;
    os_size_t   size = 0;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(ep_msg != OS_NULL);

    if (device->state != USB_STATE_CONFIGURED)
    {
        return OS_ERROR;
    }

    ep = os_usbd_find_endpoint(device, &func, ep_msg->ep_addr);
    if (ep == OS_NULL)
    {
        os_kprintf("invalid endpoint\r\n");
        return OS_ERROR;
    }

    if (EP_ADDRESS(ep) & USB_DIR_IN)
    {
        size = ep_msg->size;
        if (ep->request.remain_size >= EP_MAXPACKET(ep))
        {
            dcd_ep_write(device->dcd, EP_ADDRESS(ep), ep->request.buffer, EP_MAXPACKET(ep));
            ep->request.remain_size -= EP_MAXPACKET(ep);
            ep->request.buffer += EP_MAXPACKET(ep);
        }
        else if (ep->request.remain_size > 0)
        {
            dcd_ep_write(device->dcd, EP_ADDRESS(ep), ep->request.buffer, ep->request.remain_size);
            ep->request.remain_size = 0;
        }
        else
        {
            EP_HANDLER(ep, func, size);
        }
    }
    else
    {
        size = ep_msg->size;
        if (ep->request.remain_size == 0)
        {
            return OS_EOK;
        }

        if (size == 0)
        {
            size = dcd_ep_read(device->dcd, EP_ADDRESS(ep), ep->request.buffer);
        }
        ep->request.remain_size -= size;
        ep->request.buffer += size;

        if (ep->request.req_type == UIO_REQUEST_READ_BEST)
        {
            EP_HANDLER(ep, func, size);
        }
        else if (ep->request.remain_size == 0)
        {
            EP_HANDLER(ep, func, ep->request.size);
        }
        else
        {
            dcd_ep_read_prepare(device->dcd,
                                EP_ADDRESS(ep),
                                ep->request.buffer,
                                ep->request.remain_size > EP_MAXPACKET(ep) ? EP_MAXPACKET(ep)
                                                                           : ep->request.remain_size);
        }
    }

    return OS_EOK;
}

static os_err_t _ep0_out_notify(udevice_t device, struct ep_msg *ep_msg)
{
    uep_t     ep0;
    os_size_t size;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(ep_msg != OS_NULL);
    OS_ASSERT(device->dcd != OS_NULL);

    ep0  = &device->dcd->ep0;
    size = ep_msg->size;

    if (ep0->request.remain_size == 0)
    {
        return OS_EOK;
    }
    if (size == 0)
    {
        size = dcd_ep_read(device->dcd, EP0_OUT_ADDR, ep0->request.buffer);
        if (size == 0)
        {
            return OS_EOK;
        }
    }

    ep0->request.remain_size -= size;
    ep0->request.buffer += size;
    if (ep0->request.remain_size == 0)
    {
        /* Invoke callback */
        if (ep0->rx_indicate != OS_NULL)
        {
            ep0->rx_indicate(device, size);
        }
    }
    else
    {
        os_usbd_ep0_read(device, ep0->request.buffer, ep0->request.remain_size, ep0->rx_indicate);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will notity sof event to all of function.
 *
 * @param[in]       device          The usb device object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _sof_notify(udevice_t device)
{
    struct os_list_node *i;
    ufunction_t          func;

    OS_ASSERT(device != OS_NULL);

    /* To notity every function that sof event comes */
    for (i = device->curr_cfg->func_list.next; i != &device->curr_cfg->func_list; i = i->next)
    {
        func = (ufunction_t)os_list_entry(i, struct ufunction, list);
        if (func->ops->sof_handler != OS_NULL)
            func->ops->sof_handler(func);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will disable all USB functions.
 *
 * @param[in]       device          The usb device object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _stop_notify(udevice_t device)
{
    struct os_list_node *i;
    ufunction_t          func;

    OS_ASSERT(device != OS_NULL);

    /* To notity every function */
    for (i = device->curr_cfg->func_list.next; i != &device->curr_cfg->func_list; i = i->next)
    {
        func = (ufunction_t)os_list_entry(i, struct ufunction, list);
        FUNC_DISABLE(func);
    }

    return OS_EOK;
}

static os_size_t os_usbd_ep_write(udevice_t device, uep_t ep, void *buffer, os_size_t size)
{
    os_uint16_t maxpacket;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->dcd != OS_NULL);
    OS_ASSERT(ep != OS_NULL);

    os_schedule_lock();
    maxpacket = EP_MAXPACKET(ep);
    if (ep->request.remain_size >= maxpacket)
    {
        dcd_ep_write(device->dcd, EP_ADDRESS(ep), ep->request.buffer, maxpacket);
        ep->request.remain_size -= maxpacket;
        ep->request.buffer += maxpacket;
    }
    else
    {
        dcd_ep_write(device->dcd, EP_ADDRESS(ep), ep->request.buffer, ep->request.remain_size);
        ep->request.remain_size = 0;
    }
    os_schedule_unlock();
    return size;
}

static os_size_t os_usbd_ep_read_prepare(udevice_t device, uep_t ep, void *buffer, os_size_t size)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->dcd != OS_NULL);
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(buffer != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);

    return dcd_ep_read_prepare(device->dcd, EP_ADDRESS(ep), buffer, size > EP_MAXPACKET(ep) ? EP_MAXPACKET(ep) : size);
}

udevice_t os_usbd_device_new(void)
{
    udevice_t udevice;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_device_new\r\n"));

    /* Allocate memory for the object */
    udevice = (udevice_t)os_calloc(1, sizeof(struct udevice));
    if (udevice == OS_NULL)
    {
        os_kprintf("alloc memery failed\r\n");
        return OS_NULL;
    }
    memset(udevice, 0, sizeof(struct udevice));

    /* To initialize configuration list */
    os_list_init(&udevice->cfg_list);

    /* Insert the device object to device list */
    os_list_add(&device_list, &udevice->list);

    return udevice;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set usb device string description.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       ustring         Pointer to string pointer array.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_device_set_string(udevice_t device, const char **ustring)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(ustring != OS_NULL);

    /* Set string descriptor array to the device object */
    device->str = ustring;

    return OS_EOK;
}

os_err_t os_usbd_device_set_os_comp_id_desc(udevice_t device, usb_os_comp_id_desc_t os_comp_id_desc)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(os_comp_id_desc != OS_NULL);

    /* Set string descriptor array to the device object */
    device->os_comp_id_desc = os_comp_id_desc;
    os_list_init(&device->os_comp_id_desc->func_desc);
    return OS_EOK;
}

os_err_t os_usbd_device_set_qualifier(udevice_t device, struct usb_qualifier_descriptor *qualifier)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(qualifier != OS_NULL);

    device->dev_qualifier = qualifier;

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set an usb controller driver to a device.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       dcd             The usb device controller driver.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_device_set_controller(udevice_t device, udcd_t dcd)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(dcd != OS_NULL);

    /* Set usb device controller driver to the device */
    device->dcd = dcd;

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set an usb device descriptor to a device.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       dev_desc        The usb device descriptor.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_device_set_descriptor(udevice_t device, udev_desc_t dev_desc)
{
    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(dev_desc != OS_NULL);

    /* Copy the usb device descriptor to the device */
    memcpy((void *)&device->dev_desc, (void *)dev_desc, USB_DESC_LENGTH_DEVICE);

    return OS_EOK;
}

uconfig_t os_usbd_config_new(void)
{
    uconfig_t cfg;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_config_new\r\n"));

    /* Allocate memory for the object */
    cfg = (uconfig_t)os_calloc(1, sizeof(struct uconfig));
    if (cfg == OS_NULL)
    {
        os_kprintf("alloc memery failed\r\n");
        return OS_NULL;
    }
    memset(cfg, 0, sizeof(struct uconfig));

    /* Set default wValue */
    cfg->cfg_desc.bLength      = USB_DESC_LENGTH_CONFIG;
    cfg->cfg_desc.type         = USB_DESC_TYPE_CONFIGURATION;
    cfg->cfg_desc.wTotalLength = USB_DESC_LENGTH_CONFIG;
    cfg->cfg_desc.bmAttributes = 0xC0;
    cfg->cfg_desc.MaxPower     = 0x32;

    /* To initialize function object list */
    os_list_init(&cfg->func_list);

    return cfg;
}

uintf_t os_usbd_interface_new(udevice_t device, uintf_handler_t handler)
{
    uintf_t intf;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_interface_new\r\n"));

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Allocate memory for the object */
    intf = (uintf_t)os_calloc(1, sizeof(struct uinterface));
    if (intf == OS_NULL)
    {
        os_kprintf("alloc memery failed\r\n");
        return OS_NULL;
    }
    intf->intf_num = device->nr_intf;
    device->nr_intf++;
    intf->handler      = handler;
    intf->curr_setting = OS_NULL;

    /* To initialize the alternate setting object list */
    os_list_init(&intf->setting_list);

    return intf;
}

ualtsetting_t os_usbd_altsetting_new(os_size_t desc_size)
{
    ualtsetting_t setting;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_altsetting_new\r\n"));

    /* Parameter check */
    OS_ASSERT(desc_size > 0);

    /* Allocate memory for the object */
    setting = (ualtsetting_t)os_calloc(1, sizeof(struct ualtsetting));
    if (setting == OS_NULL)
    {
        os_kprintf("alloc memery failed\r\n");
        return OS_NULL;
    }
    /* Allocate memory for the desc */
    setting->desc = os_calloc(1, desc_size);
    if (setting->desc == OS_NULL)
    {
        os_kprintf("alloc desc memery failed\r\n");
        os_free(setting);
        return OS_NULL;
    }

    setting->desc_size = desc_size;
    setting->intf_desc = OS_NULL;

    /* To initialize endpoint list */
    os_list_init(&setting->ep_list);

    return setting;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will config an desc in alternate setting object.
 *
 * @param[in]       setting          The altsetting to be config.
 * @param[in]       desc             Use it to init desc in setting.
 * @param[in]       intf_pos         The offset of interface descriptor in desc.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_altsetting_config_descriptor(ualtsetting_t setting, const void *desc, os_off_t intf_pos)
{
    OS_ASSERT(setting != OS_NULL);
    OS_ASSERT(setting->desc != OS_NULL);

    memcpy(setting->desc, desc, setting->desc_size);
    setting->intf_desc = (uintf_desc_t)((char *)setting->desc + intf_pos);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will create an usb function object.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       dev_desc        The device descriptor.
 * @param[in]       ops             The operation set.
 *
 * @return          The operation status.
 * @retval          usb function object          Successful.
 * @retval          OS_NULL                      Fail.
 ***********************************************************************************************************************
 */
ufunction_t os_usbd_function_new(udevice_t device, udev_desc_t dev_desc, ufunction_ops_t ops)
{
    ufunction_t func;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_function_new\r\n"));

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(dev_desc != OS_NULL);

    /* Allocate memory for the object */
    func = (ufunction_t)os_calloc(1, sizeof(struct ufunction));
    if (func == OS_NULL)
    {
        os_kprintf("alloc memery failed\r\n");
        return OS_NULL;
    }
    func->dev_desc = dev_desc;
    func->ops      = ops;
    func->device   = device;
    func->enabled  = OS_FALSE;

    /* To initialize interface list */
    os_list_init(&func->intf_list);

    return func;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will create an usb endpoint object.
 *
 * @param[in]       ep_desc         The endpoint descriptor.
 * @param[in]       handler         The callback handler of object.
 *
 * @return          The operation status.
 * @retval          usb endpoint object          Successful.
 * @retval          OS_NULL                      Fail.
 ***********************************************************************************************************************
 */
uep_t os_usbd_endpoint_new(uep_desc_t ep_desc, udep_handler_t handler)
{
    uep_t ep;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_endpoint_new\r\n"));

    /* Parameter check */
    OS_ASSERT(ep_desc != OS_NULL);

    /* Allocate memory for the object */
    ep = (uep_t)os_calloc(1, sizeof(struct uendpoint));
    if (ep == OS_NULL)
    {
        os_kprintf("alloc memery failed\r\n");
        return OS_NULL;
    }
    ep->ep_desc = ep_desc;
    ep->handler = handler;
    ep->buffer  = OS_NULL;
    ep->stalled = OS_FALSE;
    os_list_init(&ep->request_list);

    return ep;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will find an usb device object.
 *
 * @param[in]       dcd             Usd device controller driver.
 *
 * @return          The operation status.
 * @retval          usb device object            Successful.
 * @retval          OS_NULL                      Fail.
 ***********************************************************************************************************************
 */
udevice_t os_usbd_find_device(udcd_t dcd)
{
    struct os_list_node *node;
    udevice_t            device;

    /* Parameter check */
    OS_ASSERT(dcd != OS_NULL);

    /* Search a device in the the device list */
    for (node = device_list.next; node != &device_list; node = node->next)
    {
        device = (udevice_t)os_list_entry(node, struct udevice, list);
        if (device->dcd == dcd)
            return device;
    }

    os_kprintf("can't find device\r\n");
    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief         This function will find an usb configuration object.
 *
 * @param[in]     device         The usb device object.
 * @param[in]     value          The configuration number.
 *
 * @return........The operation status.
 * @retval........usb configuration object......Successful.
 * @retval........OS_NULL.........              Fail.
 ***********************************************************************************************************************
 */
uconfig_t os_usbd_find_config(udevice_t device, os_uint8_t value)
{
    struct os_list_node *node;
    uconfig_t            cfg = OS_NULL;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_find_config\r\n"));

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(value <= device->dev_desc.bNumConfigurations);

    /* Search a configration in the the device */
    for (node = device->cfg_list.next; node != &device->cfg_list; node = node->next)
    {
        cfg = (uconfig_t)os_list_entry(node, struct udevice, list);
        if (cfg->cfg_desc.bConfigurationValue == value)
        {
            return cfg;
        }
    }

    os_kprintf("can't find configuration %d\r\n", value);
    return OS_NULL;
}

uintf_t os_usbd_find_interface(udevice_t device, os_uint8_t value, ufunction_t *pfunc)
{
    struct os_list_node *i, *j;
    ufunction_t          func;
    uintf_t              intf;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_find_interface\r\n"));

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(value < device->nr_intf);

    /* Search an interface in the current configuration */
    for (i = device->curr_cfg->func_list.next; i != &device->curr_cfg->func_list; i = i->next)
    {
        func = (ufunction_t)os_list_entry(i, struct ufunction, list);
        for (j = func->intf_list.next; j != &func->intf_list; j = j->next)
        {
            intf = (uintf_t)os_list_entry(j, struct uinterface, list);
            if (intf->intf_num == value)
            {
                if (pfunc != OS_NULL)
                    *pfunc = func;
                return intf;
            }
        }
    }

    os_kprintf("can't find interface %d\r\n", value);
    return OS_NULL;
}

ualtsetting_t os_usbd_find_altsetting(uintf_t intf, os_uint8_t value)
{
    struct os_list_node *i;
    ualtsetting_t        setting;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_find_altsetting\r\n"));

    /* Parameter check */
    OS_ASSERT(intf != OS_NULL);

    if (intf->curr_setting != OS_NULL)
    {
        /* If the wValue equal to the current alternate setting, then do not search */
        if (intf->curr_setting->intf_desc->bAlternateSetting == value)
            return intf->curr_setting;
    }

    /* Search a setting in the alternate setting list */
    for (i = intf->setting_list.next; i != &intf->setting_list; i = i->next)
    {
        setting = (ualtsetting_t)os_list_entry(i, struct ualtsetting, list);
        if (setting->intf_desc->bAlternateSetting == value)
            return setting;
    }

    os_kprintf("can't find alternate setting %d\r\n", value);
    return OS_NULL;
}

uep_t os_usbd_find_endpoint(udevice_t device, ufunction_t *pfunc, os_uint8_t ep_addr)
{
    uep_t                ep;
    struct os_list_node *i, *j, *k;
    ufunction_t          func;
    uintf_t              intf;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Search a endpoint in the current configuration */
    for (i = device->curr_cfg->func_list.next; i != &device->curr_cfg->func_list; i = i->next)
    {
        func = (ufunction_t)os_list_entry(i, struct ufunction, list);
        for (j = func->intf_list.next; j != &func->intf_list; j = j->next)
        {
            intf = (uintf_t)os_list_entry(j, struct uinterface, list);
            for (k = intf->curr_setting->ep_list.next; k != &intf->curr_setting->ep_list; k = k->next)
            {
                ep = (uep_t)os_list_entry(k, struct uendpoint, list);
                if (EP_ADDRESS(ep) == ep_addr)
                {
                    if (pfunc != OS_NULL)
                        *pfunc = func;
                    return ep;
                }
            }
        }
    }

    os_kprintf("can't find endpoint 0x%x\r\n", ep_addr);
    return OS_NULL;
}

/**
 ***********************************************************************************************************************
 * @brief         This function will add a configuration to an usb device.
 *
 * @param[in]     device         The usb device object.
 * @param[in]     cfg            The configuration object.
 *
 * @return........The operation status.
 * @retval........OS_EOK.........Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_device_add_config(udevice_t device, uconfig_t cfg)
{
    struct os_list_node *i, *j, *k, *m;
    ufunction_t          func;
    uintf_t              intf;
    ualtsetting_t        altsetting;
    uep_t                ep;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_device_add_config\r\n"));

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    /* Set configuration number to the configuration descriptor */
    cfg->cfg_desc.bConfigurationValue = device->dev_desc.bNumConfigurations + 1;
    device->dev_desc.bNumConfigurations++;

    for (i = cfg->func_list.next; i != &cfg->func_list; i = i->next)
    {
        func = (ufunction_t)os_list_entry(i, struct ufunction, list);

        for (j = func->intf_list.next; j != &func->intf_list; j = j->next)
        {
            intf = (uintf_t)os_list_entry(j, struct uinterface, list);
            cfg->cfg_desc.bNumInterfaces++;

            for (k = intf->setting_list.next; k != &intf->setting_list; k = k->next)
            {
                altsetting = (ualtsetting_t)os_list_entry(k, struct ualtsetting, list);

                /* Allocate address for every endpoint in the interface alternate setting */
                for (m = altsetting->ep_list.next; m != &altsetting->ep_list; m = m->next)
                {
                    ep = (uep_t)os_list_entry(m, struct uendpoint, list);
                    if (os_usbd_ep_assign(device, ep) != OS_EOK)
                    {
                        os_kprintf("endpoint assign error\r\n");
                    }
                }

                /* Construct complete configuration descriptor */
                memcpy((void *)&cfg->cfg_desc.data[cfg->cfg_desc.wTotalLength - USB_DESC_LENGTH_CONFIG],
                       (void *)altsetting->desc,
                       altsetting->desc_size);
                cfg->cfg_desc.wTotalLength += altsetting->desc_size;
            }
        }
    }

    /* Insert the configuration to the list */
    os_list_add(&device->cfg_list, &cfg->list);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief         This function will add a function to a configuration.
 *
 * @param[in]     cfg            The configuration object.
 * @param[in]     func           The function object.
 *
 * @return........The operation status.
 * @retval........OS_EOK.........Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_config_add_function(uconfig_t cfg, ufunction_t func)
{
    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_config_add_function\r\n"));

    /* Parameter check */
    OS_ASSERT(cfg != OS_NULL);
    OS_ASSERT(func != OS_NULL);

    /* Insert the function to the list */
    os_list_add(&cfg->func_list, &func->list);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief         This function will add an interface to a function.
 *
 * @param[in]     func           The function object.
 * @param[in]     intf           The interface object.
 *
 * @return........The operation status.
 * @retval........OS_EOK.........Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_function_add_interface(ufunction_t func, uintf_t intf)
{

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_function_add_interface\r\n"));

    /* Parameter check */
    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(intf != OS_NULL);

    /* Insert the interface to the list */
    os_list_add(&func->intf_list, &intf->list);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief         This function will add an alternate setting to an interface.
 *
 * @param[in]     intf           The interface object.
 * @param[in]     setting        The alternate setting object.
 *
 * @return........The operation status.
 * @retval........OS_EOK.........Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_interface_add_altsetting(uintf_t intf, ualtsetting_t setting)
{
    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_interface_add_altsetting\r\n"));

    /* Parameter check */
    OS_ASSERT(intf != OS_NULL);
    OS_ASSERT(setting != OS_NULL);

    setting->intf_desc->bInterfaceNumber = intf->intf_num;

    /* Insert the alternate setting to the list */
    os_list_add(&intf->setting_list, &setting->list);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief         This function will add an endpoint to an alternate setting.
 *
 * @param[in]     setting        The interface descriptor.
 * @param[in]     ep             The alternate setting number.
 *
 * @return........The operation status.
 * @retval........OS_EOK.........Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_altsetting_add_endpoint(ualtsetting_t setting, uep_t ep)
{
    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_altsetting_add_endpoint\r\n"));

    /* Parameter check */
    OS_ASSERT(setting != OS_NULL);
    OS_ASSERT(ep != OS_NULL);

    /* Insert the endpoint to the list */
    os_list_add(&setting->ep_list, &ep->list);

    return OS_EOK;
}

os_err_t os_usbd_os_comp_id_desc_add_os_func_comp_id_desc(usb_os_comp_id_desc_t      os_comp_id_desc,
                                                          usb_os_func_comp_id_desc_t os_func_comp_id_desc)
{
    OS_ASSERT(os_comp_id_desc != OS_NULL);
    OS_ASSERT(os_func_comp_id_desc != OS_NULL);
    os_list_add(&os_comp_id_desc->func_desc, &os_func_comp_id_desc->list);
    os_comp_id_desc->head_desc.bCount++;
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief         This function will set an alternate setting for an interface.
 *
 * @param[in]     intf           The interface descriptor.
 * @param[in]     value          The alternate setting number.
 *
 * @return........The operation status.
 * @retval........OS_EOK.........Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_set_altsetting(uintf_t intf, os_uint8_t value)
{
    ualtsetting_t setting;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_set_altsetting\r\n"));

    /* Parameter check */
    OS_ASSERT(intf != OS_NULL);

    /* Find an alternate setting */
    setting = os_usbd_find_altsetting(intf, value);

    /* Set as current alternate setting */
    intf->curr_setting = setting;

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set feature for an usb device.
 *
 * @param[in]       device         The usb device object.
 * @param[in]       value          The configuration number.
 *
 * @return          The operation status.
 * @retval          OS_TRUE        Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_set_config(udevice_t device, os_uint8_t value)
{
    uconfig_t cfg;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("os_usbd_set_config\r\n"));

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(value <= device->dev_desc.bNumConfigurations);

    /* Find a configuration */
    cfg = os_usbd_find_config(device, value);

    /* Set as current configuration */
    device->curr_cfg = cfg;

    dcd_set_config(device->dcd, value);

    return OS_TRUE;
}

os_size_t os_usbd_io_request(udevice_t device, uep_t ep, uio_request_t req)
{
    os_size_t size = 0;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(req != OS_NULL);

    if (ep->stalled == OS_FALSE)
    {
        switch (req->req_type)
        {
        case UIO_REQUEST_READ_BEST:
        case UIO_REQUEST_READ_FULL:
            ep->request.remain_size = ep->request.size;
            size                    = os_usbd_ep_read_prepare(device, ep, req->buffer, req->size);
            break;
        case UIO_REQUEST_WRITE:
            ep->request.remain_size = ep->request.size;
            size                    = os_usbd_ep_write(device, ep, req->buffer, req->size);
            break;
        default:
            os_kprintf("unknown request type\r\n");
            break;
        }
    }
    else
    {
        os_list_add(&ep->request_list, &req->list);
        OS_DEBUG_LOG(OS_DEBUG_USB, ("suspend a request\r\n"));
    }

    return size;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set feature for an usb device.
 *
 * @param[in]       device         The usb device object.
 * @param[in]       value          The configuration number.
 * @param[in]       index          The index
 *
 * @return          The operation status.
 * @retval          OS_OK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_set_feature(udevice_t device, os_uint16_t value, os_uint16_t index)
{
    OS_ASSERT(device != OS_NULL);

    if (value == USB_FEATURE_DEV_REMOTE_WAKEUP)
    {
        OS_DEBUG_LOG(OS_DEBUG_USB, ("set feature remote wakeup\r\n"));
    }
    else if (value == USB_FEATURE_ENDPOINT_HALT)
    {
        OS_DEBUG_LOG(OS_DEBUG_USB, ("set feature stall\r\n"));
        dcd_ep_set_stall(device->dcd, (os_uint32_t)(index & 0xFF));
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will clear feature for an usb device.
 *
 * @param[in]       device          The usb device object.
 * @param[in]       value           The configuration number.
 * @param[in]       index           The index
 *
 * @return          The operation status.
 * @retval          OS_EOK          Clear feature ok.
 ***********************************************************************************************************************
 */
os_err_t os_usbd_clear_feature(udevice_t device, os_uint16_t value, os_uint16_t index)
{
    OS_ASSERT(device != OS_NULL);

    if (value == USB_FEATURE_DEV_REMOTE_WAKEUP)
    {
        OS_DEBUG_LOG(OS_DEBUG_USB, ("clear feature remote wakeup\r\n"));
    }
    else if (value == USB_FEATURE_ENDPOINT_HALT)
    {
        OS_DEBUG_LOG(OS_DEBUG_USB, ("clear feature stall\r\n"));
        dcd_ep_clear_stall(device->dcd, (os_uint32_t)(index & 0xFF));
    }

    return OS_EOK;
}

os_err_t os_usbd_ep0_set_stall(udevice_t device)
{
    OS_ASSERT(device != OS_NULL);

    return dcd_ep_set_stall(device->dcd, 0);
}

os_err_t os_usbd_ep0_clear_stall(udevice_t device)
{
    OS_ASSERT(device != OS_NULL);

    return dcd_ep_clear_stall(device->dcd, 0);
}

os_err_t os_usbd_ep_set_stall(udevice_t device, uep_t ep)
{
    os_err_t ret;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);

    ret = dcd_ep_set_stall(device->dcd, EP_ADDRESS(ep));
    if (ret == OS_EOK)
    {
        ep->stalled = OS_TRUE;
    }

    return ret;
}

os_err_t os_usbd_ep_clear_stall(udevice_t device, uep_t ep)
{
    os_err_t ret;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);

    ret = dcd_ep_clear_stall(device->dcd, EP_ADDRESS(ep));
    if (ret == OS_EOK)
    {
        ep->stalled = OS_FALSE;
    }

    return ret;
}

static os_err_t os_usbd_ep_assign(udevice_t device, uep_t ep)
{
    int i = 0;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->dcd != OS_NULL);
    OS_ASSERT(device->dcd->ep_pool != OS_NULL);
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);

    while (device->dcd->ep_pool[i].addr != 0xFF)
    {
        if (device->dcd->ep_pool[i].status == ID_UNASSIGNED &&
            ep->ep_desc->bmAttributes == device->dcd->ep_pool[i].type &&
            (EP_ADDRESS(ep) & 0x80) == device->dcd->ep_pool[i].dir)
        {
            EP_ADDRESS(ep) |= device->dcd->ep_pool[i].addr;
            ep->id                         = &device->dcd->ep_pool[i];
            device->dcd->ep_pool[i].status = ID_ASSIGNED;

            OS_DEBUG_LOG(OS_DEBUG_USB, ("assigned %d\r\n", device->dcd->ep_pool[i].addr));
            return OS_EOK;
        }

        i++;
    }

    return OS_ERROR;
}

os_err_t os_usbd_ep_unassign(udevice_t device, uep_t ep)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->dcd != OS_NULL);
    OS_ASSERT(device->dcd->ep_pool != OS_NULL);
    OS_ASSERT(ep != OS_NULL);
    OS_ASSERT(ep->ep_desc != OS_NULL);

    ep->id->status = ID_UNASSIGNED;

    return OS_EOK;
}

os_err_t os_usbd_ep0_setup_handler(udcd_t dcd, struct urequest *setup)
{
    struct udev_msg msg;
    os_size_t       size;

    OS_ASSERT(dcd != OS_NULL);

    if (setup == OS_NULL)
    {
        size = dcd_ep_read(dcd, EP0_OUT_ADDR, (void *)&msg.content.setup);
        if (size != sizeof(struct urequest))
        {
            os_kprintf("read setup packet error\r\n");
            return OS_ERROR;
        }
    }
    else
    {
        memcpy((void *)&msg.content.setup, (void *)setup, sizeof(struct urequest));
    }

    msg.type = USB_MSG_SETUP_NOTIFY;
    msg.dcd  = dcd;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_err_t os_usbd_ep0_in_handler(udcd_t dcd)
{
    os_int32_t remain, mps;

    OS_ASSERT(dcd != OS_NULL);

    if (dcd->stage != STAGE_DIN)
        return OS_EOK;

    mps = dcd->ep0.id->maxpacket;
    dcd->ep0.request.remain_size -= mps;
    remain = dcd->ep0.request.remain_size;

    if (remain > 0)
    {
        if (remain >= mps)
        {
            remain = mps;
        }

        dcd->ep0.request.buffer += mps;
        dcd_ep_write(dcd, EP0_IN_ADDR, dcd->ep0.request.buffer, remain);
    }
    else
    {
        /* Last packet is MPS multiple, so send ZLP packet */
        if ((remain == 0) && (dcd->ep0.request.size > 0))
        {
            dcd->ep0.request.size = 0;
            dcd_ep_write(dcd, EP0_IN_ADDR, OS_NULL, 0);
        }
        else
        {
            /* Receive status */
            dcd->stage = STAGE_STATUS_OUT;
            dcd_ep_read_prepare(dcd, EP0_OUT_ADDR, OS_NULL, 0);
        }
    }

    return OS_EOK;
}

os_err_t os_usbd_ep0_out_handler(udcd_t dcd, os_size_t size)
{
    struct udev_msg msg;

    OS_ASSERT(dcd != OS_NULL);

    msg.type                = USB_MSG_EP0_OUT;
    msg.dcd                 = dcd;
    msg.content.ep_msg.size = size;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_err_t os_usbd_ep_in_handler(udcd_t dcd, os_uint8_t address, os_size_t size)
{
    struct udev_msg msg;

    OS_ASSERT(dcd != OS_NULL);

    msg.type                   = USB_MSG_DATA_NOTIFY;
    msg.dcd                    = dcd;
    msg.content.ep_msg.ep_addr = address;
    msg.content.ep_msg.size    = size;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_err_t os_usbd_ep_out_handler(udcd_t dcd, os_uint8_t address, os_size_t size)
{
    struct udev_msg msg;

    OS_ASSERT(dcd != OS_NULL);

    msg.type                   = USB_MSG_DATA_NOTIFY;
    msg.dcd                    = dcd;
    msg.content.ep_msg.ep_addr = address;
    msg.content.ep_msg.size    = size;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_err_t os_usbd_reset_handler(udcd_t dcd)
{
    struct udev_msg msg;

    OS_ASSERT(dcd != OS_NULL);

    msg.type = USB_MSG_RESET;
    msg.dcd  = dcd;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_err_t os_usbd_connect_handler(udcd_t dcd)
{
    struct udev_msg msg;

    OS_ASSERT(dcd != OS_NULL);

    msg.type = USB_MSG_PLUG_IN;
    msg.dcd  = dcd;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_err_t os_usbd_disconnect_handler(udcd_t dcd)
{
    struct udev_msg msg;

    OS_ASSERT(dcd != OS_NULL);

    msg.type = USB_MSG_PLUG_OUT;
    msg.dcd  = dcd;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_err_t os_usbd_sof_handler(udcd_t dcd)
{
    struct udev_msg msg;

    OS_ASSERT(dcd != OS_NULL);

    msg.type = USB_MSG_SOF;
    msg.dcd  = dcd;
    os_usbd_event_signal(&msg);

    return OS_EOK;
}

os_size_t os_usbd_ep0_write(udevice_t device, void *buffer, os_size_t size)
{
    uep_t     ep0;
    os_size_t sent_size = 0;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->dcd != OS_NULL);
    OS_ASSERT(buffer != OS_NULL);
    OS_ASSERT(size > 0);

    ep0                      = &device->dcd->ep0;
    ep0->request.size        = size;
    ep0->request.buffer      = (os_uint8_t *)buffer;
    ep0->request.remain_size = size;
    if (size >= ep0->id->maxpacket)
    {
        sent_size = ep0->id->maxpacket;
    }
    else
    {
        sent_size = size;
    }
    device->dcd->stage = STAGE_DIN;

    return dcd_ep_write(device->dcd, EP0_IN_ADDR, ep0->request.buffer, sent_size);
}

os_size_t os_usbd_ep0_read(udevice_t device, void *buffer, os_size_t size, 
    os_err_t (*rx_ind)(udevice_t device, os_size_t size))
{
    uep_t     ep0;
    os_size_t read_size = 0;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->dcd != OS_NULL);
    OS_ASSERT(buffer != OS_NULL);

    ep0                      = &device->dcd->ep0;
    ep0->request.buffer      = (os_uint8_t *)buffer;
    ep0->request.remain_size = size;
    ep0->rx_indicate         = rx_ind;
    if (size >= ep0->id->maxpacket)
    {
        read_size = ep0->id->maxpacket;
    }
    else
    {
        read_size = size;
    }
    device->dcd->stage = STAGE_DOUT;
    dcd_ep_read_prepare(device->dcd, EP0_OUT_ADDR, buffer, read_size);

    return size;
}

static struct os_mq usb_mq;

static void os_usbd_task_entry(void *parameter)
{
    while (1)
    {
        struct udev_msg msg;
        os_size_t       recv_len;
        udevice_t       device;

        /* Receive message */
        if (os_mq_recv(&usb_mq, &msg, sizeof(struct udev_msg), OS_WAIT_FOREVER, &recv_len) != OS_EOK)
            continue;

        device = os_usbd_find_device(msg.dcd);
        if (device == OS_NULL)
        {
            os_kprintf("invalid usb device\r\n");
            continue;
        }

        OS_DEBUG_LOG(OS_DEBUG_USB, ("message type %d\r\n", msg.type));

        switch (msg.type)
        {
        case USB_MSG_SOF:
            _sof_notify(device);
            break;
        case USB_MSG_DATA_NOTIFY:
            /*
             * Some buggy drivers will have USB_MSG_DATA_NOTIFY before the core
             * got configured.
             */
            _data_notify(device, &msg.content.ep_msg);
            break;
        case USB_MSG_SETUP_NOTIFY:
            _setup_request(device, &msg.content.setup);
            break;
        case USB_MSG_EP0_OUT:
            _ep0_out_notify(device, &msg.content.ep_msg);
            break;
        case USB_MSG_RESET:
            OS_DEBUG_LOG(OS_DEBUG_USB, ("reset %d\r\n", device->state));
            if (device->state == USB_STATE_ADDRESS || device->state == USB_STATE_CONFIGURED)
                _stop_notify(device);
            device->state = USB_STATE_NOTATTACHED;
            break;
        case USB_MSG_PLUG_IN:
            device->state = USB_STATE_ATTACHED;
            break;
        case USB_MSG_PLUG_OUT:
            device->state = USB_STATE_NOTATTACHED;
            _stop_notify(device);
            break;
        default:
            os_kprintf("unknown msg type %d\r\n", msg.type);
            break;
        }
    }
}

os_err_t os_usbd_event_signal(struct udev_msg *msg)
{
    OS_ASSERT(msg != OS_NULL);

    /* Send message to usb message queue */
    return os_mq_send(&usb_mq, (void *)msg, sizeof(struct udev_msg), OS_NO_WAIT);
}

OS_ALIGN(OS_ALIGN_SIZE)
static os_uint8_t     usb_task_stack[OS_USBD_TASK_STACK_SZ];
static struct os_task usb_task;
#define USBD_MQ_MSG_SZ  32
#define USBD_MQ_MAX_MSG 16
/*
 * Internal of the message queue: every message is associated with a pointer,
 * so in order to recveive USBD_MQ_MAX_MSG messages, we have to allocate more
 * than USBD_MQ_MSG_SZ*USBD_MQ_MAX_MSG memery.
 */
static os_uint8_t usb_mq_pool[(USBD_MQ_MSG_SZ + sizeof(void *)) * USBD_MQ_MAX_MSG];

os_err_t os_usbd_core_init(void)
{
    os_list_init(&device_list);

    /* Create an usb message queue */
    os_mq_init(&usb_mq, "usbd", usb_mq_pool, sizeof(usb_mq_pool), USBD_MQ_MSG_SZ);

    /* Init usb device task */
    os_task_init(&usb_task,
                 "usbd",
                 os_usbd_task_entry,
                 OS_NULL,
                 usb_task_stack,
                 OS_USBD_TASK_STACK_SZ,
                 OS_USBD_TASK_PRIO);
    /* os_task_init should always be OK, so start the task without further
     * Checking. */
    return os_task_startup(&usb_task);
}
