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
 * @file        mips_cache.c
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_assert.h>
#include "mips.h"

extern void cache_init(os_ubase_t cache_size, os_ubase_t cache_line_size);
void r4k_cache_init(void)
{
    //	cache_init(dcache_size, cpu_dcache_line_size);
}

void r4k_cache_flush_all(void)
{
    blast_dcache16();
    blast_icache16();
}


void r4k_icache_flush_all(void)
{
    blast_icache16();
}

void r4k_icache_flush_range(os_ubase_t addr, os_ubase_t size)
{
    os_ubase_t end, a;

    if (size > g_mips_core.icache_size)
    {
        blast_icache16();
    }
    else
    {
        os_ubase_t ic_lsize = g_mips_core.icache_line_size;

        a = addr & ~(ic_lsize - 1);
        end = ((addr + size) - 1) & ~(ic_lsize - 1);
        while (1)
        {
            flush_icache_line(a);
            if (a == end)
                break;
            a += ic_lsize;
        }
    }
}

void r4k_icache_lock_range(os_ubase_t addr, os_ubase_t size)
{
    os_ubase_t end, a;
    os_ubase_t ic_lsize = g_mips_core.icache_line_size;

    a = addr & ~(ic_lsize - 1);
    end = ((addr + size) - 1) & ~(ic_lsize - 1);
    while (1)
    {
        lock_icache_line(a);
        if (a == end)
            break;
        a += ic_lsize;
    }
}

void r4k_dcache_inv(os_ubase_t addr, os_ubase_t size)
{
    os_ubase_t end, a;
    os_ubase_t dc_lsize = g_mips_core.dcache_line_size;

    a = addr & ~(dc_lsize - 1);
    end = ((addr + size) - 1) & ~(dc_lsize - 1);
    while (1)
    {
        invalidate_dcache_line(a);
        if (a == end)
            break;
        a += dc_lsize;
    }
}

void r4k_dcache_wback_inv(os_ubase_t addr, os_ubase_t size)
{
    os_ubase_t end, a;

    if (size >= g_mips_core.dcache_size)
    {
        blast_dcache16();
    }
    else
    {
        os_ubase_t dc_lsize = g_mips_core.dcache_line_size;

        a = addr & ~(dc_lsize - 1);
        end = ((addr + size) - 1) & ~(dc_lsize - 1);
        while (1)
        {
            flush_dcache_line(a);
            if (a == end)
                break;
            a += dc_lsize;
        }
    }
}

#define dma_cache_wback_inv(start,size) \
    do { (void) (start); (void) (size); } while (0)
#define dma_cache_wback(start,size) \
    do { (void) (start); (void) (size); } while (0)
#define dma_cache_inv(start,size)   \
    do { (void) (start); (void) (size); } while (0)


void r4k_dma_cache_sync(os_ubase_t addr, os_size_t size, enum dma_data_direction direction)
{
    switch (direction)
    {
        case DMA_TO_DEVICE:
            r4k_dcache_wback_inv(addr, size);
            break;

        case DMA_FROM_DEVICE:
            r4k_dcache_wback_inv(addr, size);
            break;

        case DMA_BIDIRECTIONAL:
            dma_cache_wback_inv(addr, size);
            break;
        default:
            OS_ASSERT(0) ;
            break;
    }
}

