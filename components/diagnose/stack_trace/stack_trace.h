/**
***********************************************************************************************************************
* Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
* Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
* the License. You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
* an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
* specific language governing permissions and limitations under the License.
*
* @file        stack_trace.h
*
* @brief       Header file for resource moniter interface.
*
* @revision
* Date         Author          Notes
* 2021-01-13   OneOS Team      the first version
***********************************************************************************************************************
*/

#ifndef _STACK_TRACE_H_
#define _STACK_TRACE_H_

#include <oneos_config.h>
#include <os_types.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef STACK_TRACE_EN

struct call_back_trace
{
    os_uint32_t depth;
    os_size_t back_trace[CALL_BACK_TRACE_MAX_DEPTH];
};

typedef struct call_back_trace call_back_trace_t;

/* The following variables need to be initialized in each arch. */
extern os_size_t g_code_start_addr;            /* Start address of code segment. */
extern os_size_t g_code_end_addr;              /* End address of code segment. */
extern os_size_t g_main_stack_start_addr;      /* Start address of main stack. */
extern os_size_t g_main_stack_end_addr;        /* End address of main stack. */

#ifdef EXC_DUMP_STACK

extern void dump_stack(os_uint32_t stack_start_addr, os_uint32_t stack_size, os_size_t *stack_pointer);

#endif  /* EXC_DUMP_STACK */

extern os_bool_t task_is_protected(const char *name);

extern os_err_t task_stack_show(char *name, os_uint32_t context);

#endif  /* STACK_TRACE_EN*/


#endif /* _STACK_TRACE_H_ */
