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
 * @file        mem_arch.c
 *
 * @brief       adpt memory functions for beken drivers.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-18   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "include.h"
#include "arm_arch.h"
#include <string.h>
#include <os_task.h>

extern void *os_malloc(os_size_t nbytes);
extern void       *os_memset(void *src, os_uint8_t val, os_size_t count);
extern os_int32_t  os_memcmp(const void *str1, const void *str2, os_size_t count);


#if (CFG_SUPPORT_RTT) && (CFG_SOC_NAME == SOC_BK7221U)
void *dtcm_malloc(size_t size)
{
	extern void *tcm_malloc(unsigned long size); 
    return (void *)tcm_malloc(size);
}
#endif

void * os_zalloc(size_t size)
{
	void *n = (void *)os_malloc(size);
	if (n)
		os_memset(n, 0, size);
	return n;
}

int os_memcmp_const(const void *a, const void *b, size_t len)
{
    return os_memcmp(a, b, len);
}

// EOF
