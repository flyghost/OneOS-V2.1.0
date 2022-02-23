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
 * @brief       This file implements imxrt_uart driver for imxrt
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <dma/dma.h>
#include "fsl_lpuart.h"
#include "fsl_lpuart_edma.h"
#include "fsl_dmamux.h"

#include "drv_uart.h"

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

typedef struct os_imxrt_uart
{
    struct os_serial_device         serial;

    struct nxp_lpuart_info         *uart_info;
    IRQn_Type                       irqn;
    lpuart_handle_t                *handle;
    lpuart_edma_handle_t           *edma_handle;
    DMA_Type                       *rxdma_base;
    os_uint32_t                     rxdma_channel;
    edma_tcd_t                     *tcdMemoryPoolPtr;
    os_uint32_t                     clk_src;

    lpuart_transfer_t               receiveXfer;
    os_uint32_t                     rx_index;
    
    lpuart_transfer_t               sendXfer;
    os_uint32_t                     tx_count;
    
    soft_dma_t  sdma;
    os_uint32_t sdma_hard_size;

    os_list_node_t list;
}imxrt_uart_t;

static os_list_node_t imxrt_uart_list = OS_LIST_INIT(imxrt_uart_list);

void imxrt_uart_irq_callback(struct os_imxrt_uart *imxrt_uart)
{
    os_uint32_t tx_count            = 0;
    os_uint32_t status              = LPUART_GetStatusFlags(imxrt_uart->uart_info->uart_base);
    os_uint32_t enabledInterrupts   = LPUART_GetEnabledInterrupts(imxrt_uart->uart_info->uart_base);

    if ((0U != ((uint32_t)kLPUART_IdleLineFlag & status)) &&
        (0U != ((uint32_t)kLPUART_IdleLineInterruptEnable & enabledInterrupts)))
    {
        (void)LPUART_ClearStatusFlags(imxrt_uart->uart_info->uart_base, kLPUART_IdleLineFlag);
    }

    if ((0U != ((uint32_t)kLPUART_TransmissionCompleteFlag & status)) &&
        (0U != ((uint32_t)kLPUART_TransmissionCompleteInterruptEnable & enabledInterrupts)))
    {
        (void)LPUART_ClearStatusFlags(imxrt_uart->uart_info->uart_base, kLPUART_TransmissionCompleteInterruptEnable);
        LPUART_TransferEdmaHandleIRQ(imxrt_uart->uart_info->uart_base, imxrt_uart->edma_handle);
        imxrt_uart->sendXfer.dataSize = 0;
    }

    
    SDK_ISR_EXIT_BARRIER;
}

LPUART_IRQHandler_DEFINE(1);
LPUART_IRQHandler_DEFINE(2);
LPUART_IRQHandler_DEFINE(3);
LPUART_IRQHandler_DEFINE(4);
LPUART_IRQHandler_DEFINE(5);
LPUART_IRQHandler_DEFINE(6);
LPUART_IRQHandler_DEFINE(7);
LPUART_IRQHandler_DEFINE(8);

void imxrt_uart_dma_rx_callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    imxrt_uart_t *imxrt_uart = (imxrt_uart_t *)param;
    
    if (true == transferDone)
    {
        soft_dma_full_irq(&imxrt_uart->sdma);
    }
    else
    {
        soft_dma_half_irq(&imxrt_uart->sdma);
    }
}

void imxrt_uart_dma_tx_callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    imxrt_uart_t *imxrt_uart = (imxrt_uart_t *)param;
    
    if (true == transferDone)
    {
        os_hw_serial_isr_txdone((struct os_serial_device *)imxrt_uart);
    }
}

void imxrt_uart_dma_callback(LPUART_Type *base, lpuart_edma_handle_t *handle, status_t status, void *userData)
{
    imxrt_uart_t *imxrt_uart = (imxrt_uart_t *)userData;
    
    if (kStatus_LPUART_TxIdle == status)
    {
        os_hw_serial_isr_txdone((struct os_serial_device *)imxrt_uart);
    }
}

void imxrt_uart_transfer_callback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    imxrt_uart_t *imxrt_uart = (imxrt_uart_t *)userData;
    
    if (kStatus_LPUART_TxIdle == status)
    {
        os_hw_serial_isr_txdone((struct os_serial_device *)imxrt_uart);
    }
    if (kStatus_LPUART_RxIdle == status)
    {
        soft_dma_full_irq(&imxrt_uart->sdma);
    }
}

/* interrupt rx mode */
static os_uint32_t imxrt_sdma_int_get_index(soft_dma_t *dma)
{
    status_t status = kStatus_Success;
    
    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);

    if (imxrt_uart->handle != OS_NULL)
    {
        status = LPUART_TransferGetReceiveCount(imxrt_uart->uart_info->uart_base, imxrt_uart->handle, (uint32_t *)&imxrt_uart->rx_index);
    }
    
    return (status == kStatus_Success) ? imxrt_uart->rx_index : 0;
}

static os_err_t imxrt_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    status_t status = kStatus_Success;

    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);

    imxrt_uart->receiveXfer.data  = buff;
    imxrt_uart->receiveXfer.dataSize  = size;
    imxrt_uart->rx_index = 0;

    if (imxrt_uart->handle != OS_NULL)
    {
        status = LPUART_TransferReceiveNonBlocking(imxrt_uart->uart_info->uart_base, imxrt_uart->handle, &imxrt_uart->receiveXfer, OS_NULL);
    }
    
    return (status == kStatus_Success) ? OS_EOK : OS_ERROR;
}

static os_uint32_t imxrt_sdma_int_stop(soft_dma_t *dma)
{
    status_t status = kStatus_Success;

    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);
    
    if (imxrt_uart->handle != OS_NULL)
    {
        status = LPUART_TransferGetReceiveCount(imxrt_uart->uart_info->uart_base, imxrt_uart->handle, (uint32_t *)&imxrt_uart->rx_index);
        LPUART_TransferAbortReceive(imxrt_uart->uart_info->uart_base, imxrt_uart->handle);
    }
    
    return (status == kStatus_Success) ? imxrt_uart->rx_index : 0;

}

/* dma rx mode */
static os_uint32_t imxrt_sdma_dma_get_index(soft_dma_t *dma)
{
    status_t status = kStatus_Success;

    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);

    if (imxrt_uart->edma_handle->rxEdmaHandle != OS_NULL && imxrt_uart->edma_handle != OS_NULL)
    {
        imxrt_uart->rx_index = imxrt_uart->receiveXfer.dataSize - EDMA_GetRemainingMajorLoopCount(imxrt_uart->rxdma_base, imxrt_uart->rxdma_channel);
        if (imxrt_uart->rx_index == imxrt_uart->receiveXfer.dataSize)
        {
            imxrt_uart->rx_index = 0;
        }
    }
    
    return imxrt_uart->rx_index;
}

static os_err_t imxrt_sdma_dma_init(soft_dma_t *dma)
{
    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);

    if (imxrt_uart->edma_handle->rxEdmaHandle != OS_NULL && imxrt_uart->edma_handle != OS_NULL)
    {
    }

    return OS_EOK;
}

static os_err_t imxrt_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    status_t status = kStatus_Success;
    edma_transfer_config_t xferConfig;
    
    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);
    
    imxrt_uart->sdma_hard_size = size;
    imxrt_uart->receiveXfer.data = buff;
    imxrt_uart->receiveXfer.dataSize = size;
    imxrt_uart->rx_index = 0;
    
    if (imxrt_uart->edma_handle->rxEdmaHandle != OS_NULL && imxrt_uart->edma_handle != OS_NULL)
    {
        EDMA_InstallTCDMemory(imxrt_uart->edma_handle->rxEdmaHandle, imxrt_uart->tcdMemoryPoolPtr, 1U);
        
        EDMA_PrepareTransfer(&xferConfig, (void *)(uint32_t *)LPUART_GetDataRegisterAddress(imxrt_uart->uart_info->uart_base),
                         sizeof(uint8_t), buff, sizeof(uint8_t), sizeof(uint8_t), size,
                         kEDMA_PeripheralToMemory);
        
        imxrt_uart->edma_handle->rxEdmaHandle->tcdUsed = 1U;
        imxrt_uart->edma_handle->rxEdmaHandle->tail    = 0U;
        
        EDMA_TcdReset(&imxrt_uart->edma_handle->rxEdmaHandle->tcdPool[0U]);
        EDMA_TcdSetTransferConfig(&imxrt_uart->edma_handle->rxEdmaHandle->tcdPool[0U], &xferConfig, imxrt_uart->tcdMemoryPoolPtr);
        
        imxrt_uart->edma_handle->rxEdmaHandle->tcdPool[0U].CSR |= DMA_CSR_INTMAJOR_MASK | DMA_CSR_INTHALF_MASK;

        EDMA_InstallTCD(imxrt_uart->edma_handle->rxEdmaHandle->base, imxrt_uart->edma_handle->rxEdmaHandle->channel, &imxrt_uart->edma_handle->rxEdmaHandle->tcdPool[0U]);

        EDMA_SetCallback(imxrt_uart->edma_handle->rxEdmaHandle, imxrt_uart_dma_rx_callback, imxrt_uart);

        EDMA_StartTransfer(imxrt_uart->edma_handle->rxEdmaHandle);
        
        LPUART_EnableRxDMA(imxrt_uart->uart_info->uart_base, true);
        
        LPUART_EnableInterrupts(imxrt_uart->uart_info->uart_base, kLPUART_IdleLineInterruptEnable);
        EnableIRQ(imxrt_uart->irqn);
    }
    
    return (status == kStatus_Success) ? OS_EOK : OS_ERROR;
}

static os_uint32_t imxrt_sdma_dma_stop(soft_dma_t *dma)
{
    status_t status = kStatus_Success;

    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);

    if (imxrt_uart->edma_handle->rxEdmaHandle != OS_NULL && imxrt_uart->edma_handle != OS_NULL)
    {
        imxrt_uart->rx_index = imxrt_uart->receiveXfer.dataSize - EDMA_GetRemainingMajorLoopCount(imxrt_uart->rxdma_base, imxrt_uart->rxdma_channel);
        if (imxrt_uart->rx_index == imxrt_uart->receiveXfer.dataSize)
        {
            imxrt_uart->rx_index = 0;
        }
        
        EDMA_StopTransfer(imxrt_uart->edma_handle->rxEdmaHandle);
        
        LPUART_EnableRxDMA(imxrt_uart->uart_info->uart_base, false);
    }
    
    return imxrt_uart->rx_index;
}

/* sdma callback */
static void imxrt_uart_sdma_callback(soft_dma_t *dma)
{
    imxrt_uart_t *imxrt_uart = os_container_of(dma, imxrt_uart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)imxrt_uart);
}

static void imxrt_usart_sdma_init(struct os_imxrt_uart *imxrt_uart, dma_ring_t *ring)
{
    soft_dma_t *dma = &imxrt_uart->sdma;

    soft_dma_stop(dma);

    memset(&dma->hard_info, 0, sizeof(dma->hard_info));

    dma->hard_info.mode         = HARD_DMA_MODE_NORMAL;

    dma->hard_info.max_size     = 64 * 1024;
    dma->hard_info.data_timeout = uart_calc_byte_timeout_us(imxrt_uart->serial.config.baud_rate);

    if (imxrt_uart->handle != OS_NULL)
    {
        dma->hard_info.mode                 = HARD_DMA_MODE_NORMAL;
        dma->hard_info.flag                 = HARD_DMA_FLAG_FULL_IRQ;
        dma->hard_info.max_size             = 64 * 1024;
        dma->hard_info.data_timeout         = uart_calc_byte_timeout_us(imxrt_uart->serial.config.baud_rate);
        
        dma->ops.get_index                  = imxrt_sdma_int_get_index;
        dma->ops.dma_init                   = OS_NULL;
        dma->ops.dma_start                  = imxrt_sdma_int_start;
        dma->ops.dma_stop                   = imxrt_sdma_int_stop;
    }
    else if (imxrt_uart->edma_handle->rxEdmaHandle != OS_NULL && imxrt_uart->edma_handle != OS_NULL)
    {
        dma->hard_info.mode                 = HARD_DMA_MODE_CIRCULAR;
        dma->hard_info.flag                 = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
    
        dma->hard_info.max_size             = 64 * 1024;
        dma->hard_info.data_timeout         = uart_calc_byte_timeout_us(imxrt_uart->serial.config.baud_rate);
        
        dma->ops.get_index                  = imxrt_sdma_dma_get_index;
        dma->ops.dma_init                   = imxrt_sdma_dma_init;
        dma->ops.dma_start                  = imxrt_sdma_dma_start;
        dma->ops.dma_stop                   = imxrt_sdma_dma_stop;
    }
    else
    {
        LOG_D(DRV_EXT_TAG, "usart only support DMA and transfer mode!");
    }

    dma->cbs.dma_half_callback      = imxrt_uart_sdma_callback;
    dma->cbs.dma_full_callback      = imxrt_uart_sdma_callback;
    dma->cbs.dma_timeout_callback   = imxrt_uart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(&imxrt_uart->sdma, OS_TRUE);
}

static os_err_t imxrt_uart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    os_uint32_t rx_bufsz = 0;
    
    struct os_imxrt_uart *imxrt_uart;
    lpuart_config_t config;

    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    imxrt_uart = os_container_of(serial, struct os_imxrt_uart, serial);
    
    imxrt_uart->tcdMemoryPoolPtr = (edma_tcd_t *)os_dma_malloc_align(sizeof(edma_tcd_t), 32);
    
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = cfg->baud_rate;

    switch (cfg->data_bits)
    {
    case DATA_BITS_7:
        config.dataBitsCount = kLPUART_SevenDataBits;
        break;

    default:
        config.dataBitsCount = kLPUART_EightDataBits;
        break;
    }

    switch (cfg->stop_bits)
    {
    case STOP_BITS_2:
        config.stopBitCount = kLPUART_TwoStopBit;
        break;
    default:
        config.stopBitCount = kLPUART_OneStopBit;
        break;
    }

    switch (cfg->parity)
    {
    case PARITY_ODD:
        config.parityMode = kLPUART_ParityOdd;
        break;
    case PARITY_EVEN:
        config.parityMode = kLPUART_ParityEven;
        break;
    default:
        config.parityMode = kLPUART_ParityDisabled;
        break;
    }

    config.enableTx = true;
    config.enableRx = true;

    LPUART_Init(imxrt_uart->uart_info->uart_base, &config, imxrt_uart->clk_src);
    
    imxrt_usart_sdma_init(imxrt_uart, &serial->rx_fifo->ring);

    return OS_EOK;
}

static os_err_t imxrt_uart_deinit(struct os_serial_device *serial)
{
    struct os_imxrt_uart *imxrt_uart;

    OS_ASSERT(serial != OS_NULL);

    imxrt_uart = os_container_of(serial, struct os_imxrt_uart, serial);
    
    soft_dma_stop(&imxrt_uart->sdma);

    /* tx */
    if (imxrt_uart->edma_handle->txEdmaHandle != OS_NULL && imxrt_uart->edma_handle != OS_NULL)
    {
        LPUART_TransferAbortSendEDMA(imxrt_uart->uart_info->uart_base, imxrt_uart->edma_handle);
    }
    else if (imxrt_uart->handle != OS_NULL)
    {
        LPUART_TransferAbortSend(imxrt_uart->uart_info->uart_base, imxrt_uart->handle);
    }

    return OS_EOK;
}

static int imxrt_uart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    status_t status = kStatus_Success;

    struct os_imxrt_uart *imxrt_uart;

    OS_ASSERT(serial != OS_NULL);

    imxrt_uart = os_container_of(serial, struct os_imxrt_uart, serial);

    imxrt_uart->sendXfer.data = (os_uint8_t *)buff;
    imxrt_uart->sendXfer.dataSize = size;

    if (imxrt_uart->edma_handle->txEdmaHandle != OS_NULL && imxrt_uart->edma_handle != OS_NULL)
    {
        status = LPUART_SendEDMA(imxrt_uart->uart_info->uart_base, imxrt_uart->edma_handle, &imxrt_uart->sendXfer);
    }
    else if (imxrt_uart->handle != OS_NULL)
    {
        status = LPUART_TransferSendNonBlocking(imxrt_uart->uart_info->uart_base, imxrt_uart->handle, &imxrt_uart->sendXfer);
    }

    return (status == kStatus_Success) ? size : 0;
}

static int imxrt_uart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int i;
    os_base_t level;
    struct os_imxrt_uart *imxrt_uart;
    
    OS_ASSERT(serial != OS_NULL);

    imxrt_uart = os_container_of(serial, struct os_imxrt_uart, serial);

    for (i = 0; i < size; i++)
    {
        level = os_irq_lock();
        
        LPUART_WriteByte(imxrt_uart->uart_info->uart_base, buff[i]);
        while (!(LPUART_GetStatusFlags(imxrt_uart->uart_info->uart_base) & kLPUART_TxDataRegEmptyFlag));

        os_irq_unlock(level);
    }

    return size;
}

static const struct os_uart_ops imxrt_uart_ops = {
    .init       = imxrt_uart_init,
    .deinit     = imxrt_uart_deinit,

    .start_send = imxrt_uart_start_send,
    .poll_send  = imxrt_uart_poll_send,
};

os_err_t imxrt_uart_param_cfg(struct os_imxrt_uart *imxrt_uart)
{
    os_err_t err = 0;
    
    switch((os_uint32_t)imxrt_uart->uart_info->uart_base)
    {
    case (os_uint32_t)LPUART1:
        UART1_CFG_INIT(imxrt_uart, 1);
    break;
    case (os_uint32_t)LPUART2:
        UART2_CFG_INIT(imxrt_uart, 2);
    break;
    case (os_uint32_t)LPUART3:
        UART3_CFG_INIT(imxrt_uart, 3);
    break;
    case (os_uint32_t)LPUART4:
        UART4_CFG_INIT(imxrt_uart, 4);
    break;
    case (os_uint32_t)LPUART5:
        UART5_CFG_INIT(imxrt_uart, 5);
    break;
    case (os_uint32_t)LPUART6:
        UART6_CFG_INIT(imxrt_uart, 6);
    break;
    case (os_uint32_t)LPUART7:
        UART7_CFG_INIT(imxrt_uart, 7);
    break;
    case (os_uint32_t)LPUART8:
        UART8_CFG_INIT(imxrt_uart, 8);
    break;
    default:
    break;
    }
    
    if (imxrt_uart->edma_handle != OS_NULL)
    {
        imxrt_uart->edma_handle->callback  = imxrt_uart_dma_callback;
        imxrt_uart->edma_handle->userData  = imxrt_uart;
    }

    if (imxrt_uart->handle != OS_NULL)
    {
        imxrt_uart->handle->callback       = imxrt_uart_transfer_callback;
        imxrt_uart->handle->userData       = imxrt_uart;
    }
    
    if (imxrt_uart->clk_src == 0)
        return OS_ERROR;
    return OS_EOK;
}

static int imxrt_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    os_err_t    result  = 0;
    os_base_t   level;
    
    struct os_imxrt_uart *imxrt_uart = os_calloc(1, sizeof(struct os_imxrt_uart));

    OS_ASSERT(imxrt_uart);

    imxrt_uart->uart_info = (struct nxp_lpuart_info *)dev->info;

    struct os_serial_device *serial = &imxrt_uart->serial;

    serial->ops    = &imxrt_uart_ops;
    serial->config = config;
    serial->config.baud_rate = imxrt_uart->uart_info->config->baudRate_Bps;

    level = os_irq_lock();
    os_list_add_tail(&imxrt_uart_list, &imxrt_uart->list);
    os_irq_unlock(level);

    imxrt_uart_param_cfg(imxrt_uart);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO imxrt_uart_driver = {
    .name   = "LPUART_Type",
    .probe  = imxrt_usart_probe,
};

OS_DRIVER_DEFINE(imxrt_uart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

#ifdef OS_USING_CONSOLE
static LPUART_Type *console_uart = OS_NULL;
void __os_hw_console_output(char *str)
{
    if (console_uart == OS_NULL)
        return;
   while (*str)
   {        
       LPUART_WriteByte(console_uart,*str);
       while (!(LPUART_GetStatusFlags(console_uart) & kLPUART_TxDataRegEmptyFlag))
           ;
        
       str++;
   }
}

static int imxrt_usart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if(!strcmp(dev->name,OS_CONSOLE_DEVICE_NAME))
    {
        struct nxp_lpuart_info *info = (struct nxp_lpuart_info *)dev->info;
        console_uart = (LPUART_Type *)info->uart_base;
    }
    return OS_EOK;
}

OS_DRIVER_INFO imxrt_usart_early_driver = {
    .name   = "LPUART_Type",
    .probe  = imxrt_usart_early_probe,
};

OS_DRIVER_DEFINE(imxrt_usart_early_driver, CORE, OS_INIT_SUBLEVEL_LOW);
#endif



