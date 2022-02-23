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
 * @file        rt_irq.c
 *
 * @brief       Implementation of RT-Thread adaper interrupt API.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-24   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include "rtdef.h"
#include "arch_interrupt.h"

volatile rt_uint8_t rt_interrupt_nest;

void rt_interrupt_enter(void)
{
    os_ubase_t  irq_save;
    irq_save = os_irq_lock();
    rt_interrupt_nest++;
    os_irq_unlock(irq_save);
}

void rt_interrupt_leave(void)
{
    os_ubase_t  irq_save;
    irq_save = os_irq_lock();
    rt_interrupt_nest--;
    os_irq_unlock(irq_save);
}

rt_uint8_t rt_interrupt_get_nest(void)
{
    return rt_interrupt_nest;
}

rt_base_t rt_hw_interrupt_disable(void)
{
    return (rt_ubase_t)os_irq_lock();
}

void rt_hw_interrupt_enable(rt_base_t level)
{
    os_irq_unlock((os_ubase_t)level);
}
