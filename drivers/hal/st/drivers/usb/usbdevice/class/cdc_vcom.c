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
 * @file        cdc_vcom.c
 *
 * @brief       This file provides functions for cdc.
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
#include <serial/serial.h>
#include "usb/usb_device.h"
#include "cdc.h"

#ifdef OS_USB_DEVICE_CDC

#ifdef OS_VCOM_TX_TIMEOUT
#define VCOM_TX_TIMEOUT OS_VCOM_TX_TIMEOUT
#else /* !OS_VCOM_TX_TIMEOUT */
#define VCOM_TX_TIMEOUT 1000
#endif /* OS_VCOM_TX_TIMEOUT */

#define CDC_RX_BUFSIZE      128
#define CDC_MAX_PACKET_SIZE 64
#define VCOM_DEVICE         "vcom"

#ifdef OS_VCOM_TASK_STK_SIZE
#define VCOM_TASK_STK_SIZE OS_VCOM_TASK_STK_SIZE
#else /* !OS_VCOM_TASK_STK_SIZE */
#define VCOM_TASK_STK_SIZE 512
#endif /* OS_VCOM_TASK_STK_SIZE */

#ifdef OS_VCOM_TX_USE_DMA
#define VCOM_TX_USE_DMA
#endif /* OS_VCOM_TX_USE_DMA */

#ifdef OS_VCOM_SERNO
#define _SER_NO OS_VCOM_SERNO
#else /* !OS_VCOM_SERNO */
#define _SER_NO "32021919830108"
#endif /* OS_VCOM_SERNO */

#ifdef OS_VCOM_SER_LEN
#define _SER_NO_LEN OS_VCOM_SER_LEN
#else                  /* !OS_VCOM_SER_LEN */
#define _SER_NO_LEN 14 /* os_strlen("32021919830108") */
#endif                 /* OS_VCOM_SER_LEN */

OS_ALIGN(OS_ALIGN_SIZE)
static os_uint8_t              vcom_thread_stack[VCOM_TASK_STK_SIZE];
static struct os_task          vcom_thread;
static struct ucdc_line_coding line_coding;

#define CDC_TX_BUFSIZE     1024
#define CDC_BULKIN_MAXSIZE (CDC_TX_BUFSIZE / 8)

#define CDC_TX_HAS_DATE  0x01
#define CDC_TX_HAS_SPACE 0x02

struct vcom
{
    struct os_serial_device serial;
    uep_t                   ep_out;
    uep_t                   ep_in;
    uep_t                   ep_cmd;
    os_bool_t               connected;
    os_bool_t               in_sending;
    struct os_completion    wait;
    os_uint8_t              rx_rbp[CDC_RX_BUFSIZE];
    struct rb_ring_buff     rx_ringbuffer;
    os_uint8_t              tx_rbp[CDC_TX_BUFSIZE];
    struct rb_ring_buff     tx_ringbuffer;
    struct os_event         tx_event;
};

struct vcom_tx_msg
{
    struct os_serial_device *serial;
    const char              *buf;
    os_size_t                size;
};

OS_ALIGN(4)
static struct udevice_descriptor dev_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength; */
    USB_DESC_TYPE_DEVICE,     /* type; */
    USB_BCD_VERSION,          /* bcdUSB; */
    USB_CLASS_CDC,            /* bDeviceClass; */
    0x00,                     /* bDeviceSubClass; */
    0x00,                     /* bDeviceProtocol; */
    CDC_MAX_PACKET_SIZE,      /* bMaxPacketSize0; */
    _VENDOR_ID,               /* idVendor; */
    _PRODUCT_ID,              /* idProduct; */
    USB_BCD_DEVICE,           /* bcdDevice; */
    USB_STRING_MANU_INDEX,    /* iManufacturer; */
    USB_STRING_PRODUCT_INDEX, /* iProduct; */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber; */
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
    USB_CLASS_CDC,                  /* bDeviceClass */
    0x00,                           /* bDeviceSubClass */
    0x00,                           /* bDeviceProtocol */
    64,                             /* bMaxPacketSize0 */
    0x01,                           /* bNumConfigurations */
    0,
};
#endif

OS_ALIGN(4)
const static struct ucdc_comm_descriptor _comm_desc =
{
#ifdef OS_USB_DEVICE_COMPOSITE
    /* Interface Association Descriptor */
    {
        USB_DESC_LENGTH_IAD,
        USB_DESC_TYPE_IAD,
        USB_DYNAMIC,
        0x02,
        USB_CDC_CLASS_COMM,
        USB_CDC_SUBCLASS_ACM,
        USB_CDC_PROTOCOL_V25TER,
        0x00,
    },
#endif
    /* Interface Descriptor */
    {
        USB_DESC_LENGTH_INTERFACE,
        USB_DESC_TYPE_INTERFACE,
        USB_DYNAMIC,
        0x00,
        0x01,
        USB_CDC_CLASS_COMM,
        USB_CDC_SUBCLASS_ACM,
        USB_CDC_PROTOCOL_V25TER,
        0x00,
    },
    /* Header Functional Descriptor */
    {
        0x05,
        USB_CDC_CS_INTERFACE,
        USB_CDC_SCS_HEADER,
        0x0110,
    },
    /* Call Management Functional Descriptor */
    {
        0x05,
        USB_CDC_CS_INTERFACE,
        USB_CDC_SCS_CALL_MGMT,
        0x00,
        USB_DYNAMIC,
    },
    /* Abstract Control Management Functional Descriptor */
    {
        0x04,
        USB_CDC_CS_INTERFACE,
        USB_CDC_SCS_ACM,
        0x02,
    },
    /* Union Functional Descriptor */
    {
        0x05,
        USB_CDC_CS_INTERFACE,
        USB_CDC_SCS_UNION,
        USB_DYNAMIC,
        USB_DYNAMIC,
    },
    /* Endpoint Descriptor */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_IN,
        USB_EP_ATTR_INT,
        0x08,
        0xFF,
    },
};

OS_ALIGN(4)
const static struct ucdc_data_descriptor _data_desc =
{
    /* Interface descriptor */
    {
        USB_DESC_LENGTH_INTERFACE,
        USB_DESC_TYPE_INTERFACE,
        USB_DYNAMIC,
        0x00,
        0x02,
        USB_CDC_CLASS_DATA,
        0x00,
        0x00,
        0x00,
    },
    /* Endpoint, bulk out */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_OUT,
        USB_EP_ATTR_BULK,
        USB_CDC_BUFSIZE,
        0x00,
    },
    /* Endpoint, bulk in */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_IN,
        USB_EP_ATTR_BULK,
        USB_CDC_BUFSIZE,
        0x00,
    },
};
OS_ALIGN(4)
static char      serno[_SER_NO_LEN + 1] = {'\0'};
OS_WEAK os_err_t vcom_get_stored_serno(char *serno, int size);

os_err_t vcom_get_stored_serno(char *serno, int size)
{
    return OS_ERROR;
}
OS_ALIGN(4)
const static char *_ustring[] =
{
    "Language",
    "CMCC Team.",
    "Virtual Serial",
    serno,
    "Configuration",
    "Interface",
};
static void os_usb_vcom_init(struct ufunction *func);

static void _vcom_reset_state(ufunction_t func)
{
    struct vcom *data;
    int          lvl;

    OS_ASSERT(func != OS_NULL);

    data = (struct vcom *)func->user_data;

    lvl              = os_irq_lock();
    data->connected  = OS_FALSE;
    data->in_sending = OS_FALSE;
    /*os_kprintf("reset USB serial\r\n", cnt);*/
    os_irq_unlock(lvl);
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle cdc bulk in endpoint request.
 *
 * @param[in]       func            The usb function object.
 * @param[in]       size            The request size.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _ep_in_handler(ufunction_t func, os_size_t size)
{
    struct vcom *data;
    os_size_t    request_size;

    OS_ASSERT(func != OS_NULL);

    data         = (struct vcom *)func->user_data;
    request_size = data->ep_in->request.size;
    OS_DEBUG_LOG(OS_DEBUG_USB, ("_ep_in_handler %d\r\n", request_size));
    if ((request_size != 0) && ((request_size % EP_MAXPACKET(data->ep_in)) == 0))
    {
        /*
         * don't have data right now. Send a zero-length-packet to
         * terminate the transaction.
         *
         * FIXME: actually, this might not be the right place to send zlp.
         * Only the os_device_write_nonblock could know how much data is sending.
         */
        data->in_sending = OS_TRUE;

        data->ep_in->request.buffer   = OS_NULL;
        data->ep_in->request.size     = 0;
        data->ep_in->request.req_type = UIO_REQUEST_WRITE;
        os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);

        return OS_EOK;
    }

    os_completion_done(&data->wait);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle cdc bulk out endpoint request.
 *
 * @param[in]       func            The usb function object.
 * @param[in]       size            The request size.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _ep_out_handler(ufunction_t func, os_size_t size)
{
    os_uint32_t  level;
    struct vcom *data;

    OS_ASSERT(func != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_ep_out_handler %d\r\n", size));

    data = (struct vcom *)func->user_data;
    /* Ensure serial is active */
    if ((data->serial.parent.flag & OS_DEVICE_FLAG_ACTIVATED) && (data->serial.parent.open_flag & OS_DEVICE_OFLAG_OPEN))
    {
        /* Receive data from USB VCOM */
        level = os_irq_lock();

        rb_ring_buff_put(&data->rx_ringbuffer, data->ep_out->buffer, size);
        os_irq_unlock(level);

        /* Notify receive data */
        os_hw_serial_isr(&data->serial, OS_SERIAL_EVENT_RX_IND);
    }

    data->ep_out->request.buffer   = data->ep_out->buffer;
    data->ep_out->request.size     = EP_MAXPACKET(data->ep_out);
    data->ep_out->request.req_type = UIO_REQUEST_READ_BEST;
    os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle cdc interrupt in endpoint request.
 *
 * @param[in]       device           The usb device object.
 * @param[in]       size             The request size.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _ep_cmd_handler(ufunction_t func, os_size_t size)
{
    OS_ASSERT(func != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_ep_cmd_handler\r\n"));

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle cdc_get_line_coding request.
 *
 * @param[in]       device            The usb device object.
 * @param[in]       setup             The setup request.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _cdc_get_line_coding(udevice_t device, ureq_t setup)
{
    struct ucdc_line_coding data;
    os_uint16_t             size;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_cdc_get_line_coding\r\n"));

    data.dwDTERate   = 115200;
    data.bCharFormat = 0;
    data.bDataBits   = 8;
    data.bParityType = 0;
    size             = setup->wLength > 7 ? 7 : setup->wLength;

    os_usbd_ep0_write(device, (void *)&data, size);

    return OS_EOK;
}

static os_err_t _cdc_set_line_coding_callback(udevice_t device, os_size_t size)
{
    OS_DEBUG_LOG(OS_DEBUG_USB, ("_cdc_set_line_coding_callback\r\n"));

    dcd_ep0_send_status(device->dcd);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle cdc_set_line_coding request.
 *
 * @param[in]       device            The usb device object.
 * @param[in]       setup             The setup request.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _cdc_set_line_coding(udevice_t device, ureq_t setup)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("_cdc_set_line_coding\r\n"));

    os_usbd_ep0_read(device, (void *)&line_coding, sizeof(struct ucdc_line_coding), _cdc_set_line_coding_callback);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle cdc interface request.
 *
 * @param[in]       device            The usb device object.
 * @param[in]       setup             The setup request.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
static os_err_t _interface_handler(ufunction_t func, ureq_t setup)
{
    struct vcom *data;

    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(func->device != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    data = (struct vcom *)func->user_data;

    switch (setup->bRequest)
    {
    case CDC_SEND_ENCAPSULATED_COMMAND:
        break;
    case CDC_GET_ENCAPSULATED_RESPONSE:
        break;
    case CDC_SET_COMM_FEATURE:
        break;
    case CDC_GET_COMM_FEATURE:
        break;
    case CDC_CLEAR_COMM_FEATURE:
        break;
    case CDC_SET_LINE_CODING:
        _cdc_set_line_coding(func->device, setup);
        break;
    case CDC_GET_LINE_CODING:
        _cdc_get_line_coding(func->device, setup);
        break;
    case CDC_SET_CONTROL_LINE_STATE:
        data->connected = (setup->wValue & 0x01) > 0 ? OS_TRUE : OS_FALSE;
        OS_DEBUG_LOG(OS_DEBUG_USB, ("vcom state:%d \r\n", data->connected));
        dcd_ep0_send_status(func->device->dcd);
        break;
    case CDC_SEND_BREAK:
        break;
    default:
        os_kprintf("unknown cdc request\r\n", setup->request_type);
        return OS_ERROR;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will run cdc function, it will be called on handle set configuration request.
 *
 * @param[in]       func            The usb function object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _function_enable(ufunction_t func)
{
    struct vcom *data;

    OS_ASSERT(func != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("cdc function enable\r\n"));

    _vcom_reset_state(func);

    data                 = (struct vcom *)func->user_data;
    data->ep_out->buffer = os_calloc(1, CDC_RX_BUFSIZE);

    data->ep_out->request.buffer = data->ep_out->buffer;
    data->ep_out->request.size   = EP_MAXPACKET(data->ep_out);

    data->ep_out->request.req_type = UIO_REQUEST_READ_BEST;
    os_usbd_io_request(func->device, data->ep_out, &data->ep_out->request);

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will stop cdc function, it will be called on handle set configuration request.
 *
 * @param[in]       func            The usb function object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _function_disable(ufunction_t func)
{
    struct vcom *data;

    OS_ASSERT(func != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("cdc function disable\r\n"));

    _vcom_reset_state(func);

    data = (struct vcom *)func->user_data;
    if (data->ep_out->buffer != OS_NULL)
    {
        os_free(data->ep_out->buffer);
        data->ep_out->buffer = OS_NULL;
    }

    return OS_EOK;
}

static struct ufunction_ops ops =
{
    _function_enable,
    _function_disable,
    OS_NULL,
};

static os_err_t _cdc_descriptor_config(ucdc_comm_desc_t comm, 
    os_uint8_t cintf_nr, ucdc_data_desc_t data, os_uint8_t dintf_nr)
{
    comm->call_mgmt_desc.data_interface = dintf_nr;
    comm->union_desc.master_interface   = cintf_nr;
    comm->union_desc.slave_interface0   = dintf_nr;
#ifdef OS_USB_DEVICE_COMPOSITE
    comm->iad_desc.bFirstInterface = cintf_nr;
#endif

    return OS_EOK;
}

ufunction_t os_usbd_function_cdc_create(udevice_t device)
{
    ufunction_t      func;
    struct vcom     *data;
    uintf_t          intf_comm, intf_data;
    ualtsetting_t    comm_setting, data_setting;
    ucdc_data_desc_t data_desc;
    ucdc_comm_desc_t comm_desc;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    memset(serno, 0, _SER_NO_LEN + 1);
    if (vcom_get_stored_serno(serno, _SER_NO_LEN) != OS_EOK)
    {
        memset(serno, 0, _SER_NO_LEN + 1);
        memcpy(serno, _SER_NO, strlen(_SER_NO));
    }
    /* Set usb device string description */
    os_usbd_device_set_string(device, _ustring);

    /* Create a cdc function */
    func = os_usbd_function_new(device, &dev_desc, &ops);

    /* Allocate memory for cdc vcom data */
    data = (struct vcom *)os_calloc(1, sizeof(struct vcom));
    memset(data, 0, sizeof(struct vcom));
    func->user_data = (void *)data;

    /* Initilize vcom */
    os_usb_vcom_init(func);

    /* Create a cdc communication interface and a cdc data interface */
    intf_comm = os_usbd_interface_new(device, _interface_handler);
    intf_data = os_usbd_interface_new(device, _interface_handler);

    /* Create a communication alternate setting and a data alternate setting */
    comm_setting = os_usbd_altsetting_new(sizeof(struct ucdc_comm_descriptor));
    data_setting = os_usbd_altsetting_new(sizeof(struct ucdc_data_descriptor));

    /* Config desc in alternate setting */
    os_usbd_altsetting_config_descriptor(comm_setting, &_comm_desc, (os_off_t) & ((ucdc_comm_desc_t)0)->intf_desc);
    os_usbd_altsetting_config_descriptor(data_setting, &_data_desc, 0);
    /* Configure the cdc interface descriptor */
    _cdc_descriptor_config(comm_setting->desc, intf_comm->intf_num, data_setting->desc, intf_data->intf_num);

    /* Create a command endpoint */
    comm_desc    = (ucdc_comm_desc_t)comm_setting->desc;
    data->ep_cmd = os_usbd_endpoint_new(&comm_desc->ep_desc, _ep_cmd_handler);

    /* Add the command endpoint to the cdc communication interface */
    os_usbd_altsetting_add_endpoint(comm_setting, data->ep_cmd);

    /*
     * Add the communication alternate setting to the communication interface,
     * then set default setting of the interface
     */
    os_usbd_interface_add_altsetting(intf_comm, comm_setting);
    os_usbd_set_altsetting(intf_comm, 0);

    /* Add the communication interface to the cdc function */
    os_usbd_function_add_interface(func, intf_comm);

    /* Create a bulk in and a bulk endpoint */
    data_desc    = (ucdc_data_desc_t)data_setting->desc;
    data->ep_out = os_usbd_endpoint_new(&data_desc->ep_out_desc, _ep_out_handler);
    data->ep_in  = os_usbd_endpoint_new(&data_desc->ep_in_desc, _ep_in_handler);

    /* Add the bulk out and bulk in endpoints to the data alternate setting */
    os_usbd_altsetting_add_endpoint(data_setting, data->ep_in);
    os_usbd_altsetting_add_endpoint(data_setting, data->ep_out);

    /* Add the data alternate setting to the data interface
            then set default setting of the interface */
    os_usbd_interface_add_altsetting(intf_data, data_setting);
    os_usbd_set_altsetting(intf_data, 0);

    /* Add the cdc data interface to cdc function */
    os_usbd_function_add_interface(func, intf_data);

    return func;
}

static os_err_t _vcom_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    return OS_EOK;
}

static os_err_t _vcom_control(struct os_serial_device *serial, int cmd, void *arg)
{
    switch (cmd)
    {
    case OS_DEVICE_CTRL_CLR_INT:
        /* Disable rx irq */
        break;
    case OS_DEVICE_CTRL_SET_INT:
        /* Enable rx irq */
        break;
    }

    return OS_EOK;
}

static int _vcom_getc(struct os_serial_device *serial)
{
    int               result;
    os_uint8_t        ch;
    os_uint32_t       level;
    struct ufunction *func;
    struct vcom      *data;

    func = (struct ufunction *)serial->parent.user_data;
    data = (struct vcom *)func->user_data;

    result = -1;

    level = os_irq_lock();

    if (rb_ring_buff_get_char(&data->rx_ringbuffer, &ch) != 0)
    {
        result = ch;
    }

    os_irq_unlock(level);

    return result;
}

static os_size_t _vcom_rb_block_put(struct vcom *data, const os_uint8_t *buf, os_size_t size)
{
    os_uint32_t level;
    os_size_t   put_len = 0;
    os_size_t   w_ptr   = 0;
    os_uint32_t res;
    os_size_t   remain_size = size;

    while (remain_size)
    {
        level   = os_irq_lock();
        put_len = rb_ring_buff_put(&data->tx_ringbuffer, (const os_uint8_t *)&buf[w_ptr], remain_size);
        os_irq_unlock(level);
        w_ptr += put_len;
        remain_size -= put_len;
        if (put_len == 0)
        {
            os_event_recv(&data->tx_event,
                          CDC_TX_HAS_SPACE,
                          OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                          VCOM_TX_TIMEOUT,
                          &res);
        }
        else
        {
            os_event_send(&data->tx_event, CDC_TX_HAS_DATE);
        }
    }

    return size;
}

static os_size_t _vcom_tx(struct os_serial_device *serial, os_uint8_t *buf, os_size_t size, int direction)
{
    struct ufunction *func;
    struct vcom      *data;
    os_uint32_t send_size = 0;
    os_size_t ptr = 0;
    os_uint8_t crlf[2] = {'\r', '\n',};

    func = (struct ufunction *)serial->parent.user_data;
    data = (struct vcom *)func->user_data;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(buf != OS_NULL);

    OS_DEBUG_LOG(OS_DEBUG_USB, ("%s\r\n", __func__));

    if (data->connected)
    {
#if 0
        if ((serial->parent.open_flag & OS_DEVICE_FLAG_STREAM))
        {
            while (send_size < size)
            {
                while (ptr < size && buf[ptr] != '\n')
                {
                    ptr++;
                }
                if (ptr < size)
                {
                    send_size += _vcom_rb_block_put(data, (const os_uint8_t *)&buf[send_size], ptr - send_size);
                    _vcom_rb_block_put(data, crlf, 2);
                    send_size++;
                    ptr++;
                }
                else if (ptr == size)
                {
                    send_size += _vcom_rb_block_put(data, (const os_uint8_t *)&buf[send_size], ptr - send_size);
                }
                else
                {
                    break;
                }
            }
        }
        else
#endif
        {
            while (send_size < size)
            {
                send_size += _vcom_rb_block_put(data, (os_uint8_t *)&buf[send_size], size - send_size);
            }
        }
    }
    else
    {
        /* Recover dataqueue resources */
        os_hw_serial_isr(&data->serial, OS_SERIAL_EVENT_TX_DMADONE);
    }

    return size;
}
static int _vcom_putc(struct os_serial_device *serial, char c)
{
    os_uint32_t       level;
    struct ufunction *func;
    struct vcom      *data;

    func = (struct ufunction *)serial->parent.user_data;
    data = (struct vcom *)func->user_data;

    OS_ASSERT(serial != OS_NULL);

    if (data->connected)
    {
        if (c == '\n')// && (serial->parent.open_flag & OS_DEVICE_FLAG_STREAM))
        {
            level = os_irq_lock();
            rb_ring_buff_put_char_force(&data->tx_ringbuffer, '\r');
            os_irq_unlock(level);
            os_event_send(&data->tx_event, CDC_TX_HAS_DATE);
        }
        level = os_irq_lock();
        rb_ring_buff_put_char_force(&data->tx_ringbuffer, c);
        os_irq_unlock(level);
        os_event_send(&data->tx_event, CDC_TX_HAS_DATE);
    }

    return 1;
}

static const struct os_uart_ops usb_vcom_ops =
{
    _vcom_configure,
    _vcom_control,
    _vcom_putc,
    _vcom_getc,
    _vcom_tx
};

/* Vcom Tx Thread */
static void vcom_tx_thread_entry(void* parameter)
{
    os_uint32_t       level;
    os_uint32_t       res;
    struct ufunction *func = (struct ufunction *)parameter;
    struct vcom *     data = (struct vcom *)func->user_data;
    os_uint8_t        ch[CDC_BULKIN_MAXSIZE];

    while (1)
    {
        if ((os_event_recv(&data->tx_event,
                           CDC_TX_HAS_DATE,
                           OS_EVENT_OPTION_OR | OS_EVENT_OPTION_CLEAR,
                           OS_WAIT_FOREVER,
                           &res) != OS_EOK) ||
            (!(res & CDC_TX_HAS_DATE)))
        {
            continue;
        }
        if (!(res & CDC_TX_HAS_DATE))
        {
            continue;
        }
        while (rb_ring_buff_data_len(&data->tx_ringbuffer))
        {
            level = os_irq_lock();
            res   = rb_ring_buff_get(&data->tx_ringbuffer, ch, CDC_BULKIN_MAXSIZE);
            os_irq_unlock(level);

            if (!res)
            {
                continue;
            }
            if (!data->connected)
            {
                if (data->serial.parent.open_flag &
#ifndef VCOM_TX_USE_DMA
                    OS_DEVICE_FLAG_INT_TX
#else
                    OS_DEVICE_FLAG_DMA_TX
#endif
                )
                {
                    /* Drop msg */
#ifndef VCOM_TX_USE_DMA
                    os_hw_serial_isr(&data->serial, OS_SERIAL_EVENT_TX_DONE);
#else
                    os_hw_serial_isr(&data->serial, OS_SERIAL_EVENT_TX_DMADONE);
#endif
                }
                continue;
            }
            os_completion_init(&data->wait);
            data->ep_in->request.buffer = ch;
            data->ep_in->request.size   = res;

            data->ep_in->request.req_type = UIO_REQUEST_WRITE;

            os_usbd_io_request(func->device, data->ep_in, &data->ep_in->request);

            if (os_completion_wait(&data->wait, VCOM_TX_TIMEOUT) != OS_EOK)
            {
                OS_DEBUG_LOG(OS_DEBUG_USB, ("vcom tx timeout\r\n"));
            }
            if (data->serial.parent.open_flag &
#ifndef VCOM_TX_USE_DMA
                OS_DEVICE_FLAG_INT_TX
#else
                OS_DEVICE_FLAG_DMA_TX
#endif
            )
            {
#ifndef VCOM_TX_USE_DMA
                os_hw_serial_isr(&data->serial, OS_SERIAL_EVENT_TX_DONE);
#else
                os_hw_serial_isr(&data->serial, OS_SERIAL_EVENT_TX_DMADONE);
#endif
                os_event_send(&data->tx_event, CDC_TX_HAS_SPACE);
            }
        }
    }
}

static void os_usb_vcom_init(struct ufunction *func)
{
    os_err_t                result = OS_EOK;
    struct serial_configure config;
    struct vcom *data = (struct vcom *)func->user_data;

    /* Initialize ring buffer */
    rb_ring_buff_init(&data->rx_ringbuffer, data->rx_rbp, CDC_RX_BUFSIZE);
    rb_ring_buff_init(&data->tx_ringbuffer, data->tx_rbp, CDC_TX_BUFSIZE);

    os_event_init(&data->tx_event, "vcom", OS_IPC_FLAG_FIFO);

    config.baud_rate = BAUD_RATE_115200;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.parity    = PARITY_NONE;
    config.bit_order = BIT_ORDER_LSB;
    config.invert    = NRZ_NORMAL;
    config.bufsz     = CDC_RX_BUFSIZE;

    data->serial.ops       = &usb_vcom_ops;
    data->serial.serial_rx = OS_NULL;
    data->serial.config    = config;

    /* Register vcom device */
    os_hw_serial_register(&data->serial,
                          VCOM_DEVICE,
                          func);

    /* Init usb device thread */
    os_task_init(&vcom_thread, "vcom", vcom_tx_thread_entry, func, vcom_thread_stack, VCOM_TASK_STK_SIZE, 16, 20);
    result = os_task_startup(&vcom_thread);
    OS_ASSERT(result == OS_EOK);
}
struct udclass vcom_class = {.os_usbd_function_create = os_usbd_function_cdc_create};

int os_usbd_vcom_class_register(void)
{
    os_usbd_class_register(&vcom_class);
    return 0;
}
OS_PREV_INIT(os_usbd_vcom_class_register);

#endif
