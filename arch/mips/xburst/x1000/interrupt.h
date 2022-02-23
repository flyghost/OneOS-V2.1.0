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
 * @file        interrupt.h
 *
 * @brief       This file is part of OneOS.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __INTERRUPT_H
#define __INTERRUPT_H

#define INTERRUPTS_MAX  64

/*
 * Interrupt handler definition
 */
typedef void (*os_isr_handler_t)(int vector, void *param);

struct os_irq_desc
{
    os_isr_handler_t handler;
    void            *param;

#ifdef OS_USING_INTERRUPT_INFO
    char             name[OS_NAME_MAX];
    os_uint32_t      counter;
#endif
};

extern void os_hw_interrupt_init(void);
extern void os_hw_interrupt_mask(int vector);
extern void os_hw_interrupt_umask(int vector);
extern os_isr_handler_t os_hw_interrupt_install(int vector, os_isr_handler_t handler,void *param, const char *name);

#endif

