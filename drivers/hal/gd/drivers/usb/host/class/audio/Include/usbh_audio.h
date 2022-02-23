/*!
    \file  usbh_audio.h
    \brief this file contains all the prototypes for the usbh_audio.c

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

#ifndef __USBH_AUDIO_H
#define __USBH_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbh_core.h"

#define AUDIO_MAX_AUDIO_STD_INTERFACE      0x05
#define AUDIO_MAX_FREQ_SUPPORTED           0x05
#define AUDIO_MAX_STREAMING_INTERFACE      0x05
#define AUDIO_MAX_NUM_IN_TERMINAL          0x04
#define AUDIO_MAX_NUM_OUT_TERMINAL         0x04
#define AUDIO_MAX_NUM_FEATURE_UNIT         0x04
#define AUDIO_MAX_NUM_MIXER_UNIT           0x04
#define AUDIO_MAX_NUM_SELECTOR_UNIT        0x04

#define HEADPHONE_SUPPORTED                0x01
#define MICROPHONE_SUPPORTED               0x02
#define HEADSET_SUPPORTED                  0x03

#define AUDIO_MAX_SAMFREQ_NBR              5
#define AUDIO_MAX_INTERFACE_NBR            5
#define AUDIO_MAX_CONTROLS_NBR             5

typedef enum
{
    AUDIO_REQ_INIT = 1,
    AUDIO_REQ_IDLE, 
    AUDIO_REQ_SET_DEFAULT_IN_INTERFACE,
    AUDIO_REQ_SET_DEFAULT_OUT_INTERFACE,  
    AUDIO_REQ_SET_IN_INTERFACE,
    AUDIO_REQ_SET_OUT_INTERFACE,
    AUDIO_REQ_CS_REQUESTS,
} audio_req_state;

typedef enum
{
    AUDIO_REQ_SET_VOLUME = 1,
    AUDIO_REQ_SET_MUTE,
    AUDIO_REQ_GET_CURR_VOLUME,
    AUDIO_REQ_GET_MIN_VOLUME,
    AUDIO_REQ_GET_MAX_VOLUME, 
    AUDIO_REQ_GET_VOLUME, 
    AUDIO_REQ_GET_RESOLUTION,
    AUDIO_REQ_CS_IDLE,
} audio_cs_req_state;

typedef enum
{
    AUDIO_PLAYBACK_INIT = 1,
    AUDIO_PLAYBACK_SET_EP,
    AUDIO_PLAYBACK_SET_EP_FREQ,
    AUDIO_PLAYBACK_PLAY,
    AUDIO_PLAYBACK_IDLE,
} audio_play_state;

typedef enum
{
    VOLUME_UP = 1,
    VOLUME_DOWN = 2,
} audio_volume_ctrl;

typedef enum
{
    AUDIO_CONTROL_INIT = 1,
    AUDIO_CONTROL_CHANGE,
    AUDIO_CONTROL_IDLE,
    AUDIO_CONTROL_VOLUME_UP,
    AUDIO_CONTROL_VOLUME_DOWN,
} audio_control_state;

typedef enum
{
    AUDIO_DATA_START_OUT = 1,
    AUDIO_DATA_OUT,
} audio_data_processing;

/* structure for audio formate */
typedef struct
{
    uint8_t   channels;
    uint8_t   bits;
    uint32_t  sample_rate;
} audio_format;

typedef struct
{
    uint8_t              ep;
    uint16_t             ep_size;
    uint8_t              alt_settings;
    uint8_t              interface;
    uint8_t              valid;
    uint16_t             poll;
} audio_streaming_in_handler;

typedef struct
{
    uint8_t              ep;
    uint16_t             ep_size;
    uint8_t              alt_settings;
    uint8_t              interface;
    uint8_t              valid;
    uint16_t             poll;
} audio_streaming_out_handler;

typedef struct
{
    uint8_t              mute;
    uint32_t             volumeMin;
    uint32_t             volumeMax;
    uint32_t             volume;
    uint32_t             resolution;
} audio_control_attribute;

typedef struct
{
    uint8_t              ep;
    uint16_t             ep_size;
    uint8_t              interface;
    uint8_t              alt_settings;
    uint8_t              supported;

    uint8_t              pipe;
    uint8_t              poll;
    uint32_t             timer;

    uint8_t              asociated_as; 
    uint8_t              asociated_mixer; 
    uint8_t              asociated_selector; 
    uint8_t              asociated_feature; 
    uint8_t              asociated_terminal;
    uint8_t              asociated_channels;

    uint32_t             frequency; 
    uint8_t             *buf;
    uint8_t             *cbuf;
    uint32_t             partial_ptr;

    uint32_t             global_ptr;
    uint16_t             frame_length;
    uint32_t             total_length;

    audio_control_attribute attribute;
} audio_interface_stream_prop;

typedef struct
{
    uint8_t              ep;
    uint16_t             ep_size; 
    uint8_t              interface;
    uint8_t              supported;

    uint8_t              pipe;
    uint8_t              poll;
    uint32_t             timer;
} audio_interface_control_prop;

/* class-specific as(audio streaming) interface descriptor */
typedef struct
{
    uint8_t bLength; 
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bTerminalLink;
    uint8_t bDelay;
    uint8_t wFormatTag[2];
} audio_as_general_desc;

/* class-specific as(audio streaming) format type descriptor */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bFormatType;
    uint8_t bNrChannels;
    uint8_t bSubframeSize;
    uint8_t bBitResolution;
    uint8_t bSamFreqType;
    uint8_t tSamFreq[AUDIO_MAX_SAMFREQ_NBR][3];
} audio_as_format_type_desc;

/* class-specific as(audio streaming) interface descriptor */
typedef struct
{
    audio_as_general_desc       *GeneralDesc;
    audio_as_format_type_desc   *FormatTypeDesc;
} audio_as_desc;

/* 4.3.2  class-specific ac interface descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bcdADC[2];
    uint8_t  wTotalLength[2];
    uint8_t  bInCollection;
    uint8_t  baInterfaceNr[AUDIO_MAX_INTERFACE_NBR];
} audio_header_desc;

/* 4.3.2.1 input terminal descriptor */
typedef struct 
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bTerminalID;
    uint8_t  wTerminalType[2];
    uint8_t  bAssocTerminal;
    uint8_t  bNrChannels;
    uint8_t  wChannelConfig[2];
    uint8_t  iChannelNames;
    uint8_t  iTerminal;
} audio_it_desc;

/* 4.3.2.2 output terminal descriptor */
typedef struct 
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bTerminalID;
    uint8_t  wTerminalType[2];
    uint8_t  bAssocTerminal;
    uint8_t  bSourceID;
    uint8_t  iTerminal;
} audio_ot_desc;

/* 4.3.2.3 feature descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bUnitID;
    uint8_t  bSourceID;
    uint8_t  bControlSize;
    uint8_t  bmaControls[AUDIO_MAX_CONTROLS_NBR][2];
} audio_feature_desc;

/* 4.3.2.3 feature descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bUnitID; 
    uint8_t  bNrInPins; 
    uint8_t  bSourceID0; 
    uint8_t  bSourceID1;  
    uint8_t  bNrChannels;    
    uint8_t  bmChannelsConfig[2];
    uint8_t  iChannelsNames;
    uint8_t  bmaControls;
    uint8_t  iMixer;
} audio_mixer_desc;

/* 4.3.2.3 feature descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bUnitID;
    uint8_t  bNrInPins;
    uint8_t  bSourceID0;
    uint8_t  iSelector;
} audio_selector_desc;

/* class-specific ac(audio control) interface descriptor */
typedef struct
{
    audio_header_desc   *headerd_esc;
    audio_it_desc       *input_terminal_desc[AUDIO_MAX_NUM_IN_TERMINAL];
    audio_ot_desc       *output_terminal_desc[AUDIO_MAX_NUM_OUT_TERMINAL];
    audio_feature_desc  *feature_unit_desc[AUDIO_MAX_NUM_FEATURE_UNIT];
    audio_mixer_desc    *mixer_unit_desc[AUDIO_MAX_NUM_MIXER_UNIT];
    audio_selector_desc *selector_unit_desc[AUDIO_MAX_NUM_SELECTOR_UNIT];
} audio_ac_desc;

/* class-specific ac: global descriptor */
typedef struct
{
    audio_ac_desc   cs_desc; /* Only one control descriptor*/
    audio_as_desc   as_desc[AUDIO_MAX_STREAMING_INTERFACE];

    uint16_t as_num; 
    uint16_t input_terminal_num;
    uint16_t output_terminal_num;
    uint16_t feature_unit_num;
    uint16_t selector_unit_num;
    uint16_t mixer_unit_num;
} audio_class_specific_desc;

typedef struct _usbh_audio_handler
{
    audio_req_state                 req_state;
    audio_cs_req_state              cs_req_state;
    audio_play_state                play_state;
    audio_control_state             control_state;
    audio_data_processing           processing_state;

    audio_streaming_in_handler      stream_in[AUDIO_MAX_AUDIO_STD_INTERFACE];
    audio_streaming_out_handler     stream_out[AUDIO_MAX_AUDIO_STD_INTERFACE];
    audio_class_specific_desc       class_desc;

    audio_interface_stream_prop     headphone;
    audio_interface_stream_prop     microphone; 
    audio_interface_control_prop    control;
    uint16_t                        mem[8];
    uint8_t                         temp_feature;
    uint8_t                         temp_channels;
} usbh_audio_handler;


/* audio interface subclass codes */
#define AC_CLASS                        0x01

/* a.2 audio interface subclass codes */
#define USB_SUBCLASS_AUDIOCONTROL       0x01
#define USB_SUBCLASS_AUDIOSTREAMING     0x02
#define USB_SUBCLASS_MIDISTREAMING      0x03 

#define USB_DESC_TYPE_CS_INTERFACE      0x24
#define USB_DESC_TYPE_CS_ENDPOINT       0x25

/* a.5 audio class-specific ac interface descriptor subtypes */
#define UAC_HEADER                      0x01
#define UAC_INPUT_TERMINAL              0x02
#define UAC_OUTPUT_TERMINAL             0x03
#define UAC_MIXER_UNIT                  0x04
#define UAC_SELECTOR_UNIT               0x05
#define UAC_FEATURE_UNIT                0x06
#define UAC_PROCESSING_UNIT             0x07
#define UAC_EXTENSION_UNIT              0x08

/* audio class-specific endpoint descriptor subtypes */
#define EP_CONTROL_UNDEFINED            0x00
#define SAMPLING_FREQ_CONTROL           0x01
#define PITCH_CONTROL                   0x02

/* feature unit control selector */
#define CONTROL_FU_UNDEFINED            0x00
#define CONTROL_MUTE                    0x01
#define CONTROL_VOLUME                  0x02
#define CONTROL_BASS                    0x03
#define CONTROL_MID                     0x04
#define CONTROL_TREBLE                  0x05
#define CONTROL_GRAPHIC_EQUALIZER       0x06
#define CONTROL_AUTOMATIC_GAIN          0x07
#define CONTROL_DELAY                   0x08
#define CONTROL_BASS_BOOST              0x09
#define CONTROL_LOUDNESS                0x0A

/* terminal control selector */
#define TE_CONTROL_UNDEFINED            0x00
#define COPY_PROTECT_CONTROL            0x01

/* a.6 audio class-specific as interface descriptor subtypes */
#define UAC_AS_GENERAL                  0x01
#define UAC_FORMAT_TYPE                 0x02
#define UAC_FORMAT_SPECIFIC             0x03

/* a.8 audio class-specific endpoint descriptor subtypes */
#define UAC_EP_GENERAL                  0x01

/* a.9 audio class-specific request codes */
#define UAC_SET_                        0x00
#define UAC_GET_                        0x80

#define UAC__CUR                        0x1
#define UAC__MIN                        0x2
#define UAC__MAX                        0x3
#define UAC__RES                        0x4
#define UAC__MEM                        0x5

#define UAC_SET_CUR                     (UAC_SET_ | UAC__CUR)
#define UAC_GET_CUR                     (UAC_GET_ | UAC__CUR)
#define UAC_SET_MIN                     (UAC_SET_ | UAC__MIN)
#define UAC_GET_MIN                     (UAC_GET_ | UAC__MIN)
#define UAC_SET_MAX                     (UAC_SET_ | UAC__MAX)
#define UAC_GET_MAX                     (UAC_GET_ | UAC__MAX)
#define UAC_SET_RES                     (UAC_SET_ | UAC__RES)
#define UAC_GET_RES                     (UAC_GET_ | UAC__RES)
#define UAC_SET_MEM                     (UAC_SET_ | UAC__MEM)
#define UAC_GET_MEM                     (UAC_GET_ | UAC__MEM)

#define UAC_GET_STAT                    0xff

/* midi - a.1 ms class-specific interface descriptor subtypes */
#define UAC_MS_HEADER                   0x01
#define UAC_MIDI_IN_JACK                0x02
#define UAC_MIDI_OUT_JACK               0x03

/* midi - a.1 ms class-specific endpoint descriptor subtypes */
#define UAC_MS_GENERAL                  0x01

/* Terminals - 2.1 USB Terminal Types */
#define UAC_TERMINAL_UNDEFINED          0x100
#define UAC_TERMINAL_STREAMING          0x101
#define UAC_TERMINAL_VENDOR_SPEC        0x1FF

#define LE16(addr)        (((uint16_t)(addr)[0]) | \
                           ((uint16_t)(((uint32_t)(addr)[1]) << 8)))

#define LE24(addr)        (((uint32_t)(addr)[0]) | \
                           (((uint32_t)(addr)[1]) << 8) | \
                           (((uint32_t)(addr)[2]) << 16))

extern usbh_class  usbh_audio;

#define USBH_AUDIO_FREQUENCY_SET_CALLBACK   usbh_audio_playback_set

usbh_status usbh_audio_frequency_set (usbh_host *puhost,
                                      uint16_t sample_rate,
                                      uint8_t channel_num,
                                      uint8_t data_width);

usbh_status usbh_audio_play (usbh_host *puhost, uint8_t *buf, uint32_t length);
usbh_status usbh_audio_stop (usbh_host *puhost);
usbh_status usbh_audio_suspend (usbh_host *puhost);
usbh_status usbh_audio_resume (usbh_host *puhost);
usbh_status usbh_audio_volume_set (usbh_host *puhost, audio_volume_ctrl volume_ctl);

usbh_status usbh_audio_out_buffer_change (usbh_host *puhost, uint8_t *buf);

int32_t usbh_audio_out_offset_get (usbh_host *puhost);

void usbh_audio_playback_set(usbh_host *puhost);

void usbh_audio_buffer_empty_callback(usbh_host *puhost);


#ifdef __cplusplus
}
#endif

#endif /* __USBH_AUDIO_H */


