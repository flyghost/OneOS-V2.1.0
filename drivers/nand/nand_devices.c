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
 * @file        nand_devices.c
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-07-22    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include "nand.h"

static const struct nand_device_info nand_info_table[] =
{
#ifdef BSP_NAND_MT29F4G08ABADA
    {
        .name = "MT29F4G08ABADA",
        .id   = 0xaddc9095,
        .page_size  = 2048,
        .spare_size = 64,
        .block_size = 64,
        .plane_size = 2048,
        .plane_nr   = 2,
        .badflag_info.length = 1,
        .badflag_info.data = 0x00,
        .badflag_info.addr_offset = 0,
        .badflag_info.page_offset = 0,
        .hardecc_info.addr_offset = 1
    },
#endif
};

static const int nand_info_table_size = ARRAY_SIZE(nand_info_table);

const struct nand_device_info *get_nand_info_by_id(os_uint32_t id)
{
    int i;
    
    for (i = 0; i < nand_info_table_size; i++)
    {
        if (nand_info_table[i].id == id)
        {
            return &nand_info_table[i];
        }
    }

    return OS_NULL;
}

const struct nand_device_info *get_nand_info_by_name(const char *name)
{
    int i;
    
    for (i = 0; i < nand_info_table_size; i++)
    {
        if (strcmp(nand_info_table[i].name, name) == 0)
        {
            return &nand_info_table[i];
        }
    }

    return OS_NULL;
}

