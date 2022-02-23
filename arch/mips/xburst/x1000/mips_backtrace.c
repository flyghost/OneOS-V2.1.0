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
 * @file        mips_backtrace.h
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include "mips.h"
#include <os_stddef.h>
#include <os_util.h>

#define ADDUI_SP_INST           0x27bd0000
#define SW_RA_INST              0xafbf0000
#define JR_RA_INST              0x03e00008
#define INST_OP_MASK            0xffff0000
#define INST_OFFSET_MASK        0x0000ffff

#define abs(s) ((s) < 0 ? -(s):(s))

int backtrace_ctx(mips_reg_ctx *ctx)
{
    unsigned long *addr;
    unsigned long *pc, *ra, *sp;
    size_t ra_offset;
    size_t stack_size;
    int depth;
    int size = 8;

    pc = (unsigned long *)(unsigned long)ctx->CP0EPC;
    ra = (unsigned long *)(unsigned long)ctx->regs[REG_RA];
    sp = (unsigned long *)(unsigned long)ctx->regs[REG_SP];

    os_kprintf("[0x%08x]\n", pc);

    if (size == 1) return 1;

    ra_offset = stack_size = 0;

    for (addr = ra; !ra_offset || !stack_size; --addr)
    {
        switch (*addr & INST_OP_MASK) {
            case ADDUI_SP_INST:
                stack_size = abs((short)(*addr&INST_OFFSET_MASK));
                break;

            case SW_RA_INST:
                ra_offset = (short)(*addr&INST_OFFSET_MASK);
                break;

            case 0x3c1c0000:
                goto out_of_loop;

            default:
                break;
        }
    }

out_of_loop:
    if (ra_offset)  ra = *(unsigned long **)((unsigned long)sp + ra_offset);
    if (stack_size) sp = (unsigned long *)((unsigned long)sp + stack_size);

    // repeat backwar scanning
    for (depth = 1; depth < size && ra && ra != (unsigned long *)0xffffffff; ++depth)
    {
        os_kprintf("RA[%2d] : [0x%08x]\n", depth ,ra);

        ra_offset = 0;
        stack_size = 0;

        for ( addr = ra; !ra_offset || !stack_size; -- addr )
        {
            switch( *addr & INST_OP_MASK)
            {
                case ADDUI_SP_INST:
                    stack_size = abs((short)(*addr&INST_OFFSET_MASK));
                    break;

                case SW_RA_INST:
                    ra_offset = abs((short)(*addr&INST_OFFSET_MASK));
                    break;

                case 0x3c1c0000:
                    return depth +1;

                default:
                    break;
            }
        }

        ra = *(unsigned long **)((unsigned long)sp + ra_offset);
        sp = (unsigned long *)((unsigned long)sp + stack_size);
    }

    return depth;
}

int backtrace(void)
{
    unsigned long *addr;
    unsigned long *ra;
    unsigned long *sp;
    int size = 8, depth;

    size_t ra_offset;
    size_t stack_size;

    // get current $a and $sp
    __asm__ __volatile__ (
            " move %0, $ra\n"
            " move %1, $sp\n"
            : "=r"(ra), "=r"(sp)
            );

    // scanning to find the size of hte current stack frame
    stack_size  = 0;

    for ( addr = (unsigned long *)backtrace; !stack_size; ++addr)
    {
        if ((*addr & INST_OP_MASK ) == ADDUI_SP_INST )
            stack_size = abs((short)(*addr&INST_OFFSET_MASK));
        else if ( *addr == JR_RA_INST )
            break;
    }

    sp = (unsigned long *) (( unsigned long )sp + stack_size);

    // repeat backwar scanning
    for ( depth = 0; depth < size && ((( unsigned long )ra > KSEG0BASE) && (( unsigned long )ra < KSEG1BASE)); ++ depth )
    {
        os_kprintf("RA[%2d] : [0x%08x]\n", depth, ra);
        {
            //extern void os_task_exit(void);
            //if ((uint32_t)ra == (uint32_t)(os_task_exit))
                return depth;
        }

        ra_offset = 0;
        stack_size = 0;

        for ( addr = ra; !ra_offset || !stack_size; -- addr )
        {
            switch( *addr & INST_OP_MASK)
            {
                case ADDUI_SP_INST:
                    stack_size = abs((short)(*addr&INST_OFFSET_MASK));
                    break;

                case SW_RA_INST:
                    ra_offset = (short)(*addr&INST_OFFSET_MASK);
                    break;

                case 0x3c1c0000:
                    return depth +1;

                default:
                    break;
            }
        }

        ra = *(unsigned long **)((unsigned long)sp + ra_offset);
        sp = (unsigned long*) ((unsigned long)sp+stack_size );
    }

    return depth;
}

void assert_hook(const char* ex, const char* func, os_int32_t line)
{
    backtrace();
#ifdef OS_USING_SHELL
    extern os_err_t sh_list_task(os_int32_t argc, char **argv);
    sh_list_task(0,OS_NULL);
#endif
    os_kprintf("(%s) assertion failed at function:%s, line number:%d \n", ex, func, line);
}

int backtrace_init(void)
{
#ifdef OS_DEBUG
    //os_assert_set_hook(assert_hook);
#endif
    return 0;
}
OS_DEVICE_INIT(backtrace_init,OS_INIT_SUBLEVEL_LOW);
