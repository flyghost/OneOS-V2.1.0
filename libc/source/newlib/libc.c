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
 * @file        libc.c
 *
 * @brief       This file provides the libc system initialization function.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <oneos_config.h>
#include <device.h>
#if defined(OS_USING_CONSOLE)
#include <console.h>
#endif
#include <fcntl.h>
#include "libc_ext.h"

int putenv(char *string);

int libc_system_init(void)
{
#if defined(OS_USING_VFS) && defined(OS_USING_VFS_DEVFS) && defined(OS_USING_CONSOLE)
    os_device_t *dev_console;

    dev_console = os_console_get_device();
    if (dev_console)
    {
        libc_stdio_set_console(dev_console->name, O_RDWR);
    }
#endif

    return 0;
}
OS_CMPOENT_INIT(libc_system_init, "2");
