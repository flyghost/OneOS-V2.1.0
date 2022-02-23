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
 * @file        drv_common.c
 *
 * @brief       This file provides common functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_types.h>
#include <os_memory.h>
#include <drv_common.h>
#include <string.h>
#include <interrupt.h>
#include <os_stddef.h>
#include <os_task.h>

extern unsigned char _iramcopy;
extern unsigned char _iramstart;
extern unsigned char _iramend;


#ifdef OS_USING_CPLUSPLUS
int cplusplus_system_init(void)
{
    typedef void (*pfunc) ();
    extern pfunc __ctors_start__[];
    extern pfunc __ctors_end__[];
    pfunc *p;
    for (p = __ctors_end__ - 2; p > __ctors_start__; )
    {
        (*p)();
        p--;
    }
    return 0;
}
#endif

static os_err_t os_hw_board_init(void)
{
    
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern int __Vectors;
    interrupt_stack_addr = *(os_ubase_t *)&__Vectors;
#elif __ICCARM__
#error:not support iccarm
#else
    extern int irq_stack_top;
    interrupt_stack_addr = (os_ubase_t)&irq_stack_top;
#endif

    memcpy((void*)&_iramstart, (void*)&_iramcopy, (os_uint32_t)&_iramend - (os_uint32_t)&_iramstart);
    memset((void*)&__bss_start, 0x0, (os_uint32_t)&__bss_end - (os_uint32_t)&__bss_start);

    os_hw_cache_init();
    os_hw_exception_init();
    /* init hardware interrupt */
    os_hw_interrupt_init();
    os_irq_enable();

#if defined(OS_USING_SYS_HEAP)

    os_sys_heap_init();
    os_sys_heap_add(OS_HW_HEAP_BEGIN, OS_HW_HEAP_END-OS_HW_HEAP_BEGIN, OS_MEM_ALG_FIRSTFIT);

#endif

    mips_clock_init();
    ostick_config(OS_TICK_PER_SECOND);

    return OS_EOK;
}
OS_CORE_INIT(os_hw_board_init, OS_INIT_SUBLEVEL_MIDDLE);

