/****************************************************************************
 *
 *   Copyright (C) 2008-2009, 2012-2013 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <gnutt@nuttx.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

#include <oneos_config.h>
#include <sys/errno.h>
#include <string.h>
#include <poll.h>
#include <os_assert.h>
#include <os_memory.h>
#include <vfs_select.h>
#include <vfs_poll.h>
#include "vfs_private.h"

int vfs_select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    struct vfs_pollfd *pollset = NULL;
    int fd;
    int npfds;
    int msec;
    int ndx;
    int ret;

    OS_ASSERT(nfds <= FD_SETSIZE);
    /* How many pollfd structures do we need to allocate? */

    /* Initialize the descriptor list for poll() */
    ret = 0;
    for (fd = 0, npfds = 0; fd < nfds; fd++)
    {
      /* Check if any monitor operation is requested on this fd */

        if ((readfds   && FD_ISSET(fd, readfds))  ||
          (writefds  && FD_ISSET(fd, writefds)) ||
          (exceptfds && FD_ISSET(fd, exceptfds)))
        {
          /* Yes.. increment the count of pollfds structures needed */

          npfds++;
        }
    }

    /* Allocate the descriptor list for poll() */

    if (npfds > 0)
    {
        pollset = (struct vfs_pollfd *)os_malloc(npfds * sizeof(struct vfs_pollfd));
        if (pollset)
        {
            memset(pollset, 0, npfds * sizeof(struct vfs_pollfd));
        }
        else
        {
            VFS_SET_ERRNO(-ENOMEM);
            ret = -1;
        }
    }
    else
    {
        VFS_SET_ERRNO(-EINVAL);
        ret = -1;
    }

    /* Initialize the descriptor list for poll() */
    if (0 == ret)
    {
        for (fd = 0, ndx = 0; fd < nfds; fd++)
        {
            int incr = 0;

            /* The readfs set holds the set of FDs that the caller can be assured
            * of reading from without blocking.  Note that POLLHUP is included as
            * a read-able condition.  POLLHUP will be reported at the end-of-file
            * or when a connection is lost.  In either case, the read() can then
            * be performed without blocking.
            */
            if (readfds && FD_ISSET(fd, readfds))
            {
              pollset[ndx].fd      = fd;
              pollset[ndx].events |= POLLIN;
              incr                 = 1;
            }

            /* The writefds set holds the set of FDs that the caller can be assured
            * of writing to without blocking.
            */
            if (writefds && FD_ISSET(fd, writefds))
            {
              pollset[ndx].fd      = fd;
              pollset[ndx].events |= POLLOUT;
              incr                 = 1;
            }

            /* The exceptfds set holds the set of FDs that are watched for exceptions */
            if (exceptfds && FD_ISSET(fd, exceptfds))
            {
              pollset[ndx].fd      = fd;
              incr                  = 1;
            }

            ndx += incr;
        }
    }

    OS_ASSERT(ndx == npfds);

    /* Convert the timeout to milliseconds */
    if (timeout)
    {
        /* Calculate the timeout in milliseconds */
        msec = timeout->tv_sec * 1000 + timeout->tv_usec / 1000;
    }
    else
    {
        /* Any negative value of msec means no timeout */
        msec = -1;
    }

    /* Then let poll do all of the real work. */
    ret = vfs_poll(pollset, npfds, msec);

    /* Now set up the return values */
    if (readfds)
    {
        memset(readfds, 0, sizeof(fd_set));
    }

    if (writefds)
    {
        memset(writefds, 0, sizeof(fd_set));
    }

    if (exceptfds)
    {
        memset(exceptfds, 0, sizeof(fd_set));
    }

    /* Convert the poll descriptor list back into selects 3 bitsets */
    if (ret > 0)
    {
        ret = 0;
        for (ndx = 0; ndx < npfds; ndx++)
        {
            /* Check for read conditions.  Note that POLLHUP is included as a
            * read condition.  POLLHUP will be reported when no more data will
            * be available (such as when a connection is lost).  In either
            * case, the read() can then be performed without blocking.
            */

            if (readfds)
            {
                if (pollset[ndx].revents & (POLLIN | POLLHUP))
                {
                  FD_SET(pollset[ndx].fd, readfds);
                  ret++;
                }
            }

            /* Check for write conditions */
            if (writefds)
            {
                if (pollset[ndx].revents & POLLOUT)
                {
                  FD_SET(pollset[ndx].fd, writefds);
                  ret++;
                }
            }

            /* Check for exceptions */
            if (exceptfds)
            {
                if (pollset[ndx].revents & POLLERR)
                {
                  FD_SET(pollset[ndx].fd, exceptfds);
                  ret++;
                }
            }
        }
    }

    if (pollset)
    {
        os_free(pollset);
    }

    return ret;
}

