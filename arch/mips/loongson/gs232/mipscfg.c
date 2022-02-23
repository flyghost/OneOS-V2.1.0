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
 * @file        mipscfg.c
 *
 * @brief       mips cfg file
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <mips.h>

mips32_core_cfg_t g_mips_core =
{
    16,     /* icache_line_size */
    256,    /* icache_lines_per_way */
    4,      /* icache_ways */
    16,     /* dcache_line_size */
    256,    /* dcache_lines_per_way */
    4,      /* dcache_ways */
    16,     /* max_tlb_entries */
};

static os_uint16_t m_pow(os_uint16_t b, os_uint16_t n)
{
    os_uint16_t rets = 1;

    while (n--)
        rets *= b;

    return rets;
}

static os_uint16_t m_log2(os_uint16_t b)
{
    os_uint16_t rets = 0;

    while (b != 1)
    {
        b /= 2;
        rets++;
    }

    return rets;
}

/**
 * read core attribute
 */
void mips32_cfg_init(void)
{
    os_uint16_t val;
    os_uint32_t cp0_config1;

    cp0_config1 = read_c0_config();
    if (cp0_config1 & 0x80000000)
    {
        cp0_config1 = read_c0_config1();

        val = (cp0_config1 & (7<<22))>>22;
        g_mips_core.icache_lines_per_way = 64 * m_pow(2, val);
        val = (cp0_config1 & (7<<19))>>19;
        g_mips_core.icache_line_size = 2 * m_pow(2, val);
        val = (cp0_config1 & (7<<16))>>16;
        g_mips_core.icache_ways = val + 1;

        val = (cp0_config1 & (7<<13))>>13;
        g_mips_core.dcache_lines_per_way = 64 * m_pow(2, val);
        val = (cp0_config1 & (7<<10))>>10;
        g_mips_core.dcache_line_size = 2 * m_pow(2, val);
        val = (cp0_config1 & (7<<7))>>7;
        g_mips_core.dcache_ways = val + 1;

        val = (cp0_config1 & (0x3F<<25))>>25;
        g_mips_core.max_tlb_entries = val + 1;
    }
}
