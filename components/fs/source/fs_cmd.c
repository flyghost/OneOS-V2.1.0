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
 * @file        fs_cmd.c
 *
 * @brief       This file implements the shell cmd for file or direcotry operation.
 *
 * @revision
 * Date         Author          Notes
 * 2021-02-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_util.h>
#include <os_memory.h>
#include <shell.h>
#include <fcntl.h>
#include <sys/vfs.h>
#include <vfs_fs.h>
#include <vfs_posix.h>
#include "vfs_private.h"

#define CP_BUF_SIZE         2048
#define CAT_BUF_SIZE        81

struct path_node
{
    os_list_node_t list_node;
    char *src;
    char *dst;
};

struct rm_node
{
    os_list_node_t list_node;
    char *path;
};

static void _cat(const char *path)
{
    char buf[CAT_BUF_SIZE];
    int  fd;
    int  size;

    fd = vfs_open(path, O_RDONLY);
    if (fd >= 0)
    {
        do
        {
            memset(buf, 0, sizeof(buf));
            size = vfs_read(fd, buf, CAT_BUF_SIZE - 1);
            if (size > 0)
            {
                os_kprintf("%s", buf);
            }
            else if (size < 0)
            {
                os_kprintf("read file %s failed\r\n", path);
            }
            else
            {
                /* Do nothing.*/
            }
        }
        while (size > 0);
        os_kprintf("\r\n");

        vfs_close(fd);
    }
    else
    {
        os_kprintf("Open file %s failed\r\n", path);
    }
}

static void _echo(char *path, int flag, char *buf)
{
    int fd;
    int result;
    size_t len;

    fd = vfs_open(path, flag, 0);
    if (fd >= 0)
    {
        len = strlen(buf);
        result = vfs_write(fd, buf, len);
        if ((result < 0) && (len > 0))
        {
            os_kprintf("write file %s failed\r\n", path);
        }

        vfs_close(fd);
    }
    else
    {
        os_kprintf("open file %s failed\r\n", path);
    }
}

static void _ls(const char *path)
{
    FS_DIR *dp;
    char   *abs_path;
    struct dirent *dent;
    struct stat    entry_stat;

    dp = vfs_opendir(path);
    if (dp)
    {
        os_kprintf("Directory %s:\r\n", path);
        do
        {
            dent = vfs_readdir(dp);
            if (!dent)
            {
                break;
            }

            abs_path = vfs_create_absolute_path(path, dent->d_name);
            if (!abs_path)
            {
                break;
            }

            memset(&entry_stat, 0, sizeof(struct stat));
            if (0 == vfs_stat(abs_path, &entry_stat))
            {
                os_kprintf("%-20s    ", dent->d_name);
                if (S_ISDIR(entry_stat.st_mode))
                {
                    os_kprintf("%-25s\r\n", "<DIR>");
                }
                else
                {
                    os_kprintf("%-25lu\r\n", entry_stat.st_size);
                }
            }
            else
            {
                os_kprintf("BAD file: %s\r\n", dent->d_name);
            }
            vfs_destroy_absolute_path(abs_path);
        }
        while (dent);

        vfs_closedir(dp);
    }
    else
    {
        os_kprintf("No such directory\r\n");
    }
}

static struct rm_node *_rm_node_create(char *path)
{
    struct rm_node *rm_node;

    rm_node = os_malloc(sizeof(struct rm_node));
    if (OS_NULL != rm_node)
    {
        os_list_init(&rm_node->list_node);
        rm_node->path = path;
    }
    else
    {
        os_kprintf("failed create node[%s], out of memory\r\n", path);
    }

    return rm_node;
}

static void _rm_node_destroy(struct rm_node *rm_node)
{
    rm_node->path = OS_NULL;
    os_free(rm_node);

    return;
}

static void _rm_node_push(os_list_node_t *stack, struct rm_node *rm_node)
{
    os_list_add_tail(stack, &rm_node->list_node);
    //os_kprintf("push:[%s]\r\n", rm_node->path);

    return;
}

static struct rm_node *_rm_node_pop(os_list_node_t *stack)
{
    struct rm_node *rm_node;

    rm_node = os_list_tail_entry_or_null(stack, struct rm_node, list_node);
    if (rm_node)
    {
        os_list_del_init(&rm_node->list_node);
        //os_kprintf("pop :[%s]\r\n", rm_node->path);
    }

    return rm_node;
}

static void _removedir(const char *target)
{
    os_list_node_t stack_head;
    struct rm_node *push_node;
    struct rm_node *node;
    struct dirent *dent;
    struct stat stat;
    os_bool_t rm_ex;
    char *abspath;
    char *do_dir;
    int do_err;
    FS_DIR *dp;

    abspath = vfs_create_absolute_path(OS_NULL, target);
    node = _rm_node_create(abspath);
    if (OS_NULL == node)
    {
        os_free(abspath);
        return;
    }

    os_list_init(&stack_head);
    _rm_node_push(&stack_head, node);

    rm_ex = OS_FALSE;
    do
    {
        node = _rm_node_pop(&stack_head);
        if (OS_NULL == node)
        {
            break;
        }

        dp = vfs_opendir(node->path);
        if (OS_NULL == dp)
        {
            os_kprintf("open dir[%s] failed\r\n", node->path);
            os_free(node->path);
            _rm_node_destroy(node);
            node = OS_NULL;

            rm_ex = OS_TRUE;
            break;
        }

        do_dir = node->path;
        do
        {
            dent = vfs_readdir(dp);
            if (!dent)
            {
                break;
            }

            if ((0 == strcmp(dent->d_name, "..")) || (0 == strcmp(dent->d_name, ".")))
            {
                continue;
            }

            abspath = vfs_create_absolute_path(do_dir, dent->d_name);
            if (!abspath)
            {
                os_kprintf("create abspath for [%s] out of memory\r\n", dent->d_name);
                rm_ex = OS_TRUE;
                break;
            }

            memset(&stat, 0, sizeof(struct stat));
            do_err = vfs_stat(abspath, &stat);
            if (0 != do_err)
            {
                os_kprintf("stat [%s] failed\r\n", abspath);
                os_free(abspath);
                rm_ex = OS_TRUE;
                break;
            }

            if (S_ISDIR(stat.st_mode))
            {
                if (OS_NULL != node)
                {
                    _rm_node_push(&stack_head, node);
                    node = OS_NULL;
                }

                push_node = _rm_node_create(abspath);
                if (OS_NULL == push_node)
                {
                    os_free(abspath);
                    rm_ex = OS_TRUE;
                    break;
                }
                _rm_node_push(&stack_head, push_node);
            }
            else
            {
                do_err = vfs_unlink(abspath);
                os_free(abspath);
                if (do_err < 0)
                {
                    os_kprintf("rm [%s] failed\r\n", abspath);
                    rm_ex = OS_TRUE;
                    break;
                }
            }
        } while(1);

        vfs_closedir(dp);
        dp = OS_NULL;

        if (OS_NULL != node && OS_FALSE == rm_ex)
        {
            do_err = vfs_unlink(node->path);
            if (do_err < 0)
            {
                os_kprintf("rm dir [%s] failed\r\n", node->path);
                rm_ex = OS_TRUE;
            }
        }

        if (OS_NULL != node)
        {
            os_free(node->path);
            _rm_node_destroy(node);
            node = OS_NULL;
        }

        if (OS_TRUE == rm_ex)
        {
            break;
        }
    } while (1);

    while (OS_TRUE == rm_ex)
    {
        node = _rm_node_pop(&stack_head);
        if (OS_NULL == node)
        {
            os_kprintf("do cmd rm [%s] failed\r\n", target);
            break;
        }
        os_free(node->path);
        _rm_node_destroy(node);
    }

    return;
}

static void _remove(const char *target)
{
    struct stat stat;
    int do_err;

    memset(&stat, 0, sizeof(struct stat));
    do_err = vfs_stat(target, &stat);
    if (do_err < 0)
    {
        os_kprintf("rm [%s] is not exist\r\n", target);
        return;
    }

    if (S_ISDIR(stat.st_mode))
    {
        _removedir(target);
    }
    else
    {
        do_err = vfs_unlink(target);
        if (do_err < 0)
        {
            os_kprintf("rm %s failed\r\n", target);
        }
    }

    return;
}

static void _copyfile(const char *src, const char *dst)
{
    int read_size;
    int write_size;
    int fd_src;
    int fd_dst;
    int result;
    char *buf;
    char *src_abspath;
    char *dst_abspath;

    result = 0;
    fd_src = -1;
    fd_dst = -1;
    src_abspath = vfs_create_absolute_path(OS_NULL, src);
    if (src_abspath)
    {
        dst_abspath = vfs_create_absolute_path(OS_NULL, dst);
        if (dst_abspath)
        {
            if (0 == strcmp(src_abspath, dst_abspath))
            {
                os_kprintf("cp failed, %s and %s are the same file\r\n", src, dst);
                result = -1;
            }
            vfs_destroy_absolute_path(dst_abspath);
            vfs_destroy_absolute_path(src_abspath);
        }
        else
        {
            os_kprintf("out of memory\r\n");
            vfs_destroy_absolute_path(src_abspath);
            result = -1;
        }
    }
    else
    {
        os_kprintf("out of memory\r\n");
        result = -1;
    }

    if (0 == result)
    {
        fd_src = vfs_open(src, O_RDONLY);
        if (fd_src >= 0)
        {
            /* If dst file exist, O_TRUNC will clear it first. */
            fd_dst = vfs_open(dst, O_WRONLY | O_CREAT | O_TRUNC);
            if (fd_dst >= 0)
            {
                buf = os_malloc(CP_BUF_SIZE);
                if (!buf)
                {
                    vfs_close(fd_src);
                    vfs_close(fd_dst);
                    result = -1;
                    os_kprintf("out of memory\r\n");
                }
            }
            else
            {
                vfs_close(fd_src);
                result = -1;
                os_kprintf("open %s failed\r\n", dst);
            }
        }
        else
        {
            result = -1;
            os_kprintf("open %s failed\r\n", src);
        }
    }

    if (0 == result)
    {
        do
        {
            read_size = vfs_read(fd_src, buf, CP_BUF_SIZE);
            if (read_size <= 0)
            {
                break;
            }

            write_size = vfs_write(fd_dst, buf, read_size);
            if (write_size != read_size)
            {
                if (write_size < 0)
                {
                    os_kprintf("Write file data failed\r\n");
                }
                else
                {
                    os_kprintf("Write file data failed, not copy all the data\r\n");
                }
                break;
            }
        }
        while (read_size > 0);

        vfs_close(fd_src);
        vfs_close(fd_dst);
        os_free(buf);
    }
}

static struct path_node *_path_node_create(char *src, char *dst)
{
    int result;
    struct path_node *path_nd;

    path_nd = OS_NULL;

    result = vfs_mkdir(dst, 0);
    if (result >= 0)
    {
        path_nd = os_malloc(sizeof(struct path_node));
        if (path_nd)
        {
            path_nd->src = src;
            path_nd->dst = dst;
        }
        else
        {
            os_kprintf("out of memory\r\n");
        }
    }
    else
    {
        os_kprintf("mkdir %s failed\r\n", dst);
    }

    return path_nd;
}

static void _path_node_destroy(struct path_node *path_nd)
{
    vfs_destroy_absolute_path(path_nd->src);
    vfs_destroy_absolute_path(path_nd->dst);
    os_free(path_nd);
}

static void _path_node_push(os_list_node_t *stack, struct path_node *path_nd)
{
    os_list_add_tail(stack, &path_nd->list_node);
}

static struct path_node *_path_node_pop(os_list_node_t *stack)
{
    struct path_node *path_nd;

    path_nd = os_list_tail_entry_or_null(stack, struct path_node, list_node);
    if (path_nd)
    {
        os_list_del(&path_nd->list_node);
    }

    return path_nd;
}

static void _copydir(const char *src, const char *dst)
{
    struct stat stat_buf;
    FS_DIR *dp;
    struct dirent *dent;
    char *src_abspath;
    char *dst_abspath;
    int result;
    os_list_node_t stack_head;
    struct path_node *push_node;
    struct path_node *pop_node;

    os_list_init(&stack_head);

    dp = vfs_opendir(src);
    if (dp)
    {
        result = vfs_mkdir(dst, 0);
        if (result < 0)
        {
            os_kprintf("mkdir %s failed\r\n", dst);
            vfs_closedir(dp);
            dp = OS_NULL;
        }
    }
    else
    {
        os_kprintf("open %s failed\r\n", src);
    }

    pop_node = OS_NULL;

    while(dp)
    {
        do
        {
            dent = vfs_readdir(dp);
            if (!dent)
            {
                break;
            }

            if ((0 == strcmp(dent->d_name, "..")) || (0 == strcmp(dent->d_name, ".")))
            {
                continue;
            }

            src_abspath = vfs_create_absolute_path(pop_node ? pop_node->src : src, dent->d_name);
            if (!src_abspath)
            {
                os_kprintf("out of memory\r\n");
                break;
            }
            dst_abspath = vfs_create_absolute_path(pop_node ? pop_node->dst : dst, dent->d_name);
            if (!dst_abspath)
            {
                vfs_destroy_absolute_path(src_abspath);
                os_kprintf("out of memory\r\n");
                break;
            }

            memset(&stat_buf, 0, sizeof(struct stat));
            if (0 != vfs_stat(src_abspath, &stat_buf))
            {
                vfs_destroy_absolute_path(src_abspath);
                vfs_destroy_absolute_path(dst_abspath);
                os_kprintf("stat %s failed\r\n", dent->d_name);
                continue;
            }

            if (S_ISDIR(stat_buf.st_mode))
            {
                /* If directory, not copy now, push it.*/
                push_node = _path_node_create(src_abspath, dst_abspath);

                if (push_node)
                {
                    _path_node_push(&stack_head, push_node);
                }
                else
                {
                    vfs_destroy_absolute_path(src_abspath);
                    vfs_destroy_absolute_path(dst_abspath);
                    break;
                }
            }
            else
            {
                /* If file, copy it directly. */
                _copyfile(src_abspath, dst_abspath);
                vfs_destroy_absolute_path(src_abspath);
                vfs_destroy_absolute_path(dst_abspath);
            }
        }
        while (dent);

        /* Finish this round, close this direcotry. */
        vfs_closedir(dp);
        dp = OS_NULL;

        if (pop_node)
        {
            _path_node_destroy(pop_node);
        }

        /* Start next round, pop next directory and open.*/
        pop_node = _path_node_pop(&stack_head);
        if (pop_node)
        {
            dp = vfs_opendir(pop_node->src);
        }
    }
}

static void _copy(const char *src, const char *dst)
{
    struct stat stat_src;
    struct stat stat_dst;
    os_bool_t   dst_exist;
    char *file_dst;
    char *dir_dst;
    int   result_src;
    int   result_dst;

    dst_exist = OS_FALSE;
    memset(&stat_src, 0, sizeof(struct stat));
    memset(&stat_dst, 0, sizeof(struct stat));

    result_src = vfs_stat(src, &stat_src);
    if (result_src >= 0)
    {
        result_dst = vfs_stat(dst, &stat_dst);
        if (result_dst >= 0)
        {
            dst_exist = OS_TRUE;
        }
    }
    else
    {
        os_kprintf("copy failed, bad %s\r\n", src);
    }

    if (result_src >= 0)
    {
        if (S_ISREG(stat_src.st_mode))
        {
            if (S_ISREG(stat_dst.st_mode))
            {
                /* If dst is file, truncate it and copy. */
                _copyfile(src, dst);
            }
            else if (S_ISDIR(stat_dst.st_mode))
            {
                /* If dst is directory, copy src file under dst directory. */
                file_dst = vfs_create_absolute_path(dst, vfs_get_path_lastname(src));
                if (file_dst)
                {
                    _copyfile(src, file_dst);
                    vfs_destroy_absolute_path(file_dst);
                }
                else
                {
                    os_kprintf("out of memory\r\n");
                }
            }
            else
            {
                /* If dst not exist, create it and copy. */
                if (OS_FALSE == dst_exist)
                {
                    _copyfile(src, dst);
                }
                else
                {
                    /* Dst exist, but type is invaild, do nothing. */
                    os_kprintf("cp faild, %s is not regular file or directory.\r\n", dst);
                }
            }
        }
        else if (S_ISDIR(stat_src.st_mode))
        {
            if (S_ISREG(stat_dst.st_mode))
            {
                /* If dst is file, not permit copy dir to file. */
                os_kprintf("cp faild, cp dir to file is not permitted\r\n");
            }
            else if (S_ISDIR(stat_dst.st_mode))
            {
                /* If dst is directory, copy src directory under dst directory. */
                dir_dst = vfs_create_absolute_path(dst, vfs_get_path_lastname(src));
                if (dir_dst)
                {
                    _copydir(src, dir_dst);
                    vfs_destroy_absolute_path(dir_dst);
                }
                else
                {
                    os_kprintf("out of memory\r\n");
                }
            }
            else
            {
                /* If dst not exist, create dst directory, then copy directory. */
                if (OS_FALSE == dst_exist)
                {
                    _copydir(src, dst);
                }
                else
                {
                    os_kprintf("cp faild, %s is not regular file or directory.\r\n", dst);
                }
            }
        }
        else
        {
            /* Src type is invaild, do nothing. */
            os_kprintf("cp faild, %s is not regular file or directory.\r\n", src);
        }
    }
}

static void _move(const char *src, const char *dst)
{
    struct stat stat_src;
    struct stat stat_dst;
    os_bool_t   dst_exist;
    char *dst_tmp;
    char *src_tmp;
    int   result_src;
    int   result_dst;
    int   result_rename;

    dst_exist = OS_FALSE;
    result_rename = 0;
    memset(&stat_src, 0, sizeof(struct stat));
    memset(&stat_dst, 0, sizeof(struct stat));

    result_src = vfs_stat(src, &stat_src);
    if (result_src >= 0)
    {
        result_dst = vfs_stat(dst, &stat_dst);
        if (result_dst >= 0)
        {
            dst_exist = OS_TRUE;
        }
    }
    else
    {
        os_kprintf("mv failed, bad source: %s\r\n", src);
    }

    if (result_src >= 0)
    {
        os_kprintf("%s => %s\r\n", src, dst);

        if (S_ISDIR(stat_dst.st_mode))
        {
            /* If dst is directory, move src file/directory under dst directory. */
            if (S_ISREG(stat_src.st_mode) || S_ISDIR(stat_src.st_mode))
            {
                dst_tmp = (char *)os_malloc(VFS_PATH_MAX);
                if (dst_tmp)
                {
                    src_tmp = vfs_get_path_lastname(src);
                    os_snprintf(dst_tmp, VFS_PATH_MAX - 1, "%s/%s", dst, src_tmp);
                    result_rename = vfs_rename(src, dst_tmp);
                    os_free(dst_tmp);
                }
                else
                {
                    os_kprintf("out of memory\r\n");
                }
            }
            else
            {
                os_kprintf("mv failed, %s is not regular file or directory.\r\n", src);
            }
        }
        else if (S_ISREG(stat_dst.st_mode))
        {
            /* If dst is file, delete it first. */
            if (S_ISREG(stat_src.st_mode))
            {
                vfs_unlink(dst);
                result_rename = vfs_rename(src, dst);
            }
            else if (S_ISDIR(stat_src.st_mode))
            {
                os_kprintf("mv failed, mv a dir to a file is not permitted\r\n");
            }
            else
            {
                os_kprintf("mv failed, %s is not regular file or directory.\r\n", src);
            }
        }
        else
        {
            /* If dst not exist, rename it directly. */
            if (OS_FALSE == dst_exist)
            {
                result_rename = vfs_rename(src, dst);
            }
            else
            {
                os_kprintf("mv failed, %s is not regular file or directory.\r\n", dst);
            }
        }
    }

    if (result_rename < 0)
    {
        os_kprintf("mv failed, can not rename.\r\n");
    }
}

static void _df(const char *path)
{
    int result;
    int minor;
    int unit_index;
    long long     cap;
    struct statfs buffer;
    char *unit_str[] = {"KB", "MB", "GB"};

    minor = 0;

    if(path)
    {
        result = vfs_statfs(path, &buffer);
        if (0 == result)
        {
            cap = ((long long)buffer.f_bsize) * ((long long)buffer.f_bfree) / 1024LL;
            for (unit_index = 0; unit_index < 2; unit_index ++)
            {
                if (cap < 1024) 
                {
                    break;
                }

                minor = (cap % 1024) * 10 / 1024; /* Only one decimal point */
                cap = cap / 1024;
            }

            os_kprintf("disk free: %d.%d %s [ %d block, %d bytes per block ]\r\n",
                       (unsigned long)cap, 
                       minor, 
                       unit_str[unit_index], 
                       buffer.f_bfree, 
                       buffer.f_bsize);
        }
        else
        {
            os_kprintf("statfs failed\r\n");
        }
    }
    else
    {
        os_kprintf("invalid path\r\n");
    }
}

static os_err_t sh_cat(os_int32_t argc, char **argv)
{
    int i;
    os_err_t result;

    if (argc > 1)
    {
        for (i = 1; i < argc; i ++)
        {
            _cat(argv[i]);
        }

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: cat <FILE>...\r\n");
        os_kprintf("Concatenate file(s)\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(cat, sh_cat, "Concatenate FILE(s)");

static os_err_t sh_echo(os_int32_t argc, char **argv)
{
    os_err_t result;

    if (4 == argc)
    {
        if (strcmp(argv[2], ">") == 0)
        {
            _echo(argv[3], O_RDWR | O_CREAT | O_TRUNC, argv[1]);

            result = OS_EOK;
        }
        else if (strcmp(argv[2], ">>") == 0)
        {
            _echo(argv[3], O_RDWR | O_CREAT | O_APPEND, argv[1]);

            result = OS_EOK;
        }
        else
        {
            result = OS_EINVAL;
        }
    }
    else
    {
        result = OS_EINVAL;
    }

    if (result != OS_EOK)
    {
        os_kprintf("Usage: echo <STRING> > <FILE>\r\n");
        os_kprintf("Usage: echo <STRING> >> <FILE>\r\n");
    }

    return result;
}
SH_CMD_EXPORT(echo, sh_echo, "echo string to file");

static os_err_t sh_ls(os_int32_t argc, char **argv)
{
    char *cwd;
    os_err_t result;

    if (1 == argc)
    {
        cwd = os_malloc(VFS_PATH_MAX);
        if (cwd)
        {
            vfs_getcwd(cwd, VFS_PATH_MAX);
            _ls(cwd);
            os_free(cwd);
        }
        else
        {
            os_kprintf("out of memory\r\n");
        }

        result = OS_EOK;
    }
    else if (2 == argc)
    {
        _ls(argv[1]);

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: ls [DIRNAME]\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(ls, sh_ls, "List information about the FILEs.");

static os_err_t sh_cd(os_int32_t argc, char **argv)
{
    int ret;
    os_err_t result;

    if (1 == argc)
    {
        ret = vfs_chdir("/");
        if (0 != ret)
        {
            os_kprintf("No such directory: %s\r\n", "/");
        }

        result = OS_EOK;
    }
    else if (2 == argc)
    {
        ret = vfs_chdir(argv[1]);
        if (0 != ret)
        {
            os_kprintf("No such directory: %s\r\n", argv[1]);
        }

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: cd [PATH]\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(cd, sh_cd, "Change the shell working directory.");

static os_err_t sh_pwd(os_int32_t argc, char **argv)
{
    char *buf;
    os_err_t result;

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    buf = os_malloc(VFS_PATH_MAX);
    if (buf)
    {
        vfs_getcwd(buf, VFS_PATH_MAX);
        os_kprintf("%s\r\n", buf);
        os_free(buf);

        result = OS_EOK;
    }
    else
    {
        os_kprintf("out of memory\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(pwd, sh_pwd, "Print the name of the current working directory.");

static os_err_t sh_mkdir(os_int32_t argc, char **argv)
{
    int ret;
    os_err_t result;

    if (2 == argc)
    {
        ret = vfs_mkdir(argv[1], 0);
        if (ret < 0)
        {
            os_kprintf("mkdir %s failed\r\n", argv[1]);
        }

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: mkdir <DIRNAME>\r\n");
        os_kprintf("Create the DIRECTORY, if it not exist.\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(mkdir, sh_mkdir, "Create the DIRECTORY.");

static os_err_t sh_rm(os_int32_t argc, char **argv)
{
    int result;
    int i;

    if (argc > 1)
    {
        for (i = 1; i < argc; i ++)
        {
            _remove(argv[i]);
        }

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: rm <FILE>...\r\n");
        os_kprintf("Remove (unlink) the FILE(s).\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(rm, sh_rm, "Remove(unlink) the FILE(s).");

static os_err_t sh_cp(os_int32_t argc, char **argv)
{
    os_err_t result;

    if (3 == argc)
    {
        _copy(argv[1], argv[2]);

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: cp <SOURCE> <DEST> \r\n");
        os_kprintf("Copy SOURCE to DEST.\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(cp, sh_cp, "Copy SOURCE to DEST.");

static os_err_t sh_mv(os_int32_t argc, char **argv)
{
    os_err_t result;

    if (3 == argc)
    {
        _move(argv[1], argv[2]);

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: mv <SOURCE> <DEST> \r\n");
        os_kprintf("Rename SOURCE to DEST, or move SOURCE(s) to DIRECTORY.\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(mv, sh_mv, "Rename SOURCE to DEST.");

static os_err_t sh_df(os_int32_t argc, char **argv)
{
    os_err_t result;

    if (1 == argc)
    {
        _df("/");

        result = OS_EOK;
    }
    else if (2 == argc)
    {
        _df(argv[1]);

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: df [PATH]\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(df, sh_df, "disk free");

static os_err_t sh_mkfs(os_int32_t argc, char **argv)
{
    int ret;
    os_err_t result;

    if ((4 == argc) && (0 == strcmp(argv[1], "-t")))
    {
        ret = vfs_mkfs(argv[2], argv[3]);
        if (ret != 0)
        {
            os_kprintf("mkfs failed, ret=%d\r\n", ret);
        }

        result = OS_EOK;
    }
    else
    {
        os_kprintf("Usage: mkfs -t <FS_TYPE> <DEVICENAME>\r\n");

        result = OS_EINVAL;
    }

    return result;
}
SH_CMD_EXPORT(mkfs, sh_mkfs, "format disk with file system");

