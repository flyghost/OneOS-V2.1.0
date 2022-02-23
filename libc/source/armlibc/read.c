#include <sys/types.h>
#include <extension/errno_ext.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int read(int fd, void *buf, size_t nbyte)
{
#ifdef OS_USING_VFS
    return vfs_read(fd, buf, nbyte);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
