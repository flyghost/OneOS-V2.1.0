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
 * @file        rtt.c
 *
 * @brief       rtt
 *
 * @details     rtt
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <arch_interrupt.h>
#include <os_assert.h>

#include <device.h>
#include <drv_cfg.h>

#include <stdio.h>
#include <string.h>
#include "SEGGER_RTT.h"

#define RTT_BUFF_INDEX_FOR_SERIAL   (0)

struct rtt_uart
{
    struct os_serial_device serial;

    soft_dma_t  sdma;
    
    os_bool_t   rx_isr_enabled;

    os_uint8_t *buff;
    os_size_t   size;
    os_size_t   count;
};

static os_uint32_t rtt_uart_sdma_get_index(soft_dma_t *dma)
{
    struct rtt_uart *uart = os_container_of(dma, struct rtt_uart, sdma);

    if (uart->rx_isr_enabled == OS_FALSE)
        return uart->count;
    
    if (SEGGER_RTT_HasData(RTT_BUFF_INDEX_FOR_SERIAL))
    {
        uart->count += SEGGER_RTT_Read(RTT_BUFF_INDEX_FOR_SERIAL, uart->buff + uart->count, uart->size - uart->count);

        OS_ASSERT(uart->count <= uart->size);

        if (uart->count == uart->size)
            uart->count = 0;
    }

    return uart->count;
}

static os_err_t rtt_uart_sdma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct rtt_uart *uart = os_container_of(dma, struct rtt_uart, sdma);

    os_base_t level;

    level = os_irq_lock();

    uart->buff  = buff;
    uart->size  = size;
    uart->count = 0;

    uart->rx_isr_enabled = OS_TRUE;

    os_irq_unlock(level);
    
    return OS_EOK;
}

static os_uint32_t rtt_uart_sdma_stop(soft_dma_t *dma)
{
    struct rtt_uart  *uart  = os_container_of(dma, struct rtt_uart, sdma);

    uart->rx_isr_enabled = OS_FALSE;
    
    return uart->count;
}

/* sdma callback */
static void rtt_uart_sdma_callback(soft_dma_t *dma)
{
    struct rtt_uart *uart = os_container_of(dma, struct rtt_uart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void rtt_uart_sdma_init(struct rtt_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = 0;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);
    
    dma->ops.get_index          = rtt_uart_sdma_get_index;
    dma->ops.dma_init           = OS_NULL;
    dma->ops.dma_start          = rtt_uart_sdma_start;
    dma->ops.dma_stop           = rtt_uart_sdma_stop;

    dma->cbs.dma_half_callback      = rtt_uart_sdma_callback;
    dma->cbs.dma_full_callback      = rtt_uart_sdma_callback;
    dma->cbs.dma_timeout_callback   = rtt_uart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

int rtt_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct rtt_uart *uart = os_container_of(serial, struct rtt_uart, serial);

    uart->rx_isr_enabled = OS_FALSE;

    rtt_uart_sdma_init(uart, &serial->rx_fifo->ring);

    return 0;
}

int rtt_uart_deinit(struct os_serial_device *serial)
{
    struct rtt_uart *uart = os_container_of(serial, struct rtt_uart, serial);

    os_base_t level;

    level = os_irq_lock();
    
    uart->rx_isr_enabled = OS_FALSE;
    uart->buff  = OS_NULL;
    uart->size  = 0;
    uart->count = 0;

    soft_dma_stop(&uart->sdma);

    os_irq_unlock(level);

    return 0;
}

static int rtt_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    SEGGER_RTT_Write(RTT_BUFF_INDEX_FOR_SERIAL, buff, size);

    return size;
}

static const struct os_uart_ops rtt_uart_ops = {
    .init       = rtt_uart_init,
    .deinit     = rtt_uart_deinit,
    .poll_send  = rtt_uart_poll_send,
};

static struct rtt_uart uart;

static int os_hw_rtt_init(void)
{
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    memset(&uart, 0, sizeof(uart));

    uart.serial.ops    = &rtt_uart_ops;
    uart.serial.config = config;

    os_hw_serial_register(&uart.serial, OS_RTT_DEVICE_NAME, OS_NULL);

    return 0;
}

OS_PREV_INIT(os_hw_rtt_init, OS_INIT_SUBLEVEL_HIGH);

