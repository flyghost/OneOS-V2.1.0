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
 * @brief       This file implements usart driver for lpc
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-08   OneOS Team      1.2
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_memory.h>
#include <bus/bus.h>
#include <drv_cfg.h>
#include <drv_usart.h>
#include <dma/dma.h>
#include <os_clock.h>
#include <string.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.usart"
#include <drv_log.h>

#define CC_LPC_DMA_DESCRIPTOR(INDEX)    __attribute__((aligned(16))) dma_descriptor_t lpc_usart##INDEX##_dma_pingpong_desc[2] = {0};

#if defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__GNUC__)
#ifdef USART0_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(0)
#endif
#ifdef USART1_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(1)
#endif
#ifdef USART2_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(2)
#endif
#ifdef USART3_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(3)
#endif
#ifdef USART4_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(4)
#endif
#ifdef USART5_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(5)
#endif
#ifdef USART6_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(6)
#endif
#ifdef USART7_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(7)
#endif
#ifdef USART8_RX_DMA_CHANNEL
CC_LPC_DMA_DESCRIPTOR(8)
#endif
#else
#error "dma_descriptor_t not find!"
#endif

typedef struct usart
{
    struct os_serial_device         serial;
    lpc_usart_info_t               *usart_info;
    
    IRQn_Type                       irqn;
    usart_handle_t                 *usart_handle;
    usart_dma_handle_t             *usart_DmaHandle;
    dma_transfer_config_t           transferConfig;
    dma_descriptor_t               *dma_descriptor[2];
    volatile enum lpc_dma_status    dma_status;
    
    soft_dma_t                      sdma;
    os_uint32_t                     sdma_hard_size;
    
    usart_transfer_t                receiveXfer;
    os_uint32_t                     rx_index;
    
    usart_transfer_t                sendXfer;
    os_uint32_t                     tx_count;
    
    os_uint32_t                     clk_src;

    os_list_node_t list;
}lpc_usart_t;

static os_list_node_t lpc_usart_list = OS_LIST_INIT(lpc_usart_list);

void lpc_usart_dmapingpang_rx_callback(dma_handle_t *handle, void *userData, bool transferDone, uint32_t intmode)
{
    lpc_usart_t *usart = (lpc_usart_t *)userData;
    
    if (intmode == kDMA_IntA)
    {
        usart->dma_status = DMA_BUFF_INB;
        soft_dma_half_irq(&usart->sdma);
    }
    if (intmode == kDMA_IntB)
    {
        usart->dma_status = DMA_BUFF_INA;
        soft_dma_full_irq(&usart->sdma);
    }
}

void lpc_usart_dma_callback(USART_Type *base, usart_dma_handle_t *handle, status_t status, void *userData)
{
    lpc_usart_t *usart = (lpc_usart_t *)userData;
    
    if (kStatus_USART_TxIdle == status)
    {
        os_hw_serial_isr_txdone((struct os_serial_device *)usart);
    }
}

void lpc_usart_transfer_callback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    lpc_usart_t *usart = (lpc_usart_t *)userData;
    
    if (kStatus_USART_TxIdle == status)
    {
        os_hw_serial_isr_txdone((struct os_serial_device *)usart);
    }
}

/* interrupt rx mode */
static os_uint32_t mm32_sdma_int_get_index(soft_dma_t *dma)
{
    status_t status = kStatus_Success;
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);
    
    if (usart->usart_handle != OS_NULL)
    {
        status = USART_TransferGetReceiveCount(usart->usart_info->usart_base, usart->usart_handle, (uint32_t *)&usart->rx_index);
    }
    
    return (status == kStatus_Success) ? usart->rx_index : 0;
}

static os_err_t mm32_sdma_int_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    status_t status = kStatus_Success;
    
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);

    usart->receiveXfer.data  = buff;
    usart->receiveXfer.dataSize  = size;
    usart->rx_index = 0;

    if (usart->usart_handle != OS_NULL)
    {
        status = USART_TransferReceiveNonBlocking(usart->usart_info->usart_base, usart->usart_handle, &usart->receiveXfer, OS_NULL);
    }
    
    return (status == kStatus_Success) ? OS_EOK : OS_ERROR;
}

static os_uint32_t mm32_sdma_int_stop(soft_dma_t *dma)
{
    status_t status = kStatus_Success;
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);    
    
    if (usart->usart_handle != OS_NULL)
    {
        status = USART_TransferGetReceiveCount(usart->usart_info->usart_base, usart->usart_handle, (uint32_t *)&usart->rx_index);
        USART_TransferAbortReceive(usart->usart_info->usart_base, usart->usart_handle);
    }
    
    return (status == kStatus_Success) ? usart->rx_index : 0;
}

/* dma rx mode */
static os_uint32_t mm32_sdma_dma_get_index(soft_dma_t *dma)
{
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);

    if (usart->usart_DmaHandle->rxDmaHandle != OS_NULL && usart->usart_DmaHandle != OS_NULL)
    {
        if (usart->dma_status == DMA_BUFF_INA)
            usart->rx_index = usart->transferConfig.xfercfg.transferCount - DMA_GetRemainingBytes(usart->usart_DmaHandle->rxDmaHandle->base, usart->usart_DmaHandle->rxDmaHandle->channel);
        else
            usart->rx_index = usart->transferConfig.xfercfg.transferCount + usart->receiveXfer.dataSize / 2 - DMA_GetRemainingBytes(usart->usart_DmaHandle->rxDmaHandle->base, usart->usart_DmaHandle->rxDmaHandle->channel);
    }
    return usart->rx_index;
}

static os_err_t mm32_sdma_dma_init(soft_dma_t *dma)
{
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);
    
    if (usart->usart_DmaHandle->rxDmaHandle != OS_NULL && usart->usart_DmaHandle != OS_NULL)
    {
        USART_EnableRxDMA(usart->usart_info->usart_base, true);
        DMA_SetCallback(usart->usart_DmaHandle->rxDmaHandle, lpc_usart_dmapingpang_rx_callback, usart);
    }
    return OS_EOK;
}

static os_err_t mm32_sdma_dma_start(soft_dma_t *dma, void *buff, os_uint32_t size)
{
    status_t status = kStatus_Success;
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);
    
    usart->sdma_hard_size = size;
    usart->receiveXfer.data = buff;
    usart->receiveXfer.dataSize = size;
    usart->rx_index = 0;
    
    if (usart->usart_DmaHandle->rxDmaHandle != OS_NULL && usart->usart_DmaHandle != OS_NULL)
    {
        if (size < 128 || size % 2 != 0)
        {
            LOG_D(DRV_EXT_TAG, "usart need buff size need 128 bytes or more!");
            return OS_ERROR;
        }
        
        DMA_PrepareTransfer(&usart->transferConfig, (void *)&usart->usart_info->usart_base->FIFORD, buff, 1, size / 2, kDMA_PeripheralToMemory,
                            usart->dma_descriptor[1]);
        DMA_SubmitTransfer(usart->usart_DmaHandle->rxDmaHandle, &usart->transferConfig);
        usart->transferConfig.xfercfg.intA = false;
        usart->transferConfig.xfercfg.intB = true;
        DMA_CreateDescriptor(usart->dma_descriptor[1], &usart->transferConfig.xfercfg, (void *)&usart->usart_info->usart_base->FIFORD, buff + size / 2,
                             usart->dma_descriptor[0]);
        usart->transferConfig.xfercfg.intA = true;
        usart->transferConfig.xfercfg.intB = false;
        DMA_CreateDescriptor(usart->dma_descriptor[0], &usart->transferConfig.xfercfg, (void *)&usart->usart_info->usart_base->FIFORD, buff,
                             usart->dma_descriptor[1]);
        DMA_StartTransfer(usart->usart_DmaHandle->rxDmaHandle);
        usart->dma_status = DMA_BUFF_INA;
    }
    
    return (status == kStatus_Success) ? OS_EOK : OS_ERROR;
}

static os_uint32_t mm32_sdma_dma_stop(soft_dma_t *dma)
{
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);
    
    if (usart->usart_DmaHandle->rxDmaHandle != OS_NULL && usart->usart_DmaHandle != OS_NULL)
    {
        if (usart->dma_status == DMA_BUFF_INA)
            usart->rx_index = usart->transferConfig.xfercfg.transferCount - DMA_GetRemainingBytes(usart->usart_DmaHandle->rxDmaHandle->base, usart->usart_DmaHandle->rxDmaHandle->channel);
        else
            usart->rx_index = usart->transferConfig.xfercfg.transferCount + usart->receiveXfer.dataSize / 2 - DMA_GetRemainingBytes(usart->usart_DmaHandle->rxDmaHandle->base, usart->usart_DmaHandle->rxDmaHandle->channel);

        usart->dma_status = DMA_BUFF_NONE;
        DMA_AbortTransfer(usart->usart_DmaHandle->rxDmaHandle);
    }
    
    return usart->rx_index;
}

/* sdma callback */
static void lpc_usart_sdma_callback(soft_dma_t *dma)
{
    lpc_usart_t *usart = os_container_of(dma, lpc_usart_t, sdma);

    os_hw_serial_isr_rxdone((struct os_serial_device *)usart);
}

static void lpc_usart_sdma_init(struct usart *usart, dma_ring_t *ring)
{
    soft_dma_t *dma = &usart->sdma;

    soft_dma_stop(dma);

    if (usart->usart_handle != OS_NULL)
    {
        dma->hard_info.mode                 = HARD_DMA_MODE_NORMAL;
        dma->hard_info.flag                 = HARD_DMA_FLAG_FULL_IRQ;
        dma->hard_info.max_size             = 64 * 1024;
        dma->hard_info.data_timeout         = uart_calc_byte_timeout_us(usart->serial.config.baud_rate);
        
        dma->ops.get_index                  = mm32_sdma_int_get_index;
        dma->ops.dma_init                   = OS_NULL;
        dma->ops.dma_start                  = mm32_sdma_int_start;
        dma->ops.dma_stop                   = mm32_sdma_int_stop;
    }
    else if (usart->usart_DmaHandle->rxDmaHandle != OS_NULL && usart->usart_DmaHandle != OS_NULL)
    {
        dma->hard_info.mode                 = HARD_DMA_MODE_CIRCULAR;
        dma->hard_info.flag                 = HARD_DMA_FLAG_HALF_IRQ | HARD_DMA_FLAG_FULL_IRQ;
    
        dma->hard_info.max_size             = 64 * 1024;
        dma->hard_info.data_timeout         = uart_calc_byte_timeout_us(usart->serial.config.baud_rate);
        
        dma->ops.get_index                  = mm32_sdma_dma_get_index;
        dma->ops.dma_init                   = mm32_sdma_dma_init;
        dma->ops.dma_start                  = mm32_sdma_dma_start;
        dma->ops.dma_stop                   = mm32_sdma_dma_stop;
    }
    else
    {
        LOG_D(DRV_EXT_TAG, "usart only support DMA and transfer mode!");
    }
    dma->cbs.dma_half_callback              = lpc_usart_sdma_callback;
    dma->cbs.dma_full_callback              = lpc_usart_sdma_callback;
    dma->cbs.dma_timeout_callback           = lpc_usart_sdma_callback;

    soft_dma_init(dma);
    soft_dma_start(dma, ring);
    soft_dma_irq_enable(dma, OS_TRUE);
}

static os_err_t lpc_usart_init(struct os_serial_device *serial, struct serial_configure *cfg)
{
    lpc_usart_t *usart = (lpc_usart_t *)serial;
    
    OS_ASSERT(serial != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);
    
    LOG_D(DRV_EXT_TAG, "sofeware just use to config baud_rate, other param should use MCUXpresso config tool!");
        
    if (USART_SetBaudRate(usart->usart_info->usart_base, cfg->baud_rate, usart->clk_src) != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "%s set baudrate %d failed! this baudrate not support!", serial->parent.name, cfg->baud_rate);
        return OS_ERROR;
    }

    lpc_usart_sdma_init(usart, &usart->serial.rx_fifo->ring);
    return OS_EOK;
}

static os_err_t lpc_usart_deinit(struct os_serial_device *serial)
{
    lpc_usart_t *usart = (lpc_usart_t *)serial;

    /* rx */
    if (usart->usart_DmaHandle->rxDmaHandle != OS_NULL && usart->usart_DmaHandle != OS_NULL)
        USART_EnableRxDMA(usart->usart_info->usart_base, false);
    else if (usart->usart_handle != OS_NULL)
        USART_TransferAbortReceive(usart->usart_info->usart_base, usart->usart_handle);

    soft_dma_stop(&usart->sdma);

    /* tx */
    if (usart->usart_DmaHandle != OS_NULL)
        USART_TransferAbortSendDMA(usart->usart_info->usart_base, usart->usart_DmaHandle);
    else if (usart->usart_handle != OS_NULL)
        USART_TransferAbortSend(usart->usart_info->usart_base, usart->usart_handle);
    
    return OS_EOK;
}

static int lpc_usart_start_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    status_t status = kStatus_Success;
    
    lpc_usart_t *lpc_usart;
    
    OS_ASSERT(serial != OS_NULL);
    
    lpc_usart = (lpc_usart_t *)serial;

    lpc_usart->sendXfer.data = (os_uint8_t *)buff;
    lpc_usart->sendXfer.dataSize = size;

    if (lpc_usart->usart_DmaHandle->txDmaHandle != OS_NULL && lpc_usart->usart_DmaHandle != OS_NULL)
    {
        status = USART_TransferSendDMA(lpc_usart->usart_info->usart_base, lpc_usart->usart_DmaHandle, &lpc_usart->sendXfer);
    }
    else if (lpc_usart->usart_handle != OS_NULL)
    {
        status = USART_TransferSendNonBlocking(lpc_usart->usart_info->usart_base, lpc_usart->usart_handle, &lpc_usart->sendXfer);
    }
    else
    {
        USART_WriteBlocking(lpc_usart->usart_info->usart_base, lpc_usart->sendXfer.data, lpc_usart->sendXfer.dataSize);
    }

    return (status == kStatus_Success) ? size : 0;
}

static int lpc_usart_poll_send(struct os_serial_device *serial, const os_uint8_t *buff, os_size_t size)
{
    int32_t  ret;
    status_t status = kStatus_Success;
    
    lpc_usart_t *usart;
    
    os_base_t level;
    
    OS_ASSERT(serial != OS_NULL);

    usart = os_container_of(serial, lpc_usart_t, serial);

    level = os_irq_lock();
    status = USART_WriteBlocking(usart->usart_info->usart_base, (uint8_t *)buff, size);
    os_irq_unlock(level);
    
    return (status == kStatus_Success) ? size : 0;
}

static struct os_uart_ops usart_ops = {
    .init           = lpc_usart_init,
    .deinit         = lpc_usart_deinit,
    
    .start_send     = lpc_usart_start_send,
    .poll_send      = lpc_usart_poll_send,
};

void lpc_usart_parse_configs_from_configtool(lpc_usart_t *usart)
{
    struct os_serial_device *serial = &usart->serial;
    
    serial->config.baud_rate = usart->usart_info->usart_config->baudRate_Bps;
    switch (usart->usart_info->usart_config->stopBitCount)
    {
    case kUSART_OneStopBit:
        serial->config.stop_bits = STOP_BITS_1;
        break;
    case kUSART_TwoStopBit:
        serial->config.stop_bits = STOP_BITS_2;
        break;
    }
    switch (usart->usart_info->usart_config->parityMode)
    {
    case kUSART_ParityDisabled:
        serial->config.parity   = PARITY_NONE;
        break;
    case kUSART_ParityOdd:
        serial->config.parity   = PARITY_ODD;
        break;
    case kUSART_ParityEven:
        serial->config.parity   = PARITY_EVEN;
        break;
    }

    switch (usart->usart_info->usart_config->bitCountPerChar)
    {
    case kUSART_7BitsPerChar:
        serial->config.data_bits = DATA_BITS_7;
        break;
    case kUSART_8BitsPerChar:
        serial->config.data_bits = DATA_BITS_8;
        break;
    }
}

os_err_t lpc_usart_param_cfg(lpc_usart_t *usart)
{
    os_err_t err = 0;
    switch((os_uint32_t)usart->usart_info->usart_base)
    {
    case (os_uint32_t)FLEXCOMM0:
        USART0_CFG_INIT(usart, 0);
        break;
    case (os_uint32_t)FLEXCOMM1:
        USART1_CFG_INIT(usart, 1);
        break;
    case (os_uint32_t)FLEXCOMM2:
        USART2_CFG_INIT(usart, 2);
        break;
    case (os_uint32_t)FLEXCOMM3:
        USART3_CFG_INIT(usart, 3);
        break;
    case (os_uint32_t)FLEXCOMM4:
        USART4_CFG_INIT(usart, 4);
        break;
    case (os_uint32_t)FLEXCOMM5:
        USART5_CFG_INIT(usart, 5);
        break;
    case (os_uint32_t)FLEXCOMM6:
        USART6_CFG_INIT(usart, 6);
        break;
    case (os_uint32_t)FLEXCOMM7:
        USART7_CFG_INIT(usart, 7);
        break;
    case (os_uint32_t)FLEXCOMM8:
         USART8_CFG_INIT(usart, 8);
        break;
    default:
        break;
    }
    if (usart->clk_src == 0)
        return OS_ERROR;
    return OS_EOK;
}

static int lpc_usart_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;
    
    struct serial_configure config  = OS_SERIAL_CONFIG_DEFAULT;
    
    lpc_usart_info_t *usart_info = (lpc_usart_info_t *)dev->info;

    lpc_usart_t *usart = os_calloc(1, sizeof(lpc_usart_t));

    OS_ASSERT(usart);
    
    usart->usart_info = usart_info;
    
    struct os_serial_device *serial = &usart->serial;
    serial->ops    = &usart_ops;
    serial->config = config;

    result = lpc_usart_param_cfg(usart);
    if ( result == OS_ERROR)
    {
        os_free(usart);
        return result;
    }

    lpc_usart_parse_configs_from_configtool(usart);

    level = os_irq_lock();
    os_list_add_tail(&lpc_usart_list, &usart->list);
    os_irq_unlock(level);
    
    result = os_hw_serial_register(serial, dev->name, NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO lpc_usart_driver = {
    .name   = "USART_Type",
    .probe  = lpc_usart_probe,
};

OS_DRIVER_DEFINE(lpc_usart_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

#ifdef OS_USING_CONSOLE
static USART_Type *console_usart = OS_NULL;
void __os_hw_console_output(char *str)
{
    if (console_usart == OS_NULL)
        return;

    while (*str)
    {   
        USART_WriteBlocking(console_usart, (uint8_t *)str, 1);
        str++;
    }
}

static int lpc_usart_early_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    if(!strcmp(dev->name,OS_CONSOLE_DEVICE_NAME))
    {
        lpc_usart_info_t *usart_info = (lpc_usart_info_t *)dev->info;
        console_usart = (USART_Type *)usart_info->usart_base;
    }
    return OS_EOK;
}

OS_DRIVER_INFO lpc_usart_early_driver = {
    .name   = "usart_HandleTypeDef",
    .probe  = lpc_usart_early_probe,
};

OS_DRIVER_DEFINE(lpc_usart_early_driver, CORE, OS_INIT_SUBLEVEL_LOW);
#endif
