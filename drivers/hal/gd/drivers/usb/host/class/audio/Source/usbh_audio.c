/*!
    \file  usbh_audio.c
    \brief this file is the AC layer handlers for USB host AC class
    \note  This driver manages the Audio Class 1.0 following the "USB Device 
           Class Definition for Audio Devices V1.0 Mar 18, 98"

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

#include "usbh_audio.h"
#include "usbh_transc.h"
#include "usbh_pipe.h"
#include "usbh_enum.h"

#include <string.h>

static usbh_status usbh_audio_interface_init            (usbh_host *puhost);
static void usbh_audio_interface_deinit                 (usbh_host *puhost);
static usbh_status usbh_audio_process                   (usbh_host *puhost);
static usbh_status usbh_audio_sof_process               (usbh_host *puhost);
static usbh_status usbh_audio_class_request             (usbh_host *puhost);
static usbh_status usbh_audio_cs_request                (usbh_host *puhost, uint8_t feature, uint8_t channel);

static usbh_status usbh_audio_cs_request_handle         (usbh_host *puhost);

static usbh_status usbh_audio_streaming_in_find         (usbh_host *puhost);
static usbh_status usbh_audio_streaming_out_find        (usbh_host *puhost);
static usbh_status usbh_audio_hid_control_find          (usbh_host *puhost);
static usbh_status usbh_audio_cs_descriptors_parse      (usbh_host *puhost);

static usbh_status usbh_audio_headphone_path_build      (usbh_host *puhost);
static usbh_status usbh_audio_microphone_path_build     (usbh_host *puhost);

//int32_t usbh_audio_linked_unit_in_find                  (usb_core_driver *pudev, uint8_t unit_ID);
//int32_t usbh_audio_linked_unit_out_find                 (usb_core_driver *pudev, uint8_t unit_ID);

static usbh_status cs_descriptors_parse                 (audio_class_specific_desc *class_desc, uint8_t ac_subclass, uint8_t *pdesc);
static usbh_status usbh_audio_transmit                  (usbh_host *puhost);

static usbh_status usbh_ac_cur_set                      (usbh_host *puhost,
                                                         uint8_t subtype, 
                                                         uint8_t feature,
                                                         uint8_t control_selector,
                                                         uint8_t channel,
                                                         uint16_t length);

static usbh_status usbh_ac_cur_get                      (usbh_host *puhost,
                                                         uint8_t subtype, 
                                                         uint8_t feature,
                                                         uint8_t control_selector,
                                                         uint8_t channel,
                                                         uint16_t length);

static usbh_status usbh_ac_min_get                      (usbh_host *puhost,
                                                         uint8_t subtype, 
                                                         uint8_t feature,
                                                         uint8_t control_selector,
                                                         uint8_t channel,
                                                         uint16_t length);

static usbh_status usbh_ac_max_get                      (usbh_host *puhost,
                                                         uint8_t subtype, 
                                                         uint8_t feature,
                                                         uint8_t control_selector,
                                                         uint8_t channel,
                                                         uint16_t length);

static usbh_status usbh_ac_res_get                      (usbh_host *puhost,
                                                         uint8_t subtype, 
                                                         uint8_t feature,
                                                         uint8_t control_selector,
                                                         uint8_t channel,
                                                         uint16_t length);

static usbh_status usbh_audio_endpoint_controls_set     (usbh_host *puhost, uint8_t ep, uint8_t *buff);

static usbh_status audio_volume_set                     (usbh_host *puhost, uint8_t feature, uint8_t channel, uint16_t volume);

static usbh_status usbh_audio_stream_input              (usbh_host *puhost);
static usbh_status usbh_audio_stream_output             (usbh_host *puhost);
static usbh_status usbh_audio_control                   (usbh_host *puhost);
static usbh_status usbh_audio_control_attribute_set     (usbh_host *puhost, uint8_t attrib);
static int32_t usbh_audio_linked_unit_find              (usbh_host *puhost, uint8_t unit_ID);

usbh_class  usbh_audio = 
{
    AC_CLASS,
    usbh_audio_interface_init,
    usbh_audio_interface_deinit,
    usbh_audio_class_request,
    usbh_audio_process,
    usbh_audio_sof_process
};

/*!
    \brief      the function init the audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_interface_init (usbh_host *puhost)
{
    uint8_t  interface, index;
    uint16_t ep_size_out = 0;
    uint16_t ep_size_in = 0;

    usbh_status status = USBH_FAIL;
    usbh_status out_status, in_status;

    interface = usbh_interface_find (&puhost->dev_prop, AC_CLASS, USB_SUBCLASS_AUDIOCONTROL, 0x00);

    if(interface == 0xFF) {
        /* not valid interface */
        status = USBH_FAIL;
    } else {
        static usbh_audio_handler audio_handler;
        memset((void*)&audio_handler, 0U, sizeof(usbh_audio_handler));

        usbh_audio_handler *audio = &audio_handler;

        puhost->active_class->class_data = (void *)&audio_handler;

        /* 1st step:  find audio interfaces */
        in_status = usbh_audio_streaming_in_find(puhost);
        out_status = usbh_audio_streaming_out_find(puhost);

        if((out_status == USBH_FAIL) && (in_status == USBH_FAIL)) {
            status = USBH_FAIL;
        } else {
            /* 2nd step:  select audio streaming interfaces with largest endpoint size: default behavior*/ 
            for (index = 0; index < AUDIO_MAX_AUDIO_STD_INTERFACE; index ++) {
                if (audio->stream_out[index].valid == 1) {
                    if (ep_size_out < audio->stream_out[index].ep_size) {
                        ep_size_out = audio->stream_out[index].ep_size;
                        audio->headphone.interface = audio->stream_out[index].interface;
                        audio->headphone.alt_settings = audio->stream_out[index].alt_settings;
                        audio->headphone.ep = audio->stream_out[index].ep;
                        audio->headphone.ep_size = audio->stream_out[index].ep_size;
                        audio->headphone.poll = audio->stream_out[index].poll;
                        audio->headphone.supported = 1;
                    }
                }

                if (audio->stream_in[index].valid == 1) {
                    if (ep_size_in < audio->stream_in[index].ep_size) {
                        ep_size_in = audio->stream_in[index].ep_size;
                        audio->microphone.interface = audio->stream_in[index].interface;
                        audio->microphone.alt_settings = audio->stream_in[index].alt_settings;
                        audio->microphone.ep = audio->stream_in[index].ep;
                        audio->microphone.ep_size = audio->stream_in[index].ep_size;
                        audio->microphone.poll = audio->stream_out[index].poll;
                        audio->microphone.supported = 1;
                    }
                }
            }

            if (usbh_audio_hid_control_find(puhost) == USBH_OK) {
                audio->control.supported = 1;
            }

            /* 3rd step: find and parse audio interfaces */ 
            usbh_audio_cs_descriptors_parse(puhost);

            /* 4th step:  open the audio streaming pipes*/ 
            if (audio->headphone.supported == 1) {
                usbh_audio_headphone_path_build(puhost);

                audio->headphone.pipe  = usbh_pipe_allocate (puhost->data, audio->headphone.ep);

                /* open pipe for out endpoint */
                usbh_pipe_create  (puhost->data,
                                   &puhost->dev_prop,
                                   audio->headphone.pipe,
                                   USB_EPTYPE_ISOC,
                                   audio->headphone.ep_size); 

                usbh_pipe_toggle_set (puhost->data, audio->headphone.pipe, 0);
            }

            if (audio->microphone.supported == 1) {
                usbh_audio_microphone_path_build(puhost);
                audio->microphone.pipe = usbh_pipe_allocate(puhost->data, audio->microphone.ep);

                /* open pipe for in endpoint */
                usbh_pipe_create(puhost->data,
                                 &puhost->dev_prop,
                                 audio->microphone.pipe,
                                 USB_EPTYPE_ISOC,
                                 audio->microphone.ep_size); 

                usbh_pipe_toggle_set (puhost->data, audio->microphone.pipe, 0);
            }

            if (audio->control.supported == 1) {
                audio->control.pipe = usbh_pipe_allocate(puhost->data, audio->control.ep);

                /* open pipe for in endpoint */
                usbh_pipe_create(puhost->data,
                                 &puhost->dev_prop,
                                 audio->control.pipe,
                                 USB_EPTYPE_INTR,
                                 audio->control.ep_size); 

                usbh_pipe_toggle_set(puhost->data, audio->control.pipe, 0);
            }

            audio->req_state = AUDIO_REQ_INIT;
            audio->control_state = AUDIO_CONTROL_INIT;

            status = USBH_OK;
        }
    }

    return status;
}

/*!
    \brief      the function deinit the pipes used for the audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     none
*/
void usbh_audio_interface_deinit (usbh_host *puhost)
{
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    if (audio->microphone.pipe != 0x00) {
        usb_pipe_halt (puhost->data, audio->microphone.pipe);
        usbh_pipe_free (puhost->data, audio->microphone.pipe);
        audio->microphone.pipe = 0;     /* reset the pipe as free */
    }

    if (audio->headphone.pipe != 0x00) {
        usb_pipe_halt (puhost->data, audio->headphone.pipe);
        usbh_pipe_free (puhost->data, audio->headphone.pipe);
        audio->headphone.pipe = 0;     /* reset the pipe as free */
    }

    if (audio->control.pipe != 0x00) {
        usb_pipe_halt (puhost->data, audio->control.pipe);
        usbh_pipe_free (puhost->data, audio->control.pipe);
        audio->control.pipe = 0;     /* reset the pipe as free */
    }
}

/*!
    \brief      the function is responsible for handling standard requests for audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_class_request (usbh_host *puhost)
{
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;
    usbh_status status = USBH_BUSY;
    usbh_status req_status = USBH_BUSY;

    /* switch audio req state machine */
    switch (audio->req_state) {
        case AUDIO_REQ_INIT:
        case AUDIO_REQ_SET_DEFAULT_IN_INTERFACE:
            if (audio->microphone.supported == 1) {
                req_status = usbh_setinterface(puhost, audio->microphone.interface, 0);
      
                if(req_status == USBH_OK) {
                    audio->req_state = AUDIO_REQ_SET_DEFAULT_OUT_INTERFACE;
                }
            } else {
                audio->req_state = AUDIO_REQ_SET_DEFAULT_OUT_INTERFACE;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif
            }
            break;

        case AUDIO_REQ_SET_DEFAULT_OUT_INTERFACE:
            if (audio->headphone.supported == 1) {
                req_status = usbh_setinterface(puhost, audio->headphone.interface, 0);

                if (req_status == USBH_OK) {
                    audio->req_state = AUDIO_REQ_CS_REQUESTS;
                    audio->cs_req_state = AUDIO_REQ_GET_VOLUME;

                    audio->temp_feature  = audio->headphone.asociated_feature;
                    audio->temp_channels = audio->headphone.asociated_channels;
                }
            } else {
                audio->req_state = AUDIO_REQ_CS_REQUESTS;
                audio->cs_req_state = AUDIO_REQ_GET_VOLUME;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif
            }
            break;

        case AUDIO_REQ_CS_REQUESTS:
            if (usbh_audio_cs_request_handle (puhost) == USBH_OK) {
                audio->req_state = AUDIO_REQ_SET_IN_INTERFACE;
            }
            break;

        case AUDIO_REQ_SET_IN_INTERFACE:
            if (audio->microphone.supported == 1) {
                req_status = usbh_setinterface (puhost,
                                                audio->microphone.interface, 
                                                audio->microphone.alt_settings);

                if (req_status == USBH_OK) {
                    audio->req_state = AUDIO_REQ_SET_OUT_INTERFACE;
                }
            } else {
                audio->req_state = AUDIO_REQ_SET_OUT_INTERFACE;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif
            }
            break;

        case AUDIO_REQ_SET_OUT_INTERFACE:
            if (audio->headphone.supported == 1) {
                req_status = usbh_setinterface (puhost,
                                                audio->headphone.interface, 
                                                audio->headphone.alt_settings);

                if (req_status == USBH_OK) {
                    audio->req_state = AUDIO_REQ_IDLE;
                }
            } else {
                audio->req_state = AUDIO_REQ_IDLE;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif
            }
            break;

        case AUDIO_REQ_IDLE:
            audio->play_state = AUDIO_PLAYBACK_INIT;

            status = USBH_OK;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_CLASS_EVENT, 0);
#endif

        default:
            break;
    }

    return status; 
}

/*!
    \brief      the function is responsible for handling AC Specific requests for a specific feature and channel for Audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  feature: CS feature
    \param[in]  channel: audio channel
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_cs_request (usbh_host *puhost, uint8_t feature, uint8_t channel)
{
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;
    usbh_status status = USBH_BUSY;
    usbh_status req_status = USBH_BUSY;

    /* switch audio req state machine */
    switch (audio->cs_req_state) {
        case AUDIO_REQ_GET_VOLUME:
            req_status = usbh_ac_cur_get(puhost,
                                         UAC_FEATURE_UNIT,     /* subtype  */
                                         feature,              /* feature  */
                                         CONTROL_VOLUME,       /* Selector */
                                         channel,              /* channel  */
                                         0x02);                /* length   */

            if (req_status != USBH_BUSY) {
                audio->cs_req_state = AUDIO_REQ_GET_MIN_VOLUME;
                audio->headphone.attribute.volume = LE16(&(audio->mem[0]));
            }
            break;

        case AUDIO_REQ_GET_MIN_VOLUME:
            req_status = usbh_ac_min_get(puhost,
                                         UAC_FEATURE_UNIT,     /* subtype  */
                                         feature,              /* feature  */
                                         CONTROL_VOLUME,       /* Selector */
                                         channel,              /* channel  */
                                         0x02);                /* length   */

            if(req_status != USBH_BUSY) {
                audio->cs_req_state = AUDIO_REQ_GET_MAX_VOLUME;
                audio->headphone.attribute.volumeMin = LE16(&audio->mem[0]);
            }
            break;

        case AUDIO_REQ_GET_MAX_VOLUME:
            req_status = usbh_ac_max_get(puhost,
                                         UAC_FEATURE_UNIT,     /* subtype  */
                                         feature,              /* feature  */
                                         CONTROL_VOLUME,       /* Selector */
                                         channel,              /* channel  */
                                         0x02);                /* length   */

            if (req_status != USBH_BUSY) {
                audio->cs_req_state = AUDIO_REQ_GET_RESOLUTION;
                audio->headphone.attribute.volumeMax = LE16(&audio->mem[0]);

                if (audio->headphone.attribute.volumeMax < audio->headphone.attribute.volumeMin) {
                    audio->headphone.attribute.volumeMax = 0xFF00;
                }
            }
            break;

        case AUDIO_REQ_GET_RESOLUTION:
            req_status = usbh_ac_res_get(puhost,
                                         UAC_FEATURE_UNIT,     /* subtype  */
                                         feature,              /* feature  */
                                         CONTROL_VOLUME,       /* Selector */
                                         channel,              /* channel  */
                                         0x02);                /* length   */

            if (req_status != USBH_BUSY) {
                audio->cs_req_state = AUDIO_REQ_CS_IDLE;
                audio->headphone.attribute.resolution = LE16(&audio->mem[0]);
            }
            break;

        case AUDIO_REQ_CS_IDLE:
            status = USBH_OK;

        default:
            break;
    }

    return status; 
}

/*!
    \brief      the function is responsible for handling AC Specific requests for a all features and associated channels for Audio class
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_cs_request_handle (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_status cs_status = USBH_BUSY;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    cs_status = usbh_audio_cs_request (puhost, audio->temp_feature, audio->temp_channels);

    if (cs_status != USBH_BUSY) {
        if (audio->temp_channels == 1) {
            audio->temp_feature = audio->headphone.asociated_feature;
            audio->temp_channels = 0;
            status = USBH_OK; 
        } else {
            audio->temp_channels--;
        }

        audio->cs_req_state = AUDIO_REQ_GET_VOLUME;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif     
    }

    return status;
}

/*!
    \brief      the function is for managing state machine for Audio data transfers
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_process(usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    if (audio->headphone.supported == 1) {
        usbh_audio_stream_output (puhost);
    }

    if (audio->microphone.supported == 1) {
        usbh_audio_stream_input (puhost);
    }

    return status;
}

/*!
    \brief      the function is for managing the SOF callback
    \param[in]  pudev: USB core driver
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_sof_process (usbh_host *puhost)
{
    return USBH_OK;
}

/*!
    \brief      find IN Audio Streaming interfaces
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_streaming_in_find (usbh_host *puhost)
{
    uint8_t interface, alt_settings = 0U;
    usbh_status status = USBH_FAIL;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    /* Look For AUDIOSTREAMING IN interface */
    for (interface = 0; interface < USBH_MAX_INTERFACES_NUM; interface++, alt_settings = 0U) {
        do {
            usb_desc_itf *itf_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][alt_settings].itf_desc;

            if ((itf_desc->bInterfaceClass == AC_CLASS) && (itf_desc->bInterfaceSubClass == USB_SUBCLASS_AUDIOSTREAMING)) {
                usb_desc_ep *ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][alt_settings].ep_desc[0];

                if ((ep_desc->bEndpointAddress & 0x80) && (ep_desc->wMaxPacketSize > 0)) {
                    audio->stream_in[alt_settings].ep = ep_desc->bEndpointAddress;
                    audio->stream_in[alt_settings].ep_size = ep_desc->wMaxPacketSize;
                    audio->stream_in[alt_settings].interface = itf_desc->bInterfaceNumber;
                    audio->stream_in[alt_settings].alt_settings = itf_desc->bAlternateSetting;
                    audio->stream_in[alt_settings].poll = ep_desc->bInterval;
                    audio->stream_in[alt_settings].valid = 1;

                    if (status != USBH_OK) {
                        status = USBH_OK;
                    }
                }

                alt_settings++;
            } else {
                break;
            }
        } while (alt_settings < USBH_MAX_ALT_SETTING);
    }

    return status;
}

/*!
    \brief      find OUT Audio streaming interfaces
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_streaming_out_find (usbh_host *puhost)
{
    uint8_t interface, alt_settings = 0U;
    usbh_status status = USBH_FAIL ;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    /* look for audio streaming in interface */
    for (interface = 0; interface < USBH_MAX_INTERFACES_NUM; interface++, alt_settings = 0U) {
        do {
            usb_desc_itf *itf_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][alt_settings].itf_desc;

            if ((itf_desc->bInterfaceClass == AC_CLASS) && (itf_desc->bInterfaceSubClass == USB_SUBCLASS_AUDIOSTREAMING)) {
                usb_desc_ep *ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][alt_settings].ep_desc[0];

                if (((ep_desc->bEndpointAddress & 0x80) == 0x00) && (ep_desc->wMaxPacketSize > 0)) {
                    audio->stream_out[alt_settings].ep = ep_desc->bEndpointAddress;
                    audio->stream_out[alt_settings].ep_size = ep_desc->wMaxPacketSize;
                    audio->stream_out[alt_settings].interface = itf_desc->bInterfaceNumber;
                    audio->stream_out[alt_settings].alt_settings = itf_desc->bAlternateSetting;
                    audio->stream_out[alt_settings].poll = ep_desc->bInterval;
                    audio->stream_out[alt_settings].valid = 1;

                    if (status != USBH_OK) {
                        status = USBH_OK;
                    }
                }

                alt_settings++;
            } else {
                break;
            }
        } while (alt_settings < USBH_MAX_ALT_SETTING);
    }

    return status;  
}

/*!
    \brief      find HID Control interfaces
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_hid_control_find (usbh_host *puhost)
{
    uint8_t interface;
    usbh_status status = USBH_FAIL;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    /* look for audio control interface */
    interface = usbh_interface_find(&puhost->dev_prop, AC_CLASS, USB_SUBCLASS_AUDIOCONTROL, 0xFF);

    if (interface != 0xFF) {
        for (interface = 0; interface < USBH_MAX_INTERFACES_NUM; interface++) {
            usb_desc_itf *itf_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].itf_desc;

            if (itf_desc->bInterfaceClass == 0x03) {
                usb_desc_ep *ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[0];

                if ((ep_desc->wMaxPacketSize > 0) && (ep_desc->bEndpointAddress & 0x80) == 0x80) {
                  audio->control.ep = ep_desc->bEndpointAddress;
                  audio->control.ep_size = ep_desc->wMaxPacketSize;
                  audio->control.interface = itf_desc->bInterfaceNumber;
                  audio->control.poll = ep_desc->bInterval;
                  audio->control.supported = 1;
                  status = USBH_OK;
                  break;
                }
            }
        }
    }

    return status;
}

/*!
    \brief      parse AC and interfaces descriptors
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_cs_descriptors_parse (usbh_host *puhost)
{
    usb_desc_header *pdesc;
    uint16_t ptr;
    int8_t itf_index = 0;
    int8_t itf_number = 0;
    int8_t alt_setting;

    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;
    pdesc = (usb_desc_header *)&(puhost->dev_prop.cfgdesc_rawdata);
    ptr = USB_CFG_DESC_LEN;

    audio->class_desc.feature_unit_num = 0;
    audio->class_desc.input_terminal_num = 0;
    audio->class_desc.output_terminal_num = 0;
    audio->class_desc.as_num = 0;

    while(ptr < puhost->dev_prop.cfg_desc_set.cfg_desc.wTotalLength) {
        pdesc = usbh_nextdesc_get((uint8_t*) pdesc, &ptr);

        switch (pdesc->bDescriptorType) {
            case USB_DESCTYPE_ITF:
                itf_number = *((uint8_t *)pdesc + 2);
                alt_setting = *((uint8_t *)pdesc + 3);
                itf_index = usbh_interfaceindex_find (&puhost->dev_prop, itf_number, alt_setting);
                break;

            case USB_DESC_TYPE_CS_INTERFACE:
                if (itf_number <= puhost->dev_prop.cfg_desc_set.cfg_desc.bNumInterfaces) {
                    cs_descriptors_parse(&audio->class_desc,
                                         puhost->dev_prop.cfg_desc_set.itf_desc_set[itf_index][0].itf_desc.bInterfaceSubClass, 
                                         (uint8_t *)pdesc);
                }
                break;

            default:
                break; 
        }
    }

    return USBH_OK;
}

/*!
    \brief      parse AC interfaces
    \param[in]  class_desc: device class descriptor
    \param[in]  ac_subclass: AC sub class
    \param[in]  pdesc: pointer to descriptor
    \param[out] none
    \retval     none
*/
static usbh_status cs_descriptors_parse(audio_class_specific_desc *class_desc, uint8_t ac_subclass, uint8_t *pdesc)
{
    if (ac_subclass == USB_SUBCLASS_AUDIOCONTROL) {
        switch (pdesc[2]) {
            case UAC_HEADER: 
                class_desc->cs_desc.headerd_esc = (audio_header_desc *)pdesc;
                break;

            case UAC_INPUT_TERMINAL:
                class_desc->cs_desc.input_terminal_desc[class_desc->input_terminal_num++] = (audio_it_desc*) pdesc;
                break;

            case UAC_OUTPUT_TERMINAL:
                class_desc->cs_desc.output_terminal_desc[class_desc->output_terminal_num++] = (audio_ot_desc*) pdesc;
                break;

            case UAC_FEATURE_UNIT:
                class_desc->cs_desc.feature_unit_desc[class_desc->feature_unit_num++] = (audio_feature_desc*) pdesc; 
                break;

            case UAC_SELECTOR_UNIT:
                class_desc->cs_desc.selector_unit_desc[class_desc->selector_unit_num++] = (audio_selector_desc*) pdesc;
                break;

            case UAC_MIXER_UNIT:
                class_desc->cs_desc.mixer_unit_desc[class_desc->mixer_unit_num++] = (audio_mixer_desc*) pdesc; 
                break;

            default: 
                break;
        }
    } else if (ac_subclass == USB_SUBCLASS_AUDIOSTREAMING) {
        switch(pdesc[2]) {
            case UAC_AS_GENERAL:
                class_desc->as_desc[class_desc->as_num].GeneralDesc = (audio_as_general_desc*) pdesc;
                break;

            case UAC_FORMAT_TYPE:
                class_desc->as_desc[class_desc->as_num++].FormatTypeDesc = (audio_as_format_type_desc*) pdesc;
                break;

            default:
                break;
        }
    }

    return USBH_OK;
}

/*!
    \brief      link a Unit to next associated one
    \param[in]  pudev: USB core driver
    \param[in]  unit_ID: Unit identifer
    \param[out] none
    \retval     unit_ID, index and Type of the associated Unit 
*/
static int32_t usbh_audio_linked_unit_find (usbh_host *puhost, uint8_t unit_ID)
{
    uint8_t index;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    /* find feature unit */
    for (index = 0; index < audio->class_desc.feature_unit_num; index ++) {
        if (audio->class_desc.cs_desc.feature_unit_desc[index]->bSourceID == unit_ID) {
            unit_ID = audio->class_desc.cs_desc.feature_unit_desc[index]->bUnitID;

            return ((unit_ID << 16) | (UAC_FEATURE_UNIT << 8) | index);
        }
    }

    /* find mixer unit */
    for (index = 0; index < audio->class_desc.mixer_unit_num; index ++) {
        if ((audio->class_desc.cs_desc.mixer_unit_desc[index]->bSourceID0 == unit_ID) ||
             (audio->class_desc.cs_desc.mixer_unit_desc[index]->bSourceID1 == unit_ID)) {
            unit_ID = audio->class_desc.cs_desc.mixer_unit_desc[index]->bUnitID;

            return ((unit_ID << 16) | (UAC_MIXER_UNIT << 8) | index);
        }
    }

    /* find selector unit */
    for (index = 0; index < audio->class_desc.selector_unit_num; index ++) {
        if (audio->class_desc.cs_desc.selector_unit_desc[index]->bSourceID0 == unit_ID) {
            unit_ID = audio->class_desc.cs_desc.selector_unit_desc[index]->bUnitID;

            return ((unit_ID << 16) | (UAC_SELECTOR_UNIT << 8) | index); 
        }
    }

    /* find output terminal unit */  
    for (index = 0; index < audio->class_desc.output_terminal_num; index ++) {
        if (audio->class_desc.cs_desc.output_terminal_desc[index]->bSourceID == unit_ID) {
            unit_ID = audio->class_desc.cs_desc.output_terminal_desc[index]->bTerminalID;

            return ((unit_ID << 16) | (UAC_OUTPUT_TERMINAL << 8) | index);
        }
    }

    /* no associated unit found */
    return -1;
}

/*!
    \brief      build full path for microphone device
    \param[in]  pudev: USB core driver
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_microphone_path_build (usbh_host *puhost)
{
    uint8_t unit_ID = 0, type, index;
    uint32_t value;
    uint8_t terminal_index;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    /* find microphone input terminal */
    for (terminal_index = 0; terminal_index < audio->class_desc.input_terminal_num; terminal_index++) {
        if (LE16(audio->class_desc.cs_desc.input_terminal_desc[terminal_index]->wTerminalType) == 0x201) {
            unit_ID = audio->class_desc.cs_desc.input_terminal_desc[terminal_index]->bTerminalID;
            audio->microphone.asociated_channels = audio->class_desc.cs_desc.input_terminal_desc[terminal_index]->bNrChannels;
            break;
        }
    }

    do
    {
        value =  usbh_audio_linked_unit_find(puhost, unit_ID);
        index = value & 0xFF;
        type = (value >> 8) & 0xFF;
        unit_ID = (value >> 16) & 0xFF;

        switch (type) {
            case UAC_FEATURE_UNIT:
                audio->microphone.asociated_feature = index;
                break;

            case UAC_MIXER_UNIT:
                audio->microphone.asociated_mixer = index;
                break;

            case UAC_SELECTOR_UNIT:
                audio->microphone.asociated_selector = index;
                break;

            case UAC_OUTPUT_TERMINAL:
                audio->microphone.asociated_terminal = index;
                break;
        }
    } while ((type != UAC_OUTPUT_TERMINAL) && (value > 0));

    return USBH_OK;
}

/*!
    \brief      build full path for Headphone device
    \param[in]  pudev: USB core driver
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_headphone_path_build(usbh_host *puhost)
{
    uint8_t unit_ID = 0, type, index;
    uint32_t value;
    uint8_t terminal_index;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    /* find association between audio streaming and microphone */
    for (terminal_index = 0; terminal_index < audio->class_desc.input_terminal_num; terminal_index++) {
        if (LE16(audio->class_desc.cs_desc.input_terminal_desc[terminal_index]->wTerminalType) == 0x101) {
            unit_ID = audio->class_desc.cs_desc.input_terminal_desc[terminal_index]->bTerminalID;
            audio->headphone.asociated_channels =  audio->class_desc.cs_desc.input_terminal_desc[terminal_index]->bNrChannels;
            break;
        }
    }

    for (index = 0; index < audio->class_desc.as_num; index++) {
        if (audio->class_desc.as_desc[index].GeneralDesc->bTerminalLink == unit_ID) {
            audio->headphone.asociated_as = index;
            break;
        }
    }

    do {
        value =  usbh_audio_linked_unit_find(puhost, unit_ID);
        index = value & 0xFF;
        type = (value >> 8) & 0xFF;
        unit_ID = (value >> 16) & 0xFF;

        switch (type) {
            case UAC_FEATURE_UNIT:
                audio->headphone.asociated_feature = index;
                break;

            case UAC_MIXER_UNIT:
                audio->headphone.asociated_mixer = index;
                break;

            case UAC_SELECTOR_UNIT:
                audio->headphone.asociated_selector = index;
                break;

            case UAC_OUTPUT_TERMINAL:
                audio->headphone.asociated_terminal = index;
                if (LE16(audio->class_desc.cs_desc.output_terminal_desc[index]->wTerminalType) != 0x103) {
                    return  USBH_OK;
                }
                break;
        }
    } while ((type != UAC_OUTPUT_TERMINAL) && (value > 0));

    return USBH_FAIL;
}

/*!
    \brief      handle Set Cur request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  feature: feature index
    \param[in]  control_selector: control code
    \param[in]  channel: channel index
    \param[in]  length: Command length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_ac_cur_set (usbh_host *puhost,
                                    uint8_t subtype, 
                                    uint8_t feature,
                                    uint8_t control_selector,
                                    uint8_t channel,
                                    uint16_t length)
{
    uint16_t wValue, wIndex, wLength;
    uint8_t unit_ID, interface_num;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    switch(subtype) {
        case UAC_INPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.input_terminal_desc[0]->bTerminalID;
            interface_num = 0;
            wIndex = ( unit_ID << 8 ) | interface_num ;
            wValue = (COPY_PROTECT_CONTROL << 8 ) ;
            audio->mem[0] = 0x00;

            wLength = 1;
            break;

        case UAC_FEATURE_UNIT:
            unit_ID = audio->class_desc.cs_desc.feature_unit_desc[feature]->bUnitID;
            interface_num = 0;
            wIndex = ( unit_ID << 8 ) | interface_num ;
            /* holds the CS(control selector) and CN (channel number) */
            wValue =  (control_selector << 8) | channel;
            wLength = length;
            break;
    }

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_OUT | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UAC_SET_CUR,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(audio->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Cur request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  feature: feature index
    \param[in]  control_selector: control code
    \param[in]  channel: channel index
    \param[in]  length: Command length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_ac_cur_get (usbh_host *puhost,
                                    uint8_t subtype, 
                                    uint8_t feature,
                                    uint8_t control_selector,
                                    uint8_t channel,
                                    uint16_t length)
{
    uint16_t wValue = 0, wIndex = 0,wLength = 0;
    uint8_t unit_ID = 0, interface_num = 0;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    switch(subtype) {
        case UAC_INPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.input_terminal_desc[0]->bTerminalID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            wValue = (COPY_PROTECT_CONTROL << 8);
            audio->mem[0] = 0x00;

            wLength = 1;
            break;

        case UAC_FEATURE_UNIT:
            unit_ID = audio->class_desc.cs_desc.feature_unit_desc[feature]->bUnitID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            /* holds the CS(control selector ) and CN (channel number) */
            wValue =  (control_selector << 8) | channel;
            wLength = length;
            break;

        case UAC_OUTPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.output_terminal_desc[0]->bTerminalID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            wValue = (COPY_PROTECT_CONTROL << 8);
            wLength = 1; 
            break;
    }

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UAC_GET_CUR,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(audio->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Max request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  feature: feature index
    \param[in]  control_selector: control code
    \param[in]  channel: channel index
    \param[in]  length: Command length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_ac_max_get (usbh_host *puhost,
                                    uint8_t subtype, 
                                    uint8_t feature,
                                    uint8_t control_selector,
                                    uint8_t channel,
                                    uint16_t length)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t unit_ID = 0, interface_num = 0;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    switch(subtype) {
        case UAC_INPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.input_terminal_desc[0]->bTerminalID;
            interface_num =0;
            wIndex = (unit_ID << 8) | interface_num;
            wValue = (COPY_PROTECT_CONTROL << 8);
            audio->mem[0] = 0x00;

            wLength = 1;
            break;

        case UAC_FEATURE_UNIT:
            unit_ID = audio->class_desc.cs_desc.feature_unit_desc[feature]->bUnitID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            /* holds the CS(control selector ) and CN (channel number) */
            wValue = (control_selector << 8) | channel;
            wLength = length;
            break;

        case UAC_OUTPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.output_terminal_desc[0]->bTerminalID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            wValue = (COPY_PROTECT_CONTROL << 8);
            wLength = 1;
            break;
    }

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UAC_GET_MAX,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(audio->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
  
}

/*!
    \brief      handle Get Res request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  feature: feature index
    \param[in]  control_selector: control code
    \param[in]  channel: channel index
    \param[in]  length: Command length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_ac_res_get (usbh_host *puhost, 
                                    uint8_t subtype, 
                                    uint8_t feature,
                                    uint8_t control_selector,
                                    uint8_t channel,
                                    uint16_t length)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t unit_ID = 0, interface_num = 0;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    switch(subtype)
    {
        case UAC_INPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.input_terminal_desc[0]->bTerminalID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num ;
            wValue = (COPY_PROTECT_CONTROL << 8);
            audio->mem[0] = 0x00;

            wLength = 1;
            break;

        case UAC_FEATURE_UNIT:
            unit_ID = audio->class_desc.cs_desc.feature_unit_desc[feature]->bUnitID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num ;
            /* holds the CS(control selector ) and CN (channel number) */
            wValue =  (control_selector << 8) | channel;
            wLength = length;
            break;

        case UAC_OUTPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.output_terminal_desc[0]->bTerminalID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            wValue = (COPY_PROTECT_CONTROL << 8);
            wLength = 1; 
            break;

        default:
            break;
    }

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UAC_GET_RES,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(audio->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Get Min request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  subtype: subtype index
    \param[in]  feature: feature index
    \param[in]  control_selector: control code
    \param[in]  channel: channel index
    \param[in]  length: Command length
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_ac_min_get (usbh_host *puhost,
                                    uint8_t subtype, 
                                    uint8_t feature,
                                    uint8_t control_selector,
                                    uint8_t channel,
                                    uint16_t length)
{
    uint16_t wValue = 0, wIndex = 0, wLength = 0;
    uint8_t unit_ID = 0, interface_num = 0;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    switch(subtype) {
        case UAC_INPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.input_terminal_desc[0]->bTerminalID;
            interface_num = 0; /* always zero control interface */
            wIndex = (unit_ID << 8) | interface_num;
            wValue = (COPY_PROTECT_CONTROL << 8);
            audio->mem[0] = 0x00;

            wLength = 1;
            break;

        case UAC_FEATURE_UNIT:
            unit_ID = audio->class_desc.cs_desc.feature_unit_desc[feature]->bUnitID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            /* holds the CS(control selector ) and CN (channel number) */
            wValue = (control_selector << 8) | channel;
            wLength = length;
            break;

        case UAC_OUTPUT_TERMINAL:
            unit_ID = audio->class_desc.cs_desc.output_terminal_desc[0]->bTerminalID;
            interface_num = 0;
            wIndex = (unit_ID << 8) | interface_num;
            wValue = (COPY_PROTECT_CONTROL << 8);
            wLength = 1; 
            break;
    }

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_IN | USB_RECPTYPE_ITF | USB_REQTYPE_CLASS,
            .bRequest      = UAC_GET_MIN,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, (uint8_t *)(void *)(audio->mem), wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Set Endpoint Controls Request
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  ep: endpoint address
    \param[in]  buff: pointer to data
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_endpoint_controls_set(usbh_host *puhost, uint8_t ep, uint8_t *buff)
{
    uint16_t wValue, wIndex, wLength;

    wValue = SAMPLING_FREQ_CONTROL << 8;
    wIndex = ep; 
    wLength = 3; /*length of the frequency parameter*/

    if (puhost->control.ctl_state == CTL_IDLE) {
        puhost->control.setup.req = (usb_req) {
            .bmRequestType = USB_TRX_OUT | USB_RECPTYPE_EP | USB_REQTYPE_CLASS,
            .bRequest      = UAC_SET_CUR,
            .wValue        = wValue,
            .wIndex        = wIndex,
            .wLength       = wLength
        };

        usbh_ctlstate_config (puhost, buff, wLength);
    }

    return usbh_ctl_handler (puhost);
}

/*!
    \brief      handle Input stream process
    \param[in]  pudev: USB core driver
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_stream_input (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;

    return status;
}

/*!
    \brief      handle HID control process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_control (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;
    uint16_t attribute = 0;

    switch(audio->control_state) {
        case AUDIO_CONTROL_INIT:
            if((puhost->control.timer & 1) == 0) {
                audio->control.timer = puhost->control.timer;
                usbh_data_recev (puhost->data, 
                                 (uint8_t *)(audio->mem),
                                 audio->control.pipe,
                                 audio->control.ep_size);

                audio->temp_feature  = audio->headphone.asociated_feature;
                audio->temp_channels = audio->headphone.asociated_channels;

                audio->control_state = AUDIO_CONTROL_CHANGE;
            }
            break;

        case AUDIO_CONTROL_CHANGE:
            if (usbh_urbstate_get(puhost->data, audio->control.pipe) == URB_DONE) {
                attribute = LE16(&audio->mem[0]);
                if(usbh_audio_control_attribute_set (puhost, attribute) == USBH_BUSY) {
                    break;
                }
            }

            if ((puhost->control.timer - audio->control.timer) >= audio->control.poll) {
                audio->control.timer = puhost->control.timer;

                usbh_data_recev(puhost->data, 
                                (uint8_t *)(audio->mem),
                                audio->control.pipe,
                                audio->control.ep_size);
            }
            break;

        case AUDIO_CONTROL_VOLUME_UP:
            if (usbh_audio_control_attribute_set (puhost, 1) == USBH_OK) {
                audio->control_state = AUDIO_CONTROL_INIT;
                status = USBH_OK; 
            }
            break;

        case AUDIO_CONTROL_VOLUME_DOWN:
            if (usbh_audio_control_attribute_set (puhost, 2) == USBH_OK) {
                audio->control_state = AUDIO_CONTROL_INIT;
                status = USBH_OK; 
            }
            break;

        case AUDIO_CONTROL_IDLE:
        default:  
            break; 
    }

    return status;  
}

/*!
    \brief      handle output stream process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_stream_output (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY ;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;
    uint8_t *buff;

    switch (audio->play_state) {
        case AUDIO_PLAYBACK_INIT:
            if (audio->class_desc.as_desc[audio->headphone.asociated_as].FormatTypeDesc->bSamFreqType == 0) {
                audio->play_state = AUDIO_PLAYBACK_SET_EP_FREQ;        
            } else {
                audio->play_state = AUDIO_PLAYBACK_SET_EP;
            }
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_URB_EVENT, 0);
#endif
            break;

        case AUDIO_PLAYBACK_SET_EP_FREQ:
            buff = (uint8_t*)audio->class_desc.as_desc[audio->headphone.asociated_as].FormatTypeDesc->tSamFreq[0];

            status = usbh_audio_endpoint_controls_set(puhost, audio->headphone.ep, buff);
            if (status == USBH_OK) {
                audio->play_state = AUDIO_PLAYBACK_IDLE;    
            }
            break;

        case AUDIO_PLAYBACK_SET_EP:
            buff = (uint8_t *)&audio->headphone.frequency;
            status = usbh_audio_endpoint_controls_set(puhost, audio->headphone.ep, buff);

            if (status == USBH_OK) {
                audio->play_state = AUDIO_PLAYBACK_IDLE;
                usbh_audio_playback_set(puhost);
            }
            break;

        case AUDIO_PLAYBACK_IDLE:
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_CLASS_EVENT, 0);
#endif

            puhost->usr_cb->dev_user_app();

            status = USBH_OK;
            break;

        case AUDIO_PLAYBACK_PLAY:
            usbh_audio_transmit(puhost);
            status = USBH_OK;
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      handle Transmission process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_transmit (usbh_host *puhost)
{
    usbh_status status = USBH_BUSY ;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    switch (audio->processing_state) {
        case AUDIO_DATA_START_OUT:
            /* sync with start of even frame */
            if ((puhost->control.timer & 1) == 0) {
                audio->headphone.timer = puhost->control.timer;
                audio->processing_state = AUDIO_DATA_OUT;

                usbh_data_send (puhost->data, 
                                audio->headphone.buf,
                                audio->headphone.pipe,
                                audio->headphone.frame_length);

                audio->headphone.partial_ptr = audio->headphone.frame_length;
                audio->headphone.global_ptr = audio->headphone.frame_length;
                audio->headphone.cbuf = audio->headphone.buf;
            } else {
#if (USBH_USE_OS == 1)
      osDelay(1);
      osMessagePut ( pudev->os_event, USBH_CLASS_EVENT, 0);
#endif  
            }
            break;

        case AUDIO_DATA_OUT:
            if ((usbh_urbstate_get(puhost->data, audio->headphone.pipe) == URB_DONE) &&
                 ((puhost->control.timer - audio->headphone.timer) >= audio->headphone.poll)) {
                audio->headphone.timer = puhost->control.timer;

                if (audio->control.supported == 1) {
                    usbh_audio_control (puhost);
                }

                if (audio->headphone.global_ptr <= audio->headphone.total_length) {
                    usbh_data_send (puhost->data, 
                                    audio->headphone.cbuf,
                                    audio->headphone.pipe,
                                    audio->headphone.frame_length);

                    audio->headphone.cbuf += audio->headphone.frame_length;
                    audio->headphone.partial_ptr += audio->headphone.frame_length;
                    audio->headphone.global_ptr += audio->headphone.frame_length;
                } else {
                    audio->headphone.partial_ptr = 0xFFFFFFFF;
                    audio->play_state = AUDIO_PLAYBACK_IDLE;
                    usbh_audio_buffer_empty_callback(puhost);
                }
            }
            break;

        default:
            break;
    }

    return status;
}

/*!
    \brief      set audio sampling parameters
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  SampleRate: Sample Rate
    \param[in]  NbrChannels: Number of Channels
    \param[in]  BitPerSample: Bit Per Sample
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_frequency_set (usbh_host *puhost,
                                      uint16_t SampleRate,
                                      uint8_t  NbrChannels,
                                      uint8_t  BitPerSample)
{
    usbh_status status = USBH_BUSY;
    usbh_audio_handler *audio = NULL;
    uint8_t index;
    uint8_t change_freq = FALSE;
    uint32_t freq_min, freq_max;
    uint8_t num_supported_freq;

    if (puhost->cur_state == HOST_CLASS_HANDLER) {
        audio = (usbh_audio_handler *)puhost->active_class->class_data;

        if (audio->play_state == AUDIO_PLAYBACK_IDLE) {
            if (audio->class_desc.as_desc[audio->headphone.asociated_as].FormatTypeDesc->bSamFreqType == 0) {
                freq_min = LE24(audio->class_desc.as_desc[audio->headphone.asociated_as].FormatTypeDesc->tSamFreq[0]);
                freq_max = LE24(audio->class_desc.as_desc[audio->headphone.asociated_as].FormatTypeDesc->tSamFreq[1]);

                if ((SampleRate >= freq_min)&& (SampleRate <= freq_max)) {
                    change_freq = TRUE;
                }
            } else {
                num_supported_freq = (audio->class_desc.as_desc[audio->headphone.asociated_as].FormatTypeDesc->bLength - 8) / 3;

                for (index = 0; index < num_supported_freq; index++) {
                    if (SampleRate == LE24(audio->class_desc.as_desc[audio->headphone.asociated_as].FormatTypeDesc->tSamFreq[index])) {
                        change_freq = TRUE;
                        break;
                    }
                }
            }

            if (change_freq == TRUE) {
                audio->headphone.frequency = SampleRate;
                audio->headphone.frame_length = (SampleRate * BitPerSample * NbrChannels) / 8000;
                audio->play_state = AUDIO_PLAYBACK_SET_EP;
                status = USBH_OK;
            }
        }
    }

    return status;
}

/*!
    \brief      start playback process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  buf: pointer to raw audio data
    \param[in]  length: total length of the audio data
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_play (usbh_host *puhost, uint8_t *buf, uint32_t length)
{
    usbh_status status = USBH_FAIL;
    usbh_audio_handler *audio = NULL;

    if (puhost->cur_state == HOST_CLASS_HANDLER) {
        audio = (usbh_audio_handler *)puhost->active_class->class_data;

        if (audio->play_state == AUDIO_PLAYBACK_IDLE) {
            audio->headphone.buf = buf;
            audio->headphone.total_length = length;
            audio->play_state = AUDIO_PLAYBACK_PLAY;
            audio->control_state = AUDIO_CONTROL_INIT;
            audio->processing_state = AUDIO_DATA_START_OUT;
            status = USBH_OK;
#if (USBH_USE_OS == 1)
      osMessagePut ( pudev->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
    }

    return status;
}

/*!
    \brief      stop the playback process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_stop (usbh_host *puhost)
{
    usbh_status status = USBH_FAIL;

    status = usbh_audio_suspend(puhost);

    return status;
}

/*!
    \brief      suspend the playback process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     none
*/
usbh_status usbh_audio_suspend (usbh_host *puhost)
{
    usbh_status status = USBH_FAIL;
    usbh_audio_handler *audio = NULL;

    if (puhost->cur_state == HOST_CLASS_HANDLER) {
        audio = (usbh_audio_handler *)puhost->active_class->class_data;

        if (audio->play_state == AUDIO_PLAYBACK_PLAY) {
            audio->control_state = AUDIO_CONTROL_IDLE;
            audio->play_state = AUDIO_PLAYBACK_IDLE;
            status = USBH_OK;
        }
    }

    return status;
}

/*!
    \brief      resume the playback process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_resume (usbh_host *puhost)
{
    usbh_status status = USBH_FAIL;
    usbh_audio_handler *audio = NULL;

    if (puhost->cur_state == HOST_CLASS_HANDLER) {
        audio = (usbh_audio_handler *)puhost->active_class->class_data;

        if (audio->play_state == AUDIO_PLAYBACK_IDLE) {
            audio->control_state = AUDIO_CONTROL_INIT;
            audio->play_state = AUDIO_PLAYBACK_PLAY;
        }
    }

    return status;
}

/*!
    \brief      return the current buffer pointer for OUT process
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[out] none
    \retval     USB host status
*/
int32_t usbh_audio_out_offset_get (usbh_host *puhost)
{
    usbh_audio_handler *audio = NULL;

    if (puhost->cur_state == HOST_CLASS_HANDLER) {
        audio = (usbh_audio_handler *)puhost->active_class->class_data;

        if(audio->play_state == AUDIO_PLAYBACK_PLAY) {
            return audio->headphone.partial_ptr;
        }
    }

    return -1;
}

/*!
    \brief      change audio data buffer address
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  buf: buffer address
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_out_buffer_change (usbh_host *puhost, uint8_t *buf)
{
    usbh_status status = USBH_FAIL;
    usbh_audio_handler *audio = NULL;

    if (puhost->cur_state == HOST_CLASS_HANDLER) {
        audio = (usbh_audio_handler *)puhost->active_class->class_data;

        if (audio->play_state == AUDIO_PLAYBACK_PLAY) {
            if (audio->headphone.buf <= buf) {
                audio->headphone.cbuf = buf;

                if (audio->headphone.buf == buf) {
                    audio->headphone.partial_ptr = 0;
                }

                status = USBH_OK;  
            }
        }
    }

    return status;
}

/*!
    \brief      set control attribute
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  attrib: control attribute
    \param[out] none
    \retval     USB host status
*/
static usbh_status usbh_audio_control_attribute_set (usbh_host *puhost, uint8_t attrib)
{
    usbh_status status = USBH_BUSY ;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    switch (attrib) {
        case 0x01:
            audio->headphone.attribute.volume += audio->headphone.attribute.resolution;
            break;

        case 0x02:
            audio->headphone.attribute.volume -= audio->headphone.attribute.resolution; 
            break;

        default:
            break;
    }

    if (audio->headphone.attribute.volume > audio->headphone.attribute.volumeMax) {
        audio->headphone.attribute.volume =audio->headphone.attribute.volumeMax;
    }

    if (audio->headphone.attribute.volume < audio->headphone.attribute.volumeMin) {
        audio->headphone.attribute.volume =audio->headphone.attribute.volumeMin;
    }

    if (audio_volume_set (puhost, 
                          audio->temp_feature, 
                          audio->temp_channels, 
                          audio->headphone.attribute.volume) != USBH_BUSY) {
        if (audio->temp_channels == 1) {
            audio->temp_feature = audio->headphone.asociated_feature;
            audio->temp_channels = audio->headphone.asociated_channels;
            status = USBH_OK;
        } else {
            audio->temp_channels--;
        }

        audio->cs_req_state = AUDIO_REQ_GET_VOLUME;
    }

    return status;
}

/*!
    \brief      set volume
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  volume_ctl: VOLUME_UP / VOLUME_DOWN
    \param[out] none
    \retval     USB host status
*/
usbh_status usbh_audio_volume_set (usbh_host *puhost, audio_volume_ctrl volume_ctl)
{
    usbh_audio_handler *audio = NULL;

    if ((volume_ctl == VOLUME_UP) || (volume_ctl == VOLUME_DOWN)) {
        if (puhost->cur_state == HOST_CLASS_HANDLER) {
            audio = (usbh_audio_handler *)puhost->active_class->class_data;

            if (audio->play_state == AUDIO_PLAYBACK_PLAY) {
                audio->control_state = (volume_ctl == VOLUME_UP)? AUDIO_CONTROL_VOLUME_UP : AUDIO_CONTROL_VOLUME_DOWN;

                return USBH_OK;
            }
        }
    }

    return USBH_FAIL;
}

/*!
    \brief      set USB audio volume
    \param[in]  pudev: USB core driver
    \param[in]  puhost: USB host handler
    \param[in]  feature: feature Unit index
    \param[in]  channel: channel index
    \param[in]  volume: new volume
    \param[out] none
    \retval     usbh status
*/
static usbh_status audio_volume_set (usbh_host *puhost, uint8_t feature, uint8_t channel, uint16_t volume)
{
    usbh_status status = USBH_BUSY;
    usbh_audio_handler *audio = (usbh_audio_handler *)puhost->active_class->class_data;

    audio->mem[0] = volume;

    status = usbh_ac_cur_set(puhost, 
                             UAC_FEATURE_UNIT, 
                             feature,
                             CONTROL_VOLUME,
                             channel,
                             2);

    return status; 
}

/*!
    \brief      the function informs user that Settings have been changed
    \param[in]  pudev: selected usb device core
    \param[in]  puhost: selected usb host
    \param[out] none
    \retval     none
*/
__weak void usbh_audio_playback_set(usbh_host *puhost)
{

}

/*!
    \brief      the function informs user that User data are processed
    \param[in]  pudev: selected device
    \param[out] none
    \retval     none
*/
__weak void  usbh_audio_buffer_empty_callback(usbh_host *puhost)
{

}
