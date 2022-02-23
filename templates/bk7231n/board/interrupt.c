/*
 * File      : interrupt.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2013-2018, RT-Thread Development Team
 * 
 */

#include <os_hw.h>

#include "interrupt.h"
#include "intc.h"
#include "icu_pub.h"
#include "drv_model_pub.h"

extern os_uint8_t  g_os_interrupt_nest;
extern os_uint32_t os_interrupt_from_task;
extern os_uint32_t os_interrupt_to_task;
extern os_uint32_t os_task_switch_interrupt_flag;


extern void os_kprintf(const char *fmt, ...);

static void rt_hw_interrupt_handler(int vector, void *param)
{
    os_kprintf("Unhandled interrupt %d occured!!!\n", vector);
}

/**
 * This function will initialize hardware interrupt
 */
void rt_hw_interrupt_init(void)
{
    intc_init();

    /* init interrupt nest, and context in thread sp */
    g_os_interrupt_nest               = 0;
    os_interrupt_from_task        = 0;
    os_interrupt_to_task          = 0;
    os_task_switch_interrupt_flag = 0;
}

/**
 * This function will mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_mask(int vector)
{
    intc_disable(vector);
}

/**
 * This function will un-mask a interrupt.
 * @param vector the interrupt number
 */
void rt_hw_interrupt_umask(int vector)
{
    intc_enable(vector);
}

/**
 * This function will install a interrupt service routine to a interrupt.
 * @param vector the interrupt number
 * @param handler the interrupt service routine to be installed
 * @param param the interrupt service function parameter
 * @param name the interrupt name
 * @return old handler
 */
rt_isr_handler_t rt_hw_interrupt_install(int vector, rt_isr_handler_t handler,
        void *param, char *name)
{
    return OS_NULL;
}

void rt_irq_dispatch(void)
{
    intc_irq();
}

void rt_fiq_dispatch(void)
{
    intc_fiq();
}

