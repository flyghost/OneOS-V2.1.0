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
 * @file        mstorage.c
 *
 * @brief       This file provides functions for mstorage.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <os_list.h>
#include "usb/usb_device.h"
#include "mstorage.h"

#ifdef OS_USB_DEVICE_MSTORAGE

enum STAT
{
    STAT_CBW,
    STAT_CMD,
    STAT_CSW,
    STAT_RECEIVE,
    STAT_SEND,
};

typedef enum
{
    FIXED,
    COUNT,
    BLOCK_COUNT,
} CB_SIZE_TYPE;

typedef enum
{
    DIR_IN,
    DIR_OUT,
    DIR_NONE,
} CB_DIR;

typedef os_size_t (*cbw_handler)(ufunction_t func, ustorage_cbw_t cbw);

struct scsi_cmd
{
    os_uint16_t  cmd;
    cbw_handler  handler;
    os_size_t    cmd_len;
    CB_SIZE_TYPE type;
    os_size_t    data_size;
    CB_DIR       dir;
};

struct mstorage
{
    struct ustorage_csw           csw_response;
    uep_t                         ep_in;
    uep_t                         ep_out;
    int                           status;
    os_uint32_t                   cb_data_size;
    os_device_t                  *disk;
    os_uint32_t                   block;
    os_int32_t                    count;
    os_int32_t                    size;
    struct scsi_cmd              *processing;
    struct os_device_blk_geometry geometry;
};

OS_ALIGN(4)
static struct udevice_descriptor dev_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength; */
    USB_DESC_TYPE_DEVICE,     /* type; */
    USB_BCD_VERSION,          /* bcdUSB; */
    USB_CLASS_MASS_STORAGE,   /* bDeviceClass; */
    0x06,                     /* bDeviceSubClass; */
    0x50,                     /* bDeviceProtocol; */
    0x40,                     /* bMaxPacketSize0; */
    _VENDOR_ID,               /* idVendor; */
    _PRODUCT_ID,              /* idProduct; */
    USB_BCD_DEVICE,           /* bcdDevice; */
    USB_STRING_MANU_INDEX,    /* iManufacturer; */
    USB_STRING_PRODUCT_INDEX, /* iProduct; */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber; */
    USB_DYNAMIC,              /* bNumConfigurations; */
};

// FS and HS needed
OS_ALIGN(4)
static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),         /* bLength */
    USB_DESC_TYPE_DEVICEQUALIFIER, /* bDescriptorType */
    0x0200,                        /* bcdUSB */
    USB_CLASS_MASS_STORAGE,        /* bDeviceClass */
    0x06,                          /* bDeviceSubClass */
    0x50,                          /* bDeviceProtocol */
    64,                            /* bMaxPacketSize0 */
    0x01,                          /* bNumConfigurations */
    0,
};

OS_ALIGN(4)
const static struct umass_descriptor _mass_desc =
{
#ifdef OS_USB_DEVICE_COMPOSITE
    /* Interface Association Descriptor */
    {
        USB_DESC_LENGTH_IAD,
        USB_DESC_TYPE_IAD,
        USB_DYNAMIC,
        0x01,
        USB_CLASS_MASS_STORAGE,
        0x06,
        0x50,
        0x00,
    },
#endif
    {
        USB_DESC_LENGTH_INTERFACE, /* bLength; */
        USB_DESC_TYPE_INTERFACE,   /* type; */
        USB_DYNAMIC,               /* bInterfaceNumber; */
        0x00,                      /* bAlternateSetting; */
        0x02,                      /* bNumEndpoints */
        USB_CLASS_MASS_STORAGE,    /* bInterfaceClass; */
        0x06,                      /* bInterfaceSubClass; */
        0x50,                      /* bInterfaceProtocol; */
        0x00,                      /* iInterface; */
    },

    {
        USB_DESC_LENGTH_ENDPOINT,  /* bLength; */
        USB_DESC_TYPE_ENDPOINT,    /* type; */
        USB_DYNAMIC | USB_DIR_OUT, /* bEndpointAddress; */
        USB_EP_ATTR_BULK,          /* bmAttributes; */
        USB_DYNAMIC,               /* wMaxPacketSize; */
        0x00,                      /* bInterval; */
    },

    {
        USB_DESC_LENGTH_ENDPOINT, /* bLength; */
        USB_DESC_TYPE_ENDPOINT,   /* type; */
        USB_DYNAMIC | USB_DIR_IN, /* bEndpointAddress; */
        USB_EP_ATTR_BULK,         /* bmAttributes; */
        USB_DYNAMIC,              /* wMaxPacketSize; */
        0x00,                     /* bInterval; */
    },
};

OS_ALIGN(4)
const static char *_ustring[] =
{
    "Language",
    "OS Team.",
    "OS Mass Storage",
    "320219198301",
    "Configuration",
    "Interface",
};

static os_size_t _test_unit_ready(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _request_sense(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _inquiry_cmd(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _allow_removal(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _start_stop(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _mode_sense_6(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _read_capacities(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _read_capacity(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _read_10(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _write_10(ufunction_t func, ustorage_cbw_t cbw);
static os_size_t _verify_10(ufunction_t func, ustorage_cbw_t cbw);

OS_ALIGN(4)
static struct scsi_cmd cmd_data[] =
{
    {SCSI_TEST_UNIT_READY, _test_unit_ready, 6, FIXED, 0, DIR_NONE},
    {SCSI_REQUEST_SENSE, _request_sense, 6, COUNT, 0, DIR_IN},
    {SCSI_INQUIRY_CMD, _inquiry_cmd, 6, COUNT, 0, DIR_IN},
    {SCSI_ALLOW_REMOVAL, _allow_removal, 6, FIXED, 0, DIR_NONE},
    {SCSI_MODE_SENSE_6, _mode_sense_6, 6, COUNT, 0, DIR_IN},
    {SCSI_START_STOP, _start_stop, 6, FIXED, 0, DIR_NONE},
    {SCSI_READ_CAPACITIES, _read_capacities, 10, COUNT, 0, DIR_NONE},
    {SCSI_READ_CAPACITY, _read_capacity, 10, FIXED, 8, DIR_IN},
    {SCSI_READ_10, _read_10, 10, BLOCK_COUNT, 0, DIR_IN},
    {SCSI_WRITE_10, _write_10, 10, BLOCK_COUNT, 0, DIR_OUT},
    {SCSI_VERIFY_10, _verify_10, 10, FIXED, 0, DIR_NONE},
};

static void _send_status(ufunction_t func)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_send_status\r\n"));

    data                          = (struct mstorage *)func->user_data;
    data->ep_in->request.buffer   = (os_uint8_t *)&data->csw_response;
    data->ep_in->request.size     = SIZEOF_CSW;
    data->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
    data->status = STAT_CSW;
}

static os_size_t _test_unit_ready(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_test_unit_ready\r\n"));

    data                      = (struct mstorage *)func->user_data;
    data->csw_response.status = 0;

    return 0;
}

static os_size_t _allow_removal(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_allow_removal\r\n"));

    data                      = (struct mstorage *)func->user_data;
    data->csw_response.status = 0;

    return 0;
}

static os_size_t _inquiry_cmd(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;
    os_uint8_t      *buf;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_inquiry_cmd\r\n"));

    data = (struct mstorage *)func->user_data;
    buf  = data->ep_in->buffer;

    *(os_uint32_t *)&buf[0] = 0x0 | (0x80 << 8);
    *(os_uint32_t *)&buf[4] = 31;

    memset(&buf[8], 0x20, 28);
    memcpy(&buf[8], "OS", 2);
    memcpy(&buf[16], "USB Disk", 8);

    data->cb_data_size            = MIN(data->cb_data_size, SIZEOF_INQUIRY_CMD);
    data->ep_in->request.buffer   = buf;
    data->ep_in->request.size     = data->cb_data_size;
    data->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
    data->status = STAT_CMD;

    return data->cb_data_size;
}

static os_size_t _request_sense(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage           *data;
    struct request_sense_data *buf;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_request_sense\r\n"));

    data = (struct mstorage *)func->user_data;
    buf  = (struct request_sense_data *)data->ep_in->buffer;

    buf->ErrorCode                    = 0x70;
    buf->Valid                        = 0;
    buf->SenseKey                     = 2;
    buf->Information[0]               = 0;
    buf->Information[1]               = 0;
    buf->Information[2]               = 0;
    buf->Information[3]               = 0;
    buf->AdditionalSenseLength        = 0x0a;
    buf->AdditionalSenseCode          = 0x3a;
    buf->AdditionalSenseCodeQualifier = 0;

    data->cb_data_size            = MIN(data->cb_data_size, SIZEOF_REQUEST_SENSE);
    data->ep_in->request.buffer   = (os_uint8_t *)data->ep_in->buffer;
    data->ep_in->request.size     = data->cb_data_size;
    data->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
    data->status = STAT_CMD;

    return data->cb_data_size;
}

static os_size_t _mode_sense_6(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;
    os_uint8_t      *buf;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_mode_sense_6\r\n"));

    data   = (struct mstorage *)func->user_data;
    buf    = data->ep_in->buffer;
    buf[0] = 3;
    buf[1] = 0;
    buf[2] = 0;
    buf[3] = 0;

    data->cb_data_size            = MIN(data->cb_data_size, SIZEOF_MODE_SENSE_6);
    data->ep_in->request.buffer   = buf;
    data->ep_in->request.size     = data->cb_data_size;
    data->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
    data->status = STAT_CMD;

    return data->cb_data_size;
}

static os_size_t _read_capacities(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;
    os_uint8_t      *buf;
    os_uint32_t      sector_count, sector_size;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_read_capacities\r\n"));

    data         = (struct mstorage *)func->user_data;
    buf          = data->ep_in->buffer;
    sector_count = data->geometry.sector_count;
    sector_size  = data->geometry.bytes_per_sector;

    *(os_uint32_t *)&buf[0] = 0x08000000;
    buf[4]                  = sector_count >> 24;
    buf[5]                  = 0xff & (sector_count >> 16);
    buf[6]                  = 0xff & (sector_count >> 8);
    buf[7]                  = 0xff & (sector_count);
    buf[8]                  = 0x02;
    buf[9]                  = 0xff & (sector_size >> 16);
    buf[10]                 = 0xff & (sector_size >> 8);
    buf[11]                 = 0xff & sector_size;

    data->cb_data_size            = MIN(data->cb_data_size, SIZEOF_READ_CAPACITIES);
    data->ep_in->request.buffer   = buf;
    data->ep_in->request.size     = data->cb_data_size;
    data->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
    data->status = STAT_CMD;

    return data->cb_data_size;
}

static os_size_t _read_capacity(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    os_uint8_t *buf;
    os_uint32_t sector_count, sector_size;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_read_capacity\r\n"));

    data         = (struct mstorage *)func->user_data;
    buf          = data->ep_in->buffer;
    sector_count = data->geometry.sector_count;
    sector_size  = data->geometry.bytes_per_sector;

    buf[0] = sector_count >> 24;
    buf[1] = 0xff & (sector_count >> 16);
    buf[2] = 0xff & (sector_count >> 8);
    buf[3] = 0xff & (sector_count);
    buf[4] = 0x0;
    buf[5] = 0xff & (sector_size >> 16);
    buf[6] = 0xff & (sector_size >> 8);
    buf[7] = 0xff & sector_size;

    data->cb_data_size            = MIN(data->cb_data_size, SIZEOF_READ_CAPACITY);
    data->ep_in->request.buffer   = buf;
    data->ep_in->request.size     = data->cb_data_size;
    data->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
    data->status = STAT_CMD;

    return data->cb_data_size;
}

static os_size_t _read_10(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;
    os_size_t        size;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    data        = (struct mstorage *)func->user_data;
    data->block = cbw->cb[2] << 24 | cbw->cb[3] << 16 | cbw->cb[4] << 8 | cbw->cb[5] << 0;
    data->count = cbw->cb[7] << 8 | cbw->cb[8] << 0;

    OS_ASSERT(data->count < data->geometry.sector_count);

    data->csw_response.data_reside = data->cb_data_size;
    size                           = os_device_read_nonblock(data->disk, data->block, data->ep_in->buffer, 1);
    if (size == 0)
    {
        os_kprintf("read data error\r\n");
    }

    data->ep_in->request.buffer   = data->ep_in->buffer;
    data->ep_in->request.size     = data->geometry.bytes_per_sector;
    data->ep_in->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
    data->status = STAT_SEND;

    return data->geometry.bytes_per_sector;
}

static os_size_t _write_10(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    data = (struct mstorage *)func->user_data;

    data->block                    = cbw->cb[2] << 24 | cbw->cb[3] << 16 | cbw->cb[4] << 8 | cbw->cb[5] << 0;
    data->count                    = cbw->cb[7] << 8 | cbw->cb[8];
    data->csw_response.data_reside = cbw->xfer_len;
    data->size                     = data->count * data->geometry.bytes_per_sector;

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_write_10 count 0x%x block 0x%x 0x%x\r\n",
                                data->count, data->block, data->geometry.sector_count));

    data->csw_response.data_reside = data->cb_data_size;

    data->ep_out->request.buffer   = data->ep_out->buffer;
    data->ep_out->request.size     = data->geometry.bytes_per_sector;
    data->ep_out->request.req_type = UIO_REQUEST_READ_FULL;
    os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);
    data->status = STAT_RECEIVE;

    return data->geometry.bytes_per_sector;
}

static os_size_t _verify_10(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_verify_10\r\n"));

    data                      = (struct mstorage *)func->user_data;
    data->csw_response.status = 0;

    return 0;
}

static os_size_t _start_stop(ufunction_t func, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_start_stop\r\n"));

    data                      = (struct mstorage *)func->user_data;
    data->csw_response.status = 0;

    return 0;
}

static os_err_t _ep_in_handler(ufunction_t func, os_size_t size)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_ep_in_handler\r\n"));

    data = (struct mstorage *)func->user_data;

    switch (data->status)
    {
    case STAT_CSW:
        if (data->ep_in->request.size != SIZEOF_CSW)
        {
            os_kprintf("Size of csw command error\r\n");
            os_usbd_ep_set_stall(func->device, data->ep_in);
        }
        else
        {
            OS_DEBUG_LOG(OS_DEBUG_USB, ("return to cbw status\r\n"));
            data->ep_out->request.buffer   = data->ep_out->buffer;
            data->ep_out->request.size     = SIZEOF_CBW;
            data->ep_out->request.req_type = UIO_REQUEST_READ_FULL;
            os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);
            data->status = STAT_CBW;
        }
        break;
    case STAT_CMD:
        if (data->csw_response.data_reside == 0xFF)
        {
            data->csw_response.data_reside = 0;
        }
        else
        {
            data->csw_response.data_reside -= data->ep_in->request.size;
            if (data->csw_response.data_reside != 0)
            {
                OS_DEBUG_LOG(
                    OS_DEBUG_USB,
                    ("data_reside %d, request %d\r\n", data->csw_response.data_reside, data->ep_in->request.size));
                if (data->processing->dir == DIR_OUT)
                {
                    os_usbd_ep_set_stall(func->device, data->ep_out);
                }
                else
                {
                    os_usbd_ep_set_stall(func->device, data->ep_in);
                }
                data->csw_response.data_reside = 0;
            }
        }
        _send_status(func);
        break;
    case STAT_SEND:
        data->csw_response.data_reside -= data->ep_in->request.size;
        data->count--;
        data->block++;
        if (data->count > 0 && data->csw_response.data_reside > 0)
        {
            if (os_device_read_nonblock(data->disk, data->block, data->ep_in->buffer, 1) == 0)
            {
                os_kprintf("disk read error\r\n");
                os_usbd_ep_set_stall(func->device, data->ep_in);
                return OS_ERROR;
            }

            data->ep_in->request.buffer   = data->ep_in->buffer;
            data->ep_in->request.size     = data->geometry.bytes_per_sector;
            data->ep_in->request.req_type = UIO_REQUEST_WRITE;
            os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);
        }
        else
        {
            _send_status(func);
        }
        break;
    }

    return OS_EOK;
}

#ifdef MASS_CBW_DUMP
static void cbw_dump(struct ustorage_cbw *cbw)
{
    OS_ASSERT(cbw != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("signature 0x%x\r\n", cbw->signature));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("tag 0x%x\r\n", cbw->tag));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("xfer_len 0x%x\r\n", cbw->xfer_len));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("dflags 0x%x\r\n", cbw->dflags));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("lun 0x%x\r\n", cbw->lun));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("cb_len 0x%x\r\n", cbw->cb_len));
    OS_DEBUG_LOG(OS_DEBUG_USB, ("cb[0] 0x%x\r\n", cbw->cb[0]));
}
#endif

static struct scsi_cmd *_find_cbw_command(os_uint16_t cmd)
{
    int i;

    for (i = 0; i < sizeof(cmd_data) / sizeof(struct scsi_cmd); i++)
    {
        if (cmd_data[i].cmd == cmd)
            return &cmd_data[i];
    }

    return OS_NULL;
}

static void _cb_len_calc(ufunction_t func, struct scsi_cmd *cmd, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(cmd != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);

    data = (struct mstorage *)func->user_data;
    if (cmd->cmd_len == 6)
    {
        switch (cmd->type)
        {
        case COUNT:
            data->cb_data_size = cbw->cb[4];
            break;
        case BLOCK_COUNT:
            data->cb_data_size = cbw->cb[4] * data->geometry.bytes_per_sector;
            break;
        case FIXED:
            data->cb_data_size = cmd->data_size;
            break;
        default:
            break;
        }
    }
    else if (cmd->cmd_len == 10)
    {
        switch (cmd->type)
        {
        case COUNT:
            data->cb_data_size = cbw->cb[7] << 8 | cbw->cb[8];
            break;
        case BLOCK_COUNT:
            data->cb_data_size = (cbw->cb[7] << 8 | cbw->cb[8]) * data->geometry.bytes_per_sector;
            break;
        case FIXED:
            data->cb_data_size = cmd->data_size;
            break;
        default:
            break;
        }
    }
    else
    {
        os_kprintf("cmd_len error %d\r\n", cmd->cmd_len);
    }
}

static os_bool_t _cbw_verify(ufunction_t func, struct scsi_cmd *cmd, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(cmd != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);
    OS_ASSERT(func != OS_NULL);

    data = (struct mstorage *)func->user_data;
    if (cmd->cmd_len != cbw->cb_len)
    {
        os_kprintf("cb_len error\r\n");
        cmd->cmd_len = cbw->cb_len;
    }

    if (cbw->xfer_len > 0 && data->cb_data_size == 0)
    {
        os_kprintf("xfer_len > 0 && data_size == 0\r\n");
        return OS_FALSE;
    }

    if (cbw->xfer_len == 0 && data->cb_data_size > 0)
    {
        os_kprintf("xfer_len == 0 && data_size > 0");
        return OS_FALSE;
    }

    if (((cbw->dflags & USB_DIR_IN) && (cmd->dir == DIR_OUT)) || (!(cbw->dflags & USB_DIR_IN) && (cmd->dir == DIR_IN)))
    {
        os_kprintf("dir error\r\n");
        return OS_FALSE;
    }

    if (cbw->xfer_len > data->cb_data_size)
    {
        os_kprintf("xfer_len > data_size\r\n");
        return OS_FALSE;
    }

    if (cbw->xfer_len < data->cb_data_size)
    {
        os_kprintf("xfer_len < data_size\r\n");
        data->cb_data_size        = cbw->xfer_len;
        data->csw_response.status = 1;
    }

    return OS_TRUE;
}

static os_size_t _cbw_handler(ufunction_t func, struct scsi_cmd *cmd, ustorage_cbw_t cbw)
{
    struct mstorage *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(cbw != OS_NULL);
    OS_ASSERT(cmd->handler != OS_NULL);

    data             = (struct mstorage *)func->user_data;
    data->processing = cmd;
    return cmd->handler(func, cbw);
}

/**
 ***********************************************************************************************************************
 * @brief          This function will handle mass storage bulk out endpoint request.
 *
 * @param[in]      func            The usb function object.
 * @param[in]     size            The request size.
 *
 * @return         The operation status.
 * @retval         OS_EOK          Successful.
 * @retval         OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
static os_err_t _ep_out_handler(ufunction_t func, os_size_t size)
{
    struct mstorage     *data;
    struct scsi_cmd     *cmd;
    os_size_t            len;
    struct ustorage_cbw *cbw;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_ep_out_handler %d\r\n", size));

    data = (struct mstorage *)func->user_data;
    cbw  = (struct ustorage_cbw *)data->ep_out->buffer;
    if (data->status == STAT_CBW)
    {
        /* Dump cbw information */
        if (cbw->signature != CBW_SIGNATURE || size != SIZEOF_CBW)
        {
            goto exit;
        }

        data->csw_response.signature   = CSW_SIGNATURE;
        data->csw_response.tag         = cbw->tag;
        data->csw_response.data_reside = cbw->xfer_len;
        data->csw_response.status      = 0;

        OS_DEBUG_LOG(OS_DEBUG_USB, ("ep_out reside %d\r\n", data->csw_response.data_reside));

        cmd = _find_cbw_command(cbw->cb[0]);
        if (cmd == OS_NULL)
        {
            os_kprintf("can't find cbw command\r\n");
            goto exit;
        }

        _cb_len_calc(func, cmd, cbw);
        if (!_cbw_verify(func, cmd, cbw))
        {
            goto exit;
        }

        len = _cbw_handler(func, cmd, cbw);
        if (len == 0)
        {
            _send_status(func);
        }

        return OS_EOK;
    }
    else if (data->status == STAT_RECEIVE)
    {
        OS_DEBUG_LOG(OS_DEBUG_USB, ("\nwrite size %d block 0x%x oount 0x%x\r\n", size, data->block, data->size));

        data->size -= size;
        data->csw_response.data_reside -= size;

        os_device_write_nonblock(data->disk, data->block, data->ep_out->buffer, 1);

        if (data->csw_response.data_reside != 0)
        {
            data->ep_out->request.buffer   = data->ep_out->buffer;
            data->ep_out->request.size     = data->geometry.bytes_per_sector;
            data->ep_out->request.req_type = UIO_REQUEST_READ_FULL;
            os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);
            data->block++;
        }
        else
        {
            _send_status(func);
        }

        return OS_EOK;
    }

exit:
    if (data->csw_response.data_reside)
    {
        if (cbw->dflags & USB_DIR_IN)
        {
            os_usbd_ep_set_stall(func->device, data->ep_in);
        }
        else
        {
            os_usbd_ep_set_stall(func->device, data->ep_in);
            os_usbd_ep_set_stall(func->device, data->ep_out);
        }
    }
    data->csw_response.status = 1;
    _send_status(func);

    return OS_ERROR;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will handle mass storage interface request.
 *
 * @param[in]      func            The usb function object.
 * @param[in]     setup           The setup request.
 *
 * @return         The operation status.
 * @retval         OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _interface_handler(ufunction_t func, ureq_t setup)
{
    os_uint8_t lun = 0;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("mstorage_interface_handler\r\n"));

    switch (setup->bRequest)
    {
    case USBREQ_GET_MAX_LUN:

        OS_DEBUG_LOG(OS_DEBUG_USB, ("USBREQ_GET_MAX_LUN\r\n"));

        if (setup->wValue || setup->wLength != 1)
        {
            os_usbd_ep0_set_stall(func->device);
        }
        else
        {
            os_usbd_ep0_write(func->device, &lun, setup->wLength);
        }
        break;
    case USBREQ_MASS_STORAGE_RESET:

        OS_DEBUG_LOG(OS_DEBUG_USB, ("USBREQ_MASS_STORAGE_RESET\r\n"));

        if (setup->wValue || setup->wLength != 0)
        {
            os_usbd_ep0_set_stall(func->device);
        }
        else
        {
            dcd_ep0_send_status(func->device->dcd);
        }
        break;
    default:
        os_kprintf("unknown interface request\r\n");
        break;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief          This function will run mass storage function, it will be called on handle set configuration request.
 *
 * @param[in]      func            The usb function object.
 *
 * @return         The operation status.
 * @retval         OS_EOK          Successful.
 * @retval         others          Fail.
 ***********************************************************************************************************************
 */
static os_err_t _function_enable(ufunction_t func)
{
    struct mstorage *data;
    OS_ASSERT(func != OS_NULL);
    OS_DEBUG_LOG(OS_DEBUG_USB, ("Mass storage function enabled\r\n"));
    data = (struct mstorage *)func->user_data;

    data->disk = os_device_find(OS_USB_MSTORAGE_DISK_NAME);
    if (data->disk == OS_NULL)
    {
        os_kprintf("no data->disk named %s\r\n", OS_USB_MSTORAGE_DISK_NAME);
        return OS_ERROR;
    }

    if (os_device_open(data->disk) != OS_EOK)
    {
        os_kprintf("disk open error\r\n");
        return OS_ERROR;
    }

    if (os_device_control(data->disk, OS_DEVICE_CTRL_BLK_GETGEOME, (void *)&data->geometry) != OS_EOK)
    {
        os_kprintf("get disk info error\r\n");
        return OS_ERROR;
    }

    data->ep_in->buffer = (os_uint8_t *)os_calloc(1, data->geometry.bytes_per_sector);
    if (data->ep_in->buffer == OS_NULL)
    {
        os_kprintf("no memory\r\n");
        return OS_ENOMEM;
    }
    data->ep_out->buffer = (os_uint8_t *)os_calloc(1, data->geometry.bytes_per_sector);
    if (data->ep_out->buffer == OS_NULL)
    {
        os_free(data->ep_in->buffer);
        os_kprintf("no memory\r\n");
        return OS_ENOMEM;
    }

    /* Prepare to read CBW request */
    data->ep_out->request.buffer   = data->ep_out->buffer;
    data->ep_out->request.size     = SIZEOF_CBW;
    data->ep_out->request.req_type = UIO_REQUEST_READ_FULL;
    os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will stop mass storage function, will be called on handle set configuration request.
 *
 * @param[in]       func            The usb function object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _function_disable(ufunction_t func)
{
    struct mstorage *data;
    OS_ASSERT(func != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("Mass storage function disabled\r\n"));

    data = (struct mstorage *)func->user_data;
    if (data->ep_in->buffer != OS_NULL)
    {
        os_free(data->ep_in->buffer);
        data->ep_in->buffer = OS_NULL;
    }

    if (data->ep_out->buffer != OS_NULL)
    {
        os_free(data->ep_out->buffer);
        data->ep_out->buffer = OS_NULL;
    }
    if (data->disk != OS_NULL)
    {
        os_device_close(data->disk);
        data->disk = OS_NULL;
    }

    data->status = STAT_CBW;

    return OS_EOK;
}

static struct ufunction_ops ops =
{
    _function_enable,
    _function_disable,
    OS_NULL,
};
static os_err_t _mstorage_descriptor_config(umass_desc_t desc, os_uint8_t cintf_nr, os_uint8_t device_is_hs)
{
#ifdef OS_USB_DEVICE_COMPOSITE
    desc->iad_desc.bFirstInterface = cintf_nr;
#endif
    desc->ep_out_desc.wMaxPacketSize = device_is_hs ? 512 : 64;
    desc->ep_in_desc.wMaxPacketSize  = device_is_hs ? 512 : 64;
    return OS_EOK;
}

ufunction_t os_usbd_function_mstorage_create(udevice_t device)
{
    uintf_t          intf;
    struct mstorage *data;
    ufunction_t      func;
    ualtsetting_t    setting;
    umass_desc_t     mass_desc;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Set usb device string description */
    os_usbd_device_set_string(device, _ustring);

    /* Create a mass storage function */
    func                  = os_usbd_function_new(device, &dev_desc, &ops);
    device->dev_qualifier = &dev_qualifier;

    /* Allocate memory for mass storage function data */
    data = (struct mstorage *)os_calloc(1, sizeof(struct mstorage));
    memset(data, 0, sizeof(struct mstorage));
    func->user_data = (void *)data;

    /* Create an interface object */
    intf = os_usbd_interface_new(device, _interface_handler);

    /* Create an alternate setting object */
    setting = os_usbd_altsetting_new(sizeof(struct umass_descriptor));

    /* Config desc in alternate setting */
    os_usbd_altsetting_config_descriptor(setting, &_mass_desc, (os_off_t) & ((umass_desc_t)0)->intf_desc);

    /* Configure the msc interface descriptor */
    _mstorage_descriptor_config(setting->desc, intf->intf_num, device->dcd->device_is_hs);

    /* Create a bulk out and a bulk in endpoint */
    mass_desc    = (umass_desc_t)setting->desc;
    data->ep_in  = os_usbd_endpoint_new(&mass_desc->ep_in_desc, _ep_in_handler);
    data->ep_out = os_usbd_endpoint_new(&mass_desc->ep_out_desc, _ep_out_handler);

    /* Add the bulk out and bulk in endpoint to the alternate setting */
    os_usbd_altsetting_add_endpoint(setting, data->ep_out);
    os_usbd_altsetting_add_endpoint(setting, data->ep_in);

    /* Add the alternate setting to the interface, then set default setting */
    os_usbd_interface_add_altsetting(intf, setting);
    os_usbd_set_altsetting(intf, 0);

    /* Add the interface to the mass storage function */
    os_usbd_function_add_interface(func, intf);

    return func;
}
struct udclass msc_class = {.os_usbd_function_create = os_usbd_function_mstorage_create};

int os_usbd_msc_class_register(void)
{
    os_usbd_class_register(&msc_class);
    return 0;
}
OS_PREV_INIT(os_usbd_msc_class_register);

#endif
