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
 * @file        mips_fpu.h
 *
 * @brief       mips_fpu define
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _MIPS_FPU_H_
#define _MIPS_FPU_H_

#ifndef __ASSEMBLY__

#include <os_stddef.h>
#include <mips_regs.h>
/**
 * init hardware FPU
 */
#ifdef OS_USING_FPU
rt_inline void rt_hw_fpu_init(void)
{
    rt_uint32_t c0_status = 0;
    rt_uint32_t c1_status = 0;

    /* Enable CU1 */
    c0_status = read_c0_status();
    c0_status |= (ST0_CU1 | ST0_FR);
    write_c0_status(c0_status);

    /* FCSR Configs */
    c1_status = read_c1_status();
    c1_status |= (FPU_CSR_FS | FPU_CSR_FO | FPU_CSR_FN);    /* Set FS, FO, FN */
    c1_status &= ~(FPU_CSR_ALL_E);                          /* Disable exception */
    c1_status = (c1_status & (~FPU_CSR_RM)) | FPU_CSR_RN;   /* Set RN */
    write_c1_status(c1_status);

    return ;
}
#else
OS_INLINE void os_hw_fpu_init(void){} /* Do nothing */
#endif

#endif

#endif
