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
 * @file        os_kernel_internal.h
 *
 * @brief       Provides some macro definitions and function declarations, only use for kernel.
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-03   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __OS_KERNEL_INTERNAL_H__
#define __OS_KERNEL_INTERNAL_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_task.h>
#include <stdlib.h>
#include <os_memory.h>
#include <arch_interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_KERNEL_VERSION                           "Kernel-V2.1.0"

#ifdef OS_USING_SYS_HEAP
#define OS_KERNEL_MALLOC(size)                      os_malloc(size)
#define OS_KERNEL_MALLOC_ALIGN(alignment, size)     os_aligned_malloc(alignment, size)

#define OS_KERNEL_FREE(ptr)                         os_free(ptr)
#define OS_KERNEL_REALLOC(ptr, size)                os_realloc(ptr, size)
#else
#define OS_KERNEL_MALLOC(size)
#define OS_KERNEL_MALLOC_ALIGN(alignment, size)

#define OS_KERNEL_FREE(ptr)
#define OS_KERNEL_REALLOC(ptr, size)
#endif /* OS_USING_SYS_HEAP */

#define OS_KOBJ_INITED                  0x55
#define OS_KOBJ_DEINITED                0xAA

#define OS_KOBJ_ALLOC_TYPE_STATIC       0
#define OS_KOBJ_ALLOC_TYPE_DYNAMIC      1

#ifdef OS_USING_KERNEL_DEBUG

/* Kernel log level */
#define KERN_ERROR                      0       /* Error conditions */
#define KERN_WARNING                    1       /* Warning conditions */
#define KERN_INFO                       2       /* Informational */
#define KERN_DEBUG                      3       /* Debug-level messages */

#ifdef KLOG_WITH_FUNC_LINE

/**
 ***********************************************************************************************************************
 * @def         OS_KERN_LOG
 *
 * @brief       Print kernel log with function name and file line number.
 *
 * @param       level           The log level.
 * @param       tag             The log tag.
 * @param       fmt             The format.
 ***********************************************************************************************************************
 */
#define OS_KERN_LOG(level, tag, fmt, ...)                                                       \
do                                                                                              \
{                                                                                               \
    if ((level <= *g_klog_global_lvl) || (g_klog_tag_lvl_list->next != g_klog_tag_lvl_list))    \
    {                                                                                           \
        os_kernel_print(level, tag, OS_TRUE, fmt " [%s][%d]",                                   \
                        ##__VA_ARGS__, __FUNCTION__, __LINE__);                                 \
    }                                                                                           \
} while (0)

#else

/**
 ***********************************************************************************************************************
 * @def         OS_KERN_LOG
 *
 * @brief       Print kernel log without function name and file line number.
 *
 * @param       level           The log level.
 * @param       tag             The log tag.
 * @param       fmt             The format.
 ***********************************************************************************************************************
 */
#define OS_KERN_LOG(level, tag, fmt, ...)                                                       \
do                                                                                              \
{                                                                                               \
    if ((level <= *g_klog_global_lvl) || (g_klog_tag_lvl_list->next != g_klog_tag_lvl_list))    \
    {                                                                                           \
        os_kernel_print(level, tag, OS_TRUE, fmt, ##__VA_ARGS__);                               \
    }                                                                                           \
} while (0)

#endif /* KLOG_WITH_FUNC_LINE */

extern const os_int16_t     *g_klog_global_lvl;
extern const os_list_node_t *g_klog_tag_lvl_list;

extern void os_kernel_print(os_uint16_t level, const char *tag, os_bool_t newline, const char *fmt, ...);

#else
#define OS_KERN_LOG(level, tag, fmt, ...)
#endif /* OS_USING_KERNEL_DEBUG */


extern os_task_t  *g_os_current_task;
extern os_task_t  *g_os_next_task;
extern os_task_t  *g_os_high_task;

extern os_int16_t  g_os_sched_lock_cnt;

OS_INLINE os_task_t *k_task_self(void)
{
    return g_os_current_task;
}

extern void      k_tickq_init(void);
extern void      k_tickq_put(struct os_task *task,os_tick_t timeout);
extern void      k_tickq_remove(struct os_task *task);

extern void      k_readyq_put(struct os_task *task);
extern void      k_readyq_remove(struct os_task *task);
extern void      k_readyq_move_tail(struct os_task *task);

extern void      k_sched_init(void);

extern void      k_start(void);

extern void      k_recycle_task_init(void);
extern void      k_idle_task_init(void);

extern void      k_blockq_insert(os_list_node_t *head, struct os_task *task);
extern void      k_block_task(os_list_node_t *head, os_task_t *task, os_tick_t timeout, os_bool_t is_wake_prio);

extern void      k_unblock_task(os_task_t *task);
extern void      k_cancle_all_blocked_task(os_list_node_t *head);

#ifdef OS_USING_SHELL
extern void      k_show_blocked_task(os_task_t **block_task_buff, os_uint16_t buff_cnt_max, os_uint16_t block_task_cnt);
extern void      k_get_blocked_task(os_list_node_t *list_head, os_task_t **block_task_buff, os_uint16_t buff_cnt_max);
#endif

#ifdef OS_USING_TIMER
extern void      k_timer_module_init(void);
extern os_bool_t k_timer_need_handle(void);
extern void      k_move_timer_list_one_step(void);

#ifdef OS_TIMER_SORT
extern os_tick_t k_timer_get_next_remain_ticks(void);
extern void      k_timer_update_active_list(os_tick_t ticks);
#endif

#endif

#ifdef OS_CONFIG_SMP

/* TODO: */
#define OS_KERNEL_ENTER()
#define OS_KERNEL_EXIT()
#define OS_KERNEL_EXIT_SCHED()

#else

extern os_int32_t g_os_kernel_lock_cnt;

#define OS_KERNEL_INIT()    register os_ubase_t    kernel_irq_save

#ifdef OS_USING_KERNEL_LOCK_CHECK
#define OS_KERNEL_ENTER()                                   \
    do                                                      \
    {                                                       \
        kernel_irq_save = os_irq_lock();                    \
        k_kernel_enter_check();                             \
    } while (0)

#define OS_KERNEL_EXIT()                                    \
    do                                                      \
    {                                                       \
        k_kernel_exit_check();                              \
        os_irq_unlock(kernel_irq_save);                     \
    } while (0)

#define OS_KERNEL_LOCK_REF_DEC()                            \
    do                                                      \
    {                                                       \
        g_os_kernel_lock_cnt--;                             \
    } while (0) 

#define OS_KERNEL_LOCK_REF_INC()                            \
    do                                                      \
    {                                                       \
        g_os_kernel_lock_cnt++;                             \
    } while (0)
 
#else
#define OS_KERNEL_ENTER()                                   \
    do                                                      \
    {                                                       \
        kernel_irq_save = os_irq_lock();                    \
    } while (0)

#define OS_KERNEL_EXIT()                                    \
    do                                                      \
    {                                                       \
        os_irq_unlock(kernel_irq_save);                     \
    } while (0)

#define OS_KERNEL_LOCK_REF_DEC()

#define OS_KERNEL_LOCK_REF_INC()

#endif /* OS_USING_KERNEL_LOCK_CHECK */

#define OS_KERNEL_EXIT_SCHED()                                              \
    do                                                                      \
    {                                                                       \
        if ((OS_NULL == g_os_current_task) || (0 != g_os_sched_lock_cnt))   \
        {                                                                   \
            OS_KERNEL_EXIT();                                               \
        }                                                                   \
        else                                                                \
        {                                                                   \
            g_os_next_task = g_os_high_task;                                \
                                                                            \
            if (g_os_current_task != g_os_next_task)                        \
            {                                                               \
                OS_KERNEL_LOCK_REF_DEC();                                   \
                os_task_switch();                                           \
                OS_KERNEL_LOCK_REF_INC();                                   \
            }                                                               \
                                                                            \
            OS_KERNEL_EXIT();                                               \
        }                                                                   \
    } while (0)                                                             \

extern void k_kernel_enter_check(void);
extern void k_kernel_exit_check(void);

#endif /* OS_CONFIG_SMP */

#ifdef __cplusplus
}
#endif

#endif /* __OS_KERNEL_INTERNAL_H__ */

