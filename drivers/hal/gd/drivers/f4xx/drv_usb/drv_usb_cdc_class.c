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
 * @file        drv_flash_f3.c
 *
 * @brief        This file provides flash read/write/erase functions for f0.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <string.h>
#include <os_memory.h>
#include "usbh_core.h"
#include "usbh_cdc_core.h"

/*!
    \brief      user operation for host-mode initialization
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_init(void)
{
    os_kprintf("# usb init!\n");
}

/*!
    \brief      user operation for device attached
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_connected(void)
{
    os_kprintf("# usb connected!\n");
}

/*!
    \brief      user operation when unrecoveredError happens
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_unrecovered_error (void)
{
    os_kprintf("# usb error!\n");
}

/*!
    \brief      user operation for device disconnect event
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_disconnected (void)
{
    os_kprintf("# usb disconnect!\n");
}

/*!
    \brief      user operation for reset USB Device
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_reset(void)
{
    os_kprintf("# usb reset!\n");
}

/*!
    \brief      user operation for detectting device speed
    \param[in]  device_speed: device speed
    \param[out] none
    \retval     none
*/
void usbh_user_device_speed_detected(uint32_t device_speed)
{
    os_kprintf("# usb speed detected!\n");
}

/*!
    \brief      user operation when device descriptor is available
    \param[in]  device_desc: device descriptor
    \param[out] none
    \retval     none
*/
void usbh_user_device_desc_available(void *device_desc)
{
    os_kprintf("# usb device desc available!\n");
}

/*!
    \brief      usb device is successfully assigned the Address 
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_address_assigned(void)
{
    os_kprintf("# usb address assigned!\n");
}

/*!
    \brief      user operation when configuration descriptor is available
    \param[in]  cfg_desc: pointer to configuration descriptor
    \param[in]  itf_desc: pointer to interface descriptor
    \param[in]  ep_desc: pointer to endpoint descriptor
    \param[out] none
    \retval     none
*/
void usbh_user_configuration_descavailable(usb_desc_config *cfg_desc,
                                           usb_desc_itf *itf_desc,
                                           usb_desc_ep *ep_desc)
{
    os_kprintf("# usb configuration descavailable!\n");
}

/*!
    \brief      user operation when manufacturer string exists
    \param[in]  manufacturer_string: manufacturer string of usb device
    \param[out] none
    \retval     none
*/
void usbh_user_manufacturer_string(void *manufacturer_string)
{
    os_kprintf("# usb manufacturer:%s!\n", manufacturer_string);
}

/*!
    \brief      user operation when product string exists
    \param[in]  product_string: product string of usb device
    \param[out] none
    \retval     none
*/
void usbh_user_product_string(void *product_string)
{
    os_kprintf("# usb product:%s!\n", product_string);
}

/*!
    \brief      user operatin when serialNum string exists
    \param[in]  serial_num_string: serialNum string of usb device
    \param[out] none
    \retval     none
*/
void usbh_user_serialnum_string(void *serial_num_string)
{
    os_kprintf("# usb serialnum:%s!\n", serial_num_string);
}

/*!
    \brief      user response request is displayed to ask for application jump to class
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_enumeration_finish(void)
{
    os_kprintf("# usb enumeration finish!\n");
}

/*!
    \brief      user operation when device is not supported
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_device_not_supported(void)
{
    os_kprintf("# usb device not supported!\n");
}

/*!
    \brief      user action for application state entry
    \param[in]  none
    \param[out] none
    \retval     user response for user key
*/
usbh_user_status usbh_user_userinput(void)
{
    printf("# usbh user userinput\n");
    return USBH_USER_RESP_OK;
}

/*!
    \brief      user operation for device overcurrent detection event
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_over_current_detected (void)
{
    printf("# usbh over current detected\n");
}

/*!
    \brief      de-init User state and associated variables
    \param[in]  none
    \param[out] none
    \retval     none
*/
void usbh_user_deinit(void)
{
    printf("# usbh deinit\n");
}

/*!
    \brief      USB host user application
    \param[in]  none
    \param[out] none
    \retval     status
*/
int usbh_user_application(void)
{
    //os_kprintf("# usb ready!\n");

    return 0;
}

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

usbh_user_cb user_cb =
{
    usbh_user_init,
    usbh_user_deinit,
    usbh_user_device_connected,
    usbh_user_device_reset,
    usbh_user_device_disconnected,
    usbh_user_over_current_detected,
    usbh_user_device_speed_detected,
    usbh_user_device_desc_available,
    usbh_user_device_address_assigned,
    usbh_user_configuration_descavailable,
    usbh_user_manufacturer_string,
    usbh_user_product_string,
    usbh_user_serialnum_string,
    usbh_user_enumeration_finish,
    usbh_user_userinput,
    usbh_user_application,
    usbh_user_device_not_supported,
    usbh_user_unrecovered_error
};

/* serial */

static void usbh_ll_init(usbh_host *puhost)
{
#ifdef GD32F450
    /* hardware Init */
    usb_gpio_config();
#endif /* GD32F450 */

    usb_rcu_config();

    /* configure GPIO pin used for switching VBUS power and charge pump I/O */
    usb_vbus_config();

    /* register device class */
    usbh_class_register(puhost, &usbh_cdc);

    /* init host library */
    usbh_init(puhost, &user_cb);

    /* enable interrupts */
    //usb_intr_config();
}

struct gd32_cdc_uart
{
    struct os_serial_device serial;

    usbh_host usb_host;

    os_bool_t   rx_isr_enabled;

    os_uint8_t *buff;
    os_size_t   size;
    os_size_t   count;

    os_list_node_t list;
};

static os_list_node_t gd32_cdc_uart_list = OS_LIST_INIT(gd32_cdc_uart_list);

void cdc_receive_data(usbh_host *puhost, uint8_t *data, uint16_t length)
{
    int count;
    os_base_t level;
    struct gd32_cdc_uart *uart;

    os_list_for_each_entry(uart, &gd32_cdc_uart_list, struct gd32_cdc_uart, list)
    {
        if (&uart->usb_host == puhost)
        {
            level = os_hw_interrupt_disable();

            count = min(uart->size - uart->count, length);
            memcpy(uart->buff + uart->count, data, count);
            uart->count += count;

            if (uart->rx_isr_enabled && (uart->count > (uart->size >> 1)))
            {
                uart->rx_isr_enabled = OS_FALSE;
                nvic_irq_disable((uint8_t)USBFS_IRQn);
                os_hw_serial_isr_rxdone((struct os_serial_device *)uart, uart->count);
            }
            
            os_hw_interrupt_enable(level);
            break;
        }
    }
}

static int gd32_cdc_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{    
    struct gd32_cdc_uart *uart = os_container_of(serial, struct gd32_cdc_uart, serial);

    os_base_t level;

    level = os_hw_interrupt_disable();

    uart->buff  = buff;
    uart->size  = size;
    uart->count = 0;

    uart->rx_isr_enabled = OS_TRUE;

    os_hw_interrupt_enable(level);

    usb_intr_config();
    
    return 0;
}

static int gd32_cdc_uart_stop_recv(struct os_serial_device *serial)
{
    struct gd32_cdc_uart *uart = os_container_of(serial, struct gd32_cdc_uart, serial);

    nvic_irq_disable((uint8_t)USBFS_IRQn);
    
    uart->rx_isr_enabled = OS_FALSE;
    
    return 0;
}

static int gd32_cdc_uart_recv_state(struct os_serial_device *serial)
{
    struct gd32_cdc_uart *uart;
    
    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct gd32_cdc_uart, serial);

    if (uart->rx_isr_enabled == OS_TRUE)
    {
        return uart->count;
    }
    else
    {
        return OS_SERIAL_FLAG_RX_IDLE;
    }
}

static int gd32_cdc_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct gd32_cdc_uart *uart;

    uart = os_container_of(serial, struct gd32_cdc_uart, serial);

    cdc_data_send(&uart->usb_host, (uint8_t *)buff, size);

    return size;
}

static const struct os_uart_ops gd32_cdc_uart_ops = {
    .configure    = OS_NULL,

    .start_send   = OS_NULL,
    .stop_send    = OS_NULL,

    .start_recv   = gd32_cdc_uart_start_recv,
    .stop_recv    = gd32_cdc_uart_stop_recv,
    .recv_state   = gd32_cdc_uart_recv_state,
    
    .poll_send    = gd32_cdc_uart_poll_send,
    .poll_recv    = OS_NULL,
};

static int os_usbh_cdc_init(void)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;

    struct gd32_cdc_uart *uart = os_calloc(1, sizeof(struct gd32_cdc_uart));

    OS_ASSERT(uart);

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &gd32_cdc_uart_ops;
    serial->config = config;

    level = os_hw_interrupt_disable();
    os_list_add_tail(&gd32_cdc_uart_list, &uart->list);
    os_hw_interrupt_enable(level);
    
    result = os_hw_serial_register(serial, "uart_cdc", OS_DEVICE_FLAG_RDWR, NULL);
    
    OS_ASSERT(result == OS_EOK);

    usbh_ll_init(&uart->usb_host);

    return result;    
}
OS_ENV_INIT(os_usbh_cdc_init);

