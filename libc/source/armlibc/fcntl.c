#include <oneos_config.h>
#include <extension/errno_ext.h>
#include <os_stddef.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int fcntl(int fd, int flag, ...)
{
    int ret   = -1;
#ifdef OS_USING_VFS
    va_list va;
    void *arg = OS_NULL;

    va_start(va, flag);
    arg = va_arg(va, void *);
    ret = vfs_fcntl(fd, flag, arg);
    va_end(va);
#else
    errno = ENOTSUP;
#endif
    return ret;
}

