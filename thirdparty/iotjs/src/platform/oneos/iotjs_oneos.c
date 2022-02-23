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
#include "os_task.h"
#include <dlog.h>
#include <fal.h>
#include <vfs_fs.h>

#include "iotjs.h"
#include "jerryscript-port.h"
#include "iotjs_debuglog.h"
#include "uv.h"
#include "iotjs_util.h"
#include "iotjs_env.h"

typedef struct _arg_type
{
  char **val;
  int count;
} arg_type;

static arg_type s_args;
static os_task_t *s_iotjs_task = NULL;

static void free_args(arg_type *arg)
{
  if (arg->count > 0)
  {
    for (int i = 0; i < arg->count; i++)
      os_free(arg->val[i]);

    os_free(arg->val);
    arg->count = 0;
    arg->val = NULL;
  }
}

static int store_args(int argc, char **argv, arg_type *d_arg)
{
  d_arg->val = os_malloc(sizeof(char *) * argc);
  if (d_arg->val == NULL)
    return OS_ERROR;

  for (int i = 0; i < argc; i++)
  {
    d_arg->val[i] = os_malloc(strlen(argv[i]) + 1);
    if (d_arg->val[i] == OS_NULL)
    {
      for (int j = 0; j < i; j++)
      {
        os_free(d_arg->val[j]);
      }
      os_free(d_arg->val);
      return OS_ERROR;
    }
    strcpy(d_arg->val[i], argv[i]);
  }
  d_arg->count = argc;
  return OS_EOK;
}

static void iotjs_task_entry(void *param)
{
  arg_type *args = (arg_type *)param;

  iotjs_entry(args->count, args->val);

  free_args(args);
  s_iotjs_task = NULL;
}

static os_err_t sh_iotjs(os_int32_t argc, char **argv)
{
  if (s_iotjs_task != NULL)
    os_kprintf("There is already a iotjs instance running");
  else
  {
    store_args(argc, argv, &s_args);
#ifdef SHELL_TASK_PRIORITY
    s_iotjs_task = os_task_create("js-task", iotjs_task_entry,
                    &s_args, 16384 * 4, SHELL_TASK_PRIORITY + 1);
#else
    s_iotjs_task = os_task_create("js-task", iotjs_task_entry,
                    &s_args, 16384 * 4, 10);
#endif
    if (!s_iotjs_task)
    {
      os_kprintf("Failed to create iotjs task ! \n");
      return OS_ERROR;
    }
    os_task_startup(s_iotjs_task);
  }

  return OS_EOK;
}
SH_CMD_EXPORT(iotjs, sh_iotjs, "iotjs");

#if defined(BOARD_ATK_APOLLO)

#define MAIN_TAG        "MAIN"

#ifdef OS_USING_VFS_FATFS

#define OS_FS_PART_NAME	"filesystem"

static void mount_fatfs(void)
{
    if (fal_blk_device_create(OS_FS_PART_NAME))
    {
        LOG_W(MAIN_TAG, "Create a block device on the %s partition of flash successful.", OS_FS_PART_NAME);
    }
    else
    {
        LOG_E(MAIN_TAG, "Can't create a block device on '%s' partition.", OS_FS_PART_NAME);
    }

    if (vfs_mount(OS_FS_PART_NAME, "/", "fat", 0, 0) == 0)
    {
        LOG_W(MAIN_TAG, "FAT filesystem mount successful.");
    }
    else
    {
        LOG_E(MAIN_TAG, "FAT filesystem mount fail.");
        LOG_E(MAIN_TAG, "You should mkfs first, then reset board ! cmd: mkfs -t fat %s", OS_FS_PART_NAME);
    }
}
#endif

int mount_fat_fs(void)
{
    #if defined(OS_USING_VFS_FATFS)
        mount_fatfs();
    #endif
    return OS_EOK;
}
OS_CMPOENT_INIT(mount_fat_fs, OS_INIT_SUBLEVEL_LOW);

#endif