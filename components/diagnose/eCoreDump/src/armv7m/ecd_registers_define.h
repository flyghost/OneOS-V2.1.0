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
 * @file        ecd_registers_define.h
 *
 * @brief       This file provides registers set for armv7m.
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __ARMV7M_ECD_REGISTERS_DEFINE_H__
#define __ARMV7M_ECD_REGISTERS_DEFINE_H__

#include <stdint.h>

struct armv7m_core_regset {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r4;
  uint32_t r5;
  uint32_t r6;
  uint32_t r7;
  uint32_t r8;
  uint32_t r9;
  uint32_t r10;
  uint32_t r11;
  uint32_t r12;
  uint32_t sp;
  uint32_t lr;
  uint32_t pc;
  uint32_t xpsr;
};

struct arm_vfpv2_regset {
  uint64_t d0;
  uint64_t d1;
  uint64_t d2;
  uint64_t d3;
  uint64_t d4;
  uint64_t d5;
  uint64_t d6;
  uint64_t d7;
  uint64_t d8;
  uint64_t d9;
  uint64_t d10;
  uint64_t d11;
  uint64_t d12;
  uint64_t d13;
  uint64_t d14;
  uint64_t d15;
  uint32_t fpscr;
};

typedef struct armv7m_core_regset    core_regset_type;
typedef struct arm_vfpv2_regset      fp_regset_type;

#endif
