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
 * @file        cli.c
 *
 * @brief
 *
 * @revision
 * Date         Author          Notes
 * 2021-07-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "oneos_config.h"

#ifdef OS_USING_CLI
#include "cli.h"
#include <os_memory.h>
#include <shell.h>
#include <stdio.h>
#include <string.h>

#include <dlog.h>
#define TAG "CLI"

#define CLI_DEBUG 1

static cli_status sg_cli_status;

#define set_cli_status(status) sg_cli_status = status
#define get_cli_status()       sg_cli_status

int cli_control_process(struct os_cli_device *cli)
{
#define start_terminal        "start terminal"
#define end_terminal          "stop terminal"
#define start_terminal_sucess "start terminal success"
#define end_terminal_sucess   "stop terminal success"
#define control_cmd_error     "control cmd error"

    int     length;
    uint8_t cmd_str[64];

    if (cli->is_open == OS_FALSE)
    {
        return -1;
    }

    length = os_device_read_block(&cli->dev, 0, cmd_str, sizeof(cmd_str) - 1);

    if (length <= 0)
    {
        return 0;
    }

    cmd_str[length] = '\0';

    if (length > 1)
    {
        if (!strcmp((char *)cmd_str, start_terminal))
        {
            set_cli_status(CLI_OPEN);
        }
        else if (!strcmp((char *)cmd_str, end_terminal))
        {
            set_cli_status(CLI_CLOSE);
        }
        else
        {
            os_device_write_nonblock(&cli->dev, 1, control_cmd_error, strlen(control_cmd_error));
        }
    }
#if defined(CLI_DEBUG) && (CLI_DEBUG) /* for user debug */
    else
    {
        if (cmd_str[0] == '1')
        {
            set_cli_status(CLI_OPEN);
        }
        else if (cmd_str[0] == '0')
        {
            set_cli_status(CLI_CLOSE);
        }
        else
        {
            os_device_write_nonblock(&cli->dev, 1, control_cmd_error, strlen(control_cmd_error));
        }
    }
#endif
    return 0;
}

void cli_task(void *param)
{
    os_uint16_t           try_time;
    os_uint32_t           rd_size;
    os_ubase_t            irq_save;
    os_uint8_t *          recv_buf    = OS_NULL;
    struct os_cli_device *control_dev = OS_NULL;
    struct os_cli_device *data_dev    = OS_NULL;
    os_kprintf("try connect cli server ");

    for (int i = 0; i < 5; i++)
    {
        os_kprintf(".");
        os_task_msleep(1000);
    }

    printf("OK\r\n");
    control_dev = (struct os_cli_device *)os_device_find(CLI_CONTROL_DEVICE_NAM);
    if (!control_dev)
    {
        LOG_W(TAG, "not find cli control device.");
        return;
    }
    os_device_open(&control_dev->dev);
    data_dev = (struct os_cli_device *)os_device_find(CLI_DATA_DEVICE_NAME);
    if ((!data_dev) || (!data_dev->has_send_buf))
    {
        LOG_W(TAG, "not cli data device,or device is error.");
        return;
    }

    recv_buf = os_malloc(1024);
    OS_ASSERT(recv_buf != OS_NULL);
    /*init cli status*/
    set_cli_status(CLI_IDLE);
    try_time = 0;

    for (;;)
    {
        switch (get_cli_status())
        {
        case CLI_IDLE:
        {
            if (cli_control_process(control_dev) < 0)
            {
                LOG_I(TAG, "try connect cli server(%d).", ++try_time);

                if (OS_EOK != os_device_control(&control_dev->dev, NET_DEVICE_CONNECT, OS_NULL))
                {
                    os_task_msleep(1000 * 10);
                }
            }

            os_task_msleep(100);
            break;
        }

        case CLI_OPEN:
        {
            LOG_I(TAG, "device enter cli mode.");
            sh_disconnect_console();
            os_console_set_device(CLI_DATA_DEVICE_NAME);
            sh_reconnect_console();
            os_device_control(&data_dev->dev, NET_DEVICE_CONNECT, OS_NULL);
            os_device_write_nonblock(&control_dev->dev, 1, start_terminal_sucess, strlen(start_terminal_sucess));
            set_cli_status(CLI_RUN);
            break;
        }

        case CLI_CLOSE:
        {
            os_device_write_nonblock(&control_dev->dev, 1, end_terminal_sucess, strlen(end_terminal_sucess));
            sh_disconnect_console();
            os_console_set_device(OS_CONSOLE_DEVICE_NAME);
            sh_reconnect_console();
            set_cli_status(CLI_IDLE);
            LOG_I(TAG, "device exit cli mode.");
            try_time = 0;
            break;
        }

        case CLI_RUN:
        {
            os_task_msleep(10);

            /*listen control*/
            if (cli_control_process(control_dev) < 0)
            {
                set_cli_status(CLI_CLOSE);
            }

            /* release recv notify for cli*/
            os_device_recv_notify(&data_dev->dev);

            /*loop pop buffer*/
            for (;;)
            {
                if (data_dev->is_open == OS_FALSE)
                {
                    set_cli_status(CLI_CLOSE);
                    break;
                }

                irq_save = os_irq_lock();
                rd_size  = rb_ring_buff_get(data_dev->send_ring_buf, recv_buf, 1024);
                os_irq_unlock(irq_save);

                if (rd_size <= 0)
                {
                    break;
                }

                os_device_write_nonblock(&data_dev->dev, 1, recv_buf, rd_size);
            }

            break;
        }

        default:
            break;
        }
    }
}

os_err_t cli_start(void)
{
    os_task_t *task = OS_NULL;
    task            = os_task_create("cli", cli_task, NULL, 2048, 21);
    os_task_startup(task);
    return OS_EOK;
}
OS_APP_INIT(cli_start, OS_INIT_SUBLEVEL_LOW);
#endif
