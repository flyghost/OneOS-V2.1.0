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
 * @file        sched.c
 *
 * @brief       This file provides posix sched functions implementation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-28   OneOS team      First Version
 ***********************************************************************************************************************
 */
#include <sched.h>

int sched_yield(void)
{
    os_task_yield();

    return 0;
}
EXPORT_SYMBOL(sched_yield);

int sched_get_priority_min(int policy)
{
    if ((SCHED_FIFO != policy) && (SCHED_RR != policy))
    {
        return EINVAL;
    }
    
    return 0;
}
EXPORT_SYMBOL(sched_get_priority_min);

int sched_get_priority_max(int policy)
{
    if ((SCHED_FIFO != policy) && (SCHED_RR != policy))
    {
        return EINVAL;
    }

    return OS_TASK_PRIORITY_MAX - 1;
}
EXPORT_SYMBOL(sched_get_priority_max);

int sched_setscheduler(pid_t pid, int policy)
{
    return EOPNOTSUPP;
}
EXPORT_SYMBOL(sched_setscheduler);
