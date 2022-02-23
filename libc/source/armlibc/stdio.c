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
 * @file        stdio.c
 *
 * @brief       Standard input and output interface adaptation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-14   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <oneos_config.h>
#include <os_util.h>
#include "libc_ext.h"

#if defined(OS_USING_VFS) && defined(OS_USING_VFS_DEVFS)
#include <vfs_posix.h>

#define STDIO_DEVICE_NAME_MAX   32

static int std_fd = -1;
int libc_stdio_set_console(const char* device_name, int mode)
{
    int fd;
    char name[STDIO_DEVICE_NAME_MAX];

    snprintf(name, sizeof(name) - 1, "/dev/%s", device_name);
    name[STDIO_DEVICE_NAME_MAX - 1] = '\0';

    fd = open(name, mode, 0);
    if (fd >= 0)
    {
        if (std_fd >= 0)
        {
            close(std_fd);
        }
        std_fd = fd;
    }

    return std_fd;
}

int libc_stdio_get_console(void)
{
    return std_fd;
}

int libc_stdio_read(void *buffer, size_t size)
{
    if (std_fd >= 0)
    {
        return read(std_fd, buffer, size);
    }
    else
    {
        os_kprintf("Illegal stdio input!\n");
        return 0;
    }
}

int libc_stdio_write(const void *buffer, size_t size)
{
    if (std_fd >= 0)
    {
        return write(std_fd, buffer, size);
    }
    else
    {
        os_kprintf("Illegal stdio output!\n");
        return size;
    }
}
#endif
