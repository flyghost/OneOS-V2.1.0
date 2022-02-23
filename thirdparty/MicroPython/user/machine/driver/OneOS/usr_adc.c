#include "usr_adc.h"

#if MICROPY_PY_MACHINE_ADC
#include <drv_cfg.h>
#include "usr_misc.h"
#include "usr_general.h"
#include "py/runtime.h"



static int adc_read(const char *dev_name, uint32_t channel, void *buf, uint32_t bufsize)
{
    if (mpy_usr_driver_read( dev_name, channel, buf, bufsize, MP_USR_DRIVER_READ_NONBLOCK) != bufsize)
    {
        mp_err("adc read failed %d.");
    }

    return MP_MACHINE_OP_EOK;
}

static int adc_ioctl(void *device, int cmd, void* arg)
{
    os_device_t *adc_device = os_device_find(((device_info_t *)device)->owner.name);
    if (! adc_device)
    {
        mp_err("adc device cannot found!");
        return MP_MACHINE_OP_ERROR;
    }
    switch (cmd) 
    {
//        case MP_MACHINE_OP_OPEN:
//        {
//            os_device_open(adc_device);
//            break;
//        }
    case MP_MACHINE_OP_DISABLE:
    {
        (void)os_device_control(adc_device, OS_ADC_CMD_DISABLE, OS_NULL); 
        break;
    }
    case MP_MACHINE_OP_ENABLE:
    {
        if (os_device_control(adc_device, OS_ADC_CMD_ENABLE, OS_NULL) != OS_EOK)
        {
            mp_err("adc device cannot open!");
            return MP_MACHINE_OP_ERROR;
        }
        break;
    }
    }

    return MP_MACHINE_OP_EOK;
}

static struct operate adc_ops = {
    .open = mpy_usr_driver_open,
    .read = adc_read,
    .ioctl = adc_ioctl,
};

static int adc_register(void)
{
    MP_SIMILAR_DEVICE_REGISTER(MICROPYTHON_MACHINE_ADC_PRENAME, DEV_BUS, &adc_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(adc_register, OS_INIT_SUBLEVEL_LOW);
#endif

