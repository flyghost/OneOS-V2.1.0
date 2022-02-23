/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        stack_trace.c
 *
 * @brief       This file include stack backtrace.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-13   armink          the first version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>

#ifdef STACK_TRACE_EN

#include <stack_trace.h>
#include <os_util.h>
#include <string.h>
#include <os_errno.h>
#include <arch_interrupt.h>
#include <os_task.h>
#include <os_assert.h>


/* The following variables need to be initialized in each arch. */
os_size_t g_code_start_addr = 0;            /* Start address of code segment. */
os_size_t g_code_end_addr = 0;              /* End address of code segment. */
os_size_t g_main_stack_start_addr = 0;      /* Start address of main stack. */
os_size_t g_main_stack_end_addr = 0;        /* End address of main stack. */

#ifdef EXC_DUMP_STACK

/**
 ***********************************************************************************************************************
 * @brief           The function will dump all information in task or interrupt stack.
 *
 * @param[in]       stack_start_addr    Start Address of stack
 * @param[in]       stack_size          Stack size in bytes
 * @param[in]       stack_point         The stack pointer.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
void dump_stack(os_uint32_t stack_start_addr, os_uint32_t stack_size, os_size_t *stack_pointer) 
{    
    os_size_t *sp;
    
    os_kprintf("=======================   Stack info  =====================\r\n");
    for (sp = stack_pointer; (os_uint32_t) sp < stack_start_addr + stack_size; sp++) 
    {
        os_kprintf("        addr: 0x%08x      data: %08x\r\n", sp, *sp);
    }
    
    os_kprintf("-------------------------------------------------------------\r\n");    
}

#endif  /* EXC_DUMP_STACK */

/**
 ***********************************************************************************************************************
 * @brief           This function implement a assert hook function.
 *
 * @param[in]       ex              The assertion condition string.
 * @param[in]       func            The function name when assertion.
 * @param[in]       line            The file line number when assertion.
 *
 * @return          None
 ***********************************************************************************************************************
 */
OS_WEAK void assert_hook(const char* ex, const char* func, os_int32_t line) 
{
    os_irq_lock();

    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!!!    Assert   !!!!!!!!!!!!!!!!!!!!!!!\r\n");
    os_kprintf("(%s) assertion failed at function: %s, line number: %d \r\n", ex, func, line);

#ifdef OS_USING_SHELL

    os_kprintf("=======================   Heap Info   =======================\r\n");
    extern os_err_t sh_memshow(os_int32_t argc, char **argv);
    sh_memshow(0, OS_NULL);

    if(0 == os_is_irq_active())
    {
        os_kprintf("========================   Task Info  =======================\r\n");
        extern os_err_t sh_show_task_info(os_int32_t argc, char **argv);
        sh_show_task_info(0, OS_NULL);
    }

#endif /* OS_USING_SHELL */

    /* todo: need add interrupt stack back tracce. */
    if(os_is_irq_active() > 0)
    {
        os_kprintf("Assert in interrupt context of irq %d!\r\n", os_irq_num());
    }
    else
    {
        task_stack_show((char*)os_task_name(os_task_self()), 0);
    }

    os_kprintf("\r\n!!!!!!!!!!!!!!!!!!!!!!   Assert End   !!!!!!!!!!!!!!!!!!!!!!!\r\n");

    while (1);

}

/**
 ***********************************************************************************************************************
 * @brief           Determine whether the task is system protection.
 *
 * @attention       Because the idle-task and recycle-task are built-in tasks, they cannot be operated.
 *
 * @param[in]       name            Task name.
 *
 * @return          Whether the specified task can be operated.
 * @retval          OS_TRUE         The specified task cannot be operated.
 * @retval          OS_FALSE        The specified task can be operated.
 ***********************************************************************************************************************
 */
os_bool_t task_is_protected(const char *name)
{
    OS_ASSERT(name != OS_NULL);
    
    if ((strncmp(name, OS_RECYCLE_TASK_NAME, OS_NAME_MAX) == 0)
        || (strncmp(name, OS_IDLE_TASK_NAME, OS_NAME_MAX) == 0))
    {
        return OS_TRUE;
    }
    else
    {
       return OS_FALSE;
    }
}

#ifdef OS_USING_SHELL

/**
***********************************************************************************************************************
* @brief           A shell command to display task stack backtrace information.
*
* @param[in]       argc             argment count
* @param[in]       argv             argment list
*
* @return          On success, return OS_EOK; on error, OS_ERROR is returned.
* @retval          OS_EOK           Success.
* @retval          OS_ERROR         There is no task with this name in the system 
*                                   or wrong number of shell command parameters.
***********************************************************************************************************************
*/
static os_err_t sh_task_stack_show(os_int32_t argc, char **argv)
{ 
    char name[OS_NAME_MAX + 1];
    os_uint32_t len;

    if(argc < 2)
    {
        os_kprintf("Please enter the task name!\r\n");
        return OS_ERROR;
    }
   
    len = strlen(argv[1]);
    memset(name, 0, OS_NAME_MAX + 1);
    strncpy(name, argv[1], len > OS_NAME_MAX ? len : OS_NAME_MAX);

    return task_stack_show(name, 0);

}

#include <shell.h>
SH_CMD_EXPORT(show_task_stack, sh_task_stack_show, "show stack call back trace of a task, para is the name of task");

#endif  /* OS_USING_SHELL */

#endif  /* STACK_TRACE_EN */

