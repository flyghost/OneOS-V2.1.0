#include <sys/types.h>
#include <extension/errno_ext.h>
#include <os_stddef.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

char* getcwd(char *buf, size_t size )
{
#ifdef OS_USING_VFS
    return vfs_getcwd(buf, size);
#else
    errno = ENOTSUP;
    return OS_NULL;
#endif
}
