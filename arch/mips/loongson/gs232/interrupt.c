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
 * @brief       interrupt driver
 *
 * @revision
 * Date         Author          Notes
 * 2021-06-24   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <os_stddef.h>
#include <arch_hw.h>
#include "gs232.h"
#include <arch_misc.h>

#define MAX_INTR            (GS232_NR_IRQS)

static struct os_irq_desc irq_handle_table[MAX_INTR];
void os_hw_timer_handler();

static struct gs232_intc_regs volatile *gs232_hw0_icregs
= (struct gs232_intc_regs volatile *)(INTC_BASE);

/**
 * @addtogroup Loongson GS232
 */

static void os_hw_interrupt_handler(int vector, void *param)
{
    os_kprintf("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void os_hw_interrupt_init(void)
{
    os_int32_t idx;
    os_int32_t i;
    os_uint32_t c0_status = 0;

    for (i=0; i < GS232_INTC_CELLS; i++)
    {
        /* Disable */
        (gs232_hw0_icregs+i)->int_en = 0x0;
        /* Trigger active low */
        (gs232_hw0_icregs+i)->int_pol = -1;    /* Must be done here */
        /* Make all interrupts level triggered */
        (gs232_hw0_icregs+i)->int_edge = 0x00000000;
        /* Mask all interrupts */
        (gs232_hw0_icregs+i)->int_clr = 0xffffffff;
        mips_unmask_cpu_irq(i + 2);
    }

    memset(irq_handle_table, 0x00, sizeof(irq_handle_table));
    for (idx = 0; idx < MAX_INTR; idx ++)
    {
        irq_handle_table[idx].handler = os_hw_interrupt_handler;
    }
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void os_hw_interrupt_mask(int vector)
{
    /* mask interrupt */
    (gs232_hw0_icregs+(vector>>5))->int_en &= ~(1 << (vector&0x1f));
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void os_hw_interrupt_umask(int vector)
{
    (gs232_hw0_icregs+(vector>>5))->int_en |= (1 << (vector&0x1f));
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param new_handler the interrupt service routine to be installed
 * @param old_handler the old interrupt service routine
 */
os_isr_handler_t os_hw_interrupt_install(int vector, os_isr_handler_t handler,
        void *param, const char *name)
{
    os_isr_handler_t old_handler = OS_NULL;

    if (vector >= 0 && vector < MAX_INTR)
    {
        old_handler = irq_handle_table[vector].handler;

#ifdef OS_USING_INTERRUPT_INFO
        strncpy(irq_handle_table[vector].name, name, OS_NAME_MAX);
#endif /* OS_USING_INTERRUPT_INFO */
        irq_handle_table[vector].handler = handler;
        irq_handle_table[vector].param = param;
    }

    return old_handler;
}

/**
 * Call ISR
 * @IRQn ID of IRQ
 */
void gs232_do_IRQ(int IRQn)
{
    os_isr_handler_t irq_func;
    void *param;

    irq_func = irq_handle_table[IRQn].handler;
    param    = irq_handle_table[IRQn].param;

    irq_func(IRQn, param);

#ifdef OS_USING_INTERRUPT_INFO
    irq_handle_table[IRQn].counter++;
#endif

    return ;
}

void os_do_mips_cpu_irq(os_uint32_t ip)
{
    os_uint32_t intstatus, irq, n;

    if (ip == 7) {
        os_hw_timer_handler();
    } else {
        n = ip - 2;
        /* Receive interrupt signal, compute the irq */
        intstatus = (gs232_hw0_icregs+n)->int_isr & (gs232_hw0_icregs+n)->int_en;
        if (0 == intstatus)
            return ;

        irq = os_ffs(intstatus) - 1;
        gs232_do_IRQ((n<<5) + irq);

        /* ack interrupt */
        (gs232_hw0_icregs+n)->int_clr |= (1 << irq);
    }
}

