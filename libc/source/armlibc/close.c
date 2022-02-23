#include <oneos_config.h>
#include <extension/errno_ext.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

int close(int fildes)
{
#ifdef OS_USING_VFS
    return vfs_close(fildes);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
