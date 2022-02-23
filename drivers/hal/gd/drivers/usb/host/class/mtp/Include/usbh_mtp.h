/*!
    \file  usbh_mtp.h
    \brief header file for USB host MTP driver

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


#ifndef __USBH_MTP_H
#define __USBH_MTP_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbh_mtp_ptp.h"
#include "usbh_core.h"


/* communication class codes */
#define USB_MTP_CLASS                                           0x06U /* (still image class)*/
#define USB_VENDOR_CLASS                                        0xFFU

#define MTP_MAX_STORAGE_UNITS_NBR                               PTP_MAX_STORAGE_UNITS_NBR

typedef enum
{
    MTP_IDLE = 0,
    MTP_GETDEVICEINFO,
    MTP_OPENSESSION,
    MTP_CLOSESESSION,
    MTP_GETSTORAGEIDS,
    MTP_GETSTORAGEINFO,
} mtp_state;

typedef enum
{
    MTP_EVENTS_INIT = 0,
    MTP_EVENTS_GETDATA,
} mtp_events_state;

typedef struct
{
    mtp_events_state        state;
    uint32_t                timer;
    uint16_t                poll;
    ptp_event_container     container;
} mtp_event_handle;

typedef struct
{
    uint32_t        current_storageID;
    uint32_t        object_format_code;
    uint32_t        current_object_handler;
    uint8_t         object_handler_nbr;
    uint32_t        obj_depth;
} mtp_params;

typedef struct
{
    ptp_device_info     dev_info;
    ptp_storage_ID      storIDs;
    ptp_storage_info    stor_info[MTP_MAX_STORAGE_UNITS_NBR];
    ptp_object_handle   handles;
} mtp_info;

/* Structure for MTP process */
typedef struct _mtp_process
{
    mtp_info              info;
    mtp_params            params;

    uint8_t               pipe_data_in;
    uint8_t               pipe_data_out;
    uint8_t               pipe_notify;

    uint8_t               ep_data_in;
    uint8_t               ep_data_out;
    uint8_t               ep_notify;

    uint16_t              ep_size_data_in;
    uint16_t              ep_size_data_out;
    uint16_t              ep_size_notify;
    mtp_state             state;
    mtp_event_handle      events;
    ptp_handle            ptp;
    uint32_t              current_storage_unit;
    uint32_t              is_ready;
} usbh_mtp_handle;

#define mtp_storage_info      ptp_storage_info
#define mtp_object_handles    ptp_object_handle
#define mtp_object_info       ptp_object_info


extern usbh_class  usbh_mtp;

uint8_t usbh_mtp_isready (usbh_host *puhost);

usbh_status usbh_mtp_storage_select (usbh_host *puhost, uint8_t storage_idx);

usbh_status usbh_mtp_storage_num_get (usbh_host *puhost, uint8_t *storage_num);

usbh_status usbh_mtp_objects_num_get (usbh_host *puhost, 
                                      uint32_t  storage_idx,
                                      uint32_t  object_format_code,
                                      uint32_t  associationOH,
                                      uint32_t* numobs);

usbh_status usbh_mtp_storage_info_get (usbh_host *puhost, uint8_t storage_idx, mtp_storage_info *info);

usbh_status usbh_mtp_object_handles_get (usbh_host *puhost,
                                         uint32_t storage_idx,
                                         uint32_t object_format_code,
                                         uint32_t associationOH,
                                         ptp_object_handle* object_handles);

usbh_status usbh_mtp_object_info_get (usbh_host *puhost, uint32_t handle, ptp_object_info* object_info);

usbh_status usbh_mtp_object_delete (usbh_host *puhost, uint32_t handle, uint32_t object_format_code);

usbh_status usbh_mtp_object_get (usbh_host *puhost, uint32_t handle, uint8_t *object);

usbh_status usbh_mtp_partial_object_get (usbh_host *puhost,
                                         uint32_t   handle,
                                         uint32_t   offset,
                                         uint32_t   maxbytes,
                                         uint8_t   *object,
                                         uint32_t  *len);

usbh_status usbh_mtp_object_props_supported_get (usbh_host *puhost,
                                                 uint16_t   ofc,
                                                 uint32_t  *propnum,
                                                 uint16_t  *props);

usbh_status usbh_mtp_object_prop_desc_get (usbh_host *puhost,
                                           uint16_t opc,
                                           uint16_t ofc,
                                           ptp_object_prop_desc *opd);

usbh_status usbh_mtp_object_prop_list_get (usbh_host *puhost,
                                           uint32_t handle,
                                           mtp_properties *pprops,
                                           uint32_t *nrofprops);

usbh_status usbh_mtp_object_send (usbh_host *puhost,
                                  uint32_t handle,
                                  uint8_t *object,
                                  uint32_t size);

usbh_status usbh_mtp_device_prop_desc_get (usbh_host *puhost, uint16_t propcode, ptp_dev_prop_desc* device_property_desc);

void usbh_mtp_event_callback (usb_core_driver *pudev, uint32_t event, uint32_t param);

#ifdef __cplusplus
}
#endif

#endif /* __USBH_MTP_H */

