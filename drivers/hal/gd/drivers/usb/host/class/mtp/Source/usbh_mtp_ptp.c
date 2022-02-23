/*!
    \file  usbh_mtp_ptp.c
    \brief USB host PTP in MTP driver

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

#include "usbh_mtp_ptp.h"
#include "usbh_mtp.h"
#include "usbh_transc.h"
#include "usbh_enum.h"

#include <string.h>

static void ptp_decode_device_info (usbh_host *puhost, ptp_device_info *dev_info);
static void ptp_storageID_get      (usbh_host *puhost, ptp_storage_ID *stor_ids);
static void ptp_storage_info_get   (usbh_host *puhost, uint32_t storage_id, ptp_storage_info *stor_info);
static void ptp_object_info_get    (usbh_host *puhost, ptp_object_info *object_info);
static void ptp_object_prop_get    (usbh_host *puhost, ptp_object_prop_desc *opd, uint32_t opdlen);

static void ptp_device_prop_value_get(usbh_host *puhost,
                                      uint32_t *offset,
                                      uint32_t total,
                                      ptp_property_value* value,
                                      uint16_t datatype);

static uint32_t ptp_object_prop_list_get (usbh_host *puhost, mtp_properties *props, uint32_t len);


static void ptp_buffer_full_callback (usbh_host *puhost);

static void ptp_string_get (uint8_t *str, uint8_t* data, uint16_t *len);

static uint32_t ptp_array16_get (uint16_t *array, uint8_t *data, uint32_t offset);
static uint32_t ptp_array32_get (uint32_t *array, uint8_t *data, uint32_t offset);

/*!
    \brief      the function initializes the PTP protocol
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_init(usbh_host *puhost)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    /* set state to idle to be ready for operations */
    mtp->ptp.state = PTP_IDLE;
    mtp->ptp.req_state = PTP_REQ_SEND;

    return USBH_OK;
}

/*!
    \brief      the function handle the PTP protocol
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_process (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usb_urb_state urb_status = URB_IDLE;
    usbh_mtp_handle* mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;
    uint32_t len = 0U;

    switch (mtp->ptp.state) {
        case PTP_IDLE:
            /*Do Nothing */
            break;

        case PTP_OP_REQUEST_STATE:
            usbh_data_send (puhost->data,
                           (uint8_t*)(void *)&(mtp->ptp.op_container),
                            mtp->pipe_data_out,
                           (uint16_t)mtp->ptp.op_container.length);

            mtp->ptp.state = PTP_OP_REQUEST_WAIT_STATE;
            break;

        case PTP_OP_REQUEST_WAIT_STATE:
            urb_status = usbh_urbstate_get(puhost->data, mtp->pipe_data_out);

            if (urb_status == URB_DONE) {
                if (mtp->ptp.flags == PTP_DP_NODATA) {
                    mtp->ptp.state = PTP_RESPONSE_STATE;
                } else if(mtp->ptp.flags == PTP_DP_SENDDATA) {
                    mtp->ptp.state = PTP_DATA_OUT_PHASE_STATE;
                } else if(mtp->ptp.flags == PTP_DP_GETDATA) {
                    mtp->ptp.state = PTP_DATA_IN_PHASE_STATE;
                } else {
                }
            } else if(urb_status == URB_NOTREADY) {
                /* resend request */
                mtp->ptp.state = PTP_OP_REQUEST_STATE;
            } else if(urb_status == URB_STALL) {
                mtp->ptp.state  = PTP_ERROR;
            } else {
            }
            break;

        case PTP_DATA_OUT_PHASE_STATE:
            usbh_data_send (puhost->data,
                            mtp->ptp.data_ptr,
                            mtp->pipe_data_out,
                            mtp->ep_size_data_out);

            mtp->ptp.state  = PTP_DATA_OUT_PHASE_WAIT_STATE;
            break;

        case PTP_DATA_OUT_PHASE_WAIT_STATE:
            urb_status = usbh_urbstate_get(puhost->data, mtp->pipe_data_out);

            if (urb_status == URB_DONE) {
                /* adjust data pointer and data length */
                if (mtp->ptp.data_length > mtp->ep_size_data_out) {
                    mtp->ptp.data_ptr += mtp->ep_size_data_out;
                    mtp->ptp.data_length -= mtp->ep_size_data_out;
                    mtp->ptp.data_packet += mtp->ep_size_data_out;

                    if (mtp->ptp.data_packet >= PTP_USB_BULK_PAYLOAD_LEN_READ) {
                        ptp_buffer_full_callback (puhost);
                        mtp->ptp.data_packet = 0U;
                        mtp->ptp.iteration++;
                    }
                } else {
                    mtp->ptp.data_length = 0U;
                }

                /* more data to be sent */
                if (mtp->ptp.data_length > 0U) {
                    usbh_data_send (puhost->data,
                                    mtp->ptp.data_ptr,
                                    mtp->pipe_data_out,
                                    mtp->ep_size_data_out);
                } else {
                    /* if value was 0, and successful transfer, then change the state */
                    mtp->ptp.state = PTP_RESPONSE_STATE;
                }
            } else if (urb_status == URB_NOTREADY) {
                /* resend same data */
                mtp->ptp.state = PTP_DATA_OUT_PHASE_STATE;
            } else if(urb_status == URB_STALL) {
                mtp->ptp.state = PTP_ERROR;
            } else {
            }
            break;

        case PTP_DATA_IN_PHASE_STATE:
            /* send first packet */
            usbh_data_recev (puhost->data,
                             mtp->ptp.data_ptr,
                             mtp->pipe_data_in,
                             mtp->ep_size_data_in);

            mtp->ptp.state = PTP_DATA_IN_PHASE_WAIT_STATE;
            break;

        case PTP_DATA_IN_PHASE_WAIT_STATE:
            urb_status = usbh_urbstate_get(puhost->data, mtp->pipe_data_in);

            if (urb_status == URB_DONE) {
                len = usbh_xfercount_get (puhost->data, mtp->pipe_data_in);

                if (mtp->ptp.data_packet_counter++ == 0U) {
                    /* this is the first packet; so retrieve exact data length from payload */
                    mtp->ptp.data_length = *(uint32_t*)(void *)(mtp->ptp.data_ptr);
                    mtp->ptp.iteration = 0U;
                }

                if ((len >=  mtp->ep_size_data_in) && (mtp->ptp.data_length > 0U)) {
                    mtp->ptp.data_ptr += len;
                    mtp->ptp.data_length -= len;
                    mtp->ptp.data_packet += len;

                    if (mtp->ptp.data_packet >= PTP_USB_BULK_PAYLOAD_LEN_READ) {
                        ptp_buffer_full_callback (puhost);
                        mtp->ptp.data_packet = 0U;
                        mtp->ptp.iteration++;
                    }

                    /* continue receiving data*/
                    usbh_data_recev (puhost->data,
                                     mtp->ptp.data_ptr,
                                     mtp->pipe_data_in,
                                     mtp->ep_size_data_in);
                } else {
                    mtp->ptp.data_length -= len;
                    mtp->ptp.state = PTP_RESPONSE_STATE;
                }
            } else if(urb_status == URB_STALL) {
                mtp->ptp.state = PTP_ERROR;
            } else {
            }
            break;

        case PTP_RESPONSE_STATE:
            usbh_data_recev (puhost->data,
                             (uint8_t*)(void *)&(mtp->ptp.resp_container),
                             mtp->pipe_data_in,
                             PTP_USB_BULK_REQ_RESP_MAX_LEN);

            mtp->ptp.state = PTP_RESPONSE_WAIT_STATE;
            break;

        case PTP_RESPONSE_WAIT_STATE:
            urb_status = usbh_urbstate_get(puhost->data, mtp->pipe_data_in);

            if (urb_status == URB_DONE) {
                usbh_ptp_response_get (puhost, &ptp_container);

                if (ptp_container.Code == PTP_RC_OK) {
                    status = USBH_OK;
                } else {
                    status = USBH_FAIL;
                }

                mtp->ptp.req_state = PTP_REQ_SEND;
            } else if (urb_status == URB_STALL) {
                mtp->ptp.state = PTP_ERROR;
            } else {
            }
            break;

        case PTP_ERROR:
            mtp->ptp.req_state = PTP_REQ_SEND;
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      open a new session
    \param[in]  pudev: pointer to usb core instance
    \param[in]  req:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_request_send (usbh_host *puhost, ptp_container *req)
{
    usbh_status status = USBH_OK;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    /* clear ptp data container*/
    memset(&(mtp->ptp.op_container), 0, sizeof(ptp_op_container));

    /* build appropriate USB container */
    mtp->ptp.op_container.length = PTP_USB_BULK_REQ_LEN - (sizeof(uint32_t) * (5U - req->Nparam));
    mtp->ptp.op_container.type = PTP_USB_CONTAINER_COMMAND;
    mtp->ptp.op_container.code = req->Code;
    mtp->ptp.op_container.trans_id = req->Transaction_ID;
    mtp->ptp.op_container.param1 = req->Param1;
    mtp->ptp.op_container.param2 = req->Param2;
    mtp->ptp.op_container.param3 = req->Param3;
    mtp->ptp.op_container.param4 = req->Param4;
    mtp->ptp.op_container.param5 = req->Param5;

    return status;
}

/*!
    \brief      get response
    \param[in]  pudev: pointer to usb core instance
    \param[in]  resp:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_response_get (usbh_host *puhost, ptp_container *resp)
{
    usbh_status status = USBH_OK;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    /* build an appropriate PTPContainer */
    resp->Code = mtp->ptp.resp_container.code;
    resp->SessionID = mtp->ptp.session_id;
    resp->Transaction_ID = mtp->ptp.resp_container.trans_id;
    resp->Param1 = mtp->ptp.resp_container.param1;
    resp->Param2 = mtp->ptp.resp_container.param2;
    resp->Param3 = mtp->ptp.resp_container.param3;
    resp->Param4 = mtp->ptp.resp_container.param4;
    resp->Param5 = mtp->ptp.resp_container.param5;

    return status;
}

/*!
    \brief      the function informs user that data buffer is full
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     none
*/
static void ptp_buffer_full_callback(usbh_host *puhost)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;

    switch (mtp->ptp.data_container.code) {
        case PTP_OC_GET_DEVICE_INFO:
            ptp_decode_device_info (puhost, &(mtp->info.dev_info));
            break;

        case PTP_OC_GET_PARTIAL_OBJECT:
        case PTP_OC_GET_OBJECT:
            /* first packet is in the PTP data payload buffer */
            if (mtp->ptp.iteration == 0U) {
                /* copy it to object */
                memcpy(mtp->ptp.object_ptr, mtp->ptp.data_container.payload.data, PTP_USB_BULK_PAYLOAD_LEN_READ);

                /* next packet should be directly copied to object */
                mtp->ptp.data_ptr = (mtp->ptp.object_ptr + PTP_USB_BULK_PAYLOAD_LEN_READ);
            }
            break;

        case PTP_OC_SEND_OBJECT:
            /* first packet is in the PTP data payload buffer */
            if (mtp->ptp.iteration == 0U) {
                /* next packet should be directly copied to object */
                mtp->ptp.data_ptr = (mtp->ptp.object_ptr + PTP_USB_BULK_PAYLOAD_LEN_READ);
            }
            break;

        default:
            break;
    }
}

/*!
    \brief      gets device info dataset and fills deviceinfo structure
    \param[in]  pudev: pointer to usb core instance
    \param[in]  dev_info: Device info structure
    \param[out] none
    \retval     none
*/
static void ptp_decode_device_info (usbh_host *puhost, ptp_device_info *dev_info)
{
    uint16_t len = 0U;
    uint32_t totallen = 0U;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;;
    uint8_t *data = mtp->ptp.data_container.payload.data;


//  /* Max device info is PTP_USB_BULK_HS_MAX_PACKET_LEN_READ */
//  USBH_DbgLog (" MTP device info size exceeds internal buffer size. only available data are decoded.");

    if (mtp->ptp.iteration == 0U) {
        dev_info->StandardVersion = LE16(&data[PTP_DI_STANDARD_VERSION]);
        dev_info->VendorExtensionID = LE32(&data[PTP_DI_VENDOR_EXTENSION_ID]);
        dev_info->VendorExtensionVersion = LE16(&data[PTP_DI_VENDOR_EXTENSION_VERSION]);
        ptp_string_get(dev_info->VendorExtensionDesc, &data[PTP_DI_VENDOR_EXTENSION_DESC], &len);

        totallen = len * 2U + 1U;
        dev_info->FunctionalMode = LE16(&data[PTP_DI_FUNCTIONAL_MODE + totallen]);
        dev_info->OperationsSupported_len = ptp_array16_get ((uint16_t *)(void *)&dev_info->OperationsSupported, data, PTP_DI_OPERATIONS_SUPPORTED + totallen);

        totallen = totallen+dev_info->OperationsSupported_len * sizeof(uint16_t) + sizeof(uint32_t);
        dev_info->EventsSupported_len = ptp_array16_get ((uint16_t *)(void *)&dev_info->EventsSupported, data, PTP_DI_OPERATIONS_SUPPORTED + totallen);

        totallen = totallen + dev_info->EventsSupported_len * sizeof(uint16_t) + sizeof(uint32_t);
        dev_info->DevicePropertiesSupported_len = ptp_array16_get ((uint16_t *)(void *)&dev_info->DevicePropertiesSupported, data, PTP_DI_OPERATIONS_SUPPORTED + totallen);

        totallen = totallen + dev_info->DevicePropertiesSupported_len * sizeof(uint16_t) + sizeof(uint32_t);
        dev_info->CaptureFormats_len = ptp_array16_get ((uint16_t *)(void *)&dev_info->CaptureFormats, data, PTP_DI_OPERATIONS_SUPPORTED + totallen);

        totallen = totallen + dev_info->CaptureFormats_len * sizeof(uint16_t) + sizeof(uint32_t);
        dev_info->ImageFormats_len =  ptp_array16_get ((uint16_t *)(void *)&dev_info->ImageFormats, data, PTP_DI_OPERATIONS_SUPPORTED+totallen);

        totallen = totallen + dev_info->ImageFormats_len * sizeof(uint16_t) + sizeof(uint32_t);
        ptp_string_get(dev_info->Manufacturer, &data[PTP_DI_OPERATIONS_SUPPORTED + totallen], &len);

        totallen += len * 2U + 1U;
        ptp_string_get(dev_info->Model, &data[PTP_DI_OPERATIONS_SUPPORTED + totallen], &len);

        totallen += len * 2U + 1U;
        ptp_string_get(dev_info->DeviceVersion, &data[PTP_DI_OPERATIONS_SUPPORTED + totallen], &len);

        totallen += len * 2U + 1U;
        ptp_string_get(dev_info->SerialNumber, &data[PTP_DI_OPERATIONS_SUPPORTED + totallen], &len);
    }
}

/*!
    \brief      gets storage index and fills stor_ids structure
    \param[in]  pudev: pointer to usb core instance
    \param[in]  stor_ids: storage ID structure
    \param[out] none
    \retval     none
*/
static void ptp_storageID_get (usbh_host *puhost, ptp_storage_ID *stor_ids)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint8_t *data = mtp->ptp.data_container.payload.data;

    stor_ids->n = ptp_array32_get (stor_ids->Storage, data, 0U);
}

/*!
    \brief      gets storage index and fills stor_info structure
    \param[in]  pudev: pointer to usb core instance
    \param[in]  storage_id: storage ID structure
    \param[in]  stor_info:
    \param[out] none
    \retval     none
*/
static void ptp_storage_info_get (usbh_host *puhost, uint32_t storage_id, ptp_storage_info *stor_info)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint8_t *data = mtp->ptp.data_container.payload.data;

    uint16_t len;

    stor_info->StorageType = LE16(&data[PTP_SI_STORAGE_TYPE]);
    stor_info->FilesystemType = LE16(&data[PTP_SI_FILE_SYSTEM_TYPE]);
    stor_info->AccessCapability = LE16(&data[PTP_SI_ACCESS_CAPABILITY]);
    stor_info->MaxCapability = LE64(&data[PTP_SI_MAX_CAPABILITY]);
    stor_info->FreeSpaceInBytes = LE64(&data[PTP_SI_FREE_SPACE_IN_BYTES]);
    stor_info->FreeSpaceInImages = LE32(&data[PTP_SI_FREE_SPACE_IN_IMAGES]);

    ptp_string_get(stor_info->StorageDescription, &data[PTP_SI_STORAGE_DESCRIPTION], &len);
    ptp_string_get(stor_info->VolumeLabel, &data[PTP_SI_STORAGE_DESCRIPTION+len * 2U + 1U], &len);
}

/*!
    \brief      gets object information
    \param[in]  pudev: pointer to usb core instance
    \param[in]  object_info: object information structure
    \param[out] none
    \retval     none
*/
static void ptp_object_info_get (usbh_host *puhost, ptp_object_info *object_info)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint8_t *data = mtp->ptp.data_container.payload.data;
    uint16_t filenamelen;

    object_info->StorageID = LE32(&data[PTP_OI_STORAGE_ID]);
    object_info->ObjectFormat = LE16(&data[PTP_OI_OBJECT_FORMAT]);
    object_info->ProtectionStatus = LE16(&data[PTP_OI_PROTECTION_STATUS]);
    object_info->ObjectCompressedSize = LE64(&data[PTP_OI_OBJECT_COMPRESSED_SIZE]);

    /* for samsung galaxy */
    if ((data[PTP_OI_FILE_NAME_LEN] == 0U) && (data[PTP_OI_FILE_NAME_LEN + 4U] != 0U)) {
        data += 4;
    }

    object_info->ThumbFormat = LE16(&data[PTP_OI_THUMB_FORMAT]);
    object_info->ThumbCompressedSize = LE32(&data[PTP_OI_THUMB_COMPRESSED_SIZE]);
    object_info->ThumbPixWidth = LE32(&data[PTP_OI_THUMB_PIX_WIDTH]);
    object_info->ThumbPixHeight = LE32(&data[PTP_OI_THUMB_PIX_HEIGHT]);
    object_info->ImagePixWidth = LE32(&data[PTP_OI_IMAGE_PIX_WIDTH]);
    object_info->ImagePixHeight = LE32(&data[PTP_OI_IMAGE_PIX_HEIGHT]);
    object_info->ImageBitDepth = LE32(&data[PTP_OI_IMAGE_BIT_DEPTH]);
    object_info->ParentObject = LE32(&data[PTP_OI_PARENT_OBJECT]);
    object_info->AssociationType = LE16(&data[PTP_OI_ASSOCIATION_TYPE]);
    object_info->AssociationDesc = LE32(&data[PTP_OI_ASSOCIATION_DESC]);
    object_info->SequenceNumber = LE32(&data[PTP_OI_SEQUENCE_NUMBER]);
    ptp_string_get(object_info->Filename, &data[PTP_OI_FILE_NAME_LEN], &filenamelen);
}

/*!
    \brief      gets object property descriptor
    \param[in]  pudev: pointer to usb core instance
    \param[in]  opd:
    \param[in]  opdlen:
    \param[out] none
    \retval     none
*/
static void ptp_object_prop_get (usbh_host *puhost, ptp_object_prop_desc *opd, uint32_t opdlen)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint8_t *data = mtp->ptp.data_container.payload.data;
    uint32_t offset = 0U, i;

    opd->object_property_code = LE16(&data[PTP_OPD_OBJECT_PROPERTY_CODE]);
    opd->data_type = LE16(&data[PTP_OPD_DATA_TYPE]);
    opd->get_set = *(uint8_t *)(&data[PTP_OPD_GET_SET]);

    offset = PTP_OPD_FACTORY_DEFAULT_VALUE;
    ptp_device_prop_value_get (puhost, &offset, opdlen, &opd->factory_default_value, opd->data_type);

    opd->group_code = LE32(&data[offset]);
    offset += sizeof(uint32_t);

    opd->form_flag = *(uint8_t *)(&data[offset]);
    offset += sizeof(uint8_t);

    switch (opd->form_flag) {
        case PTP_OPFF_RANGE:
            ptp_device_prop_value_get(puhost, &offset, opdlen, &opd->form.range.minimum_value, opd->data_type);
            ptp_device_prop_value_get(puhost, &offset, opdlen, &opd->form.range.maximum_value, opd->data_type);
            ptp_device_prop_value_get(puhost, &offset, opdlen, &opd->form.range.step_size, opd->data_type);
            break;

        case PTP_OPFF_ENUMERATION:
            opd->form.enumf.number_of_values = LE16(&data[offset]);
            offset += sizeof(uint16_t);

            for (i = 0U; i < opd->form.enumf.number_of_values; i++) {
                ptp_device_prop_value_get(puhost, &offset, opdlen, &opd->form.enumf.supported_value[i], opd->data_type);
            }
            break;

        default:
            break;
    }
}

/*!
    \brief      gets device property value
    \param[in]  pudev: pointer to usb core instance
    \param[in]  offset: storage ID structure
    \param[in]  total:
    \param[in]  value:
    \param[in]  datatype:
    \param[out] none
    \retval     none
*/
static void ptp_device_prop_value_get(usbh_host *puhost,
                                      uint32_t *offset,
                                      uint32_t total,
                                      ptp_property_value* value,
                                      uint16_t datatype)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint8_t *data = mtp->ptp.data_container.payload.data;
    uint16_t len;

    switch (datatype) {
        case PTP_DTC_INT8:
            value->i8 = *(int8_t *)(void *)&(data[*offset]);
            *offset += 1U;
            break;

        case PTP_DTC_UINT8:
            value->u8 = *(uint8_t *)&(data[*offset]);
            *offset += 1U;
            break;

        case PTP_DTC_INT16:
            value->i16 = *(int16_t *)(void *)&(data[*offset]);
            *offset += 2U;
            break;

        case PTP_DTC_UINT16:
            value->u16 = LE16(&(data[*offset]));
            *offset += 2U;
            break;

        case PTP_DTC_INT32:
            value->i32 = *(int32_t *)(void *)(&(data[*offset]));
            *offset += 4U;
            break;

        case PTP_DTC_UINT32:
            value->u32 = LE32(&(data[*offset]));
            *offset += 4U;
            break;

        case PTP_DTC_INT64:
            value->i64 = *(int64_t *)(void *)(&(data[*offset]));
            *offset += 8U;
            break;

        case PTP_DTC_UINT64:
            value->u64 = LE64(&(data[*offset]));
            *offset += 8U;
            break;

        case PTP_DTC_UINT128:
            *offset += 16U;
            break;

        case PTP_DTC_INT128:
            *offset += 16U;
            break;

        case PTP_DTC_STR:
            ptp_string_get((uint8_t *)(void *)value->str, (uint8_t *)&(data[*offset]), &len);
            *offset += len * 2U + 1U;
            break;

        default:
            break;
    }
}

/*!
    \brief      gets object property list
    \param[in]  pudev: pointer to usb core instance
    \param[in]  props: 
    \param[in]  len:
    \param[out] none
    \retval     property count
*/
static uint32_t ptp_object_prop_list_get (usbh_host *puhost, mtp_properties *props, uint32_t len)
{
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    uint8_t *data = mtp->ptp.data_container.payload.data;
    uint32_t prop_count;
    uint32_t offset = 0U, i;

    prop_count = LE32(data);

    if (prop_count == 0U) {
        return 0U;
    }

    data += sizeof(uint32_t);
    len -= sizeof(uint32_t);

    for (i = 0U; i < prop_count; i++) {
        if (len <= 0U) {
            return 0U;
        }

        props[i].object_handle = LE32(data);
        data += sizeof(uint32_t);
        len -= sizeof(uint32_t);

        props[i].property = LE16(data);
        data += sizeof(uint16_t);
        len -= sizeof(uint16_t);

        props[i].data_type = LE16(data);
        data += sizeof(uint16_t);
        len -= sizeof(uint16_t);

        offset = 0U;

        ptp_device_prop_value_get(puhost, &offset, len, &props[i].prop_val, props[i].data_type);

        data += offset;
        len -= offset;
    }

    return prop_count;
}

/*!
    \brief      gets ASCII string form
    \param[in]  pudev: pointer to usb core instance
    \param[in]  str: ASCII string
    \param[in]  data:
    \param[in]  len:
    \param[out] none
    \retval     none
*/
static void ptp_string_get (uint8_t *str, uint8_t* data, uint16_t *len)
{
    uint16_t strlength;
    uint16_t idx;

    *len = data[0];
    strlength = (uint16_t)(2U * (uint32_t)data[0]);
    data ++; /* adjust the offset ignoring the string len */

    for (idx = 0U; idx < strlength; idx += 2U) {
        /* copy only the string and ignore the UNICODE ID, hence add the src */
        *str = data[idx];
        str++;
    }

    *str = 0U; /* mark end of string */
}

/*!
    \brief      gets 16 bit
    \param[in]  array: 
    \param[in]  data: 
    \param[in]  offset:
    \param[out] none
    \retval     array size
*/
static uint32_t ptp_array16_get (uint16_t *array, uint8_t *data, uint32_t offset)
{
    uint32_t size, idx = 0U;

    size = LE32(&data[offset]);
    while (size > idx) {
        array[idx] = (uint16_t)data[offset + (sizeof(uint16_t)*(idx + 2U))];
        idx++;
    }

    return size;
}

/*!
    \brief      gets 32 bit array form
    \param[in]  array: 
    \param[in]  data: 
    \param[in]  offset:
    \param[out] none
    \retval     array size
*/
static uint32_t ptp_array32_get (uint32_t *array, uint8_t *data, uint32_t offset)
{
    uint32_t size, idx = 0U;

    size = LE32(&data[offset]);

    while (size > idx) {
        array[idx] = LE32(&data[offset+(sizeof(uint32_t)*(idx+1U))]);
        idx++;
    }

    return size;
}

/*!
    \brief      open a new session
    \param[in]  pudev: pointer to usb core instance
    \param[in]  session: session ID (must be > 0U)
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_session_open (usbh_host *puhost, uint32_t session)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* init session params */
            mtp->ptp.transaction_id = 0x00000000U;
            mtp->ptp.session_id = session;
            mtp->ptp.flags = PTP_DP_NODATA;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_OPEN_SESSION;
            ptp_container.SessionID = session;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id++;
            ptp_container.Param1 = session;
            ptp_container.Nparam = 1U;

            /* convert request packet inti USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets storage index and fills stor_ids structure
    \param[in]  pudev: pointer to usb core instance
    \param[in]  propcode: 
    \param[in]  devicepropertydesc:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_device_prop_desc_get (usbh_host *puhost, uint16_t propcode, ptp_dev_prop_desc* device_property_desc)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;
    uint8_t *data = mtp->ptp.data_container.payload.data;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_DEVICE_PROP_DESC;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Param1 = propcode;
            ptp_container.Nparam = 1U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                device_property_desc->device_property_code = LE16(&data[PTP_DPD_DEVICE_PROPERTY_CODE]);
                device_property_desc->data_type = LE16(&data[PTP_DPD_DATA_TYPE]);
                device_property_desc->get_set = *(uint8_t *)(&data[PTP_DPD_GET_SET]);
                device_property_desc->form_flag = PTP_DPFF_NONE;
            }
            break;

        default:
            break;
    }
 
    return status;
}

/*!
    \brief      gets device information
    \param[in]  pudev: pointer to usb core instance
    \param[in]  dev_info: 
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_device_info_get (usbh_host *puhost, ptp_device_info *dev_info)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container  ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_DEVICE_INFO;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Nparam = 0U;

            /* convert request packet inti USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                ptp_decode_device_info (puhost, dev_info);
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets storage index
    \param[in]  pudev: pointer to usb core instance
    \param[in]  storage_ids:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_storageID_get (usbh_host *puhost, ptp_storage_ID *storage_ids)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_STORAGE_IDS;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Nparam = 0U;

            /* convert request packet inti USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                ptp_storageID_get (puhost, storage_ids);
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets storage information dataset
    \param[in]  pudev: pointer to usb core instance
    \param[in]  storage_id: storage ID
    \param[in]  storage_info:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_storage_info_get (usbh_host *puhost, uint32_t storage_id, ptp_storage_info *storage_info)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container  ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_STORAGE_INFO;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Param1 = storage_id;
            ptp_container.Nparam = 1U;

            /* convert request packet to USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
              ptp_storage_info_get (puhost, storage_id, storage_info);
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets objects number
    \param[in]  pudev: pointer to usb core instance
    \param[in]  storage_id: storage ID
    \param[in]  object_format_code:
    \param[in]  associationOH:
    \param[in]  numobs:
    \param[out] none
    \retval     usbh status
*/
usbh_status usbh_ptp_objects_num_get (usbh_host *puhost,
                                      uint32_t storage_id,
                                      uint32_t object_format_code,
                                      uint32_t associationOH,
                                      uint32_t* numobs)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
        /* set operation request type */
        mtp->ptp.flags = PTP_DP_NODATA;

        /* fill operation request params */
        ptp_container.Code = PTP_OC_GET_NUM_OBJECTS;
        ptp_container.SessionID = mtp->ptp.session_id;
        ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
        ptp_container.Param1 = storage_id;
        ptp_container.Param2 = object_format_code;
        ptp_container.Param3 = associationOH;
        ptp_container.Nparam = 3U;

        /* convert request packet into USB raw packet*/
        usbh_ptp_request_send (puhost, &ptp_container);

        /* setup state machine and start transfer */
        mtp->ptp.state = PTP_OP_REQUEST_STATE;
        mtp->ptp.req_state = PTP_REQ_WAIT;
        status = USBH_BUSY;
        break;

    case PTP_REQ_WAIT:
        status = usbh_ptp_process(puhost);

        if (status == USBH_OK) {
            *numobs = mtp->ptp.resp_container.param1;
        }
        break;

    default:
        break;
    }

    return status;
}

/*!
    \brief      gets storage index and fills stor_ids structure
    \param[in]  pudev: pointer to usb core instance
    \param[in]  storage_id: storage ID
    \param[in]  objectformatcod:
    \param[in]  assocationOH:
    \param[in]  object_handles:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_objects_handles_get (usbh_host *puhost,
                                          uint32_t storage_id,
                                          uint32_t object_format_code,
                                          uint32_t associationOH,
                                          ptp_object_handle* object_handles)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_OBJECT_HANDLES;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id++;
            ptp_container.Param1 = storage_id;
            ptp_container.Param2 = object_format_code;
            ptp_container.Param3 = associationOH;
            ptp_container.Nparam = 3U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                object_handles->n = ptp_array32_get (object_handles->Handler, mtp->ptp.data_container.payload.data, 0U);
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets object information
    \param[in]  pudev: pointer to usb core instance
    \param[in]  handle:
    \param[in]  object_info:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_object_info_get (usbh_host *puhost, uint32_t handle, ptp_object_info* object_info)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container  ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_OBJECT_INFO;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Param1 = handle;
            ptp_container.Nparam = 1U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
               ptp_object_info_get (puhost, object_info);
            }
            break;

            default:
            break;
    }

    return status;
}

/*!
    \brief      delete an object
    \param[in]  pudev: pointer to usb core instance
    \param[in]  handle: object handle
    \param[in]  objectformatecode:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_object_delete (usbh_host *puhost, uint32_t handle, uint32_t object_format_code)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container  ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_NODATA;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_DELETE_OBJECT;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Param1 = handle;
            ptp_container.Param2 = object_format_code;
            ptp_container.Nparam = 2U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets object
    \param[in]  pudev: pointer to usb core instance
    \param[in]  handle: 
    \param[in]  object:
    \param[out] none
    \retval     none
*/
usbh_status usbh_ptp_object_get (usbh_host *puhost, uint32_t handle, uint8_t *object)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* set object control params */
            mtp->ptp.object_ptr = object;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_OBJECT;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id++;
            ptp_container.Param1 = handle;
            ptp_container.Nparam = 1U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* Setup State machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                /* first packet is in the PTP data payload buffer */
                if (mtp->ptp.iteration == 0U) {
                    /* copy it to object */
                    memcpy(mtp->ptp.object_ptr, mtp->ptp.data_container.payload.data, PTP_USB_BULK_PAYLOAD_LEN_READ);
                }
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets object partially
    \param[in]  pudev: pointer to usb core instance
    \param[in]  handle:
    \param[in]  offset:
    \param[in]  maxbytes:
    \param[in]  object:
    \param[in]  len:
    \param[out] none
    \retval     none
*/
usbh_status usbh_ptp_partial_object_get(usbh_host *puhost,
                                        uint32_t handle,
                                        uint32_t offset,
                                        uint32_t maxbytes,
                                        uint8_t *object,
                                        uint32_t *len)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* set object control params */
            mtp->ptp.object_ptr = object;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_PARTIAL_OBJECT;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id++;
            ptp_container.Param1 = handle;
            ptp_container.Param2 = offset;
            ptp_container.Param3 = maxbytes;
            ptp_container.Nparam = 3U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                *len = mtp->ptp.resp_container.param1;

                /* first packet is in the PTP data payload buffer */
                if (mtp->ptp.iteration == 0U) {
                    /* copy it to object */
                    memcpy(mtp->ptp.object_ptr, mtp->ptp.data_container.payload.data, *len);
                }
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets supported object property
    \param[in]  pudev: pointer to usb core instance
    \param[in]  ofc: 
    \param[in]  propnum:
    \param[in]  props:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_object_props_supported_get (usbh_host *puhost,
                                                 uint16_t ofc,
                                                 uint32_t *propnum,
                                                 uint16_t *props)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_OBJECT_PROPS_SUPPORTED;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Param1 = ofc;
            ptp_container.Nparam = 1U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                *propnum = ptp_array16_get (props, mtp->ptp.data_container.payload.data, 0U);
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets object property descriptor
    \param[in]  pudev: pointer to usb core instance
    \param[in]  opc:
    \param[in]  ofc:
    \param[in]  opd:
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_object_prop_desc_get (usbh_host *puhost,
                                           uint16_t opc,
                                           uint16_t ofc,
                                           ptp_object_prop_desc *opd)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_OBJECT_PROP_DESC;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Param1 = opc;
            ptp_container.Param2 = ofc;
            ptp_container.Nparam = 2U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                ptp_object_prop_get(puhost, opd, mtp->ptp.data_length);
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      gets object prop list
    \param[in]  pudev: pointer to usb core instance
    \param[in]  handle: 
    \param[in]  pprops: 
    \param[in]  nrofprops: 
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_object_prop_list_get (usbh_host *puhost,
                                           uint32_t handle,
                                           mtp_properties *pprops,
                                           uint32_t *nrofprops)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_GETDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_length = 0U;
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;

            /* copy first packet of the object into data container */
            memcpy(mtp->ptp.data_container.payload.data, mtp->ptp.object_ptr, PTP_USB_BULK_PAYLOAD_LEN_READ);

            /* fill operation request params */
            ptp_container.Code = PTP_OC_GET_OBJ_PROPLIST;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Param1 = handle;
            ptp_container.Param2 = 0x00000000U;  /* 0x00000000U should be "all formats" */
            ptp_container.Param3 = 0xFFFFFFFFU;  /* 0xFFFFFFFFU should be "all properties" */
            ptp_container.Param4 = 0x00000000U;
            ptp_container.Param5 = 0xFFFFFFFFU;  /* return full tree below the param1 handle */
            ptp_container.Nparam = 5U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);

            if (status == USBH_OK) {
                ptp_object_prop_list_get (puhost, pprops, mtp->ptp.data_length);
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      send objects
    \param[in]  pudev: pointer to usb core instance
    \param[in]  handle:
    \param[in]  object: 
    \param[in]  size: 
    \param[out] none
    \retval     usbh_status
*/
usbh_status usbh_ptp_object_send (usbh_host *puhost,
                                  uint32_t handle,
                                  uint8_t *object,
                                  uint32_t size)
{
    usbh_status status = USBH_BUSY;
    usbh_mtp_handle *mtp = (usbh_mtp_handle *)puhost->active_class->class_data;
    ptp_container  ptp_container;

    switch (mtp->ptp.req_state) {
        case PTP_REQ_SEND:
            /* set operation request type */
            mtp->ptp.flags = PTP_DP_SENDDATA;
            mtp->ptp.data_ptr = (uint8_t *)(void *)&(mtp->ptp.data_container);
            mtp->ptp.data_packet_counter = 0U;
            mtp->ptp.data_packet = 0U;
            mtp->ptp.iteration = 0U;

            /* set object control params */
            mtp->ptp.object_ptr = object;
            mtp->ptp.data_length = size;

            /* fill operation request params */
            ptp_container.Code = PTP_OC_SEND_OBJECT;
            ptp_container.SessionID = mtp->ptp.session_id;
            ptp_container.Transaction_ID = mtp->ptp.transaction_id ++;
            ptp_container.Nparam = 0U;

            /* convert request packet into USB raw packet*/
            usbh_ptp_request_send (puhost, &ptp_container);

            /* setup state machine and start transfer */
            mtp->ptp.state = PTP_OP_REQUEST_STATE;
            mtp->ptp.req_state = PTP_REQ_WAIT;
            status = USBH_BUSY;
            break;

        case PTP_REQ_WAIT:
            status = usbh_ptp_process(puhost);
            break;

        default:
            break;
    }

    return status;
}

