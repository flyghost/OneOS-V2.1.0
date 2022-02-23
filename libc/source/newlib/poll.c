#include <oneos_config.h>
#include <poll.h>
#include <os_util.h>
#include <os_memory.h>
#if defined(OS_USING_VFS)
#include <vfs_poll.h>
#endif

int poll(struct pollfd fds[], nfds_t nfds, int timeout)
{
    struct vfs_pollfd *pfds = OS_NULL;
    int ret        = -1;
    unsigned int i = 0;

    if (OS_NULL == fds) 
    {
        os_kprintf("File Descriptor is NULL\r\n");
    } 
    else 
    {
    #if defined(OS_USING_VFS)

    #if defined(OS_USING_SYS_HEAP)
        pfds = (struct vfs_pollfd*)os_malloc(sizeof(struct vfs_pollfd) * nfds);
    #else
        pfds = OS_NULL;
    #endif
        if (OS_NULL == pfds) 
        {
            os_kprintf("Pollfd memory alloc failed\r\n");
        }
        else
        {
            for (i = 0; i < nfds; i++) 
            {
                pfds[i].fd      = fds[i].fd;
                pfds[i].events  = fds[i].events;
            }

            ret  = vfs_poll(pfds, nfds, timeout);

            for (i = 0; i < nfds; i++) 
            {
                fds[i].revents = pfds[i].revents;
            }
        #if defined(OS_USING_SYS_HEAP)
            os_free(pfds);
        #endif
        }
    #endif
    }

    return ret;
}
