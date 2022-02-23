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
 * @file        cpuport.c
 *
 * @brief       This file provides functions related to the ARM M4 architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-23   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_task.h>
#include <pthread.h>
#include <wait_for_event.h>
#include <stdio.h>
#include <arch_task.h>

volatile os_uint32_t g_interrupt_nesting;
extern os_task_t     *g_os_current_task;

typedef struct os_thread
{
    pthread_t pthread;
    void    (*task_entry)(void *arg) ;
    void     *arg;
    void    (*task_exit)(void);
    os_ubase_t xDying;
    struct event*     thread_event;
} os_thread_t;


os_ubase_t os_irq_lock(void)
{
    
    if ( g_interrupt_nesting == 0 )
    {
        disable_interrupts();
    }

    #if 0
    if(g_os_current_task != OS_NULL)
    { 
        os_thread_t *ptr = get_thread_from_task(g_os_current_task);
        printf("%s %s %d 0x%x 0x%x\r\n",__FUNCTION__,g_os_current_task->name,g_interrupt_nesting,
                            (ptr->pthread),pthread_self());
        fflush(stdout);
    }
    #endif
    g_interrupt_nesting++;
        
    return g_interrupt_nesting;
}

void os_irq_unlock(os_ubase_t irq_save)
{
    g_interrupt_nesting--;
    //g_interrupt_nesting = irq_save;
    #if 0
    if(g_os_current_task != OS_NULL)
    { 
        os_thread_t *ptr = get_thread_from_task(g_os_current_task);
        printf("%s %s %d 0x%x 0x%x\r\n",__FUNCTION__,g_os_current_task->name,g_interrupt_nesting,
                            (ptr->pthread),pthread_self());
        fflush(stdout);
    }
    #endif
    
    /* If we have reached 0 then re-enable the interrupts. */
    if( g_interrupt_nesting == 0 )
    {
        enable_interrupts();
    }
}

void os_irq_disable(void)
{

}

void os_irq_enable(void)
{

}

os_bool_t os_is_irq_active(void)
{
    return 0;
}

os_bool_t os_is_irq_disabled(void)
{
    return 0;
}

os_uint32_t os_irq_num(void)
{
    return 0;
}

os_bool_t os_is_fault_active(void)
{
    return 0;
}


