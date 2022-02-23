#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int fsync(int fd)
{
#ifdef OS_USING_VFS
    return vfs_fsync(fd);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
