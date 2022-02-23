#include <sys/types.h>
#include <extension/errno_ext.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int mkdir(const char *path, mode_t mode)
{
#ifdef OS_USING_VFS
    return vfs_mkdir(path, mode);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
