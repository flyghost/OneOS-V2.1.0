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
 * @brief       This file implements usart driver for hc32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <string.h>
#include <os_memory.h>
#include "hc32f46x_gpio.h"
#include "hc32f46x_usart.h"
#include "hc32f46x_pwc.h"
#include "hc32f46x_interrupts.h"
#include "drv_usart.h"
#include "board.h"

#ifdef BSP_USING_UART

static os_list_node_t hc32_uart_list = OS_LIST_INIT(hc32_uart_list);

/* uart driver */
typedef struct hc_uart
{
    struct os_serial_device serial_dev;
    struct hc32_usart_info *info;
    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;
    uint8_t  *rx_buff;
    uint32_t  rx_index;
    uint32_t  rx_size;
    os_list_node_t list;
} hc_usart_t;

static void uart_isr(struct os_serial_device *serial)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct hc_uart, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    if(Set == USART_GetStatus(uart->info->idx, UsartRxNoEmpty))
    {
        uart->rx_buff[uart->rx_index++] = USART_RecData(uart->info->idx);

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
}

static void UsartRxIrqCallback(void)
{
    struct hc_uart *uart;

    os_list_for_each_entry(uart, &hc32_uart_list, struct hc_uart, list)
    {
        if (uart->info->uart_device == 1)
        {
            uart_isr(&uart->serial_dev);
            break;
        }
    }
}

/* interrupt rx mode */
static os_uint32_t hc_sdma_int_get_index(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    return uart->rx_index;
}

static os_err_t hc_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    uart->rx_buff  = buff;
    uart->rx_index = 0;
    uart->rx_size  = size;

    USART_FuncCmd(uart->info->idx, UsartRx, Enable);
    USART_FuncCmd(uart->info->idx, UsartRxInt, Enable);
    USART_FuncCmd(uart->info->idx, UsartTx, Enable);

    return OS_EOK;
}

static os_uint32_t hc_sdma_int_stop(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    USART_FuncCmd(uart->info->idx, UsartRx, Disable);
    USART_FuncCmd(uart->info->idx, UsartRxInt, Disable);
    USART_FuncCmd(uart->info->idx, UsartTx, Disable);

    return uart->rx_index;
}

/* dma rx mode */
static os_uint32_t hc_sdma_dma_get_index(soft_dma_t *dma)
{
    return 0;
}

static os_err_t hc_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t hc_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    return OS_EOK;
}

static os_uint32_t hc_sdma_dma_stop(soft_dma_t *dma)
{
    return hc_sdma_dma_get_index(dma);
}

/* sdma callback */
static void hc_usart_sdma_callback(soft_dma_t *dma)
{
    hc_usart_t *uart = os_container_of(dma, hc_usart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void hc_usart_sdma_init(struct hc_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial_dev.config.baud_rate);

    if (1)
    {
        dma->ops.get_index          = hc_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = hc_sdma_int_start;
        dma->ops.dma_stop           = hc_sdma_int_stop;
    }
    else
    {
        dma->hard_info.flag        |= HARD_DMA_FLAG_FULL_IRQ;
        dma->ops.get_index          = hc_sdma_dma_get_index;
        dma->ops.dma_init           = hc_sdma_dma_init;
        dma->ops.dma_start          = hc_sdma_dma_start;
        dma->ops.dma_stop           = hc_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = hc_usart_sdma_callback;
    dma->cbs.dma_full_callback      = hc_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = hc_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static os_err_t hc_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    uart = os_container_of(serial, struct hc_uart, serial_dev);

    stc_usart_uart_init_t stcInitCfg;

    MEM_ZERO_STRUCT(stcInitCfg);

    PWC_Fcg1PeriphClockCmd(uart->info->periph, Enable);

    /* Initialize USART IO */
    PORT_SetFunc(uart->info->tx_port, uart->info->tx_pin, uart->info->tx_func, Disable);
    PORT_SetFunc(uart->info->rx_port, uart->info->rx_pin, uart->info->rx_func, Disable);

    stcInitCfg.enClkDiv = UsartClkDiv_1;
    stcInitCfg.enClkMode = UsartIntClkCkNoOutput;
    stcInitCfg.enDataLength = UsartDataBits8;
    stcInitCfg.enDetectMode = UsartStartBitFallEdge;
    stcInitCfg.enDirection = UsartDataLsbFirst;
    stcInitCfg.enHwFlow = UsartRtsEnable;
    stcInitCfg.enParity = UsartParityNone;
    stcInitCfg.enSampleMode = UsartSamleBit8;
    stcInitCfg.enStopBit = UsartOneStopBit;

    /* Initialize UART */
    if (USART_UART_Init(uart->info->idx, &stcInitCfg) != Ok)
    {
        while (1)
        {
        }
    }

    /* Set baudrate */
    if (USART_SetBaudrate(uart->info->idx, 115200) != Ok)
    {
        while (1)
        {
        }
    }

    stc_irq_regi_conf_t stcIrqRegiCfg;

    stcIrqRegiCfg.enIRQn = uart->info->irqn;
    stcIrqRegiCfg.pfnCallback = &UsartRxIrqCallback;
    stcIrqRegiCfg.enIntSrc = uart->info->int_Src;
    enIrqRegistration(&stcIrqRegiCfg);
    NVIC_SetPriority(stcIrqRegiCfg.enIRQn, DDL_IRQ_PRIORITY_DEFAULT);
    NVIC_ClearPendingIRQ(stcIrqRegiCfg.enIRQn);
    NVIC_EnableIRQ(stcIrqRegiCfg.enIRQn);

    hc_usart_sdma_init(uart, &uart->serial_dev.rx_fifo->ring);

    return OS_EOK;
}

static os_err_t hc_uart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    hc_usart_t *uart = os_container_of(serial, hc_usart_t, serial_dev);

    if (uart->info->dma_support == 0)
    {
        USART_FuncCmd(uart->info->idx, UsartRx, Disable);
        USART_FuncCmd(uart->info->idx, UsartRxInt, Disable);
        USART_FuncCmd(uart->info->idx, UsartTx, Disable);
    }
    else
    {
        soft_dma_stop(&uart->sdma);
    }

    return OS_EOK;
}

static int hc_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;
    struct hc_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct hc_uart, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();

        while (Reset == USART_GetStatus(uart->info->idx, UsartTxEmpty))
        {
        }
        USART_SendData(uart->info->idx, buff[i]);

        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops hc_uart_ops =
{
    .init       = hc_uart_init,
    .deinit     = hc_uart_deinit,

    .poll_send  = hc_uart_poll_send,
};

void __os_hw_console_output(char *str)
{
    /*
    if (console_uart == OS_NULL)
        return;
    */

    while (*str)
    {
        str++;
    }
}

static int hc32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct hc_uart *usart;

    usart = os_calloc(1, sizeof(struct hc_uart));
    OS_ASSERT(usart);

    usart->info = (struct hc32_usart_info *)dev->info;
    struct os_serial_device *serial = &usart->serial_dev;

    level = os_irq_lock();
    os_list_add_tail(&hc32_uart_list, &usart->list);
    os_irq_unlock(level);

    serial->ops    = &hc_uart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, usart);

    return 0;
}

OS_DRIVER_INFO hc32_usart_driver = {
    .name   = "Usart_Type",
    .probe  = hc32_usart_probe,
};

OS_DRIVER_DEFINE(hc32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

#endif
