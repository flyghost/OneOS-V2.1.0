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
 * @file        cputime.h
 *
 * \@brief      This file provides struct definition and cputime functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef HRTIMER_H__
#define HRTIMER_H__

#include <os_types.h>
#include <timer/timer.h>

enum os_hrtimer_state {
    OS_HRTIMER_STATE_NONE,
    OS_HRTIMER_STATE_WAIT,
    OS_HRTIMER_STATE_RUN,       // 运行态
};

struct os_hrtimer
{
    void (*timeout_func)(void *parameter);      /* callback function when hrtimer timeout */
    void *parameter;                            /* parameter for callback function */

    os_uint64_t timeout_nsec;                   /* first timeout time */
    os_uint64_t period_nsec;                    /* period timeout time */
    os_uint64_t next_nsec;

    enum os_hrtimer_state state;

    os_list_node_t list;
};
typedef struct os_hrtimer os_hrtimer_t;

void os_hrtimer_start(os_hrtimer_t *hrtimer);
void os_hrtimer_stop(os_hrtimer_t *hrtimer);

os_bool_t os_hrtimer_stoped(os_hrtimer_t *hrtimer);

os_err_t os_hrtimer_init(void);

#endif  /* HRTIMER_H__ */
