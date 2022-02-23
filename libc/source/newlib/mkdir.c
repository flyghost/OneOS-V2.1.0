#include <_ansi.h>
#include <sys/types.h>
#include <reent.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include "vfs_posix.h"
#endif

extern int _mkdir_r(struct _reent *ptr, const char *name, int mode);

int mkdir(const char *path, mode_t mode)
{
    return _mkdir_r(_REENT, path, mode);
}
