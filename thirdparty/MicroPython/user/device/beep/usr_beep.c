#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include <drv_gpio.h>
#include "user_beep.h"



static int beep_open(void *dev)
{
    device_info_t *device = (device_info_t *)dev;
    os_pin_mode(device->id, PIN_MODE_OUTPUT);
    return 0;
}

static int beep_close(void *dev)
{
    os_pin_write(((device_info_t *) dev)->id, PIN_LOW);
    return 0;
}

static int beep_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *device = (device_info_t *)dev;
    if (NULL == device)
    {
        mp_raise_ValueError("device is NULL \n");
        return -1;
    }

    switch(cmd)
    {    
    case MP_MACHINE_OP_OPEN:
        beep_open(dev);
        break;
    case MP_MACHINE_OP_CLOSE:
        beep_close(dev);
        break;
    case  MP_MACHINE_OP_DISABLE:
        os_pin_write(device->id, PIN_LOW);
        break;
    case  MP_MACHINE_OP_ENABLE:
        os_pin_write(device->id, PIN_HIGH);
        break;
    default:
     mp_raise_ValueError("Internal command error, please check!\n");
     return MP_ERROR;
    }
    return MP_EOK;
}

struct operate beep_ops = {
    .ioctl = beep_ioctl,
};

static int beep_register(void)
{
	device_info_t * beep = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == beep)
	{
		os_kprintf("mpycall_beep_register malloc mem faibeep!");
		return -1;
	}
    memset(beep, 0, sizeof(device_info_t));
	

	beep->owner.name = "beep";
	beep->owner.type = DEV_BEEP;
	
	beep->ops = &beep_ops;
	

	mpycall_device_add(beep);

	return 0;
}

OS_CMPOENT_INIT(beep_register, OS_INIT_SUBLEVEL_LOW);




