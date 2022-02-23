/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 * COPYRIGHT (C) 2006 - 2020,RT-Thread Development Team
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        nfs.c
 *
 * @brief       This file implement the nfs fuction.
 *
 * @revision
 * Date         Author          Notes
 * 2020-08-12   OneOS Team      Rafactor nfs code
 ***********************************************************************************************************************
 */

#include <stdio.h>
#include <vfs_fs.h>
#include <vfs.h>
#include <rpc/rpc.h>
#include <sys/fcntl.h>
#include <nfs.h>

#define NFS_RENAME_NO_ACTION    1

static int nfs_parse_host_export(const char *host_export,
                                 char       *host,
                                 size_t      host_len,
                                 char       *export,
                                 size_t      export_len)
{
    int index;

    for (index = 0; index < host_len; index ++)
    {
        /* it's end of string, failed */
        if (host_export[index] == 0)
            return -1;

        /* copy to host buffer */
        if (host_export[index] != ':')
            host[index] = host_export[index];
        else
            break;
    }

    /* host buffer is not enough, failed */
    if (index == host_len)
        return -1;

    /* make OS_NULL */
    host_len = index;
    host[host_len] = '\0';

    host_len ++;

    /* copy export path */
    for (index = host_len; index < host_len + export_len; index ++)
    {
        if (host_export[index] == 0)
        {
            export[index - host_len] = '\0';

            return 0;
        }

        export[index - host_len] = host_export[index];
    }

    return -1;
}

static void copy_handle(nfs_fh3 *dest, const nfs_fh3 *source)
{
    dest->data.data_len = source->data.data_len;
    dest->data.data_val = os_malloc(dest->data.data_len);

    if (!dest->data.data_val)
    {
        dest->data.data_len = 0;

        return;
    }

    memcpy(dest->data.data_val, source->data.data_val, dest->data.data_len);
}

static void nfs_free_handle(struct nfs_fh3 *handle)
{
    xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)handle);
    os_free(handle);
}

static nfs_fh3 *nfs_get_handle(struct nfs_filesystem *nfs, const char *name)
{
    nfs_fh3 *handle = OS_NULL;
    enum clnt_stat ret;
    LOOKUP3args *args;
    LOOKUP3res *res;
    char *file;
    char *path;
    char *init;

    path = os_malloc(strlen(name) + 1);
    if (!path)
    {
        return OS_NULL;
    }
    init = path;
    memcpy(init, name, strlen(name) + 1);

    args = os_malloc(sizeof(LOOKUP3args));
    if(!args)
    {
        os_free(init);
        return OS_NULL;
    }
    res = os_malloc(sizeof(LOOKUP3res));
    if(!res)
    {
        os_free(args);
        os_free(init);
        return OS_NULL;
    }

    handle = os_malloc(sizeof(nfs_fh3));
    if (!handle)
    {
        goto end;
    }

    if (path[0] == '/')
    {
        path ++;
    }
    copy_handle(handle, &nfs->root_handle);
    while ((file = strtok_r(OS_NULL, "/", &path)) != OS_NULL)
    {
        copy_handle(&args->what.dir, handle);
        xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)handle);
        args->what.name = file;
        memset(res, 0, sizeof(LOOKUP3res));
        ret = nfsproc3_lookup_3(args, res, nfs->nfs_client);
        if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
        {
            xdr_free((xdrproc_t)xdr_LOOKUP3res, (char *)res);
            xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)&args->what.dir);
            os_free(handle);
            handle = OS_NULL;
            goto end;
        }
        copy_handle(handle, &res->LOOKUP3res_u.resok.object);
        xdr_free((xdrproc_t)xdr_LOOKUP3res, (char *)res);
        xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)&args->what.dir);
    }

end:
    os_free(res);
    os_free(args);
    os_free(init);
    return handle;
}

static nfs_fh3 *nfs_get_dir_handle(struct nfs_filesystem *nfs, const char *name)
{
    nfs_fh3 *handle = OS_NULL;
    enum clnt_stat ret;
    LOOKUP3args *args;
    LOOKUP3res *res;
    char *file;
    char *path;
    char *init;

    path = os_malloc(strlen(name) + 1);
    if (!path)
    {
        return OS_NULL;
    }
    init = path;
    memcpy(init, name, strlen(name) + 1);

    args = os_malloc(sizeof(LOOKUP3args));
    if(!args)
    {
        os_free(init);
        return OS_NULL;
    }
    res = os_malloc(sizeof(LOOKUP3res));
    if(!res)
    {
        os_free(args);
        os_free(init);
        return OS_NULL;
    }

    handle = os_malloc(sizeof(nfs_fh3));
    if (!handle)
    {
        goto end;
    }

    if (path[0] == '/')
    {
        path ++;
    }
    copy_handle(handle, &nfs->root_handle);
    while ((file = strtok_r(OS_NULL, "/", &path)) != OS_NULL && path && path[0] != 0)
    {
        copy_handle(&args->what.dir, handle);
        xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)handle);
        args->what.name = file;
        memset(res, 0, sizeof(LOOKUP3res));
        ret = nfsproc3_lookup_3(args, res, nfs->nfs_client);
        if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
        {
            xdr_free((xdrproc_t)xdr_LOOKUP3res, (char *)res);
            xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)&args->what.dir);
            os_free(handle);
            handle = OS_NULL;
            goto end;
        }
        copy_handle(handle, &res->LOOKUP3res_u.resok.object);
        xdr_free((xdrproc_t)xdr_LOOKUP3res, (char *)res);
        xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)&args->what.dir);
    }

end:
    os_free(res);
    os_free(args);
    os_free(init);
    return handle;
}


struct nfs_filesystem *nfs_mount(char *data)
{
    mountres3 *res;
    struct nfs_filesystem *nfs;
    enum clnt_stat ret;

    res = os_malloc(sizeof(mountres3));
    if (!res)
    {
        return OS_NULL;
    }
    nfs = (struct nfs_filesystem *)os_malloc(sizeof(struct nfs_filesystem));
    if (!nfs)
    {
        os_free(res);
        return OS_NULL;
    }
    memset(nfs, 0, sizeof(struct nfs_filesystem));

    if (nfs_parse_host_export((const char *)data, nfs->host, HOST_LENGTH, nfs->export, EXPORT_PATH_LENGTH) < 0)
    {
        LOG_W(NFS_TAG, "host or export path error\n");
        goto err;
    }

    nfs->mount_client = clnt_create((char *)nfs->host, MOUNT_PROGRAM, MOUNT_V3, "udp");
    if (!nfs->mount_client)
    {
        LOG_W(NFS_TAG, "create mount client failed\n");
        goto err;
    }

    memset(res, '\0', sizeof(mountres3));
    ret = mountproc3_mnt_3((char *)nfs->export, res, nfs->mount_client);
    if ((ret != RPC_SUCCESS) || (res->fhs_status != MNT3_OK))
    {
        LOG_W(NFS_TAG, "nfs mount failed, ret:%d status:%d\n", ret, res->fhs_status);
        xdr_free((xdrproc_t)xdr_mountres3, (char *)res);
        goto err;
    }
    copy_handle(&nfs->root_handle, (nfs_fh3 *)&res->mountres3_u.mountinfo.fhandle);
    xdr_free((xdrproc_t)xdr_mountres3, (char *)res);
    nfs->nfs_client = clnt_create((char *)nfs->host, NFS_PROGRAM, NFS_V3, "udp");
    if (!nfs->nfs_client)
    {
        xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)&nfs->root_handle);
        LOG_W(NFS_TAG, "creat nfs client failed\n");
        goto err;
    }

    nfs->nfs_client->cl_auth = authnone_create();
    os_free(res);

    return nfs;

err:
    os_free(res);
    if (nfs)
    {
        if (nfs->mount_client)
        {
            clnt_destroy(nfs->mount_client);
        }
        
        if (nfs->nfs_client)
        {
            if (nfs->nfs_client->cl_auth)
            {
                auth_destroy(nfs->nfs_client->cl_auth);
            }
            clnt_destroy(nfs->nfs_client);
        }
        os_free(nfs);
    }

    return OS_NULL;
}

int nfs_unmount(struct nfs_filesystem *nfs)
{
    if ((nfs->mount_client) && (mountproc3_umnt_3((char *)nfs->export, OS_NULL, nfs->mount_client) != RPC_SUCCESS))
    {
        LOG_W(NFS_TAG, "unmount failed\n");

        return -1;
    }

    /* destroy nfs client */
    if (nfs->nfs_client)
    {
        if (nfs->nfs_client->cl_auth)
        {
            auth_destroy(nfs->nfs_client->cl_auth);
            nfs->nfs_client->cl_auth = OS_NULL;
        }
        clnt_destroy(nfs->nfs_client);
        nfs->nfs_client = OS_NULL;
    }

    /* destroy mount client */
    if (nfs->mount_client)
    {
        if (nfs->mount_client->cl_auth)
        {
            auth_destroy(nfs->mount_client->cl_auth);
            nfs->mount_client->cl_auth = OS_NULL;
        }
        clnt_destroy(nfs->mount_client);
        nfs->mount_client = OS_NULL;
    }

    xdr_free((xdrproc_t)xdr_nfs_fh3, (char *)&nfs->root_handle);
    os_free(nfs);

    return 0;
}


int nfs_stat(struct nfs_filesystem *nfs, const char *name, fattr3 *attributes)
{
    GETATTR3args *args;
    GETATTR3res *res;
    nfs_fh3 *handle;
    enum clnt_stat ret;
    int result = 0;

    args = os_malloc(sizeof(GETATTR3args));
    if(!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(GETATTR3res));
    if(!res)
    {
        os_free(args);
        return -ENOMEM;
    }
    handle = nfs_get_handle(nfs, name);
    if (!handle)
    {
        os_free(res);
        os_free(args);
        return -ENOENT;
    }

    memcpy(&args->object, handle, sizeof(nfs_fh3));
    memset(res, '\0', sizeof(GETATTR3res));

    ret = nfsproc3_getattr_3(args, res, nfs->nfs_client);
    if ((ret != RPC_SUCCESS) && (res->status != NFS3_OK))
    {
        LOG_W(NFS_TAG, "Getattr failed, ret:%d status:%d\n", (os_uint32_t)ret, (os_uint32_t)res->status);
        result = -1;
        goto end;
    }
    memcpy(attributes, &res->GETATTR3res_u.resok.obj_attributes, sizeof(struct fattr3));

end:
    xdr_free((xdrproc_t)xdr_GETATTR3res, (char *)&res);
    nfs_free_handle(handle);
    os_free(res);
    os_free(args);

    return result;
}

size_t nfs_get_filesize(struct nfs_filesystem *nfs, const char *name)
{
    fattr3 *info;
    int result;
    size_t size = 0;

    info = os_malloc(sizeof(fattr3));
    if (!info)
    {
        return 0;
    }
    result = nfs_stat(nfs, name, info);
    if (result  == 0)
    {
        size = info->size;
    }
    os_free(info);
    return size;
}

int nfs_get_type(struct nfs_filesystem *nfs, const char *name)
{
    fattr3 *info;
    int result;
    int type = FT_REGULAR;

    info = os_malloc(sizeof(fattr3));
    if (!info)
    {
        LOG_W(NFS_TAG, "nfs_get_type os_malloc fail");
        return type;
    }
    result = nfs_stat(nfs, name, info);
    if (result == 0)
    {
        type = (info->type == NFS3DIR) ? FT_DIRECTORY: FT_REGULAR;
    }
    os_free(info);

    return type;
}

int nfs_read(struct nfs_filesystem *nfs, struct nfs_fd *n_fd, void *buf, off_t pos, size_t size)
{
    READ3args *args;
    READ3res *res;
    ssize_t bytes;
    ssize_t total = 0;
    enum clnt_stat ret;

    if (!nfs->nfs_client)
    {
        return -EINVAL;
    }
    args = os_malloc(sizeof(READ3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(READ3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    memcpy(&args->file, n_fd->handle, sizeof(struct nfs_fh3));
    while (size > 0)
    {
        args->offset = pos;
        args->count = size > DFS_NFS_MAX_MTU ? DFS_NFS_MAX_MTU : size;
        size -= args->count;

        memset(res, 0, sizeof(READ3res));
        ret = nfsproc3_read_3(args, res, nfs->nfs_client);
        if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
        {
            LOG_W(NFS_TAG, "Read failed, ret:%d status:%d\n", (os_uint32_t)ret, (os_uint32_t)res->status);
            xdr_free((xdrproc_t)xdr_READ3res, (char *)res);
            goto end;
        }

        bytes = res->READ3res_u.resok.count;
        memcpy(buf, res->READ3res_u.resok.data.data_val, bytes);
        buf = (void *)((char *)buf + args->count);
        total += bytes;
        pos += bytes;
        n_fd->offset = pos;
        if (n_fd->size < n_fd->offset)
        {
           n_fd->size = n_fd->offset;
        }
        if (res->READ3res_u.resok.eof)
        {
            xdr_free((xdrproc_t)xdr_READ3res, (char *)res);
            //n_fd->eof = OS_TRUE;
            break;
        }
        xdr_free((xdrproc_t)xdr_READ3res, (char *)res);
    }

end:
    os_free(res);
    os_free(args);
    return total;
}


int nfs_write(struct nfs_filesystem *nfs, struct nfs_fd *n_fd, const void *buf, off_t pos, size_t size)
{
    WRITE3args *args;
    WRITE3res *res;
    ssize_t bytes;
    ssize_t total;
    enum clnt_stat ret;

    if (!nfs->nfs_client)
    {
        return -EINVAL;
    }
    args = os_malloc(sizeof(WRITE3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(WRITE3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    memcpy(&args->file, n_fd->handle, sizeof(struct nfs_fh3));
    //args.file = n_fd->handle;
    args->stable = FILE_SYNC;
    total = 0;
    while (size > 0)
    {
        args->offset = pos;
        args->data.data_val = (void *)buf;
        args->count = size > DFS_NFS_MAX_MTU ? DFS_NFS_MAX_MTU : size;
        args->data.data_len = args->count;
        size -= args->count;
        buf = (const void *)((char *)buf + args->count);

        memset(res, 0, sizeof(WRITE3res));
        ret = nfsproc3_write_3(args, res, nfs->nfs_client);
        if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
        {
            LOG_W(NFS_TAG, "Write failed, ret:%d status:%d\n", (os_uint32_t)ret, (os_uint32_t)res->status);
            xdr_free((xdrproc_t)xdr_WRITE3res, (char *)res);
            goto end;
        }

        bytes = res->WRITE3res_u.resok.count;
        total += bytes;
        pos += bytes;
        n_fd->offset = pos;
        if (n_fd->size < n_fd->offset)
        {
           n_fd->size = n_fd->offset;
        }
        xdr_free((xdrproc_t)xdr_WRITE3res, (char *)res);
    }

end:
    os_free(res);
    os_free(args);
    return total;
}

void nfs_seekfile(struct nfs_fd *n_fd, off_t offset)
{
    n_fd->offset = offset;
}

void nfs_seekdir(struct nfs_dir *dir, off_t offset)
{
    dir->cookie = offset + 2;   /*consider '.' and '..', should add 2. */
}

off_t nfs_telldir(struct nfs_dir *dir)
{
    return (off_t)dir->cookie - 2; /*consider '.' and '..', should add 2. */
}

void nfs_closefile(struct nfs_fd *n_fd)
{
    nfs_free_handle(n_fd->handle);
    os_free(n_fd);
}

void nfs_closedir(struct nfs_dir *dir)
{
    if (dir->readflag == OS_TRUE)
    {
        xdr_free((xdrproc_t)xdr_READDIRPLUS3res, (char *)&dir->res);
    }
    nfs_free_handle(dir->handle);
    os_free(dir);
}

struct nfs_fd *nfs_openfile(struct nfs_filesystem *nfs, const char *path, os_uint32_t flags)
{
    struct nfs_fd *n_fd;
    struct nfs_fh3 *handle;

    n_fd = os_malloc(sizeof(struct nfs_fd));
    if (!n_fd)
    {
        return OS_NULL;
    }
    handle = nfs_get_handle(nfs, path);
    if (!handle)
    {
        os_free(n_fd);
        return OS_NULL;
    }
    n_fd->size = nfs_get_filesize(nfs, path);
    n_fd->offset = 0;
    //n_fd->eof = (n_fd->size > 0) ? OS_FALSE : OS_TRUE;
    n_fd->handle = handle;
    if (flags & O_APPEND)
    {
        n_fd->offset = n_fd->size;
    }

    return n_fd;
}

struct nfs_dir *nfs_opendir(struct nfs_filesystem *nfs, const char *path)
{
    struct nfs_dir *dir;
    nfs_fh3 *handle;

    if (nfs_get_type(nfs, path) == FT_REGULAR)
    {
        return OS_NULL;
    }

    dir = os_malloc(sizeof(struct nfs_dir));
    if (!dir)
    {
        return OS_NULL;
    }

    handle = nfs_get_handle(nfs, path);
    if (!handle)
    {
        os_free(dir);
        return OS_NULL;
    }
    dir->handle = handle;
    dir->cookie = 0;
    memset(&dir->cookieverf, '\0', sizeof(cookieverf3));
    dir->entry = OS_NULL;
    dir->eof = OS_FALSE;
    dir->readflag = OS_FALSE;
    memset(&dir->res, '\0', sizeof(dir->res));

    return dir;
}

int nfs_mkfile(struct nfs_filesystem *nfs, const char *name, mode_t mode)
{
    CREATE3args *args;
    CREATE3res *res;
    nfs_fh3 *handle;
    enum clnt_stat ret;
    int result = 0;

    if (!nfs->nfs_client)
    {
        return -EINVAL;
    }
    args = os_malloc(sizeof(CREATE3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(CREATE3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    handle = nfs_get_dir_handle(nfs, name);
    if (!handle)
    {
        os_free(res);
        os_free(args);
        return -ENOENT;
    }
    memcpy(&args->where.dir, handle, sizeof(struct nfs_fh3));
    args->where.name = strrchr(name, '/') + 1;
    if (args->where.name == OS_NULL)
    {
        args->where.name = (char *)name;
    }
    args->how.mode = GUARDED;

    args->how.createhow3_u.obj_attributes.mode.set_it = TRUE;
    args->how.createhow3_u.obj_attributes.mode.set_mode3_u.mode = mode;
    args->how.createhow3_u.obj_attributes.uid.set_it = FALSE;
    args->how.createhow3_u.obj_attributes.gid.set_it = FALSE;
    args->how.createhow3_u.obj_attributes.size.set_it = FALSE;
    args->how.createhow3_u.obj_attributes.atime.set_it = DONT_CHANGE;
    args->how.createhow3_u.obj_attributes.mtime.set_it = DONT_CHANGE;
    memset(res, 0, sizeof(CREATE3res));
    ret = nfsproc3_create_3(args, res, nfs->nfs_client);
    if ((ret != RPC_SUCCESS) || ((res->status != NFS3_OK) && (res->status != NFS3ERR_EXIST)))
    {
        LOG_W(NFS_TAG, "Create %s failed, ret:%d status:%d\n", args->where.name, (os_uint32_t)ret, (os_uint32_t)res->status);
        result = -EINVAL;
    }
    xdr_free((xdrproc_t)xdr_CREATE3res, (char *)res);
    nfs_free_handle(handle);

    os_free(res);
    os_free(args);
    return result;
}

int nfs_mkdir(struct nfs_filesystem *nfs, const char *name, mode_t mode)
{
    MKDIR3args *args;
    MKDIR3res *res;
    nfs_fh3 *handle;
    enum clnt_stat ret;
    int result = 0;

    if (!nfs->nfs_client)
    {
        return -EINVAL;
    }

    args = os_malloc(sizeof(MKDIR3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(MKDIR3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    handle = nfs_get_dir_handle(nfs, name);
    if (!handle)
    {
        os_free(res);
        os_free(args);
        return -ENOENT;
    }

    memcpy(&args->where.dir, handle, sizeof(struct nfs_fh3));
    args->where.name = strrchr(name, '/') + 1;
    if (args->where.name == OS_NULL)
    {
        args->where.name = (char *)name;
    }

    args->attributes.mode.set_it = TRUE;
    args->attributes.mode.set_mode3_u.mode = mode;
    args->attributes.uid.set_it = FALSE;
    args->attributes.gid.set_it = FALSE;
    args->attributes.size.set_it = FALSE;
    args->attributes.atime.set_it = DONT_CHANGE;
    args->attributes.mtime.set_it = DONT_CHANGE;

    memset(res, 0, sizeof(MKDIR3res));
    ret = nfsproc3_mkdir_3(args, res, nfs->nfs_client);
    if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
    {
        LOG_W(NFS_TAG, "Mkdir failed, ret:%d status:%d\n", (os_uint32_t)ret, (os_uint32_t)res->status);
        result = -EINVAL;
    }
    xdr_free((xdrproc_t)xdr_MKDIR3res, (char *)res);
    nfs_free_handle(handle);
    os_free(res);
    os_free(args);

    return result;
}

int nfs_readdir(struct nfs_filesystem *nfs, struct nfs_dir *dir, struct dirent *d)
{
    READDIRPLUS3args *args;
    enum clnt_stat ret;

    if ((!nfs->nfs_client) || (!dir))
    {
        return -EINVAL;
    }

    /* Condition1: Initial state, not readdir before. */
    /* Condition2: Readdir before, all entrys in the last readdir's result has been fetched, 
                   and there is more entry to be read, need another readdir sequence. */
    if ((!dir->entry) && (dir->eof == OS_FALSE))
    {
        if (dir->readflag == OS_TRUE)
        {
            xdr_free((xdrproc_t)xdr_READDIRPLUS3res, (char *)&dir->res);
            dir->readflag = OS_FALSE;
        }
        args = os_malloc(sizeof(READDIRPLUS3args));
        if (!args)
        {
            return -ENOMEM;
        }
        memset(&dir->res, 0, sizeof(dir->res));
        memcpy(&args->dir, dir->handle, sizeof(struct nfs_fh3));
        args->cookie = dir->cookie;
        memcpy(&args->cookieverf, &dir->cookieverf, sizeof(cookieverf3));
        args->dircount = 1024;
        args->maxcount = 1024;

        ret = nfsproc3_readdirplus_3(args, &dir->res, nfs->nfs_client);
        os_free(args);
        /*Set readflag, indicate xdr_free in these conditon : 1.closedir 2.All result has been fetched*/
        dir->readflag = OS_TRUE;
        if ((ret != RPC_SUCCESS) || (dir->res.status != NFS3_OK))
        {
            LOG_W(NFS_TAG, "Readdir failed, ret:%d status:%d\n", ret, dir->res.status);
            xdr_free((xdrproc_t)xdr_READDIRPLUS3res, (char *)&dir->res);
            dir->readflag = OS_FALSE;
            return -EINVAL;
        }

        memcpy(&dir->cookieverf, &dir->res.READDIRPLUS3res_u.resok.cookieverf, sizeof(cookieverf3));
        dir->eof = dir->res.READDIRPLUS3res_u.resok.reply.eof;
        dir->entry = dir->res.READDIRPLUS3res_u.resok.reply.entries;
    }

    /* All entrys in the last readdir's result has been fetched, and no more entry to be read, restore inital state*/
    if ((!dir->entry) && (dir->eof == OS_TRUE))
    {
        dir->cookie = 0;
        dir->eof = OS_FALSE;
        xdr_free((xdrproc_t)xdr_READDIRPLUS3res, (char *)&dir->res);
        dir->readflag = OS_FALSE;
        return -EINVAL;
    }

    /* Fetch an entry. */
    dir->cookie = dir->entry->cookie;
    d->d_type = (dir->entry->name_attributes.post_op_attr_u.attributes.type == NFS3DIR) ? DT_DIR: DT_REG;
    //d->d_namlen = strlen(dir->entry->name);
    //d->d_reclen = (os_uint16_t)sizeof(struct dirent);
    memset(d->d_name, 0, sizeof(VFS_PATH_MAX));
    strncpy(d->d_name, dir->entry->name, VFS_PATH_MAX);

    dir->entry = dir->entry->nextentry;

    return 0;
}


int nfs_statfs(struct nfs_filesystem *nfs, struct statfs *buf)
{
    FSSTAT3args *args;
    FSSTAT3res *res;
    FSSTAT3resok *info;
    nfs_fh3 *handle;
    enum clnt_stat ret;
    int result = 0;

    args = os_malloc(sizeof(FSSTAT3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(FSSTAT3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    handle = nfs_get_handle(nfs, "/");  /* root handle */
    if (!handle)
    {
        os_free(res);
        os_free(args);
        return -ENOENT;
    }

    memcpy(&args->fsroot, handle, sizeof(struct nfs_fh3));
    //args.fsroot = *handle;
    memset(res, 0, sizeof(FSSTAT3res));
    ret = nfsproc3_fsstat_3(args, res, nfs->nfs_client);
    if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
    {
        LOG_W(NFS_TAG, "Fsstat failed, ret:%d status:%d\n", (os_uint32_t)ret, (os_uint32_t)res->status);
        result = -EINVAL;
        goto end;
    }

    info = &res->FSSTAT3res_u.resok;
    buf->f_bsize  = NFS_BLOCK_SIZE;
    buf->f_blocks = info->tbytes/buf->f_bsize;
    buf->f_bfree  = info->fbytes/buf->f_bsize;

end:
    xdr_free((xdrproc_t)xdr_FSSTAT3res, (char *)res);
    nfs_free_handle(handle);
    os_free(res);
    os_free(args);
    return result;
}


int nfs_unlinkfile(struct nfs_filesystem *nfs, const char *path)
{
    REMOVE3args *args;
    REMOVE3res *res;
    nfs_fh3 *handle;
    enum clnt_stat ret;
    int result = 0;

    args = os_malloc(sizeof(REMOVE3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(REMOVE3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    handle = nfs_get_dir_handle(nfs, path);
    if (!handle)
    {
        os_free(res);
        os_free(args);
        return -ENOENT;
    }

    memcpy(&args->object.dir, handle, sizeof(struct nfs_fh3));
    args->object.name = strrchr(path, '/') + 1;
    if (args->object.name == OS_NULL)
    {
        args->object.name = (char *)path;
    }
    memset(res, 0, sizeof(REMOVE3res));
    ret = nfsproc3_remove_3(args, res, nfs->nfs_client);
    if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
    {
        LOG_W(NFS_TAG, "Remove failed, ret:%d status:%d\n", ret, res->status);
        result = -EINVAL;
    }
    xdr_free((xdrproc_t)xdr_REMOVE3res, (char *)res);
    nfs_free_handle(handle);
    os_free(res);
    os_free(args);

    return result;
}

int nfs_unlinkdir(struct nfs_filesystem *nfs, const char *path)
{
    RMDIR3args *args;
    RMDIR3res *res;
    nfs_fh3 *handle;
    enum clnt_stat ret;
    int result = 0;

    args = os_malloc(sizeof(RMDIR3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(RMDIR3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    handle = nfs_get_dir_handle(nfs, path);
    if (!handle)
    {
        os_free(res);
        os_free(args);
        return -ENOENT;
    }

    memcpy(&args->object.dir, handle, sizeof(struct nfs_fh3));
    args->object.name = strrchr(path, '/') + 1;
    if (args->object.name == OS_NULL)
    {
        args->object.name = (char *)path;
    }
    memset(res, 0, sizeof(RMDIR3res));
    ret = nfsproc3_rmdir_3(args, res, nfs->nfs_client);
    if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
    {
        LOG_W(NFS_TAG, "Rmdir failed, ret:%d status:%d\n", (os_uint32_t)ret, (os_uint32_t)res->status);
        result = -EINVAL;
    }
    xdr_free((xdrproc_t)xdr_RMDIR3res, (char *)res);
    nfs_free_handle(handle);
    os_free(res);
    os_free(args);

    return result;
}

static int nfs_rename_check(struct nfs_filesystem *nfs, const char *oldpath, const char *newpath)
{
    fattr3 *info_new;
    fattr3 *info_old;
    struct nfs_dir *dir;
    struct dirent *d;
    int result_new;
    int result_old;
    int result = 0;
    int entry_exist = OS_FALSE;
    char *lastname = OS_NULL;
    char *fatherdirname = OS_NULL;

    /* The new pathname shall not contain a path prefix that names old. */
    if (strlen(newpath) >= strlen(oldpath))
    {
        if (strncmp(newpath, oldpath, strlen(oldpath)) == 0)
        {
            if ('/' == newpath[strlen(oldpath)])
            {
                return -EINVAL;
            }
        }
    }

    info_old = os_malloc(sizeof(struct fattr3));
    if (!info_old)
    {
        return -ENOMEM;
    }
    info_new = os_malloc(sizeof(struct fattr3));
    if (!info_new)
    {
        os_free(info_old);
        return -ENOMEM;
    }

    /* Check if the old pathname exist, if not exist, return fail. */
    result_old = nfs_stat(nfs, oldpath, info_old);
    if ((result_old < 0))
    {
        result = result_old;
        goto end;
    }

    /* If newpath and oldpath are the same existing file, shall return successfully and perform no other action.*/
    if (strcmp(newpath, oldpath) == 0)
    {
        result = NFS_RENAME_NO_ACTION;
        goto end;
    }

    /* Check if the new pathname exist */
    result_new = nfs_stat(nfs, newpath, info_new);
    if (result_new == 0)
    {
        /* If new pathname's type not same as old pathname's type, return fail.*/
        if (info_new->type != info_old->type)
        {
            result = ((info_new->type == NFS3DIR) ? (-EISDIR) : (-ENOTDIR));
            goto end;
        }

        /* If new pathname is a file, delete it first */
        if (info_new->type == NFS3REG)
        {
            if (nfs_unlinkfile(nfs, newpath) < 0)
            {
                result = -EINVAL;
                goto end;
            }
        }

        /* If new pathname is a directory, check whether is empty */
        if (info_new->type == NFS3DIR)
        {
            d = os_malloc(sizeof(struct dirent));
            if (!d)
            {
                result = -ENOMEM;
                goto end;
            }
            dir = nfs_opendir(nfs, newpath);
            if (!dir)
            {
                os_free(d);
                result = -ENOENT;
                goto end;
            }
            while (nfs_readdir(nfs, dir, d) == 0)
            {
                if ((strcmp(d->d_name, ".") != 0) && (strcmp(d->d_name, "..") != 0))
                {
                    entry_exist = OS_TRUE;
                    break;
                }
            }
            nfs_closedir(dir);
            os_free(d);
            if (entry_exist == OS_FALSE)   /* Dest directory is empty, delete it first */
            {
                if(nfs_unlinkdir(nfs, newpath) < 0)
                {
                    result = -EINVAL;
                    goto end;
                }
            }
            else                    /* Dest directory not empty, not allowed to rename*/
            {
                result = -EINVAL;
                goto end;
            }
                
        }
    }
    else
    {
        /* If the new pathname not exist, check whether the father directory exist */
        lastname = strrchr(newpath, '/');
        fatherdirname = os_malloc(strlen(newpath) + 1);
        if (!fatherdirname)
        {
            result = -ENOMEM;
            goto end;
        }
        memset(fatherdirname, 0, strlen(newpath) + 1);
        strncpy(fatherdirname, newpath, strlen(newpath) - strlen(lastname) + 1);
        result_new = nfs_stat(nfs, fatherdirname, info_new);
        os_free(fatherdirname);
        /* If dest father directory not exist, fail.*/
        if ((result_new < 0) || (info_new->type != NFS3DIR))
        {
            result = -EINVAL;
            goto end;
        }
    }

end:
    os_free(info_old);
    os_free(info_new);
    return result;
}

int nfs_rename(struct nfs_filesystem *nfs, const char *oldpath, const char *newpath)
{
    RENAME3args *args;
    RENAME3res *res;
    nfs_fh3 *sHandle;
    nfs_fh3 *dHandle;
    enum clnt_stat ret;
    int result = 0;

    if (!nfs->nfs_client)
    {
        return -EINVAL;
    }

    result = nfs_rename_check(nfs, oldpath, newpath);
    if (result < 0)
    {
        return result;
    }
    else if (result == NFS_RENAME_NO_ACTION)
    {
        return 0;
    }

    args = os_malloc(sizeof(RENAME3args));
    if (!args)
    {
        return -ENOMEM;
    }
    res = os_malloc(sizeof(RENAME3res));
    if (!res)
    {
        os_free(args);
        return -ENOMEM;
    }

    sHandle = nfs_get_dir_handle(nfs, oldpath);
    if (!sHandle)
    {
        os_free(res);
        os_free(args);
        return -ENOENT;
    }

    dHandle = nfs_get_dir_handle(nfs, newpath);
    if (!dHandle)
    {
        nfs_free_handle(sHandle);
        os_free(res);
        os_free(args);
        return -ENOENT;
    }

    memcpy(&args->from.dir, sHandle, sizeof(struct nfs_fh3));
    args->from.name = strrchr(oldpath, '/') + 1;
    if (args->from.name == OS_NULL)
        args->from.name = (char *)oldpath;

    memcpy(&args->to.dir, dHandle, sizeof(struct nfs_fh3));
    args->to.name = strrchr(newpath, '/') + 1;
    if (args->to.name == OS_NULL)
        args->to.name = (char *)newpath;

    memset(res, 0, sizeof(RENAME3res));
    ret = nfsproc3_rename_3(args, res, nfs->nfs_client);
    if ((ret != RPC_SUCCESS) || (res->status != NFS3_OK))
    {
        LOG_W(NFS_TAG, "Rename failed, ret:%d status:%d\n", (os_uint32_t)ret, (os_uint32_t)res->status);
        result = -EINVAL;
    }
    xdr_free((xdrproc_t)xdr_RENAME3res, (char *)res);
    nfs_free_handle(sHandle);
    nfs_free_handle(dHandle);
    os_free(res);
    os_free(args);

    return result;
}


