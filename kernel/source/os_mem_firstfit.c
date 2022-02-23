/*
 * Copyright (c) 2019 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
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
 * @file        os_mem_firstfit.c
 *
 * @brief       This file implements firstfit memory algorithm.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS team      Add some new function, such as realloc, mem_check, mem_trace.
 * 2020-12-18   OneOS team      fix some bug in aligned_alloc.
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_stddef.h>
#include <os_sem.h>
#include <os_assert.h>
#include <os_errno.h>
#include <os_memory.h>
#include <string.h>

#include "os_kernel_internal.h"

#define CONFIG_SYS_HEAP_ALLOC_LOOPS     8
#define CONFIG_SYS_HEAP_VALIDATE

#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define FIRSTFIT_MEM_INFO_INIT(x, size) do {\
    x->mem_total = (size);                  \
    x->mem_used = 0;                        \
    x->mem_maxused = 0;                     \
} while(0)

#define FIRSTFIT_MEM_USED_INC(x, size)  do {\
    x->mem_used += (size);                  \
    if (x->mem_used > x->mem_maxused) {     \
        x->mem_maxused = x->mem_used;       \
    }                                       \
}while(0)

#define FIRSTFIT_MEM_USED_DEC(x, size)  do {\
    x->mem_used -= (size);                  \
}while(0)

/**
 * @brief Value of @p x rounded up to the next multiple of @p align,
 *        which must be a power of 2.
 */
#define ROUND_UP(x, align)                                   \
    (((unsigned long)(x) + ((unsigned long)(align) - 1)) & \
     ~((unsigned long)(align) - 1))

#define ROUND_DOWN(x, align)                                 \
    ((unsigned long)(x) & ~((unsigned long)(align) - 1))

/* Theese validation checks are non-trivially expensive, so enable
 * only when debugging the heap code.  They shouldn't be routine
 * assertions.
 */
#ifdef CONFIG_SYS_HEAP_VALIDATE
#define CHECK(x) OS_ASSERT(x)
#else
#define CHECK(x)
#endif

/* Chunks are identified by their offset in 8 byte units from the
 * first address in the buffer (a zero-valued chunkid_t is used as a
 * null; that chunk would always point into the metadata at the start
 * of the heap and cannot be allocated).  They are prefixed by a
 * variable size header that depends on the size of the heap.  Heaps
 * with fewer than 2^15 units (256kb) of storage use shorts to store
 * the fields, otherwise the units are 32 bit integers for a 16Gb heap
 * space (larger spaces really aren't in scope for this code, but
 * could be handled similarly I suppose).  Because of that design
 * there's a certain amount of boilerplate API needed to expose the
 * field accessors since we can't use natural syntax.
 *
 * The fields are:
 *   SIZE_AND_USED: the total size (including header) of the chunk in
 *                  8-byte units.  The bottom bit stores a "used" flag.
 *   LEFT_SIZE: The size of the left (next lower chunk in memory)
 *              neighbor chunk.
 *   FREE_PREV: Chunk ID of the previous node in a free list.
 *   FREE_NEXT: Chunk ID of the next node in a free list.
 *
 * The free lists are circular lists, one for each power-of-two size
 * category.  The free list pointers exist only for free chunks,
 * obviously.  This memory is part of the user's buffer when
 * allocated.
 */
typedef os_size_t chunkid_t;

#define CHUNK_UNIT 8U

typedef struct { char bytes[CHUNK_UNIT]; } chunk_unit_t;

#ifdef OS_USING_MEM_TRACE
/* If it's using chunk, the chunk field is: |LEFT_SIZE|SIZE_AND_USED|TASK_ID  |databuf  |
   If it's free chunk, the chunk field is : |LEFT_SIZE|SIZE_AND_USED|FREE_PREV|FREE_NEXT|*/
enum chunk_fields { LEFT_SIZE, SIZE_AND_USED, TASK_ID, FREE_PREV = TASK_ID, FREE_NEXT };
#else
/* If it's using chunk, the chunk field is: |LEFT_SIZE|SIZE_AND_USED|databuf  |
   If it's free chunk, the chunk field is : |LEFT_SIZE|SIZE_AND_USED|FREE_PREV|FREE_NEXT|*/
enum chunk_fields { LEFT_SIZE, SIZE_AND_USED, FREE_PREV, FREE_NEXT };
#endif
struct z_heap_bucket {
    chunkid_t next;
};

struct z_heap {
    os_uint64_t chunk0_hdr_area;  /* matches the largest header */
    os_uint32_t len;
    os_uint32_t avail_buckets;
    struct z_heap_bucket *buckets;
};

OS_INLINE os_bool_t _k_big_heap_chunks(os_size_t chunks)
{
    return (sizeof(void *) > 4U || chunks > 0x7fffU);
}

OS_INLINE os_bool_t _k_big_heap_bytes(os_size_t bytes)
{
    return _k_big_heap_chunks(bytes / CHUNK_UNIT);
}

OS_INLINE os_bool_t _k_big_heap(struct z_heap *h)
{
    return _k_big_heap_chunks(h->len);
}

OS_INLINE chunk_unit_t *_k_chunk_buf(struct z_heap *h)
{
    /* the struct z_heap matches with the first chunk */
    return (chunk_unit_t *)h;
}

OS_INLINE os_size_t _k_chunk_field(struct z_heap *h, chunkid_t c, enum chunk_fields f)
{
    chunk_unit_t *buf;
    void         *cmem;
    os_size_t     val;

    buf  = _k_chunk_buf(h);
    cmem = &buf[c];

    if (_k_big_heap(h))
    {
        val = ((os_uint32_t *)cmem)[f];
    }
    else
    {
        val = ((os_uint16_t *)cmem)[f];
    }

    return val;
}

OS_INLINE void _k_chunk_set(struct z_heap *h, chunkid_t c, enum chunk_fields f, chunkid_t val)
{
    chunk_unit_t *buf;
    void         *cmem;

    CHECK(c <= h->len);

    buf  = _k_chunk_buf(h);
    cmem = &buf[c];

    if (_k_big_heap(h))
    {
        CHECK(val == (os_uint32_t)val);
        ((os_uint32_t *)cmem)[f] = val;
    }
    else
    {
        CHECK(val == (os_uint16_t)val);
        ((os_uint16_t *)cmem)[f] = val;
    }
}

OS_INLINE os_bool_t _k_chunk_used(struct z_heap *h, chunkid_t c)
{
    return (_k_chunk_field(h, c, SIZE_AND_USED) & 1U);
}

OS_INLINE os_size_t _k_chunk_size(struct z_heap *h, chunkid_t c)
{
    return (_k_chunk_field(h, c, SIZE_AND_USED) >> 1);
}

#ifdef OS_USING_MEM_TRACE
OS_INLINE void _k_set_chunk_task(struct z_heap *h, chunkid_t c, os_uint32_t val)
{
    chunk_unit_t *buf;
    void         *cmem;
    os_uint16_t  *ptr;

    CHECK(c <= h->len);

    buf  = _k_chunk_buf(h);
    cmem = &buf[c];

    if (_k_big_heap(h))
    {
        CHECK(val == (os_uint32_t)val);
        ((os_uint32_t *)cmem)[TASK_ID] = val;
    }
    else
    {
        ptr = (os_uint16_t *)cmem + TASK_ID;
        *ptr       = (os_uint16_t)((val >> 16) & 0x0000FFFF);
        *(ptr + 1) = (os_uint16_t)(val & 0x0000FFFF);
    }
}

OS_INLINE os_uint32_t _k_get_chunk_task(struct z_heap *h, chunkid_t c)
{
    chunk_unit_t *buf;
    void         *cmem;
    os_uint16_t  *ptr;
    os_uint32_t   val;

    CHECK(c <= h->len);

    buf  = _k_chunk_buf(h);
    cmem = &buf[c];

    if (_k_big_heap(h))
    {
        val = ((os_uint32_t *)cmem)[TASK_ID];
    }
    else
    {
        ptr = (os_uint16_t *)cmem + TASK_ID;
        val = ((((os_uint32_t)*ptr << 16) & 0xFFFF0000U) | (((os_uint32_t)*(ptr + 1)) & 0x0000FFFFU));
    }

    return val;
}
#endif

OS_INLINE void _k_set_chunk_used(struct z_heap *h, chunkid_t c, os_bool_t used)
{
    chunk_unit_t *buf;
    void         *cmem;

    buf  = _k_chunk_buf(h);
    cmem = &buf[c];

    if (_k_big_heap(h))
    {
        if (used)
        {
            ((os_uint32_t *)cmem)[SIZE_AND_USED] |= 1U;
        }
        else
        {
            ((os_uint32_t *)cmem)[SIZE_AND_USED] &= ~1U;
        }
    }
    else 
    {
        if (used)
        {
            ((os_uint16_t *)cmem)[SIZE_AND_USED] |= 1U;
        }
        else
        {
            ((os_uint16_t *)cmem)[SIZE_AND_USED] &= ~1U;
        }
    }
}

/*
 * Note: no need to preserve the used bit here as the chunk is never in use
 * when its size is modified, and potential _k_set_chunk_used() is always
 * invoked after _k_set_chunk_size().
 */
OS_INLINE void _k_set_chunk_size(struct z_heap *h, chunkid_t c, os_size_t size)
{
    _k_chunk_set(h, c, SIZE_AND_USED, size << 1);
}

OS_INLINE chunkid_t _k_prev_free_chunk(struct z_heap *h, chunkid_t c)
{
    return _k_chunk_field(h, c, FREE_PREV);
}

OS_INLINE chunkid_t _k_next_free_chunk(struct z_heap *h, chunkid_t c)
{
    return _k_chunk_field(h, c, FREE_NEXT);
}

OS_INLINE void _k_set_prev_free_chunk(struct z_heap *h, chunkid_t c,
                       chunkid_t prev)
{
    _k_chunk_set(h, c, FREE_PREV, prev);
}

OS_INLINE void _k_set_next_free_chunk(struct z_heap *h, chunkid_t c,
                       chunkid_t next)
{
    _k_chunk_set(h, c, FREE_NEXT, next);
}

OS_INLINE chunkid_t _k_left_chunk(struct z_heap *h, chunkid_t c)
{
    return (c - _k_chunk_field(h, c, LEFT_SIZE));
}

OS_INLINE chunkid_t _k_right_chunk(struct z_heap *h, chunkid_t c)
{
    return (c + _k_chunk_size(h, c));
}

OS_INLINE void _k_set_left_chunk_size(struct z_heap *h, chunkid_t c,
                       os_size_t size)
{
    _k_chunk_set(h, c, LEFT_SIZE, size);
}

OS_INLINE os_bool_t _k_solo_free_header(struct z_heap *h, chunkid_t c)
{
    return (_k_big_heap(h) && _k_chunk_size(h, c) == 1U);
}

OS_INLINE os_size_t _k_chunk_header_bytes(struct z_heap *h)
{
#ifdef OS_USING_MEM_TRACE
    return _k_big_heap(h) ? (8 + sizeof(os_task_t *)) : (4 + sizeof(os_task_t *));
#else
    return _k_big_heap(h) ? 8 : 4;
#endif
}

OS_INLINE os_size_t _k_heap_footer_bytes(os_size_t size)
{
#ifdef OS_USING_MEM_TRACE
    return _k_big_heap_bytes(size) ? (8 + sizeof(os_task_t *)) : (4 + sizeof(os_task_t *));
#else
    return _k_big_heap_bytes(size) ? 8 : 4;
#endif
}

OS_INLINE os_size_t _k_chunksz(os_size_t bytes)
{
    return ((bytes + CHUNK_UNIT - 1U) / CHUNK_UNIT);
}

OS_INLINE os_size_t _k_bytes_to_chunksz(struct z_heap *h, os_size_t bytes)
{
    return _k_chunksz(_k_chunk_header_bytes(h) + bytes);
}

OS_INLINE os_size_t _k_chunksz_to_bytes(struct z_heap *h, os_size_t chunk_sz)
{
    return (chunk_sz * CHUNK_UNIT - _k_chunk_header_bytes(h));
}

OS_INLINE os_int32_t _k_min_chunk_size(struct z_heap *h)
{
#ifdef OS_USING_MEM_TRACE
    return _k_bytes_to_chunksz(h, 0);
#else
    return _k_bytes_to_chunksz(h, 1);
#endif
}

OS_INLINE os_int32_t _k_bucket_idx(struct z_heap *h, os_size_t sz)
{
    os_size_t usable_sz;
    os_int32_t b_idx;

    usable_sz = (sz - _k_min_chunk_size(h) + 1);
    b_idx = (31 - __builtin_clz(usable_sz));

    if (b_idx < 0)
    {
        os_kprintf("warning, invalid bucket_idx:%d sz:%d\r\n", b_idx, sz);
        b_idx = 0;
    }

    return b_idx;
}

static void *_k_chunk_mem(struct z_heap *h, chunkid_t c)
{
    chunk_unit_t *buf;
    os_uint8_t   *ret;

    buf = _k_chunk_buf(h);
    ret = ((os_uint8_t *)&buf[c]) + _k_chunk_header_bytes(h);

#ifndef OS_USING_MEM_TRACE
    CHECK(!(((os_size_t)ret) & (_k_big_heap(h) ? 7 : 3)));
#endif
    return ret;
}

static void _k_free_list_remove_bidx(struct z_heap *h, chunkid_t c, os_int32_t bidx)
{
    struct z_heap_bucket *b;
    chunkid_t             first;
    chunkid_t             second;

    b = &h->buckets[bidx];
    CHECK(!_k_chunk_used(h, c));
    CHECK(b->next != 0);
    CHECK(h->avail_buckets & (1 << bidx));

    if (_k_next_free_chunk(h, c) == c)
    {
        /* this is the last chunk */
        h->avail_buckets &= ~(1 << bidx);
        b->next = 0;
    }
    else
    {
        first  = _k_prev_free_chunk(h, c);
        second = _k_next_free_chunk(h, c);

        b->next = second;
        _k_set_next_free_chunk(h, first, second);
        _k_set_prev_free_chunk(h, second, first);
    }
}

static void _k_free_list_remove(struct z_heap *h, chunkid_t c)
{
    os_int32_t bidx;

    if (!_k_solo_free_header(h, c))
    {
        bidx = _k_bucket_idx(h, _k_chunk_size(h, c));
        _k_free_list_remove_bidx(h, c, bidx);
    }
}

static void _k_free_list_add_bidx(struct z_heap *h, chunkid_t c, os_int32_t bidx)
{
    struct z_heap_bucket *b;
    chunkid_t             first;
    chunkid_t             second;

    b = &h->buckets[bidx];
    if (b->next == 0U)
    {
        CHECK((h->avail_buckets & (1 << bidx)) == 0);

        /* Empty list, first item */
        h->avail_buckets |= (1 << bidx);
        b->next = c;
        _k_set_prev_free_chunk(h, c, c);
        _k_set_next_free_chunk(h, c, c);
    }
    else
    {
        CHECK(h->avail_buckets & (1 << bidx));

        /* Insert before (!) the "next" pointer */
        second = b->next;
        first  = _k_prev_free_chunk(h, second);

        _k_set_prev_free_chunk(h, c, first);
        _k_set_next_free_chunk(h, c, second);
        _k_set_next_free_chunk(h, first, c);
        _k_set_prev_free_chunk(h, second, c);
    }
}

static void _k_free_list_add(struct z_heap *h, chunkid_t c)
{
    os_int32_t bidx;

    if (!_k_solo_free_header(h, c))
    {
        bidx = _k_bucket_idx(h, _k_chunk_size(h, c));
        _k_free_list_add_bidx(h, c, bidx);
    }
}

/* Splits a chunk "lc" into a left chunk and a right chunk at "rc".
 * Leaves both chunks marked "free"
 */
static void _k_split_chunks(struct z_heap *h, chunkid_t lc, chunkid_t rc)
{
    os_size_t sz0;
    os_size_t lsz;
    os_size_t rsz;

    CHECK(rc > lc);
    CHECK(rc - lc < _k_chunk_size(h, lc));

    sz0 = _k_chunk_size(h, lc);
    lsz = rc - lc;
    rsz = sz0 - lsz;

    _k_set_chunk_size(h, lc, lsz);
    _k_set_chunk_size(h, rc, rsz);
    _k_set_left_chunk_size(h, rc, lsz);
    _k_set_left_chunk_size(h, _k_right_chunk(h, rc), rsz);
}

/* Does not modify free list */
static void _k_merge_chunks(struct z_heap *h, chunkid_t lc, chunkid_t rc)
{
    os_size_t newsz;

    newsz = _k_chunk_size(h, lc) + _k_chunk_size(h, rc);
    _k_set_chunk_size(h, lc, newsz);
    _k_set_left_chunk_size(h, _k_right_chunk(h, rc), newsz);
}

static void _k_free_chunk(struct z_heap *h, chunkid_t c)
{
    /* Merge with free right chunk? */
    if (!_k_chunk_used(h, _k_right_chunk(h, c)))
    {
        _k_free_list_remove(h, _k_right_chunk(h, c));
        _k_merge_chunks(h, c, _k_right_chunk(h, c));
    }

    /* Merge with free left chunk? */
    if (!_k_chunk_used(h, _k_left_chunk(h, c)))
    {
        _k_free_list_remove(h, _k_left_chunk(h, c));
        _k_merge_chunks(h, _k_left_chunk(h, c), c);
        c = _k_left_chunk(h, c);
    }

    _k_free_list_add(h, c);
}

/**
 * Count Trailing Zeros (ctz). same as __builtin_ctz(x)
 */
OS_INLINE os_uint8_t _k_ctz32(os_uint32_t x)
{
#if defined (__GNUC__)
    return __builtin_ctz(x);
#else
    os_uint8_t n;

    n = 0;
    if (0 == x)
    {
        n = 32U;
    }
    else
    {
        if (0 == (x & 0X0000FFFF))
        {
            x >>= 16;
            n += 16;
        }
        if (0 == (x & 0X000000FF))
        {
            x >>= 8;
            n += 8;
        }
        if (0 == (x & 0X0000000F))
        {
            x >>= 4;
            n += 4;
        }
        if (0 == (x & 0X00000003))
        {
            x >>= 2;
            n += 2;
        }
        if (0 == (x & 0X00000001))
        {
            n += 1;
        }
    }

    return n;
#endif
}

static chunkid_t _k_alloc_chunk(struct z_heap *h, os_size_t sz)
{
    struct z_heap_bucket *b;
    os_int32_t bi;
    os_size_t  bmask;
    chunkid_t  ret_c;

    bi = _k_bucket_idx(h, sz);
    b  = &h->buckets[bi];

    ret_c = 0;
    if (bi > _k_bucket_idx(h, h->len))
    {
        /* Invaild.*/
    }
    else
    {
        /* First try a bounded count of items from the minimal bucket
         * size.  These may not fit, trying (e.g.) three means that
         * (assuming that chunk sizes are evenly distributed[1]) we
         * have a 7/8 chance of finding a match, thus keeping the
         * number of such blocks consumed by allocation higher than
         * the number of smaller blocks created by fragmenting larger
         * ones.
         *
         * [1] In practice, they are never evenly distributed, of
         * course.  But even in pathological situations we still
         * maintain our constant time performance and at worst see
         * fragmentation waste of the order of the block allocated
         * only.
         */
        if (b->next)
        {
            chunkid_t  first;
            os_int32_t i;

            first = b->next;
            i     = CONFIG_SYS_HEAP_ALLOC_LOOPS;
            do {
                chunkid_t c;

                c = b->next;
                if (_k_chunk_size(h, c) >= sz)
                {
                    _k_free_list_remove_bidx(h, c, bi);
                    ret_c = c;
                    break;
                }
                b->next = _k_next_free_chunk(h, c);
                CHECK(b->next != 0);
            } while (--i && b->next != first);
        }

        if (0 == ret_c)
        {
            /* Otherwise pick the smallest non-empty bucket guaranteed to
             * fit and use that unconditionally.
             */
            bmask = h->avail_buckets & ~((1 << (bi + 1)) - 1);

            if ((bmask & h->avail_buckets) != 0U)
            {
                os_int32_t minbucket;
                chunkid_t  c;

                minbucket = _k_ctz32(bmask & h->avail_buckets);
                c         = h->buckets[minbucket].next;
                _k_free_list_remove_bidx(h, c, minbucket);
                CHECK(_k_chunk_size(h, c) >= sz);
                ret_c = c;
            }
        }
    }

    return ret_c;
}

/*
 * Return the closest chunk ID corresponding to given memory pointer.
 * Here "closest" is only meaningful in the context of sys_heap_aligned_alloc()
 * where wanted alignment might not always correspond to a chunk header
 * boundary.
 */
static chunkid_t _k_mem_to_chunkid(struct z_heap *h, void *p)
{
    os_uint8_t *mem;
    os_uint8_t *base;

    mem  = p;
    base = (os_uint8_t *)_k_chunk_buf(h);

    return (mem - _k_chunk_header_bytes(h) - base) / CHUNK_UNIT;
}

static void _k_firstfit_mem_init(struct heap_mem *h_mem, void *mem, os_size_t bytes)
{
    os_ubase_t     addr; 
    os_ubase_t     end;
    os_size_t      buf_sz;
    struct z_heap *h;
    os_int32_t     nb_buckets;
    os_int32_t     i;
    os_size_t      chunk0_size;

    os_sem_init(&h_mem->sem, "mem_f_sem", 1, 1);

    /* Must fit in a 32 bit count of HUNK_UNIT */
    OS_ASSERT_EX(bytes / CHUNK_UNIT <= 0xffffffffU, "mem size is too big");

    /* Reserve the final marker chunk's header */
    OS_ASSERT_EX(bytes > _k_heap_footer_bytes(bytes), "mem size is too small");
    bytes -= _k_heap_footer_bytes(bytes);

    /* Round the start up, the end down */
    addr   = ROUND_UP(mem, CHUNK_UNIT);
    end    = ROUND_DOWN((os_uint8_t *)mem + bytes, CHUNK_UNIT);
    buf_sz = (end - addr) / CHUNK_UNIT;

    CHECK(end > addr);
    FIRSTFIT_MEM_INFO_INIT(h_mem, buf_sz * CHUNK_UNIT);
    OS_ASSERT_EX(buf_sz > _k_chunksz(sizeof(struct z_heap)), "mem size is too small");

    h                  = (struct z_heap *)addr;
    h->chunk0_hdr_area = 0;
    h->len             = buf_sz;
    h->avail_buckets   = 0;
    h->buckets         = (void *)(addr + CHUNK_UNIT * _k_chunksz(sizeof(struct z_heap)));
    h_mem->header      = h;

    nb_buckets  = _k_bucket_idx(h, buf_sz) + 1;
    chunk0_size = _k_chunksz(sizeof(struct z_heap) + nb_buckets * sizeof(struct z_heap_bucket));

    OS_ASSERT_EX(chunk0_size + _k_min_chunk_size(h) < buf_sz, "mem size is too small");

    for (i = 0; i < nb_buckets; i++)
    {
        h->buckets[i].next = 0;
    }

    /* chunk containing our struct z_heap */
    _k_set_chunk_size(h, 0, chunk0_size);
    FIRSTFIT_MEM_USED_INC(h_mem, chunk0_size * CHUNK_UNIT);
    _k_set_chunk_used(h, 0, OS_TRUE);

    /* chunk containing the free heap */
    _k_set_chunk_size(h, chunk0_size, buf_sz - chunk0_size);
    _k_set_left_chunk_size(h, chunk0_size, chunk0_size);

    /* the end marker chunk */
    _k_set_chunk_size(h, buf_sz, 0);
    _k_set_left_chunk_size(h, buf_sz, buf_sz - chunk0_size);
    _k_set_chunk_used(h, buf_sz, OS_TRUE);

    _k_free_list_add(h, chunk0_size);
}

static void *_k_firstfit_mem_alloc(struct heap_mem *h_mem, os_size_t bytes)
{
    struct z_heap *h;
    os_size_t      chunk_sz;
    chunkid_t      c;
    void          *mem;

    mem = OS_NULL;
    if (0U != bytes)
    {
        h        = h_mem->header;
        chunk_sz = _k_bytes_to_chunksz(h, bytes);

        (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

        c = _k_alloc_chunk(h, chunk_sz);
        if (0U == c)
        {
            /* no suitable mem. */
            (void)os_sem_post(&h_mem->sem);
        }
        else
        {
            /* Split off remainder if any */
            if (_k_chunk_size(h, c) > chunk_sz)
            {
                _k_split_chunks(h, c, c + chunk_sz);
                _k_free_list_add(h, c + chunk_sz);
            }
            FIRSTFIT_MEM_USED_INC(h_mem, _k_chunk_size(h, c) * CHUNK_UNIT);
#ifdef OS_USING_MEM_TRACE
            _k_set_chunk_task(h, c, (os_uint32_t)os_task_self());
#endif
            _k_set_chunk_used(h, c, OS_TRUE);

            (void)os_sem_post(&h_mem->sem);

            mem = _k_chunk_mem(h, c);
        }
    }

    return mem;
}

static void _k_firstfit_mem_free(struct heap_mem *h_mem, void *mem)
{
    struct z_heap *h;
    chunkid_t      c;

    if (mem)
    {
        OS_ASSERT_EX((mem >= h_mem->header) && (((os_ubase_t)mem - (os_ubase_t)h_mem->header) <= h_mem->mem_total),
             "unexpected mem addr (invalid addr?) for memory at %p", mem);

        h = h_mem->header;
        c = _k_mem_to_chunkid(h, mem);

        (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);
        /*
         * This should catch many double-free cases.
         * This is cheap enough so let's do it all the time.
         */
        OS_ASSERT_EX(_k_chunk_used(h, c),
             "unexpected mem state (double-free? or invalid addr?) for memory at %p", mem);

        /*
         * It is easy to catch many common memory overflow cases with
         * a quick check on this and next chunk header fields that are
         * immediately before and after the freed memory.
         */
        OS_ASSERT_EX(_k_left_chunk(h, _k_right_chunk(h, c)) == c,
             "corrupted mem bounds (buffer overflow?) for memory at %p", mem);

        FIRSTFIT_MEM_USED_DEC(h_mem, _k_chunk_size(h, c) * CHUNK_UNIT);

        _k_set_chunk_used(h, c, OS_FALSE);
        _k_free_chunk(h, c);

        (void)os_sem_post(&h_mem->sem);
    }
}

static void *_k_firstfit_mem_aligned_alloc(struct heap_mem *h_mem, os_size_t align, os_size_t bytes)
{
    struct z_heap *h;
    os_size_t      alloc_sz;
    os_size_t      padded_sz;
    chunkid_t      c0;
    chunkid_t      c;
    void          *mem;
    void          *mem_bound;

    h = h_mem->header;
    OS_ASSERT_EX((align & (align - 1)) == 0, "unexpected align: %lu (should be the power of 2)", align);

    mem = OS_NULL;
    if (0U != bytes)
    {
        if (align <= OS_ALIGN_SIZE)
        {
            mem = _k_firstfit_mem_alloc(h_mem, bytes);
        }
        else
        {
            /*
             * Find a free block that is guaranteed to fit.
             * We over-allocate to account for alignment and then free
             * the extra allocations afterwards.
             */
            alloc_sz  = _k_bytes_to_chunksz(h, bytes);
            padded_sz = _k_bytes_to_chunksz(h, bytes + align - 1);

            (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

            c0 = _k_alloc_chunk(h, padded_sz);

            if (0U != c0)
            {
                /* Align allocated memory */
                mem = _k_chunk_mem(h, c0);
                mem = (void *) ROUND_UP(mem, align);

                /* Get corresponding chunk */
                c = _k_mem_to_chunkid(h, mem);
                CHECK(c >= c0 && c  < c0 + padded_sz);

                /* Split and free unused prefix */
                if (c > c0)
                {
                    _k_split_chunks(h, c0, c);
                    _k_free_list_add(h, c0);
                }

                /* Split and free unused suffix */
                mem_bound = _k_chunk_mem(h, c);
                /* If mem not CHUNK_UNIT aligned, the alloc_sz should reseve one more chunk. */
                if (mem_bound != mem)
                {
                    alloc_sz++;
                }
                if (_k_chunk_size(h, c) > alloc_sz)
                {
                    _k_split_chunks(h, c, c + alloc_sz);
                    _k_free_list_add(h, c + alloc_sz);
                }
                FIRSTFIT_MEM_USED_INC(h_mem, _k_chunk_size(h, c) * CHUNK_UNIT);
#ifdef OS_USING_MEM_TRACE
                _k_set_chunk_task(h, c, (os_uint32_t)os_task_self());
#endif
                _k_set_chunk_used(h, c, OS_TRUE);
            }

            (void)os_sem_post(&h_mem->sem);
        }
    }

    return mem;
}

static void *_k_firstfit_mem_realloc(struct heap_mem *h_mem, void *mem, os_size_t bytes)
{
    struct z_heap *h;
    os_size_t chunk_sz_new;
    os_size_t chunk_sz;
    os_size_t r_chunk_sz;
    chunkid_t c;
    chunkid_t rc;
    chunkid_t split_size;
    chunkid_t newsz;
    void     *mem_new;

    mem_new = OS_NULL;

    if (OS_NULL == mem)
    {
        mem_new = _k_firstfit_mem_alloc(h_mem, bytes);
    }
    else if (0U == bytes)
    {
        _k_firstfit_mem_free(h_mem, mem);
    }
    else
    {
        OS_ASSERT_EX((mem >= h_mem->header) && (((os_ubase_t)mem - (os_ubase_t)h_mem->header) <= h_mem->mem_total),
             "unexpected mem addr (invalid addr?) for memory at %p", mem);

        h = h_mem->header;
        chunk_sz_new = _k_bytes_to_chunksz(h, bytes);
        c = _k_mem_to_chunkid(h, mem);

        (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);
        /*
         * This should catch many double-free cases.
         * This is cheap enough so let's do it all the time.
         */
        OS_ASSERT_EX(_k_chunk_used(h, c),
             "unexpected heap state (already free? or invalid addr?) for memory at %p", mem);

        /*
         * It is easy to catch many common memory overflow cases with
         * a quick check on this and next chunk header fields that are
         * immediately before and after the freed memory.
         */
        OS_ASSERT_EX(_k_left_chunk(h, _k_right_chunk(h, c)) == c,
             "corrupted heap bounds (buffer overflow?) for memory at %p", mem);

        chunk_sz = _k_chunk_size(h, c);
        CHECK(chunk_sz > 0);

        if (chunk_sz > chunk_sz_new)
        {
            /* Split off the old. */
            _k_split_chunks(h, c, c + chunk_sz_new);
            FIRSTFIT_MEM_USED_DEC(h_mem, (chunk_sz - chunk_sz_new) * CHUNK_UNIT);
#ifdef OS_USING_MEM_TRACE
            _k_set_chunk_task(h, c, (os_uint32_t)os_task_self());
#endif
            _k_set_chunk_used(h, c, OS_TRUE);
            _k_free_chunk(h, c + chunk_sz_new);

            (void)os_sem_post(&h_mem->sem);

            mem_new = _k_chunk_mem(h, c);
        }
        else if (chunk_sz < chunk_sz_new)
        {
            rc = _k_right_chunk(h, c);
            r_chunk_sz = _k_chunk_size(h, rc);

            if (!_k_chunk_used(h, rc) && (chunk_sz + r_chunk_sz >= chunk_sz_new))
            {
                /* Expand: split the right chunk and append */
                split_size = chunk_sz_new - chunk_sz;

                _k_free_list_remove(h, rc);
                /* If split_size < r_chunk_sz, split rc; if split_size = r_chunk_sz, not need split */
                if (split_size < r_chunk_sz)
                {
                    _k_split_chunks(h, rc, rc + split_size);
                    _k_free_list_add(h, rc + split_size);
                }
                newsz = chunk_sz + split_size;

                _k_set_chunk_size(h, c, newsz);
                FIRSTFIT_MEM_USED_INC(h_mem, split_size * CHUNK_UNIT);
#ifdef OS_USING_MEM_TRACE
                _k_set_chunk_task(h, c, (os_uint32_t)os_task_self());
#endif
                _k_set_chunk_used(h, c, OS_TRUE);
                _k_set_left_chunk_size(h, c + newsz, newsz);

                (void)os_sem_post(&h_mem->sem);

                mem_new = _k_chunk_mem(h, c);
           }
           else
           {
                (void)os_sem_post(&h_mem->sem);

                /* alloc new, and copy old to new, then free old. */
                mem_new = _k_firstfit_mem_alloc(h_mem, bytes);
                if (mem_new)
                {
                    (void)memcpy(mem_new, mem, _k_chunksz_to_bytes(h, chunk_sz));
                    _k_firstfit_mem_free(h_mem, mem);
                }
            }
        }
        else
        {
            (void)os_sem_post(&h_mem->sem);

            /* Same as old, do nothing. */
            mem_new = mem;
        }
    }

    return mem_new;
}

static os_size_t _k_firstfit_mem_ptr_to_size(struct heap_mem *h_mem, void *mem)
{
    struct z_heap *h;
    chunkid_t      c;
    os_size_t      chunk_sz;

    h = h_mem->header;
    c = _k_mem_to_chunkid(h, mem);
    chunk_sz = _k_chunk_size(h, c);
    return _k_chunksz_to_bytes(h, chunk_sz);
}

static void _k_firstfit_mem_deinit(struct heap_mem *h_mem)
{
    os_sem_deinit(&h_mem->sem);
}

static os_err_t _k_firstfit_mem_check(struct heap_mem *h_mem)
{
    struct z_heap *h;
    chunkid_t  c;
    chunkid_t  rc;
    chunkid_t  rc_lc;
    os_size_t  chunksize;
    os_size_t  chunksize_used;
    os_size_t  chunksize_total;
    void      *c_addr;
    void      *rc_addr;
    os_err_t   ret;
    os_int32_t i;
    os_int32_t nb_buckets;

    h = h_mem->header;
    c = 0;
    chunksize = 0;
    chunksize_used = 0;
    chunksize_total = 0;
    ret = OS_EOK;
    os_kprintf("mem_check for memory addr: 0x%8x ~ 0x%8x\r\n", (os_size_t)h_mem->header, (os_size_t)h_mem->header + h_mem->mem_total);

    (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

    chunksize = _k_chunk_size(h, c);
    /*os_kprintf("used  chunkid  chunksize\r\n"); */
    while (chunksize > 0)
    {
        /*os_kprintf("  %c  %8d  %8d\r\n", _k_chunk_used(h, c) ? '*' : '-', c , chunksize); */
        if (_k_chunk_used(h, c))
        {
            chunksize_used += chunksize;
        }
        chunksize_total += chunksize;
        rc    = _k_right_chunk(h, c);
        rc_lc = _k_left_chunk(h, rc);
        if (rc_lc != c)
        {
            c_addr  = ((os_uint8_t *)_k_chunk_mem(h, c) - _k_chunk_header_bytes(h));
            rc_addr = ((os_uint8_t *)_k_chunk_mem(h, rc) - _k_chunk_header_bytes(h));
            os_kprintf("mem_check err:chunk:%lu, r_chunk:%lu, r_chunk's left:%lu\r\n", c, rc, rc_lc);
            os_kprintf("the addr:0x%x or 0x%x maybe overwrited! please check.\r\n", rc_addr, c_addr);

            ret = OS_ERROR;
            break;
        }

        c = _k_right_chunk(h, c);
        chunksize = _k_chunk_size(h, c);
    }

    if (OS_EOK == ret)
    {
        os_kprintf("free block info:\r\n");
        os_kprintf("bucketid   chunkid   chunk_size   mem_size\r\n");

        nb_buckets = _k_bucket_idx(h, h->len) + 1;
        for (i = 0; i < nb_buckets; i++)
        {
            chunkid_t first;

            first = h->buckets[i].next;
            /*os_kprintf("bucket[%d] for chunk count %d ~ %d\r\n", i, (1 << i) - 1 + _k_min_chunk_size(h), (1 << (i + 1)) - 1 + _k_min_chunk_size(h)); */
            if (first)
            {
                os_size_t c_size;
                os_size_t mem_size;
                chunkid_t curr;

                curr = first;
                do {
                    c_size = _k_chunk_size(h, curr);
                    mem_size = (c_size * CHUNK_UNIT - _k_chunk_header_bytes(h));
                    os_kprintf("  %2d    %8lu    %8lu     0x%08lx\r\n", i, curr , c_size, mem_size);
                    curr = _k_next_free_chunk(h, curr);
                } while (curr != first);
            }
        }

        if (((chunksize_total * CHUNK_UNIT) != h_mem->mem_total) || ((chunksize_used * CHUNK_UNIT) != h_mem->mem_used))
        {
            os_kprintf("mem_check err:size_total:%lu, mem_total:%lu, size_used:%lu mem_used:%lu\r\n",
                       (os_size_t)(chunksize_total * CHUNK_UNIT),
                       h_mem->mem_total,
                       (os_size_t)(chunksize_used * CHUNK_UNIT),
                       h_mem->mem_used);
                       
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

#if defined(OS_USING_MEM_TRACE)
static os_err_t _k_firstfit_mem_trace(struct heap_mem *h_mem)
{
    struct z_heap *h;
    chunkid_t   c;
    os_size_t   chunksize;
    os_size_t   mem_header;
    os_size_t   mem_addr;
    os_uint32_t mem_size;
    os_uint32_t header_bytes;
    os_task_t*  task;
    os_err_t    ret;

    ret = OS_EOK;
    h = h_mem->header;
    header_bytes = _k_chunk_header_bytes(h);
    os_kprintf("mem_trace for memory addr: 0x%8x ~ 0x%8x\r\n", (os_size_t)h_mem->header, (os_size_t)h_mem->header + h_mem->mem_total);
    os_kprintf("used        addr          size    task\r\n");

    (void)os_sem_wait(&h_mem->sem, OS_WAIT_FOREVER);

    c = 0;
    chunksize = _k_chunk_size(h, c);
    while(chunksize > 0)
    {
        mem_header = (os_uint32_t)h + c * CHUNK_UNIT;
        mem_addr = mem_header + header_bytes;
        mem_size = _k_chunksz_to_bytes(h, chunksize);
        task = _k_chunk_used(h, c) ? (os_task_t*)_k_get_chunk_task(h, c) : OS_NULL;
        os_kprintf("%c        0x%8X    %8d    %s\r\n", _k_chunk_used(h, c) ? '*' : '-', mem_addr , mem_size, task ? os_task_name(task): "--");
        if (_k_left_chunk(h, _k_right_chunk(h, c)) != c)
        {
            os_kprintf("mem_trace err:chunk:%lu, right_chunk:%lu, left of right_chunk:%lu\r\n", c, _k_right_chunk(h, c), _k_left_chunk(h, _k_right_chunk(h, c)));
            ret = OS_ERROR;
            break;
        }

        c = _k_right_chunk(h, c);
        chunksize = _k_chunk_size(h, c);
    }

    (void)os_sem_post(&h_mem->sem);

    return ret;
}
#endif /* end of OS_USING_MEM_TRACE */

void k_firstfit_mem_init(struct heap_mem *h_mem, void *start_addr, os_size_t size)
{
    _k_firstfit_mem_init(h_mem, start_addr, size);

    h_mem->k_alloc         = _k_firstfit_mem_alloc;
    h_mem->k_aligned_alloc = _k_firstfit_mem_aligned_alloc;
    h_mem->k_free          = _k_firstfit_mem_free;
    h_mem->k_realloc       = _k_firstfit_mem_realloc;
    h_mem->k_ptr_to_size   = _k_firstfit_mem_ptr_to_size;
    h_mem->k_deinit        = _k_firstfit_mem_deinit;
    h_mem->k_mem_check     = _k_firstfit_mem_check;
#ifdef OS_USING_MEM_TRACE
    h_mem->k_mem_trace     = _k_firstfit_mem_trace;
#endif
}

