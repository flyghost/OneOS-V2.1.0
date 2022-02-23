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
 * @file        infrared_test.c
 *
 * @brief       The test file for infrared.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <os_clock.h>
#include <shell.h>
#include <string.h>
#include <infrared/infrared.h>
#include <dlog.h>
#define DBG_TAG "infrared_test"

#define INF_READ_BLOCK
#define INF_WRITE_BLOCK

static void infrared_recv_task(void *parameter)
{
    os_device_t *infrared;
    struct os_infrared_info info;
    int infrared_rx_count = 0;

    infrared = os_device_find("atk_rmt");
    OS_ASSERT(infrared);

    os_device_open(infrared);

    while (1)
    {
#ifdef INF_READ_BLOCK
        os_device_read_block(infrared, 0, &info, sizeof(info));
#else
        if (os_device_read_nonblock(infrared, 0, &info, sizeof(info)) != sizeof(info))
        {
            os_task_msleep(100);
            continue;
        }
#endif
        LOG_I(DBG_TAG,"infrared_rx_done(%d) addr: %02x, data: %02x", ++infrared_rx_count, info.addr, info.data);
    }
}

static int infrared_recv_test(void)
{
    os_task_t *task;

    task = os_task_create("ir_recv", infrared_recv_task, NULL, 2048, 3);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

static int infrared_send_test(int argc, char **argv)
{
    os_uint8_t addr = 0x5a;
    os_uint8_t data = 0x3c;
    int i, repeat, loops = 1;

    if (argc == 1 || (argc == 2 && !strcmp(argv[1], "help")))
    {
        LOG_I(DBG_TAG,"usage:");
        LOG_I(DBG_TAG,"infrared_send_test [addr] [data] [repeat] [loops]");
        return 0;
    }

    if (argc > 1)
    {
        addr = strtol(argv[1], NULL, 0);
    }

    if (argc > 2)
    {
        data = strtol(argv[2], NULL, 0);
    }

    if (argc > 3)
    {
        repeat = strtol(argv[3], NULL, 0);
    }

    if (argc > 4)
    {
        loops = strtol(argv[4], NULL, 0);
    }

    os_device_t *infrared;
    struct os_infrared_info info;

    infrared = os_device_find("atk_rmt");
    OS_ASSERT(infrared);

    os_device_open(infrared);

    os_task_t *self = os_task_self();
    os_uint8_t task_prio = os_task_get_priority(self);
    os_uint8_t high_prio = 1;    
    os_task_set_priority(self, high_prio);

    for (i = 0; i < loops; i++)
    {
        os_task_msleep(1000);

        info.addr  = addr;
        info.data  = data + i;
        info.times = repeat;
        
#ifdef INF_WRITE_BLOCK
        os_device_write_block(infrared, 0, &info, sizeof(info));
#else
        while (os_device_write_nonblock(infrared, 0, &info, sizeof(info)) != sizeof(info));
#endif
    }

    os_device_close(infrared);

    os_task_set_priority(self, task_prio);

    return 0;
}

SH_CMD_EXPORT(infrared_recv_test, infrared_recv_test, "infrared_recv_test");
SH_CMD_EXPORT(infrared_send_test, infrared_send_test, "infrared_send_test");
