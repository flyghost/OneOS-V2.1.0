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
 * @file        stubs.c
 *
 * @brief       This file provides file system related interface adaptation.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-14   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <string.h>
#include <rt_sys.h>

#include <oneos_config.h>
#include <os_stddef.h>
#include <os_assert.h>
#include <device.h>
#if defined(OS_USING_CONSOLE)
#include <console.h>
#endif
#include <sys/fcntl.h>
#include "libc_ext.h"
#include <os_task.h>
#include <shell.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

#ifdef __CLANG_ARM
__asm(".global __use_no_semihosting\n\t");
#else
#pragma import(__use_no_semihosting_swi)
#endif

/* Standard IO device handles. */
#define STDIN       0
#define STDOUT      1
#define STDERR      2

/* Standard IO device name defines. */
const char __stdin_name[]  = "STDIN";
const char __stdout_name[] = "STDOUT";
const char __stderr_name[] = "STDERR";

/**
 ***********************************************************************************************************************
 * @brief           Open a file,required by fopen() and freopen()
 *
 * @param[in]       name      File name with path.
 * @param[in]       openmode  A bitmap hose bits mostly correspond directly to the ISO mode specification.
 *
 * @return          File handle,-1 if an error occurs.
 ***********************************************************************************************************************
 */
FILEHANDLE _sys_open(const char *name, int openmode)
{
#ifdef OS_USING_VFS
    int fd;
    int mode = O_RDONLY;
#endif

    /* Register standard Input Output devices. */
    if (strcmp(name, __stdin_name) == 0)
    {
        return (STDIN);
    }
    
    if (strcmp(name, __stdout_name) == 0)
    {
        return (STDOUT);
    }
    
    if (strcmp(name, __stderr_name) == 0)
    {
        return (STDERR);
    }

#ifndef OS_USING_VFS
    errno = ENOTSUP;
    return -1;
#else
    /* Correct openmode from fopen to open */
    if (openmode & OPEN_PLUS)
    {
        if (openmode & OPEN_W)
        {
            mode |= (O_RDWR | O_TRUNC | O_CREAT);
        }
        else if (openmode & OPEN_A)
        {
            mode |= (O_RDWR | O_APPEND | O_CREAT);
        }
        else
        {
            mode |= O_RDWR;
        }
    }
    else
    {
        if (openmode & OPEN_W)
        {
            mode |= (O_WRONLY | O_TRUNC | O_CREAT);
        }
        else if (openmode & OPEN_A)
        {
            mode |= (O_WRONLY | O_APPEND | O_CREAT);
        }
    }

    fd = vfs_open(name, mode, 0);
    if (fd < 0)
    {
        return -1;
    }
    else
    {
        return fd;
    }
#endif
}

/**
 ***********************************************************************************************************************
 * @brief           Closes a file previously opened with _sys_open().
 *
 * @param[in]       fh        File handle.
 *
 * @return          The return value is 0 if successful. A nonzero value indicates an error.
 ***********************************************************************************************************************
 */
int _sys_close(FILEHANDLE fh)
{
#ifndef OS_USING_VFS
    errno = ENOTSUP;
    return -1;
#else
    if (fh <= STDERR)
    {
        return 0;
    }

    return vfs_close(fh);
#endif
}

/**
 ***********************************************************************************************************************
 * @brief           Reads the contents of a file into a buffer.
 *
 * @attention       It is also legal to signal EOF by returning no data but signalling no error (i.e. the top-bit-set 
 *                  mechanism need never be used).
 *                  So if (for example) the user is trying to read 8 bytes at a time from a file in which only 5 remain, 
 *                  this routine can do three equally valid things:
 *                  - it can return 0x80000003 (3 bytes not read due to EOF).
 *                  - OR it can return 3 (3 bytes not read), and then return 0x80000008 (8 bytes not read due to EOF) 
 *                    on the next attempt.
 *                  - OR it can return 3 (3 bytes not read), and then return 8 (8 bytes not read, meaning 0 read, meaning 
 *                    EOF) on the next attempt.
 *
 * @param[in]       fh        File handle.
 * @param[in]       len       Buffer length.
 * @param[in]       mode      For historical reasons.It contains nothing useful and must be ignored.
 * @param[out]      buf       Buffer.
 *
 * @return          Can return:
 *                  - zero if the read was completely successful.
 *                  - the number of bytes _not_ read, if the read was partially successful.
 *                  - the number of bytes not read, plus the top bit set (0x80000000), if the read was partially 
 *                    successful due to end of file.
 *                  - -1 if some error other than EOF occurred.
 ***********************************************************************************************************************
 */
int _sys_read(FILEHANDLE fh, unsigned char *buf, unsigned len, int mode)
{
#ifdef OS_USING_VFS
    int size;
#endif

    if (fh == STDIN)
    {
#ifdef OS_USING_VFS_DEVFS
        size = libc_stdio_read(buf, len);
        return len - size;
#else
        /* no stdin */
        return -1;
#endif
    }

    if ((fh == STDOUT) || (fh == STDERR))
    {
        return -1;
    }

#ifndef OS_USING_VFS
    return 0;
#else
    size = vfs_read(fh, buf, len);
    if (size >= 0)
    {
        return len - size;
    }
    else
    {
        return -1;
    }
#endif
}

/**
 ***********************************************************************************************************************
 * @brief           Writes the contents of a buffer to a file previously opened with _sys_open().
 *
 * @param[in]       fh        File handle.
 * @param[in]       len       Buffer length.
 * @param[in]       mode      For historical reasons.It contains nothing useful and must be ignored.
 * @param[in]       buf       Buffer.
 *
 * @return          A positive number representing the number of characters not written.
 *                  A negative number indicating an error.
 ***********************************************************************************************************************
 */
int _sys_write(FILEHANDLE fh, const unsigned char *buf, unsigned len, int mode)
{
#ifdef OS_USING_VFS
    int size;
#endif

    if ((fh == STDOUT) || (fh == STDERR))
    {
#if !defined(OS_USING_CONSOLE) || !defined(OS_USING_DEVICE)
        return 0;
#else
#ifdef OS_USING_VFS_DEVFS
        size = libc_stdio_write(buf, len);
        return len - size;
#else
        if (os_console_get_device())
        {
            os_device_write_block(os_console_get_device(), -1, buf, len);
            return 0;
        }

        return -1;
#endif
#endif
    }

    if (fh == STDIN)
    {
        return -1;
    }

#ifndef OS_USING_VFS
    return 0;
#else
    size = vfs_write(fh, buf, len);
    if (size >= 0)
    {
        return len - size;
    }
    else
    {
        return -1;
    }
#endif
}

/**
 ***********************************************************************************************************************
 * @brief           Puts the file pointer at offset pos from the beginning of the file.
 *
 * @param[in]       fh        File handle.
 * @param[in]       pos       Offset pos.
 *
 * @return          Negative if an error occurs.
 *                  Non-negative if no error occurs.
 ***********************************************************************************************************************
 */
int _sys_seek(FILEHANDLE fh, long pos)
{
    if (fh < STDERR)
    {
        return -1;
    }

#ifndef OS_USING_VFS
    errno = ENOTSUP;
    return -1;
#else

    /* Position is relative to the start of file fh. */
    return vfs_lseek(fh, pos, 0);
#endif
}

/**
 ***********************************************************************************************************************
 * @brief           Writes a character to the console.
 *
 * @param[in]       ch        Character to write.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void _ttywrch(int ch)
{
#ifdef OS_USING_CONSOLE
    char c;

    c = (char)ch;
    os_kprintf(&c);
#endif
}

/**
 ***********************************************************************************************************************
 * @brief           Return current length of file.
 *
 * @param[in]       fh        File handle.
 *
 * @return          Current length of the file fh
 *                  A negative error indicator
 ***********************************************************************************************************************
 */
long _sys_flen(FILEHANDLE fh)
{
#ifdef OS_USING_VFS
    struct stat stat;
#endif
    if (fh < STDERR)
    {
        return -1;
    }

#ifndef OS_USING_VFS
    errno = ENOTSUP;
    return -1;
#else
    vfs_fstat(fh, &stat);
    return stat.st_size;
#endif
}

int _sys_istty(FILEHANDLE fh)
{
    if ((STDIN <= fh) && (fh <= STDERR))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int remove(const char *filename)
{
#ifndef OS_USING_VFS
    errno = ENOTSUP;
    return -1;
#else
    return vfs_unlink(filename);
#endif
}

int _sys_tmpnam(char *name, int fileno, unsigned maxlength)
{
    errno = ENOTSUP;
    return -1;
}

char *_sys_command_string(char *cmd, int len)
{
    /* no support */
    return OS_NULL;
}

OS_WEAK void _sys_exit(int return_code)
{
    /* TODO: perhaps exit the task which is invoking this function. */
    while (1);
}

/* Use system(const char *string) implementation in the msh. */
int system(const char *string)
{
    int ret = -1;
#if defined(OS_USING_SHELL)
    if (string)
    {
        if (OS_EOK == sh_exec(string))
        {
            ret = 0;
        }
    }
#else
    errno = ENOTSUP;
#endif
    return ret;
}

#ifndef __CLANG_ARM
/* This is armlibc errno stub */
volatile int *__aeabi_errno_addr(void)
{
    return os_errno();
}
#endif

#ifdef __MICROLIB
#include <stdio.h>

int fputc(int c, FILE *f) 
{
    char ch[2] = {0};

    ch[0] = c;
    os_kprintf(&ch[0]);
    return 1;
}

int fgetc(FILE *f) 
{
#ifdef OS_USING_VFS_DEVFS
    char ch;

    if (libc_stdio_read(&ch, 1) == 1)
    {
        return ch;
    }
#endif

    errno = ENOTSUP;
    return -1;
}
#endif
