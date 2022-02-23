#include "usr_pin.h"
#include <stdlib.h>
#include "usr_misc.h"
#include "py/runtime.h"



#ifdef MICROPY_PY_MACHINE_PIN
#include <pin.h>
#include <drv_gpio.h>
#include <os_memory.h>
#include <string.h>

os_base_t gpio_list[] ={
	#if defined(GPIOA_BASE)
	GPIOA_BASE,
	#endif
	#if defined(GPIOB_BASE)
	GPIOB_BASE,
	#endif
	#if defined(GPIOC_BASE)
	GPIOC_BASE,
	#endif
	#if defined(GPIOD_BASE)
	GPIOD_BASE,
	#endif
	#if defined(GPIOE_BASE)
	GPIOE_BASE,
	#endif
	#if defined(GPIOF_BASE)
	GPIOF_BASE,
	#endif
	#if defined(GPIOG_BASE)
	GPIOG_BASE,
	#endif
	#if defined(GPIOH_BASE)
	GPIOH_BASE,
	#endif
};

//static void mpycall_pin_mode(mp_base_t pin, mp_base_t mode)
//{
//	os_pin_mode(pin, mode);
//}

i32_t mpycall_pin_attach_irq(i32_t pin, u32_t mode, void (*hdr)(void *args), void *args)
{
	return os_pin_attach_irq(pin, mode, hdr, args);
}
     
//i32_t mpycall_pin_irq_enable(mp_base_t pin, u32_t enable)
//{
//    return os_pin_irq_enable(pin, enable);
//}

static int pin_read(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    return os_pin_read((os_base_t)offset);
}

static int pin_write(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    os_pin_write((os_base_t)offset, *((os_base_t*)buf));
    return 0;
}

static int pin_ctrl(void *device, int cmd, void *arg)
{
    machine_pin_obj_t * self = device;
    uint32_t *mode = (uint32_t *)arg;
    
    switch (cmd)
    {
    case MP_PIN_OP_GET_PIN_NUM:
        return mp_pin_get_num(device, arg);
    case MP_PIN_OP_SET_MODE:
        os_pin_mode(self->pin, *mode);
        break;
    case MP_PIN_OP_IRQ_EN:
        os_pin_irq_enable(self->pin, PIN_IRQ_ENABLE);
        break;
    case MP_PIN_OP_IRQ_DIS:
        os_pin_detach_irq(self->pin);
        os_pin_irq_enable(self->pin, PIN_IRQ_DISABLE);
        break;
    }

    return 0;
}

/**
*********************************************************************************************************
*                                      ��ȡgpio�����к�
*
* @description: This function will get the index of gpio.
*
* @param      : device:         device
*
*				mesg:			the information of gpio,  such as ['A', 13]
*
* @returns    : the index of gpio (the index is created by os)
*********************************************************************************************************
*/
int mp_pin_get_num(void *device, void *mesg)
{
	char *data = (char *)mesg;
	int group_index = data[0];
	if (group_index > 'H' || group_index < 'A'){
		mp_err("parameters is wrong!");
		return -1;
	} else {
		group_index -= 'A';
	}

	return (os_base_t)((16 * (((os_base_t)(gpio_list[group_index]) - (os_base_t)GPIOA_BASE) / (0x0400UL))) + (int)data[1]);
}

static struct operate usr_pin_ops = {
    .read = pin_read,
    .write = pin_write,
    .ioctl = pin_ctrl,
};

static int pin_register(void)
{
    MP_SINGLE_DEVICE_REGISTER(pin, DEV_BUS, &usr_pin_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(pin_register, OS_INIT_SUBLEVEL_LOW);

#endif // MICROPY_PY_MACHINE_PIN


