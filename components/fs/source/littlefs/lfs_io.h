#ifndef LFS_IO_H
#define LFS_IO_H

#include <device.h>
#include <os_mutex.h>
#include "lfs.h"

#ifdef __cplusplus
extern "C"
{
#endif

struct lfs_device_ops
{
    int (*read)(const struct lfs_config *c, lfs_block_t block,lfs_off_t off,void *buffer, lfs_size_t size);
    int (*prog)(const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size);
    int (*erase)(const struct lfs_config *c, lfs_block_t block);
    int (*erase_all)(os_device_t *dev_id);
    int (*sync)(const struct lfs_config *c);
#ifdef LFS_THREADSAFE
    int (*lock)(const struct lfs_config *c);
    int (*unlock)(const struct lfs_config *c);
#endif
};

struct lfs_device_info
{
    void      *dev;
    os_mutex_t dev_lock;
};

typedef struct lfs_device_info lfs_device_info_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

