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
 * @file        cli_cms.c
 *
 * @brief
 *
 * @revision
 * Date         Author          Notes
 * 2021-07-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include "oneos_config.h"

#if defined(OS_USING_CLI) && defined(CLI_PAYLOAD_BY_CMS)
#include "cli.h"

#if defined(CMS_CONNECT_MQTT) || defined(CMS_CONNECT_TCP)
#include "cms_con_tcp.h"
#include <os_memory.h>

#ifndef CLI_DATA_PAYLOAD_SCODE
#define CLI_DATA_PAYLOAD_SCODE (2001)
#endif

#ifndef CLI_CONTROL_PAYLOAD_SCODE
#define CLI_CONTROL_PAYLOAD_SCODE (2000)
#endif

static os_err_t cli_control(os_device_t *dev, os_int32_t cmd, void *args)
{
    struct os_cli_device *cli = (struct os_cli_device *)dev;
    OS_ASSERT((cmd == NET_DEVICE_CONNECT) || (cmd == NET_DEVICE_DISCONNECT));

    if (cmd == NET_DEVICE_CONNECT)
    {
        if (cli->has_send_buf)
        {
            rb_ring_buff_reset(cli->send_ring_buf);
        }

        if (cms_tcp_connect(cli->handle) == cms_connect_success)
        {
            cli->is_open = OS_TRUE;
            return OS_EOK;
        }

        return OS_ERROR;
    }
    else if (cmd == NET_DEVICE_DISCONNECT)
    {
        if (cms_tcp_get_state(cli->handle) != cms_con_state_disconnect)
        {
            cms_tcp_disconnect(cli->handle);
        }

        cli->is_open = OS_FALSE;
        return OS_EOK;
    }

    return OS_ERROR;
}

static os_err_t cli_init(os_device_t *dev)
{
    struct os_cli_device *cli = (struct os_cli_device *)dev;

    if (cli->handle == OS_NULL)
    {
        cli->handle = cms_tcp_init(cli->scode, cli->buff_size);
    }

    if (cms_tcp_get_state(cli->handle) == cms_con_state_connect)
    {
        cli->is_open = OS_TRUE;
    }
    else
    {
        cli->is_open = OS_FALSE;
    }

    return OS_EOK;
}

static os_err_t cli_deinit(os_device_t *dev)
{
    struct os_cli_device *cli = (struct os_cli_device *)dev;

    if (cli->handle == OS_NULL)
    {
        return OS_EOK;
    }

    cms_tcp_deinit(cli->handle);
    cli->is_open = OS_FALSE;
    cli->handle  = OS_NULL;
    return OS_EOK;
}

static os_size_t cli_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    int                   result;
    struct os_cli_device *cli = (struct os_cli_device *)dev;
    result                    = cms_tcp_recv(cli->handle, buffer, size, 10);

    if (result < 0)
    {
        cli->is_open = OS_FALSE;
        return 0;
    }

    return result;
}

static os_size_t cli_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    int                   result;
    struct os_cli_device *cli = (struct os_cli_device *)dev;

    if (pos > 0)
    {
        result = cms_tcp_send(cli->handle, buffer, size);

        if (result < 0)
        {
            cli->is_open = OS_FALSE;
            return 0;
        }
    }
    else
    {
        OS_ASSERT(cli->has_send_buf);
        /* NOTE:no use mutex lock! see os_util.c os_kprintf */
        result = rb_ring_buff_put_force(cli->send_ring_buf, buffer, size);
    }

    return result;
}

const static struct os_device_ops cli_ops = {
    .init    = cli_init,
    .deinit  = cli_deinit,
    .read    = cli_read,
    .write   = cli_write,
    .control = cli_control,
};

static os_err_t cli_register(const char *name, os_bool_t has_send_buf, uint32_t scode, uint16_t buf_size)
{
    os_err_t              ret;
    os_device_t *         dev = OS_NULL;
    struct os_cli_device *cli = os_calloc(1, sizeof(struct os_cli_device));
    cli->scode                = scode;
    cli->buff_size            = buf_size;
    cli->is_open              = OS_FALSE;
    cli->has_send_buf         = has_send_buf;

    if (has_send_buf == OS_TRUE)
    {
        cli->send_ring_buf = rb_ring_buff_create(buf_size + 1024);
        OS_ASSERT(cli->send_ring_buf);
    }

    dev            = &cli->dev;
    dev->type      = OS_DEVICE_TYPE_CHAR;
    dev->ops       = &cli_ops;
    dev->user_data = cli;
    ret            = os_device_register(dev, name);
    return ret;
}

os_err_t cli_register_all(void)
{
    os_err_t ret;
    ret = cli_register(CLI_CONTROL_DEVICE_NAM, OS_FALSE, CLI_CONTROL_PAYLOAD_SCODE, 512);
    ret = cli_register(CLI_DATA_DEVICE_NAME, OS_TRUE, CLI_DATA_PAYLOAD_SCODE, 1024);
    return ret;
}

OS_ENV_INIT(cli_register_all, OS_INIT_SUBLEVEL_LOW);
#endif
#endif
