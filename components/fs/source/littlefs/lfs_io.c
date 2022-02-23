#include <fal/fal.h>
#include "lfs.h"
#include "lfs_io.h"

int lfs_flash_read(const struct lfs_config *cfg,
				   lfs_block_t				block,
				   lfs_off_t				off,
				   void 				   *buffer,
				   lfs_size_t				size)
{
	uint32_t		   len;
	struct fal_part   *fal_part;
	uint32_t		   part_off;

	LFS_ASSERT(cfg != NULL);
	LFS_ASSERT(cfg->context != NULL);
	LFS_ASSERT(buffer != NULL);

	fal_part = (struct fal_part *)((lfs_device_info_t *)cfg->context)->dev;
	part_off = block * cfg->block_size + (uint32_t)off;

	len = fal_part_read(fal_part, part_off, buffer, size);

	LFS_ASSERT(len == size);
	
	if (len != size)
	{
		return LFS_ERR_IO;
	}

	return LFS_ERR_OK;
}

int lfs_flash_write(const struct lfs_config *cfg,
					lfs_block_t 			 block,
					lfs_off_t				 off,
					const void				*buffer,
					lfs_size_t				 size)
{
	uint32_t		 len;
	struct fal_part *fal_part;
	uint32_t		 part_off;

	LFS_ASSERT(cfg != NULL);
	LFS_ASSERT(cfg->context != NULL);
	LFS_ASSERT(buffer != NULL);

	fal_part = (struct fal_part *)((lfs_device_info_t *)cfg->context)->dev;
	part_off = block * cfg->block_size + (uint32_t)off;

	len = fal_part_write(fal_part, part_off, buffer, size);

	LFS_ASSERT(len == size);
	
	if (len != size)
	{
		return LFS_ERR_IO;
	}
	/*
	else if (len == xx)
		return LFS_ERR_CORRUPT;   //LFS_ERR_CORRUPT has not implement. to do
	*/
	
	return LFS_ERR_OK;
}

int lfs_flash_erase(const struct lfs_config *cfg, lfs_block_t block)
{
	int 			  len;
	struct fal_part * fal_part;
	uint32_t		  part_off;

	LFS_ASSERT(cfg != NULL);
	LFS_ASSERT(cfg->context != NULL);

	fal_part = (struct fal_part *)((lfs_device_info_t *)cfg->context)->dev;
	part_off = block * cfg->block_size;

	len = fal_part_erase(fal_part, part_off, (size_t)cfg->block_size);

	LFS_ASSERT(len == cfg->block_size);
	
	if (len != cfg->block_size)
	{
		return LFS_ERR_IO;
	}
	/*
	else if (len == xx)
		return LFS_ERR_CORRUPT;   //LFS_ERR_CORRUPT has not implement. to do
	*/

	return LFS_ERR_OK;
}

int lfs_flash_erase_all(os_device_t *dev_id)
{
	int 		ret;
	fal_part_t *fal_part;

	fal_part = fal_part_find(device_name(dev_id));
	if (NULL == fal_part)
	{
		return LFS_ERR_IO;
	}

	ret = fal_part_erase_all(fal_part);
	if (ret < 0)
	{
		return LFS_ERR_IO;
	}

	return LFS_ERR_OK;
}


int lfs_flash_sync(const struct lfs_config *cfg)
{
	(void)cfg;
	return LFS_ERR_OK;
}

#ifdef LFS_THREADSAFE
int lfs_flash_lock(const struct lfs_config *cfg)
{
	int 		ret;
	os_mutex_t *dev_lock;

	LFS_ASSERT(cfg != NULL);
	LFS_ASSERT(cfg->context != NULL);

	dev_lock = (os_mutex_t *)(&((lfs_device_info_t *)cfg->context)->dev_lock);
	
	ret = os_mutex_lock(dev_lock, OS_WAIT_FOREVER);

	return (OS_EOK == ret) ? LFS_ERR_OK : LFS_ERR_IO;
}

int lfs_flash_unlock(const struct lfs_config *cfg)
{
	int 		ret;
	os_mutex_t *dev_lock;

	LFS_ASSERT(cfg != NULL);
	LFS_ASSERT(cfg->context != NULL);

	dev_lock = (os_mutex_t *)(&((lfs_device_info_t *)cfg->context)->dev_lock);

	ret = os_mutex_unlock(dev_lock);

	return (OS_EOK == ret) ? LFS_ERR_OK : LFS_ERR_IO;
}
#endif

const struct lfs_device_ops g_lfs_dev_ops = {
	.read	   = lfs_flash_read,
	.prog	   = lfs_flash_write,
	.erase	   = lfs_flash_erase,
	.erase_all = lfs_flash_erase_all,
	.sync	   = lfs_flash_sync,
#ifdef LFS_THREADSAFE
	.lock	   = lfs_flash_lock,
	.unlock    = lfs_flash_unlock,
#endif
};

