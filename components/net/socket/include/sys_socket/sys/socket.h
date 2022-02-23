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
 * @file        socket.h
 *
 * @brief       This file is a posix wrapper for at_sock.h.
 *
 * @revision
 * Date         Author          Notes
 * 2020-06-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <oneos_config.h>
#ifdef OS_USING_LIBC_ADAPTER
#include <sys/time.h>
#include <sys/select.h>
#endif
#if defined(BSD_USING_MOLINK)
#include <mo_socket.h>
#elif defined(BSD_USING_LWIP)
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#else
#error "Please select Molink stack with BSD socket operates or Lwip stack"
#endif /* end of if defined BSD_USING_MOLINK elif defined BSD_USING_LWIP */

int socket(int domain, int type, int protocol);
int closesocket(int fd);
int shutdown(int fd, int how);
int bind(int fd, const struct sockaddr *name, socklen_t namelen);
#if defined(BSD_USING_MOLINK)
int bind_with_cb(int fd, const struct sockaddr *name, socklen_t namelen, mo_netconn_data_callback cb);
#endif
int listen(int fd, int backlog);
int accept(int fd, struct sockaddr *addr, socklen_t *addrlen);
int connect(int fd, const struct sockaddr *name, socklen_t namelen);
int sendto(int fd, const void *data, size_t size, int flags, const struct sockaddr *to, socklen_t tolen);
int send(int fd, const void *data, size_t size, int flags);
int recvfrom(int fd, void *mem, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen);

int recv(int fd, void *mem, size_t len, int flags);
int getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlen);
int setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);
int ioctlsocket(int fd, long cmd, void *argp);
#ifndef OS_USING_LIBC_ADAPTER
int select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
#endif

struct hostent *gethostbyname(const char *name);
int  getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *ai);
int  getpeername(int fd, struct sockaddr *name, socklen_t *namelen);
int  getsockname(int fd, struct sockaddr *name, socklen_t *namelen);

#ifdef NET_USING_LWIP212
const char *inet_ntop(int af, const void *src, char *dst, int32_t size);
int         inet_pton(int af, const char *src, void *dst);
#endif /* end of NET_USING_LWIP212 */

#endif /* end of __SOCKET_H__ */
