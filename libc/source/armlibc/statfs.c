#include <sys/vfs.h>
#include <extension/errno_ext.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int statfs(const char *path, struct statfs *buf)
{
#ifdef OS_USING_VFS
    return vfs_statfs(path, buf);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
