#ifndef __ASM_SEMAPHORE_H__
#define __ASM_SEMAPHORE_H__

#define CONFIG_JFFS2_SEMAPHORE  0 // no mutex, 1 use static, 2 use dynamic
#if CONFIG_JFFS2_SEMAPHORE == 0
//#include <cyg/hal/drv_api.h>

struct semaphore {
	int x;
};

#define DECLARE_MUTEX(x)
#define DECLARE_MUTEX_LOCKED(x)

#define init_MUTEX(sem)
#define init_MUTEX_LOCKED(sem)
#define down(sem)
#define down_interruptible(sem)  0
#define down_trylock(sem)
#define up(sem)

#elif CONFIG_JFFS2_SEMAPHORE == 1
#include <os_kernel.h>

struct semaphore {
	struct os_mutex mutex;
};

#define DECLARE_MUTEX(x)
#define DECLARE_MUTEX_LOCKED(x)
inline void init_MUTEX(struct semaphore * sem)
{
   if (os_mutex_init((os_mutex_t)sem, "mutex", OS_IPC_FLAG_FIFO, OS_FALSE) == OS_EOK)
   {
	   /* detach the object from system object container */
	   os_object_deinit(&(((os_mutex_t*)sem)->parent.parent));
	   return;
   }
   os_kprintf("get an error at %s:%d \n",  __FUNCTION__, __LINE__);
   OS_ASSERT(0);
}

inline void init_MUTEX_LOCKED(struct semaphore * sem)
{
   os_enter_critical();
   if (os_mutex_init((os_mutex_t*)sem, "mutex", OS_IPC_FLAG_FIFO) == OS_EOK)
   {
	   /* detach the object from system object container */
	   os_object_deinit(&(((os_mutex_t *)sem)->parent.parent));
	   os_exit_critical();
	   os_mutex_lock((os_mutex_t *)sem, OS_WAIT_FOREVER);
	   return;
   }
   os_exit_critical();

   os_kprintf("get an error at %s:%d \n",  __FUNCTION__, __LINE__);
   OS_ASSERT(0);
}

inline down(struct semaphore * sem)
{
	os_mutex_lock((os_mutex_t *)sem, OS_WAIT_FOREVER);
}
inline int down_interruptible(struct semaphore* sem)
{
	os_mutex_lock((os_mutex_t *)sem, OS_WAIT_FOREVER);
    return 0;
}
inline up(struct semaphore * sem)
{
	os_mutex_unlock((os_mutex_t *)sem);
}
#elif CONFIG_JFFS2_SEMAPHORE == 2

#include <os_kernel.h>

struct semaphore {
	 os_mutex_t mutex;
};

#define DECLARE_MUTEX(x)
#define DECLARE_MUTEX_LOCKED(x)

inline void init_MUTEX(struct semaphore * sem)
{
	sem->mutex = os_mutex_create("mutex", OS_IPC_FLAG_FIFO, OS_FALSE);
}
inline init_MUTEX_LOCKED(struct semaphore * sem)
{
	sem->mutex = os_mutex_create("mutex", OS_IPC_FLAG_FIFO, OS_FALSE);
	os_mutex_lock(sem->mutex,  OS_WAIT_FOREVER);
}
inline down(struct semaphore * sem)
{
	os_mutex_lock(sem->mutex,  OS_WAIT_FOREVER);
}
inline int down_interruptible(struct semaphore* sem)
{
	os_mutex_lock(sem->mutex,  OS_WAIT_FOREVER);
    return 0;
}
/*
Attempt to lock the mutex pointed to by the mutex argument without waiting. 
If the mutex is already locked by some other thread then this function 
returns FALSE. If the function can lock the mutex without waiting, then 
TRUE is returned. 
void cyg_drv_mutex_unlock( cyg_drv_mutex *mutex )
*/

//#define down_trylock(struct semaphore * sem)  os_mutex_lock((os_mutex_t*)sem,  OS_IPC_WAITING_NO)
inline up(struct semaphore * sem)
{
	os_mutex_unlock(sem->mutex);
}

#else
#error "CONFIG_JFFS2_SEMAPHORE should be 0, 1 or 2"
#endif

#endif /* __ASM_SEMAPHORE_H__ */
