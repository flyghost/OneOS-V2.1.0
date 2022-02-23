#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include "typedef.h"
#include "flash_pub.h"
#include "os_types.h"

int beken_flash_read(os_uint32_t address, void *data, os_uint32_t size);
int beken_flash_write(os_uint32_t address, const void *data, os_uint32_t size);
int beken_flash_erase(os_uint32_t address);
int beken_flash_erase_with_len(os_uint32_t address, os_uint32_t size);
#endif
