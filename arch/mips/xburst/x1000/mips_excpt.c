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
 * @file        mips_excpt.c
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <arch_interrupt.h>
#include <stdio.h>
#include <os_errno.h>
#include <os_util.h>
#include <os_task.h>
#include "mips.h"
#include "mips_excpt.h"

extern int backtrace(void);
int backtrace_ctx(mips_reg_ctx *ctx);

#ifdef OS_USING_SHELL
extern os_err_t sh_show_task_info(os_int32_t argc, char **argv);
#endif

const static char *regstr[] = {
    "$0  zero", "$1  at", "$2  v0", "$3  v1", "$4  a0", "$5  a1", "$6  a2", "$7  a3",
    "$8  t0", "$9  t1", "$10 t2", "$11 t3", "$12 t4", "$13 t5", "$14 t6", "$15 t7",
    "$16 s0", "$17 s1", "$18 s2", "$19 s3", "$20 s4", "$21 s5", "$22 s6", "$23 s7",
    "$24 t8", "$25 t9", "$26 k0", "$27 k1", "$28 gp", "$29 sp", "$30 fp", "$31 ra"
};

static const char *cause_strings[32] =
{
    /*  0 */ "Int",
    /*  1 */ "TLB Mods",
    /*  2 */ "TLB Load",
    /*  3 */ "TLB Store",
    /*  4 */ "Address Load",
    /*  5 */ "Address Store",
    /*  6 */ "Instruction Bus Error",
    /*  7 */ "Data Bus Error",
    /*  8 */ "Syscall",
    /*  9 */ "Breakpoint",
    /* 10 */ "Reserved Instruction",
    /* 11 */ "Coprocessor Unuseable",
    /* 12 */ "Overflow",
    /* 13 */ "Trap",
    /* 14 */ "Instruction Virtual Coherency Error",
    /* 15 */ "FP Exception",
    /* 16 */ "Reserved 16",
    /* 17 */ "Reserved 17",
    /* 18 */ "Reserved 18",
    /* 19 */ "Reserved 19",
    /* 20 */ "Reserved 20",
    /* 21 */ "Reserved 21",
    /* 22 */ "Reserved 22",
    /* 23 */ "Watch",
    /* 24 */ "Reserved 24",
    /* 25 */ "Reserved 25",
    /* 26 */ "Reserved 26",
    /* 27 */ "Reserved 27",
    /* 28 */ "Reserved 28",
    /* 29 */ "Reserved 29",
    /* 30 */ "Reserved 30",
    /* 31 */ "Data Virtual Coherency Error"
};


/**
 * exception handle table
 */
exception_func_t sys_exception_handlers[32];

static void mod_handler(mips_reg_ctx *regs)
{
    os_kprintf("tlb modification exception\n");
    os_kprintf("exception happens, epc: 0x%08x\n", read_c0_epc());
    os_kprintf("                 cause: 0x%08x\n", read_c0_cause());

#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif

    printf("-----------------------------------------------------\n");
    printf("BACKTRACE:\n");
    backtrace();
    printf("-----------------------------------------------------\n");

    //os_hw_cpu_shutdown();
}

static void tlbl_handler(mips_reg_ctx *regs)
{
    os_kprintf("tlb exception: load\n");
    os_kprintf("exception happens, epc: 0x%08x\n", read_c0_epc());
    os_kprintf("                 cause: 0x%08x\n", read_c0_cause());

#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif
    printf("-----------------------------------------------------\n");
    printf("BACKTRACE:\n");
    backtrace();
    printf("-----------------------------------------------------\n");

    //os_hw_cpu_shutdown();
}

static void tlbs_handler(mips_reg_ctx *regs)
{
    os_kprintf("tlb exception: store\n");
    os_kprintf("exception happens, epc: 0x%08x\n", read_c0_epc());
    os_kprintf("                 cause: 0x%08x\n", read_c0_cause());

#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif

    printf("-----------------------------------------------------\n");
    printf("BACKTRACE:\n");
    backtrace();
    printf("-----------------------------------------------------\n");

    //os_hw_cpu_shutdown();
}

static void adel_handler(mips_reg_ctx *regs)
{
    os_kprintf("address error exception: load\n");
    os_kprintf("exception happens, epc: 0x%08x\n", read_c0_epc());
    os_kprintf("                 cause: 0x%08x\n", read_c0_cause());

#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif

    os_kprintf("current thread: %.*s\n", OS_NAME_MAX, os_task_self()->name);

    printf("-----------------------------------------------------\n");
    printf("BACKTRACE:\n");
    backtrace();
    printf("-----------------------------------------------------\n");

    //os_hw_cpu_shutdown();
}

static void ades_handler(mips_reg_ctx *regs)
{
    os_kprintf("address error exception: store\n");
    os_kprintf("exception happens, epc: 0x%08x\n", read_c0_epc());
    os_kprintf("                 cause: 0x%08x\n", read_c0_cause());

#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif

    printf("-----------------------------------------------------\n");
    printf("BACKTRACE:\n");
    backtrace();
    printf("-----------------------------------------------------\n");

    //os_hw_cpu_shutdown();
}

static void fpe_handler(mips_reg_ctx *regs)
{
    os_kprintf("floating point exception\n");
    os_kprintf("exception happens, epc: 0x%08x\n", read_c0_epc());
    os_kprintf("                 cause: 0x%08x\n", read_c0_cause());

#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif

    printf("-----------------------------------------------------\n");
    printf("BACKTRACE:\n");
    backtrace();
    printf("-----------------------------------------------------\n");

    //os_hw_cpu_shutdown();
}


static void unhandled_exception_handle(mips_reg_ctx *regs)
{
    int i;
    unsigned int cause = read_c0_cause();
    unsigned int exc = (cause >> 2) & 0x1f;

    os_kprintf("exception happens, epc: 0x%08x\n", regs->CP0EPC);
    os_kprintf("                 cause: 0x%08x\n", regs->CP0Cause);

    for (i = 0; i < 32; i++)
    {
        if (i % 4 == 0)
            printf("\n");
        printf("%8s %08x ", regstr[i], regs->regs[i]);
    }
    printf("\n");

#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif
    //os_hw_cpu_shutdown();
}

static void install_default_exception_handler(void)
{
    int i;

    for (i=0; i<sizeof(sys_exception_handlers)/sizeof(sys_exception_handlers[0]); i++)
        sys_exception_handlers[i] = (exception_func_t)unhandled_exception_handle;

    sys_exception_handlers[EX_MOD]  = mod_handler;
    sys_exception_handlers[EX_TLBL] = tlbl_handler;
    sys_exception_handlers[EX_TLBS] = tlbs_handler;
    sys_exception_handlers[EX_ADEL] = adel_handler;
    sys_exception_handlers[EX_ADES] = ades_handler;
    sys_exception_handlers[EX_FPE]  = fpe_handler;
}

int os_hw_exception_init(void)
{
    /* install the default exception handler */
    install_default_exception_handler();

    return OS_EOK;
}

/**
 * setup the exception handle
 */
exception_func_t os_set_except_vector(int n, exception_func_t func)
{
    exception_func_t old_handler = sys_exception_handlers[n];

    if ((n == 0) || (n > 32) || (!func))
    {
        return 0;
    }

    sys_exception_handlers[n] = func;

    return old_handler;
}

void mips_exception_handler(mips_reg_ctx *ctx)
{
    static int read_epc_count = 0;
    static int epc_save = 0;
    int 	i;
    unsigned int epc;

    if (read_epc_count > 0)
    {
        printf("ERROR: read epc fail when except handle\n");
        epc = epc_save;
        read_epc_count = 0;
    }
    else
    {
        read_epc_count++;
        epc_save 	= 0;
        epc 		= read_c0_epc();
        epc_save 	= epc;

        if (epc != 0)
        {
            printf("-----------------------------------------------------\n");
            for (i = 0; i < 4; i++)
            {
                printf("%08x:\t%08x\n",
                        (epc - 4 * 4 + i * 4),
                        *(unsigned int *) ((epc - 4 * 4 + i * 4) | 0xa0000000));
            }
            for (i = 0; i < 4; i++)
            {
                printf("%08x:\t%08x\n",
                        (epc + i * 4),
                        *(unsigned int *) ((epc + i * 4) | 0xa0000000));
            }
            printf("-----------------------------------------------------\n");
        }

        read_epc_count--;
    }

    printf("-----------------------------------------------------\n");
    unsigned int cause = read_c0_cause();
    unsigned int exc = (cause >> 2) & 0x1f;
    printf("CAUSE=%08x --> %s\n", cause, cause_strings[exc]);
    printf("EPC=%08x\n", epc);

    for (i = 0; i < 32; i++)
    {
        if ((i != 0) && (i % 4 == 0))
            printf("\n");
        printf("%8s %08x ", regstr[i], ctx->regs[i]);
    }
    printf("\n-----------------------------------------------------\n");
    printf("%s: \t %8x\n","CP0Status  ",	ctx->CP0Status);
    printf("%s: \t %8x\n","CP0DataHI  ",	ctx->CP0DataHI);
    printf("%s: \t %8x\n","CP0DataLO  ",	ctx->CP0DataLO);
    printf("%s: \t %8x\n","CP0BadVAddr",	ctx->CP0BadVAddr);
    printf("%s: \t %8x\n","CP0Cause   ",	ctx->CP0Cause);
    printf("%s: \t %8x\n","CP0EPC     ",	ctx->CP0EPC);
    printf("-----------------------------------------------------\n");

    sys_exception_handlers[exc](ctx);
    //os_hw_cpu_shutdown();
}


void mips_cache_error_handler (unsigned int  Addr)
{
    os_kprintf("cache exception happens, epc: 0x%08x\n", read_c0_epc());
#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif
    //os_hw_cpu_shutdown();
}

void mips_tlb_refill_handler(void)
{
    os_kprintf("tlb-miss happens, epc: 0x%08x\n", read_c0_epc());
    os_kprintf("                cause: 0x%08x\n", read_c0_cause());
#ifdef OS_USING_SHELL
    sh_show_task_info(0,OS_NULL);
#endif
    os_kprintf("current thread: %s\n", os_task_self()->name);
    //os_hw_cpu_shutdown();
}
