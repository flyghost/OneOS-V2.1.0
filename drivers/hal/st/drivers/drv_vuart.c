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
 * @file        virt.c
 *
 * @brief       virt
 *
 * @details     virt
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

#include "virt_uart.h"

struct virt_uart
{
    struct os_serial_device serial;

    VIRT_UART_HandleTypeDef virtUART;

    soft_dma_t  sdma;
    dma_ring_t  ring;
    os_bool_t   rx_isr_enabled;
};

static void virt_uart_rx_irq(VIRT_UART_HandleTypeDef *huart)
{
    struct virt_uart *uart = os_container_of(huart, struct virt_uart, virtUART);

    if (uart->rx_isr_enabled == OS_FALSE)
        return;
    
    copy_line_to_ring(&uart->ring, huart->pRxBuffPtr, min(huart->RxXferSize, ring_space(&uart->ring)));

    uart->ring.tail = uart->ring.head;
}

static os_uint32_t virt_uart_sdma_get_index(soft_dma_t *dma)
{
    struct virt_uart *uart = os_container_of(dma, struct virt_uart, sdma);
    
    return uart->ring.head;
}

static os_err_t virt_uart_sdma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct virt_uart *uart = os_container_of(dma, struct virt_uart, sdma);

    os_base_t level;

    level = os_irq_lock();

    uart->ring.buff = buff;
    uart->ring.head = 0;
    uart->ring.tail = 0;
    uart->ring.size = size;

    uart->rx_isr_enabled = OS_TRUE;

    os_irq_unlock(level);
    
    return OS_EOK;
}

static os_uint32_t virt_uart_sdma_stop(soft_dma_t *dma)
{
    struct virt_uart *uart = os_container_of(dma, struct virt_uart, sdma);

    uart->rx_isr_enabled = OS_FALSE;
    
    return ring_count(&uart->ring);
}

/* sdma callback */
static void virt_uart_sdma_callback(soft_dma_t *dma)
{
    struct virt_uart *uart = os_container_of(dma, struct virt_uart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void virt_uart_sdma_init(struct virt_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_CIRCULAR;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = 0;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);
    
    dma->ops.get_index          = virt_uart_sdma_get_index;
    dma->ops.dma_init           = OS_NULL;
    dma->ops.dma_start          = virt_uart_sdma_start;
    dma->ops.dma_stop           = virt_uart_sdma_stop;

    dma->cbs.dma_half_callback      = virt_uart_sdma_callback;
    dma->cbs.dma_full_callback      = virt_uart_sdma_callback;
    dma->cbs.dma_timeout_callback   = virt_uart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

int virt_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct virt_uart *uart = os_container_of(serial, struct virt_uart, serial);

    uart->rx_isr_enabled = OS_FALSE;

    virt_uart_sdma_init(uart, &serial->rx_fifo->ring);

    return 0;
}

int virt_uart_deinit(struct os_serial_device *serial)
{
    struct virt_uart *uart = os_container_of(serial, struct virt_uart, serial);

    os_base_t level;

    level = os_irq_lock();
    
    uart->rx_isr_enabled = OS_FALSE;

    soft_dma_stop(&uart->sdma);

    os_irq_unlock(level);

    return 0;
}

static int virt_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct virt_uart *uart = os_container_of(serial, struct virt_uart, serial);

    int i;

    for (i = 0; i < size; i++)
    {
        VIRT_UART_Transmit(&uart->virtUART, (uint8_t *)buff + i, 1);
    }

    return size;
}

static const struct os_uart_ops virt_uart_ops = {
    .init       = virt_uart_init,
    .deinit     = virt_uart_deinit,
    .poll_send  = virt_uart_poll_send,
};

#define VIRT_UART_NR 2

static int os_hw_virt_init(void)
{
    int i;
    char dev_name[16] = "uartRPMSG0";
    
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct virt_uart *uart = os_calloc(VIRT_UART_NR, sizeof(struct virt_uart));

    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < VIRT_UART_NR; i++, uart++)
    {
        if (VIRT_UART_Init(&uart->virtUART) != VIRT_UART_OK)
        {
            os_kprintf("VIRT_UART_Init virtUART failed.\r\n");
            return 0;
        }
        
        VIRT_UART_RegisterCallback(&uart->virtUART, VIRT_UART_RXCPLT_CB_ID, virt_uart_rx_irq);

        uart->serial.ops    = &virt_uart_ops;
        uart->serial.config = config;

        dev_name[sizeof("uartRPMSG0") - 2] = '0' + i;
        os_hw_serial_register(&uart->serial, dev_name, OS_NULL);
    }

    return 0;
}

OS_PREV_INIT(os_hw_virt_init, OS_INIT_SUBLEVEL_HIGH);

static void openamp_timer_callback(void *parameter)
{
    OPENAMP_check_for_message();
}

static os_timer_t openamp_timer;

static int os_hw_openamp_init(void)
{
    MX_OPENAMP_Init(RPMSG_REMOTE, NULL);

    os_uint64_t period_us = 10000;
    int tick = period_us * OS_TICK_PER_SECOND / 1000000;

    os_timer_init(&openamp_timer, 
                  OS_NULL,
                  openamp_timer_callback, 
                  OS_NULL,
                  tick > 0 ? tick : 1,
                  OS_TIMER_FLAG_PERIODIC);
    
    os_timer_start(&openamp_timer);

    return 0;
}

OS_POSTCORE_INIT(os_hw_openamp_init, OS_INIT_SUBLEVEL_HIGH);
