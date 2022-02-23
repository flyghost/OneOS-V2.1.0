#include <oneos_config.h>
#include <extension/errno_ext.h>
#include <sys/stat.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int fstat(int fd, struct stat *sbuf)
{
#ifdef OS_USING_VFS
    return vfs_fstat(fd, sbuf);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
