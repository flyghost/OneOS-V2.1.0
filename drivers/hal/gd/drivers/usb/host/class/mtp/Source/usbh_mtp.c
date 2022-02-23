/*!
    \file  usbh_mtp.c
    \brief USB host MTP driver

    \version 2018-10-08, V1.0.0, firmware for GD32 USBFS&USBHS
*/

/*
    Copyright (c) 2018, GigaDevice Semiconductor Inc.

    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this 
       list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice, 
       this list of conditions and the following disclaimer in the documentation 
       and/or other materials provided with the distribution.
    3. Neither the name of the copyright holder nor the names of its contributors 
       may be used to endorse or promote products derived from this software without 
       specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR 
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
OF SUCH DAMAGE.
*/

#include "usbh_mtp.h"
#include "usbh_transc.h"
#include "usbh_pipe.h"
#include "usbh_enum.h"
#include "lcd_log.h"

static usbh_status usbh_mtp_interface_init (usbh_host *puhost);
static void usbh_mtp_interface_deinit      (usbh_host *puhost);
static usbh_status usbh_mtp_process        (usbh_host *puhost);
static usbh_status usbh_mtp_class_request  (usbh_host *puhost);
static usbh_status usbh_mtp_sof            (usbh_host *puhost);

static uint8_t usbh_ctl_endpoint_find      (usbh_host *puhost);
static uint8_t usbh_data_out_endpoint_find (usbh_host *puhost);
static uint8_t usbh_data_in_endpoint_find  (usbh_host *puhost);

static usbh_status usbh_mtp_events (usbh_host *puhost);
static void mtp_event_decode (usbh_host *puhost);

usbh_class usbh_mtp =
{
    USB_MTP_CLASS,
    usbh_mtp_interface_init,
    usbh_mtp_interface_deinit,
    usbh_mtp_class_request,
    usbh_mtp_process,
    usbh_mtp_sof
};

/*!
    \brief      the function init the MTP class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static usbh_status usbh_mtp_interface_init (usbh_host *puhost)
{
    usbh_status status = USBH_OK;
    uint8_t interface, endpoint;

    usb_desc_ep *ep_desc = NULL;

    interface = puhost->dev_prop.cur_itf;

    if (interface == 0xFFU) {
        status = USBH_FAIL;
        LCD_ErrLog ("Cannot Find the interface for Still Image Class.\n");
    } else {
        static usbh_mtp_handle mtp_handler;

        memset((void *)&mtp_handler, 0U, sizeof(usbh_mtp_handle));

        endpoint = usbh_ctl_endpoint_find(puhost);

        usbh_mtp_handle *mtp = &mtp_handler;

        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[endpoint];

        /* collect the control endpoint address and length */
        mtp->ep_notify = ep_desc->bEndpointAddress;
        mtp->ep_size_notify = ep_desc->wMaxPacketSize;
        mtp->pipe_notify = usbh_pipe_allocate (puhost->data, mtp->ep_notify);
        mtp->events.poll = ep_desc->bInterval;

        /* Open pipe for Notification endpoint */
        usbh_pipe_create (puhost->data,
                          &puhost->dev_prop,
                          mtp->pipe_notify,
                          USB_EPTYPE_INTR,
                          mtp->ep_size_notify);

        usbh_pipe_toggle_set (puhost->data, mtp->pipe_notify, 0U);

        endpoint = usbh_data_in_endpoint_find(puhost);

        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[endpoint];

        /* collect the control endpoint address and length */
        mtp->ep_data_in = ep_desc->bEndpointAddress;
        mtp->ep_size_data_in = ep_desc->wMaxPacketSize;
        mtp->pipe_data_in = usbh_pipe_allocate (puhost->data, mtp->ep_data_in);

        /* open pipe for data in endpoint */
        usbh_pipe_create  (puhost->data, 
                           &puhost->dev_prop, 
                           mtp->pipe_data_in,
                           USB_EPTYPE_BULK,
                           mtp->ep_size_data_in);

        usbh_pipe_toggle_set (puhost->data, mtp->pipe_data_in, 0U);

        endpoint = usbh_data_out_endpoint_find(puhost);

        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[endpoint];

        /* collect the data out endpoint address and length */
        mtp->ep_data_out = ep_desc->bEndpointAddress;
        mtp->ep_size_data_out = ep_desc->wMaxPacketSize;
        mtp->pipe_data_out = usbh_pipe_allocate (puhost->data, mtp->ep_data_out);

        /* open pipe for data out endpoint */
        usbh_pipe_create (puhost->data,
                          &puhost->dev_prop,
                          mtp->pipe_data_out,
                          USB_EPTYPE_BULK,
                          mtp->ep_size_data_out);

        usbh_pipe_toggle_set (puhost->data, mtp->pipe_data_out, 0U);

        mtp->is_ready = 0U;
        mtp->state = MTP_OPENSESSION;
        mtp->events.state = MTP_EVENTS_INIT;

        puhost->active_class->class_data = (void *)&mtp_handler;

        return usbh_ptp_init(puhost);
    }

    return status;
}

/*!
    \brief      find MTP control interface
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static uint8_t usbh_ctl_endpoint_find (usbh_host *puhost)
{
    uint8_t interface = puhost->dev_prop.cur_itf;
    usb_desc_ep *ep_desc = NULL;

    for (uint8_t ep_num = 0U; ep_num < USBH_MAX_EP_NUM; ep_num++) {
        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[ep_num];

        if ((ep_desc->bEndpointAddress & 0x80U) &&
              (ep_desc->wMaxPacketSize > 0U) &&
                ((ep_desc->bmAttributes & USB_EPTYPE_INTR) == USB_EPTYPE_INTR)) {
            return ep_num;
        }
    }

    return 0xFFU; /* Invalid Endpoint */
}

/*!
    \brief      find MTP data OUT interface
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static uint8_t usbh_data_out_endpoint_find (usbh_host *puhost)
{
    uint8_t interface = puhost->dev_prop.cur_itf;
    usb_desc_ep *ep_desc = NULL;

    for (uint8_t ep_num = 0U; ep_num < USBH_MAX_EP_NUM; ep_num++) {
        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[ep_num];

        if (((ep_desc->bEndpointAddress & 0x80U) == 0U) &&
             (ep_desc->wMaxPacketSize > 0U) &&
               ((ep_desc->bmAttributes & USB_EPTYPE_BULK) == USB_EPTYPE_BULK)) {
            return ep_num;
        }
    }

    return 0xFFU; /* Invalid Endpoint */
}

/*!
    \brief      find MTP data IN interface
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static uint8_t usbh_data_in_endpoint_find (usbh_host *puhost)
{
    uint8_t interface = puhost->dev_prop.cur_itf;
    usb_desc_ep *ep_desc = NULL;

    for (uint8_t ep_num = 0U; ep_num < USBH_MAX_EP_NUM; ep_num++) {
        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[ep_num];

        if ((ep_desc->bEndpointAddress & 0x80U) &&
             (ep_desc->wMaxPacketSize > 0U) &&
               ((ep_desc->bmAttributes & USB_EPTYPE_BULK) == USB_EPTYPE_BULK)) {
            return ep_num;
        }
    }

    return 0xFFU; /* Invalid Endpoint */
}

/*!
    \brief      the function deinit the pipes used for the MTP class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static void usbh_mtp_interface_deinit (usbh_host *puhost)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    if (mtp->pipe_data_out) {
        usb_pipe_halt (puhost->data, mtp->pipe_data_out);
        usbh_pipe_free (puhost->data, mtp->pipe_data_out);
        mtp->pipe_data_out = 0U;     /* reset the channel as free */
    }

    if (mtp->pipe_data_in) {
        usb_pipe_halt (puhost->data, mtp->pipe_data_in);
        usbh_pipe_free (puhost->data, mtp->pipe_data_in);
        mtp->pipe_data_in = 0U;     /* reset the channel as free */
    }

    if (mtp->pipe_notify) {
        usb_pipe_halt (puhost->data, mtp->pipe_notify);
        usbh_pipe_free (puhost->data, mtp->pipe_notify);
        mtp->pipe_notify = 0U;     /* reset the channel as free */
    }
}

/*!
    \brief      the function is responsible for handling Standard requests for MTP class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static usbh_status usbh_mtp_class_request (usbh_host *puhost)
{
    return USBH_OK;
}

/*!
    \brief      the function is for managing state machine for MTP data transfers
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static usbh_status usbh_mtp_process (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t idx = 0U;

    switch (mtp->state) {
        case MTP_OPENSESSION:
            status = usbh_ptp_session_open (puhost, 1U); /* Session '0' is not valid */

            if (status == USBH_OK) {
                LCD_UsrLog("> MTP Session #0 Opened\n");
                mtp->state = MTP_GETDEVICEINFO;
            }
            break;

        case MTP_GETDEVICEINFO:
            status = usbh_ptp_device_info_get (puhost, &(mtp->info.dev_info));

            if (status == USBH_OK) {
                LCD_DbgLog(">>>>> MTP Device Information\n");
                LCD_DbgLog("Standard version: %x\n", mtp->info.dev_info.StandardVersion);
                LCD_DbgLog("Vendor ExtID: %s\n", (mtp->info.dev_info.VendorExtensionID == 6)?"MTP": "NOT SUPPORTED");
                LCD_DbgLog("Functional mode: %s\n", (mtp->info.dev_info.FunctionalMode == 0U) ? "Standard" : "Vendor");
                LCD_DbgLog("Number of Supported Operation(s): %d\n", mtp->info.dev_info.OperationsSupported_len);
                LCD_DbgLog("Number of Supported Events(s): %d\n", mtp->info.dev_info.EventsSupported_len);
                LCD_DbgLog("Number of Supported Proprieties: %d\n", mtp->info.dev_info.DevicePropertiesSupported_len);
                LCD_DbgLog("Manufacturer: %s\n", mtp->info.dev_info.Manufacturer);
                LCD_DbgLog("Model: %s\n", mtp->info.dev_info.Model);
                LCD_DbgLog("Device version: %s\n", mtp->info.dev_info.DeviceVersion);
                LCD_DbgLog("Serial number: %s\n", mtp->info.dev_info.SerialNumber);

                mtp->state = MTP_GETSTORAGEIDS;
            }
            break;

        case MTP_GETSTORAGEIDS:
            status = usbh_ptp_storageID_get (puhost, &(mtp->info.storIDs));

            if (status == USBH_OK) {
                LCD_DbgLog("Number of storage ID items: %d\n", mtp->info.storIDs.n);

                for (idx  = 0U; idx < mtp->info.storIDs.n; idx ++) {
                    LCD_DbgLog("storage#%d ID: %x\n", idx, mtp->info.storIDs.Storage[idx]);
                }

                mtp->current_storage_unit = 0U;
                mtp->state = MTP_GETSTORAGEINFO;
            }
            break;

        case MTP_GETSTORAGEINFO:
            status = usbh_ptp_storage_info_get (puhost,
                                                mtp->info.storIDs.Storage[mtp->current_storage_unit],
                                                &((mtp->info.stor_info)[mtp->current_storage_unit]));

            if (status == USBH_OK) {
                LCD_DbgLog("Volume#%lu: %s [%s]\n", mtp->current_storage_unit,
                            mtp->info.stor_info[mtp->current_storage_unit].StorageDescription,
                            mtp->info.stor_info[mtp->current_storage_unit].VolumeLabel);

                if (++mtp->current_storage_unit >= mtp->info.storIDs.n) {
                    mtp->state = MTP_IDLE;
                    mtp->is_ready = 1U;
                    mtp->current_storage_unit = 0U;
                    mtp->params.current_storageID = mtp->info.storIDs.Storage[0];

                    LCD_UsrLog("> MTP Class initialized.\n");
                    LCD_UsrLog("> %s is default storage unit\n", mtp->info.stor_info[0].StorageDescription);

                    puhost->usr_cb->dev_user_app();
                }
            }
            break;

        case MTP_IDLE:
            usbh_mtp_events(puhost);

            status = USBH_OK;
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      the function is for managing SOF callback
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     usbh_status
*/
static usbh_status usbh_mtp_sof (usbh_host *puhost)
{
    usbh_status status = USBH_OK;

    return status;
}

/*!
    \brief      select the storage unit to be used
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     usbh_status
*/
uint8_t usbh_mtp_isready (usbh_host *puhost)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;;

    return ((uint8_t)mtp->is_ready);
}

/*!
    \brief      get storage number
    \param[in]  pudev: pointer to usb core instance
    \param[out] storage_num: storage number
    \retval     usbh_status
*/
usbh_status usbh_mtp_storage_num_get (usbh_host *puhost, uint8_t *storage_num)
{
    usbh_status status = USBH_FAIL;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;;

    if (mtp->is_ready > 0U) {
        *storage_num = (uint8_t)mtp->info.storIDs.n;
        status = USBH_OK;
    }

    return status;
}

/*!
    \brief      get storage
    \param[in]  pudev: pointer to usb core instance
    \param[in]  storage_idx: storage index
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_mtp_storage_select (usbh_host *puhost, uint8_t storage_idx)
{
    usbh_status status = USBH_FAIL;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;;

    if ((storage_idx < mtp->info.storIDs.n) && (mtp->is_ready)) {
        mtp->params.current_storageID = mtp->info.storIDs.Storage[storage_idx];
        status  = USBH_OK;
    }

    return status;
}

/*!
    \brief      get storage unit information
    \param[in]  pudev: pointer to usb core instance
    \param[in]  storage_idx: storage index
    \param[out] info: storage unit information
    \retval     usbh_status
*/
usbh_status usbh_mtp_storage_info_get (usbh_host *puhost, uint8_t storage_idx, mtp_storage_info *info)
{
    usbh_status status = USBH_FAIL;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    if ((storage_idx < mtp->info.storIDs.n) && (mtp->is_ready)) {
        *info = mtp->info.stor_info[storage_idx];
        status  = USBH_OK;
    }

    return status;
}

/*!
    \brief      get object number
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  storage_idx: storage index
    \param[in]  object_format_code: object format code
    \param[in]  associationOH:
    \param[out] numobs: number of objects
    \retval     usbh_status
*/
usbh_status usbh_mtp_objects_num_get (usbh_host *puhost,
                                      uint32_t storage_idx,
                                      uint32_t object_format_code,
                                      uint32_t associationOH,
                                      uint32_t* numobs)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if ((storage_idx < mtp->info.storIDs.n) && (mtp->is_ready)) {
        while ((status = usbh_ptp_objects_num_get (puhost,
                                                   mtp->info.storIDs.Storage[storage_idx],
                                                   object_format_code,
                                                   associationOH,
                                                   numobs)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) > 5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      get object handlers
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  storage_idx: storage index
    \param[in]  object_format_code: object format code
    \param[in]  associationOH:
    \param[out] object_handles: object handles
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_handles_get (usbh_host *puhost,
                                         uint32_t storage_idx,
                                         uint32_t object_format_code,
                                         uint32_t associationOH,
                                         ptp_object_handle* object_handles)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if ((storage_idx < mtp->info.storIDs.n) && (mtp->is_ready)) {
        while ((status = usbh_ptp_objects_handles_get (puhost,
                                                       mtp->info.storIDs.Storage[storage_idx],
                                                       object_format_code,
                                                       associationOH,
                                                       object_handles)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) > 5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      get object information
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  handle: object handle
    \param[out] object_info: object information
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_info_get (usbh_host *puhost, uint32_t handle, ptp_object_info* object_info)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_object_info_get (puhost, handle, object_info)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) > 5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      delete object
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  handle: object handle
    \param[in]  object_format_code: object formate code
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_delete (usbh_host *puhost, uint32_t handle, uint32_t object_format_code)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_object_delete (puhost, handle, object_format_code)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) > 5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      get object
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  handle: object handle
    \param[out] object: object to be get
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_get (usbh_host *puhost, uint32_t handle, uint8_t *object)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_object_get (puhost, handle, object)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) > 5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      get partial object
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  handle: object handle
    \param[in]  offset: 
    \param[in]  maxbytes:
    \param[in]  len:
    \param[out] object: object to be get
    \retval     usbh_status
*/
usbh_status usbh_mtp_partial_object_get(usbh_host *puhost,
                                        uint32_t handle,
                                        uint32_t offset,
                                        uint32_t maxbytes,
                                        uint8_t *object,
                                        uint32_t *len)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_partial_object_get(puhost, handle, offset, maxbytes, object, len)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) > 5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      get supported object property
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  ofc:
    \param[in]  propnum: 
    \param[in]  props:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_props_supported_get (usbh_host *puhost,
                                                 uint16_t ofc,
                                                 uint32_t *propnum,
                                                 uint16_t *props)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_object_props_supported_get (puhost, ofc, propnum, props)) == USBH_BUSY) {
            if(((puhost->control.timer - timeout) >  5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      get object property descriptor
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  opc: 
    \param[in]  ofc: 
    \param[in]  opd:
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_prop_desc_get (usbh_host *puhost,
                                           uint16_t opc,
                                           uint16_t ofc,
                                           ptp_object_prop_desc *opd)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_object_prop_desc_get(puhost, opc, ofc, opd)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) >  5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      get object property list
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  handle: object handle
    \param[in]  pprops: 
    \param[in]  nrofprops:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_prop_list_get (usbh_host *puhost,
                                           uint32_t handle,
                                           mtp_properties *pprops,
                                           uint32_t *nrofprops)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_object_prop_list_get (puhost, handle, pprops, nrofprops)) == USBH_BUSY) {
            if(((puhost->control.timer - timeout) >  5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      send an object
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  handle: object handle
    \param[in]  object: object to be get
    \param[in]  size:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_mtp_object_send (usbh_host *puhost,
                                  uint32_t handle,
                                  uint8_t *object,
                                  uint32_t size)
{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_object_send (puhost, handle, object, size)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) >  5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      handle MTP control events
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[out] none
    \retval     usbh_status
*/
static usbh_status usbh_mtp_events (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY ;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    switch (mtp->events.state) {
        case MTP_EVENTS_INIT:
            if ((puhost->control.timer & 1U) == 0U) {
                mtp->events.timer = puhost->control.timer;
                usbh_data_recev (puhost->data,
                                (uint8_t *)(void *)&(mtp->events.container),
                                 mtp->pipe_notify,
                                (uint8_t)mtp->ep_size_notify);

                mtp->events.state = MTP_EVENTS_GETDATA ;
            }
            break;

        case MTP_EVENTS_GETDATA:
            if (usbh_urbstate_get(puhost->data, mtp->pipe_notify) == URB_DONE) {
                mtp_event_decode(puhost);
            }

            if ((puhost->control.timer - mtp->events.timer) >= mtp->events.poll) {
                mtp->events.timer = puhost->control.timer;

                usbh_data_recev (puhost->data,
                                (uint8_t *)(void *)&(mtp->events.container),
                                 mtp->pipe_notify,
                                (uint8_t)mtp->ep_size_notify);

            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      decode device event sent by responder
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     usbh_status
*/
static void mtp_event_decode (usbh_host *puhost)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    uint16_t code;
    uint32_t param1;

    /* Process the event */
    code = mtp->events.container.code;
    param1 = mtp->events.container.param1;

    switch(code) {
        case PTP_EC_UNDEFINED:
//            USBH_DbgLog("EVT: PTP_EC_Undefined in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_CANCEL_TRANSACTION:
//            USBH_DbgLog("EVT: PTP_EC_CancelTransaction in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_OBJECT_ADDED:
//            USBH_DbgLog("EVT: PTP_EC_ObjectAdded in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_OBJECT_REMOVED:
//            USBH_DbgLog("EVT: PTP_EC_ObjectRemoved in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_STORE_ADDED:
//            USBH_DbgLog("EVT: PTP_EC_StoreAdded in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_STORE_REMOVED:
//            USBH_DbgLog("EVT: PTP_EC_StoreRemoved in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_DEVICE_PROP_CHANGED:
//            USBH_DbgLog("EVT: PTP_EC_DevicePropChanged in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_OBJECT_INFO_CHANGED:
//            USBH_DbgLog("EVT: PTP_EC_ObjectInfoChanged in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_DEVICE_INFO_CHANGED:
//            USBH_DbgLog("EVT: PTP_EC_DeviceInfoChanged in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_REQUEST_OBJECT_TRANSFER:
//            USBH_DbgLog("EVT: PTP_EC_RequestObjectTransfer in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_STORE_FULL:
//            USBH_DbgLog("EVT: PTP_EC_StoreFull in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_DEVICE_RESET:
//            USBH_DbgLog("EVT: PTP_EC_DeviceReset in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_STORAGE_INFO_CHANGED :
//            USBH_DbgLog("EVT: PTP_EC_StorageInfoChanged in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_CAPTURE_COMPLETE :
//            USBH_DbgLog("EVT: PTP_EC_CaptureComplete in session %u", mtp->ptp.session_id);
            break;

        case PTP_EC_UNREPORTED_STATUS :
//            USBH_DbgLog("EVT: PTP_EC_UnreportedStatus in session %u", mtp->ptp.session_id);
            break;

        default:
//            USBH_DbgLog("Received unknown event in session %u", mtp->ptp.session_id);
            break;
    }

    usbh_mtp_event_callback(puhost->data, (uint32_t)code, param1);
}

/*!
    \brief      get device property descriptor
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to USB host
    \param[in]  propcode: 
    \param[out] devicepropertydesc: device property descriptor
    \retval     usbh_status
*/
usbh_status usbh_mtp_device_prop_desc_get (usbh_host *puhost, uint16_t propcode, ptp_dev_prop_desc* devicepropertydesc)

{
    usbh_status status = USBH_FAIL;
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint32_t timeout = puhost->control.timer;

    if (mtp->is_ready) {
        while ((status = usbh_ptp_device_prop_desc_get (puhost, propcode, devicepropertydesc)) == USBH_BUSY) {
            if (((puhost->control.timer - timeout) > 5000U) || (pudev->host.connect_status == 0U)) {
                return USBH_FAIL;
            }
        }
    }

    return status;
}

/*!
    \brief      the function informs that host has received an event
    \param[in]  pudev: pointer to usb core instance
    \param[in]  event: 
    \param[in]  param:
    \param[out] none
    \retval     usbh_status
*/
__weak void usbh_mtp_event_callback(usb_core_driver *pudev, uint32_t event, uint32_t param)
{

}

