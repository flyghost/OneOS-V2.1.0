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
 * @file        rt_hw.c
 *
 * @brief       Implementation of RT-Thread adaper hardware API.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "rtdef.h"
#include "os_assert.h"

void rt_hw_show_memory(rt_uint32_t addr, rt_uint32_t size)
{
    int i;
    int j;

    OS_ASSERT(addr);

    i = 0;
    j = 0;

    addr = addr & ~0xF;
    size = 4 * ((size + 3) / 4);

    while (i < size)
    {
        os_kprintf("0x%08x: ", addr );

        for(j = 0; j < 4; j++)
        {
            os_kprintf("0x%08x  ", *(rt_uint32_t *)addr);

            addr += 4;
            i++;
        }

        os_kprintf("\r\n");
    }

    return;
}
