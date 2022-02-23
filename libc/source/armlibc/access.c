#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int access(const char *pathname, int mode)
{
#ifdef OS_USING_VFS
    return vfs_access(pathname, mode);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
