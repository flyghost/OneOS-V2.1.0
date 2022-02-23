/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        misc_evt.h
 *
 * @brief       Create a task to deal some misc funcs.
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2021-06-07    OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __MISC_EVT_H__
#define __MISC_EVT_H__

#include <os_types.h>
#include <os_mq.h>

typedef int (*misc_evt_handle_t)(os_uint32_t misc_evt);

typedef struct bt_hci_log
{
    misc_evt_handle_t handle;
    os_uint32_t arg;
} misc_evt_t;

os_mq_t *misc_evt_mq_get(void);

int misc_evt_init(void);

#endif
