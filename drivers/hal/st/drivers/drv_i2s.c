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
 * @brief       This file implements i2s driver for stm32
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
#include <audio/i2s.h>
#include <string.h>

#define DBG_TAG "drv.i2s"
#include <dlog.h>
#include <arch_interrupt.h>
#include <clocksource.h>

#define I2S_BUFF_NUM 2

enum
{
    I2S_DMA_STATUS_STOP = 0U,
    I2S_DMA_STATUS_START = 1U,
};

typedef struct stm32_i2s_transfer
{
    uint8_t *data;
    size_t datasize;
} stm32_i2s_transfer_t;

typedef struct stm32_i2s_fifo
{
    os_uint8_t *rx_fifo[OS_AUDIO_RECORD_FIFO_COUNT];
    os_uint8_t fifo_rd_pos;
    os_uint8_t fifo_wr_pos;
} stm32_i2s_fifo_t;

struct stm32_i2s
{
    os_i2s_device_t i2s;
    I2S_HandleTypeDef *hi2s;
    os_uint8_t data_buff[OS_AUDIO_REPLAY_MP_BLOCK_SIZE * I2S_BUFF_NUM];
    stm32_i2s_transfer_t sendXfer[I2S_BUFF_NUM];
    stm32_i2s_fifo_t fifo;
    uint8_t write_index;
    uint8_t write_status;
    uint8_t read_status;
    struct os_device_cb_info info;
    
    os_list_node_t list;
    os_sem_t sem;
};

static I2S_HandleTypeDef hi2s_x_ext;
   
static os_uint8_t *i2s_dma_recv;

static os_uint16_t zero_frame[2] = {0};

static os_list_node_t stm32_i2s_list = OS_LIST_INIT(stm32_i2s_list);

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    struct stm32_i2s *i2s_dev;   
    os_uint16_t i;
    os_uint8_t temp = i2s_dev->fifo.fifo_wr_pos; 
    
    os_list_for_each_entry(i2s_dev, &stm32_i2s_list, struct stm32_i2s, list)
    {
        if (hi2s == i2s_dev->hi2s)
        {
                    
            i2s_dev->fifo.fifo_wr_pos++;

            if(i2s_dev->fifo.fifo_wr_pos >= OS_AUDIO_RECORD_FIFO_COUNT)
                i2s_dev->fifo.fifo_wr_pos = 0; 

            if(i2s_dev->fifo.fifo_rd_pos == i2s_dev->fifo.fifo_wr_pos)
            {
                i2s_dev->fifo.fifo_wr_pos = temp; 
            }
            else
            {
                for(i = 0; i < OS_AUDIO_RECORD_FIFO_SIZE; i++)
                {
                    i2s_dev->fifo.rx_fifo[i2s_dev->fifo.fifo_wr_pos][i] = i2s_dma_recv[i + OS_AUDIO_RECORD_FIFO_SIZE];
                }
                os_sem_post(&i2s_dev->sem); 
            }
                      
        }
    }
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) 
{
    struct stm32_i2s *i2s_dev;
    os_uint16_t i;
    os_uint8_t temp = i2s_dev->fifo.fifo_wr_pos; 
    
    os_list_for_each_entry(i2s_dev, &stm32_i2s_list, struct stm32_i2s, list)
    {
        if (hi2s == i2s_dev->hi2s)
        {
            
            i2s_dev->fifo.fifo_wr_pos++;

            if(i2s_dev->fifo.fifo_wr_pos >= OS_AUDIO_RECORD_FIFO_COUNT)
                i2s_dev->fifo.fifo_wr_pos = 0; 

            if(i2s_dev->fifo.fifo_rd_pos == i2s_dev->fifo.fifo_wr_pos)
            {
                i2s_dev->fifo.fifo_wr_pos = temp; 
            }
            else
            {
                for(i = 0; i < OS_AUDIO_RECORD_FIFO_SIZE; i++)
                {
                    i2s_dev->fifo.rx_fifo[i2s_dev->fifo.fifo_wr_pos][i] = i2s_dma_recv[i];
                }
                os_sem_post(&i2s_dev->sem); 
            }
            
            break;
        }
    }
}


void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    struct stm32_i2s *i2s_dev;
    
    os_list_for_each_entry(i2s_dev, &stm32_i2s_list, struct stm32_i2s, list)
    {
        if (hi2s == i2s_dev->hi2s)
        {
            if (i2s_dev->sendXfer[1].data != OS_NULL)
            {
                i2s_dev->info.data = i2s_dev->sendXfer[1].data;
                i2s_dev->info.size = i2s_dev->sendXfer[1].datasize; 
                memset(&i2s_dev->data_buff[OS_AUDIO_REPLAY_MP_BLOCK_SIZE],0,i2s_dev->sendXfer[1].datasize);
                os_hw_i2s_isr(&i2s_dev->i2s, &i2s_dev->info);
                os_sem_post(&i2s_dev->sem);
                i2s_dev->sendXfer[1].data = OS_NULL;
            }

            i2s_dev->write_index = 1;
            break;
        }
    }
}


void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
    struct stm32_i2s *i2s_dev;
    
    os_list_for_each_entry(i2s_dev, &stm32_i2s_list, struct stm32_i2s, list)
    {
        if (hi2s == i2s_dev->hi2s)
        {
            if (i2s_dev->sendXfer[0].data != OS_NULL)
            {
                i2s_dev->info.data = i2s_dev->sendXfer[0].data;
                i2s_dev->info.size = i2s_dev->sendXfer[0].datasize;
                memset(&i2s_dev->data_buff[0],0,i2s_dev->sendXfer[0].datasize);
                os_hw_i2s_isr(&i2s_dev->i2s, &i2s_dev->info);
                os_sem_post(&i2s_dev->sem);
                i2s_dev->sendXfer[0].data = OS_NULL;
            }

            i2s_dev->write_index = 0;
            break;
        }
    }
}


static os_err_t stm32_i2s_dma_transmit(os_i2s_device_t *i2s, uint8_t *buff, uint32_t size)
{
    os_base_t level;
    
    HAL_StatusTypeDef status = HAL_OK;
    
    struct stm32_i2s *i2s_dev = (struct stm32_i2s *)i2s;
    
    os_sem_wait(&i2s_dev->sem, OS_WAIT_FOREVER);
    
    level = os_irq_lock();
    memcpy(&i2s_dev->data_buff[i2s_dev->write_index * OS_AUDIO_REPLAY_MP_BLOCK_SIZE], buff, size);
    i2s_dev->sendXfer[i2s_dev->write_index].data = buff;
    i2s_dev->sendXfer[i2s_dev->write_index].datasize = size;

    if ((i2s_dev->sem.count > 0) && (i2s_dev->sem.count < I2S_BUFF_NUM))
    {
        if (i2s_dev->write_index < I2S_BUFF_NUM - 1)
            i2s_dev->write_index++;
        else
            i2s_dev->write_index = 0;
    } 
    os_irq_unlock(level);

    if (i2s_dev->write_status == I2S_DMA_STATUS_STOP)
    {
        if ((i2s_dev->sendXfer[0].data) && (i2s_dev->sendXfer[1].data))
        {                       
            status = HAL_I2S_Transmit_DMA(i2s_dev->hi2s, (os_uint16_t *)i2s_dev->data_buff, OS_AUDIO_REPLAY_MP_BLOCK_SIZE);
            i2s_dev->write_status = I2S_DMA_STATUS_START;
        }
    }
        
    if (status != HAL_OK)
    {
        LOG_E(DBG_TAG,"i2s dma start transmit failed!\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_i2s_init(struct os_i2s_device *i2s)
{            
    return OS_EOK;
}

static os_err_t stm32_i2s_dma_receive(os_i2s_device_t *i2s, uint8_t *buff, uint32_t size)
{
    os_base_t level;
    
    struct stm32_i2s *i2s_dev = (struct stm32_i2s *)i2s;
    int i;
    
    os_sem_wait(&i2s_dev->sem, OS_WAIT_FOREVER);
    
    if (i2s_dev->sem.count > OS_AUDIO_RECORD_FIFO_COUNT)
    {
        i2s_dev->sem.count = OS_AUDIO_RECORD_FIFO_COUNT;
    }

    if(i2s_dev->fifo.fifo_rd_pos == i2s_dev->fifo.fifo_wr_pos)
        return OS_EEMPTY;
    
    level = os_irq_lock();
    
    i2s_dev->fifo.fifo_rd_pos++;
    if(i2s_dev->fifo.fifo_rd_pos >= OS_AUDIO_RECORD_FIFO_COUNT)
        i2s_dev->fifo.fifo_rd_pos = 0; 
    
    for(i=0;i<OS_AUDIO_RECORD_FIFO_SIZE;i++)
    {
        buff[i] = i2s_dev->fifo.rx_fifo[i2s_dev->fifo.fifo_rd_pos][i];
    }
    
    os_irq_unlock(level);
    
    return OS_EOK;  
}

static os_err_t stm32_i2s_dma_tx_enable(os_i2s_device_t *i2s, os_bool_t enable)
{
    HAL_StatusTypeDef status;

    struct stm32_i2s *i2s_dev = (struct stm32_i2s *)i2s;
    
    if (enable == OS_TRUE)
    {
        i2s_dev->write_status = I2S_DMA_STATUS_STOP;
        os_sem_init(&i2s_dev->sem, "i2s_sem", 2, 2);
        i2s_dev->sendXfer[0].data = OS_NULL;
        i2s_dev->sendXfer[0].datasize = 0;
        i2s_dev->sendXfer[1].data = OS_NULL;
        i2s_dev->sendXfer[1].datasize = 0;
        i2s_dev->write_index = 0;
        return OS_ENOSYS;
    }
    else
    {
        status = HAL_I2S_DMAStop(i2s_dev->hi2s);
        i2s_dev->write_status = I2S_DMA_STATUS_STOP;
        os_sem_deinit(&i2s_dev->sem);
    }
    
    if (status != HAL_OK)
    {
        LOG_E(DBG_TAG,"i2s dma stop failed!\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}


static os_err_t stm32_i2s_dma_rx_enable(os_i2s_device_t *i2s, os_bool_t enable)
{
    HAL_StatusTypeDef status = HAL_OK; 
    int i;
    
    struct stm32_i2s *i2s_dev = (struct stm32_i2s *)i2s; 
    
    memcpy(&hi2s_x_ext,i2s_dev->hi2s,sizeof(I2S_HandleTypeDef));
    
    if(strcmp(BSP_AUDIO_DATA_RX_BUS,"i2s2") == 0)           
    {
        hi2s_x_ext.Instance = I2S2ext;   
    }        
    else if(strcmp(BSP_AUDIO_DATA_RX_BUS,"i2s3") == 0)
    {
        hi2s_x_ext.Instance = I2S3ext;
    }
             
    if (enable == OS_TRUE)
    {        
        i2s_dev->read_status = I2S_DMA_STATUS_STOP;

        os_sem_init(&i2s_dev->sem, "i2s_sem", OS_AUDIO_RECORD_FIFO_COUNT, OS_AUDIO_RECORD_FIFO_COUNT);
        
        for(i=0;i<OS_AUDIO_RECORD_FIFO_COUNT;i++) 
        {
            i2s_dev->fifo.rx_fifo[i] = os_calloc(1, OS_AUDIO_RECORD_FIFO_SIZE);
            if (i2s_dev->fifo.rx_fifo[i] == OS_NULL)
                return OS_ENOMEM;
            memset(i2s_dev->fifo.rx_fifo[i], 0, OS_AUDIO_RECORD_FIFO_SIZE);
        } 

        i2s_dma_recv = os_calloc(1, OS_AUDIO_RECORD_FIFO_SIZE*2);

        __HAL_I2SEXT_ENABLE(i2s_dev->hi2s);    
         
        status = HAL_I2S_Transmit_DMA(i2s_dev->hi2s,zero_frame,2);
        if (status != HAL_OK)
        {
            LOG_E(DBG_TAG,"i2s dma receive clock provider start failed!\r\n");
            return OS_ERROR;
        }

        __HAL_DMA_DISABLE_IT(i2s_dev->hi2s->hdmatx,DMA_IT_TC);     
        __HAL_DMA_DISABLE_IT(i2s_dev->hi2s->hdmatx,DMA_IT_HT); 
        
        __HAL_DMA_DISABLE(i2s_dev->hi2s->hdmatx);                
        os_task_msleep(10);	       
        __HAL_DMA_ENABLE(i2s_dev->hi2s->hdmatx);                        
        
        status = HAL_I2S_Receive_DMA(&hi2s_x_ext,(os_uint16_t *)i2s_dma_recv,OS_AUDIO_RECORD_FIFO_SIZE); 
        if (status != HAL_OK)
        {
            LOG_E(DBG_TAG,"i2s dma receive start failed!\r\n");
            return OS_ERROR;
        }
        
        i2s_dev->read_status = I2S_DMA_STATUS_START;
    }
    else
    {
        __HAL_I2SEXT_DISABLE(i2s_dev->hi2s);
        HAL_DMA_Abort_IT(hi2s_x_ext.hdmarx);
        status = HAL_I2S_DMAStop(i2s_dev->hi2s);
 
        os_sem_deinit(&i2s_dev->sem);
        
        os_free(i2s_dma_recv);
        
        for(i=0;i<OS_AUDIO_RECORD_FIFO_COUNT;i++)
        {
            os_free(i2s_dev->fifo.rx_fifo[i]);
        }
        i2s_dev->read_status = I2S_DMA_STATUS_STOP;
    }
       
    if (status != HAL_OK)
    {
        LOG_E(DBG_TAG,"i2s dma stop failed!\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t stm32_i2s_set_frq(os_i2s_device_t *i2s, uint32_t frequency)
{
    HAL_StatusTypeDef status;    
    struct stm32_i2s *i2s_dev = (struct stm32_i2s *)i2s;
    
    __HAL_I2S_DISABLE(i2s_dev->hi2s);
    i2s_dev->hi2s->Init.AudioFreq = frequency;
    status = HAL_I2S_Init(i2s_dev->hi2s);
    if (status != HAL_OK)
    {
        LOG_E(DBG_TAG,"i2s init failed!\r\n");
        return OS_ERROR;
    }
    __HAL_I2S_ENABLE(i2s_dev->hi2s);
    
    return OS_EOK;
}

static os_err_t stm32_i2s_set_channel(os_i2s_device_t *i2s, uint8_t channels)
{      
    return OS_EOK;
}

static struct os_i2s_ops stm32_i2s_ops =
{
    .init           = stm32_i2s_init,
    .transimit      = stm32_i2s_dma_transmit,
    .receive        = stm32_i2s_dma_receive,
    .enable_tx      = stm32_i2s_dma_tx_enable,
    .enable_rx      = stm32_i2s_dma_rx_enable,
    .set_frq        = stm32_i2s_set_frq,
    .set_channel    = stm32_i2s_set_channel,
};

static int stm32_i2s_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    os_base_t   level;
    struct stm32_i2s *i2s_dev = os_calloc(1, sizeof(struct stm32_i2s));

    OS_ASSERT(i2s_dev);
   
    i2s_dev->hi2s = (I2S_HandleTypeDef *)dev->info;
   
    struct os_i2s_device *i2s = &i2s_dev->i2s;
    i2s->ops = &stm32_i2s_ops;
    
    level = os_irq_lock();
    os_list_add_tail(&stm32_i2s_list, &i2s_dev->list);
    os_irq_unlock(level);

    os_i2s_register(i2s, dev->name, NULL);

    LOG_D(DBG_TAG,"stm32 i2s found.\r\n");
    
    return OS_EOK;
}


OS_DRIVER_INFO stm32_i2s_driver = {
    .name   = "I2S_HandleTypeDef",
    .probe  = stm32_i2s_probe,
};

OS_DRIVER_DEFINE(stm32_i2s_driver, PREV,OS_INIT_SUBLEVEL_LOW);
