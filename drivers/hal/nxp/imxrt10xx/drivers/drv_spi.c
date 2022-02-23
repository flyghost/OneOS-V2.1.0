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

struct os_imxrt_spi
{
    struct os_spi_bus               spi;
    struct nxp_lpspi_info          *info;

    imxrt_lpspi_mode                mode;
    os_uint32_t                     clk_src;

    IRQn_Type                       irqn;
    
    void                           *handle;
    void                           *edma_handle;
    
    lpspi_master_handle_t          *m_handle;
    lpspi_master_edma_handle_t     *m_edma_handle;
    
    lpspi_slave_handle_t           *s_handle;
    lpspi_slave_edma_handle_t      *s_edma_handle;

    lpspi_transfer_t                transfer;

    os_list_node_t                  list;
};

static os_list_node_t imxrt_spi_list = OS_LIST_INIT(imxrt_spi_list);

void imxrt_spi_irq_callback(struct os_imxrt_spi *imxrt_spi)
{

}

LPSPI_IRQHandler_DEFINE(1);
LPSPI_IRQHandler_DEFINE(2);
LPSPI_IRQHandler_DEFINE(3);
LPSPI_IRQHandler_DEFINE(4);

void imxrt_spi_m_dma_callback(LPSPI_Type *base, lpspi_master_edma_handle_t *handle, status_t status, void *userData)
{
    struct os_imxrt_spi *imxrt_spi = (struct os_imxrt_spi *)userData;
    
    if (kStatus_Success == status)
    {
    }
}

void imxrt_spi_m_transfer_callback(LPSPI_Type *base, lpspi_master_handle_t *handle, status_t status, void *userData)
{
    struct os_imxrt_spi *imxrt_spi = (struct os_imxrt_spi *)userData;
    
    if (kStatus_Success == status)
    {
    }
}

void imxrt_spi_s_dma_callback(LPSPI_Type *base, lpspi_slave_edma_handle_t *handle, status_t status, void *userData)
{
    struct os_imxrt_spi *imxrt_spi = (struct os_imxrt_spi *)userData;
    
    if (kStatus_Success == status)
    {
    }
}

void imxrt_spi_s_transfer_callback(LPSPI_Type *base, lpspi_slave_handle_t *handle, status_t status, void *userData)
{
    struct os_imxrt_spi *imxrt_spi = (struct os_imxrt_spi *)userData;
    
    if (kStatus_Success == status)
    {
    }
}

static os_err_t imxrt_spi_configure(struct os_spi_device *dev, struct os_spi_configuration *cfg)
{
    lpspi_master_config_t   masterConfig;
    lpspi_slave_config_t    slaveConfig;
    struct os_imxrt_spi    *imxrt_spi;
    
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    imxrt_spi = (struct os_imxrt_spi *)dev;
    
    if ((imxrt_spi->mode == IMXRT_LPSPI_MASTER) && !(cfg->mode & OS_SPI_SLAVE))
    {
        memcpy(&masterConfig, imxrt_spi->info->mconfig, sizeof(lpspi_master_config_t));
        
        if (cfg->mode & OS_SPI_CPHA)
        {
            masterConfig.cpha  = kLPSPI_ClockPhaseSecondEdge;
        }
        else
        {
            masterConfig.cpha  = kLPSPI_ClockPhaseFirstEdge;
        }

        if (cfg->mode & OS_SPI_CPOL)
        {
            masterConfig.cpol = kLPSPI_ClockPolarityActiveLow;
        }
        else
        {
            masterConfig.cpol = kLPSPI_ClockPolarityActiveHigh;
        }

        if (cfg->mode & OS_SPI_MSB)
        {
            masterConfig.direction = kLPSPI_MsbFirst;
        }
        else
        {
            masterConfig.direction = kLPSPI_LsbFirst;
        }
        
        masterConfig.baudRate       = cfg->max_hz;
        masterConfig.bitsPerFrame   = cfg->data_width;
        
        LPSPI_MasterInit(imxrt_spi->info->base, &masterConfig, imxrt_spi->clk_src);
    }
    else if ((imxrt_spi->mode == IMXRT_LPSPI_SLAVE) && (cfg->mode & OS_SPI_SLAVE))
    {
        memcpy(&slaveConfig, imxrt_spi->info->sconfig, sizeof(lpspi_slave_config_t));
        
        if (cfg->mode & OS_SPI_CPHA)
        {
            slaveConfig.cpha  = kLPSPI_ClockPhaseSecondEdge;
        }
        else
        {
            slaveConfig.cpha  = kLPSPI_ClockPhaseFirstEdge;
        }

        if (cfg->mode & OS_SPI_CPOL)
        {
            slaveConfig.cpol = kLPSPI_ClockPolarityActiveLow;
        }
        else
        {
            slaveConfig.cpol = kLPSPI_ClockPolarityActiveHigh;
        }

        if (cfg->mode & OS_SPI_MSB)
        {
            slaveConfig.direction = kLPSPI_MsbFirst;
        }
        else
        {
            slaveConfig.direction = kLPSPI_LsbFirst;
        }
        
        slaveConfig.bitsPerFrame   = cfg->data_width;
        
        LPSPI_SlaveInit(imxrt_spi->info->base, &slaveConfig);
    }
    else
    {
        LOG_E(DRV_EXT_TAG, "lpspi: tool config mode not support user mode!");
        return OS_ERROR;
    }
    
    return OS_EOK;
}

static os_uint32_t imxrt_spixfer(struct os_spi_device *dev, struct os_spi_message *message)
{
    status_t status = kStatus_Success;
    
    struct os_imxrt_spi *imxrt_spi = (struct os_imxrt_spi *)dev;
    
    OS_ASSERT(dev != OS_NULL);
    OS_ASSERT(dev->bus != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    if ((message->recv_buf == OS_NULL) && (message->send_buf == OS_NULL))
    {
        return 0;
    }
    
    imxrt_spi->transfer.rxData      = (uint8_t *)(message->recv_buf);
    imxrt_spi->transfer.txData      = (uint8_t *)(message->send_buf); 
    imxrt_spi->transfer.dataSize    = message->length;

    if (imxrt_spi->mode == IMXRT_LPSPI_MASTER)
    {
        if (message->cs_take)
        {
            os_pin_write(dev->cs_pin, PIN_LOW);
        }
    
        if (imxrt_spi->m_edma_handle != OS_NULL)
        {
            status = LPSPI_MasterTransferEDMA(imxrt_spi->info->base, imxrt_spi->m_edma_handle, &imxrt_spi->transfer);
        }
        else if (imxrt_spi->m_handle != OS_NULL)
        {
            status = LPSPI_MasterTransferNonBlocking(imxrt_spi->info->base, imxrt_spi->m_handle, &imxrt_spi->transfer);
        }
        else
        {
            status = LPSPI_MasterTransferBlocking(imxrt_spi->info->base, &imxrt_spi->transfer);
        }

        if(message->cs_release)
        {
            os_pin_write(dev->cs_pin, PIN_HIGH);
        }
    }
    else if (imxrt_spi->mode == IMXRT_LPSPI_SLAVE)
    {
        if (imxrt_spi->s_edma_handle != OS_NULL)
        {
            status = LPSPI_SlaveTransferEDMA(imxrt_spi->info->base, imxrt_spi->s_edma_handle, &imxrt_spi->transfer);
        }
        else if (imxrt_spi->s_handle != OS_NULL)
        {
            status = LPSPI_SlaveTransferNonBlocking(imxrt_spi->info->base, imxrt_spi->s_handle, &imxrt_spi->transfer);
        }
    }

    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "%s transfer error : %d", imxrt_spi->spi.parent.name, status);
        return 0;;
    }

    return message->length; 
}

static const struct os_spi_ops imxrt_spi_ops = {
    .configure = imxrt_spi_configure,
    .xfer      = imxrt_spixfer
};

void imxrt_spi_param_cfg(struct os_imxrt_spi *imxrt_spi)
{
    if (LPSPI_IsMaster(imxrt_spi->info->base) == true)
    {
        imxrt_spi->mode                         = IMXRT_LPSPI_MASTER;
        imxrt_spi->m_edma_handle                = (lpspi_master_edma_handle_t *)imxrt_spi->edma_handle;
        imxrt_spi->m_handle                     = (lpspi_master_handle_t *)imxrt_spi->handle;

        if (imxrt_spi->m_edma_handle != OS_NULL)
        {
            imxrt_spi->m_edma_handle->callback  = imxrt_spi_m_dma_callback;
            imxrt_spi->m_edma_handle->userData  = imxrt_spi;
        }

        if (imxrt_spi->m_handle != OS_NULL)
        {
            imxrt_spi->m_handle->callback       = imxrt_spi_m_transfer_callback;
            imxrt_spi->m_handle->userData       = imxrt_spi;
        }
    }
    else
    {
        imxrt_spi->mode                         = IMXRT_LPSPI_SLAVE;
        imxrt_spi->s_edma_handle                = (lpspi_slave_edma_handle_t *)imxrt_spi->edma_handle;
        imxrt_spi->s_handle                     = (lpspi_slave_handle_t *)imxrt_spi->handle;

        if (imxrt_spi->s_edma_handle != OS_NULL)
        {
            imxrt_spi->s_edma_handle->callback  = imxrt_spi_s_dma_callback;
            imxrt_spi->s_edma_handle->userData  = imxrt_spi;
        }
        
        if (imxrt_spi->s_handle != OS_NULL)
        {
            imxrt_spi->s_handle->callback       = imxrt_spi_s_transfer_callback;
            imxrt_spi->s_handle->userData       = imxrt_spi;
        }
    }

    switch((os_uint32_t)imxrt_spi->info->base)
    {
    case (os_uint32_t)LPSPI1:
        LPSPI1_CFG_INIT(imxrt_spi, 1);
    break;
    case (os_uint32_t)LPSPI2:
        LPSPI2_CFG_INIT(imxrt_spi, 2);
    break;
    case (os_uint32_t)LPSPI3:
        LPSPI3_CFG_INIT(imxrt_spi, 3);
    break;
    case (os_uint32_t)LPSPI4:
        LPSPI4_CFG_INIT(imxrt_spi, 4);
    break;
    default:
        break;
    }
}

static int imxrt_spi_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_err_t    result  = 0;
    os_base_t   level;

    struct os_imxrt_spi *imxrt_spi = os_calloc(1, sizeof(struct os_imxrt_spi));
    OS_ASSERT(imxrt_spi);
    
    imxrt_spi->info = (struct nxp_lpspi_info *)dev->info;
    
    imxrt_spi_param_cfg(imxrt_spi);

    imxrt_spi->spi.ops = &imxrt_spi_ops;

    level = os_irq_lock();
    os_list_add_tail(&imxrt_spi_list, &imxrt_spi->list);
    os_irq_unlock(level);
    
    result = os_spi_bus_register(&imxrt_spi->spi, dev->name, &imxrt_spi_ops);
    OS_ASSERT(result == OS_EOK);

    return result;

}

OS_DRIVER_INFO imxrt_spi_driver = {
    .name   = "LPSPI_Type",
    .probe  = imxrt_spi_probe,
};

OS_DRIVER_DEFINE(imxrt_spi_driver, PREV, OS_INIT_SUBLEVEL_LOW);

