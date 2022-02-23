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

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "drv.sai"
#include <drv_log.h>

#define SAI_TX_FIFO_SIZE         (2048)

struct stm32_sai
{
    os_device_sai_t sai;

    os_list_node_t list;
};

static os_list_node_t stm32_sai_list = OS_LIST_INIT(stm32_sai_list);

void stm32_sai_dma_transmit(os_device_sai_t *sai, uint8_t *pData, uint16_t Size)
{
    
    HAL_SAI_Transmit_DMA(sai->hsai, pData, Size);
    
}

void stm32_sai_dma_stop(os_device_sai_t *sai)
{
    OS_ASSERT(sai);
    
    HAL_SAI_DMAStop(sai->hsai);
}

void stm32_sai_frequency_set(os_device_sai_t *sai, uint32_t frequency)
{    
    OS_ASSERT(sai);

    /* Disable SAI peripheral to allow access to SAI internal registers */
    __HAL_SAI_DISABLE(sai->hsai);
    /* Update the SAI audio frequency configuration */
    sai->hsai->Init.AudioFrequency         = frequency;
    HAL_SAI_Init(sai->hsai);
    /* Enable SAI peripheral to generate MCLK */
    __HAL_SAI_ENABLE(sai->hsai);
}

void stm32_sai_channel_set(os_device_sai_t *sai, uint8_t channels)
{    

    OS_ASSERT(sai); 
    
    if (channels == 1)
    {
        sai->hsai->Init.MonoStereoMode         = SAI_MONOMODE;
    }
    else
    {
        sai->hsai->Init.MonoStereoMode         = SAI_STEREOMODE;
    }

    __HAL_SAI_DISABLE(sai->hsai);
    HAL_SAI_Init(sai->hsai);
    __HAL_SAI_ENABLE(sai->hsai);
    
}

void stm32_sai_info(os_device_sai_t *sai, uint8_t *tx_fifo,struct os_data_queue *queue, struct os_completion *cmp, os_uint8_t *event)
{ 
    struct os_sai_replay *replay = (struct os_sai_replay *) os_calloc(1, sizeof(struct os_sai_replay));
    
    memset(replay, 0, sizeof(struct os_sai_replay));
    replay->buf_info.buffer = tx_fifo;  
    replay->buf_info.total_size  = SAI_TX_FIFO_SIZE;
    replay->buf_info.block_size  = SAI_TX_FIFO_SIZE / 2;
    replay->buf_info.block_count = 2;
    
    replay->queue = queue;
    replay->cmp = cmp;
    replay->event = event;
    
    sai->replay = replay;
}

void stm32_sai_buffer_tx_complete(os_device_sai_t *sai)
{
    os_err_t result = OS_EOK;
    os_uint8_t *data;
    os_size_t dst_size, src_size;
    os_uint16_t remain_bytes = 0, index = 0;
    struct os_sai_buf_info *buf_info;

    OS_ASSERT(sai != OS_NULL);

    buf_info = &sai->replay->buf_info;
    dst_size = buf_info->block_size;

    /* check repaly queue is empty */
    if (os_data_queue_peak(sai->replay->queue, (const void **)&data, &src_size) != OS_EOK)
    {
        /* ack stop event */
        if (*sai->replay->event & SAI_REPLAY_EVT_STOP)
            os_completion_done(sai->replay->cmp);
        /* send zero frames */
        memset(&buf_info->buffer[sai->replay->pos], 0, dst_size);

        sai->replay->pos += dst_size;
        sai->replay->pos %= buf_info->total_size;
    }
    else
    {
        memset(&buf_info->buffer[sai->replay->pos], 0, dst_size);

        /* copy data from memory pool to hardware device fifo */
        while (index < dst_size)
        {
            result = os_data_queue_peak(sai->replay->queue, (const void **)&data, &src_size);
            if (result != OS_EOK)
            {
                LOG_EXT_D("under run %d, remain %d", sai->replay->pos, remain_bytes);
                sai->replay->pos -= remain_bytes;
                sai->replay->pos += dst_size;
                sai->replay->pos %= buf_info->total_size;
                sai->replay->read_index = 0;
                result = OS_EEMPTY;
                break;
            }

            remain_bytes = MIN((dst_size - index), (src_size - sai->replay->read_index));
            memcpy(&buf_info->buffer[sai->replay->pos],
                   &data[sai->replay->read_index], remain_bytes);

            index += remain_bytes;
            sai->replay->read_index += remain_bytes;
            sai->replay->pos += remain_bytes;
            sai->replay->pos %= buf_info->total_size;

            if (sai->replay->read_index == src_size)
            {
                /* free memory */
                sai->replay->read_index = 0;
                os_data_queue_pop(sai->replay->queue, (const void **)&data, &src_size, OS_IPC_WAITING_NO);
                os_mp_free(data);
            }
        }
    }
}


void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{    
    struct stm32_sai *sai_dev;

    os_list_for_each_entry(sai_dev, &stm32_sai_list, struct stm32_sai, list)
    {
        if (hsai == sai_dev->sai.hsai)
        {
            stm32_sai_buffer_tx_complete(&sai_dev->sai); 
        }
    }
}

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    struct stm32_sai *sai_dev;

    os_list_for_each_entry(sai_dev, &stm32_sai_list, struct stm32_sai, list)
    {
        if (hsai == sai_dev->sai.hsai)
        {
            stm32_sai_buffer_tx_complete(&sai_dev->sai); 
        }
    }
}

static struct os_device_sai_ops ops =
{
    .transimit       = stm32_sai_dma_transmit,
    .stop            = stm32_sai_dma_stop,
    .frequency_set   = stm32_sai_frequency_set,
    .channel_set     = stm32_sai_channel_set,
    .sai_info        = stm32_sai_info,
};

static int stm32_sai_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{    
    os_base_t   level;
    struct stm32_sai *sai_dev = os_calloc(1, sizeof(struct stm32_sai));

    OS_ASSERT(sai_dev);
   
    sai_dev->sai.hsai = (SAI_HandleTypeDef *)dev->info;

    sai_dev->sai.ops   = &ops;
    
    level = os_hw_interrupt_disable();
    os_list_add_tail(&stm32_sai_list, &sai_dev->list);
    os_hw_interrupt_enable(level);
    
    os_sai_register(dev->name, &sai_dev->sai); 

    os_kprintf("stm32 sai found.\r\n");
    
    return OS_EOK;
}


OS_DRIVER_INFO stm32_sai_driver = {
    .name   = "SAI_HandleTypeDef",
    .probe  = stm32_sai_probe,
};

OS_DRIVER_DEFINE(stm32_sai_driver, "1");
