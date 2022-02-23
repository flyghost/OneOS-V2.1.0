#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int chdir(const char *path)
{
#ifdef OS_USING_VFS
    return vfs_chdir(path);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
