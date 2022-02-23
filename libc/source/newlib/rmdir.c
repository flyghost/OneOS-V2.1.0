#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int rmdir(const char *pathname)
{
#ifdef OS_USING_VFS
    return vfs_rmdir(pathname);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
