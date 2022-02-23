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
 * @brief       This file implements uart driver for mm32
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-31   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "mm32_hal.h"
#include "mm32_it.h"
#include "drv_uart.h"
#include "drv_gpio.h"

#include <os_memory.h>
#include <bus/bus.h>
#include <dma/dma.h>
#include <board.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.uart"
#include <drv_log.h>

static os_list_node_t mm32_uart_list = OS_LIST_INIT(mm32_uart_list);

void hal_uart_dmatx_callback(void *handle, irqn_type_flag type_flag);
void hal_uart_dmarx_callback(void *handle, irqn_type_flag type_flag);
static void mm32_uart_hard_init(mm32_uart_info_t *info)
{
#ifdef GPIOA
    if ((info->tx_pin_port == GPIOA) || (info->tx_pin_port == GPIOA))
        GPIO_POAR_CLK_ENABLE(A);
#endif
#ifdef GPIOB
    if ((info->tx_pin_port == GPIOB) || (info->tx_pin_port == GPIOB))
        GPIO_POAR_CLK_ENABLE(B);
#endif
#ifdef GPIOC
    if ((info->tx_pin_port == GPIOC) || (info->tx_pin_port == GPIOC))
        GPIO_POAR_CLK_ENABLE(C);
#endif
#ifdef GPIOD
    if ((info->tx_pin_port == GPIOD) || (info->tx_pin_port == GPIOD))
        GPIO_POAR_CLK_ENABLE(D);
#endif
#ifdef GPIOE
    if ((info->tx_pin_port == GPIOE) || (info->tx_pin_port == GPIOE))
        GPIO_POAR_CLK_ENABLE(E);
#endif
#ifdef GPIOF
    if ((info->tx_pin_port == GPIOF) || (info->tx_pin_port == GPIOF))
        GPIO_POAR_CLK_ENABLE(F);
#endif
#ifdef GPIOG
    if ((info->tx_pin_port == GPIOG) || (info->tx_pin_port == GPIOG))
        GPIO_POAR_CLK_ENABLE(G);
#endif   
#ifdef GPIOH
    if ((info->tx_pin_port == GPIOH) || (info->tx_pin_port == GPIOH))
        GPIO_POAR_CLK_ENABLE(H);
#endif

    GPIO_Init(info->tx_pin_port, &info->tx_pin_info);
    GPIO_Init(info->rx_pin_port, &info->rx_pin_info);
    
#if  defined(SERIES_MM32F013XX) || defined(SERIES_MM32F027XX) || defined(SERIES_MM32F327XX)
    GPIO_PinAFConfig(info->tx_pin_port, info->tx_pin_source, info->pin_af);
    GPIO_PinAFConfig(info->rx_pin_port, info->rx_pin_source, info->pin_af);
#endif

#if defined(UART1) && defined(UART6)
    if (info->huart == UART1 || info->huart == UART6)
#elif defined(UART1)
    if (info->huart == UART1)
#else
    if (info->huart == UART6)
#endif
        RCC_APB2PeriphClockCmd(info->uart_clk, ENABLE);
    else
        RCC_APB1PeriphClockCmd(info->uart_clk, ENABLE);
    
    NVIC_Init(&info->uart_nvic_info);
    UART_Init(info->huart, &info->uart_info);
    UART_Cmd(info->huart, ENABLE);

    if (info->dma_tx_channel != 0)
        dma_info_table_config(info->dma_tx_channel, info->huart, hal_uart_dmatx_callback);
    if (info->dma_rx_channel != 0)
        dma_info_table_config(info->dma_rx_channel, info->huart, hal_uart_dmarx_callback);
}

static void mm32_uart_irq_rx_callback(mm32_uart_t *uart)
{
    if (uart->info->dma_rx_channel == 0)
    {
        OS_ASSERT(uart->rx_buff != OS_NULL);
        OS_ASSERT(uart->rx_index < uart->rx_size);

        uart->rx_buff[uart->rx_index++] = UART_ReceiveData(uart->info->huart);
        UART_ClearITPendingBit(uart->info->huart, MM32_UART_ISR_RX);
        
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
    {
        UART_ReceiveData(uart->info->huart);
        UART_ClearITPendingBit(uart->info->huart, MM32_UART_ISR_RX);
    }
}

#ifdef MM32_UART_ISR_RXIDLE
static void mm32_uart_irq_rxidle_callback(mm32_uart_t *uart)
{
    if (uart->info->dma_rx_channel == 0)
    {
        UART_ClearITPendingBit(uart->info->huart, MM32_UART_ISR_RXIDLE);
        UART_ReceiveData(uart->info->huart);
    
        if (uart->rx_index > 0)
        {
            soft_dma_timeout_irq(&uart->sdma);
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
    else
    {
        UART_ReceiveData(uart->info->huart);
        UART_ClearITPendingBit(uart->info->huart, MM32_UART_ISR_RXIDLE);
        soft_dma_timeout_irq(&uart->sdma);
    }
}
#endif
static void mm32_uart_irq_tx_callback(mm32_uart_t *uart)
{
    UART_ClearITPendingBit(uart->info->huart, MM32_UART_ISR_TX);
    uart->tx_count++;
    if(uart->tx_count >= uart->tx_size)
    {
        UART_ITConfig(uart->info->huart, MM32_UART_IER_TX, DISABLE);
        while (!UART_GetFlagStatus(uart->info->huart, UART_CSR_TXC));
        os_hw_serial_isr_txdone((struct os_serial_device *)uart);
    }
    else
    {
        UART_SendData(uart->info->huart, uart->tx_buff[uart->tx_count]);
    }
}

static void mm32_uart_irq_callback(mm32_uart_t *uart)
{
    if ((RESET != UART_GetITStatus(uart->info->huart, MM32_UART_ISR_RX))
        && ((uart->info->huart->IER & MM32_UART_IER_RX) != RESET))
    {
        mm32_uart_irq_rx_callback(uart);
    }
#ifdef MM32_UART_IER_RXIDLE
    if ((RESET != UART_GetITStatus(uart->info->huart, MM32_UART_ISR_RXIDLE))
        && ((uart->info->huart->IER & MM32_UART_IER_RXIDLE) != RESET))
    {
        mm32_uart_irq_rxidle_callback(uart);
    }
#endif
    if ((RESET != UART_GetITStatus(uart->info->huart, MM32_UART_ISR_TX))
        && ((uart->info->huart->IER & MM32_UART_IER_TX) != RESET))
    {
        mm32_uart_irq_tx_callback(uart);
    }
}

void HAL_UART_IRQHandler(UART_TypeDef *huart)
{
    struct mm32_uart *uart;

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->info->huart == huart)
        {
            mm32_uart_irq_callback(uart);
            return;
        }
    }
}

void hal_uart_dmatx_callback(void *handle, irqn_type_flag type_flag)
{
    struct mm32_uart *uart = OS_NULL;

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->info->huart != (UART_TypeDef *)handle)
            continue;

        if(type_flag == TYPE_DMA_IT_TC)
        {
            DMA_Cmd(uart->info->dma_tx_channel, DISABLE);
            os_hw_serial_isr_txdone((struct os_serial_device *)uart);
        }
    }
}

void hal_uart_dmarx_callback(void *handle, irqn_type_flag type_flag)
{
    struct mm32_uart *uart = OS_NULL;

    os_list_for_each_entry(uart, &mm32_uart_list, struct mm32_uart, list)
    {
        if (uart->info->huart != (UART_TypeDef *)handle)
            continue;

        if(type_flag == TYPE_DMA_IT_TC)
        {
            soft_dma_full_irq(&uart->sdma);
        }
        if(type_flag == TYPE_DMA_IT_HT)
        {
            soft_dma_half_irq(&uart->sdma);
        }
    }
}

void mm32_uart_dma_tx(mm32_uart_t *uart, const os_uint8_t *buff, os_size_t size)
{
    DMA_InitTypeDef DMA_InitStruct;
    
    DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (os_uint32_t)&uart->info->huart->TDR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (os_uint32_t)buff;
    DMA_InitStruct.DMA_BufferSize = size;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStruct.DMA_Priority = uart->info->dma_info.TX_DMA_Priority;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
    
    RCC_AHBPeriphClockCmd(uart->info->dma_tx_clk, ENABLE);
    DMA_DeInit(uart->info->dma_tx_channel);
    DMA_Init(uart->info->dma_tx_channel, &DMA_InitStruct);
    DMA_ITConfig(uart->info->dma_tx_channel, DMA_IT_TC, ENABLE);
    UART_DMACmd(uart->info->huart, UART_DMAReq_EN, ENABLE);
    DMA_Cmd(uart->info->dma_tx_channel, ENABLE);
    NVIC_Init(&uart->info->dma_tx_nvic_info);
}
void mm32_uart_dma_rx(mm32_uart_t *uart, os_uint8_t *buff, os_size_t size)
{
    DMA_InitTypeDef DMA_InitStruct;
    
    DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStruct.DMA_PeripheralBaseAddr = (os_uint32_t)&uart->info->huart->RDR;
    DMA_InitStruct.DMA_MemoryBaseAddr = (os_uint32_t)buff;
    DMA_InitStruct.DMA_BufferSize = size;
    DMA_InitStruct.DMA_Mode = uart->info->dma_info.RX_DMA_Mode;
    DMA_InitStruct.DMA_Priority = uart->info->dma_info.RX_DMA_Priority;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;

    RCC_AHBPeriphClockCmd(uart->info->dma_rx_clk, ENABLE);
    DMA_DeInit(uart->info->dma_rx_channel);
    DMA_Init(uart->info->dma_rx_channel, &DMA_InitStruct);
    
    DMA_ITConfig(uart->info->dma_rx_channel, DMA_IT_TC, ENABLE);
    DMA_ITConfig(uart->info->dma_rx_channel, DMA_IT_HT, ENABLE);
    UART_DMACmd(uart->info->huart, UART_DMAReq_EN, ENABLE);
    DMA_Cmd(uart->info->dma_rx_channel, ENABLE);
    NVIC_Init(&uart->info->dma_rx_nvic_info);
}

/* interrupt rx mode */
static os_uint32_t mm32_sdma_int_get_index(soft_dma_t *dma)
{
    mm32_uart_t *uart = os_container_of(dma, mm32_uart_t, sdma);

    return uart->rx_index;
}

static os_err_t mm32_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    mm32_uart_t *uart = os_container_of(dma, mm32_uart_t, sdma);

    uart->rx_buff  = buff;
    uart->rx_index = 0;
    uart->rx_size  = size;

    UART_ITConfig(uart->info->huart, MM32_UART_IER_RX, ENABLE);
#ifdef MM32_UART_IER_RXIDLE
    UART_ITConfig(uart->info->huart, MM32_UART_ISR_RXIDLE, ENABLE);
#endif
    return OS_EOK;
}

static os_uint32_t mm32_sdma_int_stop(soft_dma_t *dma)
{
    mm32_uart_t *uart = os_container_of(dma, mm32_uart_t, sdma);

    UART_ITConfig(uart->info->huart, MM32_UART_IER_RX, DISABLE);
#ifdef MM32_UART_IER_RXIDLE
    UART_ITConfig(uart->info->huart, MM32_UART_ISR_RXIDLE, DISABLE);
#endif
    
    return uart->rx_index;
}

/* dma rx mode */
static os_uint32_t mm32_sdma_dma_get_index(soft_dma_t *dma)
{
    mm32_uart_t *uart = os_container_of(dma, mm32_uart_t, sdma);

    return uart->sdma_hard_size - DMA_GetCurrDataCounter(uart->info->dma_rx_channel);
}

static os_err_t mm32_sdma_dma_init(soft_dma_t *dma)
{
    return OS_EOK;
}

static os_err_t mm32_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    mm32_uart_t *uart = os_container_of(dma, mm32_uart_t, sdma);
    
    uart->sdma_hard_size = size;
#ifdef MM32_UART_IER_RXIDLE
    UART_ITConfig(uart->info->huart, MM32_UART_ISR_RXIDLE, ENABLE);
#endif
    UART_ITConfig(uart->info->huart, MM32_UART_IER_RX, ENABLE);

    mm32_uart_dma_rx(uart, buff, size);
    
    return OS_EOK;
}

static os_uint32_t mm32_sdma_dma_stop(soft_dma_t *dma)
{
    mm32_uart_t *uart = os_container_of(dma, mm32_uart_t, sdma);

    DMA_Cmd(uart->info->dma_rx_channel, DISABLE);

    return uart->sdma_hard_size - DMA_GetCurrDataCounter(uart->info->dma_rx_channel);
}

/* sdma callback */
static void mm32_uart_sdma_callback(soft_dma_t *dma)
{
    mm32_uart_t *uart = os_container_of(dma, mm32_uart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)uart);
}

static void mm32_uart_sdma_init(struct mm32_uart *uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &uart->sdma;

    soft_dma_stop(dma);

    if (uart->info->dma_rx_channel == OS_NULL)
    {
        dma->hard_info.mode                 = HARD_DMA_MODE_CIRCULAR;
        dma->hard_info.max_size             = 64 * 1024;
#if defined(SERIES_MM32F103XX) || defined(SERIES_MM32L3XX) || defined(SERIES_MM32SPIN2XX)
        dma->hard_info.flag                 = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
#else
        dma->hard_info.flag                 = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ | HARD_DMA_FLAG_TIMEOUT_IRQ;
#endif
        dma->hard_info.data_timeout         = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);
        
        dma->ops.get_index                  = mm32_sdma_int_get_index;
        dma->ops.dma_init                   = OS_NULL;
        dma->ops.dma_start                  = mm32_sdma_int_start;
        dma->ops.dma_stop                   = mm32_sdma_int_stop;
    }
    else
    {
        if (uart->info->dma_info.RX_DMA_Mode == DMA_Mode_Circular)
            dma->hard_info.mode             = HARD_DMA_MODE_CIRCULAR;
        else
            dma->hard_info.mode             = HARD_DMA_MODE_NORMAL;

        dma->hard_info.max_size             = 64 * 1024;
#if defined(SERIES_MM32F103XX) || defined(SERIES_MM32L3XX) || defined(SERIES_MM32SPIN2XX)
        dma->hard_info.flag                 = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
#else
        dma->hard_info.flag                 = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ | HARD_DMA_FLAG_TIMEOUT_IRQ;
#endif
        dma->hard_info.data_timeout         = uart_calc_byte_timeout_us(uart->serial.config.baud_rate);
        
        dma->ops.get_index                  = mm32_sdma_dma_get_index;
        dma->ops.dma_init                   = mm32_sdma_dma_init;
        dma->ops.dma_start                  = mm32_sdma_dma_start;
        dma->ops.dma_stop                   = mm32_sdma_dma_stop;
    }

    dma->cbs.dma_half_callback              = mm32_uart_sdma_callback;
    dma->cbs.dma_full_callback              = mm32_uart_sdma_callback;
    dma->cbs.dma_timeout_callback           = mm32_uart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(dma, OS_TRUE);
}

static os_err_t mm32_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    UART_InitTypeDef            uart_info;
    struct mm32_uart           *uart;
    os_uint32_t                 data_bits;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);
    
    uart_info = uart->info->uart_info;
    uart_info.UART_BaudRate = cfg->baud_rate;

    switch (cfg->stop_bits)
    {
        case STOP_BITS_1:
            uart_info.UART_StopBits = UART_StopBits_1;
            break;
        case STOP_BITS_2:
            uart_info.UART_StopBits = UART_StopBits_2;
            break;
        default:
            return OS_EINVAL;
    }
    switch (cfg->parity)
    {
        case PARITY_NONE:
            uart_info.UART_Parity = UART_Parity_No;
            data_bits                = cfg->data_bits;
            break;
        default:
            return OS_EINVAL;
    }

    switch (data_bits)
    {
        case DATA_BITS_8:
            uart_info.UART_WordLength = UART_WordLength_8b;
            break;    
        default:
            return OS_EINVAL;
    }

    UART_Init(uart->info->huart, &uart_info);
    UART_Cmd(uart->info->huart, ENABLE);
    UART_ITConfig(uart->info->huart, MM32_UART_IER_RX, DISABLE);
#ifdef MM32_UART_IER_RXIDLE
    UART_ITConfig(uart->info->huart, MM32_UART_IER_RXIDLE, DISABLE);
#endif
    NVIC_EnableIRQ((IRQn_Type)uart->info->uart_nvic_info.NVIC_IRQChannel);
    
    mm32_uart_sdma_init(uart, &uart->serial.rx_fifo->ring);
    return OS_EOK;
}

static os_err_t mm32_uart_deinit(struct os_serial_device *serial)
{
    struct mm32_uart *uart;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    /* rx */
    UART_ITConfig(uart->info->huart, MM32_UART_IER_RX, DISABLE);
#ifdef MM32_UART_IER_RXIDLE
    UART_ITConfig(uart->info->huart, MM32_UART_ISR_RXIDLE, DISABLE);
#endif

    if (uart->info->dma_rx_channel != OS_NULL)
    {
        DMA_DeInit(uart->info->dma_rx_channel);
    }

    /* tx */
    UART_ITConfig(uart->info->huart, UART_IT_TXIEN, DISABLE);

    soft_dma_stop(&uart->sdma);
    UART_DeInit(uart->info->huart);
    UART_Cmd(uart->info->huart, DISABLE);
    return OS_EOK;
}

static int mm32_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct mm32_uart *uart;

    OS_ASSERT(serial != OS_NULL);
    uart = os_container_of(serial, struct mm32_uart, serial);
    OS_ASSERT(uart != OS_NULL);
    
    uart->tx_count = 0;
    uart->tx_buff = (os_uint8_t *)buff;
    uart->tx_size = size;

    if (uart->info->dma_tx_channel != 0)
    {
        mm32_uart_dma_tx(uart, buff, size);
    }
    else
    {
        UART_ITConfig(uart->info->huart, UART_IT_TXIEN, ENABLE);
        UART_SendData(uart->info->huart, uart->tx_buff[uart->tx_count]);
    }

    return size;
}

static int mm32_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    struct mm32_uart *uart;	

    int i;
    os_base_t level;

    OS_ASSERT(serial != OS_NULL);

    uart = os_container_of(serial, struct mm32_uart, serial);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();	
        UART_SendData(uart->info->huart, *(buff + i));
        while(!UART_GetFlagStatus(uart->info->huart, UART_CSR_TXC));
        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops mm32_uart_ops = 
{
    .init           = mm32_uart_init,
    .deinit         = mm32_uart_deinit,
    
    .start_send     = mm32_uart_start_send,
    .poll_send      = mm32_uart_poll_send,
};

static void mm32_uart_parse_configs(struct mm32_uart *uart)
{
    struct os_serial_device *serial = &uart->serial;
    
    serial->config.baud_rate = uart->info->uart_info.UART_BaudRate;

    switch (uart->info->uart_info.UART_StopBits)
    {
    case UART_StopBits_1:
        serial->config.stop_bits = STOP_BITS_1;
        break;
    case UART_StopBits_2:
        serial->config.stop_bits = STOP_BITS_2;
        break;
    }
    switch (uart->info->uart_info.UART_Mode)
    {
    case UART_Parity_No:
        serial->config.parity   = PARITY_NONE;
        break;
    case UART_Parity_Odd:
        serial->config.parity   = PARITY_ODD;
        break;
    case UART_Parity_Even:
        serial->config.parity   = PARITY_EVEN;
        break;
    }

    switch (uart->info->uart_info.UART_WordLength)
    {
    case UART_WordLength_8b:
        serial->config.data_bits = DATA_BITS_8;
        break;
    }
}

static int mm32_uart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;

    os_err_t    result  = 0;
    os_base_t   level = 0;
    struct os_serial_device *serial;
    struct mm32_uart *uart = os_calloc(1, sizeof(struct mm32_uart));
    OS_ASSERT(uart);
    
    uart->info = (struct mm32_uart_info *)dev->info;
    
#ifndef OS_USING_CONSOLE
    mm32_uart_hard_init(uart->info);
#endif

    serial = &uart->serial;
    serial->ops    = &mm32_uart_ops;
    serial->config = config;
    mm32_uart_parse_configs(uart);
    
    level = os_irq_lock();
    os_list_add_tail(&mm32_uart_list, &uart->list);
    os_irq_unlock(level);

    result = os_hw_serial_register(serial, dev->name, NULL);

    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO mm32_uart_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = mm32_uart_probe,
};

OS_DRIVER_DEFINE(mm32_uart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

#ifdef OS_USING_CONSOLE
static UART_TypeDef * console_uart = 0;
void __os_hw_console_output(char *str)
{
    if (console_uart == 0)
        return;

    while (*str)
    {
        UART_SendData(console_uart, *str);
        while(!UART_GetFlagStatus(console_uart, UART_CSR_TXC));
        str++;
    }
}

static int mm32_uart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    mm32_uart_info_t *info = (mm32_uart_info_t *)dev->info;
    mm32_uart_hard_init(info);
    if(!strcmp(dev->name, OS_CONSOLE_DEVICE_NAME))
    {
        console_uart = (UART_TypeDef *)info->huart;
    }
    return OS_EOK;
}

OS_DRIVER_INFO mm32_uart_early_driver = {
    .name   = "UART_HandleTypeDef",
    .probe  = mm32_uart_early_probe,
};

OS_DRIVER_DEFINE(mm32_uart_early_driver, CORE, OS_INIT_SUBLEVEL_LOW);
#endif

