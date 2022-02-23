#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int rename(const char *old, const char *new)
{
#ifdef OS_USING_VFS
    return vfs_rename(old, new);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
