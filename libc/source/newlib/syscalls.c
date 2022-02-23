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
 * @file        syscalls.c
 *
 * @brief       This file provides system call related adaptation, including file system, memory management, time 
 *              management,etc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-17   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <reent.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <oneos_config.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <os_util.h>
#include <os_task.h>
#include <os_clock.h>
#include <os_memory.h>
#include <device.h>
#include <shell.h>
#if defined(OS_USING_RTC)
#include <rtc.h>
#endif

#ifdef OS_USING_VFS
#include <vfs_posix.h>
#endif

#ifdef OS_USING_CONSOLE
#include <console.h>
#endif

#include <os_errno.h>

/* Reentrant versions of system calls.  */

#ifndef _REENT_ONLY
int *__errno()
{
    return os_errno();
}
#endif

int _close_r(struct _reent *ptr, int fd)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    return vfs_close(fd);
#endif
}

int _execve_r(struct _reent *ptr, const char * name, char *const *argv, char *const *env)
{
    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

int _fcntl_r(struct _reent *ptr, int fd, int cmd, int arg)
{
#ifndef OS_USING_VFS
    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    int rc;

    rc = vfs_fcntl(fd, cmd, arg);
    return rc;
#endif
}

int _fork_r(struct _reent *ptr)
{
    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

int _fstat_r(struct _reent *ptr, int fd, struct stat *pstat)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    int rc;

    rc = vfs_fstat(fd, pstat);
    return rc;
#endif
}

int _getpid_r(struct _reent *ptr)
{
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

int _isatty_r(struct _reent *ptr, int fd)
{
    if (fd >= 0 && fd < 3)
    {
        return 1;
    }

    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

int _kill_r(struct _reent *ptr, int pid, int sig)
{
    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

int _link_r(struct _reent *ptr, const char *old, const char *new)
{
    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

_off_t _lseek_r(struct _reent *ptr, int fd, _off_t pos, int whence)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    _off_t rc;

    rc = vfs_lseek(fd, pos, whence);
    return rc;
#endif
}

int _mkdir_r(struct _reent *ptr, const char *name, int mode)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    int rc;

    rc = vfs_mkdir(name, mode);
    return rc;
#endif
}

int _open_r(struct _reent *ptr, const char *file, int flags, int mode)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    int rc;

    rc = vfs_open(file, flags, mode);
    return rc;
#endif
}

_ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t nbytes)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    _ssize_t rc;

    rc = vfs_read(fd, buf, nbytes);
    return rc;
#endif
}

int _rename_r(struct _reent *ptr, const char *old, const char *new)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    int rc;

    rc = vfs_rename(old, new);
    return rc;
#endif
}

void *_sbrk_r(struct _reent *ptr, ptrdiff_t incr)
{
    /* No use this routine to get memory. */
    return OS_NULL;
}

int _stat_r(struct _reent *ptr, const char *file, struct stat *pstat)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    int rc;

    rc = vfs_stat(file, pstat);
    return rc;
#endif
}

_CLOCK_T_ _times_r(struct _reent *ptr, struct tms *ptms)
{
    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

int _unlink_r(struct _reent *ptr, const char *file)
{
#ifndef OS_USING_VFS
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
#else
    int rc;

    rc = vfs_unlink(file);
    return rc;
#endif
}

int _wait_r(struct _reent *ptr, int *status)
{
    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}

#if defined(OS_USING_DEVICE) /* && defined(OS_USING_CONSOLE) */ // TODO: need check
_ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t nbytes)
{
#ifndef OS_USING_VFS
    if (fileno(stdout) == fd)
    {
        os_device_t *console;

    #ifdef OS_USING_CONSOLE
        console = os_console_get_device();
    #else
        console = OS_NULL;
    #endif
        if (console)
        {
            return os_device_write_block(console, -1, buf, nbytes);
        }
    }

    return 0;
#else
    _ssize_t rc;

    rc = vfs_write(fd, buf, nbytes);
    return rc;
#endif
}
#endif

#define MILLISECOND_PER_SECOND  1000UL
#define MICROSECOND_PER_SECOND  1000000UL
#define NANOSECOND_PER_SECOND   1000000000UL

#define MILLISECOND_PER_TICK    (MILLISECOND_PER_SECOND / OS_TICK_PER_SECOND)
#define MICROSECOND_PER_TICK    (MICROSECOND_PER_SECOND / OS_TICK_PER_SECOND)
#define NANOSECOND_PER_TICK     (NANOSECOND_PER_SECOND  / OS_TICK_PER_SECOND)

struct timeval _timevalue = {0};
#ifdef OS_USING_DEVICE
static void libc_system_time_init(void)
{
    time_t       time;
    os_tick_t    tick;
    os_device_t *device;

    time = 0;
    device = os_device_find("rtc");
    if (device != OS_NULL)
    {
        /* Get realtime seconds. */
    #if defined(OS_USING_RTC)
        os_device_control(device, OS_DEVICE_CTRL_RTC_GET_TIME, &time);
    #endif
    }

    /* Get tick. */
    tick = os_tick_get();

    _timevalue.tv_usec = MICROSECOND_PER_SECOND - (tick % OS_TICK_PER_SECOND) * MICROSECOND_PER_TICK;
    _timevalue.tv_sec  = time - tick / OS_TICK_PER_SECOND - 1;
}
#endif

int libc_get_time(struct timespec *time)
{
    os_tick_t tick;
    static os_bool_t inited = 0;

    OS_ASSERT(time != OS_NULL);

    /* Initialize system time. */
    if (inited == 0)
    {
        libc_system_time_init();
        inited = 1;
    }

    /* Get tick. */
    tick = os_tick_get();

    time->tv_sec  = _timevalue.tv_sec + tick / OS_TICK_PER_SECOND;
    time->tv_nsec = (_timevalue.tv_usec + (tick % OS_TICK_PER_SECOND) * MICROSECOND_PER_TICK) * 1000;

    return 0;
}

int _gettimeofday_r(struct _reent *ptr, struct timeval *__tp, void *__tzp)
{
    struct timespec tp;

    if (libc_get_time(&tp) == 0)
    {
        if (__tp != OS_NULL)
        {
            __tp->tv_sec  = tp.tv_sec;
            __tp->tv_usec = tp.tv_nsec / 1000UL;
        }

        return tp.tv_sec;
    }

    /* Return "not supported". */
    ptr->_errno = ENOTSUP;
    errno       = ENOTSUP;
    return -1;
}


void *_malloc_r(struct _reent *ptr, size_t size)
{
    void* result;
#if defined(OS_USING_SYS_HEAP)
    result = (void*)os_malloc(size);
#else
    result = OS_NULL;
#endif
    if (result == OS_NULL)
    {
        ptr->_errno = ENOMEM;
        errno       = ENOMEM;
    }

    return result;
}

void *_realloc_r(struct _reent *ptr, void *old, size_t newlen)
{
    void* result;

#if defined(OS_USING_SYS_HEAP)
    result = (void*)os_realloc(old, newlen);
#else
    result = OS_NULL;
#endif
    if (result == OS_NULL)
    {
        ptr->_errno = ENOMEM;
        errno       = ENOMEM;
    }

    return result;
}

void *_calloc_r(struct _reent *ptr, size_t size, size_t len)
{
    void* result;

#if defined(OS_USING_SYS_HEAP)
    result = (void*)os_calloc(size, len);
#else
    result = OS_NULL;
#endif
    if (result == OS_NULL)
    {
        ptr->_errno = ENOMEM;
        errno       = ENOMEM;
    }

    return result;
}

void _free_r(struct _reent *ptr, void *addr)
{
#if defined(OS_USING_SYS_HEAP)
    if (addr != OS_NULL)
    {
        os_free(addr);
    }
#endif
}

void exit(int status)
{
    os_kprintf("task:%s exit with %d\n", os_task_name(os_task_self()), status);
    OS_ASSERT(0);

    while (1);
}

void _system(const char *s)
{
#if defined(OS_USING_SHELL)
    if (s)
    {
        if (OS_EOK == sh_exec(s))
        {
            ;
        }
    }
#else
    errno = ENOTSUP;
#endif
}

void __libc_init_array(void)
{
    /* We not use __libc init_aray to initialize C++ objects. */
}

extern os_err_t os_task_suspend(os_task_t *task);
extern void os_schedule(void);

void abort(void)
{
    if (os_task_self())
    {
        os_task_t *self = os_task_self();

        os_kprintf("task:%-8.*s abort!\n", OS_NAME_MAX, os_task_name(self));
        os_task_suspend(self);
    }

    while (1);
}
