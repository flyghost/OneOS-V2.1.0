
#ifndef TEST_SOCKET_H
#define TEST_SOCKET_H

// This file enables testing of these Linux-oriented unit tests under Windows. It is not 
// strictly necessary, but is convenient to have.

#ifdef __cplusplus
extern "C" {
#include <cstddef>
#include <cstdint>
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "sys/socket.h"
#include "sys/types.h"

#ifndef EINPROGRESS
#define  EINPROGRESS    112
#endif

#ifndef EAGAIN
#define  EAGAIN			11
#endif

#ifndef EWOULDBLOCK
#define  EWOULDBLOCK	140
#endif

#define	SO_ERROR	0x1007
#define	AF_INET		2
#define	SOCK_STREAM	1
#define	SOCK_DGRAM	2
#define	SO_KEEPALIVE	0x0008
#define IPPROTO_TCP     6
#define TCP_KEEPIDLE   0x03
#define TCP_KEEPINTVL  0x04
#define TCP_KEEPCNT    0x05
#define F_GETFL 3
#define F_SETFL 4

//    typedef size_t socklen_t;


    int fcntl(int fd, int cmd, ... /* arg */);

//    int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

//    int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
//    
//    int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);

    int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

    int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

    int send(int fd, const void *data, size_t size, int flags);

    int recv(int fd, void *mem, size_t len, int flags);

    int close(int fd);

#ifdef __cplusplus
}
#endif

#endif /* TEST_SOCKET_H */


