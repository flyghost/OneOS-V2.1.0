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

#include <assert.h>
#include <string.h>
#include <time.h>
#include <os_clock.h>

#include <uv.h>
#include "unix/internal.h"

//-----------------------------------------------------------------------------
// loop
int uv__platform_loop_init(uv_loop_t* loop) {
  loop->npollfds = 0;
  return 0;
}

void uv__platform_loop_delete(uv_loop_t* loop) {
  loop->npollfds = 0;
}


//-----------------------------------------------------------------------------

void uv__platform_invalidate_fd(uv_loop_t* loop, int fd) {
  int i;
  int nfd = loop->npollfds;
  for (i = 0; i < nfd; ++i) {
    struct pollfd* pfd = &loop->pollfds[i];
    if (fd == pfd->fd) {
      pfd->fd = -1;
    }
  }
}

//-----------------------------------------------------------------------------

/* should not clear handle structure with memset as
 * *data can be set before calling init function */

//-----------------------------------------------------------------------------

ssize_t readv(int fd, const struct iovec* iiovec, int count) {
  ssize_t result = 0;
  ssize_t total = 0;
  int idx;

  for (idx = 0; idx < count; ++idx) {
    result = read(fd, iiovec[idx].iov_base, iiovec[idx].iov_len);
    if (result < 0) {
      return result;
    } else {
      total += result;
    }
  }
  return total;
}


ssize_t writev(int fd, const struct iovec* iiovec, int count) {
  ssize_t result = 0;
  ssize_t total = 0;
  int idx;

  for (idx = 0; idx < count; ++idx) {
    result = write(fd, iiovec[idx].iov_base, iiovec[idx].iov_len);
    if (result < 0) {
      return result;
    } else {
      total += result;
    }
  }
  return total;
}

uint64_t uv__hrtime(uv_clocktype_t type) {
  uint32_t ms_per_tick = 1000 / OS_TICK_PER_SECOND;
  return ((uint64_t) os_tick_get() * ms_per_tick) * ((uint64_t) 1e6);
}

static void uv__add_pollfd(uv_loop_t* loop, struct pollfd* pe) {
  int i;
  int exist = 0;
  int free_idx = -1;
  for (i = 0; i < loop->npollfds; ++i) {
    struct pollfd* cur = &loop->pollfds[i];
    if (cur->fd == pe->fd) {
      cur->events = pe->events;
      exist = 1;
      break;
    }
    if (cur->fd == -1) {
      free_idx = i;
    }
  }
  if (!exist) {
    if (free_idx == -1) {
      free_idx = loop->npollfds++;
      if (free_idx >= TUV_POLL_EVENTS_SIZE)
      {
        TDLOG("uv__add_pollfd abort, because loop->npollfds (%d) reached maximum size", free_idx);
        ABORT();
      }
    }
    struct pollfd* cur = &loop->pollfds[free_idx];

    cur->fd = pe->fd;
    cur->events = pe->events;
    cur->revents = 0;
    // cur->sem = 0;
    // cur->priv = 0;
  }
}


static void uv__rem_pollfd(uv_loop_t* loop, struct pollfd* pe) {
  int i = 0;
  while (i < loop->npollfds) {
    struct pollfd* cur = &loop->pollfds[i];
    if (cur->fd == pe->fd) {
      *cur = loop->pollfds[--loop->npollfds];
    } else {
      ++i;
    }
  }
}


void uv__io_poll(uv_loop_t* loop, int timeout) {
  struct pollfd pfd;
  struct pollfd* pe;
  QUEUE* q;
  uv__io_t* w;
  uint64_t base;
  uint64_t diff;
  int nevents;
  int count;
  int nfd;
  int i;

  if (loop->nfds == 0) {
    assert(QUEUE_EMPTY(&loop->watcher_queue));
    return;
  }

  while (!QUEUE_EMPTY(&loop->watcher_queue)) {
    q = QUEUE_HEAD(&loop->watcher_queue);
    QUEUE_REMOVE(q);
    QUEUE_INIT(q);

    w = QUEUE_DATA(q, uv__io_t, watcher_queue);
    assert(w->pevents != 0);
    assert(w->fd >= 0);
    assert(w->fd < (int)loop->nwatchers);

    pfd.fd = w->fd;
    pfd.events = w->pevents;
    uv__add_pollfd(loop, &pfd);

    w->events = w->pevents;
  }

  assert(timeout >= -1);
  base = loop->time;
  count = 5;

  for (;;) {
    nfd = poll(loop->pollfds, loop->npollfds, timeout);

    SAVE_ERRNO(uv__update_time(loop));

    if (nfd == 0) {
      assert(timeout != -1);
      return;
    }

    if (nfd == -1) {
      int err = 0;
      if (err != EINTR) {
        // poll of which the watchers is null should be removed
        TDLOG("uv__io_poll abort for errno(%d)", err);
        goto handle_poll;
      }
      if (timeout == -1) {
        continue;
      }
      if (timeout == 0) {
        return;
      }
      goto update_timeout;
    }

handle_poll:
    nevents = 0;

    for (i = 0; i < loop->npollfds; ++i) {
      pe = &loop->pollfds[i];
      w = loop->watchers[pe->fd];

      if (w == NULL) {
        uv__rem_pollfd(loop, pe);
        --i;
        continue;
      }

      if (pe->fd >= 0 && (pe->revents & (POLLIN | POLLOUT | POLLHUP))) {    
        w->cb(loop, w, pe->revents);
        ++nevents;
      }
    }

    if (nevents != 0) {
      if (--count != 0) {
        timeout = 0;
        continue;
      }
      return;
    }
    if (timeout == 0) {
      return;
    }
    if (timeout == -1) {
      continue;
    }
update_timeout:
    assert(timeout > 0);

    diff = loop->time - base;
    if (diff >= (uint64_t)timeout) {
      return;
    }
    timeout -= diff;
  }
}
