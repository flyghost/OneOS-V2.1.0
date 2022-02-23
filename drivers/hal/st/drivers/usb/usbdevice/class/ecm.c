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
 * @file        ecm.c
 *
 * @brief       This file provides functions for ecm.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#ifdef OS_USB_DEVICE_ECM
#include "cdc.h"

#define DRV_EXT_LVL      DBG_WARNING
#define DBG_SECTION_NAME "ECM"
#include <drv_log.h>

/* LWIP ethernet interface */
#include <netif/ethernetif.h>

#ifndef USB_ETH_MTU
#define USB_ETH_MTU 1514
#endif
#define MAX_ADDR_LEN 6

struct os_ecm_eth
{
    /* Inherit from ethernet device */
    struct eth_device parent;
    struct ufunction *func;
    struct cdc_eps    eps;
    /* Interface address info */
    os_uint8_t host_addr[MAX_ADDR_LEN];
    os_uint8_t dev_addr[MAX_ADDR_LEN];

    OS_ALIGN(4)
    os_uint8_t rx_pool[512];
    OS_ALIGN(4)
    os_size_t rx_size;
    OS_ALIGN(4)
    os_size_t rx_offset;
    OS_ALIGN(4)
    char rx_buffer[USB_ETH_MTU];
    char tx_buffer[USB_ETH_MTU];

    struct os_semaphore tx_buffer_free;
};
typedef struct os_ecm_eth *os_ecm_eth_t;

OS_ALIGN(4)
static struct udevice_descriptor _dev_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength */
    USB_DESC_TYPE_DEVICE,     /* type */
    USB_BCD_VERSION,          /* bcdUSB */
    USB_CLASS_CDC,            /* bDeviceClass */
    USB_CDC_SUBCLASS_ETH,     /* bDeviceSubClass */
    USB_CDC_PROTOCOL_NONE,    /* bDeviceProtocol */
    0x40,                     /* bMaxPacketSize0 */
    _VENDOR_ID,               /* idVendor */
    _PRODUCT_ID,              /* idProduct */
    USB_BCD_DEVICE,           /* bcdDevice */
    USB_STRING_MANU_INDEX,    /* iManufacturer */
    USB_STRING_PRODUCT_INDEX, /* iProduct */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber */
    USB_DYNAMIC               /* bNumConfigurations */
};

OS_ALIGN(4)
const static struct ucdc_eth_descriptor _comm_desc =
{
#ifdef OS_USB_DEVICE_COMPOSITE
    /* Interface Association Descriptor */
    {
        USB_DESC_LENGTH_IAD,
        USB_DESC_TYPE_IAD,
        USB_DYNAMIC,
        0x02,
        USB_CDC_CLASS_COMM,
        USB_CDC_SUBCLASS_ETH,
        USB_CDC_PROTOCOL_NONE,
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
        USB_CDC_SUBCLASS_ETH,
        USB_CDC_PROTOCOL_NONE,
        0x00,
    },
    /* Header Functional Descriptor */
    {
        sizeof(struct ucdc_header_descriptor),
        USB_CDC_CS_INTERFACE,
        USB_CDC_SCS_HEADER,
        0x0110,
    },
    /* Union Functional Descriptor */
    {
        sizeof(struct ucdc_union_descriptor),
        USB_CDC_CS_INTERFACE,
        USB_CDC_SCS_UNION,
        USB_DYNAMIC,
        USB_DYNAMIC,
    },
    /* Abstract Control Management Functional Descriptor */
    {
        sizeof(struct ucdc_enet_descriptor),
        USB_CDC_CS_INTERFACE,
        USB_CDC_SCS_ETH,
        USB_STRING_SERIAL_INDEX,
        {0, 0, 0, 0},
        USB_ETH_MTU,
        0x00,
        0x00,
    },
    /* Endpoint Descriptor */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DIR_IN | USB_DYNAMIC,
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
        USB_CDC_SUBCLASS_ETH,
        0x00,
        0x00,
    },
    /* Endpoint, bulk out */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DIR_OUT | USB_DYNAMIC,
        USB_EP_ATTR_BULK,
        USB_DYNAMIC,
        0x00,
    },
    /* Endpoint, bulk in */
    {
        USB_DESC_LENGTH_ENDPOINT,
        USB_DESC_TYPE_ENDPOINT,
        USB_DYNAMIC | USB_DIR_IN,
        USB_EP_ATTR_BULK,
        USB_DYNAMIC,
        0x00,
    },
};

OS_ALIGN(4)
const static char *_ustring[] =
{
    "Language",      /* LANGID */
    "OS Team.",      /* MANU */
    "OS ECM device", /* PRODUCT */
    "3497F694ECAB",  /* SERIAL (MAC)*/
    "Configuration", /* CONFIG */
    "Interface",     /* INTERFACE */
};

OS_ALIGN(4)
/* FS and HS needed */
static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),         /* bLength */
    USB_DESC_TYPE_DEVICEQUALIFIER, /* bDescriptorType */
    0x0200,                        /* bcdUSB */
    USB_CLASS_CDC,                 /* bDeviceClass */
    USB_CDC_SUBCLASS_ETH,          /* bDeviceSubClass */
    USB_CDC_PROTOCOL_NONE,         /* bDeviceProtocol */
    64,                            /* bMaxPacketSize0 */
    0x01,                          /* bNumConfigurations */
    0,
};

static os_err_t _cdc_send_notifi(ufunction_t func,ucdc_notification_code_t notifi,
                                 os_uint16_t wValue,os_uint16_t wLength)
{
    static struct ucdc_management_element_notifications _notifi;
    cdc_eps_t                                           eps;
    OS_ASSERT(func != OS_NULL);
    eps                      = &((os_ecm_eth_t)func->user_data)->eps;
    _notifi.bmRequestType    = 0xA1;
    _notifi.bNotificatinCode = notifi;
    _notifi.wValue           = wValue;
    _notifi.wLength          = wLength;

    eps->ep_cmd->request.buffer   = (void *)&_notifi;
    eps->ep_cmd->request.size     = 8;
    eps->ep_cmd->request.req_type = UIO_REQUEST_WRITE;
    os_usbd_io_request(func->device, eps->ep_cmd, &eps->ep_cmd->request);
    return OS_EOK;
}

static os_err_t _ecm_set_eth_packet_filter(ufunction_t func, ureq_t setup)
{
    os_ecm_eth_t _ecm_eth = (os_ecm_eth_t)func->user_data;
    dcd_ep0_send_status(func->device->dcd);

    /* Send link up. */
    eth_device_linkchange(&_ecm_eth->parent, OS_TRUE);
    _cdc_send_notifi(func, UCDC_NOTIFI_NETWORK_CONNECTION, 1, 0);

#ifdef LWIP_USING_DHCPD
    extern void dhcpd_start(const char *netif_name);
    dhcpd_start("u0");
#endif

    return OS_EOK;
}

static os_err_t _interface_handler(ufunction_t func, ureq_t setup)
{
    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    switch (setup->bRequest)
    {
    case CDC_SET_ETH_PACKET_FILTER:
        LOG_EXT_D("CDC_SET_ETH_PACKET_FILTER");
        _ecm_set_eth_packet_filter(func, setup);
        break;
    default:
        LOG_E(DBG_TAG,"Unknow setup->bRequest: 0x%02X", setup->bRequest);
        break;
    }
    return OS_EOK;
}

static os_err_t _ep_in_handler(ufunction_t func, os_size_t size)
{
    os_ecm_eth_t ecm_device = (os_ecm_eth_t)func->user_data;
    os_sem_post(&ecm_device->tx_buffer_free);
    return OS_EOK;
}

static os_err_t _ep_out_handler(ufunction_t func, os_size_t size)
{
    os_ecm_eth_t ecm_device = (os_ecm_eth_t)func->user_data;
    memcpy((void *)(ecm_device->rx_buffer + ecm_device->rx_offset), ecm_device->rx_pool, size);
    ecm_device->rx_offset += size;
    if (size < EP_MAXPACKET(ecm_device->eps.ep_out))
    {
        ecm_device->rx_size   = ecm_device->rx_offset;
        ecm_device->rx_offset = 0;
        eth_device_ready(&ecm_device->parent);
    }
    else
    {
        ecm_device->eps.ep_out->request.buffer   = ecm_device->eps.ep_out->buffer;
        ecm_device->eps.ep_out->request.size     = EP_MAXPACKET(ecm_device->eps.ep_out);
        ecm_device->eps.ep_out->request.req_type = UIO_REQUEST_READ_BEST;
        os_usbd_io_request(ecm_device->func->device, ecm_device->eps.ep_out, &ecm_device->eps.ep_out->request);
    }

    return OS_EOK;
}
static os_err_t os_ecm_eth_init(os_device_t *dev)
{
    return OS_EOK;
}

static os_err_t os_ecm_eth_open(os_device_t *dev, os_uint16_t oflag)
{
    return OS_EOK;
}

static os_err_t os_ecm_eth_close(os_device_t *dev)
{
    return OS_EOK;
}

static os_size_t os_ecm_eth_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_set_errno(OS_ENOSYS);
    return 0;
}

static os_size_t os_ecm_eth_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_set_errno(OS_ENOSYS);
    return 0;
}
static os_err_t os_ecm_eth_control(os_device_t *dev, int cmd, void *args)
{
    os_ecm_eth_t ecm_eth_dev = (os_ecm_eth_t)dev;
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* Get mac address */
        if (args)
            memcpy(args, ecm_eth_dev->dev_addr, MAX_ADDR_LEN);
        else
            return OS_ERROR;
        break;

    default:
        break;
    }

    return OS_EOK;
}

const static struct os_device_ops ecm_device_ops =
{
    os_ecm_eth_init,
    os_ecm_eth_open,
    os_ecm_eth_close,
    os_ecm_eth_read,
    os_ecm_eth_write,
    os_ecm_eth_control
};

struct pbuf *os_ecm_eth_rx(os_device_t *dev)
{
    struct pbuf *p           = OS_NULL;
    os_uint32_t  offset      = 0;
    os_ecm_eth_t ecm_eth_dev = (os_ecm_eth_t)dev;
    if (ecm_eth_dev->rx_size != 0)
    {
        /* Allocate buffer */
        p = pbuf_alloc(PBUF_RAW, ecm_eth_dev->rx_size, PBUF_RAM);
        if (p != OS_NULL)
        {
            struct pbuf *q;

            for (q = p; q != OS_NULL; q = q->next)
            {
                /* Copy the received frame into buffer from memory pointed by the current ETHERNET DMA Rx descriptor */
                memcpy(q->payload, (os_uint8_t *)((ecm_eth_dev->rx_buffer) + offset), q->len);
                offset += q->len;
            }
        }
    }

    {
        if (ecm_eth_dev->func->device->state == USB_STATE_CONFIGURED)
        {
            ecm_eth_dev->rx_size                      = 0;
            ecm_eth_dev->rx_offset                    = 0;
            ecm_eth_dev->eps.ep_out->request.buffer   = ecm_eth_dev->eps.ep_out->buffer;
            ecm_eth_dev->eps.ep_out->request.size     = EP_MAXPACKET(ecm_eth_dev->eps.ep_out);
            ecm_eth_dev->eps.ep_out->request.req_type = UIO_REQUEST_READ_BEST;
            os_usbd_io_request(ecm_eth_dev->func->device, ecm_eth_dev->eps.ep_out, &ecm_eth_dev->eps.ep_out->request);
        }
    }

    return p;
}

os_err_t os_ecm_eth_tx(os_device_t *dev, struct pbuf *p)
{
    struct pbuf *q;
    char        *pbuffer;
    os_err_t     result      = OS_EOK;
    os_ecm_eth_t ecm_eth_dev = (os_ecm_eth_t)dev;

    if (!ecm_eth_dev->parent.link_status)
    {
        LOG_EXT_D("linkdown, drop pkg");
        return OS_EOK;
    }

    if (p->tot_len > USB_ETH_MTU)
    {
        LOG_W(DBG_TAG,"ECM MTU is:%d, but the send packet size is %d", USB_ETH_MTU, p->tot_len);
        p->tot_len = USB_ETH_MTU;
    }

    result = os_sem_wait(&device->tx_buffer_free, os_tick_from_ms(1000));
    if (result != OS_EOK)
    {
        LOG_W(DBG_TAG,"wait for buffer free timeout");
        /* If cost 1s to wait send done it said that connection is close . drop it */
        os_sem_post(&device->tx_buffer_free);
        return result;
    }

    pbuffer = (char *)&ecm_eth_dev->tx_buffer;
    for (q = p; q != NULL; q = q->next)
    {
        memcpy(pbuffer, q->payload, q->len);
        pbuffer += q->len;
    }

    {
        if (ecm_eth_dev->func->device->state == USB_STATE_CONFIGURED)
        {
            ecm_eth_dev->eps.ep_in->request.buffer   = (void *)&ecm_eth_dev->tx_buffer;
            ecm_eth_dev->eps.ep_in->request.size     = p->tot_len;
            ecm_eth_dev->eps.ep_in->request.req_type = UIO_REQUEST_WRITE;
            os_usbd_io_request(ecm_eth_dev->func->device, ecm_eth_dev->eps.ep_in, &ecm_eth_dev->eps.ep_in->request);
        }
    }

    return result;
}

static os_err_t _ep_cmd_handler(ufunction_t func, os_size_t size)
{
    return OS_EOK;
}

static os_err_t _function_enable(ufunction_t func)
{
    cdc_eps_t    eps;
    os_ecm_eth_t ecm_device = (os_ecm_eth_t)func->user_data;

    LOG_EXT_D("plugged in");

    eps                 = (cdc_eps_t)&ecm_device->eps;
    eps->ep_out->buffer = ecm_device->rx_pool;

    /* Reset eth rx tx */
    ecm_device->rx_size   = 0;
    ecm_device->rx_offset = 0;

    eps->ep_out->request.buffer   = (void *)eps->ep_out->buffer;
    eps->ep_out->request.size     = EP_MAXPACKET(eps->ep_out);
    eps->ep_out->request.req_type = UIO_REQUEST_READ_BEST;
    os_usbd_io_request(func->device, eps->ep_out, &eps->ep_out->request);
    return OS_EOK;
}

static os_err_t _function_disable(ufunction_t func)
{
    LOG_EXT_D("plugged out");

    eth_device_linkchange(&((os_ecm_eth_t)func->user_data)->parent, OS_FALSE);

    /* Reset eth rx tx */
    ((os_ecm_eth_t)func->user_data)->rx_size   = 0;
    ((os_ecm_eth_t)func->user_data)->rx_offset = 0;

    return OS_EOK;
}

static struct ufunction_ops ops = {
    _function_enable,
    _function_disable,
    OS_NULL,
};

static os_err_t _cdc_descriptor_config(ucdc_comm_desc_t comm,
                                       os_uint8_t       cintf_nr,
                                       ucdc_data_desc_t data,
                                       os_uint8_t       dintf_nr,
                                       os_uint8_t       device_is_hs)
{
    comm->call_mgmt_desc.data_interface = dintf_nr;
    comm->union_desc.master_interface   = cintf_nr;
    comm->union_desc.slave_interface0   = dintf_nr;
#ifdef OS_USB_DEVICE_COMPOSITE
    comm->iad_desc.bFirstInterface = cintf_nr;
#endif
    data->ep_out_desc.wMaxPacketSize = device_is_hs ? 512 : 64;
    data->ep_in_desc.wMaxPacketSize  = device_is_hs ? 512 : 64;
    return OS_EOK;
}

ufunction_t os_usbd_function_ecm_create(udevice_t device)
{
    ufunction_t      cdc;
    os_ecm_eth_t     _ecm_eth;
    cdc_eps_t        eps;
    uintf_t          intf_comm, intf_data;
    ualtsetting_t    comm_setting, data_setting;
    ucdc_data_desc_t data_desc;
    ucdc_eth_desc_t  comm_desc;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Set usb device string description */
    os_usbd_device_set_string(device, _ustring);

    /* Create a cdc class */
    cdc = os_usbd_function_new(device, &_dev_desc, &ops);
    os_usbd_device_set_qualifier(device, &dev_qualifier);
    _ecm_eth = os_calloc(1, sizeof(struct os_ecm_eth));
    memset(_ecm_eth, 0, sizeof(struct os_ecm_eth));
    cdc->user_data = _ecm_eth;

    _ecm_eth->func = cdc;
    /* Create a cdc class endpoints collection */
    eps = &_ecm_eth->eps;
    /* Create a cdc communication interface and a cdc data interface */
    intf_comm = os_usbd_interface_new(device, _interface_handler);
    intf_data = os_usbd_interface_new(device, _interface_handler);

    /* Create a communication alternate setting and a data alternate setting */
    comm_setting = os_usbd_altsetting_new(sizeof(struct ucdc_eth_descriptor));
    data_setting = os_usbd_altsetting_new(sizeof(struct ucdc_data_descriptor));

    /* Config desc in alternate setting */
    os_usbd_altsetting_config_descriptor(comm_setting, &_comm_desc, (os_off_t) & ((ucdc_eth_desc_t)0)->intf_desc);
    os_usbd_altsetting_config_descriptor(data_setting, &_data_desc, 0);
    /* Configure the cdc interface descriptor */
    _cdc_descriptor_config(comm_setting->desc,
                           intf_comm->intf_num,
                           data_setting->desc,
                           intf_data->intf_num,
                           device->dcd->device_is_hs);

    /* Create a command endpoint */
    comm_desc   = (ucdc_eth_desc_t)comm_setting->desc;
    eps->ep_cmd = os_usbd_endpoint_new(&comm_desc->ep_desc, _ep_cmd_handler);
    /* Add the command endpoint to the cdc communication interface */
    os_usbd_altsetting_add_endpoint(comm_setting, eps->ep_cmd);

    /* Add the communication alternate setting to the communication interface,
       then set default setting of the interface */
    os_usbd_interface_add_altsetting(intf_comm, comm_setting);
    os_usbd_set_altsetting(intf_comm, 0);
    /* Add the communication interface to the cdc class */
    os_usbd_function_add_interface(cdc, intf_comm);

    /* Create a bulk in and a bulk out endpoint */
    data_desc   = (ucdc_data_desc_t)data_setting->desc;
    eps->ep_out = os_usbd_endpoint_new(&data_desc->ep_out_desc, _ep_out_handler);
    eps->ep_in  = os_usbd_endpoint_new(&data_desc->ep_in_desc, _ep_in_handler);

    /* Add the bulk out and bulk in endpoints to the data alternate setting */
    os_usbd_altsetting_add_endpoint(data_setting, eps->ep_in);
    os_usbd_altsetting_add_endpoint(data_setting, eps->ep_out);

    /* Add the data alternate setting to the data interface
            then set default setting of the interface */
    os_usbd_interface_add_altsetting(intf_data, data_setting);
    os_usbd_set_altsetting(intf_data, 0);

    /* Add the cdc data interface to cdc class */
    os_usbd_function_add_interface(cdc, intf_data);

    os_sem_init(&_ecm_eth->tx_buffer_free, "ue_tx", 1, OS_IPC_FLAG_FIFO);
    /* OUI 00-00-00, only for test. */
    _ecm_eth->dev_addr[0] = 0x34;
    _ecm_eth->dev_addr[1] = 0x97;
    _ecm_eth->dev_addr[2] = 0xF6;
    /* Generate random MAC. */
    _ecm_eth->dev_addr[3] = 0x94;
    _ecm_eth->dev_addr[4] = 0xEC;
    _ecm_eth->dev_addr[5] = 0xAC;
    /* OUI 00-00-00, only for test. */
    _ecm_eth->host_addr[0] = 0x34;
    _ecm_eth->host_addr[1] = 0x97;
    _ecm_eth->host_addr[2] = 0xF6;
    /* Generate random MAC. */
    _ecm_eth->host_addr[3] = 0x94;
    _ecm_eth->host_addr[4] = 0xEC;
    _ecm_eth->host_addr[5] = 0xAB;
    
    _ecm_eth->parent.parent.ops = &ecm_device_ops;
    _ecm_eth->parent.parent.user_data = device;

    _ecm_eth->parent.eth_rx = os_ecm_eth_rx;
    _ecm_eth->parent.eth_tx = os_ecm_eth_tx;
    /* Register eth device */
    eth_device_init(&_ecm_eth->parent, "u0");

    /* Send link up. */
    eth_device_linkchange(&_ecm_eth->parent, OS_FALSE);

    return cdc;
}

struct udclass ecm_class = {.os_usbd_function_create = os_usbd_function_ecm_create};

int os_usbd_ecm_class_register(void)
{
    os_usbd_class_register(&ecm_class);
    return 0;
}
OS_PREV_INIT(os_usbd_ecm_class_register);

#endif /* OS_USB_DEVICE_ECM */
