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
 * @file        misc_evt.c
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
#include <os_assert.h>
#include <os_clock.h>
#include <os_memory.h>
#include <arch_interrupt.h>
#include <os_mq.h>
#include <os_mutex.h>
#include <stdint.h>
#include <stdlib.h>
#include "misc_evt.h"

#define MQ_MAX_MSG 16
#define MSG_SIZE sizeof(misc_evt_t)
#define MQ_POLL_SIZE (MQ_MAX_MSG * (MSG_SIZE + sizeof(os_mq_msg_hdr_t)))
static char s_mq_pool[MQ_POLL_SIZE];
static os_mq_t s_mq;

os_mq_t *misc_evt_mq_get(void)
{
    return &s_mq;
}

static void misc_evt_deal(void *parameter)
{
    os_size_t recv_size = 0;
    misc_evt_t recv_evt;
    while (1)
    {
        os_mq_recv(&s_mq, &recv_evt, MSG_SIZE, OS_WAIT_FOREVER, &recv_size);
        recv_evt.handle(recv_evt.arg);
    }
}

int misc_evt_init(void)
{
    os_err_t ret;
    os_task_t *task;

    ret = os_mq_init(&s_mq, "msgqueue", s_mq_pool, MQ_POLL_SIZE, MSG_SIZE);
    OS_ASSERT(OS_EOK == ret);

    task = os_task_create("misc_evt", misc_evt_deal, NULL, 512, 2);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

