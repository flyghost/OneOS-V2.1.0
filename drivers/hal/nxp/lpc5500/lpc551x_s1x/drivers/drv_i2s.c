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
 * @file        drv_i2s.c
 *
 * @brief       This file implements i2s driver for nxp.
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
#include <drv_i2s.h>
#include <i2s.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.i2s"
#include <drv_log.h>

typedef struct lpc_i2s
{
    struct os_i2s_device i2s;
    lpc_i2s_info_t *i2s_info;
    i2s_transfer_t sendXfer;
    i2s_transfer_t receiveXfer;
    os_uint32_t clk_src;
    struct os_device_cb_info info;
    i2s_transfer_t data_buff_info[I2S_NUM_BUFFERS];

    IRQn_Type irqn;
    i2s_handle_t *i2s_handle;
    i2s_dma_handle_t *i2s_DmaHandle;

    os_list_node_t list;
    os_sem_t sem;
}lpc_i2s_t;

static os_list_node_t lpc_i2s_list = OS_LIST_INIT(lpc_i2s_list);

void lpc_i2s_irq_callback(lpc_i2s_t *lpc_i2s)
{
}

I2S_IRQHandler_DEFINE(0);
I2S_IRQHandler_DEFINE(1);
I2S_IRQHandler_DEFINE(2);
I2S_IRQHandler_DEFINE(3);
I2S_IRQHandler_DEFINE(4);
I2S_IRQHandler_DEFINE(5);
I2S_IRQHandler_DEFINE(6);
I2S_IRQHandler_DEFINE(7);
I2S_IRQHandler_DEFINE(8);

void lpc_i2s_dma_callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t status, void *userData)
{
    lpc_i2s_t *lpc_i2s = (lpc_i2s_t *)userData;
    
    if (kStatus_I2S_BufferComplete == status)
    {
        if (lpc_i2s->i2s_DmaHandle->queueDriver == 0)
        {
            lpc_i2s->info.data = lpc_i2s->data_buff_info[I2S_NUM_BUFFERS - 1].data;
            lpc_i2s->info.size = lpc_i2s->data_buff_info[I2S_NUM_BUFFERS - 1].dataSize;
            lpc_i2s->data_buff_info[I2S_NUM_BUFFERS - 1].data = OS_NULL;
            lpc_i2s->data_buff_info[I2S_NUM_BUFFERS - 1].dataSize = 0;
        }
        else
        {
            lpc_i2s->info.data = lpc_i2s->data_buff_info[lpc_i2s->i2s_DmaHandle->queueDriver - 1].data;
            lpc_i2s->info.size = lpc_i2s->data_buff_info[lpc_i2s->i2s_DmaHandle->queueDriver - 1].dataSize;
            lpc_i2s->data_buff_info[lpc_i2s->i2s_DmaHandle->queueDriver - 1].data = OS_NULL;
            lpc_i2s->data_buff_info[lpc_i2s->i2s_DmaHandle->queueDriver - 1].dataSize = 0;
        }
        
        os_hw_i2s_isr(&lpc_i2s->i2s, &lpc_i2s->info);
        os_sem_post(&lpc_i2s->sem);
    }
}

void lpc_i2s_transfer_callback(I2S_Type *base, i2s_handle_t *handle, status_t status, void *userData)
{
    lpc_i2s_t *lpc_i2s = (lpc_i2s_t *)userData;
    
    if (kStatus_I2S_BufferComplete == status)
    {
        if (lpc_i2s->i2s_handle->queueDriver == 0)
        {
            lpc_i2s->info.data = lpc_i2s->data_buff_info[I2S_NUM_BUFFERS - 1].data;
            lpc_i2s->info.size = lpc_i2s->data_buff_info[I2S_NUM_BUFFERS - 1].dataSize;
        }
        else
        {
            lpc_i2s->info.data = lpc_i2s->data_buff_info[lpc_i2s->i2s_handle->queueDriver - 1].data;
            lpc_i2s->info.size = lpc_i2s->data_buff_info[lpc_i2s->i2s_handle->queueDriver - 1].dataSize;
        }
        
        os_hw_i2s_isr(&lpc_i2s->i2s, &lpc_i2s->info);
        os_sem_post(&lpc_i2s->sem); 
    }
}

static os_err_t lpc_i2s_init(struct os_i2s_device *i2s)
{
    status_t status;
    
    lpc_i2s_t *lpc_i2s = (lpc_i2s_t *)i2s;
    
    return OS_EOK;
}

static os_err_t lpc_i2s_transmit(struct os_i2s_device *i2s, uint8_t *buff, uint32_t size)
{
    status_t status;
    
    lpc_i2s_t *lpc_i2s = (lpc_i2s_t *)i2s;
    
    OS_ASSERT(buff != OS_NULL);

    lpc_i2s->sendXfer.data = buff;
    lpc_i2s->sendXfer.dataSize = size;

    os_sem_wait(&lpc_i2s->sem, OS_WAIT_FOREVER);
    
    if (lpc_i2s->i2s_DmaHandle != OS_NULL)
    {
        lpc_i2s->data_buff_info[lpc_i2s->i2s_DmaHandle->queueUser].data = buff;
        lpc_i2s->data_buff_info[lpc_i2s->i2s_DmaHandle->queueUser].dataSize = size;
        
        status = I2S_TxTransferSendDMA(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_DmaHandle, lpc_i2s->sendXfer);
        if (status != kStatus_Success)
        {
            LOG_E(DRV_EXT_TAG, "%s transfer error : %d", lpc_i2s->i2s.parent.name, status);
            return OS_ERROR;
        }
    }
    else if (lpc_i2s->i2s_handle != OS_NULL)
    {
        lpc_i2s->data_buff_info[lpc_i2s->i2s_handle->queueUser].data = buff;
        lpc_i2s->data_buff_info[lpc_i2s->i2s_handle->queueUser].dataSize = size;

        status = I2S_TxTransferNonBlocking(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_handle, lpc_i2s->sendXfer);
        if (status != kStatus_Success)
        {
            LOG_E(DRV_EXT_TAG, "%s transfer error : %d", lpc_i2s->i2s.parent.name, status);
            return OS_ERROR;
        }
    }
    else
    {
        LOG_E(DRV_EXT_TAG, "i2s driver not supports irq mode!");
        return OS_ERROR;
    }

    return OS_EOK; 
}

static os_err_t lpc_i2s_receive(struct os_i2s_device *i2s, uint8_t *buff, uint32_t size)
{
    status_t status;
    
    lpc_i2s_t *lpc_i2s = (lpc_i2s_t *)i2s;
    
    OS_ASSERT(buff != OS_NULL);

    lpc_i2s->receiveXfer.data = (uint8_t *)buff;
    lpc_i2s->receiveXfer.dataSize = size/2;


    if (lpc_i2s->i2s_DmaHandle != OS_NULL)
        status = I2S_RxTransferReceiveDMA(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_DmaHandle, lpc_i2s->receiveXfer);
    else if (lpc_i2s->i2s_handle != OS_NULL)
        status = I2S_RxTransferNonBlocking(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_handle, lpc_i2s->receiveXfer);
    else
        status = I2S_RxTransferNonBlocking(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_handle, lpc_i2s->receiveXfer);

    if (status != kStatus_Success)
    {
        LOG_E(DRV_EXT_TAG, "%s transfer error : %d", lpc_i2s->i2s.parent.name, status);
        return OS_ERROR;
    }

    return OS_EOK; 
}

static os_err_t lpc_i2s_enable(struct os_i2s_device *i2s, os_bool_t enable)
{
    status_t status;
    
    lpc_i2s_t *lpc_i2s = (lpc_i2s_t *)i2s;
    
    if (enable == OS_TRUE)
    {
        os_sem_init(&lpc_i2s->sem, "lpc_i2s_sem", I2S_NUM_BUFFERS, I2S_NUM_BUFFERS);
        return OS_EOK;
    }
    else
    {
        if (lpc_i2s->i2s_DmaHandle != OS_NULL)
            I2S_TransferAbortDMA(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_DmaHandle);
        else if (lpc_i2s->i2s_handle != OS_NULL)
        {
            I2S_TxTransferAbort(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_handle);
            I2S_RxTransferAbort(lpc_i2s->i2s_info->i2s_base, lpc_i2s->i2s_handle);
        }
        else
        {

        }
        os_sem_deinit(&lpc_i2s->sem);
    }

    return OS_EOK;
}

static os_err_t lpc_i2s_set_frq(struct os_i2s_device *i2s, uint32_t frequency)
{
    status_t status;

    lpc_i2s_t *lpc_i2s = (lpc_i2s_t *)i2s;
    
    I2S_SetBitClockRate(lpc_i2s->i2s_info->i2s_base, lpc_i2s->clk_src, frequency, lpc_i2s->i2s_info->i2s_config->dataLength, \
    lpc_i2s->i2s_info->i2s_config->frameLength / lpc_i2s->i2s_info->i2s_config->dataLength);

    return OS_EOK;
}

static os_err_t lpc_i2s_set_channel(struct os_i2s_device *i2s, uint8_t channels)
{    
    status_t status;

    return OS_EOK;
}

static const struct os_i2s_ops lpc_i2s_ops =
{
    .init           = lpc_i2s_init,
    .transimit      = lpc_i2s_transmit,
    .receive        = lpc_i2s_receive,
    .enable_tx      = lpc_i2s_enable,
    .set_frq        = lpc_i2s_set_frq,
    .set_channel    = lpc_i2s_set_channel,
};

void lpc_i2s_parse_configs_from_configtool(lpc_i2s_t *lpc_i2s)
{
    struct os_i2s_device *i2s = &lpc_i2s->i2s;
}

void lpc_i2s_param_cfg(lpc_i2s_t *lpc_i2s)
{
    switch((os_uint32_t)lpc_i2s->i2s_info->i2s_base)
    {
    case (os_uint32_t)FLEXCOMM0:
        I2S0_CFG_INIT(lpc_i2s, 0);
        break;
    case (os_uint32_t)FLEXCOMM1:
        I2S1_CFG_INIT(lpc_i2s, 1);
        break;
    case (os_uint32_t)FLEXCOMM2:
        I2S2_CFG_INIT(lpc_i2s, 2);
        break;
    case (os_uint32_t)FLEXCOMM3:
        I2S3_CFG_INIT(lpc_i2s, 3);
        break;
    case (os_uint32_t)FLEXCOMM4:
        I2S4_CFG_INIT(lpc_i2s, 4);
        break;
    case (os_uint32_t)FLEXCOMM5:
        I2S5_CFG_INIT(lpc_i2s, 5);
        break;
    case (os_uint32_t)FLEXCOMM6:
        I2S6_CFG_INIT(lpc_i2s, 6);
        break;
    case (os_uint32_t)FLEXCOMM7:
        I2S7_CFG_INIT(lpc_i2s, 7);
        break;
    case (os_uint32_t)FLEXCOMM8:
        I2S8_CFG_INIT(lpc_i2s, 8);
        break;
    default:
        break;
    }
}

static int lpc_i2s_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{   
    os_err_t    result  = 0;
    os_base_t   level;
    
    lpc_i2s_info_t *i2s_info = (lpc_i2s_info_t *)dev->info;

    lpc_i2s_t *lpc_i2s = os_calloc(1, sizeof(lpc_i2s_t));

    OS_ASSERT(lpc_i2s);
    
    lpc_i2s->i2s_info = i2s_info;
    lpc_i2s_param_cfg(lpc_i2s);
    
    struct os_i2s_device *i2s = &lpc_i2s->i2s;

    i2s->ops = &lpc_i2s_ops;

    level = os_irq_lock();
    os_list_add_tail(&lpc_i2s_list, &lpc_i2s->list);
    os_irq_unlock(level);
    
    result = os_i2s_register(i2s, dev->name, OS_NULL);
    
    OS_ASSERT(result == OS_EOK);

    return result;
}

OS_DRIVER_INFO lpc_i2s_driver = {
    .name   = "I2S_Type",
    .probe  = lpc_i2s_probe,
};

OS_DRIVER_DEFINE(lpc_i2s_driver, DEVICE, OS_INIT_SUBLEVEL_HIGH);