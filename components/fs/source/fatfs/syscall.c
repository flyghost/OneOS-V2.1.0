/*------------------------------------------------------------------------*/
/* Sample code of OS dependent controls for FatFs                         */
/* (C)ChaN, 2014                                                          */
/*------------------------------------------------------------------------*/

#include <os_util.h>
#include <os_memory.h>
#include <os_errno.h>
//#include <sys/time.h>
#include <string.h>
#include "ff.h"

DWORD get_fattime(void)
{
    DWORD fat_time = 0;

#if 0
//#ifdef OS_USING_LIBC_ADAPTER 
    time_t now;
    struct tm *p_tm;
    struct tm tm_now;

    /* get current time */
    now = time(OS_NULL);

    /* lock scheduler. */
    os_schedule_lock();
    /* converts calendar time time into local time. */
    p_tm = localtime(&now);
    /* copy the statically located variable */
    memcpy(&tm_now, p_tm, sizeof(struct tm));
    /* unlock scheduler. */
    os_schedule_unlock();

    fat_time =  (DWORD)(tm_now.tm_year - 80) << 25 |
                (DWORD)(tm_now.tm_mon + 1)   << 21 |
                (DWORD)tm_now.tm_mday        << 16 |
                (DWORD)tm_now.tm_hour        << 11 |
                (DWORD)tm_now.tm_min         <<  5 |
                (DWORD)tm_now.tm_sec / 2 ;
#endif /* OS_USING_LIBC_ADAPTER  */

    return fat_time;
}

#if _FS_REENTRANT
/*------------------------------------------------------------------------*/
/* Create a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/*  This function is called in f_mount() function to create a new
   synchronization object, such as semaphore and mutex. When a 0 is returned,
   the f_mount() function fails with FR_INT_ERR.
*/

int ff_cre_syncobj (	/* 1:Function succeeded, 0:Could not create the sync object */
	BYTE vol,			/* Corresponding volume (logical drive number) */
	_SYNC_t *sobj		/* Pointer to return the created sync object */
)
{
    char name[8];
    _SYNC_t mutex;

    os_snprintf(name, sizeof(name), "fat%d", vol);
    mutex = os_mutex_create(name, OS_FALSE);
    if (mutex != OS_NULL)
    {
        *sobj = mutex;
        return OS_TRUE;
    }

    return OS_FALSE;
}



/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* 1:Function succeeded, 0:Could not delete due to any error */
	_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
    if (sobj != OS_NULL)
        os_mutex_destroy(sobj);

    return OS_TRUE;
}



/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* 1:Got a grant to access the volume, 0:Could not get a grant */
	_SYNC_t sobj	/* Sync object to wait */
)
{
    if (os_mutex_lock(sobj, _FS_TIMEOUT) == OS_EOK)
        return OS_TRUE;

    return OS_FALSE;
}



/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	_SYNC_t sobj	/* Sync object to be signaled */
)
{
	os_mutex_unlock(sobj);
}

#endif




#if _USE_LFN == 3	/* LFN with a working buffer on the heap */
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/
/* If a NULL is returned, the file function fails with FR_NOT_ENOUGH_CORE.
*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block */
	UINT msize		/* Number of bytes to allocate */
)
{
	return os_malloc(msize);
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
	void* mblock	/* Pointer to the memory block to free */
)
{
	os_free(mblock);
}

#endif
