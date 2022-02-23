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

#include <soft_dma.h>
#include <dma_ram.h>

#if !defined(OS_SOFT_DMA_SUPPORT_CIRCLE_MODE) && !defined(OS_SOFT_DMA_SUPPORT_NORMAL_MODE)
#error:OS_SOFT_DMA_SUPPORT_CIRCLE_MODE or OS_SOFT_DMA_SUPPORT_NORMAL_MODE must be define
#endif

int ring_count(dma_ring_t *ring)
{
    int head = ring->head;
    
    if (head >= ring->tail)
    {
        return head - ring->tail;
    }
    else
    {
        return head - ring->tail + ring->size;
    }   
}

int ring_space(dma_ring_t *ring)
{
    return ring->size - ring_count(ring) - 1;
}

void copy_ring_to_line(dma_ring_t *ring, os_uint8_t *line, int count)
{
    int copy_count;

    OS_ASSERT(ring->buff != OS_NULL);
    OS_ASSERT(ring->head < ring->size);
    OS_ASSERT(ring->tail < ring->size);

    OS_ASSERT(line != OS_NULL);

    OS_ASSERT(count < ring->size);

    if (count == 0)
        return;
    
    if (ring->tail + count <= ring->size)
    {
        memcpy(line, ring->buff + ring->tail, count);
        ring->tail += count;
        if (ring->tail >= ring->size)
            ring->tail = 0;
    }
    else
    {
        copy_count = ring->size - ring->tail;
        count -= copy_count;

        memcpy(line, ring->buff + ring->tail, copy_count);
        ring->tail = 0;
        
        memcpy(line + copy_count, ring->buff, count);
        ring->tail = count;
    }
}

void copy_line_to_ring(dma_ring_t *ring, os_uint8_t *line, int count)
{
    int copy_count;

    OS_ASSERT(ring->buff != OS_NULL);
    OS_ASSERT(ring->head < ring->size);
    OS_ASSERT(ring->tail < ring->size);

    OS_ASSERT(line != OS_NULL);

    OS_ASSERT(count < ring->size);
    
    if (ring->head + count <= ring->size)
    {
        memcpy(ring->buff + ring->head, line, count);
        ring->head += count;
        if (ring->head == ring->size)
            ring->head = 0;
    }
    else
    {
        copy_count = ring->size - ring->head;
        count -= copy_count;

        memcpy(ring->buff + ring->head, line, copy_count);
        ring->head = 0;
        
        memcpy(ring->buff, line + copy_count, count);
        ring->head = count;
    }
}

void copy_ring_to_ring(dma_ring_t *dst, dma_ring_t *src, int count)
{
    int copy_count;

    OS_ASSERT(dst->buff != OS_NULL);
    OS_ASSERT(dst->head < dst->size);
    OS_ASSERT(dst->tail < dst->size);
    
    OS_ASSERT(src->buff != OS_NULL);
    OS_ASSERT(src->head < src->size);
    OS_ASSERT(src->tail < src->size);

    OS_ASSERT(count < dst->size);
    OS_ASSERT(count < src->size);
    
    if (src->tail + count <= src->size)
    {
        copy_line_to_ring(dst, src->buff + src->tail, count);
        src->tail += count;
        if (src->tail == src->size)
            src->tail = 0;
    }
    else
    {
        copy_count = src->size - src->tail;
        count -= copy_count;

        copy_line_to_ring(dst, src->buff + src->tail, copy_count);
        src->tail = 0;
        
        copy_line_to_ring(dst, src->buff, count);
        src->tail = count;
    }
}

#ifdef OS_SOFT_DMA_SUPPORT_NORMAL_MODE

static os_uint8_t *soft_dma_normal_buffer_init(soft_dma_t *dma, dma_ring_t *ring)
{
    /* buffer */
    memset(&dma->buffer, 0, sizeof(dma->buffer));

    /* soft buffer */
    dma->buffer.buff = ring;

    /* hard buffer */
    int size = 128;
    
    if (!(dma->hard_info.flag & HARD_DMA_FLAG_HALF_IRQ))
    {
        size = max(size, (os_uint64_t)600 / dma->hard_info.data_timeout + 1);
    }

    size = min(size, ring->size);
    size = min(size, dma->hard_info.max_size);
    
    dma->buffer.hard_buffs[0].size = ring->size;
    dma->buffer.hard_buffs[0].buff = ring->buff;

    dma->buffer.hard_buffs[1].size = size;
    dma->buffer.hard_buffs[2].size = size;

#ifdef OS_USING_DMA_RAM
    dma->buffer.hard_buffs[1].buff = os_dma_malloc_align(size * 2, 1);
#else
    dma->buffer.hard_buffs[1].buff = os_calloc(1, size * 2);
#endif
    
    dma->buffer.hard_buffs[2].buff = dma->buffer.hard_buffs[1].buff + size;

    dma->buffer.hard_buff = &dma->buffer.hard_buffs[0];

    OS_ASSERT(dma->buffer.hard_buffs[0].buff != OS_NULL);
    OS_ASSERT(dma->buffer.hard_buffs[1].buff != OS_NULL);
    OS_ASSERT(dma->buffer.hard_buffs[2].buff != OS_NULL);

    return dma->buffer.hard_buff->buff;
}

static os_err_t soft_dma_normal_updata_buffer(soft_dma_t *dma, os_uint32_t head)
{
    os_base_t level = os_irq_lock();

    os_uint32_t last_hard_head = dma->buffer.hard_buff->head;
    
    if (dma->buffer.hard_buff == &dma->buffer.hard_buffs[0])
    {            
        dma->buffer.hard_buff->head = head;
    
        dma->buffer.hard_buff = &dma->buffer.hard_buffs[1];
        dma->buffer.hard_buff->head = 0;
        dma->buffer.hard_buff->tail = 0;
        
        head = dma->ops.dma_stop(dma);
        dma->ops.dma_start(dma, dma->buffer.hard_buff->buff, dma->buffer.hard_buff->size);

        OS_ASSERT(head >= last_hard_head);
        OS_ASSERT(head <= dma->buffer.hard_buffs[0].size);
        dma->buffer.buff->head += head - last_hard_head;
        if (dma->buffer.buff->head >= dma->buffer.buff->size)
            dma->buffer.buff->head = 0;
    }
    else if (head > 0)
    {
        dma_ring_t *hard_buff = dma->buffer.hard_buff;
        
        head = dma->ops.dma_stop(dma);

        os_uint32_t buff_head = dma->buffer.buff->head + head;

        if (buff_head >= dma->buffer.buff->size)
        {
            buff_head -= dma->buffer.buff->size;
        }

        if (dma->buffer.buff->size - buff_head > dma->buffer.hard_buffs[1].size)
        {
            dma->buffer.hard_buff = &dma->buffer.hard_buffs[0];

            dma->buffer.hard_buff->buff = dma->buffer.buff->buff + buff_head;
            dma->buffer.hard_buff->size = dma->buffer.buff->size - buff_head;
        }
        else
        {
            if (dma->buffer.hard_buff == &dma->buffer.hard_buffs[1])
            {
                dma->buffer.hard_buff = &dma->buffer.hard_buffs[2];
            }
            else if (dma->buffer.hard_buff == &dma->buffer.hard_buffs[2])
            {
                dma->buffer.hard_buff = &dma->buffer.hard_buffs[1];
            }
            else
            {
                OS_ASSERT(OS_FALSE);
            }
        }

        dma->buffer.hard_buff->head  = 0;
        dma->buffer.hard_buff->tail  = 0;

        dma->ops.dma_start(dma, dma->buffer.hard_buff->buff, dma->buffer.hard_buff->size);

        if (head < hard_buff->size)
        {
            hard_buff->head = head;
            copy_ring_to_ring(dma->buffer.buff, hard_buff, head);
            OS_ASSERT(dma->buffer.buff->head < dma->buffer.buff->size);
        }
        else if (head == hard_buff->size)
        {
            os_kprintf("drop some data\r\n");
        }
        else
        {
            OS_ASSERT(OS_FALSE);
        }
    }

    os_irq_unlock(level);
    
    return OS_EOK;
}

#endif

#ifdef OS_SOFT_DMA_SUPPORT_CIRCLE_MODE

static os_uint8_t *soft_dma_circular_buffer_init(soft_dma_t *dma, dma_ring_t *ring)
{
    /* buffer */
    memset(&dma->buffer, 0, sizeof(dma->buffer));

    /* soft buffer */
    dma->buffer.buff = ring;

    /* hard buffer */
    dma->buffer.hard_buff = &dma->buffer.hard_buffs[0];
    
    if (ring->size <= dma->hard_info.max_size)
    {
        dma->buffer.hard_buff->size = ring->size;
        dma->buffer.hard_buff->buff = ring->buff;
    }
    else
    {
        dma->buffer.hard_buff->size = dma->hard_info.max_size;
        
#ifdef OS_USING_DMA_RAM
        dma->buffer.hard_buff->buff = os_dma_malloc_align(dma->hard_info.max_size, 1);
#else
        dma->buffer.hard_buff->buff = os_calloc(1, dma->hard_info.max_size);
#endif
    }

    OS_ASSERT(dma->buffer.hard_buff->buff != OS_NULL);
    
    return dma->buffer.hard_buff->buff;
}

static os_err_t soft_dma_circular_updata_buffer(soft_dma_t *dma, os_uint32_t head)
{
    dma->buffer.hard_buff->head = head;
    
    if (dma->buffer.hard_buff->head == dma->buffer.hard_buff->size)
        dma->buffer.hard_buff->head = 0;

    if (dma->buffer.buff->size <= dma->hard_info.max_size)
    {
        dma->buffer.buff->head = head;
    }
    else
    {
        copy_ring_to_ring(dma->buffer.buff, dma->buffer.hard_buff, ring_count(dma->buffer.hard_buff));
    }

    return OS_EOK;
}

#endif

static os_uint8_t *soft_dma_buffer_init(soft_dma_t *dma, dma_ring_t *ring)
{
#ifdef OS_SOFT_DMA_SUPPORT_CIRCLE_MODE
    if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        return soft_dma_circular_buffer_init(dma, ring);
    }
#endif

#ifdef OS_SOFT_DMA_SUPPORT_NORMAL_MODE
    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
        return soft_dma_normal_buffer_init(dma, ring);
    }
#endif
    
    return OS_NULL;
}

static os_err_t soft_dma_buffer_deinit(soft_dma_t *dma)
{
#ifdef OS_SOFT_DMA_SUPPORT_CIRCLE_MODE
    if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        if (dma->buffer.hard_buff->buff != dma->buffer.buff->buff)
        {
#ifdef OS_USING_DMA_RAM
            os_dma_free_align(dma->buffer.hard_buff->buff);
#else
            os_free(dma->buffer.hard_buff->buff);
#endif
        }
    
        memset(&dma->buffer, 0, sizeof(dma->buffer));
    }
#endif

#ifdef OS_SOFT_DMA_SUPPORT_NORMAL_MODE
    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
#ifdef OS_USING_DMA_RAM
        os_dma_free_align(dma->buffer.hard_buffs[1].buff);
#else
        os_free(dma->buffer.hard_buffs[1].buff);
#endif
        
        memset(&dma->buffer, 0, sizeof(dma->buffer));
    }
#endif
    
    return OS_EOK;
}

static os_err_t soft_dma_updata_buffer(soft_dma_t *dma, os_uint32_t head)
{
    os_uint32_t last_head = dma->buffer.buff->head;

#ifdef OS_SOFT_DMA_SUPPORT_CIRCLE_MODE
    if (dma->hard_info.mode == HARD_DMA_MODE_CIRCULAR)
    {
        soft_dma_circular_updata_buffer(dma, head);
    }
#endif

#ifdef OS_SOFT_DMA_SUPPORT_NORMAL_MODE
    if (dma->hard_info.mode == HARD_DMA_MODE_NORMAL)
    {
        soft_dma_normal_updata_buffer(dma, head);
    }
#endif

    dma->hard_info.last_head  = dma->buffer.hard_buff->head;
    dma->hard_info.last_stamp = dma->buffer.hard_buff->head;

    /* update irq status */
    if (last_head < dma->buffer.buff->size / 2)
    {
        if (head >= dma->buffer.buff->size / 2)
        {
            dma->status |= SOFT_DMA_HALF;
        }
    }
    else
    {
        if (head < last_head)
        {
            dma->status |= SOFT_DMA_FULL;
        }
    }
    
    return OS_EOK;
}


#ifdef OS_SOFT_DMA_SUPPORT_SIMUL_TIMEOUT

static void soft_dma_timer_callback(void *parameter)
{
    soft_dma_t *dma = (soft_dma_t *)parameter;

    os_uint32_t last_head = dma->hard_info.last_head;
    os_uint32_t head = dma->ops.get_index(dma);

    dma->hard_info.last_head = head;

    if (last_head < dma->buffer.hard_buff->size / 2)
    {
        if (head >= dma->buffer.hard_buff->size / 2)
        {
            if (!(dma->hard_info.flag & HARD_DMA_FLAG_HALF_IRQ))
            {
                soft_dma_half_irq(dma);
            }
        }
    }
    else
    {
        if (head < last_head)
        {
            if (!(dma->hard_info.flag & HARD_DMA_FLAG_FULL_IRQ))
            {
                soft_dma_full_irq(dma);
            }
        }
    }

    if (last_head != dma->hard_info.last_stamp && last_head == head)
    {
        soft_dma_timeout_irq(dma);
    }
}

#ifdef OS_USING_HRTIMER
static void soft_dma_timer_start(soft_dma_t *dma, os_uint32_t period_us)
{
    OS_ASSERT(os_hrtimer_stoped(&dma->timer));

    dma->timer.next_nsec    = period_us * (os_uint64_t)1000;
    dma->timer.period_nsec  = period_us * (os_uint64_t)1000;
    dma->timer.timeout_func = soft_dma_timer_callback;
    dma->timer.parameter    = dma;
    
    os_hrtimer_start(&dma->timer);

    dma->timer_inited = OS_TRUE;
}

static void soft_dma_timer_stop(soft_dma_t *dma)
{
    if (dma->timer_inited == OS_FALSE)
        return;

    while (!os_hrtimer_stoped(&dma->timer))
        os_hrtimer_stop(&dma->timer);

    dma->timer_inited = OS_FALSE;
}
#else
static void soft_dma_timer_start(soft_dma_t *dma, os_uint32_t period_us)
{
    int tick = (os_uint64_t)period_us * OS_TICK_PER_SECOND / 1000000;

    os_timer_init(&dma->timer, 
                  OS_NULL,
                  soft_dma_timer_callback, 
                  dma,
                  tick > 0 ? tick : 1,
                  OS_TIMER_FLAG_PERIODIC);
    
    os_timer_start(&dma->timer);
    dma->timer_inited = OS_TRUE;
}

static void soft_dma_timer_stop(soft_dma_t *dma)
{
    if (dma->timer_inited == OS_FALSE)
        return;

    while (os_timer_stop(&dma->timer) != OS_EOK);

    OS_ASSERT(!os_timer_is_active(&dma->timer));

    os_timer_deinit(&dma->timer);

    dma->timer_inited = OS_FALSE;
}
#endif

static void soft_dma_timer_init(soft_dma_t *dma)
{
    os_uint32_t count = OS_UINT32_MAX / 2;
    os_uint32_t timeout_us = 0;

    if (!(dma->hard_info.flag & HARD_DMA_FLAG_HALF_IRQ)
     || !(dma->hard_info.flag & HARD_DMA_FLAG_FULL_IRQ))
    {
        count = dma->buffer.hard_buff->size / 6;
    }

    if (!(dma->hard_info.flag & HARD_DMA_FLAG_TIMEOUT_IRQ))
    {
        count = min(10, count + 1);
    }

    if (count == (OS_UINT32_MAX / 2))
        return;

    timeout_us = dma->hard_info.data_timeout * count;

    /* warnning 100 us low limit */
    if (timeout_us < 100)
    {
        timeout_us = 100;
    }

    soft_dma_timer_start(dma, timeout_us);
}

static void soft_dma_timer_deinit(soft_dma_t *dma)
{
    soft_dma_timer_stop(dma);
}

#else
static void soft_dma_timer_init(soft_dma_t *dma){}
static void soft_dma_timer_deinit(soft_dma_t *dma){}
#endif

#if defined(OS_SOFT_DMA_SUPPORT_SIMUL_TIMEOUT) && defined(OS_USING_TICKLESS_LPMGR)

#include <lpmgr.h>

static int soft_dma_timer_suspend(void *priv, os_uint8_t mode)
{
    soft_dma_t *dma = (soft_dma_t *)priv;
    soft_dma_timer_deinit(dma);
    return OS_EOK;
}

static void soft_dma_timer_resume(void *priv, os_uint8_t mode)
{
    soft_dma_t *dma = (soft_dma_t *)priv;
    soft_dma_timer_init(dma);
    
    soft_dma_timer_callback(dma);
    soft_dma_timer_callback(dma);
}

static const struct os_lpmgr_device_ops soft_dma_lpmgr_ops =
{
    soft_dma_timer_suspend,
    soft_dma_timer_resume,
};

#endif

os_err_t soft_dma_init(soft_dma_t *dma)
{
    OS_ASSERT(dma->ops.get_index != OS_NULL);
    OS_ASSERT(dma->ops.dma_start != OS_NULL);
    OS_ASSERT(dma->ops.dma_stop != OS_NULL);
    OS_ASSERT(dma->cbs.dma_half_callback != OS_NULL);
    OS_ASSERT(dma->cbs.dma_full_callback != OS_NULL);
    OS_ASSERT(dma->cbs.dma_timeout_callback != OS_NULL);

#ifndef OS_SOFT_DMA_SUPPORT_NORMAL_MODE
    OS_ASSERT(dma->hard_info.mode != HARD_DMA_MODE_NORMAL);
#endif

#ifndef OS_SOFT_DMA_SUPPORT_CIRCLE_MODE
    OS_ASSERT(dma->hard_info.mode != HARD_DMA_MODE_CIRCULAR);
#endif

    if (dma->ops.dma_init != OS_NULL)
        dma->ops.dma_init(dma);

#if defined(OS_SOFT_DMA_SUPPORT_SIMUL_TIMEOUT) && defined(OS_USING_TICKLESS_LPMGR)
    os_lpmgr_device_register(dma, &soft_dma_lpmgr_ops);
#endif
    return OS_EOK;
}

os_err_t soft_dma_deinit(soft_dma_t *dma)
{
#if defined(OS_SOFT_DMA_SUPPORT_SIMUL_TIMEOUT) && defined(OS_USING_TICKLESS_LPMGR)
    os_lpmgr_device_unregister(dma, &soft_dma_lpmgr_ops);
#endif
    return OS_EOK;
}

os_err_t soft_dma_start(soft_dma_t *dma, dma_ring_t *ring)
{
    OS_ASSERT((dma->status & SOFT_DMA_EN) == 0);

    soft_dma_buffer_init(dma, ring);

    OS_ASSERT(dma->buffer.hard_buff->buff != OS_NULL);

    dma->hard_info.last_head = 0;
    dma->hard_info.last_stamp = 0;

    dma->ops.dma_start(dma, dma->buffer.hard_buff->buff, dma->buffer.hard_buff->size);

    soft_dma_timer_init(dma);

    dma->status |= SOFT_DMA_EN;

    return OS_EOK;
}

os_err_t soft_dma_stop(soft_dma_t *dma)
{
    if ((dma->status & SOFT_DMA_EN) == 0)
        return OS_EOK;

    soft_dma_timer_deinit(dma);

    soft_dma_buffer_deinit(dma);
    
    dma->ops.dma_stop(dma);

    dma->hard_info.last_head = 0;
    dma->hard_info.last_stamp = 0;

    dma->status &= ~SOFT_DMA_EN;

    return OS_EOK;
}

static void soft_dma_irq_callback(soft_dma_t *dma)
{
    int count = ring_count(dma->buffer.buff);

    if (count <= 0)
    {
        dma->status &= ~(SOFT_DMA_HALF | SOFT_DMA_FULL | SOFT_DMA_TIMEOUT);
        return;
    }

    if (dma->irq_mask & dma->status & SOFT_DMA_HALF)
    {
        dma->cbs.dma_half_callback(dma);
        dma->status &= ~SOFT_DMA_HALF;
    }

    if (dma->irq_mask & dma->status & SOFT_DMA_FULL)
    {
        dma->cbs.dma_full_callback(dma);
        dma->status &= ~SOFT_DMA_FULL;
    }

    if (dma->irq_mask & dma->status & SOFT_DMA_TIMEOUT)
    {
        dma->cbs.dma_timeout_callback(dma);
        dma->status &= ~SOFT_DMA_TIMEOUT;
    }
}

static void soft_dma_irq(soft_dma_t *dma)
{
    os_uint32_t head = dma->ops.get_index(dma);

    soft_dma_updata_buffer(dma, head);

    soft_dma_irq_callback(dma);
}

/* irq handle */
void soft_dma_half_irq(soft_dma_t *dma)
{
    soft_dma_irq(dma);
}

void soft_dma_full_irq(soft_dma_t *dma)
{
    soft_dma_irq(dma);
}

void soft_dma_timeout_irq(soft_dma_t *dma)
{
    dma->status |= SOFT_DMA_TIMEOUT;
    soft_dma_irq(dma);
}

/* irq mask */
void soft_dma_irq_enable(soft_dma_t *dma, os_bool_t enable)
{
    if (enable == OS_TRUE)
    {
        dma->irq_mask |= SOFT_DMA_HALF | SOFT_DMA_FULL | SOFT_DMA_TIMEOUT;
    }
    else
    {
        dma->irq_mask &= ~(SOFT_DMA_HALF | SOFT_DMA_FULL | SOFT_DMA_TIMEOUT);
    }
}

