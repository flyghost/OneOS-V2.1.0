/*!
    \file  standard_hid_core.h
    \brief definitions for HID core

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

#ifndef __MIDI_CORE_H
#define __MIDI_CORE_H

#include "usbd_enum.h"

#define USB_MIDI_CONFIG_DESC_LEN          0x61U
#define CS_INTERFACE                      0x24U
#define CS_ENDPOINT                       0x25U
#define MS_GENERAL                        0x01U
#define MS_HEADER                         0x01U
#define MIDI_IN_JACK                      0x02U
#define MIDI_OUT_JACK                     0x03U
#define EMBEDDED                          0x01U
#define EXTERNAL                          0x02U

#pragma pack(1)

typedef struct _usb_class_desc_ac_itf {
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t bDescriptorSubtype;           /*!< descriptor sub type */
    uint16_t bcdADC;                      /*!< procotol version */
    uint16_t bTotalLength;                /*!< total length */
    uint8_t bInCollection;                /*!< streaming interface number */
    uint8_t bInterfaceNr;                 /*!< streaming interface ID */
} usb_class_desc_ac_itf;

typedef struct _usb_class_desc_sc_itf {
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t bDescriptorSubtype;           /*!< descriptor sub type */
    uint16_t bcdMSC;                      /*!< MIDI streaming class procotol version */
    uint16_t bTotalLength;                /*!< total length of relevant streaming interface desc */
} usb_class_desc_sc_itf;

typedef struct _usb_class_midi_input_itf {
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t bDescriptorSubtype;           /*!< descriptor sub type */
    uint8_t bJackType;                    /*!< jack type */
    uint8_t bJackID;                      /*!< jack ID */
    uint8_t iJack;                        /*!< jack string index */
} usb_class_midi_input_itf;

typedef struct _usb_class_midi_output_itf {
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t bDescriptorSubtype;           /*!< descriptor sub type */
    uint8_t bJackType;                    /*!< jack type */
    uint8_t bJackID;                      /*!< jack ID */
    uint8_t bNrInputPins;                 /*!< input pins number */
    uint8_t baSourceID;                   /*!< source input jack ID */
    uint8_t baSourcePin;                  /*!< source pin */
    uint8_t iJack;                        /*!< jack string index */
} usb_class_midi_output_itf;

typedef struct _usb_midi_ep {
    usb_desc_header header;               /*!< descriptor header, including type and size. */
    uint8_t  bEndpointAddress;            /*!< logical address of the endpoint */
    uint8_t  bmAttributes;                /*!< endpoint attributes */
    uint16_t wMaxPacketSize;              /*!< size of the endpoint bank, in bytes */
    uint8_t  bInterval;                   /*!< polling interval in milliseconds for the endpoint if it is an INTERRUPT or ISOCHRONOUS type */
    uint8_t  bRefresh;                    /*!< reset to 0 */
    uint8_t  bSynchAddress;               /*!< the address of the endpoint used to communicate synchronization information if required by this endpoint */ 
} midi_desc_ep;

typedef struct _usb_class_midi_ep {
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t bDescriptorSubtype;           /*!< descriptor sub type */
    uint8_t bNumEmbMIDIJack;              /*!< embedded jack MIDI jack number */
    uint8_t baAssocJackID;                /*!< embedded jack ID */
} usb_class_midi_ep;

typedef struct
{
    usb_desc_config           config;
    usb_desc_itf              midi_ctr_itf;
    usb_class_desc_ac_itf     ac_itf;
    usb_desc_itf              midi_str_itf;
    usb_class_desc_sc_itf     sc_itf;
    usb_class_midi_input_itf  embedded_input_inf;
    usb_class_midi_input_itf  external_input_inf;
    usb_class_midi_output_itf embedded_output_inf;
    usb_class_midi_output_itf external_output_inf;
    midi_desc_ep              midi_epin;
    usb_class_midi_ep         class_epin;
    midi_desc_ep              midi_epout;
    usb_class_midi_ep         class_epout;
}usb_midi_desc_config_set;

#pragma pack()

typedef struct {
    uint8_t data[MIDI_IN_PACKET];
    __IO uint8_t prev_transfer_complete;
} midi_keyboard_handler;

typedef struct {
    void (*midi_itf_config) (void);
    void (*midi_itf_data_process) (usb_dev *udev);
} midi_fop_handler;

extern usb_desc midi_desc;
extern usb_class_core usbd_midi_cb;

/* function declarations */

uint8_t midi_itfop_register (usb_dev *udev, midi_fop_handler *midi_fop);

/* send message */
uint8_t midi_report_send (usb_dev *pudev, uint8_t *report, uint16_t len);

#endif  /* __MIDI_CORE_H */
