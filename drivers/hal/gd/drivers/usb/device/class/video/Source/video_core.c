/*!
    \file  video_core.c
    \brief USB video device class core functions

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

#include "video_core.h"
#include "dci_ov2640.h"

#include <stdio.h>
#include <string.h>

#define USBD_VID                     0x28E9U
#define USBD_PID                     0x9695U

/* local function prototypes ('static') */
static uint8_t video_init        (usb_dev *udev, uint8_t config_index);
static uint8_t video_deinit      (usb_dev *udev, uint8_t config_index);
static uint8_t video_req_handler (usb_dev *udev, usb_req *req);
static uint8_t video_set_itf     (usb_dev *udev, usb_req *req);
static uint8_t video_data_in     (usb_dev *udev, uint8_t ep_num);
static uint8_t video_data_out    (usb_dev *udev, uint8_t ep_num);
static uint8_t usbd_video_sof    (usb_dev *udev);

static void video_req_getcurrent (usb_dev *udev, usb_req *req);
static void video_req_setcurrent (usb_dev *udev, usb_req *req);

static uint16_t jpeg_data_len(usb_dev *udev);

/* data array for Video Probe and Commit */
__ALIGN_BEGIN video_control videoCommitControl __ALIGN_END =
{
    {0x00, 0x00},                     // bmHint
    {0x01},                           // bFormatIndex
    {0x01},                           // bFrameIndex
    {DBVAL(INTERVAL),},               // dwFrameInterval
    {0x00, 0x00,},                    // wKeyFrameRate
    {0x00, 0x00,},                    // wPFrameRate
    {0x00, 0x00,},                    // wCompQuality
    {0x00, 0x00,},                    // wCompWindowSize
    {0x00, 0x00},                     // wDelay
    {DBVAL(MAX_FRAME_SIZE)},          // dwMaxVideoFrameSize
    {0x00, 0x00, 0x00, 0x00},         // dwMaxPayloadTransferSize
    {0x00, 0x00, 0x00, 0x00},         // dwClockFrequency
    {0x00},                           // bmFramingInfo
    {0x00},                           // bPreferedVersion
    {0x00},                           // bMinVersion
    {0x00},                           // bMaxVersion
};

__ALIGN_BEGIN video_control videoProbeControl __ALIGN_END =
{
    {0x00, 0x00},                     // bmHint
    {0x01},                           // bFormatIndex
    {0x01},                           // bFrameIndex
    {DBVAL(INTERVAL),},               // dwFrameInterval
    {0x00, 0x00,},                    // wKeyFrameRate
    {0x00, 0x00,},                    // wPFrameRate
    {0x00, 0x00,},                    // wCompQuality
    {0x00, 0x00,},                    // wCompWindowSize
    {0x00, 0x00},                     // wDelay
    {DBVAL(MAX_FRAME_SIZE)},          // dwMaxVideoFrameSize
    {0x00, 0x00, 0x00, 0x00},         // dwMaxPayloadTransferSize
    {0x00, 0x00, 0x00, 0x00},         // dwClockFrequency
    {0x00},                           // bmFramingInfo
    {0x00},                           // bPreferedVersion
    {0x00},                           // bMinVersion
    {0x00},                           // bMaxVersion
};

usb_class_core usbd_video_cb = {
    .init      = video_init,
    .deinit    = video_deinit,
    .req_proc  = video_req_handler,
    .set_intf  = video_set_itf,
    .data_in   = video_data_in,
    .data_out  = video_data_out,
    .SOF       = usbd_video_sof
};

/* note:it should use the c99 standard when compiling the below codes */
/* USB standard device descriptor */
__ALIGN_BEGIN const usb_desc_dev video_dev_desc __ALIGN_END =
{
    .header = 
     {
         .bLength          = USB_DEV_DESC_LEN, 
         .bDescriptorType  = USB_DESCTYPE_DEV
     },
    .bcdUSB                = 0x0200U,
    .bDeviceClass          = 0xEFU,
    .bDeviceSubClass       = 0x02U,
    .bDeviceProtocol       = 0x01U,
    .bMaxPacketSize0       = USB_FS_EP0_MAX_LEN,
    .idVendor              = USBD_VID,
    .idProduct             = USBD_PID,
    .bcdDevice             = 0x0100U,
    .iManufacturer         = STR_IDX_MFC,
    .iProduct              = STR_IDX_PRODUCT,
    .iSerialNumber         = STR_IDX_SERIAL,
    .bNumberConfigurations = USBD_CFG_MAX_NUM
};

/* USB device configuration descriptor */
__ALIGN_BEGIN const usb_uvc_desc_config_set video_config_set __ALIGN_END = 
{
    .config =
    {
         .header = 
         {
             .bLength          = USB_CFG_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_CONFIG
         },
        .wTotalLength = USB_VIDEO_DESC_SIZ,
        .bNumInterfaces = 0x02,
        .bConfigurationValue = 0x01,
        .iConfiguration = 0x00,
        .bmAttributes = 0x80,
        .bMaxPower = USB_CONFIG_POWER_MA(500),
    },

    /* Interface Association Descriptor */
    .iad =
    {
         .header = 
         {
             .bLength          = USB_IAD_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_IAD
         },
        .bFirstInterface = 0x00,
        .bInterfaceCount = 0x02,
        .bFunctionClass = CC_VIDEO,
        .bFunctionSubClass = SC_VIDEO_INTERFACE_COLLECTION,
        .bFunctionProtocol = PC_PROTOCOL_UNDEFINED ,
        .iFunction = 0x02,
    },
        
    /* VideoControl Interface Descriptor */
    .vc_itf =
    {
         .header = 
         {
             .bLength          = USB_ITF_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber = USB_UVC_VCIF_NUM,
        .bAlternateSetting = 0x00,
        .bNumEndpoints = 0x00,
        .bInterfaceClass = CC_VIDEO,
        .bInterfaceSubClass = SC_VIDEOCONTROL,
        .bInterfaceProtocol = PC_PROTOCOL_UNDEFINED,
        .iInterface = 0x02, 
    },
    
    /* Class-specific VC Interface Descriptor */
    .vc_head =
    {
         .header = 
         {
             .bLength          = UVC_VC_INTERFACE_HEADER_DESC_SIZE(1), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VC_HEADER,
        .bcdVDC = UVC_VERSION,
        .wTotalLength = VC_TERMINAL_SIZ,
        .dwClockFrequency = 0x005B8D80,
        .bInCollection = 0x01,
        .baInterfaceNr = 0x01,
    },
    /* input terminal descriptor (Camera) */
    .vc_input_terminal =
    {
         .header = 
         {
             .bLength          = UVC_CAMERA_TERMINAL_DESC_SIZE(2), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VC_INPUT_TERMINAL,
        .bTerminalID = 0x01,
        .wTerminalType = ITT_CAMERA,
        .bAssocTerminal = 0x00,
        .iTerminal = 0x00,
        .wObjectiveFocalLengthMin = 0x0000,
        .wObjectiveFocalLengthMax = 0x0000,
        .wOcularFocalLength = 0x0000,
        .bControlSize = 0x02,
        .bmControls = 0x0000,
    },
    
    /* output terminal descriptor */
    .vc_output_terminal =
    {
         .header = 
         {
             .bLength          = UVC_OUTPUT_TERMINAL_DESC_SIZE(0), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VC_OUTPUT_TERMINAL,
        .bTerminalID = 0x02,
        .wTerminalType = TT_STREAMING,
        .bAssocTerminal = 0x00,
        .bSourceID = 0x01,
        .iTerminal = 0x00,
    },

    /* Video Streaming (VS) Interface Descriptor */
    .vs_itf0 =
    {
         .header = 
         {
             .bLength          = USB_ITF_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber = USB_UVC_VSIF_NUM,
        .bAlternateSetting = 0x00,
        .bNumEndpoints = 0x00,
        .bInterfaceClass = CC_VIDEO,
        .bInterfaceSubClass = SC_VIDEOSTREAMING,
        .bInterfaceProtocol = PC_PROTOCOL_UNDEFINED,
        .iInterface = 0x00, 
    },

    /* Class-specific VS Header Descriptor (Input) */
    .vs_input_header =
    {
         .header = 
         {
             .bLength          = UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(1,1), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_INPUT_HEADER,
        .bNumFormats = 0x01,
        .wTotalLength = VC_HEADER_SIZ,
        .bEndpointAddress = VIDEO_IN_EP,
        .bmInfo = 0x00,
        .bTerminalLink = 0x02,
        .bStillCaptureMethod = 0x00,
        .bTriggerSupport = 0x01,
        .bTriggerUsage = 0x00,
        .bControlSize = 0x01,
        .bmControls = 0x00
    },

    /* Class-specific VS Format Descriptor  */
    .vs_format =
    {
         .header = 
         {
             .bLength          = VS_FORMAT_UNCOMPRESSED_DESC_SIZE, 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_FORMAT_MJPEG,
        .bFormatIndex = 0x01,
        .bNumFrameDescriptors = 0x01,
        .bmFlags = 0x01,
        .bDefaultFrameIndex = 0x01,
        .bAspectRatioX = 0x00,
        .bAspectRatioY = 0x00, 
        .bmInterlaceFlags = 0x00,
        .bCopyProtect = 0x00,
    },

    /* Class-specific VS Frame Descriptor */
    .vs_frame =
    {
         .header = 
         {
             .bLength          = VS_FRAME_UNCOMPRESSED_DESC_SIZE, 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_FRAME_UNCOMPRESSED,
        .bFrameIndex = 0x01,
        .bmCapabilities = 0x02,
        .wWidth = WIDTH,
        .wHeight = HEIGHT ,
        .dwMinBitRate = MIN_BIT_RATE,
        .dwMaxBitRate = MAX_BIT_RATE, 
        .dwMaxVideoFrameBufferSize = MAX_FRAME_SIZE,
        .dwDefaultFrameInterval = INTERVAL,
        .bFrameIntervalType = 0x00,
        .dwMinFrameInterval = INTERVAL,
        .dwMaxFrameInterval = INTERVAL,
        .dwFrameIntervalStep = 0x00000000,
    },

    /* Color Matching Descriptor */
    .vs_colorformat = 
    {
         .header = 
         {
             .bLength          = VS_COLOR_MATCHING_DESC_SIZE, 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_COLORFORMAT,
        .bColorPrimaries = 0x01,
        .bTransferCharacteristics = 0x01,
        .bMatrixCoefficients = 0x04
    },

    /* Video Streaming (VS) Interface Descriptor, alternate setting 1 = operational setting */
    .vs_itf1 =
    {
         .header = 
         {
             .bLength          = USB_ITF_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber = USB_UVC_VSIF_NUM,
        .bAlternateSetting = 0x01,
        .bNumEndpoints = 0x01,
        .bInterfaceClass = CC_VIDEO,
        .bInterfaceSubClass = SC_VIDEOSTREAMING,
        .bInterfaceProtocol = PC_PROTOCOL_UNDEFINED,
        .iInterface = 0x00, 
    },

    /* Standard VS Isochronous Video data Endpoint Descriptor */
    .vs_ep =
    {
         .header = 
         {
             .bLength          = USB_EP_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_EP
         },
        .bEndpointAddress = VIDEO_IN_EP,
        .bmAttributes = USB_EP_ATTR_ISO,
        .wMaxPacketSize = VIDEO_PACKET_SIZE,
        .bInterval = 0x01,
    }
};

#ifdef USE_USB_HS
__ALIGN_BEGIN const usb_uvc_desc_config_set video_other_speed_config_set __ALIGN_END = 
{
    .config =
    {
         .header = 
         {
             .bLength          = USB_CFG_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_OTHER_SPD_CONFIG
         },
        .wTotalLength = USB_VIDEO_DESC_SIZ,
        .bNumInterfaces = 0x02,
        .bConfigurationValue = 0x01,
        .iConfiguration = 0x00,
        .bmAttributes = 0x80,
        .bMaxPower = USB_CONFIG_POWER_MA(500),
    },

    /* Interface Association Descriptor */
    .iad =
    {
         .header = 
         {
             .bLength          = USB_IAD_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_IAD
         },
        .bFirstInterface = 0x00,
        .bInterfaceCount = 0x02,
        .bFunctionClass = CC_VIDEO,
        .bFunctionSubClass = SC_VIDEO_INTERFACE_COLLECTION,
        .bFunctionProtocol = PC_PROTOCOL_UNDEFINED ,
        .iFunction = 0x02,
    },
        
    /* VideoControl Interface Descriptor */
    .vc_itf =
    {
         .header = 
         {
             .bLength          = USB_ITF_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber = USB_UVC_VCIF_NUM,
        .bAlternateSetting = 0x00,
        .bNumEndpoints = 0x00,
        .bInterfaceClass = CC_VIDEO,
        .bInterfaceSubClass = SC_VIDEOCONTROL,
        .bInterfaceProtocol = PC_PROTOCOL_UNDEFINED,
        .iInterface = 0x02, 
    },
    
    /* Class-specific VC Interface Descriptor */
    .vc_head =
    {
         .header = 
         {
             .bLength          = UVC_VC_INTERFACE_HEADER_DESC_SIZE(1), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VC_HEADER,
        .bcdVDC = UVC_VERSION,
        .wTotalLength = VC_TERMINAL_SIZ,
        .dwClockFrequency = 0x005B8D80,
        .bInCollection = 0x01,
        .baInterfaceNr = 0x01,
    },
    /* input terminal descriptor (Camera) */
    .vc_input_terminal =
    {
         .header = 
         {
             .bLength          = UVC_CAMERA_TERMINAL_DESC_SIZE(2), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VC_INPUT_TERMINAL,
        .bTerminalID = 0x01,
        .wTerminalType = ITT_CAMERA,
        .bAssocTerminal = 0x00,
        .iTerminal = 0x00,
        .wObjectiveFocalLengthMin = 0x0000,
        .wObjectiveFocalLengthMax = 0x0000,
        .wOcularFocalLength = 0x0000,
        .bControlSize = 0x02,
        .bmControls = 0x0000,
    },
    
    /* output terminal descriptor */
    .vc_output_terminal =
    {
         .header = 
         {
             .bLength          = UVC_OUTPUT_TERMINAL_DESC_SIZE(0), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VC_OUTPUT_TERMINAL,
        .bTerminalID = 0x02,
        .wTerminalType = TT_STREAMING,
        .bAssocTerminal = 0x00,
        .bSourceID = 0x01,
        .iTerminal = 0x00,
    },

    /* Video Streaming (VS) Interface Descriptor */
    .vs_itf0 =
    {
         .header = 
         {
             .bLength          = USB_ITF_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber = USB_UVC_VSIF_NUM,
        .bAlternateSetting = 0x00,
        .bNumEndpoints = 0x00,
        .bInterfaceClass = CC_VIDEO,
        .bInterfaceSubClass = SC_VIDEOSTREAMING,
        .bInterfaceProtocol = PC_PROTOCOL_UNDEFINED,
        .iInterface = 0x00, 
    },

    /* Class-specific VS Header Descriptor (Input) */
    .vs_input_header =
    {
         .header = 
         {
             .bLength          = UVC_VS_INTERFACE_INPUT_HEADER_DESC_SIZE(1,1), 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_INPUT_HEADER,
        .bNumFormats = 0x01,
        .wTotalLength = VC_HEADER_SIZ,
        .bEndpointAddress = VIDEO_IN_EP,
        .bmInfo = 0x00,
        .bTerminalLink = 0x02,
        .bStillCaptureMethod = 0x00,
        .bTriggerSupport = 0x01,
        .bTriggerUsage = 0x00,
        .bControlSize = 0x01,
        .bmControls = 0x00
    },

    /* Class-specific VS Format Descriptor  */
    .vs_format =
    {
         .header = 
         {
             .bLength          = VS_FORMAT_UNCOMPRESSED_DESC_SIZE, 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_FORMAT_MJPEG,
        .bFormatIndex = 0x01,
        .bNumFrameDescriptors = 0x01,
        .bmFlags = 0x01,
        .bDefaultFrameIndex = 0x01,
        .bAspectRatioX = 0x00,
        .bAspectRatioY = 0x00, 
        .bmInterlaceFlags = 0x00,
        .bCopyProtect = 0x00,
    },

    /* Class-specific VS Frame Descriptor */
    .vs_frame =
    {
         .header = 
         {
             .bLength          = VS_FRAME_UNCOMPRESSED_DESC_SIZE, 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_FRAME_UNCOMPRESSED,
        .bFrameIndex = 0x01,
        .bmCapabilities = 0x02,
        .wWidth = WIDTH,
        .wHeight = HEIGHT ,
        .dwMinBitRate = MIN_BIT_RATE,
        .dwMaxBitRate = MAX_BIT_RATE, 
        .dwMaxVideoFrameBufferSize = MAX_FRAME_SIZE,
        .dwDefaultFrameInterval = INTERVAL,
        .bFrameIntervalType = 0x00,
        .dwMinFrameInterval = INTERVAL,
        .dwMaxFrameInterval = INTERVAL,
        .dwFrameIntervalStep = 0x00000000,
    },

    /* Color Matching Descriptor */
    .vs_colorformat = 
    {
         .header = 
         {
             .bLength          = VS_COLOR_MATCHING_DESC_SIZE, 
             .bDescriptorType  = CS_INTERFACE
         },
        .bDescriptorSubtype = VS_COLORFORMAT,
        .bColorPrimaries = 0x01,
        .bTransferCharacteristics = 0x01,
        .bMatrixCoefficients = 0x04
    },

    /* Video Streaming (VS) Interface Descriptor, alternate setting 1 = operational setting */
    .vs_itf1 =
    {
         .header = 
         {
             .bLength          = USB_ITF_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_ITF
         },
        .bInterfaceNumber = USB_UVC_VSIF_NUM,
        .bAlternateSetting = 0x01,
        .bNumEndpoints = 0x01,
        .bInterfaceClass = CC_VIDEO,
        .bInterfaceSubClass = SC_VIDEOSTREAMING,
        .bInterfaceProtocol = PC_PROTOCOL_UNDEFINED,
        .iInterface = 0x00, 
    },

    /* Standard VS Isochronous Video data Endpoint Descriptor */
    .vs_ep =
    {
         .header = 
         {
             .bLength          = USB_EP_DESC_LEN, 
             .bDescriptorType  = USB_DESCTYPE_EP
         },
        .bEndpointAddress = VIDEO_IN_EP,
        .bmAttributes = USB_EP_ATTR_ISO,
        .wMaxPacketSize = VIDEO_PACKET_SIZE,
        .bInterval = 0x01,
    }
};

__ALIGN_BEGIN const uint8_t usbd_qualifier_desc[10] __ALIGN_END = 
{
    0x0A, 
    0x06,
    0x00, 
    0x02,
    0x00, 
    0x00,
    0x00, 
    0x40,
    0x01, 
    0x00
};
#endif /* USE_USB_HS */

/* USB language ID descriptor */
__ALIGN_BEGIN static const usb_desc_LANGID usbd_language_id_desc __ALIGN_END = 
{
    .header = 
     {
         .bLength         = sizeof(usb_desc_LANGID), 
         .bDescriptorType = USB_DESCTYPE_STR
     },

    .wLANGID = ENG_LANGID
};

/* USB manufacture string */
__ALIGN_BEGIN static const usb_desc_str manufacturer_string __ALIGN_END= 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(10), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
    .unicode_string = {'G', 'i', 'g', 'a', 'D', 'e', 'v', 'i', 'c', 'e'}
};

/* USB product string */
__ALIGN_BEGIN static const usb_desc_str product_string __ALIGN_END= 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(20), 
         .bDescriptorType = USB_DESCTYPE_STR,
     },
    .unicode_string = {'G', 'D', '3', '2', '-', 'U', 'S', 'B', '_', 'V', 'i', 'd', 'e', 'o', 'C', 'a', 'm', 'e', 'r', 'a'}
};

/* USBD serial string */
__ALIGN_BEGIN static usb_desc_str serial_string __ALIGN_END= 
{
    .header = 
     {
         .bLength         = USB_STRING_LEN(12), 
         .bDescriptorType = USB_DESCTYPE_STR,
     }
};

/* USB string descriptor */
void *const usbd_video_strings[] = 
{
    [STR_IDX_LANGID]  = (uint8_t *)&usbd_language_id_desc,
    [STR_IDX_MFC]     = (uint8_t *)&manufacturer_string,
    [STR_IDX_PRODUCT] = (uint8_t *)&product_string,
    [STR_IDX_SERIAL]  = (uint8_t *)&serial_string
};

usb_desc video_desc = {
    .dev_desc    = (uint8_t *)&video_dev_desc,
    .config_desc = (uint8_t *)&video_config_set,
    .strings     = usbd_video_strings,
#ifdef USE_USB_HS
    .other_speed_config_desc = (uint8_t *)&video_other_speed_config_set,
    .qualifier_desc = (uint8_t *)&usbd_qualifier_desc
#endif /* USE_USB_HS */
};

/*!
    \brief      initialize the VIDEO device
    \param[in]  udev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
uint8_t video_init (usb_dev *udev, uint8_t config_index)
{
    static usbd_video_handler video_handler;

    memset((void *)&video_handler, 0, sizeof(usbd_video_handler));

    usbd_ep_setup (udev, &(video_config_set.vs_ep));

    video_handler.payload_header[0] = 0x02;
    video_handler.play_status = PLAY_STATUS_STOP;
    video_handler.packets_in_frame = 1U;

    udev->dev.class_data[USBD_VIDEO_INTERFACE] = (void *)&video_handler;

    return USBD_OK;
}

/*!
    \brief      de-initialize the VIDEO device
    \param[in]  udev: pointer to USB device instance
    \param[in]  config_index: configuration index
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t video_deinit (usb_dev *udev, uint8_t config_index)
{
    /* deinitialize video endpoints */
    usbd_ep_clear(udev, VIDEO_IN_EP);

    return USBD_OK;
}

/*!
    \brief      handle the VIDEO class-specific requests
    \param[in]  udev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t video_req_handler (usb_dev *udev, usb_req *req)
{
    uint8_t status = REQ_NOTSUPP;

    switch (req->bRequest) {
    case GET_CUR:
    case GET_DEF:
    case GET_MIN:
    case GET_MAX:
        video_req_getcurrent(udev, req);

        status = REQ_SUPP;
        break;

    case SET_CUR:
        video_req_setcurrent(udev, req);

        status = REQ_SUPP;
        break;

    default:
        break;
    }

    return status;
}

/*!
    \brief      handle the VIDEO set interface requests
    \param[in]  udev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t video_set_itf (usb_dev *udev, usb_req *req)
{
    usbd_video_handler *video = (usbd_video_handler *)udev->dev.class_data[USBD_VIDEO_INTERFACE];

    udev->dev.class_core->alter_set = (uint8_t)(req->wValue);

    if (udev->dev.class_core->alter_set == 1) {
        video->play_status = PLAY_STATUS_READY;

        dci_start();
    } else {
        usbd_fifo_flush (udev, VIDEO_IN_EP);

        video->play_status = PLAY_STATUS_STOP;

        dci_stop();
    }

    return USBD_OK;
}

/*!
    \brief      handle the VIDEO get current requests
    \param[in]  udev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static void video_req_getcurrent(usb_dev *udev, usb_req *req)
{
    usb_transc *transc_in = &udev->dev.transc_in[0];

    /* send the current mute state */
    usbd_fifo_flush (udev, EP0_OUT);

    if (req->wValue == 256) {
        transc_in->xfer_buf = (uint8_t*)&videoProbeControl;
    } else if (req->wValue == 512) {
        transc_in->xfer_buf = (uint8_t*)&videoCommitControl;
    } else {
        /* no operation */
    }

    transc_in->remain_len = req->wLength;
    udev->dev.class_core->command = GET_CUR;
}

/*!
    \brief      handle the VIDEO set current requests
    \param[in]  udev: pointer to USB device instance
    \param[in]  req: device class-specific request
    \param[out] none
    \retval     USB device operation status
*/
static void video_req_setcurrent(usb_dev *udev, usb_req *req)
{
    usb_transc *transc_out = &udev->dev.transc_out[0];

    if (req->wLength) {
        /* prepare the reception of the buffer over EP0 */
        if (req->wValue == 256) {
            transc_out->xfer_buf = (uint8_t*)&videoProbeControl;
        } else if (req->wValue == 512) {
            transc_out->xfer_buf = (uint8_t*)&videoCommitControl;
        } else {
            /* no operation */
        }

        transc_out->remain_len = req->wLength;
        udev->dev.class_core->command = SET_CUR;
    }
}

/*!
    \brief      handles the VIDEO IN data stage
    \param[in]  udev: pointer to USB device instance
    \param[in]  ep_num: endpoint number
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t video_data_in (usb_dev *udev, uint8_t ep_num)
{
    static uint16_t packet_count = 0;

    static uint16_t last_packet_size = 0;
    static uint32_t picture_pos = 0;

    static uint8_t tx_enable_flag = 0;
    static uint8_t packet[VIDEO_PACKET_SIZE];

    usbd_video_handler *video = (usbd_video_handler *)udev->dev.class_data[USBD_VIDEO_INTERFACE];

    uint16_t i = 0U;

    usbd_fifo_flush(udev, VIDEO_IN_EP);

    /* get packet number per jpeg frame */
    if (tx_enable_flag == 0) {
        if (video->jpeg_encode_done == 1) {
            tx_enable_flag = 1;
            picture_pos = 0;
            packet_count = 0;

            video->jpeg_fram_size = jpeg_data_len(udev);

            video->packets_in_frame = (video->jpeg_fram_size / ((uint16_t)VIDEO_PACKET_SIZE - 2)) + 1;
            last_packet_size = (video->jpeg_fram_size - ((video->packets_in_frame - 1) * ((uint16_t)VIDEO_PACKET_SIZE - 2)) + 2);

            video->jpeg_encode_done = 0;
        }
    }

    packet[0] = video->payload_header[0];
    packet[1] = video->payload_header[1];
    
    /* prepare the packet data to send */
    for (i = 2; i < VIDEO_PACKET_SIZE; i++) {
        packet[i] = video->frame_ptr[picture_pos];
        picture_pos ++;
    }

    /* send the jpeg frame data via USB interface */
    if ((video->play_status == PLAY_STATUS_STREAMING) || (video->play_status == PLAY_STATUS_READY)) {
        if (packet_count < (video->packets_in_frame - 1)) {
            usbd_ep_send(udev, VIDEO_IN_EP, packet, VIDEO_PACKET_SIZE);
            packet_count++;
        } else {
            usbd_ep_send(udev, VIDEO_IN_EP, packet, last_packet_size);

            video->jpeg_encode_done = 1;

            if (video->payload_header[1] == 1) {
                video->payload_header[1] = 0 ;
            } else {
                video->payload_header[1] = 1 ;
            } 
        
            video->play_status = PLAY_STATUS_READY;
            
            if (video->jpeg_fram_size > 2) {
                tx_enable_flag = 0;
                packet_count = 0;
                picture_pos = 0;
            }
            
            /* get the jpeg frame data via DCI interface */
            dci_stop();
            dci_dma_config((uint32_t)(bufptr_datain), BUF_SIZE);
            dci_start();
        }
    } else {
        packet_count = 0;
        picture_pos = 0;
    }


    return USBD_OK;
}

/*!
    \brief      handles the VIDEO OUT data stage
    \param[in]  udev: pointer to USB device instance
    \param[in]  ep_num: endpoint number
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t video_data_out (usb_dev *udev, uint8_t ep_num)
{
    return USBD_OK;
}

/*!
    \brief      handles SOF event
    \param[in]  udev: pointer to USB device instance
    \param[out] none
    \retval     USB device operation status
*/
static uint8_t usbd_video_sof (usb_dev *udev)
{
    uint8_t payload[2] = {0x02, 0x00};

    usbd_video_handler *video = (usbd_video_handler *)udev->dev.class_data[USBD_VIDEO_INTERFACE];

    if ((PLAY_STATUS_READY == video->play_status)) {

        usbd_ep_send (udev, VIDEO_IN_EP, payload, 2);

        video->play_status = PLAY_STATUS_STREAMING;
    }

    return USBD_OK;
}

/*!
    \brief      get jpeg frame length
    \param[in]  none
    \param[out] none
    \retval     return frame length
*/
static uint16_t jpeg_data_len(usb_dev *udev)
{
    uint16_t len = 0, i = 0, real_len = 0;

    usbd_video_handler *video = (usbd_video_handler *)udev->dev.class_data[USBD_VIDEO_INTERFACE];

    len = 4 * BUF_SIZE;
    video->frame_ptr = (uint8_t *)(bufptr_dataout);

    for (i = 0;i < len; i++) {
        if ((video->frame_ptr[i] == 0xFF) && (video->frame_ptr[i + 1] == 0xD9)) {
            real_len = i + 2;
            break;
        }
    }

    return real_len;
}
