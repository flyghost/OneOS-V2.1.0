#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int open(const char *path, int oflag, ...)
{
#ifdef OS_USING_VFS
    return vfs_open(path, oflag);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
