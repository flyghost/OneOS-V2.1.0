/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <string.h>
#include <device.h>
#include <block/block_device.h>
#include "ffconf.h"
#include "ff.h"

static os_device_t *disk[_VOLUMES] = {0};
#include "diskio.h"		/* FatFs lower layer API */




/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
    os_size_t result;
    os_device_t *device = disk[pdrv];

    result = os_device_read_nonblock(device, sector, buff, count);
    if (result == count)
    {
        return RES_OK;
    }

    return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    os_size_t result;
    os_device_t *device = disk[pdrv];

    result = os_device_write_nonblock(device, sector, buff, count);
    if (result == count)
    {
        return RES_OK;
    }

    return RES_ERROR;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    os_device_t *device = disk[pdrv];

    if (device == OS_NULL)
        return RES_ERROR;

    if (cmd == GET_SECTOR_COUNT)
    {
        struct os_blk_geometry geometry;

        memset(&geometry, 0, sizeof(geometry));
        os_device_control(device, OS_DEVICE_CTRL_BLK_GETGEOME, &geometry);

        *(DWORD *)buff = geometry.capacity / geometry.block_size;
        if (*(DWORD *)buff == 0)
            return RES_ERROR;
    }
    else if (cmd == GET_SECTOR_SIZE)
    {
        struct os_blk_geometry geometry;

        memset(&geometry, 0, sizeof(geometry));
        os_device_control(device, OS_DEVICE_CTRL_BLK_GETGEOME, &geometry);

        *(WORD *)buff = (WORD)(geometry.block_size);
    }
    else if (cmd == GET_BLOCK_SIZE) /* Get erase block size in unit of sectors (DWORD) */
    {
        struct os_blk_geometry geometry;

        memset(&geometry, 0, sizeof(geometry));
        os_device_control(device, OS_DEVICE_CTRL_BLK_GETGEOME, &geometry);

        *(DWORD *)buff = 1;
    }
    else if (cmd == CTRL_SYNC)
    {
        os_device_control(device, OS_DEVICE_CTRL_BLK_SYNC, OS_NULL);
    }
    else if (cmd == CTRL_TRIM)
    {
        /* not need .*/
    }

    return RES_OK;
}


DRESULT disk_set_devid(int disk_id, os_device_t * device_id)
{
	if (disk_id >= _VOLUMES)
	{
		return RES_ERROR;
	}
		
    disk[disk_id] = device_id;
	
	return RES_OK;
}

os_device_t *disk_get_devid(int disk_id)
{
	if (disk_id >= _VOLUMES) 
	{
		return OS_NULL;
	}

    return disk[disk_id];
}
