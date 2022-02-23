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
 * @file        exception.h
 *
 * @brief       exception head file
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "ptrace.h"

#ifndef __ASSEMBLY__

typedef void (* exception_func_t)(struct pt_regs *regs);

extern int os_hw_exception_init(void);
extern exception_func_t sys_exception_handlers[];
extern void os_do_mips_cpu_irq(os_uint32_t ip);
exception_func_t os_set_except_vector(int n, exception_func_t func);
extern void mips_mask_cpu_irq(os_uint32_t irq);
extern void mips_unmask_cpu_irq(os_uint32_t irq);
#endif

#endif /* end of __EXCEPTION_H__ */

