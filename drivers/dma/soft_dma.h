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
 * @file        uart_dma.c
 *
 * @brief       This file provides uart dma functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _DRIVERS_SOFT_DMA_H_
#define _DRIVERS_SOFT_DMA_H_

#include "string.h"
#include <os_stddef.h>
#include <os_timer.h>
#include <os_memory.h>
#include <driver.h>

#ifdef OS_SOFT_DMA_SUPPORT_SIMUL_TIMEOUT
#include <timer/hrtimer.h>
#endif

#define HARD_DMA_MODE_NORMAL        (0)
#define HARD_DMA_MODE_CIRCULAR      (1)

#define HARD_DMA_FLAG_HALF_IRQ      (1 << 1)
#define HARD_DMA_FLAG_FULL_IRQ      (1 << 2)
#define HARD_DMA_FLAG_TIMEOUT_IRQ   (1 << 3)

#define SOFT_DMA_EN                 (1 << 0)
#define SOFT_DMA_HALF               (1 << 1)
#define SOFT_DMA_FULL               (1 << 2)
#define SOFT_DMA_TIMEOUT            (1 << 3)

typedef struct soft_dma soft_dma_t;

typedef struct hard_info
{
    os_uint32_t mode;               /* HARD_DMA_MODE_CIRCULAR */
    os_uint32_t max_size;           /* 64 * 1024 */
    
    os_uint32_t flag;               /* HARD_DMA_FLAG_HALF_IRQ */
    os_uint32_t data_timeout;       /* 90 us/Byte */
    os_uint32_t last_stamp;
    os_uint32_t last_head;
}hard_dma_info_t;

typedef struct hard_dma_ops
{
    os_uint32_t (*get_index)(soft_dma_t *dma);
    os_err_t    (*dma_init)(soft_dma_t *dma);
    os_err_t    (*dma_start)(soft_dma_t *dma, void *buff, os_uint32_t size);
    os_uint32_t (*dma_stop)(soft_dma_t *dma);
}hard_dma_ops_t;

typedef struct soft_dma_callbacks
{
    void (*dma_half_callback)(soft_dma_t *dma);
    void (*dma_full_callback)(soft_dma_t *dma);
    void (*dma_timeout_callback)(soft_dma_t *dma);
}soft_dma_callbacks_t;

typedef struct dma_ring
{
    os_uint8_t *buff;
    int size;
    int head;
    int tail;
}dma_ring_t;

typedef struct soft_dma_rx_buffer
{
    os_uint32_t flag;
    dma_ring_t *buff;
    dma_ring_t *hard_buff;
    dma_ring_t  hard_buffs[3];
}soft_dma_rx_buffer_t;

struct soft_dma
{
    hard_dma_info_t         hard_info;
    hard_dma_ops_t          ops;
    soft_dma_callbacks_t    cbs;

#ifdef OS_SOFT_DMA_SUPPORT_SIMUL_TIMEOUT
    os_bool_t               timer_inited;
#ifdef OS_USING_HRTIMER
    os_hrtimer_t            timer;
#else
    os_timer_t              timer;
#endif
#endif
    
    soft_dma_rx_buffer_t    buffer;
    
    os_uint32_t             status;
    os_uint32_t             irq_mask;
};

os_err_t soft_dma_init(soft_dma_t *dma);
os_err_t soft_dma_deinit(soft_dma_t *dma);
os_err_t soft_dma_start(soft_dma_t *dma, dma_ring_t *ring);
os_err_t soft_dma_stop(soft_dma_t *dma);

void soft_dma_irq_enable(soft_dma_t *dma, os_bool_t enable);
void soft_dma_half_irq(soft_dma_t *dma);
void soft_dma_full_irq(soft_dma_t *dma);
void soft_dma_timeout_irq(soft_dma_t *dma);

int ring_count(dma_ring_t *ring);
int ring_space(dma_ring_t *ring);
void copy_ring_to_line(dma_ring_t *ring, os_uint8_t *line, int count);
void copy_line_to_ring(dma_ring_t *ring, os_uint8_t *line, int count);
void copy_ring_to_ring(dma_ring_t *dst, dma_ring_t *src, int count);

#endif // _DRIVERS_SOFT_DMA_H_
