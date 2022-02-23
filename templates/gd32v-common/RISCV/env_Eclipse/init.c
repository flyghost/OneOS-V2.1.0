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
 * @file        init.c
 *
 * @brief       This file implements riscv_clock_init.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <gd32vf103.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "riscv_encoding.h"
#include "n200_func.h"

extern uint32_t disable_mcycle_minstret();
void            _init()
{
    //	SystemInit();

    /* ECLIC init */
    //	eclic_init(ECLIC_NUM_INTERRUPTS);
    //	eclic_mode_enable();

    // printf("After ECLIC mode enabled, the mtvec value is %x \n\n\r", read_csr(mtvec));

    /* It must be NOTED:
     * In the RISC-V arch, if user mode and PMP supported, then by default if PMP is not configured
     * with valid entries, then user mode cannot access any memory, and cannot execute any instructions.
     * So if switch to user-mode and still want to continue, then you must configure PMP first */

    // pmp_open_all_space();
    // switch_m2u_mode();

    /* Before enter into main, add the cycle/instret disable by default to save power,
     * only use them when needed to measure the cycle/instret */
    //	disable_mcycle_minstret();
}

void _fini()
{
}

void riscv_clock_init(void)
{
    SystemInit();

    /* ECLIC init */
    eclic_init(ECLIC_NUM_INTERRUPTS);
    eclic_mode_enable();
    disable_mcycle_minstret();
}
