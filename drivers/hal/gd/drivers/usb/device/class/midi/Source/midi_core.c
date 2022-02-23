/*!
    \file  midi_core.c
    \brief MIDI class driver

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

#include "midi_core.h"

#include <string.h>

#define USBD_VID                     0x28e9
#define USBD_PID                     0x0610

//extern __IO uint8_t prev_transfer_complete;

//extern uint8_t key_buffer[];

/* Note:it should use the C99 standard when compiling the below codes */
/* USB standard device descriptor */
const usb_desc_dev midi_dev_desc =
{
    .header = 
     {
         .bLength          = USB_DEV_DESC_LEN, 
         .bDescriptorType  = USB_DESCTYPE_DEV
     },
    .bcdUSB                = 0x0200U,
    .bDeviceClass          = 0x00U,
    .bDeviceSubClass       = 0x00U,
    .bDeviceProtocol       = 0x00U,
    .bMaxPacketSize0       = USB_FS_EP0_MAX_LEN,
    .idVendor              = USBD_VID,
    .idProduct             = USBD_PID,
    .bcdDevice             = 0x0100U,
    .iManufacturer         = STR_IDX_MFC,
    .iProduct              = STR_IDX_PRODUCT,
    .iSerialNumber         = STR_IDX_SERIAL,
    .bNumberConfigurations = USBD_CFG_MAX_NUM
};

const usb_midi_desc_config_set midi_config_desc = 
{
    .config = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_config), 
             .bDescriptorType = USB_DESCTYPE_CONFIG 
         },
        .wTotalLength         = sizeof(usb_midi_desc_config_set),
        .bNumInterfaces       = 0x02U,
        .bConfigurationValue  = 0x01U,
        .iConfiguration       = 0x00U,
        .bmAttributes         = 0xA0U,
        .bMaxPower            = 0x32U
    },

    .midi_ctr_itf = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_itf), 
             .bDescriptorType = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber     = 0x00U,
        .bAlternateSetting    = 0x00U,
        .bNumEndpoints        = 0x00U,
        .bInterfaceClass      = 0x01U,
        .bInterfaceSubClass   = 0x01U,
        .bInterfaceProtocol   = 0x00U,
        .iInterface           = 0x00U
    },
    
    .ac_itf =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_desc_ac_itf), 
             .bDescriptorType = CS_INTERFACE
         },
        .bDescriptorSubtype   = MS_HEADER,
        .bcdADC               = 0x0100U,
        .bTotalLength         = 0x0009U,
        .bInCollection        = 0x01U,
        .bInterfaceNr         = 0x01U
    },
    
        .midi_str_itf = 
    {
        .header = 
         {
             .bLength         = sizeof(usb_desc_itf), 
             .bDescriptorType = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber     = 0x01U,
        .bAlternateSetting    = 0x00U,
        .bNumEndpoints        = 0x02U,
        .bInterfaceClass      = 0x01U,
        .bInterfaceSubClass   = 0x03U,
        .bInterfaceProtocol   = 0x00U,
        .iInterface           = 0x00U
    },

    .sc_itf =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_desc_sc_itf), 
             .bDescriptorType = CS_INTERFACE
         },
        .bDescriptorSubtype   = MS_HEADER,
        .bcdMSC               = 0x0100U,
        .bTotalLength         = 0x0041U,
    },

    .embedded_input_inf =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_midi_input_itf), 
             .bDescriptorType = CS_INTERFACE
         },
         .bDescriptorSubtype  = MIDI_IN_JACK,
         .bJackType           = EMBEDDED,
         .bJackID             = 0x01U,
         .iJack               = 0x00U
    },
    
    .external_input_inf =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_midi_input_itf), 
             .bDescriptorType = CS_INTERFACE
         },
         .bDescriptorSubtype  = MIDI_IN_JACK,
         .bJackType           = EXTERNAL,
         .bJackID             = 0x02U,
         .iJack               = 0x00U
    },
    
    .embedded_output_inf =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_midi_output_itf), 
             .bDescriptorType = CS_INTERFACE
         },
         .bDescriptorSubtype  = MIDI_OUT_JACK,
         .bJackType           = EMBEDDED,
         .bJackID             = 0x03U,
         .bNrInputPins        = 0x01U,
         .baSourceID          = 0x02U,
         .baSourcePin         = 0x01U,     
         .iJack               = 0x00U
    },
    
    .external_output_inf =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_midi_output_itf), 
             .bDescriptorType = CS_INTERFACE
         },
         .bDescriptorSubtype  = MIDI_OUT_JACK,
         .bJackType           = EXTERNAL,
         .bJackID             = 0x04U,
         .bNrInputPins        = 0x01U,
         .baSourceID          = 0x01U,
         .baSourcePin         = 0x01U,     
         .iJack               = 0x00U
    },
    
    .midi_epin = 
    {
        .header = 
         {
             .bLength         = sizeof(midi_desc_ep), 
             .bDescriptorType = USB_DESCTYPE_EP
         },
        .bEndpointAddress     = MIDI_IN_EP,
        .bmAttributes         = USB_EP_ATTR_BULK,
        .wMaxPacketSize       = MIDI_IN_PACKET,
        .bInterval            = 0x00U,
        .bRefresh             = 0x00U,
        .bSynchAddress        = 0X00U
    },
    
    .class_epin =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_midi_ep), 
             .bDescriptorType = CS_ENDPOINT
         },
         .bDescriptorSubtype  = MS_GENERAL,
         .bNumEmbMIDIJack     = 0x01U,
         .baAssocJackID       = 0x03U
    },
    
    .midi_epout = 
    {
        .header = 
         {
             .bLength         = sizeof(midi_desc_ep), 
             .bDescriptorType = USB_DESCTYPE_EP
         },
        .bEndpointAddress     = MIDI_OUT_EP,
        .bmAttributes         = USB_EP_ATTR_BULK,
        .wMaxPacketSize       = MIDI_OUT_PACKET,
        .bInterval            = 0x00U,
        .bRefresh             = 0x00U,
        .bSynchAddress        = 0X00U
    },
    
        .class_epout =
    {
         .header = 
         {
             .bLength         = sizeof(usb_class_midi_ep), 
             .bDescriptorType = CS_ENDPOINT
         },
         .bDescriptorSubtype  = MS_GENERAL,
         .bNumEmbMIDIJack     = 0x01U,
         .baAssocJackID       = 0x01U
    }
};

/* USB language ID Descriptor */
const usb_desc_LANGID usbd_language_id_desc = 
{
    .header = 
     {
         .bLength         = sizeof(usb_desc_LANGID), 
         .bDescriptorType = USB_DESCTYPE_STR
     },
    .wLANGID              = ENG_LANGID
};

/* USB manufacture string */
static const usb_desc_str manufacturer_string = 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(10), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
    .unicode_string = {'G', 'i', 'g', 'a', 'D', 'e', 'v', 'i', 'c', 'e'}
};

/* USB product string */
static const usb_desc_str product_string = 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(13), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
    .unicode_string = {'G', 'D', '3', '2', '-', 'U', 'S', 'B', '_', 'M', 'I', 'D', 'I'}
};

/* USBD serial string */
static usb_desc_str serial_string = 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(12), 
         .bDescriptorType = USB_DESCTYPE_STR,
     }
};

/* USB string descriptor */
void *const usbd_midi_strings[] = 
{
    [STR_IDX_LANGID]  = (uint8_t *)&usbd_language_id_desc,
    [STR_IDX_MFC]     = (uint8_t *)&manufacturer_string,
    [STR_IDX_PRODUCT] = (uint8_t *)&product_string,
    [STR_IDX_SERIAL]  = (uint8_t *)&serial_string
};

usb_desc midi_desc = {
    .dev_desc    = (uint8_t *)&midi_dev_desc,
    .config_desc = (uint8_t *)&midi_config_desc,
    .strings     = usbd_midi_strings
};

/* local function prototypes ('static') */
static uint8_t midi_init    (usb_dev *udev, uint8_t config_index);
static uint8_t midi_deinit  (usb_dev *udev, uint8_t config_index);
static uint8_t midi_req     (usb_dev *udev, usb_req *req);
static uint8_t midi_data_in (usb_dev *udev, uint8_t ep_num);

usb_class_core usbd_midi_cb = {
    .alter_set       = 0,

    .init            = midi_init,
    .deinit          = midi_deinit,
    .req_proc        = midi_req,
    .data_in         = midi_data_in
};

/*!
    \brief      register MIDI interface operation functions
    \param[in]  udev: pointer to USB device instance
    \param[in]  midi_fop: MIDI operation functuons structure
    \param[out] none
    \retval     USB device operation status
*/
uint8_t midi_itfop_register (usb_dev *udev, midi_fop_handler *midi_fop)
{
    if (NULL != midi_fop) {
        udev->dev.user_data = (void *)midi_fop;

        return USBD_OK;
    }

    return USBD_FAIL;
}

/*!
    \brief      send keyboard report
    \param[in]  pudev: pointer to USB device instance
    \param[in]  report: pointer to MIDI report
    \param[in]  len: data length
    \param[out] none
    \retval     USB device operation status
*/
uint8_t midi_report_send (usb_dev *pudev, uint8_t *report, uint16_t len)
{
    midi_keyboard_handler *midi = (midi_keyboard_handler *)pudev->dev.class_data[USBD_MIDI_INTERFACE];

    midi->prev_transfer_complete = 0;

    usbd_ep_send(pudev, MIDI_IN_EP, report, len);

    return USBD_OK;
}

/*!
    \brief      initialize the MIDI device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t midi_init (usb_dev *udev, uint8_t config_index)
{
    static midi_keyboard_handler midi_handler;

    memset((void *)&midi_handler, 0, sizeof(midi_keyboard_handler));

    /* initialize the data Tx and Rx endpoint */
    midi_desc_ep in_ep = midi_config_desc.midi_epin;

    usb_desc_ep inep = {
        .header           = in_ep.header,
        .bEndpointAddress = in_ep.bEndpointAddress,
        .bmAttributes     = in_ep.bmAttributes,
        .wMaxPacketSize   = in_ep.wMaxPacketSize,
        .bInterval        = in_ep.bInterval 
    };
    
    midi_desc_ep out_ep = midi_config_desc.midi_epout;

    usb_desc_ep outep = {
        .header           = out_ep.header,
        .bEndpointAddress = out_ep.bEndpointAddress,
        .bmAttributes     = out_ep.bmAttributes,
        .wMaxPacketSize   = out_ep.wMaxPacketSize,
        .bInterval        = out_ep.bInterval 
    };

    usbd_ep_setup (udev, &inep);
    usbd_ep_setup (udev, &outep);

    midi_handler.prev_transfer_complete = 1U;

    /* configure the head of note on message */
    midi_handler.data[0] = 0x09;
    /* configure note on message channel 0 */
    midi_handler.data[1] = 0x90;
    /* set volume maximization */
    midi_handler.data[3] = 0x7F;

    udev->dev.class_data[USBD_MIDI_INTERFACE] = (void *)&midi_handler;

    if (NULL != udev->dev.user_data) {
        ((midi_fop_handler *)udev->dev.user_data)->midi_itf_config();
    }


    return USBD_OK;
}

/*!
    \brief      de-initialize the MIDI device
    \param[in]  pudev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t midi_deinit (usb_dev *pudev, uint8_t config_index)
{
    /* deinitialize MIDI endpoints */
    usbd_ep_clear(pudev, MIDI_IN_EP);
    usbd_ep_clear(pudev, MIDI_OUT_EP);

    return USBD_OK;
}

/*!
    \brief      handle the MIDI class-specific requests
    \param[in]  pudev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t midi_req (usb_dev *pudev, usb_req *req)
{
    return USBD_OK;
}

/*!
    \brief      handle data stage
    \param[in]  pudev: pointer to USB device instance
    \param[in]  ep_num: endpoint identifier
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t midi_data_in (usb_dev *pudev, uint8_t ep_num)
{
    midi_keyboard_handler *midi = (midi_keyboard_handler *)pudev->dev.class_data[USBD_MIDI_INTERFACE];

    if (midi->data[2] != 0) {
        midi->data[2] = 0x00;

        usbd_ep_send(pudev, MIDI_IN_EP, midi->data, MIDI_IN_PACKET);
    } else {
        midi->prev_transfer_complete = 1;
    }

    return USBD_OK;
}
