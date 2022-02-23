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
 * @file        os_memory.c
 *
 * @brief       This file implements system memory management and mulitiheap management.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS team      First version.
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_errno.h>
#include <os_memory.h>
#include <os_spinlock.h>
#include "os_kernel_internal.h"

#define MEM_TAG             "MEM"

struct alg_init_func {
    enum os_mem_alg     mem_alg;
    void (*mem_init) (struct heap_mem *h_mem, void *start_addr, os_size_t size);
};

#ifdef OS_USING_ALG_FIRSTFIT
extern void k_firstfit_mem_init(struct heap_mem *h_mem, void *start_addr, os_size_t size);
#endif
#ifdef OS_USING_ALG_BUDDY
extern void k_buddy_mem_init(struct heap_mem *h_mem, void *start_addr, os_size_t size);
#endif

static struct alg_init_func alg_init_table[] = {
#ifdef OS_USING_ALG_FIRSTFIT
    {OS_MEM_ALG_FIRSTFIT, k_firstfit_mem_init},
#endif
#ifdef OS_USING_ALG_BUDDY
    {OS_MEM_ALG_BUDDY,    k_buddy_mem_init},
#endif
    {OS_MEM_ALG_DEFAULT,  OS_NULL}
};

#ifdef OS_USING_MEM_HEAP

static os_list_node_t gs_os_heap_resource_list_head = OS_LIST_INIT(gs_os_heap_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_heap_resource_list_lock);

/**
 ***********************************************************************************************************************
 * @brief           This function initializes memory_heap.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 * @param[in]       name            The name of memory_heap.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_memheap_init(os_memheap_t *heap, const char *name)
{
    os_list_node_t *pos;
    os_memheap_t   *item_heap;
    os_err_t        ret;

    OS_ASSERT(heap);

    ret = OS_EOK;

    os_spin_lock(&gs_os_heap_resource_list_lock);

    os_list_for_each(pos, &gs_os_heap_resource_list_head)
    {
        item_heap = os_list_entry(pos, os_memheap_t, resource_node);
        if (item_heap == heap)
        {
            os_spin_unlock(&gs_os_heap_resource_list_lock);

            OS_KERN_LOG(KERN_ERROR, MEM_TAG, "The heap(addr: %p) already exist", item_heap);
            ret = OS_EINVAL;
            break;
        }
    }

    if (OS_EOK == ret)
    {
        os_list_add_tail(&gs_os_heap_resource_list_head, &heap->resource_node);

        os_spin_unlock(&gs_os_heap_resource_list_lock);

        if (OS_NULL != name)
        {
            strncpy(&heap->name[0], name, OS_NAME_MAX);
            heap->name[OS_NAME_MAX] = '\0';
        }
        else
        {
            heap->name[0] = '\0';
        }
        heap->h_mem = OS_NULL;
        heap->object_inited = OS_KOBJ_INITED;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function deinitializes a memory_heap object.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 *
 * @return          The operation result.
 * @retval          OS_ENOSYS       This function not implemented now.
 ***********************************************************************************************************************
 */
os_err_t os_memheap_deinit(os_memheap_t *heap)
{
    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);

    /* TODO. Not implement now.
       For a perfect deinit function, should carefully deal with multiple discrete memory,
       then it will do harm to the efficiency of memroy alloc and free. Since the demand of deinit
       seems not so necessarily right now, not implemnet.*/

    return OS_ENOSYS;
}

/**
 ***********************************************************************************************************************
 * @brief           This function add memory zone to memory_heap which has been inited, combine multiple memory zone.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 * @param[in]       start_addr      The start address of memory zone.
 * @param[in]       size            The size of this memory zone.
 * @param[in]       alg             The memory algorithm for this memory zone.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_memheap_add(os_memheap_t *heap, void *start_addr, os_size_t size, enum os_mem_alg alg)
{
    os_size_t        start;
    struct heap_mem *h_mem;
    struct heap_mem *h_mem_new;
    os_bool_t        mem_inited = OS_FALSE;
    os_size_t        i;
    os_list_node_t  *pos;
    os_memheap_t    *item_heap;
    struct heap_mem *h_mem_temp;
    os_size_t        h_mem_temp_start;
    os_size_t        h_mem_temp_end;
    os_err_t         ret;

    OS_ASSERT(OS_NULL != heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);
    OS_ASSERT(alg <= OS_MEM_ALG_DEFAULT);

    if (OS_MEM_ALG_DEFAULT == alg)
    {
        alg = ((enum os_mem_alg)0);
    }

    ret = OS_EOK;
    start = OS_ALIGN_UP((os_size_t)(char *)start_addr, 4U);
    OS_ASSERT(((os_size_t)(char *)start_addr + size) > start);
    size  = OS_ALIGN_DOWN((os_size_t)(char *)start_addr + size - start, 4U);

    if (size > sizeof(struct heap_mem))
    {
        os_spin_lock(&gs_os_heap_resource_list_lock);
        /* Search each memheap. */
        os_list_for_each(pos, &gs_os_heap_resource_list_head)
        {
            item_heap = os_list_entry(pos, os_memheap_t, resource_node);
            /* Every memheap may contains multiple memory. Check each memory. */
            h_mem_temp = item_heap->h_mem;
            while(OS_NULL != h_mem_temp)
            {
                h_mem_temp_start = (os_size_t)h_mem_temp;
                h_mem_temp_end   = (os_size_t)(char *)h_mem_temp->header + h_mem_temp->mem_total;
                OS_ASSERT(h_mem_temp_start < h_mem_temp_end);

                if (!((start >= h_mem_temp_end) || (h_mem_temp_start >= (start + size))))
                {
                    OS_KERN_LOG(KERN_ERROR, MEM_TAG, "memory is overlaped, please check !");
                    ret = OS_EINVAL;
                    break;
                }

                h_mem_temp = h_mem_temp->next;
            }
			
            if (OS_EOK != ret)
            {
                os_spin_unlock(&gs_os_heap_resource_list_lock);
                break;
            }
        }
    }
    else
    {
        OS_KERN_LOG(KERN_ERROR, MEM_TAG, "memory is too small !");
        ret = OS_ENOMEM;
    }

    if (OS_EOK == ret)
    {
        h_mem_new = (struct heap_mem *)start;
        (void)memset((void *)h_mem_new, 0, sizeof(struct heap_mem));
        start += sizeof(struct heap_mem);
        size  -= sizeof(struct heap_mem);

        for (i = 0; i < (sizeof(alg_init_table)/ sizeof(struct alg_init_func)); i++)
        {
            if (alg == alg_init_table[i].mem_alg)
            {
                alg_init_table[i].mem_init(h_mem_new, (void *)(char *)start, size);
                mem_inited = OS_TRUE;
                break;
            }
        }
        
        if (OS_TRUE == mem_inited)
        {
            if (OS_NULL != heap->h_mem)
            {
                h_mem = heap->h_mem;
                while (OS_NULL != h_mem->next)
                {
                    h_mem = h_mem->next;
                }
                h_mem->next = h_mem_new;
            }
            else
            {
                heap->h_mem = h_mem_new;
            }
        }
        else
        {
            OS_KERN_LOG(KERN_ERROR, MEM_TAG, "memory algorithm %d not support, please check !", (os_int32_t)alg);
            ret = OS_EINVAL;
        }
		
        os_spin_unlock(&gs_os_heap_resource_list_lock);
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocates 'size' bytes memory space from memory_heap.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 * @param[in]       size            The size of the requested space in bytes.
 *
 * @return          The pointer to allocated memory or NULL if no free memory was found.
 ***********************************************************************************************************************
 */
void *os_memheap_alloc(os_memheap_t *heap, os_size_t size)
{
    struct heap_mem *h_mem;
    void            *ptr;

    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);

    ptr = OS_NULL;
    h_mem= heap->h_mem;
    while (h_mem && h_mem->k_alloc)
    {
        ptr = h_mem->k_alloc(h_mem, size);
        if (ptr)
        {
            break;
        }

        h_mem = h_mem->next;
    }

    return ptr;
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocates a memory, which address is aligned to the specified align size.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 * @param[in]       align           The alignment size.
 * @param[in]       size            The allocated memory block size.
 *
 * @return          The pointer to allocated memory or NULL if no free memory was found.
 ***********************************************************************************************************************
 */
void *os_memheap_aligned_alloc(os_memheap_t *heap, os_size_t align, os_size_t size)
{
    struct heap_mem *h_mem;
    void            *ptr;

    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);

    ptr = OS_NULL;
    h_mem= heap->h_mem;
    while (h_mem && h_mem->k_aligned_alloc)
    {
        ptr = h_mem->k_aligned_alloc(h_mem, align, size);
        if (ptr)
        {
            break;
        }

        h_mem = h_mem->next;
    }

    return ptr;
}

/**
 ***********************************************************************************************************************
 * @brief           This function changes the size of the memory to 'newsize' bytes.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 * @param[in]       ptr             The pointer to old memory block.
 * @param[in]       size            The size of the requested memory in bytes.
 *
 * @return          The pointer to allocated memory or NULL if no free memory was found.
 ***********************************************************************************************************************
 */
void *os_memheap_realloc(os_memheap_t *heap, void *ptr, os_size_t size)
{
    struct heap_mem *h_mem;
    struct heap_mem *h_mem_new;
    os_size_t        oldsize;
    void            *ptr_new;

    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);

    h_mem   = OS_NULL;
    ptr_new = OS_NULL;

    if (!ptr)
    {
        ptr_new = os_memheap_alloc(heap, size);
    }
    else
    {
        /* Find the original h_mem. */
        h_mem = heap->h_mem;
        while(h_mem)
        {
            if ((ptr >= h_mem->header) && (((os_size_t)ptr - (os_size_t)h_mem->header) <= h_mem->mem_total))
            {
                break;
            }
            h_mem = h_mem->next;
        }

        /* Realloc in the original h_mem. */
        if (h_mem && h_mem->k_realloc)
        {
            ptr_new = h_mem->k_realloc(h_mem, ptr, size);
        }

        OS_ASSERT_EX(h_mem, "unexpected heap or mem addr (invalid addr: %p ?)", ptr);
    }

    if ((!ptr_new) && h_mem && (h_mem->k_free))
    {
        /* Alloc in the other h_mem. */
        h_mem_new = heap->h_mem;
        while (h_mem_new)
        {
            if ((h_mem_new != h_mem) && (h_mem_new->k_alloc))
            {
                ptr_new = h_mem_new->k_alloc(h_mem_new, size);
                if (ptr_new)
                {
                    oldsize = h_mem->k_ptr_to_size(h_mem, ptr);
                    memcpy(ptr_new, ptr, oldsize > size ? size : oldsize);
                    h_mem->k_free(h_mem, ptr);
                    break;
                }
            }
            h_mem_new = h_mem_new->next;
        }
    }

    return ptr_new;
}

/**
 ***********************************************************************************************************************
 * @brief           This function frees the memory space pointed to by ptr, which allocated by a previous
 *                  os_memheap_malloc() or os_memheap_realloc() or os_memheap_aligned_alloc().
 *
 * @param[in]       heap            The pointer to memory_heap object.
 * @param[in]       ptr             The pointer to memory block.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_memheap_free(os_memheap_t *heap, void *ptr)
{
    struct heap_mem *h_mem;

    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);
    OS_ASSERT(ptr);

    h_mem = heap->h_mem;
    while(h_mem)
    {
        if ((ptr >= h_mem->header) && ((os_size_t)ptr <= ((os_size_t)h_mem->header + h_mem->mem_total)))
        {
            if (h_mem->k_free)
            {
                h_mem->k_free(h_mem, ptr);
            }
            break;
        }
        h_mem = h_mem->next;
    }

    OS_ASSERT_EX(h_mem, "unexpected heap or mem addr (invalid addr: %p ?)", ptr);
}

/**
 ***********************************************************************************************************************
 * @brief           This function get memory_heap info.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 * @param[in,out]   info            The memory_heap info, which include mem_total, mem_used, mem_maxused.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_memheap_info(os_memheap_t *heap, os_meminfo_t *info)
{
    struct heap_mem *h_mem;

    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);

    (void)memset(info, 0, sizeof(os_meminfo_t));

    h_mem = heap->h_mem;
    while (h_mem)
    {
        info->mem_total   += h_mem->mem_total;
        info->mem_used    += h_mem->mem_used;
        info->mem_maxused += h_mem->mem_maxused;
        h_mem = h_mem->next;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function check whether the memory_heap is correct.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 *
 * @retval          OS_EOK          Check successful.
 * @retval          else            Check error.
 ***********************************************************************************************************************
 */
os_err_t os_memheap_check(os_memheap_t *heap)
{
    struct heap_mem *h_mem;
    os_err_t         ret;

    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);

    ret = OS_EOK;

    h_mem = heap->h_mem;
    while (h_mem)
    {
        if (h_mem->k_mem_check)
        {
            ret = h_mem->k_mem_check(h_mem);
            if (ret != OS_EOK)
            {
                break;
            }
        }
        h_mem = h_mem->next;
    }

    return ret;
}

#ifdef OS_USING_MEM_TRACE
/**
 ***********************************************************************************************************************
 * @brief           This function trace task info for every allocated memory.
 *
 * @param[in]       heap            The pointer to memory_heap object.
 *
 * @retval          OS_EOK          Trace successful.
 * @retval          else            Trace error.
 ***********************************************************************************************************************
 */
os_err_t os_memheap_trace(os_memheap_t *heap)
{
    struct heap_mem *h_mem;
    os_err_t         ret;

    OS_ASSERT(heap);
    OS_ASSERT(OS_KOBJ_INITED == heap->object_inited);

    ret = OS_EOK;

    h_mem = heap->h_mem;
    while (h_mem)
    {
        if (h_mem->k_mem_trace)
        {
            h_mem->k_mem_trace(h_mem);
            if (ret != OS_EOK)
            {
                break;
            }
        }
        h_mem = h_mem->next;
    }

    return ret;
}
#endif

#ifdef OS_USING_SHELL
#include <shell.h>

os_err_t sh_memheap_show(os_int32_t argc, char **argv)
{
    os_meminfo_t     info;
    struct heap_mem *h_mem;
    os_list_node_t  *pos;
    os_memheap_t    *item_heap;
    os_uint32_t      i;
    os_uint32_t      j;

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    i = 0;

    os_spin_lock(&gs_os_heap_resource_list_lock);

    os_list_for_each(pos, &gs_os_heap_resource_list_head)
    {
        os_spin_unlock(&gs_os_heap_resource_list_lock);

        /* Now the deinit function not implemented, 
           it's not necessary to protect for read access of item_heap. */
        item_heap = os_list_entry(pos, os_memheap_t, resource_node);
        os_memheap_info(item_heap, &info);
        os_kprintf("memheap[%d] : memory total:%d used:%d max_used:%d name:%s\r\n",
                i, info.mem_total, info.mem_used, info.mem_maxused, item_heap->name);
        h_mem = item_heap->h_mem;
        j = 0;
        while(h_mem)
        {
            os_kprintf("----mem[%d] : %p~%p\r\n", j, h_mem->header,
                        (void *)((os_size_t)h_mem->header + (os_size_t)h_mem->mem_total));
            h_mem = h_mem->next;
            j++;
        }
        i++;

        os_spin_lock(&gs_os_heap_resource_list_lock);
    }

    os_spin_unlock(&gs_os_heap_resource_list_lock);

    return OS_EOK;
}

SH_CMD_EXPORT(show_heap, sh_memheap_show, "show memheap information");
#endif
#endif  /* end of OS_USING_MEM_HEAP */

#ifdef OS_USING_SYS_HEAP

static os_memheap_t gs_sys_heap;

/**
 ***********************************************************************************************************************
 * @brief           This function initializes the system_memory_heap.
 *
 * @param[in]       None.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_sys_heap_init(void)
{
    os_err_t ret;

    (void)memset(&gs_sys_heap, 0, sizeof(os_memheap_t));
    ret = os_memheap_init(&gs_sys_heap, "SYS_HEAP");
    if (OS_EOK != ret)
    {
        OS_KERN_LOG(KERN_ERROR, MEM_TAG, "system memory heap init failed!");
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function add memory zone to the system_memory_heap, combine multiple memory zone together.
 *
 * @param[in]       start_addr      The start address of memory.
 * @param[in]       size            The size of this memory zone.
 * @param[in]       alg             The memory algorithm for this memory zone.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_sys_heap_add(void *start_addr, os_size_t size, enum os_mem_alg alg)
{
    return os_memheap_add(&gs_sys_heap, start_addr, size, alg);
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocates 'size' bytes from system_memory_heap, returns pointer of the allocated memory.
 *
 * @param[in]       size            The size of the requested block in bytes.
 *
 * @return          The pointer to allocated memory or OS_NULL if no free memory was found.
 ***********************************************************************************************************************
 */
void *os_malloc(os_size_t size)
{
    return os_memheap_alloc(&gs_sys_heap, size);
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocates a memory block from system_memory_heap, which address is aligned to the 
 *                  specified alignment size.
 *
 * @param[in]       align           The alignment size.
 * @param[in]       size            The allocated memory block size.
 *
 * @return          The allocated memory pointer.
 * @retval          OS_NULL         Allocated memory block failed.
 * @retval          else            Allocated memory block successfull.
 ***********************************************************************************************************************
 */
void *os_aligned_malloc(os_size_t align, os_size_t size)
{
    return os_memheap_aligned_alloc(&gs_sys_heap, align, size);
}

/**
 ***********************************************************************************************************************
 * @brief           This function changes the size of the memory block pointed to by 'ptr' to 'newsize' bytes. If size
 *                  is equal to 0, it works in the same way as os_free(). If 'ptr' is OS_NULL, it works in the same way
 *                  as os_malloc().
 *
 * @param[in]       ptr             The pointer of memory block to change.
 * @param[in]       size            The size of new memory block.
 *
 * @return          The pointer to newly allocated memory or NULL.
 ***********************************************************************************************************************
 */
void *os_realloc(void *ptr, os_size_t size)
{
    return os_memheap_realloc(&gs_sys_heap, ptr, size);
}

/**
 ***********************************************************************************************************************
 * @brief           This function allocates memory for an array of 'count' elements of 'size' bytes each and returns a
 *                  pointer to the allocated memory. The memory is set to zero.
 *
 * @param[in]       count           Number of array to allocate.
 * @param[in]       size            Size of each element to allocate.
 *
 * @return          The pointer to allocated memory or OS_NULL if no free memory was found.
 ***********************************************************************************************************************
 */
void *os_calloc(os_size_t count, os_size_t size)
{
    void *ptr;

    ptr = os_memheap_alloc(&gs_sys_heap, count * size);
    if (ptr)
    {
        (void)memset(ptr, 0, count * size);
    }
    return ptr;
}

/**
 ***********************************************************************************************************************
 * @brief           This function frees the memory space pointed to by 'ptr', which allocated by os_malloc(), or 
 *                  os_realloc() or os_calloc() or os_aligned_malloc().
 *
 * @param[in]       ptr             The pointer to memory space.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_free(void *ptr)
{
    os_memheap_free(&gs_sys_heap, ptr);
}

/**
 ***********************************************************************************************************************
 * @brief           This function get memory info for system_memory_heap.
 *
 * @param[in,out]   info            The system_memory_heap info, which include mem_total, mem_used, mem_maxused.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_memory_info(os_meminfo_t *info)
{
    OS_ASSERT(info);
    os_memheap_info(&gs_sys_heap, info);
}

/**
 ***********************************************************************************************************************
 * @brief           This function check whether the system_memory_heap is correct.
 *
 * @param[in]       NONE.
 *
 * @retval          OS_EOK          Check successful.
 * @retval          else            Check error.
 ***********************************************************************************************************************
 */
os_err_t os_memory_check(void)
{
    return os_memheap_check(&gs_sys_heap);
}

#ifdef OS_USING_MEM_TRACE
/**
 ***********************************************************************************************************************
 * @brief           This fuction trace task info for every allocated memory from system_memory_heap.
 *
 * @param[in]       NONE.
 *
 * @retval          OS_EOK          Trace successful.
 * @retval          else            Trace error.
 ***********************************************************************************************************************
 */
os_err_t os_memory_trace(void)
{
    return os_memheap_trace(&gs_sys_heap);
}
#endif

#ifdef OS_USING_SHELL
#include <shell.h>

os_err_t sh_memshow(os_int32_t argc, char **argv)
{
    os_meminfo_t info;

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_memory_info(&info);
    os_kprintf("memory total    : %d\r\n", info.mem_total);
    os_kprintf("memory used     : %d\r\n", info.mem_used);
    os_kprintf("memory max used : %d\r\n", info.mem_maxused);

    return OS_EOK;
}
SH_CMD_EXPORT(show_mem, sh_memshow, "show memory usage information");

os_err_t sh_memcheck(os_int32_t argc, char **argv)
{
    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    return os_memory_check();
}
SH_CMD_EXPORT(check_mem, sh_memcheck, "check memory data");

#ifdef OS_USING_MEM_TRACE
os_err_t sh_memtrace(os_int32_t argc, char **argv)
{
    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    return os_memory_trace();
}
SH_CMD_EXPORT(trace_mem, sh_memtrace, "trace memory used by task");
#endif  /* end of OS_USING_MEM_TRACE */

#endif  /* end of OS_USING_SHELL */

#endif  /* end of OS_USING_SYS_HEAP */


