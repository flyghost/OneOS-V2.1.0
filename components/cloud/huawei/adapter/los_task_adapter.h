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
 * @file        los_task_adapter.h
 *
 * @brief       huawei cloud sdk file "los_task.h" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __LOS_TASK_ADAPTER__
#define __LOS_TASK_ADAPTER__

#include "los_config_adapter.h"
#include "los_typedef_adapter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef VOID *(*TSK_ENTRY_FUNC)(UINTPTR param1, UINTPTR param2, UINTPTR param3, UINTPTR param4);
typedef struct tagTskInitParam
{
    TSK_ENTRY_FUNC pfnTaskEntry; /**< Task entrance function */
    UINT16         usTaskPrio;   /**< Task priority */
    UINTPTR        auwArgs[4];   /**< Task parameters, of which the maximum number is four */
    UINT32         uwStackSize;  /**< Task stack size */
    CHAR *         pcName;       /**< Task name */
    UINT32         uwResved;     /**< It is automatically deleted if set to LOS_TASK_STATUS_DETACHED.
                                      It is unable to be deleted if set to 0. */
} TSK_INIT_PARAM_S;

UINT32 LOS_TaskCreate(UINT32 *taskID, TSK_INIT_PARAM_S *initParam, void **task);
VOID   LOS_TaskDestroy(void *task);
UINT32 LOS_TaskDelay(UINT32 tick);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LOS_TASK_ADAPTER__ */
