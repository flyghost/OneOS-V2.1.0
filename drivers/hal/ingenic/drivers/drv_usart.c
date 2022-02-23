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
 * @brief       This file implements usart driver for ingenic
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_usart.h>
#include <os_errno.h>
#include <drv_common.h>
#include <ingenic_ost.h>
#include <interrupt.h>
#include <arch_interrupt.h>
#include <os_memory.h>
#include <string.h>

static os_err_t ingenic_configure(struct os_serial_device *serial, struct serial_configure *cfg)
{

    struct ingenic_uart *uart;
    struct uart_hw_config hw_cfg;

    OS_ASSERT(serial != OS_NULL);
    serial->config = *cfg;

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    hw_cfg.baud_rate = cfg->baud_rate;
    hw_cfg.bit_order = cfg->bit_order;
    hw_cfg.data_bits = cfg->data_bits;
    hw_cfg.parity = cfg->parity;
    hw_cfg.stop_bits = cfg->stop_bits;

    return uart_init(uart->usart_info->hw_base,&hw_cfg);

}

static int ingenic_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    os_base_t level;
    os_uint32_t i=0,retry=0;
    struct ingenic_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = serial->parent.user_data;
    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        while(uart_sendstate(uart->usart_info->hw_base))
        {
            retry ++;
            if (retry > 0xfffff)
            {
                retry = 0;
            }
        }
        uart_senddatapoll(uart->usart_info->hw_base,*(os_uint8_t *)(buff+i));
        os_irq_unlock(level);
    }
    return size;
}

static void ingenic_usart_sdma_callback(soft_dma_t *dma)
{
    struct ingenic_uart *uart = os_container_of(dma, struct ingenic_uart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static os_uint32_t ingenic_sdma_int_get_index(soft_dma_t *dma)
{
    struct ingenic_uart *uart = os_container_of(dma, struct ingenic_uart, sdma);

    return uart->rx_index;
}

static os_err_t ingenic_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct ingenic_uart *uart = os_container_of(dma, struct ingenic_uart, sdma);

    uart->rx_buff  = buff;
    uart->rx_index = 0;
    uart->rx_size  = size;

    uart_enableirq(uart->usart_info->irqno,uart->usart_info->hw_base);

    return OS_EOK;
}

static os_uint32_t ingenic_sdma_int_stop(soft_dma_t *dma)
{
    struct ingenic_uart *uart = os_container_of(dma, struct ingenic_uart, sdma);

    uart_disableirq(uart->usart_info->irqno,uart->usart_info->hw_base);

    return ingenic_sdma_int_get_index(dma);
}


static void ingenic_usart_sdma_init(struct ingenic_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial_dev.config.baud_rate);

    if (uart->usart_info->use_dma == 0)
    {
        dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
        dma->ops.get_index          = ingenic_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = ingenic_sdma_int_start;
        dma->ops.dma_stop           = ingenic_sdma_int_stop;
    }
    else
    {
        dma->hard_info.flag         = 0;
        dma->ops.get_index          = OS_NULL;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = OS_NULL;
        dma->ops.dma_stop           = OS_NULL;
    }

    dma->cbs.dma_half_callback      = ingenic_usart_sdma_callback;
    dma->cbs.dma_full_callback      = ingenic_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = ingenic_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static os_err_t ingenic_usart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct ingenic_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = (struct ingenic_uart *)serial->parent.user_data;

    ingenic_configure(serial,cfg);


    ingenic_usart_sdma_init(uart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t ingenic_usart_deinit(struct os_serial_device *serial)
{
    struct ingenic_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct ingenic_uart, serial_dev);

    if(uart->usart_info->use_dma ==0)
    {
        uart_disableirq(uart->usart_info->irqno,uart->usart_info->hw_base);
    }

    soft_dma_stop(&uart->sdma);

    return 0;
}


static const struct os_uart_ops ingenic_uart_ops = {
    .init         = ingenic_usart_init,
    .deinit       = ingenic_usart_deinit,
    .start_send   = OS_NULL,
    .poll_send    = ingenic_uart_poll_send,
};

static void uart_isr(struct os_serial_device *serial)
{

    struct ingenic_uart* uart = serial->parent.user_data;

    while(1){


        if(!uart_receivestate(uart->usart_info->hw_base))
        {
            uart->rx_buff[uart->rx_index++] = uart_receivedata(uart->usart_info->hw_base);

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
        else
            break;
    }


}

static void uart_irq_handler(int irqno, void *param)
{
    os_ubase_t isr;
    struct os_serial_device *serial = (struct os_serial_device*)param;
    struct ingenic_uart *uart = serial->parent.user_data;

    /* read interrupt status and clear it */
    isr = uart_get_interrupt(uart->usart_info->hw_base);
    if (isr & UARTISR_IID_RDI)      /* Receive Data Available */
    {
        uart_isr(serial);
    }

    return;
}

static void __os_hw_usart_init(struct ingenic_uart *uart,const os_device_info_t *dev)
{

    uart_gpio_func_init((uint32_t)uart->usart_info->hw_base);
    clk_enable(clk_get(dev->name));
    os_hw_interrupt_install(uart->usart_info->irqno, uart_irq_handler,&(uart->serial_dev),dev->name);

    return;
}

static int ingenic_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_uint32_t i;
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t result  = 0;

    struct ingenic_uart *uart = os_calloc(1, sizeof(struct ingenic_uart));

    OS_ASSERT(uart);

    uart->usart_info = (struct ingenic_usart_info *)dev->info;

    struct os_serial_device *serial = &(uart->serial_dev);

    __os_hw_usart_init(uart,dev);

    serial->ops = &ingenic_uart_ops;
    serial->config = config;

    result = os_hw_serial_register(serial, dev->name, uart);

    OS_ASSERT(result == OS_EOK);

    return result;
}

void uart_send(const os_uint8_t *buff, os_size_t size)
{
    os_base_t level;
    os_uint32_t i=0,retry=0;

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        while(uart_sendstate(UART2_BASE))
        {
            retry ++;
            if (retry > 0xfffff)
            {
                retry = 0;
            }
        }
        uart_senddatapoll(UART2_BASE,*(os_uint8_t *)(buff+i));
        os_irq_unlock(level);
    }
}

void __os_hw_console_output(char *str)
{
    uart_send(str,strlen(str));
}

OS_DRIVER_INFO ingenic_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = ingenic_usart_probe,
};

OS_DRIVER_DEFINE(ingenic_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);



