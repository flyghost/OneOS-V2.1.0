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
 * @file        os_dma.c
 *
 * @brief       this file implements dam
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-10-29    OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_memory.h>
#include <driver.h>

#ifdef DMA_HEAP_SIZE

#ifndef OS_USING_MEM_HEAP
#error:please define OS_USING_MEM_HEAP for dma heap
#endif

static struct os_memheap g_dma_heap;

void os_dma_mem_init(void)
{
    os_memheap_init(&g_dma_heap, "dma");
    os_memheap_add(&g_dma_heap, (void *)DMA_HEAP_BEGIN, (os_size_t)DMA_HEAP_SIZE, OS_MEM_ALG_DEFAULT);
}

void *os_dma_malloc_align(os_size_t size, os_size_t align)
{
    void *ptr = os_memheap_aligned_alloc(&g_dma_heap, align, size);

    if (ptr != OS_NULL)
    {
        memset(ptr, 0, size);
    }

    return ptr;
}

void os_dma_free_align(void *ptr)
{
    os_memheap_free(&g_dma_heap, ptr);
}

#else

void os_dma_mem_init(void)
{
    
}

void *os_dma_malloc_align(os_size_t size, os_size_t align)
{
    void *ptr = os_aligned_malloc(align, size);

    if (ptr != OS_NULL)
    {
        memset(ptr, 0, size);
    }

    return ptr;
}

void os_dma_free_align(void *ptr)
{
    os_free(ptr);
}

#endif

