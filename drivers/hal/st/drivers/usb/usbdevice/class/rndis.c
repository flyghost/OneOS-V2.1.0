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
 * @file        rndis.c
 *
 * @brief       This file provides functions for rndis.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#ifdef OS_USB_DEVICE_RNDIS
#include "cdc.h"
#include "rndis.h"
#include "ndis.h"

/* Define RNDIS_DELAY_LINK_UP by menuconfig for delay linkup */

#define DRV_EXT_LVL      DBG_WARNING
#define DBG_SECTION_NAME "RNDIS"
#include <drv_log.h>

/* LWIP ethernet interface */
#include <netif/ethernetif.h>

struct os_rndis_response
{
    struct os_list_node list;
    const void         *buffer;
};

#define MAX_ADDR_LEN 6
struct os_rndis_eth
{
    /* Inherit from ethernet device */
    struct eth_device parent;
    struct ufunction *func;
    /* Interface address info */
    os_uint8_t host_addr[MAX_ADDR_LEN];
    os_uint8_t dev_addr[MAX_ADDR_LEN];

#ifdef RNDIS_DELAY_LINK_UP
    struct os_timer timer;
#endif /* RNDIS_DELAY_LINK_UP */

    OS_ALIGN(4)
    os_uint8_t rx_pool[512];
    OS_ALIGN(4)
    os_uint8_t tx_pool[512];

    os_uint32_t cmd_pool[2];
    OS_ALIGN(4)
    char      rx_buffer[sizeof(struct rndis_packet_msg) + USB_ETH_MTU + 14];
    os_size_t rx_offset;
    os_size_t rx_length;
    os_bool_t rx_flag;
    os_bool_t rx_frist;

    OS_ALIGN(4)
    char                tx_buffer[sizeof(struct rndis_packet_msg) + USB_ETH_MTU + 14];
    struct os_semaphore tx_buffer_free;

    struct os_list_node response_list;
    os_bool_t           need_notify;
    struct cdc_eps      eps;
};
typedef struct os_rndis_eth *os_rndis_eth_t;
static os_uint32_t           oid_packet_filter = 0x0000000;

OS_ALIGN(4)
static struct udevice_descriptor _dev_desc =
{
    USB_DESC_LENGTH_DEVICE,   /* bLength */
    USB_DESC_TYPE_DEVICE,     /* type */
    USB_BCD_VERSION,          /* bcdUSB */
    0xEF,                     /* bDeviceClass */
    0x04,                     /* bDeviceSubClass */
    0x01,                     /* bDeviceProtocol */
    USB_CDC_BUFSIZE,          /* bMaxPacketSize0 */
    _VENDOR_ID,               /* idVendor */
    _PRODUCT_ID,              /* idProduct */
    USB_BCD_DEVICE,           /* bcdDevice */
    USB_STRING_MANU_INDEX,    /* iManufacturer */
    USB_STRING_PRODUCT_INDEX, /* iProduct */
    USB_STRING_SERIAL_INDEX,  /* iSerialNumber */
    USB_DYNAMIC               /* bNumConfigurations */
};

/* Communcation interface descriptor */
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
        USB_CDC_PROTOCOL_VENDOR,
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
        USB_CDC_PROTOCOL_VENDOR,
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
        USB_DIR_IN | USB_DYNAMIC,
        USB_EP_ATTR_INT,
        0x08,
        0x0A,
    },
};

/* Data interface descriptor */
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
const static char* _ustring[] =
{
    "Language",                 /* LANGID */
    "OS Team.",          /* MANU */
    "OS RNDIS device",   /* PRODUCT */
    "1.1.0",                    /* SERIAL */
    "Configuration",            /* CONFIG */
    "Interface",                /* INTERFACE */
    USB_STRING_OS
};

OS_ALIGN(4)
struct usb_os_function_comp_id_descriptor rndis_func_comp_id_desc =
{
    .bFirstInterfaceNumber = USB_DYNAMIC,
    .reserved1             = 0x01,
    .compatibleID          = {'R', 'N', 'D', 'I', 'S', 0x00, 0x00, 0x00},
    .subCompatibleID       = {'5', '1', '6', '2', '0', '0', '1', 0x00},
    .reserved2             = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

/* FS and HS needed */
OS_ALIGN(4)
static struct usb_qualifier_descriptor dev_qualifier =
{
    sizeof(dev_qualifier),         /* bLength */
    USB_DESC_TYPE_DEVICEQUALIFIER, /* bDescriptorType */
    0x0200,                        /* bcdUSB */
    USB_CLASS_CDC,                 /* bDeviceClass */
    USB_CDC_SUBCLASS_ACM,          /* bDeviceSubClass */
    USB_CDC_PROTOCOL_VENDOR,       /* bDeviceProtocol */
    64,                            /* bMaxPacketSize0 */
    0x01,                          /* bNumConfigurations */
    0,
};

/* Supported OIDs */
OS_ALIGN(4)
const static os_uint32_t oid_supported_list[] =
{
    /* General OIDs */
    OID_GEN_SUPPORTED_LIST,
    OID_GEN_HARDWARE_STATUS,
    OID_GEN_MEDIA_SUPPORTED,
    OID_GEN_MEDIA_IN_USE,
    OID_GEN_MAXIMUM_FRAME_SIZE,
    OID_GEN_LINK_SPEED,
    OID_GEN_TRANSMIT_BLOCK_SIZE,
    OID_GEN_RECEIVE_BLOCK_SIZE,
    OID_GEN_VENDOR_ID,
    OID_GEN_VENDOR_DESCRIPTION,
    OID_GEN_VENDOR_DRIVER_VERSION,
    OID_GEN_CURRENT_PACKET_FILTER,
    OID_GEN_MAXIMUM_TOTAL_SIZE,
    OID_GEN_MEDIA_CONNECT_STATUS,

    OID_GEN_PHYSICAL_MEDIUM,

    /* General Statistic OIDs */
    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER,

    /* Please configure us */
    OID_GEN_RNDIS_CONFIG_PARAMETER,

    /* 802.3 OIDs */
    OID_802_3_PERMANENT_ADDRESS,
    OID_802_3_CURRENT_ADDRESS,
    OID_802_3_MULTICAST_LIST,
    OID_802_3_MAXIMUM_LIST_SIZE,

    /* 802.3 Statistic OIDs */
    OID_802_3_RCV_ERROR_ALIGNMENT,
    OID_802_3_XMIT_ONE_COLLISION,
    OID_802_3_XMIT_MORE_COLLISIONS,

    OID_802_3_MAC_OPTIONS,
};

static os_uint8_t rndis_message_buffer[RNDIS_MESSAGE_BUFFER_SIZE];

static void _rndis_response_available(ufunction_t func)
{
    os_rndis_eth_t device = (os_rndis_eth_t)func->user_data;
    os_uint32_t   *data;
    if (device->need_notify == OS_TRUE)
    {
        device->need_notify                  = OS_FALSE;
        data                                 = (os_uint32_t *)device->eps.ep_cmd->buffer;
        data[0]                              = RESPONSE_AVAILABLE;
        data[1]                              = 0;
        device->eps.ep_cmd->request.buffer   = device->eps.ep_cmd->buffer;
        device->eps.ep_cmd->request.size     = 8;
        device->eps.ep_cmd->request.req_type = UIO_REQUEST_WRITE;
        os_usbd_io_request(func->device, device->eps.ep_cmd, &device->eps.ep_cmd->request);
    }
}

static os_err_t _rndis_init_response(ufunction_t func, rndis_init_msg_t msg)
{
    rndis_init_cmplt_t        resp;
    struct os_rndis_response *response;

    response = os_calloc(1, sizeof(struct os_rndis_response));
    resp     = os_calloc(1, sizeof(struct rndis_init_cmplt));

    if ((response == OS_NULL) || (resp == OS_NULL))
    {
        LOG_E(DBG_TAG,"%s,%d: no memory!", __func__, __LINE__);

        if (response != OS_NULL)
            os_free(response);

        if (resp != OS_NULL)
            os_free(resp);

        return OS_ENOMEM;
    }

    resp->RequestId             = msg->RequestId;
    resp->MessageType           = REMOTE_NDIS_INITIALIZE_CMPLT;
    resp->MessageLength         = sizeof(struct rndis_init_cmplt);
    resp->MajorVersion          = RNDIS_MAJOR_VERSION;
    resp->MinorVersion          = RNDIS_MAJOR_VERSION;
    resp->Status                = RNDIS_STATUS_SUCCESS;
    resp->DeviceFlags           = RNDIS_DF_CONNECTIONLESS;
    resp->Medium                = RNDIS_MEDIUM_802_3;
    resp->MaxPacketsPerTransfer = 1;
    resp->MaxTransferSize       = USB_ETH_MTU + 58; /* Space for 1280 IP buffer, Ethernet Header */
    resp->PacketAlignmentFactor = 3;
    resp->AfListOffset          = 0;
    resp->AfListSize            = 0;

    response->buffer = resp;

    {
        os_base_t level = os_irq_lock();
        os_list_add_tail(&((os_rndis_eth_t)func->user_data)->response_list, &response->list);
        os_irq_unlock(level);
    }

    return OS_EOK;
}

static rndis_query_cmplt_t _create_resp(os_size_t size)
{
    rndis_query_cmplt_t resp;

    resp = os_calloc(1, sizeof(struct rndis_query_cmplt) + size);

    if (resp == OS_NULL)
    {
        LOG_E(DBG_TAG,"%s,%d: no memory!", __func__, __LINE__);
        return OS_NULL;
    }

    resp->InformationBufferLength = size;

    return resp;
}

static void _copy_resp(rndis_query_cmplt_t resp, const void *buffer)
{
    char *resp_buffer = (char *)resp + sizeof(struct rndis_query_cmplt);
    memcpy(resp_buffer, buffer, resp->InformationBufferLength);
}

static void _set_resp(rndis_query_cmplt_t resp, os_uint32_t value)
{
    os_uint32_t *response = (os_uint32_t *)((char *)resp + sizeof(struct rndis_query_cmplt));
    *response             = value;
}

static os_err_t _rndis_query_response(ufunction_t func, rndis_query_msg_t msg)
{
    rndis_query_cmplt_t       resp = OS_NULL;
    struct os_rndis_response *response;
    os_err_t                  ret = OS_EOK;

    switch (msg->Oid)
    {
        /*
         * General OIDs
         */
    case OID_GEN_SUPPORTED_LIST:
        resp = _create_resp(sizeof(oid_supported_list));
        if (resp == OS_NULL)
            break;
        _copy_resp(resp, oid_supported_list);
        break;

    case OID_GEN_PHYSICAL_MEDIUM:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, NDIS_MEDIUM_802_3);
        break;

    case OID_GEN_MAXIMUM_FRAME_SIZE:
    case OID_GEN_TRANSMIT_BLOCK_SIZE:
    case OID_GEN_RECEIVE_BLOCK_SIZE:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, USB_ETH_MTU);
        break;

    case OID_GEN_MAXIMUM_TOTAL_SIZE:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, USB_ETH_MTU + RNDIS_MESSAGE_BUFFER_SIZE);
        break;

    case OID_GEN_LINK_SPEED:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, (func->device->dcd->device_is_hs ? (480UL * 1000 * 1000) : (12UL * 1000 * 1000)) / 100);
        break;

    case OID_GEN_MEDIA_CONNECT_STATUS:
        /* Link_status */
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;

#ifdef RNDIS_DELAY_LINK_UP
        if (((os_rndis_eth_t)func->user_data)->parent.link_status)
        {
            _set_resp(resp, NDIS_MEDIA_STATE_CONNECTED);
        }
        else
        {
            _set_resp(resp, NDIS_MEDIA_STATE_DISCONNECTED);
        }
#else
        _set_resp(resp, NDIS_MEDIA_STATE_CONNECTED);
#endif /* RNDIS_DELAY_LINK_UP */
        break;

    case OID_GEN_VENDOR_ID:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, 0x12345678); /* Only for test */
        break;

    case OID_GEN_VENDOR_DESCRIPTION:
    {
        const char vendor_desc[] = "OS RNDIS";

        resp = _create_resp(sizeof(vendor_desc));
        if (resp == OS_NULL)
            break;
        _copy_resp(resp, vendor_desc);
    }
    break;

    case OID_GEN_VENDOR_DRIVER_VERSION:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, 0x0000200);
        break;

        /* statistics OIDs */
    case OID_GEN_XMIT_OK:
    case OID_GEN_RCV_OK:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, 1);
        break;

    case OID_GEN_XMIT_ERROR:
    case OID_GEN_RCV_ERROR:
    case OID_GEN_RCV_NO_BUFFER:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, 0);
        break;

        /*
         * ieee802.3 OIDs
         */
    case OID_802_3_MAXIMUM_LIST_SIZE:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, 1);
        break;

    case OID_802_3_PERMANENT_ADDRESS:
    case OID_802_3_CURRENT_ADDRESS:
        resp = _create_resp(sizeof(((os_rndis_eth_t)func->user_data)->host_addr));
        if (resp == OS_NULL)
            break;
        _copy_resp(resp, ((os_rndis_eth_t)func->user_data)->host_addr);
        break;

    case OID_802_3_MULTICAST_LIST:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, 0xE000000);
        break;

    case OID_802_3_MAC_OPTIONS:
        resp = _create_resp(4);
        if (resp == OS_NULL)
            break;
        _set_resp(resp, 0);
        break;

    default:
        LOG_W(DBG_TAG,"Not support OID %X", msg->Oid);
        ret = OS_ERROR;
        break;
    }

    response = os_calloc(1, sizeof(struct os_rndis_response));
    if ((response == OS_NULL) || (resp == OS_NULL))
    {
        LOG_E(DBG_TAG,"%s,%d: no memory!", __func__, __LINE__);

        if (response != OS_NULL)
            os_free(response);

        if (resp != OS_NULL)
            os_free(resp);

        return OS_ENOMEM;
    }

    resp->RequestId               = msg->RequestId;
    resp->MessageType             = REMOTE_NDIS_QUERY_CMPLT;
    resp->InformationBufferOffset = 16;

    resp->Status        = RNDIS_STATUS_SUCCESS;
    resp->MessageLength = sizeof(struct rndis_query_cmplt) + resp->InformationBufferLength;

    response->buffer = resp;

    {
        os_base_t level = os_irq_lock();
        os_list_add_tail(&((os_rndis_eth_t)func->user_data)->response_list, &response->list);
        os_irq_unlock(level);
    }

    return ret;
}

static os_err_t _rndis_set_response(ufunction_t func, rndis_set_msg_t msg)
{
    rndis_set_cmplt_t         resp;
    struct os_rndis_response *response;

    response = os_calloc(1, sizeof(struct os_rndis_response));
    resp     = os_calloc(1, sizeof(struct rndis_set_cmplt));

    if ((response == OS_NULL) || (resp == OS_NULL))
    {
        LOG_E(DBG_TAG,"%s,%d: no memory!", __func__, __LINE__);

        if (response != OS_NULL)
            os_free(response);

        if (resp != OS_NULL)
            os_free(resp);

        return OS_ENOMEM;
    }

    resp->RequestId     = msg->RequestId;
    resp->MessageType   = REMOTE_NDIS_SET_CMPLT;
    resp->MessageLength = sizeof(struct rndis_set_cmplt);

    switch (msg->Oid)
    {
    case OID_GEN_CURRENT_PACKET_FILTER:
        oid_packet_filter = *((os_uint32_t *)((os_uint8_t *)&(msg->RequestId) + msg->InformationBufferOffset));
        /* TODO: make complier happy */
        oid_packet_filter = oid_packet_filter;

        LOG_EXT_D("OID_GEN_CURRENT_PACKET_FILTER");

#ifdef RNDIS_DELAY_LINK_UP
        /* link up. */
        os_timer_start(&((os_rndis_eth_t)func->user_data)->timer);
#else
        eth_device_linkchange(&((os_rndis_eth_t)func->user_data)->parent, OS_TRUE);
#endif /* RNDIS_DELAY_LINK_UP */
        break;

    case OID_802_3_MULTICAST_LIST:
        break;

    default:
        LOG_W(DBG_TAG,"Unknow rndis set 0x%02X", msg->Oid);
        resp->Status = RNDIS_STATUS_FAILURE;
        return OS_EOK;
    }

    resp->Status = RNDIS_STATUS_SUCCESS;

    response->buffer = resp;

    {
        os_base_t level = os_irq_lock();
        os_list_add_tail(&((os_rndis_eth_t)func->user_data)->response_list, &response->list);
        os_irq_unlock(level);
    }

    return OS_EOK;
}

static os_err_t _rndis_reset_response(ufunction_t func, rndis_set_msg_t msg)
{
    struct rndis_reset_cmplt *resp;
    struct os_rndis_response *response;

    response = os_calloc(1, sizeof(struct os_rndis_response));
    resp     = os_calloc(1, sizeof(struct rndis_reset_cmplt));

    if ((response == OS_NULL) || (resp == OS_NULL))
    {
        LOG_E(DBG_TAG,"%s,%d: no memory!", __func__, __LINE__);

        if (response != OS_NULL)
            os_free(response);

        if (resp != OS_NULL)
            os_free(resp);

        return OS_ENOMEM;
    }

    /* Reset packet filter */

    oid_packet_filter = 0x0000000;

    /* Link down eth */

    eth_device_linkchange(&((os_rndis_eth_t)func->user_data)->parent, OS_FALSE);

    /* Reset eth rx tx */
    ((os_rndis_eth_t)func->user_data)->rx_frist = OS_TRUE;
    ((os_rndis_eth_t)func->user_data)->rx_flag  = OS_FALSE;

    resp->MessageType     = REMOTE_NDIS_RESET_CMPLT;
    resp->MessageLength   = sizeof(struct rndis_reset_cmplt);
    resp->Status          = RNDIS_STATUS_SUCCESS;
    resp->AddressingReset = 1;

    response->buffer = resp;

    {
        os_base_t level = os_irq_lock();
        os_list_add_tail(&((os_rndis_eth_t)func->user_data)->response_list, &response->list);
        os_irq_unlock(level);
    }

    return OS_EOK;
}

static os_err_t _rndis_keepalive_response(ufunction_t func, rndis_keepalive_msg_t msg)
{
    rndis_keepalive_cmplt_t   resp;
    struct os_rndis_response *response;

    response = os_calloc(1, sizeof(struct os_rndis_response));
    resp     = os_calloc(1, sizeof(struct rndis_keepalive_cmplt));

    if ((response == OS_NULL) || (resp == OS_NULL))
    {
        LOG_E(DBG_TAG,"%s,%d: no memory!", __func__, __LINE__);

        if (response != OS_NULL)
            os_free(response);

        if (resp != OS_NULL)
            os_free(resp);

        return OS_ENOMEM;
    }

    resp->MessageType   = REMOTE_NDIS_KEEPALIVE_CMPLT;
    resp->MessageLength = sizeof(struct rndis_keepalive_cmplt);
    resp->Status        = RNDIS_STATUS_SUCCESS;

    response->buffer = resp;

    {
        os_base_t level = os_irq_lock();
        os_list_add_tail(&((os_rndis_eth_t)func->user_data)->response_list, &response->list);
        os_irq_unlock(level);
    }

    return OS_EOK;
}

static os_err_t _rndis_msg_parser(ufunction_t func, os_uint8_t *msg)
{
    os_err_t ret = OS_ERROR;

    switch (((rndis_gen_msg_t)msg)->MessageType)
    {
    case REMOTE_NDIS_INITIALIZE_MSG:
        LOG_EXT_D("REMOTE_NDIS_INITIALIZE_MSG");
        ret = _rndis_init_response(func, (rndis_init_msg_t)msg);
        break;

    case REMOTE_NDIS_HALT_MSG:
        LOG_EXT_D("REMOTE_NDIS_HALT_MSG");
        /* Link down. */
        eth_device_linkchange(&((os_rndis_eth_t)func->user_data)->parent, OS_FALSE);

        /* Reset eth rx tx */
        ((os_rndis_eth_t)func->user_data)->rx_frist = OS_TRUE;
        ((os_rndis_eth_t)func->user_data)->rx_flag  = OS_FALSE;
        break;

    case REMOTE_NDIS_QUERY_MSG:
        LOG_EXT_D("REMOTE_NDIS_QUERY_MSG");
        ret = _rndis_query_response(func, (rndis_query_msg_t)msg);
        break;

    case REMOTE_NDIS_SET_MSG:
        LOG_EXT_D("REMOTE_NDIS_SET_MSG");
        ret = _rndis_set_response(func, (rndis_set_msg_t)msg);
        break;

    case REMOTE_NDIS_RESET_MSG:
        LOG_EXT_D("REMOTE_NDIS_RESET_MSG");
        ret = _rndis_reset_response(func, (rndis_set_msg_t)msg);
        break;

    case REMOTE_NDIS_KEEPALIVE_MSG:
        LOG_EXT_D("REMOTE_NDIS_KEEPALIVE_MSG");
        ret = _rndis_keepalive_response(func, (rndis_keepalive_msg_t)msg);
        break;

    default:
        LOG_W(DBG_TAG,"not support RNDIS msg %X", ((rndis_gen_msg_t)msg)->MessageType);
        ret = OS_ERROR;
        break;
    }

    if (ret == OS_EOK)
        _rndis_response_available(func);

    return ret;
}

static ufunction_t function = OS_NULL;
static os_err_t    send_encapsulated_command_done(udevice_t device, os_size_t size)
{
    if (function != OS_NULL)
    {
        dcd_ep0_send_status(device->dcd);
        _rndis_msg_parser(function, rndis_message_buffer);
        function = OS_NULL;
    }
    return OS_EOK;
}

static os_err_t _rndis_send_encapsulated_command(ufunction_t func, ureq_t setup)
{
    OS_ASSERT(setup->wLength <= sizeof(rndis_message_buffer));
    function = func;
    os_usbd_ep0_read(func->device, rndis_message_buffer, setup->wLength, send_encapsulated_command_done);

    return OS_EOK;
}

static os_err_t _rndis_get_encapsulated_response(ufunction_t func, ureq_t setup)
{
    rndis_gen_msg_t           msg;
    struct os_rndis_response *response;

    if (os_list_empty(&((os_rndis_eth_t)func->user_data)->response_list))
    {
        LOG_EXT_D("response_list is empty!");
        ((os_rndis_eth_t)func->user_data)->need_notify = OS_TRUE;
        return OS_EOK;
    }

    response = (struct os_rndis_response *)((os_rndis_eth_t)func->user_data)->response_list.next;

    msg = (rndis_gen_msg_t)response->buffer;
    os_usbd_ep0_write(func->device, (void *)msg, msg->MessageLength);

    {
        os_base_t level = os_irq_lock();
        os_list_del(&response->list);
        os_irq_unlock(level);
    }

    os_free((void *)response->buffer);
    os_free(response);

    if (!os_list_empty(&((os_rndis_eth_t)func->user_data)->response_list))
    {
        os_uint32_t *data;

        LOG_I(DBG_TAG,"auto append next response!");
        data    = (os_uint32_t *)((os_rndis_eth_t)func->user_data)->eps.ep_cmd->buffer;
        data[0] = RESPONSE_AVAILABLE;
        data[1] = 0;
        ((os_rndis_eth_t)func->user_data)->eps.ep_cmd->request.buffer =
            ((os_rndis_eth_t)func->user_data)->eps.ep_cmd->buffer;
        ((os_rndis_eth_t)func->user_data)->eps.ep_cmd->request.size     = 8;
        ((os_rndis_eth_t)func->user_data)->eps.ep_cmd->request.req_type = UIO_REQUEST_WRITE;
        os_usbd_io_request(func->device,
                           ((os_rndis_eth_t)func->user_data)->eps.ep_cmd,
                           &((os_rndis_eth_t)func->user_data)->eps.ep_cmd->request);
    }
    else
    {
        ((os_rndis_eth_t)func->user_data)->need_notify = OS_TRUE;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle rndis interface request.
 *
 * @param[in]       func            The usb function object.
 * @param[in]       setup           The setup request.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _interface_handler(ufunction_t func, ureq_t setup)
{
    OS_ASSERT(func != OS_NULL);
    OS_ASSERT(setup != OS_NULL);

    switch (setup->bRequest)
    {
    case CDC_SEND_ENCAPSULATED_COMMAND:
        _rndis_send_encapsulated_command(func, setup);
        break;

    case CDC_GET_ENCAPSULATED_RESPONSE:
        _rndis_get_encapsulated_response(func, setup);
        break;

    default:
        LOG_W(DBG_TAG,"unkown setup->request 0x%02X !", setup->bRequest);
        break;
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle rndis bulk in endpoint request.
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
    os_sem_post(&((os_rndis_eth_t)func->user_data)->tx_buffer_free);
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle RNDIS bulk out endpoint request.
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
    cdc_eps_t eps;
    char     *data = OS_NULL;

    eps  = (cdc_eps_t) & ((os_rndis_eth_t)func->user_data)->eps;
    data = (char *)eps->ep_out->buffer;

    if (((os_rndis_eth_t)func->user_data)->rx_frist == OS_TRUE)
    {
        rndis_packet_msg_t msg = (rndis_packet_msg_t)data;

        ((os_rndis_eth_t)func->user_data)->rx_length = msg->DataLength;
        ((os_rndis_eth_t)func->user_data)->rx_offset = 0;

        if (size >= 44)
        {
            data += sizeof(struct rndis_packet_msg);
            size -= sizeof(struct rndis_packet_msg);
            ((os_rndis_eth_t)func->user_data)->rx_frist = OS_FALSE;
            memcpy(&((os_rndis_eth_t)func->user_data)->rx_buffer[((os_rndis_eth_t)func->user_data)->rx_offset],
                   data,
                   size);
            ((os_rndis_eth_t)func->user_data)->rx_offset += size;
        }
    }
    else
    {
        memcpy(&((os_rndis_eth_t)func->user_data)->rx_buffer[((os_rndis_eth_t)func->user_data)->rx_offset], data, size);
        ((os_rndis_eth_t)func->user_data)->rx_offset += size;
    }

    if (((os_rndis_eth_t)func->user_data)->rx_offset >= ((os_rndis_eth_t)func->user_data)->rx_length)
    {
        ((os_rndis_eth_t)func->user_data)->rx_frist = OS_TRUE;
        ((os_rndis_eth_t)func->user_data)->rx_flag  = OS_TRUE;
        eth_device_ready(&(((os_rndis_eth_t)func->user_data)->parent));
    }
    else
    {
        eps->ep_out->request.buffer   = eps->ep_out->buffer;
        eps->ep_out->request.size     = EP_MAXPACKET(eps->ep_out);
        eps->ep_out->request.req_type = UIO_REQUEST_READ_BEST;
        os_usbd_io_request(func->device, eps->ep_out, &eps->ep_out->request);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will handle RNDIS interrupt in endpoint request.
 *
 * @param[in]       func            The usb function object.
 * @param[in]       size            The request size.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _ep_cmd_handler(ufunction_t func, os_size_t size)
{
#if 0
    _rndis_response_available(func);
#endif
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will run cdc class, it will be called on handle set configuration request.
 *
 * @param[in]       func            The usb function object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _function_enable(ufunction_t func)
{
    cdc_eps_t eps;

    LOG_I(DBG_TAG,"plugged in");

    eps                 = (cdc_eps_t) & ((os_rndis_eth_t)func->user_data)->eps;
    eps->ep_in->buffer  = ((os_rndis_eth_t)func->user_data)->tx_pool;
    eps->ep_out->buffer = ((os_rndis_eth_t)func->user_data)->rx_pool;
    eps->ep_cmd->buffer = (os_uint8_t *)((os_rndis_eth_t)func->user_data)->cmd_pool;

    eps->ep_out->request.buffer   = eps->ep_out->buffer;
    eps->ep_out->request.size     = EP_MAXPACKET(eps->ep_out);
    eps->ep_out->request.req_type = UIO_REQUEST_READ_BEST;
    os_usbd_io_request(func->device, eps->ep_out, &eps->ep_out->request);

    ((os_rndis_eth_t)func->user_data)->rx_flag  = OS_FALSE;
    ((os_rndis_eth_t)func->user_data)->rx_frist = OS_TRUE;

#ifdef RNDIS_DELAY_LINK_UP
    /* Stop link up timer. */
    os_timer_stop(&((os_rndis_eth_t)func->user_data)->timer);
#endif /* RNDIS_DELAY_LINK_UP */

    /* Clean resp chain list. */
    {
        struct os_rndis_response *response;
        os_base_t                 level = os_irq_lock();

        while (!os_list_empty(&((os_rndis_eth_t)func->user_data)->response_list))
        {
            response = (struct os_rndis_response *)((os_rndis_eth_t)func->user_data)->response_list.next;

            os_list_del(&response->list);
            os_free((void *)response->buffer);
            os_free(response);
        }

        ((os_rndis_eth_t)func->user_data)->need_notify = OS_TRUE;
        os_irq_unlock(level);
    }

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will stop cdc class, it will be called on handle set configuration request.
 *
 * @param[in]       func            The usb function object.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
static os_err_t _function_disable(ufunction_t func)
{
    LOG_I(DBG_TAG,"plugged out");

#ifdef RNDIS_DELAY_LINK_UP
    /* Stop link up timer. */
    os_timer_stop(&((os_rndis_eth_t)func->user_data)->timer);
#endif /* RNDIS_DELAY_LINK_UP */

    /* Clean resp chain list. */
    {
        struct os_rndis_response *response;
        os_base_t                 level = os_irq_lock();

        while (!os_list_empty(&((os_rndis_eth_t)func->user_data)->response_list))
        {
            response = (struct os_rndis_response *)((os_rndis_eth_t)func->user_data)->response_list.next;
            LOG_EXT_D("remove resp chain list!");

            os_list_del(&response->list);
            os_free((void *)response->buffer);
            os_free(response);
        }

        ((os_rndis_eth_t)func->user_data)->need_notify = OS_TRUE;
        os_irq_unlock(level);
    }

    /* Link down. */
    eth_device_linkchange(&((os_rndis_eth_t)func->user_data)->parent, OS_FALSE);

    /* Reset eth rx tx */
    ((os_rndis_eth_t)func->user_data)->rx_frist = OS_TRUE;
    ((os_rndis_eth_t)func->user_data)->rx_flag  = OS_FALSE;

    return OS_EOK;
}

static struct ufunction_ops ops =
{
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

#ifdef NET_USING_LWIP
static os_err_t os_rndis_eth_init(os_device_t *dev)
{
    return OS_EOK;
}

static os_err_t os_rndis_eth_open(os_device_t *dev, os_uint16_t oflag)
{
    return OS_EOK;
}

static os_err_t os_rndis_eth_close(os_device_t *dev)
{
    return OS_EOK;
}

static os_size_t os_rndis_eth_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    os_set_errno(OS_ENOSYS);
    return 0;
}

static os_size_t os_rndis_eth_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    os_set_errno(OS_ENOSYS);
    return 0;
}
static os_err_t os_rndis_eth_control(os_device_t *dev, int cmd, void *args)
{
    os_rndis_eth_t rndis_eth_dev = (os_rndis_eth_t)dev;
    switch (cmd)
    {
    case NIOCTL_GADDR:
        /* Get mac address */
        if (args)
            memcpy(args, rndis_eth_dev->dev_addr, MAX_ADDR_LEN);
        else
            return OS_ERROR;
        break;

    default:
        break;
    }

    return OS_EOK;
}

/* Ethernet device interface */

struct pbuf *os_rndis_eth_rx(os_device_t *dev)
{
    struct pbuf   *p      = OS_NULL;
    os_uint32_t    offset = 0;
    os_rndis_eth_t device = (os_rndis_eth_t)dev;
    if (device->rx_flag == OS_FALSE)
    {
        return p;
    }

    if (device->rx_length != 0)
    {
        /* Allocate buffer */
        p = pbuf_alloc(PBUF_LINK, device->rx_length, PBUF_RAM);
        if (p != OS_NULL)
        {
            struct pbuf *q;

            for (q = p; q != OS_NULL; q = q->next)
            {
                /* Copy the received frame into buffer from memory pointed by the current ETHERNET DMA Rx descriptor */
                memcpy(q->payload, (os_uint8_t *)((device->rx_buffer) + offset), q->len);
                offset += q->len;
            }
        }
    }

    {
        device->rx_flag                      = OS_FALSE;
        device->eps.ep_out->request.buffer   = device->eps.ep_out->buffer;
        device->eps.ep_out->request.size     = EP_MAXPACKET(device->eps.ep_out);
        device->eps.ep_out->request.req_type = UIO_REQUEST_READ_BEST;
        os_usbd_io_request(device->func->device, device->eps.ep_out, &device->eps.ep_out->request);
    }

    return p;
}

os_err_t os_rndis_eth_tx(os_device_t *dev, struct pbuf *p)
{
    struct pbuf   *q;
    char          *buffer;
    os_err_t       result = OS_EOK;
    os_rndis_eth_t device = (os_rndis_eth_t)dev;

    if (!device->parent.link_status)
    {
        LOG_I(DBG_TAG,"linkdown, drop pkg");
        return OS_EOK;
    }

    if (p->tot_len > sizeof(device->tx_buffer))
    {
        LOG_W(DBG_TAG,"RNDIS MTU is:%d, but the send packet size is %d", sizeof(device->tx_buffer), p->tot_len);
        p->tot_len = sizeof(device->tx_buffer);
    }

    /* Wait for buffer free. */
    result = os_sem_wait(&device->tx_buffer_free, os_tick_from_ms(1000));
    if (result != OS_EOK)
    {
        LOG_W(DBG_TAG,"wait for buffer free timeout");
        /* If cost 1s to wait send done it said that connection is close . drop it */
        os_sem_post(&device->tx_buffer_free);
        return result;
    }

    buffer = (char *)&device->tx_buffer + sizeof(struct rndis_packet_msg);
    for (q = p; q != NULL; q = q->next)
    {
        memcpy(buffer, q->payload, q->len);
        buffer += q->len;
    }

    /* Send */
    {
        rndis_packet_msg_t msg;

        msg = (rndis_packet_msg_t)&device->tx_buffer;

        msg->MessageType         = REMOTE_NDIS_PACKET_MSG;
        msg->DataOffset          = sizeof(struct rndis_packet_msg) - 8;
        msg->DataLength          = p->tot_len;
        msg->OOBDataLength       = 0;
        msg->OOBDataOffset       = 0;
        msg->NumOOBDataElements  = 0;
        msg->PerPacketInfoOffset = 0;
        msg->PerPacketInfoLength = 0;
        msg->VcHandle            = 0;
        msg->Reserved            = 0;
        msg->MessageLength       = sizeof(struct rndis_packet_msg) + p->tot_len;

        if ((msg->MessageLength & 0x3F) == 0)
        {
            /* Pad a dummy. */
            msg->MessageLength += 1;
        }

        device->eps.ep_in->request.buffer   = (void *)&device->tx_buffer;
        device->eps.ep_in->request.size     = msg->MessageLength;
        device->eps.ep_in->request.req_type = UIO_REQUEST_WRITE;
        os_usbd_io_request(device->func->device, device->eps.ep_in, &device->eps.ep_in->request);
    }

    return result;
}

const static struct os_device_ops rndis_device_ops = 
{
    os_rndis_eth_init,
    os_rndis_eth_open,
    os_rndis_eth_close,
    os_rndis_eth_read,
    os_rndis_eth_write,
    os_rndis_eth_control,
};

#endif /* NET_USING_LWIP */

#ifdef RNDIS_DELAY_LINK_UP

static os_err_t _rndis_indicate_status_msg(ufunction_t func, os_uint32_t status)
{
    rndis_indicate_status_msg_t resp;
    struct os_rndis_response   *response;

    response = os_calloc(1, sizeof(struct os_rndis_response));
    resp     = os_calloc(1, sizeof(struct rndis_indicate_status_msg));

    if ((response == OS_NULL) || (resp == OS_NULL))
    {
        LOG_E(DBG_TAG,"%s,%d: no memory!", __func__, __LINE__);

        if (response != OS_NULL)
            os_free(response);

        if (resp != OS_NULL)
            os_free(resp);

        return OS_ENOMEM;
    }

    resp->MessageType        = REMOTE_NDIS_INDICATE_STATUS_MSG;
    resp->MessageLength      = 20; /* sizeof(struct rndis_indicate_status_msg) */
    resp->Status             = status;
    resp->StatusBufferLength = 0;
    resp->StatusBufferOffset = 0;

    response->buffer = resp;
    {
        os_base_t level = os_irq_lock();
        os_list_add_tail(&((os_rndis_eth_t)func->user_data)->response_list, &response->list);
        os_irq_unlock(level);
    }

    _rndis_response_available(func);

    return OS_EOK;
}

static void timer_timeout(void *parameter)
{
    LOG_I(DBG_TAG,"delay link up!");
    _rndis_indicate_status_msg(((os_rndis_eth_t)parameter)->func, RNDIS_STATUS_MEDIA_CONNECT);
    eth_device_linkchange(&((os_rndis_eth_t)parameter)->parent, OS_TRUE);
}
#endif /* RNDIS_DELAY_LINK_UP */

ufunction_t os_usbd_function_rndis_create(udevice_t device)
{
    ufunction_t      cdc;
    os_rndis_eth_t   _rndis;
    cdc_eps_t        eps;
    uintf_t          intf_comm, intf_data;
    ualtsetting_t    comm_setting, data_setting;
    ucdc_data_desc_t data_desc;
    ucdc_comm_desc_t comm_desc;

    /* Parameter check */
    OS_ASSERT(device != OS_NULL);

    /* Set usb device string description */
    os_usbd_device_set_string(device, _ustring);

    /* Create a cdc class */
    cdc = os_usbd_function_new(device, &_dev_desc, &ops);
    os_usbd_device_set_qualifier(device, &dev_qualifier);
    _rndis = os_calloc(1, sizeof(struct os_rndis_eth));
    memset(_rndis, 0, sizeof(struct os_rndis_eth));
    cdc->user_data = _rndis;

    _rndis->func = cdc;
    /* Create a cdc class endpoints collection */
    eps = &_rndis->eps;
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
    _cdc_descriptor_config(comm_setting->desc,
                           intf_comm->intf_num,
                           data_setting->desc,
                           intf_data->intf_num,
                           device->dcd->device_is_hs);

    /* Create a command endpoint */
    comm_desc   = (ucdc_comm_desc_t)comm_setting->desc;
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

    os_usbd_os_comp_id_desc_add_os_func_comp_id_desc(device->os_comp_id_desc, &rndis_func_comp_id_desc);

#ifdef NET_USING_LWIP

    os_list_init(&_rndis->response_list);
    _rndis->need_notify = OS_TRUE;

    os_sem_init(&_rndis->tx_buffer_free, "ue_tx", 1, OS_IPC_FLAG_FIFO);

#ifdef RNDIS_DELAY_LINK_UP
    os_timer_init(&_rndis->timer,
                  "RNDIS",
                  timer_timeout,
                  _rndis,
                  OS_TICK_PER_SECOND * 2,
                  OS_TIMER_FLAG_ONE_SHOT | OS_TIMER_FLAG_SOFT_TIMER);
#endif /* RNDIS_DELAY_LINK_UP */

    /* OUI 00-00-00, only for test. */
    _rndis->dev_addr[0] = 0x34;
    _rndis->dev_addr[1] = 0x97;
    _rndis->dev_addr[2] = 0xF6;
    /* Generate random MAC. */
    _rndis->dev_addr[3] = 0x94;
    _rndis->dev_addr[4] = 0xEA;
    _rndis->dev_addr[5] = 0x12;
    /* OUI 00-00-00, only for test. */
    _rndis->host_addr[0] = 0x34;
    _rndis->host_addr[1] = 0x97;
    _rndis->host_addr[2] = 0xF6;
    /* Generate random MAC. */
    _rndis->host_addr[3] = 0x94;
    _rndis->host_addr[4] = 0xEA;
    _rndis->host_addr[5] = 0x13;

    _rndis->parent.parent.ops = &rndis_device_ops;
    _rndis->parent.parent.user_data = device;

    _rndis->parent.eth_rx = os_rndis_eth_rx;
    _rndis->parent.eth_tx = os_rndis_eth_tx;

    /* Register eth device */
    eth_device_init(&((os_rndis_eth_t)cdc->user_data)->parent, "u0");

#endif /* NET_USING_LWIP */

    return cdc;
}

struct udclass rndis_class = {.os_usbd_function_create = os_usbd_function_rndis_create};

int os_usbd_rndis_class_register(void)
{
    os_usbd_class_register(&rndis_class);
    return 0;
}
OS_PREV_INIT(os_usbd_rndis_class_register);

#endif /* OS_USB_DEVICE_RNDIS */
