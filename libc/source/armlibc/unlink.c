#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int unlink(const char *path)
{
#ifdef OS_USING_VFS
    return vfs_unlink(path);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
