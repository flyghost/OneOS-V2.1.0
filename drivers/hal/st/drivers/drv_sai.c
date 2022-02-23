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
 * @file        drv_sai.c
 *
 * @brief       This file implements sai driver for stm32
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "board.h"
#include "drv_cfg.h"
#include <os_memory.h>
#include <bus/bus.h>
#include <audio/sai.h>
#include <string.h>

#define DBG_EXT_TAG "drv.sai"
#include <dlog.h>
#include <arch_interrupt.h>
#include <clocksource.h>

#define SAI_BUFF_NUM 2

enum
{
    SAI_DMA_STATUS_STOP = 0U,
    SAI_DMA_STATUS_START = 1U,
};


typedef struct stm32_sai_transfer
{
    uint8_t *data;
    size_t datasize;
} stm32_sai_transfer_t;

typedef struct stm32_sai_fifo
{
    os_uint8_t *rx_fifo[OS_AUDIO_RECORD_FIFO_COUNT];
    os_uint8_t  fifo_rd_pos;
    os_uint8_t  fifo_wr_pos;
} stm32_sai_fifo_t;

struct stm32_sai
{
    os_device_sai_t sai;
    SAI_HandleTypeDef *hsai;
    os_uint8_t data_buff[OS_AUDIO_REPLAY_MP_BLOCK_SIZE * SAI_BUFF_NUM];
    stm32_sai_transfer_t sendXfer[SAI_BUFF_NUM];
    stm32_sai_fifo_t fifo;
    os_uint8_t write_index;
    os_uint8_t write_status;
    os_uint8_t read_status;
    
    struct os_device_cb_info info;
    
    os_list_node_t list;
    os_sem_t sem;
};

static os_uint8_t *sai_dma_recv;

static os_uint8_t zero_frame[2] = {0};

static os_list_node_t stm32_sai_list = OS_LIST_INIT(stm32_sai_list);

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    struct stm32_sai *sai_dev;
    
    os_list_for_each_entry(sai_dev, &stm32_sai_list, struct stm32_sai, list)
    {
        if (hsai == sai_dev->hsai)
        {
            if (sai_dev->sendXfer[1].data != OS_NULL)
            {
                sai_dev->info.data = sai_dev->sendXfer[1].data;
                sai_dev->info.size = sai_dev->sendXfer[1].datasize;
                memset(&sai_dev->data_buff[OS_AUDIO_REPLAY_MP_BLOCK_SIZE], 0, sai_dev->sendXfer[1].datasize);
                os_hw_sai_isr(&sai_dev->sai, &sai_dev->info);
                os_sem_post(&sai_dev->sem);
                sai_dev->sendXfer[1].data = OS_NULL;
            }

            sai_dev->write_index = 1;
            break;
        }
    }
}

void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    struct stm32_sai *sai_dev;
    
    os_list_for_each_entry(sai_dev, &stm32_sai_list, struct stm32_sai, list)
    {
        if (hsai == sai_dev->hsai)
        {
            if (sai_dev->sendXfer[0].data != OS_NULL)
            {
                sai_dev->info.data = sai_dev->sendXfer[0].data;
                sai_dev->info.size = sai_dev->sendXfer[0].datasize;
                memset(&sai_dev->data_buff[0], 0, sai_dev->sendXfer[0].datasize);
                os_hw_sai_isr(&sai_dev->sai, &sai_dev->info);
                os_sem_post(&sai_dev->sem);
                sai_dev->sendXfer[0].data = OS_NULL;
            }

            sai_dev->write_index = 0;
            break;
        }
    }
}

static void HAL_SAI_RxCallback(SAI_HandleTypeDef *hsai, int pos)
{
    struct stm32_sai *sai_dev;   
    
    os_list_for_each_entry(sai_dev, &stm32_sai_list, struct stm32_sai, list)
    {
        if (hsai == sai_dev->hsai)
        {                 
            os_uint8_t next_fifo_wr_pos = sai_dev->fifo.fifo_wr_pos + 1;

            if (next_fifo_wr_pos >= OS_AUDIO_RECORD_FIFO_COUNT)
                next_fifo_wr_pos = 0; 

            if (next_fifo_wr_pos == sai_dev->fifo.fifo_rd_pos)
                return;

            sai_dev->fifo.fifo_wr_pos = next_fifo_wr_pos;
            
            memcpy(sai_dev->fifo.rx_fifo[sai_dev->fifo.fifo_wr_pos], sai_dma_recv + pos, OS_AUDIO_RECORD_FIFO_SIZE);

            os_sem_post(&sai_dev->sem);
        }
    }
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) 
{
    HAL_SAI_RxCallback(hsai, 0);
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    HAL_SAI_RxCallback(hsai, OS_AUDIO_RECORD_FIFO_SIZE);
}

static os_err_t stm32_sai_dma_transmit(os_device_sai_t *sai, os_uint8_t *buff, uint32_t size)
{
    os_base_t level;
    
    HAL_StatusTypeDef status = HAL_OK;
    
    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    os_sem_wait(&sai_dev->sem, OS_WAIT_FOREVER);
    
    level = os_irq_lock();
    memcpy(&sai_dev->data_buff[sai_dev->write_index * OS_AUDIO_REPLAY_MP_BLOCK_SIZE], buff, size);
    sai_dev->sendXfer[sai_dev->write_index].data = buff;
    sai_dev->sendXfer[sai_dev->write_index].datasize = size;
    
    if ((sai_dev->sem.count > 0) && (sai_dev->sem.count < SAI_BUFF_NUM))
    {
        if (sai_dev->write_index < SAI_BUFF_NUM - 1)
            sai_dev->write_index++;
        else
            sai_dev->write_index = 0;
    } 
    os_irq_unlock(level);
    
    if (sai_dev->write_status == SAI_DMA_STATUS_STOP)
    {
        if ((sai_dev->sendXfer[0].data) && (sai_dev->sendXfer[1].data))
        {
            status = HAL_SAI_Transmit_DMA(sai_dev->hsai, sai_dev->data_buff, OS_AUDIO_REPLAY_MP_BLOCK_SIZE);
            sai_dev->write_status = SAI_DMA_STATUS_START;
        }
    }
        
    if (status != HAL_OK)
    {
        LOG_E(DBG_EXT_TAG,"sai dma start transmit failed!\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_sai_dma_receive(os_device_sai_t *sai, os_uint8_t *buff, uint32_t size) 
{   
    os_base_t level;
    
    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    int i;
    
    os_sem_wait(&sai_dev->sem, OS_WAIT_FOREVER);
    
    if (sai_dev->sem.count > OS_AUDIO_RECORD_FIFO_COUNT)
    {
        sai_dev->sem.count = OS_AUDIO_RECORD_FIFO_COUNT;
    }
               
    if (sai_dev->fifo.fifo_rd_pos == sai_dev->fifo.fifo_wr_pos)
        return OS_EEMPTY;
       
    level = os_irq_lock();
    
    sai_dev->fifo.fifo_rd_pos++;
    if(sai_dev->fifo.fifo_rd_pos >= OS_AUDIO_RECORD_FIFO_COUNT)
        sai_dev->fifo.fifo_rd_pos = 0; 
        
    for (i = 0; i < OS_AUDIO_RECORD_FIFO_SIZE; i++)
    {
        buff[i] = sai_dev->fifo.rx_fifo[sai_dev->fifo.fifo_rd_pos][i];
    }
    
    os_irq_unlock(level);
    
    return OS_EOK;  
    
}

static os_err_t stm32_sai_dma_rx_enable(os_device_sai_t *sai, os_bool_t enable)
{
    HAL_StatusTypeDef status;
    os_device_t *sai_clk;
    struct stm32_sai *sai_clk_dev;
    int i;

    struct stm32_sai *sai_dev = (struct stm32_sai *)sai; 

    if (enable == OS_TRUE)
    {        
        sai_dev->read_status = SAI_DMA_STATUS_STOP;
        
        os_sem_init(&sai_dev->sem, "sai_sem", OS_AUDIO_RECORD_FIFO_COUNT, OS_AUDIO_RECORD_FIFO_COUNT);        
        
        sai_clk = os_device_find(BSP_AUDIO_DATA_TX_BUS);
        
        if (sai_clk == OS_NULL)
        {
            LOG_E(DBG_EXT_TAG, "can not find the sai receiver clock provider!\r\n");
            return OS_ERROR;
        }
        
        sai_clk_dev = (struct stm32_sai *)sai_clk;

        for (i = 0; i < OS_AUDIO_RECORD_FIFO_COUNT; i++)
        {
            sai_dev->fifo.rx_fifo[i] = os_calloc(1, OS_AUDIO_RECORD_FIFO_SIZE);
            if (sai_dev->fifo.rx_fifo[i] == OS_NULL)
                return OS_ENOMEM;
            memset(sai_dev->fifo.rx_fifo[i], 0, OS_AUDIO_RECORD_FIFO_SIZE);
        }

        sai_dev->fifo.fifo_rd_pos = 0;
        sai_dev->fifo.fifo_wr_pos = 0;

        /* half word */
        sai_dma_recv = os_calloc(1, OS_AUDIO_RECORD_FIFO_SIZE * 2);
        
        status = HAL_SAI_Transmit(sai_clk_dev->hsai, &zero_frame[0], 2, 0);
        if (status != HAL_OK)
        {
            LOG_E(DBG_EXT_TAG, "sai receiver clock device start failed!\r\n");
            return OS_ERROR;
        }
        
        status = HAL_SAI_Receive_DMA(sai_dev->hsai, sai_dma_recv, OS_AUDIO_RECORD_FIFO_SIZE);
        if (status != HAL_OK)
        {
            LOG_E(DBG_EXT_TAG, "sai dma start receive failed!\r\n");
            return OS_ERROR;
        }
        
        sai_dev->read_status = SAI_DMA_STATUS_START;
        
        return OS_EOK;
    }
    else
    {
        status = HAL_SAI_DMAStop(sai_dev->hsai);
        sai_dev->read_status = SAI_DMA_STATUS_STOP;
        
        os_sem_deinit(&sai_dev->sem);
        
        os_free(sai_dma_recv);
        
        for (i = 0; i < OS_AUDIO_RECORD_FIFO_COUNT; i++)
        {
            os_free(sai_dev->fifo.rx_fifo[i]);
        }
    }
       
    if (status != HAL_OK)
    {
        LOG_E(DBG_EXT_TAG, "sai dma stop failed!\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_sai_dma_tx_enable(os_device_sai_t *sai, os_bool_t enable)
{
    HAL_StatusTypeDef status;

    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    if (enable == OS_TRUE)
    {
        sai_dev->write_status = SAI_DMA_STATUS_STOP;
        os_sem_init(&sai_dev->sem, "sai_sem", 2, 2);
        sai_dev->sendXfer[0].data = OS_NULL;
        sai_dev->sendXfer[0].datasize = 0;
        sai_dev->sendXfer[1].data = OS_NULL;
        sai_dev->sendXfer[1].datasize = 0;
        sai_dev->write_index = 0;
        return OS_EOK;
    }
    else
    {
        status = HAL_SAI_DMAStop(sai_dev->hsai);
        sai_dev->write_status = SAI_DMA_STATUS_STOP;
        os_sem_deinit(&sai_dev->sem);
    }
    
    if (status != HAL_OK)
    {
        LOG_E(DBG_EXT_TAG,"sai dma stop failed!\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_sai_set_frq(os_device_sai_t *sai, uint32_t frequency)
{
    HAL_StatusTypeDef status;
    
    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    __HAL_SAI_DISABLE(sai_dev->hsai);
    sai_dev->hsai->Init.AudioFrequency = frequency;
    status = HAL_SAI_Init(sai_dev->hsai);
    if (status != HAL_OK)
    {
        LOG_E(DBG_EXT_TAG,"sai init failed!\r\n");
        return OS_ERROR;
    }
    __HAL_SAI_ENABLE(sai_dev->hsai);

    return OS_EOK;
}

static os_err_t stm32_sai_set_channel(os_device_sai_t *sai, uint8_t channels)
{    
    HAL_StatusTypeDef status;

    struct stm32_sai *sai_dev = (struct stm32_sai *)sai;
    
    if (channels == 1)
    {
        sai_dev->hsai->Init.MonoStereoMode = SAI_MONOMODE;
    }
    else
    {
        sai_dev->hsai->Init.MonoStereoMode = SAI_STEREOMODE;
    }
    
    __HAL_SAI_DISABLE(sai_dev->hsai);
    status = HAL_SAI_Init(sai_dev->hsai);
    if (status != HAL_OK)
    {
        LOG_E(DBG_EXT_TAG,"sai set channel failed!\r\n");
        return OS_ERROR;
    }
    __HAL_SAI_ENABLE(sai_dev->hsai);

    return OS_EOK;

}

const static struct os_device_sai_ops ops =
{
    .transimit      = stm32_sai_dma_transmit,
    .receive        = stm32_sai_dma_receive,
    .enable_tx      = stm32_sai_dma_tx_enable,
    .enable_rx      = stm32_sai_dma_rx_enable,
    .set_frq        = stm32_sai_set_frq,
    .set_channel    = stm32_sai_set_channel,
};

static int stm32_sai_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t level;
    
    struct stm32_sai *sai_dev = os_calloc(1, sizeof(struct stm32_sai));

    OS_ASSERT(sai_dev);
   
    sai_dev->hsai = (SAI_HandleTypeDef *)dev->info;

    sai_dev->sai.ops = &ops;
    
    level = os_irq_lock();
    os_list_add_tail(&stm32_sai_list, &sai_dev->list);
    os_irq_unlock(level);

    os_sai_register(dev->name, &sai_dev->sai); 

    LOG_D(DBG_EXT_TAG,"stm32 sai found.\r\n");
    
    return OS_EOK;
}


OS_DRIVER_INFO stm32_sai_driver = {
    .name   = "SAI_HandleTypeDef",
    .probe  = stm32_sai_probe,
};

OS_DRIVER_DEFINE(stm32_sai_driver,PREV,OS_INIT_SUBLEVEL_LOW);
