#ifndef _SYS_SELECT_H__
#define _SYS_SELECT_H__

#include <oneos_config.h>
#include <sys/time.h>
#include <string.h>

#if !defined(PRESET_FD_SETSIZE)
#define FD_SETSIZE      64                  /* default set FD_SETSIZE */
#else
#define FD_SETSIZE      PRESET_FD_SETSIZE
#endif

#define NBBY            8                   /* number of bits in a byte */

typedef unsigned long   fd_mask;
#define NFDBITS (sizeof (fd_mask) * NBBY)   /* bits per mask */
#ifndef howmany
#define howmany(x,y)    (((x)+((y)-1))/(y))
#endif

/* We use a macro for fd_set so that including Sockets.h afterwards
   can work.  */
typedef	struct _types_fd_set {
	fd_mask	fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} _types_fd_set;

#define fd_set          _types_fd_set

#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((void*)(p), 0, sizeof(*(p)))

extern int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);

#endif /* _SYS_SELECT_H__ */

