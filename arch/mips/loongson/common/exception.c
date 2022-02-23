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
 * @file        exception.c
 *
 * @brief       exception handle file
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_types.h>
#include <os_errno.h>
#include <exception.h>

/**
 * @addtogroup MIPS
 */

extern os_ubase_t __ebase_entry;
os_ubase_t os_interrupt_from_thread;
os_ubase_t os_interrupt_to_thread;
os_ubase_t os_thread_switch_interrupt_flag;

const char *exception_name[] = {
    "Interrupt",
    "(X)TLB Modify Exception",
    "(X)TLB Read/Fetch Exception",
    "(X)TLB Write Exception",
    "Address Read/Fetch Exception",
    "Address Write Exception",
    "",
    "",
    "Syscall",
    "Breakpoint",
    "Reversed Instruction Exception",
    "Coprocessor Unit Invalid",
    "Overflow",
    "Trap",
    "FPU Exception in Vector Instruction",
    "FPU Exception",
    "Loongson Custom Exception",
    "",
    "",
    "(X)TLB Read Denied Exception",
    "(X)TLB Execute Denied Exception",
    "Vector Module Disabled Exception",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "Cache Error Exception",
    ""
};

os_base_t os_hw_interrupt_disable(void)
{
    os_base_t status = read_c0_status();
    status &= ST0_IE;
    clear_c0_status(ST0_IE);
    return status;
}

void os_hw_interrupt_enable(os_base_t level)
{
    os_base_t status = read_c0_status();
    status &= ~ST0_IE;
    status |= level;
    write_c0_status(status);
}

/**
 * exception handle table
 */
#define OS_EXCEPTION_MAX 32
exception_func_t sys_exception_handlers[OS_EXCEPTION_MAX];

/**
 * setup the exception handle
 */
exception_func_t os_set_except_vector(int n, exception_func_t func)
{
    exception_func_t old_handler = sys_exception_handlers[n];

    if ((n == 0) || (n > OS_EXCEPTION_MAX) || (!func))
    {
        return 0;
    }

    sys_exception_handlers[n] = func;

    return old_handler;
}

void mips_dump_regs(struct pt_regs *regs)
{
    int i, j;
    for (i = 0; i < 32 / 4; i++)
    {
        for (j = 0; j < 4; j++)
        {
            int reg = 4 * i + j;
            os_kprintf("%d: 0x%08x, ", reg, regs->regs[reg]);
        }
        os_kprintf("\n");
    }
}

void tlb_refill_handler(void)
{
    os_kprintf("TLB-Miss Happens, EPC: 0x%08x\n", read_c0_epc());
    os_hw_cpu_shutdown();
}

void cache_error_handler(void)
{
    os_kprintf("Cache Exception Happens, EPC: 0x%08x\n", read_c0_epc());
    os_hw_cpu_shutdown();
}

static void unhandled_exception_handle(struct pt_regs *regs)
{
    os_kprintf("Unknown Exception, EPC: 0x%p, CAUSE: 0x%08x\n", read_c0_epc(), read_c0_cause());
    os_kprintf("Exception Name:%s\n",exception_name[(read_c0_cause() >> 2) & 0x1f]);
#ifdef SOC_LS2K1000
    os_kprintf("ExeCode = 0x%08x,BadAddr = 0x%p\n",(read_c0_cause() >> 2) & 0x1f,mmu_tlb_get_bad_vaddr());
#else
    os_kprintf("ExeCode = 0x%08x\n",(read_c0_cause() >> 2) & 0x1f);
#endif
    os_kprintf("ST0: 0x%08x ",regs->cp0_status);
    os_kprintf("ErrorPC: 0x%p\n",read_c0_errorepc());
    mips_dump_regs(regs);
    os_hw_cpu_shutdown();
}

static void install_default_exception_handler(void)
{
    os_int32_t i;

    for (i = 0; i < OS_EXCEPTION_MAX; i++)
        sys_exception_handlers[i] =
            (exception_func_t)unhandled_exception_handle;
}

int os_hw_exception_init(void)
{
    os_ubase_t ebase = (os_ubase_t)&__ebase_entry;
#ifdef ARCH_MIPS64
    ebase |= 0xffffffff00000000;
#endif
    write_c0_ebase(ebase);
    clear_c0_status(ST0_BEV | ST0_ERL | ST0_EXL);
    clear_c0_status(ST0_IM | ST0_IE);
    set_c0_status(ST0_CU0);
    /* install the default exception handler */
    install_default_exception_handler();

    return OS_EOK;
}

void os_general_exc_dispatch(struct pt_regs *regs)
{
    os_ubase_t cause, exccode;
    cause = read_c0_cause();
    exccode = (cause & CAUSEF_EXCCODE) >> CAUSEB_EXCCODE;

    if (exccode == 0)
    {
        os_ubase_t status, pending;

        status = read_c0_status();
        pending = (cause & CAUSEF_IP) & (status & ST0_IM);
        if (pending & CAUSEF_IP0)
            os_do_mips_cpu_irq(0);
        if (pending & CAUSEF_IP1)
            os_do_mips_cpu_irq(1);
        if (pending & CAUSEF_IP2)
            os_do_mips_cpu_irq(2);
        if (pending & CAUSEF_IP3)
            os_do_mips_cpu_irq(3);
        if (pending & CAUSEF_IP4)
            os_do_mips_cpu_irq(4);
        if (pending & CAUSEF_IP5)
            os_do_mips_cpu_irq(5);
        if (pending & CAUSEF_IP6)
            os_do_mips_cpu_irq(6);
        if (pending & CAUSEF_IP7)
            os_do_mips_cpu_irq(7);
    }
    else
    {
        if (sys_exception_handlers[exccode])
            sys_exception_handlers[exccode](regs);
    }
}

/* Mask means disable the interrupt */
void mips_mask_cpu_irq(os_uint32_t irq)
{
    clear_c0_status(1 << (STATUSB_IP0 + irq));
}

/* Unmask means enable the interrupt */
void mips_unmask_cpu_irq(os_uint32_t irq)
{
    set_c0_status(1 << (STATUSB_IP0 + irq));
}

