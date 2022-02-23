#include <sys/errno.h>
#include <dlog.h>
#include <os_util.h>
#include <os_memory.h>
#include <oneos_config.h>
#ifdef OS_USING_VFS
#include <vfs_posix.h>
#endif
#include <dirent.h>

#define DIRENT_TAG "DIRENT"

int closedir(DIR *pdir)
{
    int res = -1;

    if (!pdir) 
    {
        errno = EBADF;
        LOG_E(DIRENT_TAG, "Invalid directory stream descriptor\r\n");
    } 
    else
    {
    #ifdef OS_USING_VFS
        res = vfs_closedir(pdir->priv);
    #if defined (OS_USING_SYS_HEAP)
        os_free(pdir);
    #endif
    #endif
    }

    return res;
}

DIR *opendir(const char *path)
{
    DIR *pdir  = OS_NULL;

#ifdef OS_USING_VFS
    void *priv = OS_NULL;

    if (OS_NULL == path) 
    {
        errno = ENOENT;
        LOG_E(DIRENT_TAG, "Path is null\r\n");
    } 
    else 
    {
        if (OS_NULL != (priv = vfs_opendir(path)))
        {
        #if defined (OS_USING_SYS_HEAP)
            pdir = (DIR*)os_malloc(sizeof(DIR));
        #else
            pdir = OS_NULL;
        #endif
            if (OS_NULL != pdir)
            {
                pdir->priv = priv;
            }
        }
    }
#endif

    return pdir;
}

struct dirent *readdir(DIR *pdir)
{
    struct dirent *pdirent = OS_NULL;

    if (!pdir) 
    {
        errno = EBADF;
        LOG_E(DIRENT_TAG, "Invalid directory stream descriptor\r\n");
    }
    else 
    {
    #ifdef OS_USING_VFS
        pdirent = vfs_readdir(pdir->priv);
    #endif
    }

    return pdirent;
}

void rewinddir(DIR *pdir)
{
    if (!pdir) 
    {
        errno = EBADF;
        LOG_E(DIRENT_TAG, "Invalid directory stream descriptor\r\n");
    } 
    else 
    {
    #ifdef OS_USING_VFS
        vfs_rewinddir(pdir->priv);
    #endif
    }
}

void seekdir(DIR *pdir, long ofst)
{
    if (!pdir) 
    {
        errno = EBADF;
        LOG_E(DIRENT_TAG, "Invalid directory stream descriptor\r\n");
    } 
    else 
    {
    #ifdef OS_USING_VFS
        vfs_seekdir(pdir->priv, ofst);
    #endif
    }
}

long telldir(DIR *pdir)
{
    long res = 0;

    if (!pdir) 
    {
        errno = EBADF;
        LOG_E(DIRENT_TAG, "Invalid directory stream descriptor\r\n");
    }
    else 
    {
    #ifdef OS_USING_VFS
        res = vfs_telldir(pdir->priv);
    #endif
    }

    return res;
}

