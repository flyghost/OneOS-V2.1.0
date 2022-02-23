/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        usr_dac.c
 *
 * @brief       The driver file for dac.
 *
 * @revision
 * Date         Author          		Notes
 * 2020-08-13   MicroPYthon Team      First Version
 * 
 * @history 
 * Date         Author          		Modification
 * 2020-12-01	chow				  Replace dac api to device api
 ***********************************************************************************************************************
 */

#include "py/runtime.h"

#if (MICROPY_PY_MACHINE_DAC)
#include <device.h>
#include "usr_dac.h"
#include "usr_misc.h"
#include "dac.h"
#include "usr_general.h"


static int dac_write(const char *dev_name, uint32_t channel, void *buf, uint32_t bufsize)
{
    return mpy_usr_driver_write(dev_name, channel, buf, bufsize, MP_USR_DRIVER_WRITE_NONBLOCK);
}

static int dac_ioctl(void *device, int cmd, void* arg)
{
    os_device_t *dac_device = os_device_find(((device_info_t *)device)->owner.name);
    if (!dac_device)
    {
        mp_err("Couldn't find dac device !!! \n");
        return MP_MACHINE_OP_ERROR;
    }
    //mp_log("name: %s, channel:%d", dac_device->name, arg);
    switch (cmd) {
        case MP_MACHINE_OP_DISABLE:
        {
            os_device_control(dac_device, OS_DAC_CMD_DISABLE, arg);
            break;
        }
        case MP_MACHINE_OP_ENABLE:
        { 
            os_device_control(dac_device, OS_DAC_CMD_ENABLE, arg);
            break;
        }
    }

    return MP_MACHINE_OP_EOK;
}

static struct operate dac_ops = {
    .open = mpy_usr_driver_open,
    .write = dac_write,
    .ioctl = dac_ioctl,
};


static int dac_register(void)
{
    MP_SIMILAR_DEVICE_REGISTER(MICROPYTHON_MACHINE_DAC_PRENAME, DEV_BUS, &dac_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(dac_register, OS_INIT_SUBLEVEL_LOW);

#endif


