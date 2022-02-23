/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 *use this file except in compliance with the License. You may obtain a copy of
 *the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 *distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *License for the specific language governing permissions and limitations under
 *the License.
 *
 * @file        arm32.c
 *
 * @brief       This file provides arch specified function.
 *architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "ecd_arch_define.h"
#include "ecd_registers_define.h"

void arm32_fill_note_prstatus_desc(uint8_t *desc, core_regset_type *regset)
{
    static uint32_t task_id = 3539;
    uint16_t * signal = (uint16_t *)&desc[12];
    uint32_t * lwpid = (uint32_t *)&desc[24];

    memset(desc, 0, ECOREDUMP_PRSTATUS_SIZE);
    *signal = 0;
    *lwpid = task_id++;
    memcpy(desc + 72, regset, sizeof(core_regset_type));
}

void arm32_fill_note_fpregset_desc(uint8_t *desc, fp_regset_type *regset)
{
    if (regset != NULL)
    {
        memcpy(desc, regset, sizeof(fp_regset_type) - sizeof(uint32_t));
        memcpy(desc + 32 * 8, &regset->fpscr, sizeof(uint32_t));
    }
    else
    {
        memset(desc, 0, ECOREDUMP_FPREGSET_SIZE);
    }
}
