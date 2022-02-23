#include <oneos_config.h>
#include <sys/time.h>
#include <sys/select.h>
#include <extension/errno_ext.h>
#include <extension/signal_ext.h>
#if defined(OS_USING_VFS)
#include <vfs_select.h>
#endif

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, 
        struct timeval *timeout)
{
#if defined(OS_USING_VFS)
    return vfs_select(nfds, readfds, writefds, exceptfds, timeout);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
