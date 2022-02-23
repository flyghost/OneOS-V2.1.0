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
 * @brief       This file implements usart driver for cm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <string.h>
#include "drv_usart.h"

static USART_Module *console_uart = OS_NULL;
static os_list_node_t cm32_uart_list = OS_LIST_INIT(cm32_uart_list);

/* uart driver */
typedef struct cm_uart
{
    struct os_serial_device serial_dev;
    struct cm32_usart_info *info;
    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;

    uint8_t  *rx_buff;
    uint32_t  rx_index;
    uint32_t  rx_size;
    os_list_node_t list;
} cm_uart_t;

static void uart_isr(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    cm_uart_t *uart = os_container_of(serial, cm_uart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    if(USART_GetFlagStatus(uart->info->idx, USART_FLAG_RXDNE) != RESET)
    {
        uart->rx_buff[uart->rx_index++] = USART_ReceiveData(uart->info->idx);

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

void uart_irq(int device_num)
{
    struct cm_uart *uart;

    os_list_for_each_entry(uart, &cm32_uart_list, struct cm_uart, list)
    {
        if (uart->info->uart_device == device_num)
        {
            uart_isr(&uart->serial_dev);
            break;
        }
    }
}

void USART1_IRQHandler(void)
{
    uart_irq(1);
}

void USART2_IRQHandler(void)
{
    uart_irq(2);
}

void USART3_IRQHandler(void)
{
    uart_irq(3);
}

void UART4_IRQHandler(void)
{
    uart_irq(4);
}

void UART5_IRQHandler(void)
{
    uart_irq(5);
}

/* interrupt rx mode */
static os_uint32_t cm_uart_sdma_int_get_index(soft_dma_t *dma)
{
    cm_uart_t *uart = os_container_of(dma, cm_uart_t, sdma);

    return uart->rx_index;
}

static os_err_t cm_uart_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    cm_uart_t *uart = os_container_of(dma, cm_uart_t, sdma);

    uart->rx_buff  = buff;
    uart->rx_index = 0;
    uart->rx_size  = size;

    USART_ConfigInt(uart->info->idx, USART_INT_RXDNE, ENABLE);

    return OS_EOK;
}

static os_uint32_t cm_uart_sdma_int_stop(soft_dma_t *dma)
{
    cm_uart_t *uart = os_container_of(dma, cm_uart_t, sdma);

    USART_ConfigInt(uart->info->idx, USART_INT_RXDNE, DISABLE);

    return uart->rx_index;
}

#if defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA) \
    || defined(UART4_USING_DMA) || defined(UART5_USING_DMA)
/* dma rx mode */
static os_uint32_t cm_uart_sdma_dma_get_index(soft_dma_t *dma)
{
    cm_uart_t *uart = os_container_of(dma, cm_uart_t, sdma);

    if (uart->info->dma_support == 1)
    {
        return uart->sdma_hard_size - DMA_GetCurrDataCounter(uart->info->dma_channel);
    }

    return 0;
}

static os_err_t cm_uart_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t cm_uart_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    cm_uart_t *uart = os_container_of(dma, cm_uart_t, sdma);

    uart->sdma_hard_size = size;

    DMA_InitType DMA_InitStructure;

    /* DMA clock enable */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_DMA, ENABLE);

    DMA_DeInit(uart->info->dma_channel);
    DMA_InitStructure.PeriphAddr     = uart->info->periph_addr + 0x04;
    DMA_InitStructure.MemAddr        = (uint32_t)buff;
    DMA_InitStructure.Direction      = DMA_DIR_PERIPH_SRC;
    DMA_InitStructure.BufSize        = size;
    DMA_InitStructure.PeriphInc      = DMA_PERIPH_INC_DISABLE;
    DMA_InitStructure.DMA_MemoryInc  = DMA_MEM_INC_ENABLE;
    DMA_InitStructure.PeriphDataSize = DMA_PERIPH_DATA_SIZE_BYTE;
    DMA_InitStructure.MemDataSize    = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.CircularMode   = DMA_MODE_CIRCULAR;
    DMA_InitStructure.Priority       = DMA_PRIORITY_VERY_HIGH;
    DMA_InitStructure.Mem2Mem        = DMA_M2M_DISABLE;
    DMA_Init(uart->info->dma_channel, &DMA_InitStructure);
    DMA_RequestRemap(uart->info->dma_remap, DMA, uart->info->dma_channel, ENABLE);

    USART_EnableDMA(uart->info->idx, USART_DMAREQ_RX, ENABLE);
    DMA_EnableChannel(uart->info->dma_channel, ENABLE);

    return OS_EOK;
}

static os_uint32_t cm_uart_sdma_dma_stop(soft_dma_t *dma)
{
    cm_uart_t *uart = os_container_of(dma, cm_uart_t, sdma);

    if (uart->info->dma_support == 1)
    {
        USART_EnableDMA(uart->info->idx, USART_DMAREQ_RX, DISABLE);
        DMA_EnableChannel(uart->info->dma_channel, DISABLE);
    }

    return cm_uart_sdma_dma_get_index(dma);
}
#endif

/* sdma callback */
static void cm_uart_sdma_callback(soft_dma_t *dma)
{
    cm_uart_t *uart = os_container_of(dma, cm_uart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void cm_uart_sdma_init(struct cm_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;
    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(uart->serial_dev.config.baud_rate);

    if (uart->info->dma_support == 0)
    {
        //dma->hard_info.flag         = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;

        dma->ops.get_index          = cm_uart_sdma_int_get_index;
        dma->ops.dma_init           = OS_NULL;
        dma->ops.dma_start          = cm_uart_sdma_int_start;
        dma->ops.dma_stop           = cm_uart_sdma_int_stop;
    }
#if defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA) \
    || defined(UART4_USING_DMA) || defined(UART5_USING_DMA)
    else
    {
        dma->hard_info.flag        |= HARD_DMA_FLAG_FULL_IRQ;
        dma->ops.get_index          = cm_uart_sdma_dma_get_index;
        dma->ops.dma_init           = cm_uart_sdma_dma_init;
        dma->ops.dma_start          = cm_uart_sdma_dma_start;
        dma->ops.dma_stop           = cm_uart_sdma_dma_stop;
    }
#endif

    dma->cbs.dma_half_callback      = cm_uart_sdma_callback;
    dma->cbs.dma_full_callback      = cm_uart_sdma_callback;
    dma->cbs.dma_timeout_callback   = cm_uart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&uart->sdma, OS_TRUE);
}

void GPIO_Configuration_uart(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    cm_uart_t *uart = os_container_of(serial, cm_uart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    GPIO_InitType GPIO_InitStructure;

    /* Initialize GPIO_InitStructure */
    GPIO_InitStruct(&GPIO_InitStructure);

    /* Configure UARTx Tx as alternate function push-pull */
    GPIO_InitStructure.Pin            = uart->info->tx_pin;
    GPIO_InitStructure.GPIO_Mode      = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Alternate = uart->info->tx_af;
    GPIO_InitPeripheral(uart->info->tx_port, &GPIO_InitStructure);

    /* Configure UARTx Rx as alternate function push-pull and pull-up */
    GPIO_InitStructure.Pin            = uart->info->rx_pin;
    GPIO_InitStructure.GPIO_Pull      = GPIO_Pull_Up;
    GPIO_InitStructure.GPIO_Alternate = uart->info->rx_af;
    GPIO_InitPeripheral(uart->info->rx_port, &GPIO_InitStructure);
}

void RCC_Configuration_uart(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    cm_uart_t *uart = os_container_of(serial, cm_uart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    /* Enable GPIO clock */
    RCC_EnableAPB2PeriphClk(uart->info->uart_gpio_clk, ENABLE);

    /* Enable UARTx Clock */
    if ((uart->info->uart_device == 2) || (uart->info->uart_device == 3))
    {
        RCC_EnableAPB1PeriphClk(uart->info->uart_clk, ENABLE);
    }
    else
    {
        RCC_EnableAPB2PeriphClk(uart->info->uart_clk, ENABLE);
    }
}

void NVIC_Configuration_uart(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);
    cm_uart_t *uart = os_container_of(serial, cm_uart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    NVIC_InitType NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    /* Enable the UARTx Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel            = uart->info->irqn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd         = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static os_err_t cm_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    USART_InitType USART_InitStructure;

    /* System Clocks Configuration */
    RCC_Configuration_uart(serial);

    /* NVIC configuration */
    NVIC_Configuration_uart(serial);

    /* Configure the GPIO ports */
    GPIO_Configuration_uart(serial);

    OS_ASSERT(serial != OS_NULL);
    cm_uart_t *uart = os_container_of(serial, cm_uart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    /* UARTx configuration */
    USART_InitStructure.BaudRate            = cfg->baud_rate;
    USART_InitStructure.WordLength          = USART_WL_8B;
    switch (cfg->stop_bits)
    {
    case STOP_BITS_1:
        USART_InitStructure.StopBits        = USART_STPB_1;
        break;
    case STOP_BITS_2:
        USART_InitStructure.StopBits        = USART_STPB_2;
        break;
    default:
        return OS_EINVAL;
    }
    switch (cfg->parity)
    {
    case PARITY_NONE:
        USART_InitStructure.Parity          = USART_PE_NO;
        break;
    case PARITY_ODD:
        USART_InitStructure.Parity          = USART_PE_ODD;
        break;
    case PARITY_EVEN:
        USART_InitStructure.Parity          = USART_PE_EVEN;
        break;
    default:
        return OS_EINVAL;
    }
    USART_InitStructure.HardwareFlowControl = USART_HFCTRL_NONE;
    USART_InitStructure.Mode                = USART_MODE_RX | USART_MODE_TX;

    /* Configure UARTx */
    USART_Init(uart->info->idx, &USART_InitStructure);
    /* Enable the UARTx */
    USART_Enable(uart->info->idx, ENABLE);

    if (uart->info->dma_support == 0)
    {
        USART_ConfigInt(uart->info->idx, USART_INT_RXDNE, ENABLE);
    }

    cm_uart_sdma_init(uart, &uart->serial_dev.rx_fifo->ring);

    return OS_EOK;
}

static os_err_t cm_uart_deinit(struct os_serial_device *serial)
{
    OS_ASSERT(serial != OS_NULL);

    cm_uart_t *uart = os_container_of(serial, cm_uart_t, serial_dev);

    if (uart->info->dma_support == 0)
    {
        USART_ConfigInt(uart->info->idx, USART_INT_RXDNE, DISABLE);
    }
#if defined(UART1_USING_DMA) || defined(UART2_USING_DMA) || defined(UART3_USING_DMA) \
    || defined(UART4_USING_DMA) || defined(UART5_USING_DMA)
    else
    {
        USART_EnableDMA(uart->info->idx, USART_DMAREQ_RX, DISABLE);
        DMA_EnableChannel(uart->info->dma_channel, DISABLE);
    }
#endif

    return OS_EOK;
}

/*
static int cm_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    return 0;
}
*/

static int cm_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);
    cm_uart_t *uart = os_container_of(serial, cm_uart_t, serial_dev);

    OS_ASSERT(uart != OS_NULL);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();

        USART_SendData(uart->info->idx, buff[i]);
        /* Loop until UARTx DAT register is empty */
        while (USART_GetFlagStatus(uart->info->idx, USART_FLAG_TXDE) == RESET)
        {
        }

        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops cm_uart_ops =
{
    .init         = cm_uart_init,
    .deinit       = cm_uart_deinit,

    .poll_send    = cm_uart_poll_send,
};

void __os_hw_console_output(char *str)
{
    if (console_uart == OS_NULL)
        return;

    while (*str)
    {
        //USART_SendData(console_uart, *str);
        str++;
    }
}

static int cm32_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    struct cm_uart *usart;

    usart = os_calloc(1, sizeof(struct cm_uart));
    OS_ASSERT(usart);

    usart->info = (struct cm32_usart_info *)dev->info;
    struct os_serial_device *serial = &usart->serial_dev;

    level = os_irq_lock();
    os_list_add_tail(&cm32_uart_list, &usart->list);
    os_irq_unlock(level);

    serial->ops    = &cm_uart_ops;
    serial->config = config;

    /* register uart device */
    os_hw_serial_register(serial, dev->name, usart);

    return 0;
}

OS_DRIVER_INFO cm32_usart_driver = {
    .name   = "Usart_Type",
    .probe  = cm32_usart_probe,
};

OS_DRIVER_DEFINE(cm32_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);
