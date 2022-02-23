/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2018,RT - Thread Development Team
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
 * @file        console_be.c
 *
 * @brief       Console backend realization.
 *
 * @revision
 * Date         Author          Notes
 * 2018-09-04   armink          First version.
 * 2020-03-25   OneOS Team      Format and request resource.
 ***********************************************************************************************************************
 */

#include <arch_interrupt.h>
#include <os_errno.h>
#include <string.h>
#include <dlog.h>
#include <os_assert.h>
#include <board.h>

#ifdef DLOG_BACKEND_USING_CONSOLE

static dlog_backend_t gs_console;

static void dlog_console_backend_output(struct dlog_backend *backend, char *log, os_size_t len)
{
    OS_UNREFERENCE(backend);
    OS_UNREFERENCE(len);

    OS_ASSERT(OS_NULL != log);

    os_hw_console_output(log);

    return;
}

static os_err_t dlog_console_backend_init(void)
{
    os_err_t ret;

    ret = dlog_init();
    if (OS_EOK == ret)
    {
        memset(&gs_console, 0, sizeof(dlog_backend_t));
        strncpy(gs_console.name, "console", OS_NAME_MAX);

        gs_console.support_isr   = OS_TRUE;
        gs_console.support_color = OS_TRUE;
        gs_console.output        = dlog_console_backend_output;
        
        (void)dlog_backend_register(&gs_console);
    }
    
    return ret;
}
OS_PREV_INIT(dlog_console_backend_init, OS_INIT_SUBLEVEL_MIDDLE);

#endif /* DLOG_BACKEND_USING_CONSOLE */

