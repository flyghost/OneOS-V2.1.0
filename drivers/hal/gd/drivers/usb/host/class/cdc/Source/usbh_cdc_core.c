/*!
    \file  usbh_cdc_core.c
    \brief USB host CDC class driver

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

#include "usbh_cdc_core.h"
#include <string.h>

#ifdef USB_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment = 4
    #endif
#endif /* USB_HS_INTERNAL_DMA_ENABLED */

__ALIGN_BEGIN uint8_t tx_buf[CDC_BUFFER_SIZE] __ALIGN_END;

#ifdef USB_HS_INTERNAL_DMA_ENABLED
    #if defined ( __ICCARM__ ) /*!< IAR Compiler */
        #pragma data_alignment = 4
    #endif
#endif /* USB_HS_INTERNAL_DMA_ENABLED */

__ALIGN_BEGIN uint8_t rx_buf[CDC_BUFFER_SIZE] __ALIGN_END;


static void cdc_init_txrxparam(usbh_host *puhost);

void cdc_receive_data(usbh_host *puhost, uint8_t *data, uint16_t length);

static void cdc_process_transmission(usbh_host *puhost);

static void cdc_process_reception(usbh_host *puhost);

static usbh_status cdc_interface_init (usbh_host *puhost);

void cdc_interface_deinit (usbh_host *puhost);

static usbh_status cdc_handle(usbh_host *puhost);

static usbh_status cdc_class_request(usbh_host *puhost);

#if (USBH_USE_FREERTOS == 1) || (USBH_USE_RTX == 1)
static osMessageQId *g_cdc_event = OS_NULL;
#endif

usbh_class usbh_cdc = 
{
    USB_CLASS_CDC,
    cdc_interface_init,
    cdc_interface_deinit,
    cdc_class_request,
    cdc_handle
};

/*!
    \brief      init the CDC class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     operation status: response for USB CDC driver intialization
*/
static usbh_status cdc_interface_init (usbh_host *puhost)
{
    usbh_status status = USBH_OK;

    uint8_t interface = usbh_interface_find(&puhost->dev_prop, USB_CLASS_CDC, USB_CDC_SUBCLASS_ACM, USB_CDC_PROTOCOL_AT);
    interface = 3;

    if (g_cdc_event == OS_NULL)
    {
        osMessageQDef(osqueue, 1, uint16_t);
        g_cdc_event = osMessageCreate(osMessageQ(osqueue), NULL);
        osMessagePut(g_cdc_event, USBH_STATE_CHANGED_EVENT, 0);
    }

    if (0xFFU == interface) {
        puhost->usr_cb->dev_not_supported();

        status = USBH_FAIL;
    } else {
        usbh_interface_select(&puhost->dev_prop, interface);

        static usbh_cdc_handler cdc_handler;

        memset(&cdc_handler, 0U, sizeof(usbh_cdc_handler));

        puhost->active_class->class_data = (void *)&cdc_handler;

        usb_desc_ep *ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[0];

        /* collect the notification endpoint address and length */
        if (ep_desc->bEndpointAddress & 0x80U) {
            cdc_handler.cmd_itf.ep_notify = ep_desc->bEndpointAddress;
            cdc_handler.cmd_itf.ep_size_notify = ep_desc->wMaxPacketSize;
        }

        /* allocate the length for host channel number in */
        cdc_handler.cmd_itf.pipe_notify = usbh_pipe_allocate (puhost->data, cdc_handler.cmd_itf.ep_notify);

        /* open channel for in endpoint */
        usbh_pipe_create (puhost->data,
                          &puhost->dev_prop,
                          cdc_handler.cmd_itf.pipe_notify,
                          USB_EPTYPE_INTR,
                          cdc_handler.cmd_itf.ep_size_notify);

        usbh_pipe_toggle_set(puhost->data, cdc_handler.cmd_itf.pipe_notify, 0U);

        //interface = usbh_interface_find(&puhost->dev_prop, USB_CLASS_DATA, USB_CDC_SUBCLASS_RESERVED, USB_CDC_PROTOCOL_NONE);

        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[1];

        if (ep_desc->bEndpointAddress & 0x80U) {
            cdc_handler.data_itf.ep_in = ep_desc->bEndpointAddress;
            cdc_handler.data_itf.ep_size_in = ep_desc->wMaxPacketSize;
        } else {
            cdc_handler.data_itf.ep_out = ep_desc->bEndpointAddress;
            cdc_handler.data_itf.ep_size_out = ep_desc->wMaxPacketSize;
        }

        ep_desc = &puhost->dev_prop.cfg_desc_set.itf_desc_set[interface][0].ep_desc[2];

        if (ep_desc->bEndpointAddress & 0x80U) {
            cdc_handler.data_itf.ep_in = ep_desc->bEndpointAddress;
            cdc_handler.data_itf.ep_size_in = ep_desc->wMaxPacketSize;
        } else {
            cdc_handler.data_itf.ep_out = ep_desc->bEndpointAddress;
            cdc_handler.data_itf.ep_size_out = ep_desc->wMaxPacketSize;
        }

        /* allocate the length for host channel number out */
        cdc_handler.data_itf.pipe_out = usbh_pipe_allocate (puhost->data, cdc_handler.data_itf.ep_out);

        /* allocate the length for host channel number in */
        cdc_handler.data_itf.pipe_in = usbh_pipe_allocate (puhost->data, cdc_handler.data_itf.ep_in);

        /* open channel for OUT endpoint */
        usbh_pipe_create (puhost->data, 
                          &puhost->dev_prop,
                          cdc_handler.data_itf.pipe_out,
                          USB_EPTYPE_BULK,
                          cdc_handler.data_itf.ep_size_out);

        /* open channel for IN endpoint */
        usbh_pipe_create (puhost->data, 
                          &puhost->dev_prop,
                          cdc_handler.data_itf.pipe_in,
                          USB_EPTYPE_BULK,
                          cdc_handler.data_itf.ep_size_in);

        usbh_pipe_toggle_set(puhost->data, cdc_handler.data_itf.ep_out, 0U);
        usbh_pipe_toggle_set(puhost->data, cdc_handler.data_itf.pipe_in, 0U);

        cdc_handler.req_state = CDC_GET_LINE_CODING_RQUEST;

        /* initilise the Tx/Rx params */
        cdc_init_txrxparam(puhost);
    }

    return status;
}

/*!
    \brief      deinit the host channels used for the CDC class
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     none
*/
void cdc_interface_deinit (usbh_host *puhost)
{
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

    /* reset the channel as free */
    if (cdc->cmd_itf.pipe_notify) {
        usb_pipe_halt (puhost->data, cdc->cmd_itf.pipe_notify);
        usbh_pipe_free (puhost->data, cdc->cmd_itf.pipe_notify);

        cdc->cmd_itf.pipe_notify = 0;
    }

    /* reset the channel as free */
    if (cdc->data_itf.pipe_out) {
        usb_pipe_halt (puhost->data, cdc->data_itf.pipe_out);
        usbh_pipe_free (puhost->data, cdc->data_itf.pipe_out);

        cdc->data_itf.pipe_out = 0;
    }

    /* reset the channel as free */
    if (cdc->data_itf.pipe_in) {
        usb_pipe_halt (puhost->data, cdc->data_itf.pipe_in);
        usbh_pipe_free (puhost->data, cdc->data_itf.pipe_in);

        cdc->data_itf.pipe_in = 0;
    }

}

/*!
    \brief      handler CDC class requests
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     usbh_status: response for USB class request
*/
static usbh_status cdc_class_request (usbh_host *puhost)
{
    return USBH_OK; 
}

/*!
    \brief      managing state machine for CDC data transfers
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     usbh_status
*/

static usbh_status cdc_handle (usbh_host *puhost)
{
    usbh_status status = USBH_OK;
    

    //enable receive
    cdc_start_reception(puhost);

    /* call application process */
    puhost->usr_cb->dev_user_app();

    /* handle the transmission */
    cdc_process_transmission(puhost);

    /* always send in packet to device */
    cdc_process_reception(puhost);

    return status;
}

/*!
    \brief      the function is responsible for sending data to the device
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     none
*/
static void cdc_process_transmission(usbh_host *puhost)
{
    static uint32_t len ;
    usb_urb_state urb_status_tx = URB_IDLE;
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

    urb_status_tx = usbh_urbstate_get(puhost->data, cdc->data_itf.pipe_out);

    switch (cdc->tx_param.cdc_cur_state) {
        case CDC_IDLE:
            break;

        case CDC_SEND_DATA:
            if ((urb_status_tx == URB_DONE) || (urb_status_tx == URB_IDLE)) {
                /* check the data length is more then the usbh_cdc_handler.data_itf.length */
                if (cdc->tx_param.data_length > cdc->data_itf.ep_size_out) {
                    len = cdc->data_itf.ep_size_out;

                    /* send the data */
                    usbh_data_send (puhost->data,
                                    cdc->tx_param.prxtx_buff, 
                                    cdc->data_itf.pipe_out, 
                                    len);
                } else {
                    len = cdc->tx_param.data_length;

                    /* send the remaining data */
                    usbh_data_send (puhost->data,
                                    cdc->tx_param.prxtx_buff, 
                                    cdc->data_itf.pipe_out, 
                                    len);
                }

                cdc->tx_param.cdc_cur_state = CDC_DATA_SENT;
            }
            break;

        case CDC_DATA_SENT:
            /* check the status done for transmssion */
            if (urb_status_tx == URB_DONE) {
                /* point to next chunck of data */
                cdc->tx_param.prxtx_buff += len;

                /* decrease the data length */
                cdc->tx_param.data_length -= len;

                if (cdc->tx_param.data_length == 0) {
                    cdc->tx_param.cdc_cur_state = CDC_IDLE;
                    osMessagePut(g_cdc_event, USBH_STATE_CHANGED_EVENT, 0);
                } else {
                    cdc->tx_param.cdc_cur_state = CDC_SEND_DATA;

#ifdef USBH_USE_RTOS
                    osMessagePut(puhost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif
                }
            } else if (urb_status_tx == URB_NOTREADY) {
                /* send the same data */
                usbh_data_send (puhost->data,
                                (cdc->tx_param.prxtx_buff), 
                                cdc->data_itf.pipe_out, 
                                len);
            }
            break;

        default:
            break;
    }
}

/*!
    \brief      the function is responsible for reception of data from the device
    \param[in]  pudev: pointer to usb core instance
    \param[in]  puhost: pointer to usb host
    \param[out] none
    \retval     none
*/
static void cdc_process_reception(usbh_host *puhost)
{
    usb_core_driver *pudev = (usb_core_driver *)puhost->data;
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

repeat:
    if (cdc->rx_enabled == 1) {
        usb_urb_state urb_status_rx = usbh_urbstate_get(pudev, cdc->data_itf.pipe_in);

        switch (cdc->rx_param.cdc_cur_state) {
            case CDC_IDLE:
                /* check the received length lesser then the remaining space available in the buffer */
                if (cdc->rx_param.data_length < (cdc->rx_param.buffer_len - cdc->data_itf.ep_size_in)) {
                    /* receive the data */
                    usbh_data_recev(pudev,
                                    cdc->rx_param.pfill_buff,
                                    cdc->data_itf.pipe_in, 
                                    cdc->data_itf.ep_size_in);

                    /* change the cdc state to USBH_CDC_GET_DATA*/
                    cdc->rx_param.cdc_cur_state = CDC_GET_DATA;

                    pudev->regs.gr->GINTEN |= GINTEN_RXFNEIE;
                }
                break;

            case CDC_GET_DATA:
                /* check the last state of the device is URB_DONE */
                if (urb_status_rx == URB_DONE) {
                    /* move the pointer as well as datalength */
                    cdc->rx_param.data_length += pudev->host.pipe[cdc->data_itf.pipe_in].xfer_count;
                    cdc->rx_param.pfill_buff += pudev->host.pipe[cdc->data_itf.pipe_in].xfer_count;

                    /* Process the recived data */
                    cdc_receive_data(puhost, cdc->rx_param.pempty_buff, cdc->rx_param.data_length);

                    cdc->rx_param.pfill_buff = cdc->rx_param.pempty_buff;
                    cdc->rx_param.data_length = 0;    /* reset the data length to zero */

                    /*change the state od the CDC state*/
                    cdc->rx_param.cdc_cur_state = CDC_IDLE;

                    goto repeat;
                }
                break;

            default:
                break;
        }
    }
}

/*!
    \brief      initialize the transmit and receive buffer and its parameter
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void cdc_init_txrxparam(usbh_host *puhost)
{
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

    /* initialize the transmit buffer and its parameter */
    cdc->tx_param.cdc_cur_state = CDC_IDLE;
    cdc->tx_param.data_length = 0;
    cdc->tx_param.prxtx_buff = tx_buf;

    /* initialize the receive buffer and its parameter */
    cdc->rx_param.cdc_cur_state = CDC_IDLE;
    cdc->rx_param.data_length = 0;
    cdc->rx_param.pfill_buff = rx_buf;
    cdc->rx_param.pempty_buff = rx_buf;
    cdc->rx_param.buffer_len = sizeof(rx_buf);
}

/*!
    \brief      call back function from cdc core layer to redirect the received data on the user out put system
    \param[in]  cdc_data: data structure
    \param[out] none
    \retval     none
*/
OS_WEAK void cdc_receive_data(usbh_host *puhost, uint8_t *data, uint16_t length)
{
    
}

/*!
    \brief      send data to the device
    \param[in]  data: data pointer
    \param[in]  length: data length
    \param[out] none
    \retval     none
*/
void cdc_data_send (usbh_host *puhost, uint8_t *data, uint16_t length)
{
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

#ifdef USBH_USE_RTOS
    if (g_cdc_event == OS_NULL)
        return;

    osEvent event = osMessageGet(g_cdc_event, 50000);
    if (event.status == osEventTimeout)
    {
        return;
    }
#endif

    if (cdc->tx_param.cdc_cur_state == CDC_IDLE) {
        cdc->tx_param.prxtx_buff = data; 
        cdc->tx_param.data_length = length;
        cdc->tx_param.cdc_cur_state = CDC_SEND_DATA;

#ifdef USBH_USE_RTOS
        osMessagePut (puhost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif
    }
}

/*!
    \brief      send dummy data to the device
    \param[in]  none
    \param[out] none
    \retval     none
*/
void cdc_dummydata_send (usbh_host *puhost)
{
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

    static uint8_t cdc_send_buf[17] = {0x43, 0x6F, 0x6E, 0x6E, 0x65, 0x63, 0x74, 0x69, 0x76, 0x69, 0x74, 0x79, 0x20, 0x6C, 0x69, 0x6E, 0x65};

    if (cdc->tx_param.cdc_cur_state == CDC_IDLE) {
        cdc->tx_param.prxtx_buff = cdc_send_buf; 
        cdc->tx_param.data_length = sizeof(cdc_send_buf);
        cdc->tx_param.cdc_cur_state = CDC_SEND_DATA;

#ifdef USBH_USE_RTOS
        osMessagePut (puhost->os_event, USBH_STATE_CHANGED_EVENT, 0);
#endif
    }
}

/*!
    \brief      enable CDC receive
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     none
*/
void cdc_start_reception (usbh_host *puhost)
{
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

    cdc->rx_enabled = 1;
}

/*!
    \brief      stop CDC receive
    \param[in]  pudev: pointer to usb core instance
    \param[out] none
    \retval     none
*/
void cdc_stop_reception (usbh_host *puhost)
{
    usbh_cdc_handler *cdc = (usbh_cdc_handler *)puhost->active_class->class_data;

    cdc->rx_enabled = 0;

    usb_pipe_halt(puhost->data, cdc->data_itf.pipe_in);
    usbh_pipe_free(puhost->data, cdc->data_itf.pipe_in);
}

