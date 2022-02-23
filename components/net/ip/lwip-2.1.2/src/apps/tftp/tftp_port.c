/*
 * File      : tftp_port.c
 * This file is part of CMCC IOT OS
 * COPYRIGHT (C) 2012-2020, CMCC IOT
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */


#include <os_task.h>
#include <dfs_posix.h>
#include <lwip/apps/tftp_server.h>

static struct tftp_context ctx;

static void* tftp_open(const char* fname, const char* mode, u8_t write)
{
    int fd = -1;

    if (!strcmp(mode, "octet"))
    {
        if (write)
        {
            fd = open(fname, O_WRONLY | O_CREAT, 0);
        }
        else
        {
            fd = open(fname, O_RDONLY, 0);
        }
    }
    else
    {
        os_kprintf("tftp: No support this mode(%s).\r\n", mode);
    }

    return (void *) fd;
}

static int tftp_write(void* handle, struct pbuf* p)
{
    int fd = (int) handle;

    return write(fd, p->payload, p->len);
}

#if defined(OS_USING_SHELL)
#include <shell.h>

static void tftp_server(uint8_t argc, char **argv)
{
    ctx.open = tftp_open;
    ctx.close = (void (*)(void *)) close;
    ctx.read = (int (*)(void *, void *, int)) read;
    ctx.write = tftp_write;

    if (tftp_init(&ctx) == ERR_OK)
    {
        os_kprintf("TFTP server start successfully.\r\n");
    }
    else
    {
        os_kprintf("TFTP server start failed.\r\n");
    }
}

SH_CMD_EXPORT(tftp_server, tftp_server, "start tftp server.");

#endif /* defined(OS_USING_SHELL) */
