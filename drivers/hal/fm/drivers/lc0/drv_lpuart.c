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
 * @brief       This file implements usart driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_lpuart.h>

static os_list_node_t fm33_lpuart_list = OS_LIST_INIT(fm33_lpuart_list);

static void lpuart_isr(struct os_serial_device *serial)
{
    struct fm33_lpuart *lpuart = os_container_of(serial, struct fm33_lpuart, serial);

    if((FL_ENABLE == FL_LPUART_IsEnabledIT_RXBuffFull(lpuart->info->hlpuart))
            && (FL_SET == FL_LPUART_IsActiveFlag_RXBuffFull(lpuart->info->hlpuart)))
    {
        lpuart->rx_buff[lpuart->rx_index++] = FL_LPUART_ReadRXBuff(lpuart->info->hlpuart);

        if (lpuart->rx_index == (lpuart->rx_size / 2))
        {
            soft_dma_half_irq(&lpuart->sdma);
        }

        if (lpuart->rx_index == lpuart->rx_size)
        {
            lpuart->rx_index = 0;
            soft_dma_full_irq(&lpuart->sdma);
        }
    }

    if((FL_ENABLE == FL_LPUART_IsEnabledIT_TXShiftBuffEmpty(lpuart->info->hlpuart))
            && (FL_SET == FL_LPUART_IsActiveFlag_TXShiftBuffEmpty(lpuart->info->hlpuart)))
    {
        FL_LPUART_ClearFlag_TXShiftBuffEmpty(lpuart->info->hlpuart);
 
        if (lpuart->tx_size)
        {
            if(lpuart->tx_index < lpuart->tx_size)
            {
                FL_LPUART_WriteTXBuff(lpuart->info->hlpuart, lpuart->tx_buff[lpuart->tx_index++]);
            }
            else
            {
                FL_LPUART_DisableIT_TXBuffEmpty(lpuart->info->hlpuart);

                os_hw_serial_isr_txdone(serial);
            }
        }
    }

}

static void lpuart_rx_irq_callback(os_uint32_t index)
{
    struct fm33_lpuart *lpuart = OS_NULL;

    os_list_for_each_entry(lpuart, &fm33_lpuart_list, struct fm33_lpuart, list)
    {
        if (lpuart->info->index == index)
            lpuart_isr(&lpuart->serial);
    }

}

void LPUART_DMA_IRQHandler_Callback(void)
{
    struct fm33_lpuart *lpuart = OS_NULL;

    os_list_for_each_entry(lpuart, &fm33_lpuart_list, struct fm33_lpuart, list)
    {
        if (lpuart->info->hdma == OS_NULL)
        {
            continue;
        }

        if (FL_DMA_IsActiveFlag_TransferHalfComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel) == FL_SET)
        {
            soft_dma_half_irq(&lpuart->sdma);
            FL_DMA_ClearFlag_TransferHalfComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel);
        }
        else if (FL_DMA_IsActiveFlag_TransferComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel) == FL_SET)
        {
            soft_dma_half_irq(&lpuart->sdma);
            FL_DMA_ClearFlag_TransferComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel);
        }
    }
}

void LPUART0_IRQHandler(void)
{
    lpuart_rx_irq_callback(0);
}

void LPUART1_IRQHandler(void)
{
    lpuart_rx_irq_callback(1);
}

static os_uint32_t fm33_sdma_int_get_index(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    return lpuart->rx_index;
}

static os_err_t fm33_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_lpuart *lpuart = os_container_of(dma, struct fm33_lpuart, sdma);

    lpuart->rx_buff  = buff;
    lpuart->rx_size  = size;
    lpuart->rx_index = 0;

    FL_LPUART_EnableIT_RXBuffFull(lpuart->info->hlpuart);

    FL_LPUART_EnableRX(lpuart->info->hlpuart);

    return OS_EOK;
}

static os_uint32_t fm33_sdma_int_stop(soft_dma_t *dma)
{
    struct fm33_lpuart  *uart  = os_container_of(dma, struct fm33_lpuart, sdma);

    FL_LPUART_DisableIT_RXBufFull(uart->info->hlpuart);

    FL_LPUART_DisableRX(uart->info->hlpuart);

    return uart->rx_index;
}

static os_uint32_t fm33_sdma_dma_get_index(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    return (os_uint32_t)FL_DMA_ReadMemoryAddress(lpuart->info->hdma, lpuart->info->hdma_rx_channel) - (os_uint32_t)lpuart->rx_buff;
}

static os_err_t fm33_sdma_dma_init(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    FL_DMA_Init(lpuart->info->hdma, &lpuart->info->dmaInitStruct_rx, lpuart->info->hdma_rx_channel);

    return OS_EOK;
}

static os_err_t fm33_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);
    FL_DMA_ConfigTypeDef DMA_ConfigStruct = {0};

    lpuart->rx_buff  = buff;
    lpuart->rx_size  = size;

    FL_DMA_EnableIT_TransferHalfComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel);
    FL_DMA_EnableIT_TransferComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel);

    DMA_ConfigStruct.memoryAddress = (os_uint32_t)buff;
    DMA_ConfigStruct.transmissionCount = size - 1; 
    FL_DMA_StartTransmission(lpuart->info->hdma, &DMA_ConfigStruct, lpuart->info->hdma_rx_channel);

    FL_LPUART_EnableRX(lpuart->info->hlpuart);

    return OS_EOK;
}
static os_uint32_t fm33_sdma_dma_stop(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    OS_ASSERT(lpuart->info->hdma != OS_NULL);

    FL_DMA_DisableIT_TransferHalfComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel);
    FL_DMA_DisableIT_TransferComplete(lpuart->info->hdma, lpuart->info->hdma_rx_channel);

    FL_DMA_DisableChannel(lpuart->info->hdma, lpuart->info->hdma_rx_channel);

    return (os_uint8_t *)FL_DMA_ReadMemoryAddress(lpuart->info->hdma, lpuart->info->hdma_rx_channel) - lpuart->rx_buff;
}

static void fm33_lpuart_sdma_callback(soft_dma_t *dma)
{
    struct fm33_lpuart  *lpuart  = os_container_of(dma, struct fm33_lpuart, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)lpuart);
}

static void fm33_lpuart_sdma_init(struct fm33_lpuart *uart, dma_ring_t *ring)
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

    dma->cbs.dma_half_callback      = fm33_lpuart_sdma_callback;
    dma->cbs.dma_full_callback      = fm33_lpuart_sdma_callback;
    dma->cbs.dma_timeout_callback   = fm33_lpuart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static os_err_t fm33_lpuart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct fm33_lpuart *lpuart          = OS_NULL;
    FL_LPUART_InitTypeDef  LPUART_InitStructer = {0};

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    lpuart = os_container_of(serial, struct fm33_lpuart, serial);
    OS_ASSERT(lpuart != OS_NULL);

    FL_GPIO_Init(lpuart->info->port, &lpuart->info->GPIO_InitStruct_tx_pin);
    FL_GPIO_Init(lpuart->info->port, &lpuart->info->GPIO_InitStruct_rx_pin);

    LPUART_InitStructer.clockSrc = lpuart->info->clock_src;
    LPUART_InitStructer.baudRate = lpuart->info->baud_rate;

    switch (cfg->data_bits)
    {
        case DATA_BITS_7:
            LPUART_InitStructer.dataWidth = FL_LPUART_DATA_WIDTH_7B;
            break;
        case DATA_BITS_8:
            LPUART_InitStructer.dataWidth = FL_LPUART_DATA_WIDTH_8B;
            break;
        case DATA_BITS_9:
            LPUART_InitStructer.dataWidth = FL_LPUART_DATA_WIDTH_9B;
            break;
        default:
            return OS_EINVAL;
    }

    switch (cfg->parity)
    {
        case PARITY_NONE:
            LPUART_InitStructer.parity = FL_LPUART_PARITY_NONE;
            break;
        case PARITY_ODD:
            LPUART_InitStructer.parity = FL_LPUART_PARITY_ODD;
            break;
        case PARITY_EVEN:
            LPUART_InitStructer.parity = FL_LPUART_PARITY_EVEN;
            break;
        default:
            return OS_EINVAL;
    }

    switch (cfg->stop_bits)
    {
        case STOP_BITS_1:
            LPUART_InitStructer.stopBits = FL_LPUART_STOP_BIT_WIDTH_1B;
            break;
        case STOP_BITS_2:
            LPUART_InitStructer.stopBits = FL_LPUART_STOP_BIT_WIDTH_2B;
            break;
        default:
            return OS_EINVAL;
    }

    LPUART_InitStructer.transferDirection   = FL_UART_DIRECTION_TX;
    FL_LPUART_Init(lpuart->info->hlpuart, &LPUART_InitStructer);


    NVIC_DisableIRQ(lpuart->info->irqn);
    NVIC_SetPriority(lpuart->info->irqn, lpuart->info->irq_priority);
    NVIC_EnableIRQ(lpuart->info->irqn);

    fm33_lpuart_sdma_init(lpuart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t fm33_lpuart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    struct fm33_lpuart  *lpuart  = os_container_of(serial, struct fm33_lpuart, serial);

    if (lpuart->info->hdma != OS_NULL)
    {
        soft_dma_stop(&lpuart->sdma);
    }

    FL_LPUART_DeInit(lpuart->info->hlpuart);

    return OS_EOK;
}

static int fm33_lpuart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct fm33_lpuart  *lpuart = os_container_of(serial, struct fm33_lpuart, serial);

    OS_ASSERT(serial != OS_NULL);

    lpuart->tx_buff  = buff;
    lpuart->tx_size  = size;
    lpuart->tx_index = 0;

    FL_LPUART_ClearFlag_TXShiftBuffEmpty(lpuart->info->hlpuart);
    FL_LPUART_EnableIT_TXShiftBuffEmpty(lpuart->info->hlpuart);

    FL_LPUART_WriteTXBuff(lpuart->info->hlpuart, lpuart->tx_buff[lpuart->tx_index++]);

    return size;
}

static int fm33_lpuart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct fm33_lpuart  *lpuart = os_container_of(serial, struct fm33_lpuart, serial);

    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();

        FL_LPUART_WriteTXBuff(lpuart->info->hlpuart, buff[i]);
        while(FL_LPUART_IsActiveFlag_TXShiftBuffEmpty(lpuart->info->hlpuart) == 0);
        FL_LPUART_ClearFlag_TXShiftBuffEmpty(lpuart->info->hlpuart);

        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops fm33_lpuart_ops =
{
    .init       = fm33_lpuart_init,
    .deinit     = fm33_lpuart_deinit,

    .start_send = fm33_lpuart_start_send,
    .poll_send  = fm33_lpuart_poll_send,
};

static int fm33_lpuart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    struct fm33_lpuart *lpuart     = OS_NULL;

    lpuart = os_calloc(1, sizeof(struct fm33_lpuart));
    OS_ASSERT(lpuart);

    lpuart->info = (struct fm33_lpuart_info *)dev->info;
    struct os_serial_device *serial = &lpuart->serial;

    level = os_irq_lock();
    os_list_add_tail(&fm33_lpuart_list, &lpuart->list);
    os_irq_unlock(level);

    serial->ops    = &fm33_lpuart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, lpuart);

    return 0;
}

OS_DRIVER_INFO fm33_lpuart_driver = {
    .name   = "Lpuart_Type",
    .probe  = fm33_lpuart_probe,
};

OS_DRIVER_DEFINE(fm33_lpuart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

