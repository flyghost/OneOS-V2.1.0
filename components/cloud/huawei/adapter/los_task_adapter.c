/*
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
 * @file        los_task_adapter.c
 *
 * @brief       huawei cloud sdk file "los_task.c" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <os_task.h>
#include <os_assert.h>
#include <string.h>
#include "los_task_adapter.h"

typedef void (*entry)(void *parameter);

UINT32 LOS_TaskCreate(UINT32 *taskID, TSK_INIT_PARAM_S *initParam, void **task)
{
    const os_uint8_t task_prio_adapter     = 20;    // To be decided
    char             name[OS_NAME_MAX + 1] = {0};
    size_t           len                   = strlen((const char *)initParam->pcName);

    len = len > OS_NAME_MAX ? OS_NAME_MAX : len;
    memcpy(name, initParam->pcName, len);
    os_task_t *os_task = os_task_create(name,
                                        (entry)initParam->pfnTaskEntry,
                                        OS_NULL,
                                        initParam->uwStackSize,
                                        task_prio_adapter + initParam->usTaskPrio);
    OS_ASSERT(os_task != OS_NULL);
    *task = (void *)os_task;
    return os_task_startup(os_task);
}

VOID LOS_TaskDestroy(void *task)
{
    if (task == NULL)
        return;
    os_task_destroy((os_task_t *)task);
    os_task_tsleep(0);
}

UINT32 LOS_TaskDelay(UINT32 tick)
{
    return os_task_tsleep(tick);
}
