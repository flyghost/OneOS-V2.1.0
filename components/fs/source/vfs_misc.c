/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        vfs_misc.c
 *
 * @brief       This file implements the miscellaneous funciton for vfs.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_memory.h>
#include <os_mutex.h>
#include <os_assert.h>
#include <vfs.h>
#include <dlog.h>

#define VFS_CWD_TAG             "VFS_CWD_TAG"

static os_mutex_t vfs_cwd_lock;

#define VFS_WORKDIR_LOCK_INIT() do {                                                                \
                                    os_err_t cwdlock_ret = os_mutex_init(&vfs_cwd_lock, "vfscwdlock", OS_FALSE);\
                                    if (OS_EOK != cwdlock_ret) {                                    \
                                        LOG_E(VFS_CWD_TAG, "vfs_cwd_lock init faild:%d", cwdlock_ret);  \
                                    }                                                               \
                                    OS_ASSERT(OS_EOK == cwdlock_ret);                               \
                                }while(0)

#define VFS_WORKDIR_LOCK()      do {                                                                \
                                    os_err_t cwdlock_ret = os_mutex_lock(&vfs_cwd_lock, OS_WAIT_FOREVER);\
                                    if (OS_EOK != cwdlock_ret) {                                    \
                                        LOG_E(VFS_CWD_TAG, "vfs_cwd_lock lock faild:%d", cwdlock_ret);  \
                                    }                                                               \
                                    OS_ASSERT(OS_EOK == cwdlock_ret);                               \
                                }while(0)

#define VFS_WORKDIR_UNLOCK()    do {                                                                \
                                    os_err_t cwdlock_ret = os_mutex_unlock(&vfs_cwd_lock);          \
                                    if (OS_EOK != cwdlock_ret) {                                    \
                                        LOG_E(VFS_CWD_TAG, "vfs_cwd_lock unlock faild:%d", cwdlock_ret);\
                                    }                                                               \
                                    OS_ASSERT(OS_EOK == cwdlock_ret);                               \
                                }while(0)

static char working_dir[VFS_PATH_MAX] = {"/"};

void working_dir_init(void)
{
    VFS_WORKDIR_LOCK_INIT();
    memset(working_dir, 0, sizeof(working_dir));
    working_dir[0] = '/';
}

char *working_dir_get(char *buf, size_t size)
{
    VFS_WORKDIR_LOCK();
    strncpy(buf, &working_dir[0], size);
    VFS_WORKDIR_UNLOCK();

    return buf;
}

void working_dir_set(char *path)
{
    VFS_WORKDIR_LOCK();
    strncpy(working_dir, path, VFS_PATH_MAX);
    VFS_WORKDIR_UNLOCK();
}

static char *_vfs_conjunction_path(const char *dirpath, const char *filepath)
{
    char *fullpath;
    char *fulldir;
    os_bool_t   dir_path_alloc;
    os_uint32_t length;

    fullpath = OS_NULL;
    fulldir = OS_NULL;
    dir_path_alloc = OS_FALSE;

    if (filepath[0] == '/') /* file is an absolute path, use it directly */
    {
        length = strlen(filepath) + 1;
        fullpath = os_malloc(length);
        if (fullpath)
        {
            strncpy(fullpath, filepath, length);
        }
    }
    else                    /* file is not an absolute path, add it follow direcotry. */
    {
        VFS_WORKDIR_LOCK();

        if (!dirpath)
        {
            fulldir = working_dir;
        }
        else if (dirpath[0] == '/')
        {
            fulldir = (char *)dirpath;
        }
        else
        {
            /* Convert to absolute dir. */
            length = strlen(working_dir) + strlen(dirpath) + 2;
            fulldir = os_malloc(length);
            if (fulldir)
            {
                dir_path_alloc = OS_TRUE;
                os_snprintf(fulldir, length, "%s/%s", working_dir, dirpath);
            }
        }

        if (fulldir)
        {
            length = strlen(fulldir) + strlen(filepath) + 2;
            fullpath = os_malloc(length);
            if (fullpath) /* Join path and file name */
            {
                os_snprintf(fullpath, length, "%s/%s", fulldir, filepath);
            }

            if (OS_TRUE == dir_path_alloc)
            {
                os_free(fulldir);
            }
        }

        VFS_WORKDIR_UNLOCK();
    }

     return fullpath;
}

static char *_vfs_normalize_path(char *fullpath)
{
    char *dst0;
    char *dst;
    char *src;
    os_bool_t up_one_flag;
    os_bool_t fail_flag;

    src = fullpath;
    dst = fullpath;
    dst0 = fullpath;
    while (1)
    {
        char c;

        c = *src;
        up_one_flag = OS_FALSE;
        fail_flag = OS_FALSE;

        if (c == '.')
        {
            if (!src[1])
            {
                /* '.' and ends */
                src ++;
            }
            else if (src[1] == '/')
            {
                /* './' case */
                src += 2;

                while ((*src == '/') && (*(src+1) != '\0'))
                {
                    src ++;
                }
                continue;
            }
            else if (src[1] == '.')
            {
                if (!src[2])
                {
                    /* '..' and ends case */
                    src += 2;
                    up_one_flag = OS_TRUE;
                }
                else if (src[2] == '/')
                {
                    /* '../' case */
                    src += 3;

                    while ((*src == '/') && (*(src+1) != '\0'))
                    {
                        src ++;
                    }
                    up_one_flag = OS_TRUE;
                }
                else
                {
                    /* do nothing. */
                }
            }
            else
            {
                /* do nothing. */
            }
        }

        if (OS_FALSE == up_one_flag)
        {
            /* Copy up the next '/' and erase all '/' */
            while ((c = *src++) != '\0' && c != '/')
            {
                *dst ++ = c;
            }

            if (c == '/')
            {
                *dst ++ = '/';
                while (c == '/')
                {
                    c = *src++;
                }

                src --;
            }
            else if (!c)
            {
                break;
            }
            else
            {
                /* do nothing. */
            }
        }
        else
        {
            dst --;
            if (dst < dst0)
            {
                fail_flag = OS_TRUE;
                fullpath = OS_NULL;
                break;
            }
            while (dst0 < dst && dst[-1] != '/')
            {
                dst --;
            }
        }
    }

    if (OS_FALSE == fail_flag)
    {
        *dst = '\0';

        /* Remove '/' in the end of path if exist */
        dst --;
        if ((dst != fullpath) && (*dst == '/'))
        {
            *dst = '\0';
        }

        /* Final check fullpath is not empty, for the special path of lwext "/.." */
        if ('\0' == fullpath[0])
        {
            fullpath[0] = '/';
            fullpath[1] = '\0';
        }
    }

    return fullpath;
}

const char *vfs_create_absolute_path(const char *dirpath, const char *filepath)
{
    char *fullpath;
    char *normalizepath;

    normalizepath = OS_NULL;

    if (filepath)
    {
        fullpath = _vfs_conjunction_path(dirpath, filepath);
        if (fullpath)
        {
            normalizepath = _vfs_normalize_path(fullpath);
            if (!normalizepath)
            {
                os_free(fullpath);
            }
        }
    }

    return normalizepath;
}

void vfs_destroy_absolute_path(char *path)
{
    os_free((void *)path);
}

const char *vfs_get_rel_mnt_path(const char *mntpath, const char *abspath)
{
    const char *path;

    if (strlen(mntpath) == strlen(abspath))
    {
        path = "/";
    }
    else
    {
        path = abspath + strlen(mntpath);
        if ((*path != '/') && (path != abspath))
        {
            path --;
        }
    }

    return path;
}

const char *vfs_create_rel_mnt_path(const char *mntpath, const char *abspath)
{
    const char *path;
    char *path_new;

    path = vfs_get_rel_mnt_path(mntpath, abspath);
    /* path_new = strdup(path); */
    path_new = (char *)os_malloc(strlen(path) + 1);
    if (path_new)
    {
        strcpy(path_new, path);
    }

    return path_new;
}

const char *vfs_get_path_lastname(const char *path)
{
    const char *ptr;

    ptr = strrchr(path, '/');
    if (ptr)
    {
        /* found '/', Skip character '/'. */
        ptr++;
    }
    else
    {
        /* not found '/', use path directly. */
        ptr = path;
    }

    return ptr;
}

