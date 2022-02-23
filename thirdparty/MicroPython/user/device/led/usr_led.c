#include <stdio.h>
#include <string.h>
#include <os_memory.h>
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "usr_led.h"
#include "drv_gpio.h"

static int led_ioctl(void *dev, int cmd, void *arg)
{
    device_info_t *led = dev;
    if (!led)
    {
        mp_raise_ValueError("device is NULL \n");
        return MP_ERROR;
    }

    switch (cmd)
    {
    case MP_MACHINE_OP_INIT:
        os_pin_mode(led->id, PIN_MODE_OUTPUT);
        os_pin_write(led->id, !PIN_LOW);
        break;
    case MP_MACHINE_OP_ENABLE:
        os_pin_write(led->id, PIN_LOW);
        break;
    case MP_MACHINE_OP_DEINIT:
    case MP_MACHINE_OP_DISABLE:
        os_pin_write(led->id, !PIN_LOW);
        break;
    default:
        mp_raise_ValueError("the cmd is wrong, please check!\n");
        return MP_ERROR;
    }
    
    return MP_EOK;
}

static struct operate led_ops = {
    .ioctl = led_ioctl,
};

static int usr_led_register(void)
{
    device_info_t * led = (device_info_t *)os_malloc(sizeof(device_info_t));

    if(NULL == led)
    {
        os_kprintf("mpycall_led_register malloc mem failed!");
        return -1;
    }
    memset(led, 0, sizeof(device_info_t));


    led->owner.name = "LED";
    led->owner.type = DEV_LED;

    led->ops = &led_ops;


    mpycall_device_add(led);

    return 0;
}

OS_CMPOENT_INIT(usr_led_register, OS_INIT_SUBLEVEL_LOW);




