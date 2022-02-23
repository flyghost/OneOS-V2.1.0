/*!
    \file  vido_core.h
    \brief the header file of USB video device class core functions

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

#ifndef __VIDEO_CORE_H
#define __VIDEO_CORE_H

#include "usbd_enum.h"

/* USB video device class specification version 1.10 */
#ifdef UVC_1_1
    #define UVC_VERSION                            0x0110      // UVC 1.1
#else
    #define UVC_VERSION                            0x0100      // UVC 1.0
#endif

/* UVC class, subclass codes */
/* (USB_Video_Class_1.1.pdf, 3.2 Device Descriptor) */
#define UVC_DEVICE_CLASS_MISCELLANEOUS             0xFE     // kommt evtl. wieder raus
#define UVC_DEVICE_SUBCLASS                        0x02     // kommt evtl. wieder raus
#define UVC_DEVICE_PROTOCOL                        0x01     // kommt evtl. wieder raus

/* Video Interface Class Codes */
/* (USB_Video_Class_1.1.pdf, A.1 Video Interface Class Code) */
#define CC_VIDEO                                   0x0E

/* Video Interface Subclass Codes */
/* (USB_Video_Class_1.1.pdf, A.2 Video Interface Subclass Code) */
#define SC_UNDEFINED                               0x00
#define SC_VIDEOCONTROL                            0x01
#define SC_VIDEOSTREAMING                          0x02
#define SC_VIDEO_INTERFACE_COLLECTION              0x03

/* Video Interface Protocol Codes */
/* (USB_Video_Class_1.1.pdf, A.3 Video Interface Protocol Codes) */
#define PC_PROTOCOL_UNDEFINED                      0x00

/* Video Class-Specific Descriptor Types */
/* (USB_Video_Class_1.1.pdf, A.4 Video Class-Specific Descriptor Types) */
#define CS_UNDEFINED                               0x20
#define CS_DEVICE                                  0x21
#define CS_CONFIGURATION                           0x22
#define CS_STRING                                  0x23
#define CS_INTERFACE                               0x24
#define CS_ENDPOINT                                0x25

/* Video Class-Specific VideoControl Interface Descriptor Subtypes */
/* (USB_Video_Class_1.1.pdf, A.5 Video Class-Specific VC Interface Descriptor Subtypes) */
#define VC_DESCRIPTOR_UNDEFINED                    0x00
#define VC_HEADER                                  0x01
#define VC_INPUT_TERMINAL                          0x02
#define VC_OUTPUT_TERMINAL                         0x03
#define VC_SELECTOR_UNIT                           0x04
#define VC_PROCESSING_UNIT                         0x05
#define VC_EXTENSION_UNIT                          0x06

/* Video Class-Specific VideoStreaming Interface Descriptor Subtypes */
/* (USB_Video_Class_1.1.pdf, A.6 Video Class-Specific VS Interface Descriptor Subtypes) */
#define VS_UNDEFINED                               0x00
#define VS_INPUT_HEADER                            0x01
#define VS_OUTPUT_HEADER                           0x02
#define VS_STILL_IMAGE_FRAME                       0x03
#define VS_FORMAT_UNCOMPRESSED                     0x04
#define VS_FRAME_UNCOMPRESSED                      0x05
#define VS_FORMAT_MJPEG                            0x06
#define VS_FRAME_MJPEG                             0x07
#define VS_FORMAT_MPEG2TS                          0x0A
#define VS_FORMAT_DV                               0x0C
#define VS_COLORFORMAT                             0x0D
#define VS_FORMAT_FRAME_BASED                      0x10
#define VS_FRAME_FRAME_BASED                       0x11
#define VS_FORMAT_STREAM_BASED                     0x12

/* Video Class-Specific Endpoint Descriptor Subtypes */
/* (USB_Video_Class_1.1.pdf, A.7 Video Class-Specific Endpoint Descriptor Subtypes) */
#define EP_UNDEFINED                               0x00
#define EP_GENERAL                                 0x01
#define EP_ENDPOINT                                0x02
#define EP_INTERRUPT                               0x03

/* Video Class-Specific Request Codes */
/* (USB_Video_Class_1.1.pdf, A.8 Video Class-Specific Request Codes) */
#define RC_UNDEFINED                               0x00
#define SET_CUR                                    0x01
#define GET_CUR                                    0x81
#define GET_MIN                                    0x82
#define GET_MAX                                    0x83
#define GET_RES                                    0x84
#define GET_LEN                                    0x85
#define GET_INFO                                   0x86
#define GET_DEF                                    0x87

/* VideoControl Interface Control Selectors */
/* (USB_Video_Class_1.1.pdf, A.9.1 VideoControl Interface Control Selectors) */
#define VC_CONTROL_UNDEFINED                       0x00
#define VC_VIDEO_POWER_MODE_CONTROL                0x01
#define VC_REQUEST_ERROR_CODE_CONTROL              0x02

/* Request Error Code Control */
/* (USB_Video_Class_1.1.pdf, 4.2.1.2 Request Error Code Control) */
#define NO_ERROR_ERR                               0x00
#define NOT_READY_ERR                              0x01
#define WRONG_STATE_ERR                            0x02
#define POWER_ERR                                  0x03
#define OUT_OF_RANGE_ERR                           0x04
#define INVALID_UNIT_ERR                           0x05
#define INVALID_CONTROL_ERR                        0x06
#define INVALID_REQUEST_ERR                        0x07
#define UNKNOWN_ERR                                0xFF


/* Terminal Control Selectors */
/* (USB_Video_Class_1.1.pdf, A.9.2 Terminal Control Selectors) */
#define TE_CONTROL_UNDEFINED                       0x00

/* Selector Unit Control Selectors */
/* (USB_Video_Class_1.1.pdf, A.9.3 Selector Unit Control Selectors)*/
#define SU_CONTROL_UNDEFINED                       0x00
#define SU_INPUT_SELECT_CONTROL                    0x01

/* Camera Terminal Control Selectors */
/* (USB_Video_Class_1.1.pdf, A.9.4 Camera Terminal Control Selectors) */
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

/* Processing Unit Control Selectors */
/* (USB_Video_Class_1.1.pdf, A.9.5 Processing Unit Control Selectors) */
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

/* Extension Unit Control Selectors */
/* (USB_Video_Class_1.1.pdf, A.9.6 Extension Unit Control Selectors) */
#define XU_CONTROL_UNDEFINED                       0x00

/* VideoStreaming Interface Control Selectors */
/* (USB_Video_Class_1.1.pdf, A.9.7 VideoStreaming Interface Control Selectors) */
#define VS_CONTROL_UNDEFINED                       0x00
#define VS_PROBE_CONTROL                           0x01
#define VS_COMMIT_CONTROL                          0x02
#define VS_STILL_PROBE_CONTROL                     0x03
#define VS_STILL_COMMIT_CONTROL                    0x04
#define VS_STILL_IMAGE_TRIGGER_CONTROL             0x05
#define VS_STREAM_ERROR_CODE_CONTROL               0x06
#define VS_GENERATE_KEY_FRAME_CONTROL              0x07
#define VS_UPDATE_FRAME_SEGMENT_CONTROL            0x08
#define VS_SYNC_DELAY_CONTROL                      0x09


/* Defined Bits Containing Capabilities of the Control */
/* (USB_Video_Class_1.1.pdf, 4.1.2 Table 4-3 Defined Bits Containing Capabilities of the Control) */
#define SUPPORTS_GET                               0x01
#define SUPPORTS_SET                               0x02
#define STATE_DISABLED                             0x04
#define AUTOUPDATE_CONTROL                         0x08
#define ASYNCHRONOUS_CONTROL                       0x10

/* USB Terminal Types */
/* (USB_Video_Class_1.1.pdf, B.1 USB Terminal Types) */
#define TT_VENDOR_SPECIFIC                         0x0100
#define TT_STREAMING                               0x0101

/* Input Terminal Types */
/* (USB_Video_Class_1.1.pdf, B.2 Input Terminal Types) */
#define ITT_VENDOR_SPECIFIC                        0x0200
#define ITT_CAMERA                                 0x0201
#define ITT_MEDIA_TRANSPORT_INPUT                  0x0202

/* Output Terminal Types */
/* (USB_Video_Class_1.1.pdf, B.3 Output Terminal Types) */
#define OTT_VENDOR_SPECIFIC                        0x0300
#define OTT_DISPLAY                                0x0301
#define OTT_MEDIA_TRANSPORT_OUTPUT                 0x0302

/* External Terminal Types */
/* (USB_Video_Class_1.1.pdf, B.4 External Terminal Types) */
#define EXTERNAL_VENDOR_SPECIFIC                   0x0400
#define COMPOSITE_CONNECTOR                        0x0401
#define SVIDEO_CONNECTOR                           0x0402
#define COMPONENT_CONNECTOR                        0x0403

#define WBVAL(x) (x & 0xFF),((x >> 8) & 0xFF)
#define DBVAL(x) (x & 0xFF),((x >> 8) & 0xFF),((x >> 16) & 0xFF),((x >> 24) & 0xFF)

#ifdef UXGA_MODE
#define WIDTH                                      (uint16_t)800 
#define HEIGHT                                     (uint16_t)600
#define CAM_FPS                                    15
#endif

#ifdef SVGA_MODE
#define WIDTH                                      (uint16_t)400 
#define HEIGHT                                     (uint16_t)300
#define CAM_FPS                                    30
#endif

#define VIDEO_PACKET_SIZE                          (uint16_t)(1020) //128+130
#define MIN_BIT_RATE                               (uint32_t)(WIDTH * HEIGHT * 16 * CAM_FPS)//16 bit
#define MAX_BIT_RATE                               (uint32_t)(WIDTH * HEIGHT * 16 * CAM_FPS)
#define MAX_FRAME_SIZE                             (uint32_t)(40000)
#define INTERVAL                                   (uint32_t)(10000000 / CAM_FPS)

#define UVC_VC_ENDPOINT_DESC_SIZE                       (uint8_t)5
#define UVC_VC_INTERFACE_HEADER_DESC_SIZE(n)            (uint8_t)(12 + n)
#define UVC_CAMERA_TERMINAL_DESC_SIZE(n)                (uint8_t)(15 + n)
#define UVC_OUTPUT_TERMINAL_DESC_SIZE(n)                (uint8_t)(9 + n)
#define UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(a,b)    (uint8_t)(13 + a * b)

#define VS_FORMAT_UNCOMPRESSED_DESC_SIZE                (uint8_t)(0x0b)
#define VS_FRAME_UNCOMPRESSED_DESC_SIZE                 (uint8_t)(0x26)
#define VS_COLOR_MATCHING_DESC_SIZE                     (uint8_t)(6)

#define USB_VIDEO_DESC_SIZ (uint32_t)(\
            USB_CFG_DESC_LEN + \
            USB_IAD_DESC_LEN + \
            USB_ITF_DESC_LEN +  \
            UVC_VC_INTERFACE_HEADER_DESC_SIZE(1) + \
            UVC_CAMERA_TERMINAL_DESC_SIZE(2) + \
            UVC_OUTPUT_TERMINAL_DESC_SIZE(0) + \
            USB_ITF_DESC_LEN +  \
            UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(1,1) + \
            VS_FORMAT_UNCOMPRESSED_DESC_SIZE + \
            VS_FRAME_UNCOMPRESSED_DESC_SIZE  + \
            VS_COLOR_MATCHING_DESC_SIZE +\
            USB_ITF_DESC_LEN + \
            USB_EP_DESC_LEN)

#define VC_TERMINAL_SIZ (uint16_t)(UVC_VC_INTERFACE_HEADER_DESC_SIZE(1) + \
            UVC_CAMERA_TERMINAL_DESC_SIZE(2) + UVC_OUTPUT_TERMINAL_DESC_SIZE(0))

#define VC_HEADER_SIZ (uint16_t)(UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(1,1) + \
            VS_FORMAT_UNCOMPRESSED_DESC_SIZE + VS_FRAME_UNCOMPRESSED_DESC_SIZE + \
            VS_COLOR_MATCHING_DESC_SIZE)

#define USB_UVC_VCIF_NUM                           0
#define USB_UVC_VSIF_NUM                           1

#define VIDEO_TOTAL_IF_NUM                         2

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                    ((mA)/2)


/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_DIRECTION_MASK                0x80
#define USB_ENDPOINT_OUT(addr)                     ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                      ((addr) | 0x80)

enum uvc_play_status {
    PLAY_STATUS_STOP = 0,
    PLAY_STATUS_READY = 1,
    PLAY_STATUS_STREAMING = 2
};

/* UVC 1.0 uses only 26 first bytes */
typedef struct  _video_control {
    uint8_t    bmHint[2];                      // 2
    uint8_t    bFormatIndex[1];                // 3
    uint8_t    bFrameIndex[1];                 // 4
    uint8_t    dwFrameInterval[4];             // 8
    uint8_t    wKeyFrameRate[2];               // 10
    uint8_t    wPFrameRate[2];                 // 12
    uint8_t    wCompQuality[2];                // 14
    uint8_t    wCompWindowSize[2];             // 16
    uint8_t    wDelay[2];                      // 18
    uint8_t    dwMaxVideoFrameSize[4];         // 22
    uint8_t    dwMaxPayloadTransferSize[4];    // 26
    uint8_t    dwClockFrequency[4];            // 30
    uint8_t    bmFramingInfo[1];               // 31
    uint8_t    bPreferedVersion[1];            // 32
    uint8_t    bMinVersion[1];                 // 33
    uint8_t    bMaxVersion[1];                 // 34
} video_control;


#pragma pack(1)

typedef struct
{
    usb_desc_header  header;              /*!< regular descriptor header containing the descriptor's type and length */
    uint8_t bFirstInterface;              /*!< bFirstInterface */
    uint8_t bInterfaceCount;              /*!< bInterfaceCount */
    uint8_t bFunctionClass;               /*!< bFunctionClass */
    uint8_t bFunctionSubClass;            /*!< bFunctionSubClass */
    uint8_t bFunctionProtocol;            /*!< bFunctionProtocol */
    uint8_t iFunction;                    /*!< iFunction  */
} usb_desc_IAD;

typedef struct
{
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t  bDescriptorSubtype;          /*!< VC_HEADER subtype. */
    uint16_t bcdVDC;                      /*!< revision of class specification that this device is based upon */
    uint16_t wTotalLength;                /*!< total size of class-specific descriptors */
    uint32_t dwClockFrequency;            /*!< this device will provide timestamps and a device clock reference based on a XXMHz clock */
    uint8_t  bInCollection;               /*!< number of streaming interfaces */
    uint8_t  baInterfaceNr;               /*!< video streaming interface 4 belongs to this video control interface */
} usb_desc_vc_head;

typedef struct
{
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t  bDescriptorSubtype;          /*!< VC_INPUT_TERMINAL subtype */
    uint8_t  bTerminalID;                 /*!< ID of this input terminal */
    uint16_t wTerminalType;               /*!< ITT_CAMERA type. this terminal is a camera terminal representing the CMOS sensor */
    uint8_t  bAssocTerminal;              /*!< no association */
    uint8_t  iTerminal;                   /*!< unused */
    uint16_t wObjectiveFocalLengthMin;    /*!< minimum focal length (objective) */
    uint16_t wObjectiveFocalLengthMax;    /*!< maximum focal length (objective) */
    uint16_t wOcularFocalLength;          /*!< focal length (ocular) */
    uint8_t  bControlSize;                /*!< the size of the bmControls */
    uint16_t bmControls;                  /*!< supported controls */
} usb_desc_vc_input_terminal;

typedef struct
{
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t  bDescriptorSubtype;          /*!< OUTPUT_TERMINAL descriptor subtype */
    uint8_t  bTerminalID;                 /*!< ID of this terminal */
    uint16_t wTerminalType;               /*!< terminal type */
    uint8_t  bAssocTerminal;              /*!< no association */
    uint8_t  bSourceID;                   /*!< the input pin of this unit is connected to the output pin of source ID unit */
    uint8_t  iTerminal;                   /*!< unused */
} usb_desc_vc_output_terminal;

typedef struct
{
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t  bDescriptorSubtype;          /*!< VS_INPUT_HEADER descriptor subtype. */
    uint8_t  bNumFormats;                 /*!< one format descriptor follows */
    uint16_t wTotalLength;                /*!< total size of class-specific video streaming interface descriptors */
    uint8_t  bEndpointAddress;            /*!< address of the isochronous endpoint used for video data */
    uint8_t  bmInfo;                      /*!< no dynamic format change supported */
    uint8_t  bTerminalLink;               /*!< this video streaming interface supplies terminal ID (output Terminal) */
    uint8_t  bStillCaptureMethod;         /*!< device supports still image capture method 1. */
    uint8_t  bTriggerSupport;             /*!< hardware trigger supported for still image capture */
    uint8_t  bTriggerUsage;               /*!< hardware trigger should initiate a still image capture */
    uint8_t  bControlSize;                /*!< size of the bmaControls field */
    uint8_t  bmControls;                  /*!< video streaming specific controls supported. */
} usb_desc_vs_input_header;

typedef struct
{
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t  bDescriptorSubtype;          /*!< VS_FORMAT_MJPEG subtype. */
    uint8_t  bFormatIndex;                /*!< first (and only) format descriptor */
    uint8_t  bNumFrameDescriptors;        /*!< one frame descriptor for this format follows */
    uint8_t  bmFlags;                     /*!< uses fixed size samples */
    uint8_t  bDefaultFrameIndex;          /*!< default frame index is 1. */
    uint8_t  bAspectRatioX;               /*!< non-interlaced stream not required. */
    uint8_t  bAspectRatioY;               /*!< non-interlaced stream not required */
    uint8_t  bmInterlaceFlags;            /*!< non-interlaced stream */
    uint8_t  bCopyProtect;                /*!< no restrictions imposed on the duplication of this video stream */
} usb_desc_vs_format;

typedef struct
{
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t   bDescriptorSubtype;         /*!< VS_FORMAT_MJPEG subtype */
    uint8_t   bFrameIndex;                /*!< first (and only) format descriptor */
    uint8_t   bmCapabilities;             /*!< D1: Fixed frame-rate */
    uint16_t  wWidth;                     /*!< width of frame (2bytes) */
    uint16_t  wHeight;                    /*!< height of frame (2bytes) */
    uint32_t  dwMinBitRate;               /*!< min bit rate in bits/s (4bytes) */
    uint32_t  dwMaxBitRate;               /*!< max bit rate in bits/s (4bytes) */
    uint32_t  dwMaxVideoFrameBufferSize;  /*!< maximum video or still frame size, in bytes */
    uint32_t  dwDefaultFrameInterval;     /*!< default frame interval */
    uint8_t   bFrameIntervalType;         /*!< discrete frame interval */
    uint32_t  dwMinFrameInterval;         /*!< minimum frame interval */
    uint32_t  dwMaxFrameInterval;         /*!< maximum frame interval */
    uint32_t  dwFrameIntervalStep;        /*!< no frame interval step supported */
} usb_desc_vs_frame_uncompressed;

typedef struct
{
    usb_desc_header header;               /*!< descriptor header, including type and size */
    uint8_t  bDescriptorSubtype;          /*!< descriptor subtype VS_COLORFORMAT */
    uint8_t  bColorPrimaries;             /*!< bColorPrimarie : 1: BT.709, sRGB (default) */
    uint8_t  bTransferCharacteristics;    /*!< bTransferCharacteristics : 1: BT.709 (default) */
    uint8_t  bMatrixCoefficients;         /*!< bMatrixCoefficients : 1: BT. 709 */
} usb_desc_vs_colorformat;

#pragma pack()

/* USB configuration descriptor structure */
typedef struct
{
    usb_desc_config                 config;
    usb_desc_IAD                    iad;
    usb_desc_itf                    vc_itf;
    usb_desc_vc_head                vc_head;
    usb_desc_vc_input_terminal      vc_input_terminal;
    usb_desc_vc_output_terminal     vc_output_terminal;
    usb_desc_itf                    vs_itf0;
    usb_desc_vs_input_header        vs_input_header;
    usb_desc_vs_format              vs_format;
    usb_desc_vs_frame_uncompressed  vs_frame;
    usb_desc_vs_colorformat         vs_colorformat;
    usb_desc_itf                    vs_itf1;
    usb_desc_ep                     vs_ep;
} usb_uvc_desc_config_set;

typedef struct _usbd_video_handler
{
    uint8_t play_status;
    uint8_t payload_header[2];

    __IO uint8_t jpeg_encode_done;

    uint8_t *frame_ptr;
    uint16_t jpeg_fram_size;
    uint32_t packets_in_frame;
}usbd_video_handler;

extern usb_desc video_desc;
extern usb_class_core usbd_video_cb;

#endif /* __VIDEO_CORE_H */
