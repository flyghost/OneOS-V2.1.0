/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        socket.c
 *
 * @brief       Implement socket functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-21   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <sys/socket.h>
#include <dlog.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <os_errno.h>
#include <os_assert.h>
#include <os_memory.h>

#define BSD_SOCKET_TAG "BSD_SOCKET"

#ifdef OS_USING_VFS_DEVFS

#include <vfs_devfs.h>

#define SHUTDOWN_OFFSET      1
#define SOCKET_DEV_NAME      "socket"
#define SOCKET_DEV_PATH_NAME "/dev/socket"

typedef enum
{
    SET_SOCKET_ID,
    SET_SHUTDOWN_MODE
} IOCTL_CMD;

struct socket_private
{
    int socket_id;
    int shutdown_how;
};

extern struct vfs_file *fd_to_fp(int fd);

static int sockfs_adapter_open(struct vfs_file *file);
static int sockfs_adapter_close(struct vfs_file *file);
static int sockfs_adapter_read(struct vfs_file *file, off_t pos, void *buf, size_t count);
static int sockfs_adapter_write(struct vfs_file *file, off_t pos, const void *buf, size_t count);
static int sockfs_adapter_ioctl(struct vfs_file *file, unsigned long cmd, void *args);
#ifdef OS_USING_IO_MULTIPLEXING
static int sockfs_adapter_poll(struct vfs_file *file, struct vfs_pollfd *req, os_bool_t poll_setup);
#endif

static const struct dev_file_ops gs_sockfs_fops = {
    sockfs_adapter_open,
    sockfs_adapter_close,
    sockfs_adapter_read,
    sockfs_adapter_write,
    sockfs_adapter_ioctl,
#ifdef OS_USING_IO_MULTIPLEXING
    sockfs_adapter_poll,
#endif
};

#define GET_SOCKETID_FROM_FILE(SOCKET, FILE)                                                                           \
    if (FILE->private == NULL)                                                                                         \
    {                                                                                                                  \
        return OS_ERROR;                                                                                               \
    }                                                                                                                  \
    struct socket_private *sock_private = FILE->private;                                                               \
    SOCKET                              = sock_private->socket_id;

#define GET_SOCKETID_SHUTDOWN_FROM_FILE(SOCKET, SHUTDOWN, FILE)                                                        \
    if (FILE->private == NULL)                                                                                         \
    {                                                                                                                  \
        return OS_ERROR;                                                                                               \
    }                                                                                                                  \
    struct socket_private *sock_private = FILE->private;                                                               \
    SOCKET                              = sock_private->socket_id;                                                     \
    SHUTDOWN                            = sock_private->shutdown_how;

#define SET_SOCKETID_TO_FILE(SOCKET, FILE)                                                                             \
    if (FILE->private == NULL)                                                                                         \
    {                                                                                                                  \
        return OS_ERROR;                                                                                               \
    }                                                                                                                  \
    struct socket_private *sock_private = FILE->private;                                                               \
    sock_private->socket_id             = SOCKET;

#define SET_SHUTDOWN_TO_FILE(SHUTDOWN, FILE)                                                                           \
    if (FILE->private == NULL)                                                                                         \
    {                                                                                                                  \
        return OS_ERROR;                                                                                               \
    }                                                                                                                  \
    struct socket_private *sock_private = FILE->private;                                                               \
    sock_private->shutdown_how          = SHUTDOWN;

#define GET_SOCKETID_FROM_FD(SOCKET, FD)                                                                               \
    struct vfs_file *file = fd_to_fp(FD);                                                                              \
    if (OS_NULL == file)                                                                                               \
    {                                                                                                                  \
        return OS_ERROR;                                                                                               \
    }                                                                                                                  \
    GET_SOCKETID_FROM_FILE(SOCKET, file)

#define TRANS_FD_TO_SOCKETID_IN_SET(FD, SOCKET, SET_FROM, SET_TO)                                                      \
    if (SET_FROM && FD_ISSET(FD, SET_FROM))                                                                            \
    {                                                                                                                  \
        GET_SOCKETID_FROM_FD(SOCKET, FD)                                                                               \
        if (SET_TO)                                                                                                    \
        {                                                                                                              \
            FD_SET(SOCKET, SET_TO);                                                                                    \
        }                                                                                                              \
    }

#define TRANS_SOCKETID_TO_FD_IN_SET_AND_COUNT(FD, SOCKET, SET_BAK, SET_FROM, SET_TO, RESULT)                           \
    if (SET_BAK && FD_ISSET(FD, SET_BAK))                                                                              \
    {                                                                                                                  \
        GET_SOCKETID_FROM_FD(SOCKET, FD)                                                                               \
        if (SET_FROM && FD_ISSET(SOCKET, SET_FROM))                                                                    \
        {                                                                                                              \
            if (SET_TO)                                                                                                \
            {                                                                                                          \
                FD_SET(FD, SET_TO);                                                                                    \
                ++RESULT;                                                                                              \
            }                                                                                                          \
        }                                                                                                              \
    }

#endif

#if defined(BSD_USING_MOLINK)

#include "mo_common.h"

#ifdef OS_USING_VFS_DEVFS
static int sockfs_adapter_open(struct vfs_file *file)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    return OS_EOK;
}

static int sockfs_adapter_close(struct vfs_file *file)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return OS_ERROR;
    }

    int socket = -1;
    GET_SOCKETID_FROM_FILE(socket, file)
    int ret = mo_closesocket(default_module, socket);

    if (file->private != OS_NULL)
    {
        os_free(file->private);
        file->private = OS_NULL;
    }
    return ret;
}

static int sockfs_adapter_read(struct vfs_file *file, off_t pos, void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    return OS_ERROR;
}

static int sockfs_adapter_write(struct vfs_file *file, off_t pos, const void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    return OS_ERROR;
}

static int sockfs_adapter_ioctl(struct vfs_file *file, unsigned long cmd, void *args)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    os_bool_t b_match = OS_FALSE;

    if (file->private == OS_NULL)
    {
        file->private = os_malloc(sizeof(struct socket_private));
        if (file->private)
        {
            memset(file->private, 0, sizeof(struct socket_private));
        }
    }

    switch (cmd)
    {
    case SET_SOCKET_ID:
    {
        SET_SOCKETID_TO_FILE((int)args, file)
        b_match = OS_TRUE;
        break;
    }
    case SET_SHUTDOWN_MODE:
    {
        SET_SHUTDOWN_TO_FILE((int)args, file)
        b_match = OS_TRUE;
        break;
    }
    default:
        break;
    }

    return (b_match == OS_TRUE ? OS_EOK : OS_ERROR);
}

#ifdef OS_USING_IO_MULTIPLEXING
static int sockfs_adapter_poll(struct vfs_file *file, struct vfs_pollfd *req, os_bool_t poll_setup)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);
    OS_ASSERT(req != OS_NULL);

    int socket = -1;
    GET_SOCKETID_FROM_FILE(socket, file)
    return mo_poll(socket, req, poll_setup);
}
#else
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    // save socket id
    fd_set readset_tmp;
    fd_set writeset_tmp;
    fd_set exceptset_tmp;
    // backup fd
    fd_set readset_bak;
    fd_set writeset_bak;
    fd_set exceptset_bak;
    // init fd_set for save socket id
    FD_ZERO(&readset_tmp);
    FD_ZERO(&writeset_tmp);
    FD_ZERO(&exceptset_tmp);
    // init fd_set for backup
    FD_ZERO(&readset_bak);
    FD_ZERO(&writeset_bak);
    FD_ZERO(&exceptset_bak);
    // backup
    if (readset)
    {
        memcpy(&readset_bak, readset, sizeof(fd_set));
    }
    if (writeset)
    {
        memcpy(&writeset_bak, writeset, sizeof(fd_set));
    }
    if (exceptset)
    {
        memcpy(&exceptset_bak, exceptset, sizeof(fd_set));
    }
    int fd     = 0;
    int socket = 0;
    // tansfor to socket id
    for (fd = 0; fd < maxfdp1; fd++)
    {
        TRANS_FD_TO_SOCKETID_IN_SET(fd, socket, readset, &readset_tmp)
        TRANS_FD_TO_SOCKETID_IN_SET(fd, socket, writeset, &writeset_tmp)
        TRANS_FD_TO_SOCKETID_IN_SET(fd, socket, exceptset, &exceptset_tmp)
    }
    // select
    int result = mo_select(maxfdp1, &readset_tmp, &writeset_tmp, &exceptset_tmp, timeout);
    if (readset)
    {
        FD_ZERO(readset);
    }

    if (writeset)
    {
        FD_ZERO(writeset);
    }

    if (exceptset)
    {
        FD_ZERO(exceptset);
    }
    // tansfor to fd
    if (result > 0)
    {
        result = 0;
        for (fd = 0; fd < maxfdp1; fd++)
        {
            TRANS_SOCKETID_TO_FD_IN_SET_AND_COUNT(fd, socket, &readset_bak, &readset_tmp, readset, result)
            TRANS_SOCKETID_TO_FD_IN_SET_AND_COUNT(fd, socket, &writeset_bak, &writeset_tmp, writeset, result)
            TRANS_SOCKETID_TO_FD_IN_SET_AND_COUNT(fd, socket, &exceptset_bak, &exceptset_tmp, exceptset, result)
        }
    }
    return result;
}
#endif
#else
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    return mo_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif

int socket(int domain, int type, int protocol)
{
    int socket;

    mo_object_t *default_module = mo_get_default();

    if (OS_NULL == default_module)
    {
        os_set_errno(-EBADF);
        return OS_ERROR;
    }

    socket = mo_socket(default_module, domain, type, protocol);
    if (socket < 0)
    {
        os_set_errno(-ENOMEM);
        return OS_ERROR;
    }

#ifdef OS_USING_VFS_DEVFS
    int fd;

    fd = open(SOCKET_DEV_PATH_NAME, O_RDWR);
    if (fd < 0)
    {
        mo_closesocket(default_module, socket);
        os_set_errno(-ENOMEM);
        return OS_ERROR;
    }
    ioctl(fd, SET_SOCKET_ID, socket);

    return fd;
#else
    return socket;
#endif
}

int closesocket(int fd)
{
    int ret;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    ret = close(fd);
#else
    ret    = mo_closesocket(default_module, fd);
#endif

    return ret;
}

int shutdown(int fd, int how)
{
    LOG_E(BSD_SOCKET_TAG, "OneOS module is not support shutdown");
    return -1;
}

int bind(int fd, const struct sockaddr *name, socklen_t namelen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_bind(default_module, socket, name, namelen);
}

int bind_with_cb(int fd, const struct sockaddr *name, socklen_t namelen, mo_netconn_data_callback cb)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_bind_with_cb(default_module, socket, name, namelen, cb);
}

int listen(int fd, int backlog)
{
    LOG_E(BSD_SOCKET_TAG, "OneOS module is not support listen");
    return -1;
}

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    LOG_E(BSD_SOCKET_TAG, "OneOS module is not support accept");
    return -1;
}

int connect(int fd, const struct sockaddr *name, socklen_t namelen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_connect(default_module, socket, name, namelen);
}

int sendto(int fd, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_sendto(default_module, socket, data, size, flags, to, tolen);
}

int send(int fd, const void *data, size_t size, int flags)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_send(default_module, socket, data, size, flags);
}

int recvfrom(int fd, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_recvfrom(default_module, socket, mem, len, flags, from, fromlen);
}

int recv(int fd, void *mem, size_t len, int flags)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_recv(default_module, socket, mem, len, flags);
}

int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_getsockopt(default_module, socket, level, optname, optval, optlen);
}

int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int socket;

    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return -1;
    }

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return mo_setsockopt(default_module, socket, level, optname, optval, optlen);
}

struct hostent *gethostbyname(const char *name)
{
    mo_object_t *default_module = mo_get_default();
    if (OS_NULL == default_module)
    {
        return OS_NULL;
    }

    return mo_gethostbyname(default_module, name);
}

int getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
    return mo_getaddrinfo(nodename, servname, hints, res);
}

void freeaddrinfo(struct addrinfo *ai)
{
    mo_freeaddrinfo(ai);
}

int getpeername(int fd, struct sockaddr *name, socklen_t *namelen)
{
    LOG_E(BSD_SOCKET_TAG, "OneOS module is not support getpeername");
    return -1;
}

int getsockname(int fd, struct sockaddr *name, socklen_t *namelen)
{
    LOG_E(BSD_SOCKET_TAG, "OneOS module is not support getsockname");
    return -1;
}

int ioctlsocket(int fd, long cmd, void *argp)
{
    LOG_E(BSD_SOCKET_TAG, "OneOS module is not support ioctlsocket");
    return -1;
}

#elif defined(BSD_USING_LWIP)

#ifdef OS_USING_VFS_DEVFS
static int sockfs_adapter_open(struct vfs_file *file)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    return OS_EOK;
}

static int sockfs_adapter_close(struct vfs_file *file)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    int ret          = OS_ERROR;
    int socket       = -1;
    int shutdown_how = 0;
    GET_SOCKETID_SHUTDOWN_FROM_FILE(socket, shutdown_how, file)
    if (shutdown_how)
    {
        ret = lwip_shutdown(socket, shutdown_how - SHUTDOWN_OFFSET);
    }
    else
    {
        ret = lwip_close(socket);
    }
    if (file->private != OS_NULL)
    {
        os_free(file->private);
        file->private = OS_NULL;
    }
    return ret;
}

static int sockfs_adapter_read(struct vfs_file *file, off_t pos, void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    return OS_ERROR;
}

static int sockfs_adapter_write(struct vfs_file *file, off_t pos, const void *buf, size_t count)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    return OS_ERROR;
}

static int sockfs_adapter_ioctl(struct vfs_file *file, unsigned long cmd, void *args)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);

    os_bool_t b_match = OS_FALSE;

    if (file->private == OS_NULL)
    {
        file->private = os_malloc(sizeof(struct socket_private));
        if (file->private)
        {
            memset(file->private, 0, sizeof(struct socket_private));
        }
    }

    switch (cmd)
    {
    case SET_SOCKET_ID:
    {
        SET_SOCKETID_TO_FILE((int)args, file)
        b_match = OS_TRUE;
        break;
    }
    case SET_SHUTDOWN_MODE:
    {
        SET_SHUTDOWN_TO_FILE((int)args, file)
        b_match = OS_TRUE;
        break;
    }
    default:
        break;
    }

    return (b_match == OS_TRUE ? OS_EOK : OS_ERROR);
}
#ifdef OS_USING_IO_MULTIPLEXING
static int sockfs_adapter_poll(struct vfs_file *file, struct vfs_pollfd *req, os_bool_t poll_setup)
{
    OS_ASSERT(file != OS_NULL);
    OS_ASSERT(file->type == FT_DEVICE);
    OS_ASSERT(req != OS_NULL);

    int socket = -1;
    GET_SOCKETID_FROM_FILE(socket, file)
    return lwip_posix_poll(socket, req, poll_setup);
}
#else
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    // save socket id
    fd_set readset_tmp;
    fd_set writeset_tmp;
    fd_set exceptset_tmp;
    // backup fd
    fd_set readset_bak;
    fd_set writeset_bak;
    fd_set exceptset_bak;
    // init fd_set for save socket id
    FD_ZERO(&readset_tmp);
    FD_ZERO(&writeset_tmp);
    FD_ZERO(&exceptset_tmp);
    // init fd_set for backup
    FD_ZERO(&readset_bak);
    FD_ZERO(&writeset_bak);
    FD_ZERO(&exceptset_bak);
    // backup
    if (readset)
    {
        memcpy(&readset_bak, readset, sizeof(fd_set));
    }
    if (writeset)
    {
        memcpy(&writeset_bak, writeset, sizeof(fd_set));
    }
    if (exceptset)
    {
        memcpy(&exceptset_bak, exceptset, sizeof(fd_set));
    }
    int fd     = 0;
    int socket = 0;
    // tansfor to socket id
    for (fd = 0; fd < maxfdp1; fd++)
    {
        TRANS_FD_TO_SOCKETID_IN_SET(fd, socket, readset, &readset_tmp)
        TRANS_FD_TO_SOCKETID_IN_SET(fd, socket, writeset, &writeset_tmp)
        TRANS_FD_TO_SOCKETID_IN_SET(fd, socket, exceptset, &exceptset_tmp)
    }
    // select
    int result = lwip_select(maxfdp1, &readset_tmp, &writeset_tmp, &exceptset_tmp, timeout);
    if (readset)
    {
        FD_ZERO(readset);
    }

    if (writeset)
    {
        FD_ZERO(writeset);
    }

    if (exceptset)
    {
        FD_ZERO(exceptset);
    }
    // tansfor to fd
    if (result > 0)
    {
        result = 0;
        for (fd = 0; fd < maxfdp1; fd++)
        {
            TRANS_SOCKETID_TO_FD_IN_SET_AND_COUNT(fd, socket, &readset_bak, &readset_tmp, readset, result)
            TRANS_SOCKETID_TO_FD_IN_SET_AND_COUNT(fd, socket, &writeset_bak, &writeset_tmp, writeset, result)
            TRANS_SOCKETID_TO_FD_IN_SET_AND_COUNT(fd, socket, &exceptset_bak, &exceptset_tmp, exceptset, result)
        }
    }
    return result;
}
#endif
#else
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    return lwip_select(maxfdp1, readset, writeset, exceptset, timeout);
}
#endif

int socket(int domain, int type, int protocol)
{
    int socket;

    socket = lwip_socket(domain, type, protocol);
    if (socket < 0)
    {
        os_set_errno(-ENOMEM);
        return OS_ERROR;
    }

#ifdef OS_USING_VFS_DEVFS
    int fd;

    fd = open(SOCKET_DEV_PATH_NAME, O_RDWR);
    if (fd < 0)
    {
        lwip_close(socket);
        os_set_errno(-ENOMEM);
        return OS_ERROR;
    }
    ioctl(fd, SET_SOCKET_ID, socket);

    return fd;
#else
    return socket;
#endif
}

int closesocket(int fd)
{
    int ret;

#ifdef OS_USING_VFS_DEVFS
    ret = close(fd);
#else
    ret = lwip_close(fd);
#endif

    return ret;
}

int shutdown(int fd, int how)
{
#ifdef OS_USING_VFS_DEVFS

    ioctl(fd, SET_SHUTDOWN_MODE, how + SHUTDOWN_OFFSET);
    return close(fd);
#else
    return lwip_shutdown(fd, how);
#endif
}

int bind(int fd, const struct sockaddr *name, socklen_t namelen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_bind(socket, name, namelen);
}

int listen(int fd, int backlog)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_listen(socket, backlog);
}

int accept(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    int socket;
    int accept_sk;
    
#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    accept_sk = lwip_accept(socket, addr, addrlen);
#ifdef OS_USING_VFS_DEVFS
    int accept_fd;

    accept_fd = open(SOCKET_DEV_PATH_NAME, O_RDWR);
    if (accept_fd < 0)
    {
        lwip_close(accept_sk);
        os_set_errno(-ENOMEM);
        return OS_ERROR;
    }
    ioctl(accept_fd, SET_SOCKET_ID, accept_sk);

    return accept_fd;
#else
    return accept_sk;
#endif
}

int connect(int fd, const struct sockaddr *name, socklen_t namelen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_connect(socket, name, namelen);
}

int sendto(int fd, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_sendto(socket, data, size, flags, to, tolen);
}

int send(int fd, const void *data, size_t size, int flags)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_send(socket, data, size, flags);
}

int recvfrom(int fd, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_recvfrom(socket, mem, len, flags, from, fromlen);
}

int recv(int fd, void *mem, size_t len, int flags)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_recv(socket, mem, len, flags);
}

int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_getsockopt(socket, level, optname, optval, optlen);
}

int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_setsockopt(socket, level, optname, optval, optlen);
}

struct hostent *gethostbyname(const char *name)
{
    return lwip_gethostbyname(name);
}

int getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res)
{
    return lwip_getaddrinfo(nodename, servname, hints, res);
}

void freeaddrinfo(struct addrinfo *ai)
{
    lwip_freeaddrinfo(ai);
}

int getpeername(int fd, struct sockaddr *name, socklen_t *namelen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_getpeername(socket, name, namelen);
}

int getsockname(int fd, struct sockaddr *name, socklen_t *namelen)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_getsockname(socket, name, namelen);
}

int ioctlsocket(int fd, long cmd, void *argp)
{
    int socket;

#ifdef OS_USING_VFS_DEVFS
    GET_SOCKETID_FROM_FD(socket, fd)
#else
    socket = fd;
#endif

    return lwip_ioctl(socket, cmd, argp);
}

#ifdef NET_USING_LWIP212
const char *inet_ntop(int af, const void *src, char *dst, int32_t size)
{
    return lwip_inet_ntop(af, src, dst, size);
}

int inet_pton(int af, const char *src, void *dst)
{
    return lwip_inet_pton(af, src, dst);
}
#endif

#endif

#ifdef OS_USING_VFS_DEVFS
int socket_init(void)
{
    return devfs_register_device(SOCKET_DEV_NAME, (struct dev_file_ops *)&gs_sockfs_fops, OS_NULL);
}

OS_CMPOENT_INIT(socket_init, OS_INIT_SUBLEVEL_LOW);
#endif
