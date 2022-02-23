/*!
    \file  usbh_video.h
    \brief this file contains all the prototypes for the usbh_video.c

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

#ifndef __USBH_VIDEO_H
#define __USBH_VIDEO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbh_core.h"

/* maximum endpoint size in bytes */
#define UVC_RX_FIFO_SIZE_LIMIT                  1024
   
/* image width and height */
#define UVC_DEFAULT_TARGET_WIDTH                800
#define UVC_DEFAULT_TARGET_HEIGHT               600
 
#define UVC_DEFAULT_CAPTURE_MODE                USBH_VIDEO_MJPEG
//#define UVC_DEFAULT_CAPTURE_MODE                USBH_VIDEO_YUY2
   
/* uncompressed image frame size in byte */
#define UVC_UNCOMP_FRAME_SIZE                   (UVC_DEFAULT_TARGET_WIDTH * UVC_DEFAULT_TARGET_HEIGHT * 2)

#define UVC_MAX_FRAME_SIZE                      UVC_UNCOMP_FRAME_SIZE

//TODO - UVC_MAX_FRAME_SIZE for MJPEG mode can be smaller.
//Needed calve is send by camera - see "USBH_VS_GetCur" - dwMaxVideoFrameSize

/* video cs request data length */
#define LEN_DEV_POWRE_MODE                      1U
#define LEN_REQ_ERR_CODE                        1U
#define LEN_SCANNING_MODE                       1U
#define LEN_AUTO_EXPOSURE_MODE                  1U
#define LEN_AUTO_EXPOSURE_PRIO                  1U
#define LEN_EXPOSURE_TIME_ABSO                  4U
#define LEN_EXPOSURE_TIME_RELAT                 1U
#define LEN_FOCUS_ABSO                          2U
#define LEN_FOCUS_RELAT                         2U
#define LEN_FOCUS_AUTO                          1U
#define LEN_IRIS_ABSO                           2U
#define LEN_IRIS_RELAT                          1U
#define LEN_ZOOM_ABSO                           2U
#define LEN_ZOOM_RELAT                          3U
#define LEN_PAN_TILT_ABSO                       8U
#define LEN_PAN_TILT_RELAT                      4U
#define LEN_ROLL_ABSO                           2U
#define LEN_ROLL_RELAT                          2U
#define LEN_PRIVACY_SHUTTER                     1U
#define LEN_SELECTOR_UNIT_REQ_CTRL              1U
#define LEN_BACKLIGHT_COMPENSATION              2U
#define LEN_BRIGHTNESS                          2U
#define LEN_CONTRAST                            2U
#define LEN_GAIN                                2U
#define LEN_POWER_LINE_FREQUENCY                1U
#define LEN_HUE                                 2U
#define LEN_HUE_AUTO                            1U
#define LEN_SATURATION                          2U
#define LEN_SHARPNESS                           2U
#define LEN_GAMMA                               2U
#define LEN_WHITE_BALANCE_TEMPERATURE           2U
#define LEN_WHITE_BALANCE_TEMPERATURE_AUTO      1U
#define LEN_WHITE_BALANCE_COMPONENT             4U
#define LEN_WHITE_BALANCE_COMPONENT_AUTO        1U
#define LEN_DIGITAL_MULTIPLIER                  2U
#define LEN_DIGITAL_MULTIPLIER_LIMIT            2U
#define LEN_ANALOG_VIDEO_STANDARD               1U
#define LEN_ANALOG_VIDEO_LOCK_STATUS            1U

#define VC_FEATURE_MAX                          20

#define CT_CONTROL_UNDEFINED                       0x00
#define CT_SCANNING_MODE_CONTROL                   0x01
#define CT_AE_MODE_CONTROL                         0x02
#define CT_AE_PRIORITY_CONTROL                     0x03
#define CT_EXPOSURE_TIME_ABSOLUTE_CONTROL          0x04
#define CT_EXPOSURE_TIME_RELATIVE_CONTROL          0x05
#define CT_FOCUS_ABSOLUTE_CONTROL                  0x06
#define CT_FOCUS_RELATIVE_CONTROL                  0x07
#define CT_FOCUS_AUTO_CONTROL                      0x08
#define CT_IRIS_ABSOLUTE_CONTROL                   0x09
#define CT_IRIS_RELATIVE_CONTROL                   0x0A
#define CT_ZOOM_ABSOLUTE_CONTROL                   0x0B
#define CT_ZOOM_RELATIVE_CONTROL                   0x0C
#define CT_PANTILT_ABSOLUTE_CONTROL                0x0D
#define CT_PANTILT_RELATIVE_CONTROL                0x0E
#define CT_ROLL_ABSOLUTE_CONTROL                   0x0F
#define CT_ROLL_RELATIVE_CONTROL                   0x10
#define CT_PRIVACY_CONTROL                         0x11

#define PU_CONTROL_UNDEFINED                       0x00
#define PU_BACKLIGHT_COMPENSATION_CONTROL          0x01
#define PU_BRIGHTNESS_CONTROL                      0x02
#define PU_CONTRAST_CONTROL                        0x03
#define PU_GAIN_CONTROL                            0x04
#define PU_POWER_LINE_FREQUENCY_CONTROL            0x05
#define PU_HUE_CONTROL                             0x06
#define PU_SATURATION_CONTROL                      0x07
#define PU_SHARPNESS_CONTROL                       0x08
#define PU_GAMMA_CONTROL                           0x09
#define PU_WHITE_BALANCE_TEMPERATURE_CONTROL       0x0A
#define PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL  0x0B
#define PU_WHITE_BALANCE_COMPONENT_CONTROL         0x0C
#define PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL    0x0D
#define PU_DIGITAL_MULTIPLIER_CONTROL              0x0E
#define PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL        0x0F
#define PU_HUE_AUTO_CONTROL                        0x10
#define PU_ANALOG_VIDEO_STANDARD_CONTROL           0x11
#define PU_ANALOG_LOCK_STATUS_CONTROL              0x12

typedef enum
{
    USBH_VIDEO_MJPEG = 0,
    USBH_VIDEO_YUY2,
} usbh_video_target_format;

/* states for video state machine */
typedef enum
{
    VIDEO_INIT = 0,
    VIDEO_IDLE,
    VIDEO_CS_REQUESTS,
    VIDEO_SET_DEFAULT_FEATURE_UNIT,
    VIDEO_SET_INTERFACE,
    VIDEO_SET_STREAMING_INTERFACE,
    VIDEO_SET_CUR1,
    VIDEO_GET_RES,
    VIDEO_GET_CUR1,
    VIDEO_SET_CUR2,
    VIDEO_GET_CUR2,
    VIDEO_SET_CUR3,
    VIDEO_SET_INTERFACE0,
    VIDEO_SET_INTERFACE1,
    VIDEO_SET_INTERFACE2,
    VIDEO_ISOC_OUT,
    VIDEO_ISOC_IN,
    VIDEO_ISOC_POLL,
    VIDEO_ERROR,
} video_state;

typedef enum
{
    VIDEO_REQ_INIT = 1,
    VIDEO_REQ_IDLE, 
    VIDEO_REQ_SET_DEFAULT_IN_INTERFACE,
    VIDEO_REQ_SET_IN_INTERFACE,
    VIDEO_REQ_CS_VC_ITT_REQUESTS,
    VIDEO_REQ_CS_VC_PRU_REQUESTS,
    VIDEO_REQ_CS_VC_EXU_REQUESTS,
    VIDEO_REQ_CS_VS_REQUESTS,
} video_req_state;

typedef enum
{
    VIDEO_REQ_GET_INFO = 1,
    VIDEO_REQ_GET_MIN,
    VIDEO_REQ_GET_MAX,
    VIDEO_REQ_GET_DEF,
    VIDEO_REQ_GET_RESOLUTION,
    VIDEO_REQ_GET_LEN,
    VIDEO_REQ_GET_CUR,
    VIDEO_REQ_SET_CUR,
    VIDEO_REQ_SET_CUR2,
    VIDEO_REQ_CS_IDLE,
} video_cs_req_state;

typedef enum
{
    VIDEO_ITT_IDLE = 1,
    VIDEO_ITT_GET_INFO,
    VIDEO_ITT_GET_MIN,
    VIDEO_ITT_GET_MAX,
    VIDEO_ITT_GET_RES,
    VIDEO_ITT_GET_DEF,
    VIDEO_ITT_END
} video_cs_itt;

typedef enum
{
    VIDEO_PRU_IDLE = -1,
    VIDEO_PRU_GET_INFO = 0,
    VIDEO_PRU_GET_MIN,
    VIDEO_PRU_GET_MAX,
    VIDEO_PRU_GET_RES,
    VIDEO_PRU_GET_DEF,
    VIDEO_PRU_END
} video_cs_pru;

typedef enum
{
    VIDEO_EXU_IDLE = 1,
    VIDEO_EXU_GET_LEN,
    VIDEO_EXU_GET_INFO,
    VIDEO_EXU_GET_MIN,
    VIDEO_EXU_GET_MAX,
    VIDEO_EXU_GET_RES,
    VIDEO_EXU_GET_DEF,
    VIDEO_EXU_END
} video_cs_exu;

typedef enum
{
    VIDEO_CONTROL_INIT = 1,
    VIDEO_CONTROL_CHANGE,
    VIDEO_CONTROL_IDLE,
} video_control_state;

typedef enum
{
    VIDEO_STATE_IDLE = 1,
    VIDEO_STATE_START_IN,
    VIDEO_STATE_DATA_IN,
} video_stream_state;

typedef struct
{
    uint8_t              Ep;            //bEndpointAddress
    uint16_t             EpSize;        //wMaxPacketSize
    uint8_t              AltSettings;   //bAlternateSetting
    uint8_t              interface;     //bInterfaceNumber
    uint8_t              valid; 
    uint16_t             Poll;          //bInterval
}video_stream_in_handle;

typedef struct
{
    uint8_t              Ep;
    uint16_t             EpSize;
    uint8_t              interface;
    uint8_t              AltSettings;
    uint8_t              supported;

    uint8_t              Pipe;
    uint8_t              Poll;
    uint32_t             timer;

    uint8_t              asociated_as;

    uint8_t              *buf;
    uint8_t              *cbuf;
    uint32_t             partial_ptr;

    uint32_t             global_ptr;
    uint16_t             frame_length;
    uint32_t             total_length;
}video_interface_stream_prop;

typedef struct
{
    uint8_t              Ep;
    uint16_t             EpSize; 
    uint8_t              interface; 
    uint8_t              supported; 

    uint8_t              Pipe;
    uint8_t              Poll;
    uint32_t             timer;
}video_interface_control_prop;

#define VIDEO_MAX_VIDEO_STD_INTERFACE      0x05

/* video control descriptor */
#define VIDEO_MAX_NUM_IN_TERMINAL          10
#define VIDEO_MAX_NUM_OUT_TERMINAL         4
#define VIDEO_MAX_NUM_SELECTOR_UNIT        2
#define VIDEO_MAX_NUM_PROCESSING_UNIT      2
#define VIDEO_MAX_NUM_EXTENSION_UNIT       2

/* video steream descriptor */
#define VIDEO_MAX_NUM_IN_HEADER            3

/* video steream descriptor */
#define VIDEO_MAX_MJPEG_FORMAT             3
#define VIDEO_MAX_MJPEG_FRAME_D            10

#define VIDEO_MAX_UNCOMP_FORMAT            3
#define VIDEO_MAX_UNCOMP_FRAME_D           10

#define VIDEO_MAX_SAMFREQ_NBR              5
#define VIDEO_MAX_INTERFACE_NBR            5
#define VIDEO_MAX_CONTROLS_NBR             5

#define VS_PROBE_CONTROL                   0x01
#define VS_COMMIT_CONTROL                  0x02

/*  class-specific vc header descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //must be UVC_VC_HEADER
    uint8_t  bcdUVC[2];
    uint8_t  wTotalLength[2];       //header+units+terminals
    uint8_t  dwClockFrequency[4];
    uint8_t  bInCollection;
    uint8_t  baInterfaceNr[VIDEO_MAX_INTERFACE_NBR];
} video_desc_header;

/* vc input terminal descriptor */
typedef struct 
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //must be UVC_VC_INPUT_TERMINAL
    uint8_t  bTerminalID;
    uint8_t  wTerminalType[2];      //must be 0x0201 = (ITT_CAMERA)
    uint8_t  bAssocTerminal;
    uint8_t  iTerminal;
    uint8_t  wObjectiveFocalLengthMin[2];
    uint8_t  wObjectiveFocalLengthMax[2];
    uint8_t  wOcularFocalLength[2];
    uint8_t  bControlSize;
    uint8_t  bmControls[3];         //in fact, size of this array if defined by "bControlSize" value
} video_desc_it;

/* vc output terminal descriptor */
typedef struct 
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //must be UVC_VC_INPUT_TERMINAL
    uint8_t  bTerminalID;
    uint8_t  wTerminalType[2];
    uint8_t  bAssocTerminal;
    uint8_t  bSourceID;
    uint8_t  iTerminal;
} video_desc_ot;

/* vc camera terminal descriptor */
typedef video_desc_it VIDEO_CTDescTypeDef;

/* selector unit descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bUnitID;
    uint8_t  bNrInPins;
    uint8_t  bSourceID0;
    uint8_t  iSelector;
}video_desc_selector;

/* processing unit descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //Must be VC_PROCESSING_UNIT
    uint8_t  bUnitID;
    uint8_t  bSourceID;
    uint8_t  wMaxMultiplier[2];
    uint8_t  bControlSize;
    uint8_t  bmControls[3];
    uint8_t  iProcessing;
//    uint8_t  bmVideoStandards;
}video_desc_processing;

/* externsion unit descriptor */
typedef struct
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //Must be VC_EXTENSION_UNIT
    uint8_t  bUnitID;
    uint8_t  guidExtensionCode[16];
    uint8_t  bNumControls;
    uint8_t  bNrInPins;
    uint8_t  baSourceID0;
    uint8_t  bControlSize;
    uint8_t  bmControls[3];
    uint8_t  iExtension;
}video_desc_extension;

typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t wMaxTransferSize[2];
}video_cs_endpoint;

/* video stream descriptors */

/* vs input header descriptor */
typedef struct 
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;//must be UVC_VS_INPUT_HEADER
    uint8_t  bNumFormats;
    uint8_t  wTotalLength[2];
    uint8_t  bEndPointAddress;
    uint8_t  bmInfo;
    uint8_t  bTerminalLink;
    uint8_t  bStillCaptureMethod;
    uint8_t  bTriggerSupport;
    uint8_t  bTriggerUsage;
    uint8_t  bControlSize;
    uint8_t  bmaControls;
} video_desc_in_header;

/* vs mjpeg format descriptor */
typedef struct
{
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bFormatIndex;
    uint8_t bNumFrameDescriptors;
    uint8_t bmFlags;
    uint8_t bDefaultFrameIndex;
    uint8_t bAspectRatioX;
    uint8_t bAspectRatioY;
    uint8_t bmInterlaceFlags;
    uint8_t bCopyProtect;
} video_desc_mjpeg_format;


/* vs mjpeg frame descriptor */
typedef struct 
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //must be UVC_VS_INPUT_HEADER
    uint8_t  bFrameIndex;
    uint8_t  bmCapabilities;
    uint8_t  wWidth[2];
    uint8_t  wHeight[2];
    uint8_t  dwMinBitRate[4];
    uint8_t  dwMaxBitRate[4];
    uint8_t  dwDefaultFrameInterval[4];
    uint8_t  bFrameIntervalType;
    //dwFrameInterval*N is here
} video_desc_mjpeg_frame;

/* vs uncompressed format typ descriptor */
typedef struct 
{
    uint8_t  bLength;           
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //must be UVC_VS_INPUT_HEADER
    uint8_t  bFormatIndex;
    uint8_t  bNumFrameDescriptors;
    uint8_t  guidFormat[16];
    uint8_t  bBitsPerPixel;
    uint8_t  bDefaultFrameIndex;
    uint8_t  bAspectRatioX;
    uint8_t  bAspectRatioY;
    uint8_t  bmInterfaceFlags;
    uint8_t  bCopyProtect;
} video_desc_uncomp_format;

/* vs uncompressed frame descriptor */
typedef struct 
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;    //must be UVC_VS_INPUT_HEADER
    uint8_t  bFrameIndex;
    uint8_t  bmCapabilities;
    uint8_t  wWidth[2];
    uint8_t  wHeight[2];
    uint8_t  dwMinBitRate[4];
    uint8_t  dwMaxBitRate[4];
    uint8_t  dwMaxVideoFrameBufferSize[4];
    uint8_t  dwDefaultFrameInterval[4];
    uint8_t  bFrameIntervalType;
    uint8_t  dwMinFrameInterval[4];
    uint8_t  dwMaxFrameInterval[4];
    uint8_t  dwFrameIntervalStep[4];
    //dwFrameInterval*N is here
} video_desc_uncomp_frame;

/* class-specific vc (video control) interface descriptor*/
typedef struct
{
    video_desc_header       *header_desc;
    video_desc_it           *input_terminal_desc[VIDEO_MAX_NUM_IN_TERMINAL];
    video_desc_ot           *output_terminal_desc[VIDEO_MAX_NUM_OUT_TERMINAL];
    video_desc_selector     *selector_unit_desc[VIDEO_MAX_NUM_SELECTOR_UNIT];
    video_desc_processing   *processing_unit_desc[VIDEO_MAX_NUM_PROCESSING_UNIT];
    video_desc_extension    *extension_unit_desc[VIDEO_MAX_NUM_EXTENSION_UNIT];
    video_cs_endpoint       *cs_endpoint_desc;
} video_desc_vc;

/* class-specific vs (video stream) interface descriptor*/
typedef struct
{
    video_desc_in_header       *input_header[VIDEO_MAX_NUM_IN_HEADER];
    video_desc_mjpeg_format    *mjpeg_format[VIDEO_MAX_MJPEG_FORMAT];
    video_desc_mjpeg_frame     *mjpeg_frame[VIDEO_MAX_MJPEG_FRAME_D];
    video_desc_uncomp_format   *uncomp_format[VIDEO_MAX_UNCOMP_FORMAT];
    video_desc_uncomp_frame    *uncomp_frame[VIDEO_MAX_MJPEG_FRAME_D];
} video_desc_vs;

typedef struct
{
    video_desc_vc   cs_desc;    /* only one control descriptor*/
    video_desc_vs   vs_desc;

    uint16_t        input_terminal_num;
    uint16_t        output_terminal_num;
    uint16_t        camera_terminal_num;
    uint16_t        selector_unit_num;
    uint16_t        processing_unit_num;
    uint16_t        extension_unit_num;

    uint8_t         input_header_num;

    uint8_t         mjpeg_format_num;
    uint8_t         mjpeg_frame_num;

    uint8_t         uncomp_format_num;
    uint8_t         uncomp_frame_num;
} video_desc_class_specific;

//UVC 1.0 uses only 26 first bytes
typedef __packed struct 
{
    uint16_t    bmHint;
    uint8_t     bFormatIndex;
    uint8_t     bFrameIndex;
    uint32_t    dwFrameInterval;
    uint16_t    wKeyFrameRate;
    uint16_t    wPFrameRate;
    uint16_t    wCompQuality;
    uint16_t    wCompWindowSize;
    uint16_t    wDelay;
    uint32_t    dwMaxVideoFrameSize;
    uint32_t    dwMaxPayloadTransferSize;
    uint32_t    dwClockFrequency;
    uint8_t     bmFramingInfo;
    uint8_t     bPreferedVersion;
    uint8_t     bMinVersion;
    uint8_t     bMaxVersion;
} video_probe;

typedef struct _video_process
{
    video_req_state                 req_state;
    video_cs_req_state              cs_req_state;
    video_cs_itt                    cs_itt_req_state;
    video_cs_pru                    cs_pru_req_state;
    video_cs_exu                    cs_exu_req_state;
    video_control_state             control_state;
    video_stream_state              steam_in_state;

    video_stream_in_handle          stream_in[VIDEO_MAX_VIDEO_STD_INTERFACE];
    video_desc_class_specific       class_desc;

    video_interface_control_prop    control;
    video_interface_stream_prop     camera;

    video_probe                     probe_params;

    uint16_t                        mem[8];
    uint16_t                        extern_info_len;
    uint8_t                         enum_feature;
    uint8_t                         enum_index;
    uint32_t                        enum_control;
} usbh_video_handle;


/* Video Interface Subclass Codes */
#define CC_VIDEO                                        0x0E

/* Video Interface Subclass Codes */
#define USB_SUBCLASS_UNDEFINED                          0x00
#define USB_SUBCLASS_VIDEOCONTROL                       0x01
#define USB_SUBCLASS_VIDEOSTREAMING                     0x02
#define USB_SUBCLASS_VIDEO_INTERFACE_COLLECTION         0x03

/* video interface protocol */
#define USB_PROTOCOL_UNDEFINED                          0x00

/* Class specific */
#define USB_DESC_TYPE_CS_INTERFACE                      0x24
#define USB_DESC_TYPE_CS_ENDPOINT                       0x25

/* Video Class-Specific VideoControl Interface Descriptor Subtypes 
  (USB_Video_Class_1.1.pdf, A.5 Video Class-Specific VC Interface Descriptor Subtypes) */
#define UVC_VC_HEADER                                   0x01
#define UVC_VC_INPUT_TERMINAL                           0x02
#define UVC_VC_OUTPUT_TERMINAL                          0x03
#define UVC_VC_SELECTOR_UNIT                            0x04
#define UVC_VC_PROCESSING_UNIT                          0x05
#define UVC_VC_EXTENSION_UNIT                           0x06

/* Video Class-Specific VideoStreaming Interface Descriptor Subtypes
  (USB_Video_Class_1.1.pdf, A.6 Video Class-Specific VS Interface Descriptor Subtypes) */
#define UVC_VS_UNDEFINED                                0x00
#define UVC_VS_INPUT_HEADER                             0x01
#define UVC_VS_OUTPUT_HEADER                            0x02
#define UVC_VS_STILL_IMAGE_FRAME                        0x03
#define UVC_VS_FORMAT_UNCOMPRESSED                      0x04
#define UVC_VS_FRAME_UNCOMPRESSED                       0x05
#define UVC_VS_FORMAT_MJPEG                             0x06
#define UVC_VS_FRAME_MJPEG                              0x07
#define UVC_VS_FORMAT_MPEG2TS                           0x0A
#define UVC_VS_FORMAT_DV                                0x0C
#define UVC_VS_COLORFORMAT                              0x0D
#define UVC_VS_FORMAT_FRAME_BASED                       0x10
#define UVC_VS_FRAME_FRAME_BASED                        0x11
#define UVC_VS_FORMAT_STREAM_BASED                      0x12

#define UVC_AS_GENERAL                                  0x01
#define UVC_FORMAT_TYPE                                 0x02
#define UVC_FORMAT_SPECIFIC                             0x03

/* Video Class-Specific Endpoint Descriptor Subtypes */
#define UVC_EP_GENERAL                                  0x01
#define UVC_EP_STANDARD                                 0x02
#define UVC_EP_INTERRUPT                                0x03

/* Video Class-Specific Request Codes */
#define UVC_SET_                                        0x00
#define UVC_GET_                                        0x80

#define UVC__CUR                                        0x1
#define UVC__MIN                                        0x2
#define UVC__MAX                                        0x3
#define UVC__RES                                        0x4
#define UVC__LEN                                        0x5
#define UVC__INFO                                       0x6
#define UVC__DEF                                        0x7

#define UVC_SET_CUR                                     (UVC_SET_ | UVC__CUR)
#define UVC_GET_CUR                                     (UVC_GET_ | UVC__CUR)
#define UVC_SET_MIN                                     (UVC_SET_ | UVC__MIN)
#define UVC_GET_MIN                                     (UVC_GET_ | UVC__MIN)
#define UVC_SET_MAX                                     (UVC_SET_ | UVC__MAX)
#define UVC_GET_MAX                                     (UVC_GET_ | UVC__MAX)
#define UVC_SET_RES                                     (UVC_SET_ | UVC__RES)
#define UVC_GET_RES                                     (UVC_GET_ | UVC__RES)
#define UVC_SET_LEN                                     (UVC_SET_ | UVC__LEN)
#define UVC_GET_LEN                                     (UVC_GET_ | UVC__LEN)
#define UVC_GET_INFO                                    (UVC_GET_ | UVC__INFO)
#define UVC_GET_DEF                                     (UVC_GET_ | UVC__DEF)


#define UVC_GET_STAT                                    0xff

/* Terminals - 2.1 USB Terminal Types */
#define UVC_TERMINAL_UNDEFINED                          0x100
#define UVC_TERMINAL_STREAMING                          0x101
#define UVC_TERMINAL_VENDOR_SPEC                        0x1FF

/* Table B - 2 Input Termeinal Types */
#define ITT_VENDOR_SPECIFIC                             0x0200
#define ITT_CAMERA                                      0x0201
#define ITT_MEDIA_TRANSPORT_INPUT                       0x0202

/* Table B - 3 Output Terminal Types */
#define OTT_VENDOR_SPECIFIC                             0x0300
#define OTT_DISPLAY                                     0x0301
#define OTT_MEDIA_TRANSPORT_OUTPUT                      0x0302

#define LE16(addr)        (((uint16_t)(addr)[0]) | \
                           ((uint16_t)(((uint32_t)(addr)[1]) << 8)))

#define LE24(addr)        (((uint32_t)(addr)[0]) | \
                           (((uint32_t)(addr)[1]) << 8) | \
                           (((uint32_t)(addr)[2]) << 16))

extern usbh_class  usbh_video;

void video_stream_init_buffers(uint8_t* buffer0, uint8_t* buffer1);

void video_stream_ready_update(void);

usbh_status usbh_video_frequency_set (usbh_host *puhost,
                                      uint16_t sample_rate,
                                      uint8_t channel_num,
                                      uint8_t data_width);

usbh_status usbh_video_process (usbh_host *puhost);


#ifdef __cplusplus
}
#endif

#endif /* __USBH_VIDEO_H */


