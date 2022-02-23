/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "usr_misc.h"



#if MICROPY_PY_MACHINE_WDT
#include <string.h>
#include <device.h>
#include <os_memory.h>
#include "usr_wdt.h"
#include "watchdog.h"
#include "usr_general.h"

void middle_wdt_print(mp_wdt_device_handler * wdt_device, const mp_print_t *print)
{
    mp_printf(print, "wdt device port name: iwdg");
}

static int wdt_start(void)
{
    os_device_t *dev_wdt = NULL;
    
    dev_wdt = os_device_find(WDT_NAME);
    if (NULL == dev_wdt)
    {
        mp_err("Couldn't find watchdog! Please initialized it");
        return -1;
    }

    if (os_device_control(dev_wdt, OS_DEVICE_CTRL_WDT_START, OS_NULL) != OS_EOK)
    {
        mp_err("start iwdg failed!");
        return -1;
    }
    return 0;
}

static int wdt_refresh(void)
{
    os_device_t *dev_wdt = NULL;
    
    dev_wdt = os_device_find(WDT_NAME);
    if (NULL == dev_wdt)
    {
        mp_err("Couldn't find watchdog! Please initialized it");
        return -1;
    }


    if (os_device_control(dev_wdt, OS_DEVICE_CTRL_WDT_KEEPALIVE, OS_NULL) != OS_EOK)
    {
        mp_err("start iwdg failed!");
        return -1;
    }
    
    return 0;
}

static int watchdog_ioctl(void *dev, int cmd, void *arg)
{
    switch (cmd) 
    {
    case  MP_MACHINE_OP_ENABLE:
        return wdt_start();
    case  MP_MACHINE_OP_OPEN:
        return wdt_refresh();
    default:
        mp_err("the cmd is wrong, please check!");
        return MP_ERROR;
    }
}

static struct operate wdt_ops = {
    .open =  mpy_usr_driver_open,
    .close = mpy_usr_driver_close,
    .ioctl = watchdog_ioctl,
};

static int wdt_register(void)
{
    MP_SINGLE_DEVICE_REGISTER(wdt, DEV_WDT, &wdt_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(wdt_register, OS_INIT_SUBLEVEL_LOW);



#endif
