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
 * @file        sched.h
 *
 * @brief       Posix scheduling policies definition and function declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SCHED_H__
#define __SCHED_H__

#include <pthread.h>

/* Thread scheduling policies. */
enum
{
    SCHED_OTHER = 0,
    SCHED_FIFO,
    SCHED_RR,
    SCHED_MIN   = SCHED_OTHER,
    SCHED_MAX   = SCHED_RR
};

#ifdef __cplusplus
extern "C"
{
#endif

int sched_yield(void);
int sched_get_priority_min(int policy);
int sched_get_priority_max(int policy);
int sched_setscheduler(pid_t pid, int policy);

#ifdef __cplusplus
}
#endif

#endif
