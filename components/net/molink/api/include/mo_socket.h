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
 * @file        mo_socket.h
 *
 * @brief       module link kit socket api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_SOCKETS_H__
#define __MO_SOCKETS_H__

#include "mo_netconn.h"

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <sys/select.h>

#ifdef OS_USING_IO_MULTIPLEXING
#include <vfs_devfs.h>
#endif

#ifdef OS_USING_LIBC_ADAPTER
#include <sys/select.h>
#endif

#ifdef MOLINK_USING_SOCKETS_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef MOLINK_NUM_SOCKETS
#define MOLINK_NUM_SOCKETS 16
#endif

#define MOLINK_SOCKETS_FD_MAX   MOLINK_NUM_SOCKETS

#ifndef MOLINK_DNS_MAX_NAME_LEN
#define MOLINK_DNS_MAX_NAME_LEN 256
#endif

#define EAI_NONAME  200
#define EAI_SERVICE 201
#define EAI_FAIL    202
#define EAI_MEMORY  203
#define EAI_FAMILY  204

#define AI_PASSIVE     0x01
#define AI_CANONNAME   0x02
#define AI_NUMERICHOST 0x04
#define AI_NUMERICSERV 0x08
#define AI_V4MAPPED    0x10
#define AI_ALL         0x20
#define AI_ADDRCONFIG  0x40

/* Socket protocol types (TCP/UDP/RAW) */
#define SOCK_STREAM 1
#define SOCK_DGRAM  2
#define SOCK_RAW    3

#define SOCK_MAX (SOCK_RAW + 1)

/* Option flags per-socket. These must match the SOF_ flags in ip.h (checked in init.c) */
#define SO_REUSEADDR 0x0004 /* Allow local address reuse */
#define SO_KEEPALIVE 0x0008 /* keep connections alive */
#define SO_BROADCAST 0x0020 /* permit to send and to receive broadcast messages (see IP_SOF_BROADCAST option) */

/* Additional options, not kept in so_options */
#define SO_DEBUG       0x0001 /* Unimplemented: turn on debugging info recording */
#define SO_ACCEPTCONN  0x0002 /* socket has had listen() */
#define SO_DONTROUTE   0x0010 /* Unimplemented: just use interface addresses */
#define SO_USELOOPBACK 0x0040 /* Unimplemented: bypass hardware when possible */
#define SO_LINGER      0x0080 /* linger on close if data present */
#define SO_DONTLINGER  ((int)(~SO_LINGER))
#define SO_OOBINLINE   0x0100 /* Unimplemented: leave received OOB data in line */
#define SO_REUSEPORT   0x0200 /* Unimplemented: allow local address & port reuse */
#define SO_SNDBUF      0x1001 /* Unimplemented: send buffer size */
#define SO_RCVBUF      0x1002 /* receive buffer size */
#define SO_SNDLOWAT    0x1003 /* Unimplemented: send low-water mark */
#define SO_RCVLOWAT    0x1004 /* Unimplemented: receive low-water mark */
#define SO_SNDTIMEO    0x1005 /* send timeout */
#define SO_RCVTIMEO    0x1006 /* receive timeout */
#define SO_ERROR       0x1007 /* get error status and clear */
#define SO_TYPE        0x1008 /* get socket type */
#define SO_CONTIMEO    0x1009 /* Unimplemented: connect timeout */
#define SO_NO_CHECK    0x100a /* don't create UDP checksum */

/* Level number for (get/set)sockopt() to apply to socket itself */
#define SOL_SOCKET 0xfff /* options for socket level */

#define AF_UNSPEC       0
#define AF_INET         2
#ifdef MOLINK_USING_IPV6
#define AF_INET6        10
#else /* MOLINK_USING_IPV6 */
#define AF_INET6        AF_UNSPEC
#endif /* MOLINK_USING_IPV6 */
#define PF_INET         AF_INET
#define PF_INET6        AF_INET6
#define PF_UNSPEC       AF_UNSPEC

#define IPPROTO_IP      0
#define IPPROTO_ICMP    1
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17
#ifdef MOLINK_USING_IPV6
#define IPPROTO_IPV6    41
#define IPPROTO_ICMPV6  58
#endif /* MOLINK_USING_IPV6 */
#define IPPROTO_UDPLITE 136
#define IPPROTO_RAW     255

/* Flags we can use with send and recv */
#define MSG_PEEK     0x01 /* Peeks at an incoming message */
#define MSG_WAITALL  0x02 /* Unimplemented: Requests that the function block until the full */
                          /* amount of data requested can be returned */
#define MSG_OOB      0x04 /* Unimplemented: Requests out-of-band data. The significance and semantics */
                          /* of out-of-band data are protocol-specific */
#define MSG_DONTWAIT 0x08 /* Nonblocking i/o for this operation only */
#define MSG_MORE     0x10 /* Sender will send more */

/* Options for level IPPROTO_IP */
#define IP_TOS 1
#define IP_TTL 2

/* Options for level IPPROTO_TCP */
#define TCP_NODELAY   0x01 /* don't delay send to coalesce packets */
#define TCP_KEEPALIVE 0x02 /* send KEEPALIVE probes when idle for pcb->keep_idle milliseconds */
#define TCP_KEEPIDLE  0x03 /* set pcb->keep_idle  - Same as TCP_KEEPALIVE, but use seconds for get/setsockopt */
#define TCP_KEEPINTVL 0x04 /* set pcb->keep_intvl - Use seconds for get/setsockopt */
#define TCP_KEEPCNT   0x05 /* set pcb->keep_cnt   - Use number of probes sent for get/setsockopt */

/* Options and types related to multicast membership */
#define IP_ADD_MEMBERSHIP  3
#define IP_DROP_MEMBERSHIP 4
/* Options and types for UDP multicast traffic handling */
#define IP_MULTICAST_TTL  5
#define IP_MULTICAST_IF   6
#define IP_MULTICAST_LOOP 7

/* The Type of Service provides an indication of the abstract parameters of the quality of service desired */
#define IPTOS_TOS_MASK    0x1E
#define IPTOS_TOS(tos)    ((tos)&IPTOS_TOS_MASK)
#define IPTOS_LOWDELAY    0x10
#define IPTOS_THROUGHPUT  0x08
#define IPTOS_RELIABILITY 0x04
#define IPTOS_LOWCOST     0x02
#define IPTOS_MINCOST     IPTOS_LOWCOST

/* The Network Control precedence designation is intended to be used within a network only */
#define IPTOS_PREC_MASK            0xe0
#define IPTOS_PREC(tos)            ((tos)&IPTOS_PREC_MASK)
#define IPTOS_PREC_NETCONTROL      0xe0
#define IPTOS_PREC_INTERNETCONTROL 0xc0
#define IPTOS_PREC_CRITIC_ECP      0xa0
#define IPTOS_PREC_FLASHOVERRIDE   0x80
#define IPTOS_PREC_FLASH           0x60
#define IPTOS_PREC_IMMEDIATE       0x40
#define IPTOS_PREC_PRIORITY        0x20
#define IPTOS_PREC_ROUTINE         0x00

/* Options for shatdown type */
#ifndef SHUT_RD
#define SHUT_RD   0
#define SHUT_WR   1
#define SHUT_RDWR 2
#endif

#if !defined(socklen_t) && !defined(SOCKLEN_T_DEFINED)
typedef uint32_t socklen_t;
#endif

#if !defined(sa_family_t) && !defined(SA_FAMILY_T_DEFINED)
typedef uint8_t sa_family_t;
#endif

/* If your port already typedef's in_port_t, define IN_PORT_T_DEFINED to prevent this code from redefining it. */
#if !defined(in_port_t) && !defined(IN_PORT_T_DEFINED)
typedef uint16_t in_port_t;
#endif

struct hostent
{
    char  *h_name;            /* Official name of the host. */
    char **h_aliases;         /* A pointer to an array of pointers to alternative host names, */
                              /* terminated by a null pointer. */
    int    h_addrtype;        /* Address type. */
    int    h_length;          /* The length, in bytes, of the address. */
    char **h_addr_list;       /* A pointer to an array of pointers to network addresses */
                              /* (in network byte order) for the host,terminated by a null pointer */
#define h_addr h_addr_list[0] /* for backward compatibility */
};

struct sockaddr
{
    uint8_t     sa_len;
    sa_family_t sa_family;
    char        sa_data[14];
};

struct sockaddr_in
{
    uint8_t        sin_len;
    sa_family_t    sin_family;
    in_port_t      sin_port;
    struct in_addr sin_addr;
#define SIN_ZERO_LEN 8
    char           sin_zero[SIN_ZERO_LEN];
};

#ifdef MOLINK_USING_IPV6
struct sockaddr_in6 {
  uint8_t         sin6_len;      /* length of this structure    */
  sa_family_t     sin6_family;   /* AF_INET6                    */
  in_port_t       sin6_port;     /* Transport layer port #      */
  uint32_t        sin6_flowinfo; /* IPv6 flow information       */
  struct in6_addr sin6_addr;     /* IPv6 address                */
  uint32_t        sin6_scope_id; /* Set of interfaces for scope */
};
#endif

struct addrinfo
{
    int              ai_flags;     /* Input flags. */
    int              ai_family;    /* Address family of socket. */
    int              ai_socktype;  /* Socket type. */
    int              ai_protocol;  /* Protocol of socket. */
    socklen_t        ai_addrlen;   /* Length of socket address. */
    struct sockaddr *ai_addr;      /* Socket address of socket. */
    char            *ai_canonname; /* Canonical name of service location. */
    struct addrinfo *ai_next;      /* Pointer to next in list. */
};

struct sockaddr_storage
{
    uint8_t     s2_len;
    sa_family_t ss_family;
    char        s2_data1[2];
    uint32_t    s2_data2[3];
};

typedef struct ip_mreq
{
    struct in_addr imr_multiaddr; /* IP multicast address of group */
    struct in_addr imr_interface; /* local IP address of interface */
} ip_mreq;

#ifdef OS_USING_IO_MULTIPLEXING
typedef struct poll_req
{
    struct vfs_pollfd *req;
    os_slist_node_t    req_list;
} poll_req_t;
#endif /* OS_USING_IO_MULTIPLEXING */

typedef struct mo_sock
{
    mo_netconn_t *netconn;

    void       *lastdata;       /* data that was left from the previous read */
    os_size_t   lastlen;        /* data length that was left from the previous read */
    os_uint16_t lastoffset;     /* offset in the data that was left from the previous read */
    os_int32_t  recv_timeout;   /* timeout to wait for received data in milliseconds */

    os_int8_t   rcvevent;       /* number of times data was received, set by event_callback(), 
                                   tested by the receive and select functions */
    os_uint8_t  sendevent;      /* number of times data was ACKed (free send buffer), 
                                   set by event_callback(),tested by select */
    os_uint8_t  errevent;       /* error happened for this socket, set by event_callback(), tested by select */
    os_uint32_t select_waiting; /* counter of how many threads are waiting for this socket using select */
#ifdef OS_USING_IO_MULTIPLEXING
    os_slist_node_t req_slist_head;
#endif
} mo_sock_t;

#if 0
/* FD_SET used for mo_select */
#ifndef FD_SET
#undef  FD_SETSIZE
/* Make FD_SETSIZE match NUM_SOCKETS in socket.c */
#define FD_SETSIZE    MOLINK_SOCKETS_FD_MAX

typedef unsigned long       fd_mask;
#define NBBY                (8) /* number of bits in a byte */
#define NFDBITS             (sizeof(fd_mask) * NBBY) /* bits per mask */
#define __FD_MASK(n)        ((fd_mask)(1UL << ((n) % NFDBITS)))

#ifndef HOWMANY
#define HOWMANY(x,y)        (((x)+((y)-1))/(y))
#endif /* HOWMANY */

#define FDSAFESET(n, code)  do {if (((n) < MOLINK_SOCKETS_FD_MAX) && (((int)(n)) >= 0)) { code; }} while(0)
#define FDSAFEGET(n, code)  (((n) < MOLINK_SOCKETS_FD_MAX) && (((int)(n)) >= 0) ? (code) : 0)

#define FD_SET(n, p)  FDSAFESET(n, (p)->fd_bits[(n)/NFDBITS] |=  __FD_MASK(n))
#define FD_CLR(n, p)  FDSAFESET(n, (p)->fd_bits[(n)/NFDBITS] &= ~__FD_MASK(n))
#define FD_ISSET(n,p) FDSAFEGET(n, (p)->fd_bits[(n)/NFDBITS] &   __FD_MASK(n))
#define FD_ZERO(p)    memset((void*)(p), 0, sizeof(*(p)))

typedef struct fd_set
{
  fd_mask fd_bits[HOWMANY(FD_SETSIZE, NFDBITS)];
} fd_set;

#elif FD_SETSIZE < MOLINK_SOCKETS_FD_MAX
#error "external FD_SETSIZE too small for number of sockets"
#else
#endif /* FD_SET */
#endif

#if MOLINK_SOCKETS_FD_MAX < MOLINK_NUM_SOCKETS
#error "sockets fd max config error"
#endif

int mo_socket(mo_object_t *module, int domain, int type, int protocol);
int mo_closesocket(mo_object_t *module, int socket);
int mo_bind(mo_object_t *module, int socket, const struct sockaddr *name, socklen_t namelen);
int mo_bind_with_cb(mo_object_t *module, int socket, const struct sockaddr *name, socklen_t namelen, mo_netconn_data_callback cb);
int mo_connect(mo_object_t *module, int socket, const struct sockaddr *name, socklen_t namelen);
int mo_sendto(mo_object_t           *module,
              int                    socket,
              const void            *data,
              size_t                 size,
              int                    flags,
              const struct sockaddr *to,
              socklen_t              tolen);
int mo_send(mo_object_t *module, int socket, const void *data, size_t size, int flags);
int mo_recvfrom(mo_object_t     *module,
                int              socket,
                void            *mem,
                size_t           len,
                int              flags,
                struct sockaddr *from,
                socklen_t       *fromlen);
int mo_recv(mo_object_t *module, int socket, void *mem, size_t len, int flags);
int mo_getsockopt(mo_object_t *module, int socket, int level, int optname, void *optval, socklen_t *optlen);
int mo_setsockopt(mo_object_t *module, int socket, int level, int optname, const void *optval, socklen_t optlen);
struct hostent *mo_gethostbyname(mo_object_t *module, const char *name);
int  mo_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
void mo_freeaddrinfo(struct addrinfo *ai);
int  mo_select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);

#ifdef OS_USING_IO_MULTIPLEXING
int mo_poll(int socket, struct vfs_pollfd *req, os_bool_t poll_setup);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_SOCKETS_OPS */

#endif /* __MO_SOCKETS_H__ */
