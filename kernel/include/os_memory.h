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
 * @file        os_memory.h
 *
 * @brief       Header file for memory management interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS team      First Version
 ***********************************************************************************************************************
 */

#ifndef __OS_MEMORY_H__
#define __OS_MEMORY_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_sem.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_MEM_HEAP

#ifndef OS_USING_SEMAPHORE
#error "Macro OS_USING_SEMAPHORE must be set."
#endif

#if !defined(OS_USING_ALG_FIRSTFIT) && !defined(OS_USING_ALG_BUDDY)
#error "ERROR! At least one memory algorithm must be choose."
#endif

enum os_mem_alg
{
#ifdef OS_USING_ALG_FIRSTFIT
    OS_MEM_ALG_FIRSTFIT,
#endif
#ifdef OS_USING_ALG_BUDDY
    OS_MEM_ALG_BUDDY,
#endif
    OS_MEM_ALG_DEFAULT          /*Use default memory algorithm */
};

struct heap_mem
{
    void    *(*k_alloc)         (struct heap_mem *h_mem, os_size_t size);
    void    *(*k_aligned_alloc) (struct heap_mem *h_mem, os_size_t align, os_size_t size);
    void     (*k_free)          (struct heap_mem *h_mem, void *mem);
    void    *(*k_realloc)       (struct heap_mem *h_mem, void *mem, os_size_t newsize);
    void     (*k_deinit)        (struct heap_mem *h_mem);
    os_err_t (*k_mem_check)     (struct heap_mem *h_mem);
#ifdef OS_USING_MEM_TRACE
    os_err_t (*k_mem_trace)     (struct heap_mem *h_mem);
#endif
    os_size_t(*k_ptr_to_size)   (struct heap_mem *h_mem, void *mem);
    void                       *header;         /* Pointer to current memory's start address. */
    struct heap_mem            *next;           /* Pointer to next memory's heap_mem object. */
    os_size_t                   mem_total;
    os_size_t                   mem_used;
    os_size_t                   mem_maxused;
    struct os_semaphore         sem;
    enum os_mem_alg             alg;
};

struct os_meminfo 
{
    os_size_t                   mem_total;
    os_size_t                   mem_used;
    os_size_t                   mem_maxused;
};
typedef struct os_meminfo os_meminfo_t;

#ifdef OS_USING_SYS_HEAP
void            os_sys_heap_init(void);
extern os_err_t os_sys_heap_add(void *start_addr, os_size_t size, enum os_mem_alg alg);
extern void    *os_malloc(os_size_t size);
extern void    *os_aligned_malloc(os_size_t align, os_size_t size);
extern void     os_free(void *ptr);
extern void    *os_realloc(void *ptr, os_size_t size);
extern void    *os_calloc(os_size_t count, os_size_t size);
extern void     os_memory_info(os_meminfo_t *info);
extern os_err_t os_memory_check(void);
#ifdef OS_USING_MEM_TRACE
extern os_err_t os_memory_trace(void);
#endif

#endif /* end of OS_USING_SYS_HEAP */

struct os_memheap
{
    struct heap_mem            *h_mem;
    os_uint8_t                  object_inited;
    char                        name[OS_NAME_MAX + 1];
    os_list_node_t              resource_node;          /* Node in resource list */
};
typedef struct os_memheap os_memheap_t;

extern os_err_t os_memheap_init(os_memheap_t *heap, const char *name);
extern os_err_t os_memheap_add(os_memheap_t *heap, void *start_addr, os_size_t size, enum os_mem_alg alg);
extern void    *os_memheap_alloc(os_memheap_t *heap, os_size_t size);
extern void    *os_memheap_aligned_alloc(os_memheap_t *heap, os_size_t align, os_size_t size);
extern void    *os_memheap_realloc(os_memheap_t *heap, void *ptr, os_size_t size);
extern void     os_memheap_free(os_memheap_t *heap, void *ptr);
extern void     os_memheap_info(os_memheap_t *heap, os_meminfo_t *info);
extern os_err_t os_memheap_check(os_memheap_t *heap);
#ifdef OS_USING_MEM_TRACE
extern os_err_t os_memheap_trace(os_memheap_t *heap);
#endif
extern os_err_t os_memheap_deinit(os_memheap_t *heap);

#endif /* end of OS_USING_MEM_HEAP */

#ifdef OS_USING_MEM_POOL

#ifdef OS_USING_MP_CHECK_TAG
#define OS_MEMPOOL_BLK_HEAD_SIZE                4
#else
#define OS_MEMPOOL_BLK_HEAD_SIZE                0
#endif
#define OS_MEMPOOL_SIZE(block_count, block_size) ((OS_ALIGN_UP(block_size + OS_MEMPOOL_BLK_HEAD_SIZE, OS_ALIGN_SIZE)) * block_count)

struct os_mempool
{
    void                       *start_addr;             /* Memory pool start. */
    void                       *free_list;              /* Avaliable memory blocks list. */
    os_size_t                   size;                   /* Size of memory pool. */
    os_size_t                   blk_size;               /* Size of memory blocks, maybe not the size for users. */
    os_size_t                   blk_total_num;          /* Numbers of memory block. */
    os_size_t                   blk_free_num;           /* Numbers of free memory block. */
    os_list_node_t              task_list_head;         /* Task suspend on this memory pool. */
    os_list_node_t              resource_node;          /* Node in resource list */
    os_uint8_t                  object_alloc_type;      /* Indicates whether object is allocated */
    os_uint8_t                  object_inited;          /* Whether inited. */
    char                        name[OS_NAME_MAX + 1];  /* mempool name. */
};
typedef struct os_mempool os_mp_t;

struct os_mpinfo 
{
    os_size_t                   blk_size;               /* Size can be used for users in one memory block. */
    os_size_t                   blk_total_num;          /* Numbers of memory block. */
    os_size_t                   blk_free_num;           /* Numbers of free memory block. */
};
typedef struct os_mpinfo os_mpinfo_t;

extern os_err_t os_mp_init(os_mp_t *mp, const char *name, void *start, os_size_t size, os_size_t blk_size);
extern os_err_t os_mp_deinit(os_mp_t *mp);
#ifdef OS_USING_SYS_HEAP
extern os_mp_t *os_mp_create(const char *name, os_size_t blk_count, os_size_t blk_size);
extern os_err_t os_mp_destroy(os_mp_t *mp);
#endif
extern void    *os_mp_alloc(os_mp_t *mp, os_tick_t timeout);
extern void     os_mp_free(os_mp_t *mp, void *mem);
extern void     os_mp_info(os_mp_t *mp, os_mpinfo_t *info);

#endif /*  end of OS_USING_MEM_POOL */

#ifdef __cplusplus
}
#endif

#endif /* __OS_MEMORY_H__ */

