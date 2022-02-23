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
 * @file        os_safety.h
 *
 * @brief       Header file for safety interface.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-08   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __OS_SAFETY_H__
#define __OS_SAFETY_H__

#include <arch_interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_SAFETY_MECHANISM
extern void os_safety_task_stack_overflow_set_hook(void (*hook)(void));
extern void os_safety_assert_set_hook(void (*hook)(void));
extern void os_safety_exception_set_hook(void (*hook)(void));

extern void os_safety_task_stack_overflow_process(void);
extern void os_safety_assert_process(void);
extern void os_safety_exception_process(void);
#else
#define os_safety_task_stack_overflow_process()         \
do                                                      \
{                                                       \
    os_irq_disable();                                   \
    while(1);                                           \
} while (0)

#define os_safety_assert_process()                      \
do                                                      \
{                                                       \
    os_irq_disable();                                   \
    while(1);                                           \
} while (0)

#define os_safety_exception_process()                   \
do                                                      \
{                                                       \
    os_irq_disable();                                   \
    while(1);                                           \
} while (0)
#endif /* OS_USING_SAFETY_MECHANISM */

#ifdef __cplusplus
}
#endif

#endif /* __OS_SAFETY_H__ */

