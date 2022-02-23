#include <_ansi.h>
#include <sys/types.h>
#include <reent.h>
#include <extension/errno_ext.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

#if !defined(HAVE_FCNTL)
extern int _fcntl_r(struct _reent *ptr, int fd, int cmd, int arg);

int fcntl(int fd, int cmd, ...)
{
    int ret = -1;
#ifdef OS_USING_VFS
    va_list va;
    int arg;

    va_start(va, cmd);
    arg = va_arg(va, int);
    ret = _fcntl_r(_REENT, fd, cmd, arg);
    va_end(va);
#else
    errno = ENOTSUP;
#endif

    return ret;
}
#endif
