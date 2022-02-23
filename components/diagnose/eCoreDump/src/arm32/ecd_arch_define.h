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
 * @file        ecd_arch_define.h
 *
 * @brief       This file provides elf arch define for arm32.
 *architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-26   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ECD_ARCH_DEFINE_H__
#define __ECD_ARCH_DEFINE_H__

#include "ecd_registers_define.h"

#define ECOREDUMP_ELF_CLASS           ELFCLASS32
#define ECOREDUMP_ELF_ENDIAN          ELFDATA2LSB
#define ECOREDUMP_MACHINE             EM_ARM
#define ECOREDUMP_OSABI               ELFOSABI_ARM

#define ECOREDUMP_PRSTATUS_SIZE       148
#define ECOREDUMP_FPREGSET_SIZE       (32 * 8 + 4)

#define fill_note_prstatus_desc     arm32_fill_note_prstatus_desc
#define fill_note_fpregset_desc     arm32_fill_note_fpregset_desc

void arm32_fill_note_prstatus_desc(uint8_t * desc, core_regset_type * regset);
void arm32_fill_note_fpregset_desc(uint8_t * desc, fp_regset_type * regset);

#endif
