#include <stdio.h>
#include <string.h>
#include <os_memory.h>

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"

#include "usr_key.h"
#include "drv_gpio.h"
#include "pin.h"


static int key_set_pin(usr_pin_obj_t *pin)
{
    os_pin_mode(pin->pin, pin->mode);
    return 0;
}


static int key_callback(usr_key_obj_t *self, fun_0_1_t pin_callback)
{
	os_pin_attach_irq(self->pin.pin, self->pin.irq_mode, pin_callback, (void *)self);
	os_pin_irq_enable(self->pin.pin, PIN_IRQ_ENABLE);
	return 0;
}


static int key_ioctl(void *dev, int cmd, void *arg)
{
	device_info_t *self = dev;
	device_key_obj_t *key = arg;
	
	if (!self)
	{
		mp_raise_ValueError("device is NULL \n");
		return MP_ERROR;
	}
	
	switch(cmd)
    {
        case MP_MACHINE_OP_INIT:
			//self->open_flag = MP_MACHINE_INIT_FLAG;
            break;
		case MP_MACHINE_OP_DEINIT:
			os_pin_detach_irq(key->key.pin.pin);
			os_pin_irq_enable(key->key.pin.pin, PIN_IRQ_DISABLE);
			break;
        case MP_MACHINE_OP_SET_PARAM:
			return key_set_pin(&(key->key.pin));
		
		case MP_MACHINE_OP_CALLBACK:{
			return key_callback(&key->key, (fun_0_1_t)key->isr_handler);
		}
        default:{
            mp_raise_ValueError("the cmd is wrong, please check!\n");
            return MP_ERROR;
		}
    }
	return MP_EOK;
}

static struct operate key_ops = {
    .ioctl = key_ioctl,
};

int mpycall_key_register(void)
{
	device_info_t * key = (device_info_t *)os_malloc(sizeof(device_info_t));
	
	if(NULL == key)
	{
		os_kprintf("mpycall_key_register malloc mem failed!");
		return -1;
	}
    memset(key, 0, sizeof(device_info_t));
	

	key->owner.name = "KEY";
	key->owner.type = DEV_KEY;
	
	key->ops = &key_ops;
    
	mpycall_device_add(key);

	return 0;
}

OS_CMPOENT_INIT(mpycall_key_register, OS_INIT_SUBLEVEL_LOW);







