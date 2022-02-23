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
 * @file        cache.c
 *
 * @brief       This file provides cache functions related to the ARM M7 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <arch_cache.h>
#include <os_assert.h>
#include <board.h>

/* The L1-caches on all Cortex-M7s are divided into lines of 32 bytes. */
#define L1CACHE_LINESIZE_BYTE       (32)

/**
 ***********************************************************************************************************************
 * @brief           Enable I-Cache.
 *
 * @param           No parameter.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_cpu_icache_enable(void)
{
    SCB_EnableICache();
}

/**
 ***********************************************************************************************************************
 * @brief           Disable I-Cache.
 *
 * @param           No parameter.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_cpu_icache_disable(void)
{
    SCB_DisableICache();
}

/**
 ***********************************************************************************************************************
 * @brief           Get I-Cache status.
 *
 * @param           No parameter.
 *
 * @return          Unsupport,return 0.
 ***********************************************************************************************************************
 */
os_base_t os_cpu_icache_status(void)
{
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function handles cpu instruction cache operations.
 *
 * @param[in]       ops             Operation code,only support OS_HW_CACHE_INVALIDATE.
 * @param[in]       addr            Address to be operated.
 * @param[in]       size            The length to be operated.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_cpu_icache_ops(os_int32_t ops, void* addr, os_int32_t size)
{
    os_uint32_t address = ((os_uint32_t)addr) & ((os_uint32_t) ~(L1CACHE_LINESIZE_BYTE - 1));
    os_int32_t size_byte = size + address - (os_uint32_t)addr;
    os_uint32_t linesize = 32U;
    
    if (ops & OS_HW_CACHE_INVALIDATE)
    {
        __DSB();
        while (size_byte > 0)
        {
            SCB->ICIMVAU = address;
            address += linesize;
            size_byte -= linesize;
        }
        __DSB();
        __ISB();
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Enable D-Cache.
 *
 * @param           No parameter.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_cpu_dcache_enable(void)
{
    SCB_EnableDCache();
}

/**
 ***********************************************************************************************************************
 * @brief           Disable D-Cache.
 *
 * @param           No parameter.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_cpu_dcache_disable(void)
{
    SCB_DisableDCache();
}

/**
 ***********************************************************************************************************************
 * @brief           Get D-Cache status.
 *
 * @param           No parameter.
 *
 * @return          Unsupport,return 0.
 ***********************************************************************************************************************
 */
os_base_t os_cpu_dcache_status(void)
{
    return 0;
}

/**
 ***********************************************************************************************************************
 * @brief           This function handles cpu data cache operations.
 *
 * @param[in]       ops             Operation code.
 *                                  OS_HW_CACHE_FLUSH:Clean by address
 *                                  OS_HW_CACHE_INVALIDATE:Invalidate by address
 *                                  OS_HW_CACHE_FLUSH | OS_HW_CACHE_INVALIDATE:Clean and Invalidate by address
 * @param[in]       addr            Address to be operated.
 * @param[in]       size            The length to be operated.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_cpu_dcache_ops(os_int32_t ops, void* addr, os_int32_t size)
{
    os_uint32_t start_addr = ((os_uint32_t)addr) & ((os_uint32_t)~(L1CACHE_LINESIZE_BYTE - 1));
    os_uint32_t size_byte = size + (os_uint32_t)addr - start_addr;

    if ((ops & OS_HW_CACHE_FLUSH) && (ops & OS_HW_CACHE_INVALIDATE))
    {
        SCB_CleanInvalidateDCache_by_Addr((uint32_t *)start_addr, size_byte);
    }
    else if (ops & OS_HW_CACHE_FLUSH)
    {
        SCB_CleanDCache_by_Addr((uint32_t *)start_addr, size_byte);
    }
    else if (ops & OS_HW_CACHE_INVALIDATE)
    {
        SCB_InvalidateDCache_by_Addr((uint32_t *)start_addr, size_byte);
    }
    else
    {
        OS_ASSERT(0);
    }
}

