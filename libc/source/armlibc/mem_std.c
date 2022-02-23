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
 * @file        mem_std.c
 *
 * @brief       This file redirects the memory management interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-14   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_stddef.h>
#include <sys/types.h>
#include <os_memory.h>

#ifdef OS_USING_SYS_HEAP

#ifdef __CC_ARM
/* Avoid the heap and heap-using library functions supplied by arm. */
#pragma import(__use_no_heap)
#endif

void *malloc(size_t n)
{
#if defined(OS_USING_SYS_HEAP)
    return os_malloc(n);
#else
    return OS_NULL;
#endif
}

void *realloc(void *rmem, size_t newsize)
{
#if defined(OS_USING_SYS_HEAP)
    return os_realloc(rmem, newsize);
#else
    return OS_NULL;
#endif
}

void *calloc(size_t nelem, size_t elsize)
{
#if defined(OS_USING_SYS_HEAP)
    return os_calloc(nelem, elsize);
#else
    return OS_NULL;
#endif
}

void free(void *rmem)
{
#if defined(OS_USING_SYS_HEAP)
    if (OS_NULL != rmem) 
    {
        os_free(rmem);
    }
#endif
}
#endif
