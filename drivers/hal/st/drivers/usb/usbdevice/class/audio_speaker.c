/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        audio_speaker.c
 *
 * @brief       This file provides functions for audio.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <arch_interrupt.h>
#include <os_task.h>
#include <os_list.h>
#include <drv_cfg.h>

#include "usb/usb_device.h"
#include "audio.h"

#define DRV_EXT_TAG "usbd.audio.speaker"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>

#define AUDIO_SAMPLERATE 16000
#define AUDIO_CHANNEL    1
#define RESOLUTION_BITS  16

#define RESOLUTION_BYTE (RESOLUTION_BITS / 8)
#define AUDIO_PER_MS_SZ ((AUDIO_SAMPLERATE * AUDIO_CHANNEL * RESOLUTION_BYTE) / 1000)
#define AUDIO_BUFFER_SZ (AUDIO_PER_MS_SZ * 20) /* 20ms */

#if defined(OS_USBD_SPEAKER_DEVICE_NAME)
#define SPEAKER_DEVICE_NAME OS_USBD_SPEAKER_DEVICE_NAME
#else
#define SPEAKER_DEVICE_NAME "sound0"
#endif

#define EVENT_AUDIO_START (1 << 0)
#define EVENT_AUDIO_STOP  (1 << 1)
#define EVENT_AUDIO_DATA  (1 << 2)

/* Uac speaker descriptor define */

#define UAC_CS_INTERFACE 0x24
#define UAC_CS_ENDPOINT  0x25

#define UAC_MAX_PACKET_SIZE    64
#define UAC_EP_MAX_PACKET_SIZE 32
#define UAC_CHANNEL_NUM        AUDIO_CHANNEL
#define UAC_INTR_NUM           1
#define UAC_CH_NUM             1
#define UAC_FORMAT_NUM         1

struct uac_ac_descriptor
{
#ifdef OS_USB_DEVICE_COMPOSITE
    struct uiad_descriptor iad_desc;
#endif
    struct uinterface_descriptor intf_desc;
    DECLARE_UAC_AC_HEADER_DESCRIPTOR(UAC_INTR_NUM) hdr_desc;
    struct uac_input_terminal_descriptor   it_desc;
    struct uac1_output_terminal_descriptor ot_desc;
#if UAC_USE_FEATURE_UNIT
    DECLARE_UAC_FEATURE_UNIT_DESCRIPTOR(UAC_CH_NUM) feature_unit_desc;
#endif
};

struct uac_as_descriptor
{
    struct uinterface_descriptor     intf_desc;
    struct uac1_as_header_descriptor hdr_desc;
    DECLARE_UAC_FORMAT_TYPE_I_DISCRETE_DESC(UAC_FORMAT_NUM) format_type_desc;
    struct uendpoint_descriptor        ep_desc;
    struct uac_iso_endpoint_descriptor as_ep_desc;
};

struct uac_audio_speaker
{
    os_device_t *dev;
    os_event_t  *event;
    os_uint8_t   open_count;

    os_uint8_t  *buffer;
    os_uint32_t  buffer_index;

    uep_t ep;
};
static struct uac_audio_speaker speaker;

OS_ALIGN(4)
static struct udevice_descriptor dev_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength; */
    USB_DESC_TYPE_DEVICE,     /* type; */
    USB_BCD_VERSION,          /* bcdUSB; */
    USB_CLASS_DEVICE,         /* bDeviceClass; */
    0x00,                     /* bDeviceSubClass; */
    0x00,                     /* bDeviceProtocol; */
    UAC_MAX_PACKET_SIZE,      /* bMaxPacketSize0; */
    _VENDOR_ID,               /* idVendor; */
    _PRODUCT_ID,              /* idProduct; */
    USB_BCD_DEVICE,           /* bcdDevice; */
    USB_STRING_MANU_INDEX,    /* iManufacturer; */
    USB_STRING_PRODUCT_INDEX, /* iProduct; */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber;Unused. */
    USB_DYNAMIC,              /* bNumConfigurations; */
};

#if 0
/* FS and HS needed */
OS_ALIGN(4)
static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),          /* bLength */
    USB_DESC_TYPE_DEVICEQUALIFIER,  /* bDescriptorType */
    0x0200,                         /* bcdUSB */
    USB_CLASS_AUDIO,                /* bDeviceClass */
    0x00,                           /* bDeviceSubClass */
    0x00,                           /* bDeviceProtocol */
    64,                             /* bMaxPacketSize0 */
    0x01,                           /* bNumConfigurations */
    0,
};
#endif

OS_ALIGN(4)
const static char *_ustring[] =
{
    "Language",
    "Team.",
    "Speaker",
    "32021919830108",
    "Configuration",
    "Interface",
};

OS_ALIGN(4)
static struct uac_ac_descriptor ac_desc =
{
#ifdef OS_USB_DEVICE_COMPOSITE
    /* Interface Association Descriptor */
    {
        USB_DESC_LENGTH_IAD,
        USB_DESC_TYPE_IAD,
        USB_DYNAMIC,
        0x02,
        USB_CLASS_AUDIO,
        USB_SUBCLASS_AUDIOSTREAMING,
        0x00,
        0x00,
    },
#endif
    /* Interface Descriptor */
    {
        USB_DESC_LENGTH_INTERFACE,
        USB_DESC_TYPE_INTERFACE,
        USB_DYNAMIC,
        0x00,
        0x00,
        USB_CLASS_AUDIO,
        USB_SUBCLASS_AUDIOCONTROL,
        0x00,
        0x00,
    },
    /* Header Descriptor */
    {
        UAC_DT_AC_HEADER_SIZE(UAC_INTR_NUM),
        UAC_CS_INTERFACE,
        UAC_HEADER,
        0x0100, /* Version: 1.00 */
        0x0027, /* Total length: 39 */
        0x01,   /* Total number of interfaces: 1 */
        {0x01}, /* Interface number: 1 */
    },
    /* Input Terminal Descriptor */
    {
        UAC_DT_INPUT_TERMINAL_SIZE,
        UAC_CS_INTERFACE,
        UAC_INPUT_TERMINAL,
        0x01,   /* Terminal ID: 1 */
        0x0101, /* Terminal Type: USB Streaming (0x0101) */
        0x00,   /* Assoc Terminal: 0 */
        0x01,   /* Number Channels: 1 */
        0x0000, /* Channel Config: 0x0000 */
        0x00,   /* Channel Names: 0 */
        0x00,   /* Terminal: 0 */
    },
    /* Output Terminal Descriptor */
    {
        UAC_DT_OUTPUT_TERMINAL_SIZE,
        UAC_CS_INTERFACE,
        UAC_OUTPUT_TERMINAL,
        0x02,   /* Terminal ID: 2 */
        0x0302, /* Terminal Type: Headphones (0x0302) */
        0x00,   /* Assoc Terminal: 0 */
        0x01,   /* Source ID: 1 */
        0x00,   /* Terminal: 0 */
    },
#if UAC_USE_FEATURE_UNIT
    /* Feature unit Descriptor */
    {
        UAC_DT_FEATURE_UNIT_SIZE(UAC_CH_NUM),
        UAC_CS_INTERFACE,
        UAC_FEATURE_UNIT,
        0x02,
        0x0101,
        0x00,
        0x01,
    },
#endif
};

OS_ALIGN(4)
static struct uinterface_descriptor as_desc0 =
{
    USB_DESC_LENGTH_INTERFACE,
    USB_DESC_TYPE_INTERFACE,
    USB_DYNAMIC,
    0x00,
    0x00,
    USB_CLASS_AUDIO,
    USB_SUBCLASS_AUDIOSTREAMING,
    0x00,
    0x00,
};

OS_ALIGN(4)
static struct uac_as_descriptor as_desc =
{
    /* Interface Descriptor */
    {
        USB_DESC_LENGTH_INTERFACE,
        USB_DESC_TYPE_INTERFACE,
        USB_DYNAMIC,
        0x01,
        0x01,
        USB_CLASS_AUDIO,
        USB_SUBCLASS_AUDIOSTREAMING,
        0x00,
        0x00,
    },
    /* General AS Descriptor */
    {
        UAC_DT_AS_HEADER_SIZE,
        UAC_CS_INTERFACE,
        UAC_AS_GENERAL,
        0x01, /* Terminal ID: 1 */
        0x01, /* Interface delay in frames: 1 */
        UAC_FORMAT_TYPE_I_PCM,
    },
    /* Format type i Descriptor */
    {
        UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(UAC_FORMAT_NUM),
        UAC_CS_INTERFACE,
        UAC_FORMAT_TYPE,
        UAC_FORMAT_TYPE_I,
        UAC_CHANNEL_NUM,
        2, /* Subframe Size: 2 */
        RESOLUTION_BITS,
        0x01, /* Samples Frequence Type: 1 */
        {0},  /* Samples Frequence */
    },
    /* Endpoint Descriptor */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_OUT,
        USB_EP_ATTR_ISOC,
        UAC_EP_MAX_PACKET_SIZE,
        0x01,
    },
    /* AS Endpoint Descriptor */
    {
        UAC_ISO_ENDPOINT_DESC_SIZE,
        UAC_CS_ENDPOINT,
        UAC_MS_GENERAL,
    },
};

void speaker_entry(void *parameter)
{
    struct os_audio_caps caps = {0};
    os_uint32_t          e, index;

    speaker.buffer = os_calloc(1, AUDIO_BUFFER_SZ);
    if (speaker.buffer == OS_NULL)
    {
        LOG_E(DBG_TAG,"malloc failed");
        goto __exit;
    }

    speaker.dev = os_device_find(SPEAKER_DEVICE_NAME);
    if (speaker.dev == OS_NULL)
    {
        LOG_E(DBG_TAG,"can't find device:%s", SPEAKER_DEVICE_NAME);
        goto __exit;
    }

    while (1)
    {
        if (os_event_recv(speaker.event,
                          EVENT_AUDIO_START | EVENT_AUDIO_STOP,
                          OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                          1000,
                          &e) != OS_EOK)
        {
            continue;
        }
        if (speaker.open_count == 0)
        {
            continue;
        }
        LOG_EXT_D("play start");

        os_device_open(speaker.dev, OS_DEVICE_OFLAG_WRONLY);

        caps.main_type               = AUDIO_TYPE_OUTPUT;
        caps.sub_type                = AUDIO_DSP_PARAM;
        caps.udata.config.samplerate = AUDIO_SAMPLERATE;
        caps.udata.config.channels   = AUDIO_CHANNEL;
        os_device_control(speaker.dev, AUDIO_CTL_CONFIGURE, &caps);

        while (1)
        {
            if (os_event_recv(speaker.event,
                              EVENT_AUDIO_DATA | EVENT_AUDIO_STOP,
                              OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                              1000,
                              &e) != OS_EOK)
            {
                if (speaker.open_count > 0)
                    continue;
                else
                    break;
            }
            if (e & EVENT_AUDIO_DATA)
            {
                index = (speaker.buffer_index >= AUDIO_BUFFER_SZ / 2) ? 0 : (AUDIO_BUFFER_SZ / 2);
                os_device_write_nonblock(speaker.dev, 0, speaker.buffer + index, AUDIO_BUFFER_SZ / 2);
            }
            else if (e & EVENT_AUDIO_STOP)
            {
                break;
            }
        }
        LOG_EXT_D("play stop");
        os_device_close(speaker.dev);
    }

__exit:
    if (speaker.buffer)
        os_free(speaker.buffer);
}

static os_err_t _audio_start(ufunction_t func)
{
    speaker.ep->request.buffer   = speaker.buffer;
    speaker.ep->request.size     = UAC_EP_MAX_PACKET_SIZE;
    speaker.ep->request.req_type = UIO_REQUEST_READ_FULL;
    os_usbd_io_request(func->device, speaker.ep, &speaker.ep->request);

    speaker.open_count++;
    os_event_send(speaker.event, EVENT_AUDIO_START);

    return 0;
}

static os_err_t _audio_stop(ufunction_t func)
{
    speaker.open_count--;
    os_event_send(speaker.event, EVENT_AUDIO_STOP);
    return 0;
}

static os_err_t _ep_data_handler(ufunction_t func, os_size_t size)
{
    OS_ASSERT(func != OS_NULL);
    LOG_EXT_D("_ep_data_handler");

    speaker.ep->request.buffer   = speaker.buffer + speaker.buffer_index;
    speaker.ep->request.size     = UAC_EP_MAX_PACKET_SIZE;
    speaker.ep->request.req_type = UIO_REQUEST_READ_FULL;
    os_usbd_io_request(func->device, speaker.ep, &speaker.ep->request);

    speaker.buffer_index += UAC_EP_MAX_PACKET_SIZE;
    if (speaker.buffer_index >= AUDIO_BUFFER_SZ)
    {
        speaker.buffer_index = 0;
        os_event_send(speaker.event, EVENT_AUDIO_DATA);
    }
    else if (speaker.buffer_index == AUDIO_BUFFER_SZ / 2)
    {
        os_event_send(speaker.event, EVENT_AUDIO_DATA);
    }

    return OS_EOK;
}

static os_err_t _interface_as_handler(ufunction_t func, ureq_t setup)
{
    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    LOG_EXT_D("_interface_as_handler");

    if ((setup->request_type & USB_REQ_TYPE_MASK) == USB_REQ_TYPE_STANDARD)
    {
        switch (setup->bRequest)
        {
        case USB_REQ_GET_INTERFACE:
            break;
        case USB_REQ_SET_INTERFACE:
            LOG_EXT_D("set interface handler");
            if (setup->wValue == 1)
            {
                _audio_start(func);
            }
            else if (setup->wValue == 0)
            {
                _audio_stop(func);
            }
            break;
        default:
            LOG_EXT_D("unknown uac request 0x%x", setup->bRequest);
            return OS_ERROR;
        }
    }

    return OS_EOK;
}

static os_err_t _function_enable(ufunction_t func)
{
    OS_ASSERT(func != OS_NULL);

    LOG_EXT_D("uac function enable");

    return OS_EOK;
}

static os_err_t _function_disable(ufunction_t func)
{
    OS_ASSERT(func != OS_NULL);

    LOG_EXT_D("uac function disable");
    _audio_stop(func);
    return OS_EOK;
}

static struct ufunction_ops ops =
{
    _function_enable,
    _function_disable,
    OS_NULL,
};

static os_err_t _uac_descriptor_config(struct uac_ac_descriptor *ac,
                                       os_uint8_t                cintf_nr,
                                       struct uac_as_descriptor *as,
                                       os_uint8_t                sintf_nr)
{
    ac->hdr_desc.baInterfaceNr[0] = sintf_nr;
#ifdef OS_USB_DEVICE_COMPOSITE
    ac->iad_desc.bFirstInterface = cintf_nr;
#endif

    return OS_EOK;
}

static os_err_t _uac_samplerate_config(struct uac_as_descriptor *as, os_uint32_t samplerate)
{
    as->format_type_desc.tSamFreq[0][2] = samplerate >> 16 & 0xff;
    as->format_type_desc.tSamFreq[0][1] = samplerate >> 8 & 0xff;
    as->format_type_desc.tSamFreq[0][0] = samplerate & 0xff;
    return OS_EOK;
}

ufunction_t os_usbd_function_uac_speaker_create(udevice_t device)
{
    ufunction_t               func;
    uintf_t                   intf_ac, intf_as;
    ualtsetting_t             setting_as0;
    ualtsetting_t             setting_ac, setting_as;
    struct uac_as_descriptor *as_desc_t;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Set usb device string description */
    os_usbd_device_set_string(device, _ustring);

    /* Create a uac function */
    func = os_usbd_function_new(device, &dev_desc, &ops);
    /* not support HS */
    // os_usbd_device_set_qualifier(device, &dev_qualifier);

    /* Create interface */
    intf_ac = os_usbd_interface_new(device, OS_NULL);
    intf_as = os_usbd_interface_new(device, _interface_as_handler);

    /* Create alternate setting */
    setting_ac  = os_usbd_altsetting_new(sizeof(struct uac_ac_descriptor));
    setting_as0 = os_usbd_altsetting_new(sizeof(struct uinterface_descriptor));
    setting_as  = os_usbd_altsetting_new(sizeof(struct uac_as_descriptor));
    /* Config desc in alternate setting */
    os_usbd_altsetting_config_descriptor(setting_ac, &ac_desc, (os_off_t) & ((struct uac_ac_descriptor *)0)->intf_desc);
    os_usbd_altsetting_config_descriptor(setting_as0, &as_desc0, 0);
    os_usbd_altsetting_config_descriptor(setting_as, &as_desc, (os_off_t) & ((struct uac_as_descriptor *)0)->intf_desc);
    /* Configure the uac interface descriptor */
    _uac_descriptor_config(setting_ac->desc, intf_ac->intf_num, setting_as->desc, intf_as->intf_num);
    _uac_samplerate_config(setting_as->desc, AUDIO_SAMPLERATE);

    /* Create endpoint */
    as_desc_t  = (struct uac_as_descriptor *)setting_as->desc;
    speaker.ep = os_usbd_endpoint_new(&as_desc_t->ep_desc, _ep_data_handler);

    /* Add the endpoint to the alternate setting */
    os_usbd_altsetting_add_endpoint(setting_as, speaker.ep);

    /* Add the alternate setting to the interface, then set default setting of the interface */
    os_usbd_interface_add_altsetting(intf_ac, setting_ac);
    os_usbd_set_altsetting(intf_ac, 0);
    os_usbd_interface_add_altsetting(intf_as, setting_as0);
    os_usbd_interface_add_altsetting(intf_as, setting_as);
    os_usbd_set_altsetting(intf_as, 0);

    /* Add the interface to the uac function */
    os_usbd_function_add_interface(func, intf_ac);
    os_usbd_function_add_interface(func, intf_as);

    return func;
}

int audio_speaker_init(void)
{
    os_task_t *speaker_tid;
    speaker.event = os_event_create("speaker_event", OS_IPC_FLAG_FIFO);

    speaker_tid = os_task_create("speaker_task", speaker_entry, OS_NULL, 1024, 5, 10);

    if (speaker_tid != OS_NULL)
        os_task_startup(speaker_tid);
    return OS_EOK;
}
OS_CMPOENT_INIT(audio_speaker_init);

/* Register uac class */
static struct udclass uac_speaker_class = {.os_usbd_function_create = os_usbd_function_uac_speaker_create};

int os_usbd_uac_speaker_class_register(void)
{
    os_usbd_class_register(&uac_speaker_class);
    return 0;
}
OS_PREV_INIT(os_usbd_uac_speaker_class_register);
