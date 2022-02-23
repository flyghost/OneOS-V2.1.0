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
#include <os_mutex.h>

#ifdef OS_USING_IO_MULTIPLEXING

#include <vfs_devfs.h>

#define EVENT_DEV_NAME "event"
#define SOCKET_DEV_PATH_NAME "/dev/event"

typedef enum
{
  DUMMY
} IOCTL_CMD;

typedef struct _eventfs_private_t
{
  uint64_t event;
  int      has_event;
  os_mutex_t *mutex;
} eventfs_private_t;

extern struct vfs_file *fd_to_fp(int fd);

static int eventfs_adapter_open(struct vfs_file *file);
static int eventfs_adapter_close(struct vfs_file *file);
static int eventfs_adapter_read(struct vfs_file *file, off_t pos, void *buf, size_t count);
static int eventfs_adapter_write(struct vfs_file *file, off_t pos, const void *buf, size_t count);
static int eventfs_adapter_ioctl(struct vfs_file *file, unsigned long cmd, void *args);
static int eventfs_adapter_poll(struct vfs_file *file, struct vfs_pollfd *req, os_bool_t poll_setup);

static os_list_node_t s_eventfs_poll_list;

static const struct dev_file_ops gs_eventfs_fops = {
  eventfs_adapter_open,
  eventfs_adapter_close,
  eventfs_adapter_read,
  eventfs_adapter_write,
  eventfs_adapter_ioctl,
  eventfs_adapter_poll,
};

static int eventfs_adapter_open(struct vfs_file *file)
{
  OS_ASSERT(file != OS_NULL);
  OS_ASSERT(file->type == FT_DEVICE);

  file->private = os_malloc(sizeof(eventfs_private_t));
  OS_ASSERT(file->private != OS_NULL);

  eventfs_private_t *epriv = (eventfs_private_t *)file->private;
  epriv->event = 0;
  epriv->has_event = 0;
  epriv->mutex = os_mutex_create("eventfs", 0);

  return OS_EOK;
}

static int eventfs_adapter_close(struct vfs_file *file)
{
  OS_ASSERT(file != OS_NULL);
  OS_ASSERT(file->private != OS_NULL);
  OS_ASSERT(file->type == FT_DEVICE);

  eventfs_private_t *epriv = (eventfs_private_t *)file->private;

  os_free(file->private);
  os_mutex_destroy(epriv->mutex);

  return OS_EOK;
}

static int eventfs_adapter_read(struct vfs_file *file, off_t pos, void *buf, size_t count)
{
  OS_ASSERT(file != OS_NULL);
  OS_ASSERT(file->private != OS_NULL);
  OS_ASSERT(file->type == FT_DEVICE);

  size_t size = 0;

  eventfs_private_t *epriv = (eventfs_private_t *)file->private;

  os_mutex_lock(epriv->mutex, OS_TICK_MAX);

  if (epriv->has_event != 0) {
    size = sizeof(epriv->event) < count ? sizeof(epriv->event) : count;
    memcpy(buf, &epriv->event, size);
    epriv->has_event = 0;
  }

  os_mutex_unlock(epriv->mutex);

  return size;
}

static int eventfs_adapter_write(struct vfs_file *file, off_t pos, const void *buf, size_t count)
{
  OS_ASSERT(file != OS_NULL);
  OS_ASSERT(file->private != OS_NULL);
  OS_ASSERT(file->type == FT_DEVICE);

  eventfs_private_t *epriv = (eventfs_private_t *)file->private;

  os_mutex_lock(epriv->mutex, OS_TICK_MAX);

  size_t size = sizeof(epriv->event) < count ? sizeof(epriv->event) : count;
  uint8_t * src = (uint8_t *)buf;
  uint8_t * dst = (uint8_t *)&epriv->event;
  for (size_t i = 0; i < size; i++) {
    dst[i] |= src[i];
  }
  epriv->has_event = 1;

  struct vfs_pollfd *fds;
  struct vfs_pollfd *tmp;

  os_list_for_each_entry_safe(fds, tmp, &s_eventfs_poll_list, struct vfs_pollfd, node) {
    struct vfs_file * tmp_file = fd_to_fp(fds->fd);
    eventfs_private_t *tmp_epriv = (eventfs_private_t *)tmp_file->private;
    if (tmp_epriv->has_event != 0)
      devfs_poll_notify(fds, POLLIN);
  }

  os_mutex_unlock(epriv->mutex);

  return size;
}

static int eventfs_adapter_ioctl(struct vfs_file *file, unsigned long cmd, void *args)
{
  OS_ASSERT(file != OS_NULL);
  OS_ASSERT(file->type == FT_DEVICE);

  return OS_EOK;
}

static int dev_fops_poll_init(struct vfs_file *file, struct vfs_pollfd *fds)
{
  OS_ASSERT(file != OS_NULL);
  OS_ASSERT(file->private != OS_NULL);
  eventfs_private_t *epriv = (eventfs_private_t *)file->private;

  if (fds->events & POLLIN) {
    os_mutex_lock(epriv->mutex, OS_TICK_MAX);

    os_list_add_tail(&s_eventfs_poll_list, &fds->node);

    if (epriv->has_event != 0)
      devfs_poll_notify(fds, POLLIN);

    os_mutex_unlock(epriv->mutex);
  }

  return fds->events;
}

static int dev_fops_poll_deinit(struct vfs_file *file, struct vfs_pollfd *fds)
{
  eventfs_private_t *epriv = (eventfs_private_t *)file->private;

  os_mutex_lock(epriv->mutex, OS_TICK_MAX);

  os_list_del(&fds->node);

  os_mutex_unlock(epriv->mutex);

  return 0;
}

static int eventfs_adapter_poll(struct vfs_file *file, struct vfs_pollfd *req, os_bool_t poll_setup)
{
  OS_ASSERT(file != OS_NULL);
  OS_ASSERT(file->type == FT_DEVICE);
  OS_ASSERT(req != OS_NULL);

  if (poll_setup == OS_TRUE)
    return dev_fops_poll_init(file, req);
  else
    return dev_fops_poll_deinit(file, req);
}

int event_init(void)
{
  os_list_init(&s_eventfs_poll_list);
  return devfs_register_device(EVENT_DEV_NAME, (struct dev_file_ops *)&gs_eventfs_fops, OS_NULL);
}

OS_CMPOENT_INIT(event_init, OS_INIT_SUBLEVEL_LOW);
#endif
