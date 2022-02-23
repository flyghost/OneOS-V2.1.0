#include <oneos_config.h>
#include <os_stddef.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int ioctl(int fd, unsigned long request, ...)
{
    int ret = -1;
#ifdef OS_USING_VFS
    va_list va;
    void *arg = OS_NULL;

    va_start(va, request);
    arg = va_arg(va, void *);
    ret = vfs_ioctl(fd, request, arg);
    va_end(va);
#else
    errno = ENOTSUP;
#endif
    return ret;
}

