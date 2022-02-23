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
 * @file        os_mem_buddy.c
 *
 * @brief       This file implements buddy memory algorithm.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-08   OneOS team      First version.
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_stddef.h>
#include <os_sem.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_memory.h>
#include "os_kernel_internal.h"
#include "string.h"

#define MIN_BLOCK_SIZE          16
#define MAX_BUDDY_LEVEL         16
#define INVALID_BUDDY_LEVEL     (MAX_BUDDY_LEVEL + 1)

#define BLK_STAT_FREE           0
#define BLK_STAT_USED           1
#define BLK_WITH_BUDDY          0
#define BLK_NO_BUDDY            1
#define BLK_TAG_NOT_DUPLICATE   0
#define BLK_TAG_DUPLICATE       1
#define BLK_TAG_ADJACENT        2

#define BLK_MAGIC_TAG           0x5A

#define BLK_HEAD_TAG_SIZE       4
#ifdef OS_USING_MEM_TRACE
#define BLK_TASK_TAG_SIZE       sizeof(os_base_t)
#define BLK_HEAD_SIZE           (BLK_HEAD_TAG_SIZE + BLK_TASK_TAG_SIZE)
#else
#define BLK_HEAD_SIZE           (BLK_HEAD_TAG_SIZE)
#endif
#define BLK_PTR_TO_ID(h, ptr, level)    (((os_uint8_t *)ptr - (os_uint8_t *)h->blk_start) / h->level_size[level])
#define BLK_ID_TO_PTR(h, id, level)     (struct buddy_block *)((os_uint8_t *)h->blk_start + id * h->level_size[level])

#define BLK_ID_TO_BUDDY_ID(id)      ((id & 0x1) ? (id - 1) : (id + 1))

#define SIZE_WITH_HEAD(size)        (size + BLK_HEAD_SIZE)

#define BLK_TO_MEM(blk)     ((void *)((os_uint8_t *)blk + BLK_HEAD_SIZE))
#define MEM_TO_BLK(ptr)     ((struct buddy_block *)((os_uint8_t *)ptr - BLK_HEAD_SIZE))

#define BLK_HAS_BUDDY(h, level, blkid)  (((blkid & (~1)) + 1) < h->level_blkid_max[level])

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define MAX_BLOCK_SIZE  MIN(MIN_BLOCK_SIZE * (1 << (MAX_BUDDY_LEVEL - 1)), OS_ALG_BUDDY_MAX_BLOCK_SIZE)

#define BUDDY_MEM_INFO_INIT(x, size)    do {\
    x->mem_total = (size);                  \
    x->mem_used = 0;                        \
    x->mem_maxused = 0;                     \
} while(0)

#define BUDDY_MEM_USED_INC(x, size)     do {\
    x->mem_used += (size);                  \
    if (x->mem_used > x->mem_maxused) {     \
        x->mem_maxused = x->mem_used;       \
    }                                       \
}while(0)

#define BUDDY_MEM_USED_DEC(x, size)     do {\
    x->mem_used -= (size);                  \
}while(0)

#ifdef OS_USING_MEM_TRACE
#define SET_MEM_TASK(blk)               do {\
    *((os_base_t *)((os_uint8_t *)blk + BLK_HEAD_TAG_SIZE)) = (os_base_t)os_task_self();\
} while(0)

#define GET_MEM_TASK(blk)   ((blk->duplicate == BLK_TAG_ADJACENT) ?                       \
        (*((os_base_t *)((os_uint8_t *)blk + BLK_HEAD_TAG_SIZE + BLK_HEAD_TAG_SIZE))):  \
        (*((os_base_t *)((os_uint8_t *)blk + BLK_HEAD_TAG_SIZE))))
#endif

struct buddy_heap {
    os_size_t       heap_size;
    os_size_t       level_num;
    os_size_t       level_size[MAX_BUDDY_LEVEL];
    os_size_t       level_blkid_max[MAX_BUDDY_LEVEL];
    os_list_node_t  freelist[MAX_BUDDY_LEVEL];
    void           *blk_start;
    void           *blk_end;
};

struct buddy_block {
    os_uint8_t      level;
    os_uint8_t      used;
    os_uint8_t      duplicate;
    os_uint8_t      magic_tag;
    os_list_node_t  list_node;
};

static os_size_t _k_cal_prev_power_of_2(os_size_t size)
{
    os_size_t i;
    os_size_t val;

    if (0U == size)
    {
        val = 0;
    }
    else
    {
        for (i = 0; size > 1; i++)
        {
            size >>= 1;
        }

        val = (1 << i);
    }

    return val;
}

static void _k_buddy_init_level_size(struct buddy_heap *h, os_size_t size)
{
    os_size_t i;
    os_size_t level_size;
    os_size_t max_block_size;

    max_block_size = MIN(MAX_BLOCK_SIZE, size);
    level_size = _k_cal_prev_power_of_2(max_block_size);

    for (i = 0; (i < MAX_BUDDY_LEVEL) && (level_size >= MIN_BLOCK_SIZE); i++)
    {
        h->level_size[i] = level_size;
        h->level_blkid_max[i] = size / level_size;
        level_size >>= 1;
    }
    h->level_num = i;

    OS_ASSERT(h->level_num >= 1);
}

OS_INLINE os_size_t _k_size_to_level(struct buddy_heap *h, os_size_t size)
{
    os_size_t lvl;

    lvl = h->level_num - 1;
    if (size > h->level_size[0])
    {
        /* Size is larger than max_blk_size, the max_blk_size depends on 
           OS_ALG_BUDDY_MAX_BLOCK_SIZE, MAX_BUDDY_LEVEL, and total memory size*/
        if (size > OS_ALG_BUDDY_MAX_BLOCK_SIZE)
        {
            os_kprintf("The alloc size is too large, you may increase OS_ALG_BUDDY_MAX_BLOCK_SIZE \r\n");
        }
        lvl = INVALID_BUDDY_LEVEL;
    }
    else
    {
        while (size > h->level_size[lvl])
        {
            lvl--;
        }
    }

    return lvl;
}
static void _k_buddy_init_free_list(struct buddy_heap *h, os_size_t size)
{
    os_size_t i;
    os_size_t level_0_blk_num;
    os_size_t level_size;
    os_size_t residual_size;
    struct buddy_block *blk;

    level_0_blk_num = size / h->level_size[0];
    residual_size = size % h->level_size[0];

    for (i = 0; i < MAX_BUDDY_LEVEL; i++)
    {
        os_list_init(&h->freelist[i]);
    }

    blk = h->blk_start;
    for (i = 0; i < level_0_blk_num; i++)
    {
        blk->level = 0;
        blk->used  = BLK_STAT_FREE;
        blk->duplicate = BLK_TAG_NOT_DUPLICATE;
        blk->magic_tag = BLK_MAGIC_TAG;
        os_list_add_tail(&h->freelist[0], &blk->list_node);
        blk = (struct buddy_block *)((os_uint8_t *)blk + h->level_size[0]);
    }

    while (residual_size >= MIN_BLOCK_SIZE)
    {
        level_size = _k_cal_prev_power_of_2(residual_size);
        residual_size = residual_size % level_size;

        blk->level = _k_size_to_level(h, level_size);
        blk->used  = BLK_STAT_FREE;
        blk->duplicate = BLK_TAG_NOT_DUPLICATE;
        blk->magic_tag = BLK_MAGIC_TAG;
        os_list_add_tail(&h->freelist[blk->level], &blk->list_node);
        blk = (struct buddy_block *)((os_uint8_t *)blk + h->level_size[blk->level]);
    }

    if (residual_size > 0)
    {
        h->heap_size -= residual_size;
    }
    h->blk_end = blk;
}

static void* _k_alloc_block(struct buddy_heap *h, os_size_t level)
{
    os_list_node_t *node;
    struct buddy_block *block;

    block = OS_NULL;
    while (1)
    {
        node = os_list_first(&h->freelist[level]);
        if (node)
        {
            os_list_del(node);
            block = os_container_of(node, struct buddy_block, list_node);
            OS_ASSERT(block->level == level);
            break;
        }

        if (level-- == 0)
        {
            break;
        }
    }

    return block;
}

static void _k_split_block(struct buddy_heap *h, struct buddy_block *block, os_size_t level)
{
    os_size_t level_new;
    struct buddy_block *block_new;

    for (level_new = (block->level + 1); level_new <= level; level_new++)
    {
        block_new = (struct buddy_block *)((os_uint8_t *)block + h->level_size[level_new]);
        block_new->level = level_new;
        block_new->used =  BLK_STAT_FREE;
        block_new->duplicate = BLK_TAG_NOT_DUPLICATE;
        block_new->magic_tag = BLK_MAGIC_TAG;
        os_list_add(&h->freelist[level_new], &block_new->list_node);
    }
}

struct buddy_block *_k_split_block_aligned(struct buddy_heap *h, struct buddy_block *block, os_size_t align, os_size_t bytes)
{
    void *mem;
    void *mem_begin;
    void *mem_end;
    os_size_t blk_size;
    os_size_t level_split;
    struct buddy_block *blk_begin;
    struct buddy_block *blk_end;
    struct buddy_block *blk_mid;
    struct buddy_block *blk_split;

    blk_split = block;
    mem = (void *)OS_ALIGN_UP((os_size_t)BLK_TO_MEM(block), align);
    blk_size  = h->level_size[block->level];
    blk_begin = block;
    blk_end   = (struct buddy_block *)((os_size_t)blk_begin + blk_size);
    blk_mid   = (struct buddy_block *)((os_size_t)blk_begin + (blk_size >> 1));
    mem_begin = (void *)((os_size_t)mem - BLK_HEAD_SIZE);
    mem_end   = (void *)OS_ALIGN_UP((os_size_t)mem + bytes, OS_ALIGN_SIZE);
    level_split = block->level;

    while(((mem_begin >=  (void *)blk_begin) && (mem_end <= (void *)blk_mid))
        || ((mem_begin >= (void *)blk_mid) && (mem_end <= (void *)blk_end)))
    {
        if (level_split >= (h->level_num - 1))
        {
            break;
        }

        level_split++;
        blk_begin->level     = level_split;
        blk_begin->duplicate = BLK_TAG_NOT_DUPLICATE;
        blk_begin->magic_tag = BLK_MAGIC_TAG;
        blk_mid->level       = level_split;
        blk_mid->duplicate   = BLK_TAG_NOT_DUPLICATE;
        blk_mid->magic_tag   = BLK_MAGIC_TAG;

        if (mem_begin >= (void *)blk_mid)
        {
            blk_begin->used = BLK_STAT_FREE;
            blk_mid->used   = BLK_STAT_USED;
            os_list_add(&h->freelist[level_split], &blk_begin->list_node);
            blk_split = blk_mid;
            blk_begin = blk_mid;
        }
        else
        {
            blk_begin->used = BLK_STAT_USED;
            blk_mid->used   = BLK_STAT_FREE;
            os_list_add(&h->freelist[level_split], &blk_mid->list_node);
            blk_split = blk_begin;
            blk_end   = blk_mid;
        }
        blk_mid = (struct buddy_block *)((os_size_t)blk_begin + (((os_size_t)blk_end - (os_size_t)blk_begin) >> 1));
    }

    return blk_split;
}

static void _k_merge_block(struct buddy_heap *h, struct buddy_block *block)
{
    os_size_t block_buddy_id;
    struct buddy_block *block_buddy;
    struct buddy_block *block_merge = block;
    os_size_t blkid_merge;
    os_size_t level_merge;

    blkid_merge = BLK_PTR_TO_ID(h, block, block->level);
    level_merge = block->level;
    block_merge->used = BLK_STAT_FREE;

    /* level 0 should not be merged. */
    while ((level_merge > 0) && BLK_HAS_BUDDY(h, block_merge->level, blkid_merge))
    {
        OS_ASSERT(block_merge->used == BLK_STAT_FREE);
        block_buddy_id = BLK_ID_TO_BUDDY_ID(blkid_merge);
        block_buddy = BLK_ID_TO_PTR(h, block_buddy_id, level_merge);
        if ((block_buddy->used == BLK_STAT_USED) || (block_buddy->level != block_merge->level))
        {
            break;
        }

        os_list_del(&block_buddy->list_node);
        level_merge -= 1; 
        blkid_merge >>= 1;    /* For example, if blkid is 6 or 7, the upper level blkid is 3.*/
        block_merge = BLK_ID_TO_PTR(h, blkid_merge, level_merge);
        block_merge->level = level_merge;
    }

    os_list_add(&h->freelist[level_merge], &block_merge->list_node);
}

static void _k_buddy_mem_init(struct heap_mem *h_mem, void *mem, os_size_t bytes)
{
    os_size_t start;
    os_size_t size;
    struct buddy_heap *h;

    os_sem_init(&h_mem->sem, "mem_b_sem", 1, 1);

    OS_ASSERT_EX(bytes > sizeof(struct buddy_heap), "heap size is too small");

    OS_ASSERT(bytes > OS_ALIGN_SIZE);
    start = OS_ALIGN_UP((os_size_t)mem, OS_ALIGN_SIZE);
    size  = OS_ALIGN_DOWN((os_size_t)mem + bytes - start, OS_ALIGN_SIZE);
    OS_ASSERT(size >= (OS_ALIGN_UP(sizeof(struct buddy_heap) + MIN_BLOCK_SIZE, OS_ALIGN_SIZE)));

    h             = (struct buddy_heap *)start;
    h->heap_size  = size;
    h->blk_start  = (void *)(start + OS_ALIGN_UP(sizeof(struct buddy_heap), OS_ALIGN_SIZE));
    h_mem->header = h;

    size -= OS_ALIGN_UP(sizeof(struct buddy_heap), OS_ALIGN_SIZE);
    _k_buddy_init_level_size(h, size);
    _k_buddy_init_free_list(h, size);
    BUDDY_MEM_INFO_INIT(h_mem, h->heap_size);
    BUDDY_MEM_USED_INC(h_mem, OS_ALIGN_UP(sizeof(struct buddy_heap), OS_ALIGN_SIZE));
}

static void *_k_buddy_mem_alloc(struct heap_mem *h_mem, os_size_t bytes)
{
    struct buddy_heap  *h;
    struct buddy_block *block;
    os_size_t size;
    os_size_t level_req;
    void *mem;

    mem = OS_NULL;
    h = h_mem->header;

    size = SIZE_WITH_HEAD(bytes);
    level_req = _k_size_to_level(h, size);
    if (INVALID_BUDDY_LEVEL != level_req)
    {
        (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

        block = _k_alloc_block(h, level_req);
        if (block)
        {
            if (block->level < level_req)
            {
                _k_split_block(h, block, level_req);
            }
            block->level = level_req;
            block->used = BLK_STAT_USED;
            block->duplicate = BLK_TAG_NOT_DUPLICATE;
            block->magic_tag = BLK_MAGIC_TAG;
#ifdef OS_USING_MEM_TRACE
            SET_MEM_TASK(block);
#endif
            mem = BLK_TO_MEM(block);
            BUDDY_MEM_USED_INC(h_mem, h->level_size[level_req]);
        }

        (void)os_sem_post(&h_mem->sem);
    }

    return mem;
}

static void _k_buddy_mem_free(struct heap_mem *h_mem, void *mem)
{
    struct buddy_heap  *h;
    struct buddy_block *block;
    struct buddy_block *block_tmp;
    os_size_t level;
    os_size_t blkid;

    if (mem)
    {
        h = h_mem->header;
        OS_ASSERT_EX((mem >= h->blk_start) && (mem <= h->blk_end),
            "unexpected mem addr (invalid addr?) for memory at %p", mem);
        block = MEM_TO_BLK(mem);

        (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

        OS_ASSERT_EX(block->used == BLK_STAT_USED,
            "unexpected mem state (double-free? or invalid addr?) for memory at %p", mem);
        OS_ASSERT_EX(block->magic_tag == BLK_MAGIC_TAG,
            "unexpected header info (been write illegal?) for memory at %p", mem);

        if ((block->duplicate == BLK_TAG_DUPLICATE) || (block->duplicate == BLK_TAG_ADJACENT))
        {
            /* Get actual block header from duplicate block header */
            blkid = BLK_PTR_TO_ID(h, block, block->level);
            block_tmp = BLK_ID_TO_PTR(h, blkid, block->level);
            OS_ASSERT(block >= block_tmp);
            OS_ASSERT(block_tmp->level == block->level);
            OS_ASSERT(block_tmp->used == block->used);
            OS_ASSERT(block_tmp->magic_tag == block->magic_tag);
            block = block_tmp;
        }
        level = block->level;
        BUDDY_MEM_USED_DEC(h_mem, h->level_size[level]);
        _k_merge_block(h, block);

        (void)os_sem_post(&h_mem->sem);
    }
}

static void *_k_buddy_mem_realloc(struct heap_mem *h_mem, void *mem, os_size_t bytes)
{
    struct buddy_heap *h;
    struct buddy_block *block;
    struct buddy_block *block_tmp;
    os_size_t level_new;
    os_size_t size_new;
    os_size_t blkid;
    void *mem_new;

    h = h_mem->header;

    if (OS_NULL == mem)
    {
        mem_new = _k_buddy_mem_alloc(h_mem, bytes);
    }
    else
    {
        mem_new = OS_NULL;

        if (0U == bytes)
        {
            _k_buddy_mem_free(h_mem, mem);
            level_new = INVALID_BUDDY_LEVEL;
        }
        else
        {
            size_new = SIZE_WITH_HEAD(bytes);
            level_new = _k_size_to_level(h, size_new);
        }

        if (INVALID_BUDDY_LEVEL != level_new)
        {
            OS_ASSERT_EX((mem >= h->blk_start) && (mem <= h->blk_end),
                "unexpected mem addr (invalid addr?) for memory at %p", mem);
            block = MEM_TO_BLK(mem);

            (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

            OS_ASSERT_EX(block->used == BLK_STAT_USED, 
                "unexpected mem state (already free? or invalid addr?) for memory at %p", mem);
            OS_ASSERT_EX(block->magic_tag == BLK_MAGIC_TAG, 
                "unexpected header info (been write illegal?) for memory at %p", mem);

            if ((block->duplicate == BLK_TAG_DUPLICATE) || (block->duplicate == BLK_TAG_ADJACENT))
            {
                /* Get actual block header from duplicate block header */
                blkid = BLK_PTR_TO_ID(h, block, block->level);
                block_tmp = BLK_ID_TO_PTR(h, blkid, block->level);
                OS_ASSERT(block >= block_tmp);
                OS_ASSERT(block_tmp->level == block->level);
                OS_ASSERT(block_tmp->used == block->used);
                OS_ASSERT(block_tmp->magic_tag == block->magic_tag);
                block = block_tmp;
            }

            if (block->level < level_new)
            {
                /* Split off the old. */
                _k_split_block(h, block, level_new);
                BUDDY_MEM_USED_DEC(h_mem, h->level_size[block->level] - h->level_size[level_new]);
                block->level = level_new;
                block->used = BLK_STAT_USED;
                block->duplicate = BLK_TAG_NOT_DUPLICATE;
                block->magic_tag = BLK_MAGIC_TAG;
#ifdef OS_USING_MEM_TRACE
                SET_MEM_TASK(block);
#endif

                (void)os_sem_post(&h_mem->sem);

                mem_new = BLK_TO_MEM(block);
            }
            else if (block->level > level_new)
            {
                (void)os_sem_post(&h_mem->sem);

                /* alloc new, and copy old to new, then free old. */
                mem_new = _k_buddy_mem_alloc(h_mem, bytes);
                if (mem_new)
                {
                    memcpy(mem_new, mem, (os_size_t)block + h->level_size[block->level] - (os_size_t)mem);
                    _k_buddy_mem_free(h_mem, mem);
                }
            }
            else
            {
                (void)os_sem_post(&h_mem->sem);

                /* Same as old, do nothing. */
                mem_new = mem;
            }
        }
    }

    return mem_new;
}

static void *_k_buddy_mem_aligned_alloc(struct heap_mem *h_mem, os_size_t align, os_size_t bytes)
{
    struct buddy_heap  *h;
    struct buddy_block *block;
    struct buddy_block *block_duplicate;
    os_size_t size_before;
    os_size_t size_after;
    void *mem;

    h = h_mem->header;
    OS_ASSERT_EX((align & (align - 1)) == 0, "unexpected align: %lu (should be the power of 2)", align);

    mem = OS_NULL;
    if (0U != bytes)
    {
        if (align <= OS_ALIGN_SIZE)
        {
            mem = _k_buddy_mem_alloc(h_mem, bytes);
        }
        else
        {
            mem = _k_buddy_mem_alloc(h_mem, (bytes + align - 1));

            if (mem)
            {
                (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

                block = MEM_TO_BLK(mem);
                size_before = h->level_size[block->level];
                block = _k_split_block_aligned(h, block, align, bytes);
                size_after = h->level_size[block->level];
                if (size_before > size_after)
                {
                    BUDDY_MEM_USED_DEC(h_mem, size_before - size_after);
                }

                /* Get the aligned memory addr to user. */
                mem = (void *)OS_ALIGN_UP((os_size_t)BLK_TO_MEM(block), align);
                OS_ASSERT(mem >= BLK_TO_MEM(block));
                OS_ASSERT((os_size_t)mem + bytes <= (os_size_t)block + h->level_size[block->level]);

                /* Fill duplicate header, when mem free, we can get the actual header from this header info */
                block_duplicate = MEM_TO_BLK(mem);
                OS_ASSERT(block_duplicate >= block);

                if (block_duplicate > block)
                {
                    block_duplicate->level = block->level;
                    block_duplicate->used = block->used;
                    block_duplicate->magic_tag = BLK_MAGIC_TAG;
#ifdef OS_USING_MEM_TRACE
                    /* If ADJACENT : the block will fill as:
                        | BLK_TAG | BLK_TAG_DUP | BLK_TASK    | MEM TO USER |
                       If DUPLICATE: the block will fill as:
                        | BLK_TAG | BLK_TASK    |not use(hole)| BLK_TAG_DUP | BLK_TASK_DUP | MEM TO USER |*/
                    block_duplicate->duplicate = ((os_ubase_t)block_duplicate >= ((os_ubase_t)block + BLK_HEAD_SIZE)) ? \
                                                BLK_TAG_DUPLICATE : BLK_TAG_ADJACENT;
#else
                    /* The block will fill as:
                        | BLK_TAG |not use(hole)| BLK_TAG_DUP | MEM TO USER |*/
                    block_duplicate->duplicate = BLK_TAG_DUPLICATE;
#endif
                    block->duplicate = block_duplicate->duplicate;
                }

#ifdef OS_USING_MEM_TRACE
                /* Whether the block tag adjacent, duplicate or not, save task after the last block tag. */
                SET_MEM_TASK(block_duplicate);
                /* If tag duplicate, we also save task after block tag (which before block duplicate tag). */
                if (block->duplicate == BLK_TAG_DUPLICATE)
                {
                    SET_MEM_TASK(block);
                }
#endif
                (void)os_sem_post(&h_mem->sem);
            }
        }
    }

    return mem;
}

static os_size_t _k_buddy_mem_ptr_to_size(struct heap_mem *h_mem, void *mem)
{
    struct buddy_heap  *h = h_mem->header;
    struct buddy_block *block;
    os_size_t size;

    block = MEM_TO_BLK(mem);
    size = h->level_size[block->level] - BLK_HEAD_SIZE;

    return size;
}

static void _k_buddy_mem_deinit(struct heap_mem *h_mem)
{
    os_sem_deinit(&h_mem->sem);
}

static os_err_t _k_buddy_mem_check(struct heap_mem *h_mem)
{
    struct buddy_heap  *h = h_mem->header;
    struct buddy_block *block;
    struct buddy_block *block_buddy;
    os_size_t block_buddy_id;
    os_size_t block_size = 0;
    os_size_t block_id = 0;
    os_size_t size_used = 0;
    os_size_t size_total = 0;
    os_size_t level = 0;
    os_size_t i = 0;
    os_err_t  ret;

    os_kprintf("mem_check for memory addr: 0x%8x ~ 0x%8x\r\n", 
                (os_size_t)h_mem->header, (os_size_t)h_mem->header + h_mem->mem_total);

    OS_ASSERT_EX((os_size_t)h_mem->header + OS_ALIGN_UP(sizeof(struct buddy_heap), OS_ALIGN_SIZE) == (os_size_t)h->blk_start,
        "mem has been damaged? mem header:0x%x blk_start:0x%x", h_mem->header, h->blk_start);
    OS_ASSERT_EX((os_size_t)h_mem->header + h->heap_size == (os_size_t)h->blk_end,
        "mem has been damaged? mem header:0x%x blk_start:0x%x heap_size:0x%x", 
        h_mem->header, h->blk_end, h->heap_size);
    OS_ASSERT_EX(h->level_num <= MAX_BUDDY_LEVEL,
        "heap has been damaged? level_num:%lu MAX_BUDDY_LEVEL:%d", h->level_num, MAX_BUDDY_LEVEL);
    for (i = 0; i < h->level_num - 1; i++)
    {
        OS_ASSERT_EX(h->level_size[i] == (h->level_size[i + 1] << 1),
            "heap has been damaged? level[%d]:0x%x level[%d]:0x%x", i, h->level_size[i], i + 1, h->level_size[i + 1]);

    }

    for (i = 0; i < h->level_num; i++)
    {
        OS_ASSERT_EX((os_size_t)h->blk_start + h->level_size[i] * h->level_blkid_max[i] <= (os_size_t)h->blk_end,
            "heap has been damaged? blk_start:0x%x blk_end:0x%x level[%d] size:0x%x blkid_max[%lu]:0x%x", 
            h->blk_start, h->blk_end, i, h->level_size[i], h->level_blkid_max[i]);
    }
    block = (struct buddy_block *)h->blk_start;
    size_used  = ((os_size_t)h->blk_start - (os_size_t)h);
    size_total = size_used;

    /*os_kprintf("used   mem_addr    blk_addr    blk_size   blk_lvl-blk_id   buddy_addr\r\n"); */

    (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

    ret = OS_EOK;
    while ((void *)block < h->blk_end)
    {
        OS_ASSERT_EX(block->magic_tag == BLK_MAGIC_TAG, 
            "unexpected header info (been write illegal?) for memory at %p", BLK_TO_MEM(block));

        block_size = h->level_size[block->level];
        if (block->used)
        {
            size_used += block_size;
        }
        size_total += block_size;

        block_id = BLK_PTR_TO_ID(h, block, block->level);
        if (BLK_HAS_BUDDY(h, block->level, block_id))
        {
            block_buddy_id = BLK_ID_TO_BUDDY_ID(block_id);
            block_buddy = BLK_ID_TO_PTR(h, block_buddy_id, block->level);
        }
        else
        {
            block_buddy = OS_NULL;
        }

        /*os_kprintf("%c     0x%08x  0x%08x  0x%08x    %2d-%08d   0x%08x\r\n",
            block->used ? '*':'-', BLK_TO_MEM(block), block, block_size, block->level, block_id, block_buddy); */
        if (block_buddy && (block->used == BLK_STAT_FREE) && (block_buddy->used == BLK_STAT_FREE)
            && (block->level == block_buddy->level) && (block->level != 0))
        {
            os_kprintf("mem_check ERR!!! (blk_lvl:%d) buddy block 0x%8x and 0x%8x both free?\r\n", block->level, block, block_buddy);

            ret = OS_ERROR;
            break;
        }

        block = (struct buddy_block *)((os_uint8_t *)block + block_size);
    }

    if (OS_EOK == ret)
    {
        os_kprintf("free block info:\r\n");
        os_kprintf("-blk_addr    blk_size    mem_size\r\n");

        for (level = 0; level < h->level_num; level++)
        {
            os_list_for_each_entry(block, &h->freelist[level], struct buddy_block, list_node)
            {
                block_size = h->level_size[block->level];
                os_kprintf("0x%08x  0x%08x  0x%08x\r\n", block, block_size, block_size - BLK_HEAD_SIZE);
            }
        }

        if ((size_total != h_mem->mem_total) || (size_used != h_mem->mem_used))
        {
            os_kprintf("mem_check ERR!!! size_total:%lu, mem_total:%lu, size_used:%lu mem_used:%lu\r\n",
                    size_total, h_mem->mem_total, size_used, h_mem->mem_used);

            ret = OS_ERROR;
        }
        else
        {
            os_kprintf("memory addr     : 0x%8x\r\n", h_mem->header);
            os_kprintf("memory total    : %lu\r\n", h_mem->mem_total);
            os_kprintf("memory used     : %lu\r\n", h_mem->mem_used);
            os_kprintf("memory max used : %lu\r\n", h_mem->mem_maxused);
            os_kprintf("mem_check ok!\r\n");
        }
    }

    (void)os_sem_post(&h_mem->sem);

    return ret;
}

#ifdef OS_USING_MEM_TRACE
static os_err_t _k_buddy_mem_trace(struct heap_mem *h_mem)
{
    struct buddy_heap *h = h_mem->header;
    struct buddy_block *block;
    os_size_t size_used;
    os_size_t size_total;
    os_size_t block_size;
    os_task_t*task;
    os_err_t  ret;

    os_kprintf("mem_trace for memory addr: 0x%8x ~ 0x%8x\r\n",
               (os_size_t)h_mem->header, (os_size_t)h_mem->header + h_mem->mem_total);

    os_kprintf("used   mem_addr    blk_addr    blk_size   blk_lvl   task\r\n");
    block = (struct buddy_block *)h->blk_start;
    size_used = ((os_size_t)h->blk_start - (os_size_t)h);
    size_total = size_used;

    ret = OS_EOK;

    (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

    while (block < h->blk_end)
    {
        OS_ASSERT_EX(block->magic_tag == BLK_MAGIC_TAG, 
            "unexpected header info (been write illegal?) for memory at %p", BLK_TO_MEM(block));

        block_size = h->level_size[block->level];
        if (block->used)
        {
            size_used += block_size;
        }
        size_total += block_size;

        task = block->used ? (os_task_t *)GET_MEM_TASK(block) : OS_NULL;
        os_kprintf("%c     0x%08x  0x%08x  0x%08x    %2d   %s\r\n",
                    block->used ? '*':'-', BLK_TO_MEM(block), block, block_size, block->level, 
                    task ? os_task_name(task): "--");

        block = (struct buddy_block *)((os_uint8_t *)block + block_size);
    }

    if ((size_total != h_mem->mem_total) || (size_used != h_mem->mem_used))
    {
        os_kprintf("mem_trace ERR!!! size_total:%lu, mem_total:%lu, size_used:%lu mem_used:%lu\r\n",
                   size_total, h_mem->mem_total, size_used, h_mem->mem_used);

        ret = OS_ERROR;
    }

    (void)os_sem_post(&h_mem->sem);

    return ret;
}
#endif

void k_buddy_mem_init(struct heap_mem *h_mem, void *start_addr, os_size_t size)
{
    _k_buddy_mem_init(h_mem, start_addr, size);

    h_mem->k_alloc         = _k_buddy_mem_alloc;
    h_mem->k_aligned_alloc = _k_buddy_mem_aligned_alloc;
    h_mem->k_free          = _k_buddy_mem_free;
    h_mem->k_realloc       = _k_buddy_mem_realloc;
    h_mem->k_ptr_to_size   = _k_buddy_mem_ptr_to_size;
    h_mem->k_deinit        = _k_buddy_mem_deinit;
    h_mem->k_mem_check     = _k_buddy_mem_check;
#ifdef OS_USING_MEM_TRACE
    h_mem->k_mem_trace     = _k_buddy_mem_trace;
#endif
}

