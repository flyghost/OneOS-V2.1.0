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
 * @file        drv_usart.c
 *
 * @brief       This file implements usart driver for gd32v.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_usart.h>
#include <os_task.h>
#include "gd32vf103.h"
#include <os_errno.h>
#include <os_assert.h>
#include <drv_cfg.h>
#include <os_memory.h>

#if !defined(OS_USING_USART0) && !defined(OS_USING_USART1) && !defined(OS_USING_USART2)
#error "Please define at least one BSP_USING_UARTx"
#endif

static os_list_node_t gd32_uart_list = OS_LIST_INIT(gd32_uart_list);

static void uart_isr(struct os_serial_device *serial);
static void dma_isr(struct os_serial_device *serial);

#if defined(OS_USING_USART0)
void USART0_IRQHandler(void)
{
    struct gd32_uart *uart;

    os_list_for_each_entry(uart, &gd32_uart_list, struct gd32_uart, list)
    {
        if (uart->usart_info->uart_periph == USART0)
        {
            uart_isr(&uart->serial);
            break;
        }
    }
}

void DMA0_Channel4_IRQHandler(void)
{
    struct gd32_uart *uart;

    os_list_for_each_entry(uart, &gd32_uart_list, struct gd32_uart, list)
    {
        if (uart->usart_info->uart_periph == USART0)
        {
            dma_isr(&uart->serial);
            break;
        }
    }
}

#endif

#if defined(OS_USING_USART1)
void USART1_IRQHandler(void)
{
    struct gd32_uart *uart;

    os_list_for_each_entry(uart, &gd32_uart_list, struct gd32_uart, list)
    {
        if (uart->usart_info->uart_periph == USART1)
        {
            uart_isr(&uart->serial);
            break;
        }
    }
}

void DMA0_Channel5_IRQHandler(void)
{
    struct gd32_uart *uart;

    os_list_for_each_entry(uart, &gd32_uart_list, struct gd32_uart, list)
    {
        if (uart->usart_info->uart_periph == USART1)
        {
            dma_isr(&uart->serial);
            break;
        }
    }
}
#endif

void __gd32_usart_init(struct gd32_uart *uart)
{

    rcu_periph_clock_enable(uart->usart_info->tx_gpio_clk);
    rcu_periph_clock_enable(uart->usart_info->rx_gpio_clk);
    rcu_periph_clock_enable(uart->usart_info->per_clk);

    gpio_init(uart->usart_info->tx_port, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, uart->usart_info->tx_pin);
    gpio_init(uart->usart_info->rx_port, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, uart->usart_info->rx_pin);

    if(uart->usart_info->use_dma)
    {
        rcu_periph_clock_enable(uart->usart_info->dma_clk);
        eclic_irq_enable(uart->usart_info->dma_irqn, 2, 0);
        eclic_irq_enable(uart->usart_info->irqn, 2, 2);
    }
    else
    {
        eclic_irq_enable(uart->usart_info->irqn, 2, 2);
    }

    return;
}


static os_uint32_t gd32_sdma_int_get_index(soft_dma_t *dma)
{
    struct gd32_uart *uart = os_container_of(dma, struct gd32_uart, sdma);

    return uart->rx_index;
}

static os_err_t gd32_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct gd32_uart *uart = os_container_of(dma, struct gd32_uart, sdma);

    uart->rx_buff  = buff;
    uart->rx_index = 0;
    uart->rx_size  = size;

    usart_interrupt_enable(uart->usart_info->uart_periph, USART_INT_RBNE);

    return OS_EOK;
}

static os_uint32_t gd32_sdma_int_stop(soft_dma_t *dma)
{
    struct gd32_uart *uart = os_container_of(dma, struct gd32_uart, sdma);

    usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_RBNE);

    return gd32_sdma_int_get_index(dma);
}


static os_uint32_t gd32_sdma_dma_get_index(soft_dma_t *dma)
{
    os_uint32_t count = 0;
    struct gd32_uart *uart = os_container_of(dma, struct gd32_uart, sdma);

    count = uart->sdma_hard_size - dma_transfer_number_get(uart->usart_info->dma_periph,uart->usart_info->dma_channel);

}

static os_err_t gd32_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t gd32_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{    
    dma_parameter_struct dma_init_struct;
    struct gd32_uart *uart = os_container_of(dma, struct gd32_uart, sdma);

    uart->sdma_hard_size = size;

    dma_deinit(uart->usart_info->dma_periph, uart->usart_info->dma_channel);
    dma_struct_para_init(&dma_init_struct);
    dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY;
    dma_init_struct.memory_addr = (uint32_t)buff;
    dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;
    dma_init_struct.number = size;
    dma_init_struct.periph_addr = (uint32_t)&USART_DATA(uart->usart_info->uart_periph);
    dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_init_struct.memory_width = DMA_PERIPHERAL_WIDTH_8BIT;
    dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
    dma_init(uart->usart_info->dma_periph, uart->usart_info->dma_channel, &dma_init_struct);

    dma_circulation_enable(uart->usart_info->dma_periph, uart->usart_info->dma_channel);
    dma_memory_to_memory_disable(uart->usart_info->dma_periph, uart->usart_info->dma_channel);

    usart_dma_receive_config(uart->usart_info->uart_periph, USART_DENR_ENABLE);
    dma_interrupt_enable(uart->usart_info->dma_periph, uart->usart_info->dma_channel, DMA_INT_FTF);
    dma_channel_enable(uart->usart_info->dma_periph, uart->usart_info->dma_channel);

    return OS_EOK;
}

static os_uint32_t gd32_sdma_dma_stop(soft_dma_t *dma)
{
    struct gd32_uart *uart = os_container_of(dma, struct gd32_uart, sdma);

    usart_dma_receive_config(uart->usart_info->uart_periph, USART_DENR_DISABLE);
    dma_interrupt_disable(uart->usart_info->dma_periph, uart->usart_info->dma_channel, DMA_INT_FTF);
    dma_channel_disable(uart->usart_info->dma_periph, uart->usart_info->dma_channel);

    return gd32_sdma_dma_get_index(dma);
}

static void gd32_usart_sdma_callback(soft_dma_t *dma)
{
    struct gd32_uart *uart = os_container_of(dma, struct gd32_uart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void gd32_usart_sdma_init(struct gd32_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode             = HARD_DMA_MODE_CIRCULAR;
    dma->hard_info.max_size         = 64 * 1024;
    dma->hard_info.data_timeout     = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);

    if (uart->usart_info->use_dma == 0)
    {
        dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;

        dma->ops.get_index          = gd32_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = gd32_sdma_int_start;
        dma->ops.dma_stop           = gd32_sdma_int_stop;
    }
    else
    {
        dma->hard_info.flag         = 0;
        dma->ops.get_index          = gd32_sdma_dma_get_index;
        dma->ops.dma_init           = gd32_sdma_dma_init;
        dma->ops.dma_start          = gd32_sdma_dma_start;
        dma->ops.dma_stop           = gd32_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = gd32_usart_sdma_callback;
    dma->cbs.dma_full_callback      = gd32_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = gd32_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static os_err_t gd32_usart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct gd32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = (struct gd32_uart *)serial->parent.user_data;

    __gd32_usart_init(uart);

    usart_baudrate_set(uart->usart_info->uart_periph, cfg->baud_rate);

    switch (cfg->data_bits)
    {
        case DATA_BITS_9:
            usart_word_length_set(uart->usart_info->uart_periph, USART_WL_9BIT);
            break;

        default:
            usart_word_length_set(uart->usart_info->uart_periph, USART_WL_8BIT);
            break;
    }

    switch (cfg->stop_bits)
    {
        case STOP_BITS_2:
            usart_stop_bit_set(uart->usart_info->uart_periph, USART_STB_2BIT);
            break;
        default:
            usart_stop_bit_set(uart->usart_info->uart_periph, USART_STB_1BIT);
            break;
    }

    switch (cfg->parity)
    {
        case PARITY_ODD:
            usart_parity_config(uart->usart_info->uart_periph, USART_PM_ODD);
            break;
        case PARITY_EVEN:
            usart_parity_config(uart->usart_info->uart_periph, USART_PM_EVEN);
            break;
        default:
            usart_parity_config(uart->usart_info->uart_periph, USART_PM_NONE);
            break;
    }
    usart_hardware_flow_rts_config(uart->usart_info->uart_periph, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(uart->usart_info->uart_periph, USART_CTS_DISABLE);
    usart_receive_config(uart->usart_info->uart_periph, USART_RECEIVE_ENABLE);
    usart_transmit_config(uart->usart_info->uart_periph, USART_TRANSMIT_ENABLE);
    usart_enable(uart->usart_info->uart_periph);


    gd32_usart_sdma_init(uart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t gd32_usart_deinit(struct os_serial_device *serial)
{
    struct gd32_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct gd32_uart, serial);

    if(uart->usart_info->use_dma)
    {

        usart_dma_receive_config(uart->usart_info->uart_periph, USART_DENR_DISABLE);
        dma_interrupt_disable(uart->usart_info->dma_periph, uart->usart_info->dma_channel, DMA_INT_FTF);
        dma_channel_disable(uart->usart_info->dma_periph, uart->usart_info->dma_channel);
        usart_receive_config(uart->usart_info->uart_periph, USART_RECEIVE_DISABLE);
        usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_RBNE);
    }
    else
    {
        usart_receive_config(uart->usart_info->uart_periph, USART_RECEIVE_DISABLE);
        usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_RBNE);
    }

    usart_transmit_config(uart->usart_info->uart_periph, USART_TRANSMIT_DISABLE);
    usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_TBE);

    soft_dma_stop(&uart->sdma);

    return 0;
}

static void dma_isr(struct os_serial_device *serial)
{
    struct gd32_uart *uart;
    uint32_t count = 0;

    OS_ASSERT(serial != OS_NULL);

    uart = (struct gd32_uart *)serial->parent.user_data;

    if(dma_interrupt_flag_get(uart->usart_info->dma_periph, uart->usart_info->dma_channel, DMA_INT_FLAG_FTF)){
        dma_interrupt_flag_clear(uart->usart_info->dma_periph, uart->usart_info->dma_channel, DMA_INT_FLAG_G);
        soft_dma_half_irq(&uart->sdma);
    }

}

/**
 ***********************************************************************************************************************
 * @brief           uart_isr:Uart common interrupt process. This need add to uart ISR.
 *
 * @param[in]       serial          serial device.
 *
 * @return          none
 ***********************************************************************************************************************
 */
static void uart_isr(struct os_serial_device *serial)
{

    struct gd32_uart *uart = (struct gd32_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    if(RESET != usart_interrupt_flag_get(uart->usart_info->uart_periph, USART_INT_FLAG_RBNE) &&
            (usart_flag_get(uart->usart_info->uart_periph, USART_FLAG_RBNE) != RESET))
    {  
        uart->rx_buff[uart->rx_index++] = (unsigned char)usart_data_receive(uart->usart_info->uart_periph);  

        while ((usart_flag_get(uart->usart_info->uart_periph, USART_FLAG_TC) == RESET));

        if (uart->rx_index == (uart->rx_size / 2))
        {
            soft_dma_half_irq(&uart->sdma);
        }

        if (uart->rx_index == uart->rx_size)
        {
            uart->rx_index = 0;
            soft_dma_full_irq(&uart->sdma);
        }

    }

    if(RESET != usart_interrupt_flag_get(uart->usart_info->uart_periph, USART_INT_FLAG_TBE))
    {
        uart->tx_count++;
        if(uart->tx_count == uart->tx_size)
        {            
            uart->tx_size = 0;
            uart->tx_count = 0;

            usart_interrupt_disable(uart->usart_info->uart_periph, USART_INT_TBE);
            os_hw_serial_isr_txdone(serial);
            return;
        }
        usart_data_transmit(uart->usart_info->uart_periph, uart->tx_buff[uart->tx_count]);
    }

}

static int gd32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct gd32_uart *uart = (struct gd32_uart *)serial->parent.user_data;

    uart->tx_size = size;
    uart->tx_count = 0;
    uart->tx_buff = buff;

    usart_interrupt_enable(uart->usart_info->uart_periph, USART_INT_TBE);
    usart_data_transmit(uart->usart_info->uart_periph, uart->tx_buff[0]);

    return size;
}

static int gd32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct gd32_uart *uart;
    os_size_t i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    uart = (struct gd32_uart *)serial->parent.user_data;

    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        usart_data_transmit(uart->usart_info->uart_periph,buff[i]);
        while ((usart_flag_get(uart->usart_info->uart_periph, USART_FLAG_TC) == RESET));
        os_irq_unlock(level);
    }    

    return size;

}


static const struct os_uart_ops gd32_uart_ops =
{
    .init         = gd32_usart_init,
    .deinit       = gd32_usart_deinit,
    .start_send   = gd32_uart_start_send,
    .poll_send    = gd32_uart_poll_send,
};

static int gd32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    os_err_t    result  = 0;
    os_base_t   level;

    struct gd32_uart *uart = os_calloc(1, sizeof(struct gd32_uart));

    OS_ASSERT(uart);

    uart->usart_info = (struct gd_usart_info *)dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &gd32_uart_ops;
    serial->config = config;

    level = os_irq_lock();
    os_list_add_tail(&gd32_uart_list, &uart->list);
    os_irq_unlock(level);

    result = os_hw_serial_register(serial, dev->name, uart);

    OS_ASSERT(result == OS_EOK);

    return result;
}


static void __putchar(const os_uint8_t buff)
{
    usart_data_transmit(USART0,buff);
    while ((usart_flag_get(USART0, USART_FLAG_TC) == RESET));
}

void __os_hw_console_output(char *str)
{
    os_size_t i;
    os_size_t size = strlen(str);

    for (i = 0; i < size; i++)
    {
        __putchar((const os_uint8_t)str[i]);
    }    

    return;
}

OS_DRIVER_INFO gd32_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = gd32_usart_probe,
};

OS_DRIVER_DEFINE(gd32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

