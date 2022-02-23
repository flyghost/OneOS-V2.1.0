#include <sys/types.h>
#include <extension/errno_ext.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

off_t lseek(int fildes, off_t offset, int whence)
{
#ifdef OS_USING_VFS
    return vfs_lseek(fildes, offset, whence);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
