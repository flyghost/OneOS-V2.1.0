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
 * @file        interrupt.c
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <arch_interrupt.h>
#include <x1000.h>
#include <os_util.h>
#include <string.h>
#include <interrupt.h>
#include "../common/mips.h"

extern os_uint32_t g_os_interrupt_nest;
os_uint32_t g_os_task_switch_interrupt_flag;

static struct os_irq_desc isr_table[INTERRUPTS_MAX];

static void os_hw_interrupt_handler(int vector, void *param)
{
    os_kprintf("Unhandled interrupt %d occured!!!\n", vector);
}

void os_hw_interrupt_init(void)
{
    os_int32_t idx;

    clear_c0_status(0xff04); /* clear ERL */
    set_c0_status(0x0400);   /* set IP2 */

    memset(isr_table, 0x00, sizeof(isr_table));
    for (idx = 0; idx < INTERRUPTS_MAX; idx ++)
    {
        isr_table[idx].handler = os_hw_interrupt_handler;
    }

    /* init interrupt nest and task switch interrupt flag */
    g_os_interrupt_nest               = 0;
    g_os_task_switch_interrupt_flag = 0;

    /* enable cpu interrupt mask */
    set_c0_status(IE_IRQ0 | IE_IRQ1);
}

void os_hw_interrupt_mask(int vector)
{
    /* mask interrupt */
    __intc_mask_irq(vector);
}

void os_hw_interrupt_umask(int vector)
{
    __intc_unmask_irq(vector);
}

os_isr_handler_t os_hw_interrupt_install(int vector, os_isr_handler_t handler,
                                         void *param, const char *name)
{
    os_isr_handler_t old_handler = OS_NULL;

    if(vector < INTERRUPTS_MAX)
    {
        old_handler = isr_table[vector].handler;

#ifdef OS_USING_INTERRUPT_INFO
        os_strncpy(isr_table[vector].name, name, OS_NAME_MAX);
#endif /* OS_USING_INTERRUPT_INFO */
        isr_table[vector].handler = handler;
        isr_table[vector].param = param;
    }

    return old_handler;
}

static __inline int fls(int x)
{
    __asm__("clz %0, %1" : "=r" (x) : "r" (x));

    return 32 - x;
}

void os_interrupt_dispatch(void *ptreg)
{
    void *param;
    os_isr_handler_t irq_func;

    int irq = 0, group;
    os_uint32_t intc_ipr0 = 0, intc_ipr1 = 0, vpu_pending = 0;

    os_uint32_t c0_status, c0_cause;
    os_uint32_t pending_im;

    /* check os timer */
    c0_status = read_c0_status();
    c0_cause = read_c0_cause();

    pending_im = (c0_cause & ST0_IM) & (c0_status & ST0_IM);

    if (pending_im & CAUSEF_IP3)
    {
        extern void os_hw_ost_handler(void);
        os_hw_ost_handler();
        return;
    }
    if (pending_im & CAUSEF_IP2)
    {
        intc_ipr0 = REG_INTC_IPR(0);
        intc_ipr1 = REG_INTC_IPR(1);

        if (intc_ipr0)
        {
            irq = fls(intc_ipr0) - 1;
            intc_ipr0 &= ~(1<<irq);
        }
        else if(intc_ipr1)
        {
            irq = fls(intc_ipr1) - 1;
            intc_ipr1 &= ~(1<<irq);
            irq += 32;
        }
        else
        {
            //VPU
        }

        if (irq >= INTERRUPTS_MAX)
            os_kprintf("max interrupt, irq=%d\n", irq);

        /* do interrupt */
        irq_func = isr_table[irq].handler;
        param = isr_table[irq].param;
        (*irq_func)(irq, param);

#ifdef OS_USING_INTERRUPT_INFO
        isr_table[irq].counter++;
#endif /* OS_USING_INTERRUPT_INFO */

        /* ack interrupt */
        __intc_ack_irq(irq);
    }

    if (pending_im & CAUSEF_IP0)
        os_kprintf("CAUSEF_IP0\n");
    if (pending_im & CAUSEF_IP1)
        os_kprintf("CAUSEF_IP1\n");
    if (pending_im & CAUSEF_IP4)
        os_kprintf("CAUSEF_IP4\n");
    if (pending_im & CAUSEF_IP5)
        os_kprintf("CAUSEF_IP5\n");
    if (pending_im & CAUSEF_IP6)
        os_kprintf("CAUSEF_IP6\n");
    if (pending_im & CAUSEF_IP7)
        os_kprintf("CAUSEF_IP7\n");
}

#ifdef OS_USING_INTERRUPT_INFO
int list_irqs(void)
{
    int index;

    os_kprintf("interrupt list:\n");
    os_kprintf("----------------\n");
    os_kprintf("name     counter\n");
    for (index = 0; index < INTERRUPTS_MAX; index ++)
    {
        if (isr_table[index].handler != os_hw_interrupt_handler)
        {
            os_kprintf("%-*.*s %d\n", OS_NAME_MAX, OS_NAME_MAX, isr_table[index].name, isr_table[index].counter);
        }
    }

    return 0;
}
#ifdef OS_USING_SHELL
#include <shell.h>
SH_CMD_EXPORT(show_irqs, list_irqs, "Show irqs information");
#endif
#endif

unsigned int spin_lock_irqsave(void)
{
    register unsigned int t;
    t = read_c0_status();
    write_c0_status((t & (~1)));
    return (t);
}

void spin_unlock_irqrestore(unsigned int val)
{
    write_c0_status(val);
}
