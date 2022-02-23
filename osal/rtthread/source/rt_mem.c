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
 * @file        rt_mem.c
 *
 * @brief       Implementation of RT-Thread adaper memory management function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-19   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include "os_assert.h"
#include "rtthread.h"
#include "os_memory.h"
#include "os_errno.h"
#include "os_list.h"

#ifdef RT_USING_MEMPOOL

static os_list_node_t gs_os_mp_adapt_resource_list_head = OS_LIST_INIT(gs_os_mp_adapt_resource_list_head);

rt_err_t rt_mp_init(struct rt_mempool *mp,
                    const char        *name,
                    void              *start,
                    rt_size_t          size,
                    rt_size_t          block_size)
{
    os_err_t ret;

    OS_ASSERT(mp);
    OS_ASSERT(start);

    ret = os_mp_init(&mp->os_mp, name, start, size, block_size);
    if(OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    mp->is_static = OS_TRUE;

    mp->start_addr = start;
    mp->size = size;

    os_schedule_lock();
    os_list_add_tail(&gs_os_mp_adapt_resource_list_head, &mp->resource_node);
    os_schedule_unlock();

    return RT_EOK;
}

rt_err_t rt_mp_detach(struct rt_mempool *mp)
{
    os_err_t ret;

    OS_ASSERT(mp);
    OS_ASSERT(OS_TRUE == mp->is_static);

    os_list_del(&mp->resource_node);

    ret = os_mp_deinit(&mp->os_mp);
    if(OS_EOK != ret)
    {
           return -RT_ERROR;
    }

    return RT_EOK;
}

#ifdef RT_USING_HEAP
rt_mp_t rt_mp_create(const char *name,
                     rt_size_t   block_count,
                     rt_size_t   block_size)
{
    void       *block_start;
    rt_mp_t     mp;
    os_err_t    ret;
    os_size_t   pool_size;

    OS_ASSERT(block_count);

    mp = (rt_mp_t)os_malloc(sizeof(struct rt_mempool));

    if (OS_NULL == mp)
    {
        return RT_NULL;
    }

    /* Calculate the size of the memory pool according to block_size and block_count */
    pool_size   = OS_MEMPOOL_SIZE(block_count, block_size);

    block_start = os_malloc(pool_size);

    if (OS_NULL == block_start)
    {
        os_free(mp);
        return RT_NULL;
    }

    ret = os_mp_init(&mp->os_mp, name, block_start, pool_size, block_size);

    if (OS_EOK != ret)
    {
        os_free(mp);
        os_free(block_start);
        return RT_NULL;
    }

    mp->is_static = OS_FALSE;

    mp->start_addr = block_start;
    mp->size = pool_size;

    os_schedule_lock();
    os_list_add_tail(&gs_os_mp_adapt_resource_list_head, &mp->resource_node);
    os_schedule_unlock();

    return mp;
}

rt_err_t rt_mp_delete(rt_mp_t mp)
{
    os_err_t    ret;

    OS_ASSERT(mp);
    OS_ASSERT(OS_FALSE == mp->is_static);

    os_list_del(&mp->resource_node);

    ret = os_mp_deinit(&mp->os_mp);

    os_free(mp->start_addr);
    os_free(mp);

    if(OS_EOK != ret)
    {
           return -RT_ERROR;
    }

    return RT_EOK;
}
#endif /* RT_USING_HEAP */

void *rt_mp_alloc(rt_mp_t mp, rt_int32_t time)
{
    return os_mp_alloc(&mp->os_mp,time);
}

void rt_mp_free(void *block)
{
    os_list_node_t *pos;
    rt_mp_t iter_mp;

    os_schedule_lock();
    os_list_for_each(pos, &gs_os_mp_adapt_resource_list_head)
    {
        iter_mp = os_list_entry(pos, struct rt_mempool, resource_node);
        if ((block >= iter_mp->start_addr) && ((char *)block < ((char *)iter_mp->start_addr + iter_mp->size)))
        {
            os_schedule_unlock();
            os_mp_free(&iter_mp->os_mp, block);
            return;
        }
    }
    os_schedule_unlock();
    OS_ASSERT(0);
}
#endif

#ifdef RT_USING_HEAP
void rt_system_heap_init(void *begin_addr, void *end_addr)
{
    os_sys_heap_init();
    os_sys_heap_add(begin_addr, (os_size_t)end_addr - (os_size_t)begin_addr, OS_MEM_ALG_DEFAULT);
}

void *rt_malloc(rt_size_t nbytes)
{
    return os_malloc(nbytes);
}

void rt_free(void *ptr)
{
    if (RT_NULL == ptr)
        return;
    os_free(ptr);
}

void *rt_realloc(void *ptr, rt_size_t nbytes)
{
    return os_realloc(ptr, nbytes);
}

void *rt_calloc(rt_size_t count, rt_size_t size)
{
    return os_calloc(count, size);
}

void *rt_malloc_align(rt_size_t size, rt_size_t align)
{
    return os_aligned_malloc(align, size);
}

void rt_free_align(void *ptr)
{
    if (RT_NULL == ptr)
        return;
    os_free(ptr);
}

void rt_memory_info(rt_uint32_t *total,
                    rt_uint32_t *used,
                    rt_uint32_t *max_used)
{
    os_meminfo_t meminfo;

    os_memory_info(&meminfo);

    *total = meminfo.mem_total;
    *used = meminfo.mem_used;
    *max_used = meminfo.mem_maxused;
}
#endif /* RT_USING_HEAP */

#ifdef RT_USING_MEMHEAP
static os_list_node_t gs_os_memheap_adapt_resource_list_head = OS_LIST_INIT(gs_os_memheap_adapt_resource_list_head);

rt_err_t rt_memheap_init(struct rt_memheap *memheap,
                         const char        *name,
                         void              *start_addr,
                         rt_size_t          size)
{
    os_err_t ret;

    OS_ASSERT(memheap);

    ret = os_memheap_init(&memheap->os_memheap, name);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    ret = os_memheap_add(&memheap->os_memheap, start_addr, size, OS_MEM_ALG_DEFAULT);
    if (OS_EOK != ret)
    {
        return -RT_ERROR;
    }

    memheap->start_addr = start_addr;
    memheap->size = size;
    os_schedule_lock();
    os_list_add_tail(&gs_os_memheap_adapt_resource_list_head, &memheap->resource_node);
    os_schedule_unlock();

    return RT_EOK;
}

rt_err_t rt_memheap_detach(struct rt_memheap *heap)
{
    OS_ASSERT(heap);

    os_list_del(&heap->resource_node);
    os_memheap_deinit(&heap->os_memheap);

    return RT_EOK;
}

void *rt_memheap_alloc(struct rt_memheap *heap, rt_size_t size)
{
    OS_ASSERT(heap);

    return os_memheap_alloc(&heap->os_memheap, size);
}

void *rt_memheap_realloc(struct rt_memheap *heap,
                         void              *ptr,
                         rt_size_t          newsize)
{
    OS_ASSERT(heap);

    return os_memheap_realloc(&heap->os_memheap, ptr, (os_size_t)newsize);
}

void rt_memheap_free(void *ptr)
{
    os_list_node_t *pos;
    struct rt_memheap *iter_memheap;

    os_schedule_lock();
    os_list_for_each(pos, &gs_os_memheap_adapt_resource_list_head)
    {
        iter_memheap = os_list_entry(pos, struct rt_memheap, resource_node);
        if ((ptr >= iter_memheap->start_addr) && ((char *)ptr < ((char *)iter_memheap->start_addr + iter_memheap->size)))
        {
            os_schedule_unlock();
            os_memheap_free(&iter_memheap->os_memheap, ptr);
            return;
        }
    }
    os_schedule_unlock();
    OS_ASSERT(0);
}
#endif /* RT_USING_MEMHEAP */
