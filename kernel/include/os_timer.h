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
 * @file        os_timer.h
 *
 * @brief       Header file for timer interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __OS_TIMER_LIST_H__
#define __OS_TIMER_LIST_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_TIMER

#define OS_TIMER_FLAG_ONE_SHOT      0x0U   /* Oneshot timer. */
#define OS_TIMER_FLAG_PERIODIC      0x1U   /* Periodic timer. */

struct os_timer_active_node
{
    os_list_node_t   active_list;
    os_uint8_t       flag;
    os_uint8_t       info;
};

typedef struct os_timer_active_node os_timer_active_node_t;

struct os_timer
{
    void                 (*timeout_func)(void *timeout_param);    /* Timeout function. */
    void                  *parameter;                             /* Timeout function's parameter. */
    os_tick_t              init_ticks;                            /* Timer timeout tick. */
    os_tick_t              round_ticks;                           /* Timeout tick remaining. */
    os_uint32_t            index;
    os_list_node_t         list;
    os_timer_active_node_t active_node;
    char                   name[OS_NAME_MAX + 1];
};

typedef struct os_timer os_timer_t;

#ifdef OS_USING_SYS_HEAP
extern os_timer_t *os_timer_create(const char   *name,
                                   void        (*function)(void *parameter),
                                   void         *parameter,
                                   os_tick_t     timeout,
                                   os_uint8_t    flag);
extern os_err_t    os_timer_destroy(os_timer_t *timer);
#endif

extern os_err_t    os_timer_init(os_timer_t  *timer,
                                 const char  *name,
                                 void       (*function)(void *parameter),
                                 void        *parameter,
                                 os_tick_t    timeout,
                                 os_uint8_t   flag);

extern void        os_timer_deinit(os_timer_t *timer);
extern os_err_t    os_timer_start(os_timer_t *timer);
extern os_err_t    os_timer_stop(os_timer_t *timer);
extern os_err_t    os_timer_set_timeout_ticks(os_timer_t *timer, os_tick_t timeout);
extern os_tick_t   os_timer_get_timeout_ticks(os_timer_t *timer);
extern os_tick_t   os_timer_get_remain_ticks(os_timer_t *timer);
extern os_err_t    os_timer_set_oneshot(os_timer_t *timer);
extern os_err_t    os_timer_set_periodic(os_timer_t *timer);
extern os_bool_t   os_timer_is_periodic(os_timer_t *timer);
extern os_bool_t   os_timer_is_active(os_timer_t *timer);

#endif /* End of OS_USING_TIMER */

#ifdef __cplusplus
}
#endif

#endif
