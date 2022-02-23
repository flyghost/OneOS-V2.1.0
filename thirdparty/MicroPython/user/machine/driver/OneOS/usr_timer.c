#include <string.h>
#include "usr_misc.h"
#include "py/runtime.h"

#include "os_task.h"
#include "os_clock.h"
#include "os_timer.h"
#include "usr_timer.h"
#include "py/mpthread.h"
#include <os_memory.h>
#include <os_sem.h>
#include <os_errno.h>

mp_uint_t mp_hal_ticks_us(void) {
	return os_tick_get() * 1000000UL / OS_TICK_PER_SECOND;
}

mp_uint_t mp_hal_ticks_ms(void) {
	return os_tick_get() * 1000 / OS_TICK_PER_SECOND;
}

mp_uint_t mp_hal_ticks_cpu(void) {
	return os_tick_get();
}

void mp_hal_delay_us(mp_uint_t us) 
{
    os_tick_t t_len = us * OS_TICK_PER_SECOND / 1000000L;
    os_uint32_t u_len = us % (1000000L / OS_TICK_PER_SECOND);
    mp_hal_delay_ms(t_len * 1000 / OS_TICK_PER_SECOND);
    extern void os_hw_us_delay(os_uint32_t delay_us);
    MP_THREAD_GIL_EXIT();
    os_hw_us_delay(u_len);
    MP_THREAD_GIL_ENTER();
    MICROPY_EVENT_POLL_HOOK;
}

void mp_hal_delay_ms(mp_uint_t ms) 
{
    os_tick_t t_len = ms * OS_TICK_PER_SECOND / 1000L;
    os_tick_t t_end;
    void *root_th = NULL;
    
    if(t_len < 1)
    {
        return;
    }
    extern void *mpy_get_main_thd_id(void);
    root_th = mpy_get_main_thd_id();
    if (root_th == os_task_self())
    {
        t_end = os_tick_get() + t_len;
        MP_STATE_VM(sleep_sem).count = 0;
        while (1) 
        {
            MP_THREAD_GIL_EXIT();
            if (OS_ETIMEOUT == os_sem_wait(&MP_STATE_VM(sleep_sem), t_len))
            {
                MP_THREAD_GIL_ENTER();
                break;
            }
            MP_THREAD_GIL_ENTER();
            MICROPY_EVENT_POLL_HOOK;
            t_len = t_end - os_tick_get();
            if (t_len >= (OS_TICK_MAX / 2))
            {
                break;
            }
        }
    }
    else
    {
        MP_THREAD_GIL_EXIT();
        os_task_tsleep(t_len);
        MP_THREAD_GIL_ENTER();
    }
}

#if (MICROPY_PY_MACHINE_TIMER) 


static int timer_enable(void *device, uint16_t flag)
{
    device_info_t *timer =  ((machine_timer_obj_t *)device)->timer;
    return os_timer_start((os_timer_t *)(timer->other));
}

static int timer_disable(void *device)
{
    device_info_t *timer =  ((machine_timer_obj_t *)device)->timer;
    os_timer_stop((os_timer_t *)(timer->other));
    return MP_MACHINE_OP_EOK;
}


static uint8_t timer_get_os_timer_flag(uint8_t mpy_flag)
{
    uint8_t flag = 0;

    if (mpy_flag == MP_TIMER_FLAG_PERIODIC) 
    {
        flag = OS_TIMER_FLAG_PERIODIC;
    } 
    else if (mpy_flag == MP_TIMER_FLAG_ONE_SHOT ) 
    {
        flag = OS_TIMER_FLAG_ONE_SHOT;
    }

    return flag;
}

static int timer_ctrl(void *device, int cmd, void* arg)
{
    machine_timer_obj_t *dev = device;
    uint8_t flag = 0;
    int32_t ret = OS_ERROR;
    
    switch (cmd)
    {
    case MP_MACHINE_OP_ENABLE:
        ret = timer_enable(device, 0);
        break;
    case MP_MACHINE_OP_DISABLE:
        ret = timer_disable(device);
        break;
    case MP_MACHINE_OP_CREATE:
        flag = timer_get_os_timer_flag(dev->timer->owner.flag);
        mp_log("Create timer{dev_name:%s, timeout:%d, flag:%d.}.",
                dev->dev_name, dev->timeout, flag);
        dev->timer->other = os_timer_create(dev->dev_name, (fun_0_1_t)arg, dev, dev->timeout, flag);
        if (NULL != dev->timer->other)
        {
            ret = OS_EOK;
        }
        break;
    case MP_MACHINE_OP_DELETE:
        ret = os_timer_destroy(dev->timer->other);
        break;
    }
    
    return ret;
}

STATIC struct operate usr_timer_ops = {
    .ioctl = timer_ctrl,
};

static int timer_register(void)
{
    MP_SINGLE_DEVICE_REGISTER(timer, DEV_TIMER, &usr_timer_ops);
    return MP_EOK;
}
OS_DEVICE_INIT(timer_register, OS_INIT_SUBLEVEL_LOW);

#endif

