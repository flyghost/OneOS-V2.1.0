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
 * @file        yaffs_osglue.c
 *
 * @brief       This file is adapter for OS operation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-09-16   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <os_mutex.h>
#include <sys/time.h>

#include "yaffs/yaffs_guts.h"
#include "yaffs/yaffs_trace.h"

static os_mutex_t *mutex = OS_NULL;
static int yaffs_err;
unsigned int yaffs_trace_mask = YAFFS_TRACE_MOUNT|YAFFS_TRACE_ERROR|YAFFS_TRACE_BUG;

void yaffsfs_SetError(int err)
{
    yaffs_err = err;
}

int yaffsfs_GetLastError(void)
{
    return yaffs_err;
}

void yaffsfs_Lock(void)
{
    os_mutex_lock(mutex, OS_WAIT_FOREVER);
}

void yaffsfs_Unlock(void)
{
    os_mutex_unlock(mutex);
}

void yaffsfs_LockInit(void)
{
    mutex = os_mutex_create("ymutex", OS_FALSE);
}

void yaffsfs_LockDestroy(void)
{
    os_mutex_destroy(mutex);
}


u32 yaffsfs_CurrentTime(void)
{
#ifdef OS_USING_LIBC_ADAPTER 
    time_t now;

    now = time(OS_NULL);
    return now;
#else
    return 0;
#endif
}

void *yaffsfs_malloc(size_t size)
{
    void *ptr = os_malloc(size);

    if (!ptr)
    {
        LOG_E("YAFFS", "ERR: malloc fail!!! size:%d", size);
    }
    return ptr;
}

void yaffsfs_free(void *ptr)
{
	if (ptr == OS_NULL) return;
    os_free(ptr);
}

void yaffsfs_OSInitialisation(void)
{
    yaffsfs_LockInit();
}

int yaffsfs_CheckMemRegion(const void *addr, size_t size, int write_request)
{
    return 0;
}

void yaffs_bug_fn(const char *file_name, int line_no)
{
    LOG_E("YAFFS", "bug_fn:%s line_no:%d", file_name, line_no);
}

