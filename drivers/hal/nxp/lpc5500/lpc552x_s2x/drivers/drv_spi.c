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
 * @file        drv_spi.c
 *
 * @brief       This file implements spi driver for nxp.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-01   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_task.h>
#include <device.h>
#include <os_memory.h>
#include <string.h>
#include <drv_spi.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.spi"
#include <drv_log.h>

typedef struct lpc_spi
{
    struct os_spi_bus spi;
    struct lpc_spi_info *spi_info;
    spi_transfer_t transXfer;
    os_uint32_t clk_src;

    IRQn_Type irqn;
    spi_master_handle_t *spi_handle;
    spi_dma_handle_t *spi_DmaHandle;

    os_list_node_t list;
}lpc_spi_t;

static os_list_node_t lpc_spi_list = OS_LIST_INIT(lpc_spi_list);

void lpc_spi_irq_callback(lpc_spi_t *lpc_spi)
{
    if (SPI_GetStatusFlags(lpc_spi->spi_info->spi_base) & kSPI_RxNotEmptyFlag)
    {
        os_device_recv_notify(&lpc_spi->spi.parent);
    }
}

SPI_IRQHandler_DEFINE(0);
SPI_IRQHandler_DEFINE(1);
SPI_IRQHandler_DEFINE(2);
SPI_IRQHandler_DEFINE(3);
SPI_IRQHandler_DEFINE(4);
SPI_IRQHandler_DEFINE(5);
SPI_IRQHandler_DEFINE(6);
SPI_IRQHandler_DEFINE(7);
SPI_IRQHandler_DEFINE(8);

void lpc_spi_dma_callback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData)
{
    struct lpc_spi *lpc_spi = (struct lpc_spi *)userData;
    
    if (kStatus_SPI_Idle == status)
    {
        os_device_recv_notify(&lpc_spi->spi.parent);
    }
}

void lpc_spi_transfer_callback(SPI_Type *base, spi_master_handle_t *handle, status_t status, void *userData)
{
    struct lpc_spi *lpc_spi = (struct lpc_spi *)userData;
    
    if (kStatus_SPI_Idle == status)
    {
        os_device_recv_notify(&lpc_spi->spi.parent);
    }
}

static os_err_t lpc_spi_configure(struct os_spi_device *device, struct os_spi_configuration *cfg)
{
    struct lpc_spi *lpc_spi = (struct lpc_spi *)device->bus;
    
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    LOG_I(DRV_EXT_TAG, "nxp spi driver don't support config! please use config tool!");
    
    return OS_EOK;
}

static os_uint32_t lpc_spixfer(struct os_spi_device *device, struct os_spi_message *message)
{
    status_t status;
    
    struct lpc_spi *lpc_spi = (struct lpc_spi *)device->bus;
    
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL); 

    if (message->cs_take)
    {
        os_pin_write(device->pin, PIN_LOW);
    }

    if ((message->recv_buf == OS_NULL) && (message->send_buf == OS_NULL))
    {
        return 0;
    }
    lpc_spi->transXfer.rxData = (uint8_t *)(message->recv_buf); 
    lpc_spi->transXfer.txData = (uint8_t *)(message->send_buf); 
    lpc_spi->transXfer.dataSize = message->length;

    if (lpc_spi->spi_DmaHandle != OS_NULL)
        status = SPI_MasterTransferDMA(lpc_spi->spi_info->spi_base, lpc_spi->spi_DmaHandle, &lpc_spi->transXfer);
    else if (lpc_spi->spi_handle != OS_NULL)
        status = SPI_MasterTransferNonBlocking(lpc_spi->spi_info->spi_base, lpc_spi->spi_handle, &lpc_spi->transXfer);
    else
        status = SPI_MasterTransferBlocking(lpc_spi->spi_info->spi_base, &lpc_spi->transXfer);

    if(message->cs_release)
    {
        os_pin_write(device->pin, PIN_HIGH);
    }

    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "%s transfer error : %d", lpc_spi->spi.parent.name, status);
        message->length = 0;
    }

    return message->length; 
}

static const struct os_spi_ops lpc_spi_ops = {
    .configure = lpc_spi_configure,
    .xfer      = lpc_spixfer
};

void lpc_spi_param_cfg(struct lpc_spi *lpc_spi)
{
    switch((os_uint32_t)lpc_spi->spi_info->spi_base)
    {
    case (os_uint32_t)FLEXCOMM0:
        SPI0_CFG_INIT(lpc_spi, 0);
        break;
    case (os_uint32_t)FLEXCOMM1:
        SPI1_CFG_INIT(lpc_spi, 1);
        break;
    case (os_uint32_t)FLEXCOMM2:
        SPI2_CFG_INIT(lpc_spi, 2);
        break;
    case (os_uint32_t)FLEXCOMM3:
        SPI3_CFG_INIT(lpc_spi, 3);
        break;
    case (os_uint32_t)FLEXCOMM4:
        SPI4_CFG_INIT(lpc_spi, 4);
        break;
    case (os_uint32_t)FLEXCOMM5:
        SPI5_CFG_INIT(lpc_spi, 5);
        break;
    case (os_uint32_t)FLEXCOMM6:
        SPI6_CFG_INIT(lpc_spi, 6);
        break;
    case (os_uint32_t)FLEXCOMM7:
        SPI7_CFG_INIT(lpc_spi, 7);
        break;
    case (os_uint32_t)FLEXCOMM8:
        SPI8_CFG_INIT(lpc_spi, 8);
        break;
    default:
        break;
    }
}

static int lpc_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;
    
    struct lpc_spi_info *spi_info = (struct lpc_spi_info *)dev->info;

    struct lpc_spi *lpc_spi = os_calloc(1, sizeof(struct lpc_spi));

    OS_ASSERT(lpc_spi);
    
    lpc_spi->spi_info = spi_info;
    lpc_spi_param_cfg(lpc_spi);
    
    struct os_spi_bus *spi = &lpc_spi->spi;

    spi->ops = &lpc_spi_ops;

    level = os_irq_lock();
    os_list_add_tail(&lpc_spi_list, &lpc_spi->list);
    os_irq_unlock(level);
    
    result = os_spi_bus_register(spi, dev->name, &lpc_spi_ops);
    OS_ASSERT(result == OS_EOK);

    return result;

}

OS_DRIVER_INFO lpc_spi_driver = {
    .name   = "SPI_Type",
    .probe  = lpc_spi_probe,
};

OS_DRIVER_DEFINE(lpc_spi_driver, PREV, OS_INIT_SUBLEVEL_LOW);

