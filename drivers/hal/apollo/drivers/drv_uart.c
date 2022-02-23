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
 * @file        drv_uart.c
 *
 * @brief       This file implements uart driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_memory.h>

#include <string.h>

#include "am_mcu_apollo.h"
#include "bsp.h"

/* AM uart driver */
struct apollo_uart
{
    struct os_serial_device serial;

    UART_HandleTypeDef *huart;

    os_list_node_t list;
};

static os_list_node_t am_uart_list = OS_LIST_INIT(am_uart_list);

static void GPIO_Configuration(void)
{
#if defined(BSP_USING_UART0)
    /* Make sure the UART RX and TX pins are enabled */
    am_hal_gpio_pin_config(UART0_GPIO_TX, UART0_GPIO_CFG_TX | AM_HAL_GPIO_PULL24K);
    am_hal_gpio_pin_config(UART0_GPIO_RX, UART0_GPIO_CFG_RX | AM_HAL_GPIO_PULL24K);
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
    /* Make sure the UART RX and TX pins are enabled */
    am_hal_gpio_pin_config(UART1_GPIO_TX, UART1_GPIO_CFG_TX | AM_HAL_GPIO_PULL24K);
    am_hal_gpio_pin_config(UART1_GPIO_RX, UART1_GPIO_CFG_RX | AM_HAL_GPIO_PULL24K);
#endif /* BSP_USING_UART1 */
}

static void RCC_Configuration(UART_HandleTypeDef *uart)
{
    /* Power on the selected UART */
    am_hal_uart_pwrctrl_enable(uart->uart_device);

    /* Start the UART interface, apply the desired configuration settings */
    am_hal_uart_clock_enable(uart->uart_device);

    /* Disable the UART before configuring it */
    am_hal_uart_disable(uart->uart_device);
}

static os_err_t am_uart_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct apollo_uart *a_uart;
    UART_HandleTypeDef *     uart;
    am_hal_uart_config_t uart_cfg;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    
    a_uart = os_container_of(serial, struct apollo_uart, serial);
    OS_ASSERT(a_uart != OS_NULL);
    
    uart = a_uart->huart;
    OS_ASSERT(uart != OS_NULL);
    
    GPIO_Configuration();
    RCC_Configuration(uart);

    memset(&uart_cfg, 0, sizeof(uart_cfg));

    /* Get the configure */
    uart_cfg.ui32BaudRate = cfg->baud_rate;

    if (cfg->data_bits == DATA_BITS_5)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_5;
    else if (cfg->data_bits == DATA_BITS_6)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_6;
    else if (cfg->data_bits == DATA_BITS_7)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_7;
    else if (cfg->data_bits == DATA_BITS_8)
        uart_cfg.ui32DataBits = AM_HAL_UART_DATA_BITS_8;

    if (cfg->stop_bits == STOP_BITS_1)
        uart_cfg.bTwoStopBits = false;
    else if (cfg->stop_bits == STOP_BITS_2)
        uart_cfg.bTwoStopBits = true;

    if (cfg->parity == PARITY_NONE)
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_NONE;
    else if (cfg->parity == PARITY_ODD)
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_ODD;
    else if (cfg->parity == PARITY_EVEN)
        uart_cfg.ui32Parity = AM_HAL_UART_PARITY_EVEN;

    uart_cfg.ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE;

    /* UART Config */
    am_hal_uart_config(uart->uart_device, &uart_cfg);

    /* Enable the UART FIFO */
    //am_hal_uart_fifo_config(uart->uart_device, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

    /* Enable the uart interrupt in the NVIC */
    am_hal_interrupt_enable(uart->uart_interrupt);
    am_hal_interrupt_priority_set(uart->uart_interrupt,10);
    /* Enable the UART */
    am_hal_uart_enable(uart->uart_device);
    return OS_EOK;
}
static void uart_isr(struct os_serial_device *serial)
{
    uint32_t        status;
    struct apollo_uart *a_uart;
    UART_HandleTypeDef *     uart;

    OS_ASSERT(serial != OS_NULL);
    a_uart = os_container_of(serial, struct apollo_uart, serial);
    OS_ASSERT(a_uart != OS_NULL);
    
    uart = a_uart->huart;
    OS_ASSERT(uart != OS_NULL);

    /* Read the interrupt status */
    status = am_hal_uart_int_status_get(uart->uart_device, true);

    /* Clear the UART interrupt */
    am_hal_uart_int_clear(uart->uart_device, status);

    if (status & (AM_HAL_UART_INT_RX_TMOUT))
    {
        //os_hw_serial_isr_rxdone(serial, 1);
    }

    if (status & AM_HAL_UART_INT_RX)
    {
        am_hal_uart_char_receive_polled(uart->uart_device, (char *)uart->buff);
        os_hw_serial_isr_rxdone(serial, 1);
    }

    if (status & AM_HAL_UART_INT_TX)
    {
        os_hw_serial_isr_txdone(serial);
    }
    
}

static int am_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct apollo_uart *a_uart;
    UART_HandleTypeDef *     uart;

    OS_ASSERT(serial != OS_NULL);
    a_uart = os_container_of(serial, struct apollo_uart, serial);
    OS_ASSERT(a_uart != OS_NULL);
    
    uart = a_uart->huart;
    OS_ASSERT(uart != OS_NULL);
    
    if(!(am_hal_uart_int_enable_get(uart->uart_device) & AM_HAL_UART_INT_TX))
    {
        am_hal_uart_int_enable(uart->uart_device,AM_HAL_UART_INT_TX);
    }
    am_hal_uart_char_transmit_polled(uart->uart_device,(char)*buff);
    /*for (i = 0; i < size; i++)
    {
        if(i == (size-1))
        {
            am_hal_uart_int_enable(uart->uart_device,AM_HAL_UART_INT_TX);
        }
        am_hal_uart_char_transmit_polled(uart->uart_device, buff[i]);
    }
    return size;
    */

    return 1;
}

static int am_uart_stop_send(struct os_serial_device *serial)
{
    struct apollo_uart *a_uart;
    UART_HandleTypeDef *     uart;

    OS_ASSERT(serial != OS_NULL);
    a_uart = os_container_of(serial, struct apollo_uart, serial);
    OS_ASSERT(a_uart != OS_NULL);
    
    uart = a_uart->huart;
    OS_ASSERT(uart != OS_NULL);

     am_hal_uart_int_disable(uart->uart_device, AM_HAL_UART_INT_TX);

    return 0;
}

static int am_uart_start_recv(struct os_serial_device *serial, os_uint8_t *buff, os_size_t size)
{
    struct apollo_uart *a_uart;
    UART_HandleTypeDef *     uart;

    OS_ASSERT(serial != OS_NULL);
    a_uart = os_container_of(serial, struct apollo_uart, serial);
    OS_ASSERT(a_uart != OS_NULL);
    
    uart = a_uart->huart;
    OS_ASSERT(uart != OS_NULL);
/*   if(am_hal_uart_int_status_get(uart->uart_device, true) & AM_HAL_UART_INT_RX)
    {
        am_hal_uart_char_receive_polled(uart->uart_device, (char *)buff);
        return 1;
    }*/
    uart->buff = buff;
    if(!(am_hal_uart_int_enable_get(uart->uart_device) & AM_HAL_UART_INT_RX))
    {
        am_hal_uart_int_enable(uart->uart_device,AM_HAL_UART_INT_RX);
        return 0;
    }
    return 0;
}

static int am_uart_stop_recv(struct os_serial_device *serial)
{
    struct apollo_uart *a_uart;
    UART_HandleTypeDef *     uart;

    OS_ASSERT(serial != OS_NULL);
    a_uart = os_container_of(serial, struct apollo_uart, serial);
    OS_ASSERT(a_uart != OS_NULL);
    
    uart = a_uart->huart;
    OS_ASSERT(uart != OS_NULL);

    am_hal_uart_int_disable(uart->uart_device,AM_HAL_UART_INT_RX);

    return 0;
}


static int am_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;    
    struct apollo_uart *a_uart;
    UART_HandleTypeDef *     uart;

    OS_ASSERT(serial != OS_NULL);
    a_uart = os_container_of(serial, struct apollo_uart, serial);
    OS_ASSERT(a_uart != OS_NULL);
    
    uart = a_uart->huart;
    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        am_hal_uart_char_transmit_polled(uart->uart_device, buff[i]);
    }

    return size;
}


static const struct os_uart_ops am_uart_ops =
{
    .init         = am_uart_configure,

    .start_send   = am_uart_start_send,
    .stop_send    = am_uart_stop_send,

    .start_recv   = am_uart_start_recv,
    .stop_recv    = am_uart_stop_recv,
    
    .poll_send    = am_uart_poll_send,
};

#if defined(BSP_USING_UART0)
/* UART0 device driver structure */
void am_uart_isr(void)
{
    struct apollo_uart *am_uart;

//    am_uart = os_container_of(&huart0, struct apollo_uart, huart);
    os_list_for_each_entry(am_uart, &am_uart_list, struct apollo_uart, list)
    {
        if (am_uart->huart == &huart0)
        {
            uart_isr(&am_uart->serial);
            break;
        }
    }
}
#ifdef SOC_APOLLO2_XXX
void am_uart0_isr(void)
{
	am_uart_isr();
}
#endif
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
/* UART1 device driver structure */
void am_uart1_isr(void)
{
    struct apollo_uart *am_uart;

    os_list_for_each_entry(am_uart, &am_uart_list, struct apollo_uart, list)
    {
        if (am_uart->huart == &huart1)
        {
            uart_isr(&am_uart->serial);
            break;
        }
    }
}
#endif /* BSP_USING_UART1 */


static int am_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;

    struct apollo_uart *uart = (struct apollo_uart *)os_calloc(1, sizeof(struct apollo_uart));

    OS_ASSERT(uart);

    uart->huart = (UART_HandleTypeDef *)dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &am_uart_ops;
    serial->config = config;

    level = os_irq_lock();
    os_list_add_tail(&am_uart_list, &uart->list);
    os_irq_unlock(level);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}


void __os_hw_console_output(char *str)
{
    (void)str;
}

OS_DRIVER_INFO am_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = am_usart_probe,
};

OS_DRIVER_DEFINE(am_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

