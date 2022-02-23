#include <sys/stat.h>
#include <extension/errno_ext.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int stat(const char *path, struct stat *sbuf)
{
#ifdef OS_USING_VFS
    return vfs_stat(path, sbuf);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
