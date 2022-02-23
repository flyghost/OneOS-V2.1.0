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
 * @file        vector_table.c
 *
 * @brief       This file provides vector_table for cotex-m.
 *
 * @revision
 * Date         Author          Notes
 * 2021-09-23   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_common.h>

int OS_USED OS_SECTION("reserved_ram") rsv_ram_start = 0;

#define RESERVED_RAM_VECTOR_ADDR   (&rsv_ram_start + 1)
#define RESERVED_RAM_VECTOR_HOOK   (&rsv_ram_start + 2)

#if !defined(COTEX_M_VECTORS_BASE) && !defined(COTEX_M_STACK_END)
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern int __Vectors;
    extern int ___initial_sp;
    
    #define COTEX_M_VECTORS_BASE  __Vectors
    #define COTEX_M_STACK_END     ___initial_sp
#elif defined(__ICCARM__) || defined(__ICCRX__)
#error: not support iar
#elif defined(__GNUC__)
    extern int g_pfnVectors;
    extern int _estack;

    #define COTEX_M_VECTORS_BASE  g_pfnVectors
    #define COTEX_M_STACK_END     _estack
#endif
#endif

typedef void (*vector_entry)(void);

static const vector_entry vector_table[];

static void cotex_m_irq_hook(int irq)
{
#if defined(__CC_ARM) || defined(__CLANG_ARM)
    extern const int os_irq_hook$$Base;
    extern const int os_irq_hook$$Limit;
    
    const os_irq_hook *hook = (os_irq_hook *)&os_irq_hook$$Base;
    int nr = (os_irq_hook *)&os_irq_hook$$Limit - hook;
    
#elif defined(__ICCARM__) || defined(__ICCRX__) /* for IAR Compiler */
    const os_irq_hook *hook = (os_irq_hook *)__section_begin("os_irq_hook");
    int nr = (os_irq_hook *)__section_end("os_irq_hook") - hook;
#elif defined(__GNUC__)                         /* for GCC Compiler */
    extern const int __os_irq_hook_start;
    extern const int __os_irq_hook_end;
    const os_irq_hook *hook = (os_irq_hook *)&__os_irq_hook_start;
    int nr = (os_irq_hook *)&__os_irq_hook_end - hook;
#endif

    int i;

    for (i = 0; i < nr; i++)
    {
        hook[i](irq);
    }
}

static os_err_t cotex_m_set_vector(void)
{
#if defined(BSP_INCLUDE_VECTOR_TABLE) && !defined(ARCH_ARM_CORTEX_M0)
    SCB->VTOR = (os_uint32_t)vector_table;
#endif

    *(os_uint32_t *)RESERVED_RAM_VECTOR_ADDR = (os_uint32_t)&COTEX_M_VECTORS_BASE;

    *(os_uint32_t *)RESERVED_RAM_VECTOR_HOOK = (os_uint32_t)&cotex_m_irq_hook;
    
    interrupt_stack_addr = &COTEX_M_STACK_END;

    return OS_EOK;
}

OS_CORE_INIT(cotex_m_set_vector, OS_INIT_SUBLEVEL_HIGH);

static void cotex_m_vector_entry(void)
{
    vector_entry *vtable;
    os_irq_hook   hook;
    
    int irq = os_irq_num();
    
    if (irq == 0)
    {
        irq = 1;
        cotex_m_set_vector();
    }
    else
    {
        hook = (os_irq_hook)*(os_uint32_t *)RESERVED_RAM_VECTOR_HOOK;
        hook(irq);
    }

    vtable = (vector_entry *)*(os_uint32_t *)RESERVED_RAM_VECTOR_ADDR;
    vtable[irq]();
}

#ifdef BSP_INCLUDE_VECTOR_TABLE

#if defined(__CC_ARM)

__asm static void ___PendSV_Handler(void)
{
    import rsv_ram_start

    ldr r0, =rsv_ram_start
    ldr r0, [r0, #4]
    ldr r0, [r0, #0x38]
    bx  r0
}

#elif defined(__ICCARM__) || defined(__ICCRX__) /* for IAR Compiler */
#error: not support iar
#elif defined(__GNUC__) || defined(__CLANG_ARM)

void __PendSV_Handler(void)
{
    __asm__ __volatile__(
        ".global ___PendSV_Handler\n"
        ".type   ___PendSV_Handler, %function\n"
        "\n"
        "___PendSV_Handler:\n"
        "   ldr r0, =rsv_ram_start\n"
        "   ldr r0, [r0, #4]\n"
        "   ldr r0, [r0, #0x38]\n"
        "   bx  r0\n");
}

void ___PendSV_Handler(void);

#endif

#endif

static const vector_entry OS_USED OS_SECTION("vtor_table") vector_table[] =
{    
    (vector_entry)(&COTEX_M_STACK_END), /* system stack */
    cotex_m_vector_entry,               /* reset */
    
#ifdef BSP_INCLUDE_VECTOR_TABLE
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    0,
    0,
    0,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    0,
    (vector_entry)(&___PendSV_Handler),
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
#if !defined(ARCH_ARM_CORTEX_M0) && !defined(ARCH_ARM_CORTEX_M0_PLUS)
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
    cotex_m_vector_entry,
#endif
#endif
};

