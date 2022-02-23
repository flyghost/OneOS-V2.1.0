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
 * @file        mips_cfg.h
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef _MIPS_CFG_H_
#define _MIPS_CFG_H_

#ifndef __ASSEMBLY__
#include <stdint.h>
typedef struct mips32_core_cfg
{
    uint16_t icache_line_size;
    uint16_t icache_size;
    uint16_t dcache_line_size;
    uint16_t dcache_size;

    uint16_t max_tlb_entries;   /* number of tlb entry */
} mips32_core_cfg_t;

extern mips32_core_cfg_t g_mips_core;

#endif /* __ASSEMBLY__ */

#endif /* _MIPS_CFG_H_ */
