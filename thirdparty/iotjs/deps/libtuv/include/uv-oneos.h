
#ifndef UV_ONEOS_H
#define UV_ONEOS_H

#include <pthread.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>

#include <netdb.h>
// #include <sys/uio.h>

#ifndef TUV_POLL_EVENTS_SIZE
#define TUV_POLL_EVENTS_SIZE  32
#endif

// TUV_CHANGES@20161130: FIXME: What is the reasonable nubmer?
#ifndef IOV_MAX
#define IOV_MAX TUV_POLL_EVENTS_SIZE
#endif

// //---------------------------------------------------------------------------
// // TUV_CHANGES@20161130: NuttX doesn't provide ENOTSUP.
// #define ENOTSUP       EOPNOTSUPP

// TUV_CHANGES@20171130:
// Not defined macros. Copied from x86-64 linux system header
#define STDIN_FILENO  0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

#define SIGPROF       27

//-----------------------------------------------------------------------------
// TUV_CHANGES@20171130: Not used. I believe we can remove those.
// #define _SC_CLK_TCK           0x0006
// #define _SC_NPROCESSORS_ONLN  0x0061
#define CLOCK_MONOTONIC       1

//-----------------------------------------------------------------------------
// date time extension
// uint64_t uv__time_precise();


//-----------------------------------------------------------------------------
// thread and mutex
#define UV_PLATFORM_RWLOCK_T pthread_mutex_t

//-----------------------------------------------------------------------------
// uio
ssize_t readv(int __fd, const struct iovec* __iovec, int __count);
ssize_t writev(int __fd, const struct iovec* __iovec, int __count);


//-----------------------------------------------------------------------------
// etc
int getpeername(int sockfd, struct sockaddr* addr, socklen_t* addrlen);

// Maximum queue length specifiable by listen
#define SOMAXCONN 8

//-----------------------------------------------------------------------------
// structure extension for nuttx                                                          
//                                                                                        
#define UV_PLATFORM_LOOP_FIELDS                                               \
  struct pollfd pollfds[TUV_POLL_EVENTS_SIZE];                                \
  int npollfds;                                                               \



#endif // UV_NUTTX_H
