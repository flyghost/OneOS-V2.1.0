/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the \"License\ you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * \@file        drv_uart.c
 *
 * \@brief       This file implements uart driver for FM33A0xx.
 *
 * \@revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include <drv_cfg.h>
#include <device.h>

#include <string.h>

#include "drv_common.h"
#include "drv_uart.h"
#include <drv_log.h>

#include "soft_dma.h"

static os_list_node_t fm33_uart_list = OS_LIST_INIT(fm33_uart_list);

struct fm33_uart
{
    struct os_serial_device serial;
    struct fm33_usart_info *info;

    soft_dma_t  sdma;

    os_uint8_t *rx_buff;
    os_uint32_t rx_size;
    os_uint32_t rx_index;

    const os_uint8_t *tx_buff;
    os_uint32_t tx_size;
    os_uint32_t tx_index;

    os_list_node_t list;
};

static void uart_isr(struct os_serial_device *serial)
{
    struct fm33_uart *uart = os_container_of(serial, struct fm33_uart, serial);

    if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(uart->info->husart))
            && (FL_SET == FL_UART_IsActiveFlag_RXBuffFull(uart->info->husart)))
    {
        uart->rx_buff[uart->rx_index++] = FL_UART_ReadRXBuff(uart->info->husart);

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

    if((FL_ENABLE == FL_UART_IsEnabledIT_TXShiftBuffEmpty(uart->info->husart))
            && (FL_SET == FL_UART_IsActiveFlag_TXShiftBuffEmpty(uart->info->husart)))
    {
        FL_UART_ClearFlag_TXShiftBuffEmpty(uart->info->husart);
 
        if (uart->tx_size)
        {
            if(uart->tx_index < uart->tx_size)
            {
                FL_UART_WriteTXBuff(uart->info->husart, uart->tx_buff[uart->tx_index++]);
            }
            else
            {
                FL_UART_DisableIT_TXBuffEmpty(uart->info->husart);

                os_hw_serial_isr_txdone(serial);
            }
        }
    }
}

static void usart_rx_irq_callback(os_uint32_t  usart_index)
{
    struct fm33_uart *uart;

    os_list_for_each_entry(uart, &fm33_uart_list, struct fm33_uart, list)
    {
        if (uart->info->index == usart_index)
        {
            uart_isr(&uart->serial);
            break;
        }
    }
}

void UART_DMA_IRQHandler_Callback(void)
{
    struct fm33_uart *uart = OS_NULL;

    os_list_for_each_entry(uart, &fm33_uart_list, struct fm33_uart, list)
    {
        if (uart->info->hdma == OS_NULL)
        {
            continue;
        }

        if (FL_DMA_IsActiveFlag_TransferHalfComplete(uart->info->hdma, uart->info->hdma_rx_channel) == FL_SET)
        {
            soft_dma_half_irq(&uart->sdma);
            FL_DMA_ClearFlag_TransferHalfComplete(uart->info->hdma, uart->info->hdma_rx_channel);
        }
        else if (FL_DMA_IsActiveFlag_TransferComplete(uart->info->hdma, uart->info->hdma_rx_channel) == FL_SET)
        {
            soft_dma_half_irq(&uart->sdma);
            FL_DMA_ClearFlag_TransferComplete(uart->info->hdma, uart->info->hdma_rx_channel);
        }
    }
}

#if defined(BSP_USING_UART0)
void UART0_IRQHandler(void)
{
    usart_rx_irq_callback(0);
}
#endif /* BSP_USING_UART0 */

#if defined(BSP_USING_UART1)
void UART1_IRQHandler(void)
{
    usart_rx_irq_callback(1);
}
#endif /* BSP_USING_UART1 */

#if defined(BSP_USING_UART4)
void UART4_IRQHandler(void)
{
    usart_rx_irq_callback(4);
}
#endif /* BSP_USING_UART4 */

#if defined(BSP_USING_UART5)
void UART5_IRQHandler(void)
{
    usart_rx_irq_callback(5);
}
#endif /* BSP_USING_UART5 */


static os_uint32_t fm33_sdma_int_get_index(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    return uart->rx_index;
}

static os_err_t fm33_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_uart *uart = os_container_of(dma, struct fm33_uart, sdma);

    uart->rx_buff  = buff;
    uart->rx_size  = size;
    uart->rx_index = 0;

    FL_UART_EnableIT_RXBuffFull(uart->info->husart);

    FL_UART_EnableRX(uart->info->husart);

    return OS_EOK;
}

static os_uint32_t fm33_sdma_int_stop(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    FL_UART_DisableIT_RXBuffFull(uart->info->husart);

    FL_UART_DisableRX(uart->info->husart);

    return uart->rx_index;
}

static os_uint32_t fm33_sdma_dma_get_index(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    return (os_uint32_t)FL_DMA_ReadMemoryAddress(uart->info->hdma, uart->info->hdma_rx_channel) - (os_uint32_t)uart->rx_buff;
}

static os_err_t fm33_sdma_dma_init(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    FL_DMA_Init(uart->info->hdma, &uart->info->dmaInitStruct_rx, uart->info->hdma_rx_channel);

    return OS_EOK;
}

static os_err_t fm33_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_uart *uart = os_container_of(dma, struct fm33_uart, sdma);
    FL_DMA_ConfigTypeDef DMA_ConfigStruct = {0};

    uart->rx_buff  = buff;
    uart->rx_size  = size;

    FL_DMA_EnableIT_TransferHalfComplete(uart->info->hdma, uart->info->hdma_rx_channel);
    FL_DMA_EnableIT_TransferComplete(uart->info->hdma, uart->info->hdma_rx_channel);

    DMA_ConfigStruct.memoryAddress = (os_uint32_t)buff;
    DMA_ConfigStruct.transmissionCount = size - 1; 
    FL_DMA_StartTransmission(uart->info->hdma, &DMA_ConfigStruct, uart->info->hdma_rx_channel);

    FL_UART_EnableRX(uart->info->husart);

    return OS_EOK;
}
static os_uint32_t fm33_sdma_dma_stop(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    OS_ASSERT(uart->info->hdma != OS_NULL);

    FL_DMA_DisableIT_TransferHalfComplete(uart->info->hdma, uart->info->hdma_rx_channel);
    FL_DMA_DisableIT_TransferComplete(uart->info->hdma, uart->info->hdma_rx_channel);

    FL_DMA_DisableChannel(uart->info->hdma, uart->info->hdma_rx_channel);

    return (os_uint8_t *)FL_DMA_ReadMemoryAddress(uart->info->hdma, uart->info->hdma_rx_channel) - uart->rx_buff;
}

static void fm33_usart_sdma_callback(soft_dma_t *dma)
{
    struct fm33_uart  *uart  = os_container_of(dma, struct fm33_uart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void fm33_usart_sdma_init(struct fm33_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);

    if (uart->info->hdma == OS_NULL)
    {
        dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;
        
        dma->ops.get_index          = fm33_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = fm33_sdma_int_start;
        dma->ops.dma_stop           = fm33_sdma_int_stop;
    }
    else
    {
        if (uart->info->dmaInitStruct_rx.circMode == FL_ENABLE)
        {
            dma->hard_info.mode     = HARD_DMA_MODE_CIRCULAR;
        }
        else
        {
            dma->hard_info.mode     = HARD_DMA_MODE_NORMAL;
        }

        dma->ops.get_index          = fm33_sdma_dma_get_index;
        dma->ops.dma_init           = fm33_sdma_dma_init;
        dma->ops.dma_start          = fm33_sdma_dma_start;
        dma->ops.dma_stop           = fm33_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = fm33_usart_sdma_callback;
    dma->cbs.dma_full_callback      = fm33_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = fm33_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static os_err_t fm33_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct fm33_uart *uart = OS_NULL;
    FL_UART_InitTypeDef UART_InitStruct = {0};

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct fm33_uart, serial);
    OS_ASSERT(uart != OS_NULL);

    FL_GPIO_Init(uart->info->port, &uart->info->GPIO_InitStruct);

    UART_InitStruct.clockSrc = uart->info->clock_src;
    UART_InitStruct.baudRate            = cfg->baud_rate;

    switch (cfg->data_bits)
    {
        case DATA_BITS_7:
            UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_7B;
            break;
        case DATA_BITS_8:
            UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
            break;
        case DATA_BITS_9:
            UART_InitStruct.dataWidth = FL_UART_DATA_WIDTH_9B;
            break;
        default:
            return OS_EINVAL;
    }

    switch (cfg->parity)
    {
        case PARITY_NONE:
            UART_InitStruct.parity = FL_UART_PARITY_NONE;
            break;
        case PARITY_ODD:
            UART_InitStruct.parity = FL_UART_PARITY_ODD;
            break;
        case PARITY_EVEN:
            UART_InitStruct.parity = FL_UART_PARITY_EVEN;
            break;
        default:
            return OS_EINVAL;
    }

    switch (cfg->stop_bits)
    {
        case STOP_BITS_1:
            UART_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
            break;
        case STOP_BITS_2:
            UART_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_2B;
            break;
        default:
            return OS_EINVAL;
    }

    UART_InitStruct.transferDirection   = FL_UART_DIRECTION_TX;
    FL_UART_Init(uart->info->husart, &UART_InitStruct);

    NVIC_DisableIRQ(uart->info->irqn);
    NVIC_SetPriority(uart->info->irqn, uart->info->irq_priority);
    NVIC_EnableIRQ(uart->info->irqn);

    fm33_usart_sdma_init(uart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t fm33_uart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    struct fm33_uart *uart = os_container_of(serial, struct fm33_uart, serial);

    if (uart->info->hdma != OS_NULL)
    {
        soft_dma_stop(&uart->sdma);
    }

    FL_UART_DeInit(uart->info->husart);

    return OS_EOK;
}

static int fm33_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct fm33_uart *uart = os_container_of(serial, struct fm33_uart, serial);

    OS_ASSERT(serial != OS_NULL);

    uart->tx_buff  = buff;
    uart->tx_size  = size;
    uart->tx_index = 0;

    FL_UART_ClearFlag_TXShiftBuffEmpty(uart->info->husart);
    FL_UART_EnableIT_TXShiftBuffEmpty(uart->info->husart);

    FL_UART_WriteTXBuff(uart->info->husart, uart->tx_buff[uart->tx_index++]);

    return size;
}

static int fm33_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct fm33_uart *uart = os_container_of(serial, struct fm33_uart, serial);

    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        FL_UART_WriteTXBuff(uart->info->husart, buff[i]);
        while(FL_SET != FL_UART_IsActiveFlag_TXBuffEmpty(uart->info->husart));
        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops fm33_uart_ops =
{
    .init       = fm33_uart_init,
    .deinit     = fm33_uart_deinit,

    .start_send = fm33_uart_start_send,
    .poll_send  = fm33_uart_poll_send,
};

static int fm33_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;

    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct fm33_uart *usart;

    usart = os_calloc(1, sizeof(struct fm33_uart));
    OS_ASSERT(usart);

    usart->info = (struct fm33_usart_info *)dev->info;
    struct os_serial_device *serial = &usart->serial;

    level = os_irq_lock();
    os_list_add_tail(&fm33_uart_list, &usart->list);
    os_irq_unlock(level);

    serial->ops    = &fm33_uart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, usart);

    return 0;
}

OS_DRIVER_INFO fm33_usart_driver = {
    .name   = "Usart_Type",
    .probe  = fm33_usart_probe,
};

OS_DRIVER_DEFINE(fm33_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);
