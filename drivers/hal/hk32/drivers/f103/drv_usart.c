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
 * @brief       This file implements usart driver for hk32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <bus/bus.h>
#include <dma/dma.h>
#include <os_stddef.h>
#include <string.h>
#include <stdint.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

#include <drv_common.h>
#include <drv_usart.h>

typedef struct hk32_usart
{
    struct os_serial_device serial;

    const struct hk32_usart_info *info;

    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;

    DMA_InitTypeDef DMA_InitStructure;

    os_uint8_t *rx_buff;
    os_uint32_t rx_index;
    os_uint32_t rx_size;

    const os_uint8_t *tx_buff;
    os_uint32_t tx_count;
    os_uint32_t tx_size;

    os_list_node_t list;
}hk32_usart_t;

static os_list_node_t hk32_usart_list = OS_LIST_INIT(hk32_usart_list);

static const struct hk32_usart_info *console_uart = OS_NULL;

static void hk32_usart_interrupt_rx(hk32_usart_t *uart)
{
    if (USART_GetITStatus(uart->info->huart, USART_IT_RXNE) != RESET)
    {
        OS_ASSERT(uart->rx_buff != OS_NULL);
        OS_ASSERT(uart->rx_index < uart->rx_size);
        
        USART_ClearFlag(uart->info->huart, USART_IT_RXNE);
        USART_ITConfig(uart->info->huart, USART_IT_IDLE, ENABLE);

        uart->rx_buff[uart->rx_index++] = USART_ReceiveData(uart->info->huart);
    }

    if (USART_GetITStatus(uart->info->huart, USART_IT_IDLE) != RESET)
    {
        USART_ClearFlag(uart->info->huart, USART_IT_IDLE);
        USART_ReceiveData(uart->info->huart);
    
        if (uart->rx_index > 0)
        {
            soft_dma_timeout_irq(&uart->sdma);
        }
    }

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

static void hk32_usart_dma_rx(hk32_usart_t *uart)
{
    if (USART_GetITStatus(uart->info->huart, USART_IT_RXNE) != RESET)
    {
        USART_ClearFlag(uart->info->huart, USART_IT_RXNE);
        USART_ReceiveData(uart->info->huart);
    }

    if (USART_GetITStatus(uart->info->huart, USART_IT_IDLE) != RESET)
    {
        USART_ClearFlag(uart->info->huart, USART_IT_IDLE);
        USART_ReceiveData(uart->info->huart);

        soft_dma_timeout_irq(&uart->sdma);
    }
}

static void hk32_usart_irq_callback(hk32_usart_t *uart)
{
    /* rx */
    if (uart->info->dma_channel == OS_NULL)
    {
        hk32_usart_interrupt_rx(uart);
    }
    else
    {
        hk32_usart_dma_rx(uart);
    }

    /* tx */
    if (USART_GetITStatus(uart->info->huart, USART_IT_TXE) != RESET)
    {
        USART_ClearFlag(uart->info->huart, USART_IT_TXE);
        
        if (uart->tx_size > 0)
        {
            if (uart->tx_count < uart->tx_size)
                USART_SendData(uart->info->huart, uart->tx_buff[uart->tx_count++]);

            if (uart->tx_count >= uart->tx_size)
            {
                uart->tx_size = 0;
                USART_ITConfig(uart->info->huart, USART_IT_TXE, DISABLE);
                os_hw_serial_isr_txdone((struct os_serial_device *)uart);
            }
        }
    }
}

static void usart_irqhandler(USART_TypeDef *huart)
{
    hk32_usart_t *uart;
    
    os_list_for_each_entry(uart, &hk32_usart_list, hk32_usart_t, list)
    {
        if (uart->info->huart == huart)
        {
            hk32_usart_irq_callback(uart);
            return;
        }
    }
}

static void usart_dma_irqhandler(USART_TypeDef *huart)
{
    hk32_usart_t *uart;
    
    os_list_for_each_entry(uart, &hk32_usart_list, hk32_usart_t, list)
    {
        if (uart->info->huart == huart)
        {
            soft_dma_half_irq(&uart->sdma);
            return;
        }
    }
}

void USART1_IRQHandler(void)
{
#ifdef USART1
    usart_irqhandler(USART1);
#endif
}

void USART2_IRQHandler(void)
{
#ifdef USART2
    usart_irqhandler(USART2);
#endif
}

void USART3_IRQHandler(void)
{
#ifdef USART3
    usart_irqhandler(USART3);
#endif
}

void DMA1_Channel5_IRQHandler(void)
{
    DMA_ClearITPendingBit(DMA1_IT_HT5);
    DMA_ClearITPendingBit(DMA1_IT_TC5);

#ifdef USART1
    usart_dma_irqhandler(USART1);
#endif
}

void DMA1_Channel6_IRQHandler(void)
{
    DMA_ClearITPendingBit(DMA1_IT_HT6);
    DMA_ClearITPendingBit(DMA1_IT_TC6);

#ifdef USART2
    usart_dma_irqhandler(USART2);
#endif
}

void DMA1_Channel3_IRQHandler(void)
{
    DMA_ClearITPendingBit(DMA1_IT_HT3);
    DMA_ClearITPendingBit(DMA1_IT_TC3);

#ifdef USART3
    usart_dma_irqhandler(USART3);
#endif
}

/* interrupt rx mode */
static os_uint32_t hk32_sdma_int_get_index(soft_dma_t *dma)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    return uart->rx_index;
}

static os_err_t hk32_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    uart->rx_buff  = buff;
    uart->rx_index = 0;
    uart->rx_size  = size;

    USART_ITConfig(uart->info->huart, USART_IT_RXNE, ENABLE);
    
    return OS_EOK;
}

static os_uint32_t hk32_sdma_int_stop(soft_dma_t *dma)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    USART_ITConfig(uart->info->huart, USART_IT_RXNE, DISABLE);
    USART_ITConfig(uart->info->huart, USART_IT_IDLE, DISABLE);
    
    return hk32_sdma_int_get_index(dma);
}

/* dma rx mode */
static os_uint32_t hk32_sdma_dma_get_index(soft_dma_t *dma)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    return uart->sdma_hard_size - DMA_GetCurrDataCounter(uart->info->dma_channel);
}

static os_err_t hk32_sdma_dma_init(soft_dma_t *dma)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    RCC_AHBPeriphClockCmd(uart->info->dma_rcc, ENABLE);

    uart->DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&uart->info->huart->DR;
    uart->DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    uart->DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    uart->DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    uart->DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    uart->DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    uart->DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    uart->DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    uart->DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = uart->info->dma_irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    return OS_EOK;
}

static os_err_t hk32_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    uart->sdma_hard_size = size;

    uart->DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buff;
    uart->DMA_InitStructure.DMA_BufferSize = size;
    
    DMA_Init(uart->info->dma_channel, &uart->DMA_InitStructure);
    DMA_Cmd(uart->info->dma_channel, ENABLE);
    USART_DMACmd(uart->info->huart, USART_DMAReq_Rx, ENABLE);

    USART_ITConfig(uart->info->huart, USART_IT_IDLE, ENABLE);
    USART_ITConfig(uart->info->huart, USART_IT_RXNE, ENABLE);

    DMA_ITConfig(uart->info->dma_channel, DMA_IT_HT | DMA_IT_TC, ENABLE);

    return OS_EOK;
}

static os_uint32_t hk32_sdma_dma_stop(soft_dma_t *dma)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    DMA_Cmd(uart->info->dma_channel, DISABLE);

    return hk32_sdma_dma_get_index(dma);
}

/* sdma callback */
static void hk32_usart_sdma_callback(soft_dma_t *dma)
{
    hk32_usart_t *uart = os_container_of(dma, hk32_usart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void hk32_usart_sdma_init(struct hk32_usart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_CIRCULAR;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ | HARD_DMA_FLAG_TIMEOUT_IRQ;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);

    if (uart->info->dma_channel == OS_NULL)
    {
        dma->ops.get_index          = hk32_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = hk32_sdma_int_start;
        dma->ops.dma_stop           = hk32_sdma_int_stop;
    }
    else
    {
        dma->ops.get_index          = hk32_sdma_dma_get_index;
        dma->ops.dma_init           = hk32_sdma_dma_init;
        dma->ops.dma_start          = hk32_sdma_dma_start;
        dma->ops.dma_stop           = hk32_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback      = hk32_usart_sdma_callback;
    dma->cbs.dma_full_callback      = hk32_usart_sdma_callback;
    dma->cbs.dma_timeout_callback   = hk32_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

static int __hk32_usart_init(const struct hk32_usart_info *uart_info, struct serial_configure *cfg)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    os_uint32_t       data_bits;
    
    RCC_APB2PeriphClockCmd(uart_info->gpio_tx_rcc, ENABLE);
    RCC_APB2PeriphClockCmd(uart_info->gpio_rx_rcc, ENABLE);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = uart_info->gpio_tx_pin;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_Init(uart_info->gpio_tx_port, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin   = uart_info->gpio_rx_pin;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(uart_info->gpio_rx_port, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = cfg->baud_rate;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
        break;
    case STOP_BITS_2:
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
        break;
    default:
        return OS_EINVAL;
    }
    switch (cfg->parity)
    {
    case PARITY_NONE:
        USART_InitStructure.USART_Parity = USART_Parity_No;
        data_bits                 = cfg->data_bits;
        break;
    case PARITY_ODD:
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
        data_bits                 = cfg->data_bits + 1;
        break;
    case PARITY_EVEN:
        USART_InitStructure.USART_Parity = USART_Parity_Even;
        data_bits                 = cfg->data_bits + 1;
        break;
    default:
        return OS_EINVAL;
    }

    switch (data_bits)
    {
    case DATA_BITS_8:
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        break;
    case DATA_BITS_9:
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
        break;
    default:
        return OS_EINVAL;
    }

    if (uart_info->rcc_tpye == HK32_RCC_APB1)
        RCC_APB1PeriphClockCmd(uart_info->rcc, ENABLE);

    if (uart_info->rcc_tpye == HK32_RCC_APB2)
        RCC_APB2PeriphClockCmd(uart_info->rcc, ENABLE);

    USART_DeInit(uart_info->huart);    
    USART_Init(uart_info->huart, &USART_InitStructure);
    USART_ITConfig(uart_info->huart, USART_IT_IDLE, DISABLE);
    USART_ITConfig(uart_info->huart, USART_IT_RXNE, DISABLE);
    USART_Cmd(uart_info->huart, ENABLE);

    return OS_EOK;
}

static os_err_t hk32_usart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    struct hk32_usart *uart;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct hk32_usart, serial);

    __hk32_usart_init(uart->info, cfg);

    NVIC_InitTypeDef  NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannel = uart->info->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    hk32_usart_sdma_init(uart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t hk32_usart_deinit(struct os_serial_device *serial)
{
    struct hk32_usart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct hk32_usart, serial);

    /* rx */
    USART_ITConfig(uart->info->huart, USART_IT_IDLE, DISABLE);
    USART_ITConfig(uart->info->huart, USART_IT_RXNE, DISABLE);

    if (uart->info->dma_channel != OS_NULL)
    {
        soft_dma_stop(&uart->sdma);
        DMA_DeInit(uart->info->dma_channel);
    }

    /* tx */
    USART_ITConfig(uart->info->huart, USART_IT_TXE, DISABLE);

    uart->tx_buff  = OS_NULL;
    uart->tx_count = 0;
    uart->tx_size  = 0;

    return 0;
}

static int hk32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct hk32_usart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct hk32_usart, serial);

    uart->tx_buff  = buff;
    uart->tx_count = 0;
    uart->tx_size  = size;

    USART_ITConfig(uart->info->huart, USART_IT_TXE, ENABLE);

    return size;
}

static int hk32_usart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    struct hk32_usart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct hk32_usart, serial);

    for (i = 0; i < size; i++)
    {
        while (USART_GetFlagStatus(uart->info->huart, USART_FLAG_TC) == RESET);
        USART_SendData(uart->info->huart, buff[i]);
    }

    return size;
}

static const struct os_uart_ops hk32_usart_ops = {
    .init         = hk32_usart_init,
    .deinit       = hk32_usart_deinit,

    .start_send   = hk32_uart_start_send,
    .poll_send    = hk32_usart_poll_send,
};

static int hk32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t    result  = 0;
    os_base_t   level;

    struct hk32_usart *uart = os_calloc(1, sizeof(struct hk32_usart));

    OS_ASSERT(uart);

    uart->info = dev->info;

    struct os_serial_device *serial = &uart->serial;

    serial->ops    = &hk32_usart_ops;
    serial->config = config;

    level = os_irq_lock();
    os_list_add_tail(&hk32_usart_list, &uart->list);
    os_irq_unlock(level);

    result = os_hw_serial_register(serial, dev->name, NULL);

    OS_ASSERT(result == OS_EOK);

    return result;
}

void __os_hw_console_output(char *str)
{
    int i;
    
    if (console_uart == OS_NULL)
        return;

    for (i = 0; i < strlen(str); i++)
    {
        while (USART_GetFlagStatus(console_uart->huart, USART_FLAG_TC) == RESET);
            USART_SendData(console_uart->huart, str[i]);
    }

}

OS_DRIVER_INFO hk32_usart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = hk32_usart_probe,
};

OS_DRIVER_DEFINE(hk32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

static int hk32_usart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if(strcmp(dev->name, OS_CONSOLE_DEVICE_NAME))
        return OS_EOK;
    
    console_uart = (const struct hk32_usart_info *)dev->info;

    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    __hk32_usart_init(console_uart, &config);
    
    return OS_EOK;
}

OS_DRIVER_INFO hk32_usart_early_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = hk32_usart_early_probe,
};

OS_DRIVER_DEFINE(hk32_usart_early_driver, CORE, OS_INIT_SUBLEVEL_LOW);
