#include "oneos_config.h"
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "os_assert.h"
#include "shell.h"
#include "os_errno.h"
#include "device.h"
#include "os_task.h"
#include "dlog.h"
#include "os_mq.h"
#include "os_memory.h"

#include "iotjs.h"
#include "jerryscript-port.h"
#include "iotjs_debuglog.h"

enum
{
  OP_OPEN_FILE = 110,
  OP_WRITE_BYTES,
  OP_CLOSE_FILE,
  OP_EXIT,
  OP_MAX
};

enum
{
  STAT_WAIT_LEN = 120,
  STAT_RECEIVE_LEN,
  STAT_RECEIVE_PAYLOAD,
  STAT_MAX
};

enum
{
  MSG_TYPE_OPEN_FILE = 130,
  MSG_TYPE_CLOSE_FILE,
  MSG_TYPE_FILE_DATA,
  MSG_TYPE_EXIT,
  MSG_TYPE_MAX
};

struct rz_work_t
{
  uint32_t cmd;
  union
  {
    char *file_name;
    struct
    {
      uint16_t len;
      uint8_t *buf;
    } data;
  } u;
};

struct packet
{
  uint16_t len;
  uint8_t *data;
};

struct message
{
  uint8_t type;
  uint8_t data[1];
};

static os_task_t *s_task;
static os_mq_t *s_mq;
static os_device_t *s_dev;

static char s_magic[4];
static const char MAGIC[4] = {0x98, 0x89, 0x67, 0x45};

#if 0
static char trace[2048];
static int trace_idx = 0;
#endif

static uint16_t uint16_encode(uint8_t *u)
{
  return (uint16_t)(u[1] << 8 | u[0]);
}

static void work_task(void *param)
{
  int fd;
  os_size_t max_len;
  os_err_t ret;
  struct rz_work_t work;

  while (1)
  {
    ret = os_mq_recv(s_mq, &work, sizeof(struct rz_work_t), OS_WAIT_FOREVER, &max_len);

    if (ret == OS_EOK)
    {
      switch (work.cmd)
      {
      case OP_OPEN_FILE:
        OS_ASSERT(work.u.file_name != OS_NULL);
        int fd = open(work.u.file_name, O_RDWR | O_CREAT);
        os_free(work.u.file_name);
        OS_ASSERT(fd > 0);
        break;

      case OP_WRITE_BYTES:
        write(fd, work.u.data.buf, work.u.data.len);
        os_free(work.u.data.buf);
        break;

      case OP_CLOSE_FILE:
        close(fd);
        break;

      case OP_EXIT:
        os_mq_destroy(s_mq);
        os_device_close(s_dev);
        return;

      default:
        OS_ASSERT(0);
        break;
      }
    }
  }
}

static void start_work(void)
{
#if 0
  trace_idx = 0;
#endif
  s_task = os_task_create("rz_work", work_task, NULL, 2048, OS_TASK_PRIORITY_MAX - 2);
  s_mq = os_mq_create("rz_mq", sizeof(struct rz_work_t), 32);
  s_dev = os_device_find(OS_CONSOLE_DEVICE_NAME);

  OS_ASSERT(s_task != OS_NULL);
  OS_ASSERT(s_mq != OS_NULL);
  OS_ASSERT(s_dev != OS_NULL);

  os_task_startup(s_task);
  os_device_open(s_dev);
}

static struct packet receive_packet(void)
{
  char ch;
  struct packet pkt;
  char lenstr[2];
  int rx_cnt = 0;
  int rx_idx = 0;
  int m_idx = 0;
  int state = STAT_WAIT_LEN;

  while (1)
  {
    switch (state)
    {
    case STAT_WAIT_LEN:
      rx_cnt = os_device_read_block(s_dev, 0, &ch, 1);
      OS_ASSERT(rx_cnt > 0);
      if (ch == MAGIC[m_idx])
        m_idx++;
      else
        m_idx = 0;
      if (4 == m_idx)
        state = STAT_RECEIVE_LEN;
      break;

    case STAT_RECEIVE_LEN:
      rx_cnt = os_device_read_block(s_dev, 0, &ch, 1);
      OS_ASSERT(rx_cnt > 0);
      lenstr[rx_idx++] = ch;
      if (rx_idx == 2)
      {
        state = STAT_RECEIVE_PAYLOAD;
        pkt.len = uint16_encode(lenstr);
        pkt.data = NULL;
        if (0 == pkt.len)
        {
          os_uint16_t unused = os_mq_get_unused_entry_count(s_mq);
          os_device_write_nonblock(s_dev, 0, &unused, 2);
          return pkt;
        }
        pkt.data = os_malloc(pkt.len);
        OS_ASSERT(pkt.data != OS_NULL);
        rx_idx = 0;
      }
      break;

    case STAT_RECEIVE_PAYLOAD:
      rx_cnt = os_device_read_block(s_dev, 0, &pkt.data[rx_idx], pkt.len - rx_idx);
      OS_ASSERT(rx_cnt > 0);
      rx_idx += rx_cnt;
      if (rx_idx == pkt.len)
      {
        os_uint16_t unused = os_mq_get_unused_entry_count(s_mq);
        os_device_write_nonblock(s_dev, 0, &unused, 2);
        return pkt;
      }
      break;

    default:
      OS_ASSERT(0);
      break;
    }
  }
}

static os_err_t sh_recvfiles(os_int32_t argc, char **argv)
{
  os_err_t ret;
  struct rz_work_t work;
  struct packet pkt;
  struct message * msg;

  start_work();

#if 0
  trace_idx += snprintf(&trace[trace_idx], sizeof(trace) - trace_idx,
                      "<1>start_work\r\n");
#endif

  while (1)
  {
    pkt = receive_packet();
    msg = (struct message *)pkt.data;

    switch(msg->type)
    {
      case MSG_TYPE_OPEN_FILE:
        work.cmd = OP_OPEN_FILE;
        work.u.file_name = os_malloc(pkt.len);
        memcpy(work.u.file_name, msg->data, pkt.len - 1);
        work.u.file_name[pkt.len - 1] = '\0';
        ret = os_mq_send(s_mq, &work, sizeof(struct rz_work_t), OS_NO_WAIT);
        OS_ASSERT(ret == OS_EOK);
        break;

      case MSG_TYPE_FILE_DATA:
        work.cmd = OP_WRITE_BYTES;
        work.u.data.len = pkt.len - 1;
        work.u.data.buf = msg->data;
        ret = os_mq_send(s_mq, &work, sizeof(struct rz_work_t), OS_NO_WAIT);
        OS_ASSERT(ret == OS_EOK);
        break;

      case MSG_TYPE_CLOSE_FILE:
        work.cmd = OP_CLOSE_FILE;
        ret = os_mq_send(s_mq, &work, sizeof(struct rz_work_t), OS_NO_WAIT);
        OS_ASSERT(ret == OS_EOK);
        break;

      case MSG_TYPE_EXIT:
        work.cmd = OP_EXIT;
        ret = os_mq_send(s_mq, &work, sizeof(struct rz_work_t), OS_NO_WAIT);
        os_task_msleep(1000);
#if 0
        for (int i = 0; i < trace_idx; i++)
          os_kprintf("%c", trace[i]);
        os_kprintf("\r\n");
#endif
        return 0;

      default:
        break;
    }
  }

  return 0;
}
SH_CMD_EXPORT(recvfiles, sh_recvfiles, "receive file");
