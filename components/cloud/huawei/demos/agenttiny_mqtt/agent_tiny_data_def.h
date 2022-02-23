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
 * @file        agent_tiny_data_def.h
 *
 * @brief       Data format of APP message defined on cloud platform
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __AGENT_TINY_DATA_DEF_H_
#define __AGENT_TINY_DATA_DEF_H_
#include <os_mutex.h>
#include <os_mq.h>
#include "los_typedef_adapter.h"

#define MQ_MAX_MSG 2

typedef enum
{
    APP_MESSAGE_NULL,
    APP_MESSAGE_JSON,      // Data format: json
    APP_MESSAGE_BINARY,    // Data format: Binary code stream
    APP_MESSAGE_MAX
} app_message_type_t;

/*  app_data_t:
    User app data */
typedef struct
{
    struct os_mutex   *mutex;
    app_message_type_t type;
    size_t             len;
    void              *buff;
} app_data_t;

typedef void (*entry)(void *parameter);
#endif
