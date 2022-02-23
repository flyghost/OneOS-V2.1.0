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
 * @file        handlers.c
 *
 * @brief       This file provides macro definition for handlers.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include "riscv_encoding.h"
#include "n200_func.h"

__attribute__((weak)) uintptr_t handle_nmi()
{
    write(1, "nmi\n", 5);
    _exit(1);
    return 0;
}

__attribute__((weak)) uintptr_t handle_trap(uintptr_t mcause, uintptr_t sp)
{
    if (mcause == 0xFFF)
    {
        handle_nmi();
    }
    write(1, "trap\n", 5);
    // printf("In trap handler, the mcause is %d\n", mcause);
    // printf("In trap handler, the mepc is 0x%x\n", read_csr(mepc));
    // printf("In trap handler, the mtval is 0x%x\n", read_csr(mbadaddr));
    _exit(mcause);
    return 0;
}
