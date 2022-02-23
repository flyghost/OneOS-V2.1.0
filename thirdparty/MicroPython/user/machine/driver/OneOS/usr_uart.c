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
#if (MICROPY_PY_MACHINE_UART)
#include "usr_misc.h"
#include "usr_uart.h"
#include "device.h"
#include "usr_general.h"
#include <os_clock.h>


int uart_timeout = 1;


static int uart_read(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    int num = uart_timeout * OS_TICK_PER_SECOND / 1000;
    uint32_t tick_start = os_tick_get();
    
    int readlen = bufsize;
    unsigned char *ptr = buf;
    os_device_t *uart_device = NULL;
    
    uart_device = os_device_find(dev_name);
    if (!uart_device) 
    {
        mp_err("Find UART device failed!");
        return MP_ERROR;
    }

    do
    {
        if(os_device_read_nonblock(uart_device, 0, ptr++, 1) == 0)
        {
            os_task_tsleep(1);
            if (-1 == uart_timeout)
            {
                continue;
            }
            if (os_tick_get() - tick_start >= num)
            {
                break;
            }
        }
        else
        {
            bufsize--;
        }

    } while(bufsize);

    return (readlen - bufsize);
}

static int uart_write(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    return mpy_usr_driver_write(dev_name, offset, buf, bufsize, MP_USR_DRIVER_WRITE_NONBLOCK);
}

static int uart_set_cfg(os_device_t *uart_device, int cmd, void* arg)
{
    struct  serial_configure config = OS_SERIAL_CONFIG_DEFAULT;

    if (cmd == MP_UART_SET_USR_CFG)
    {
        uart_timeout = ((middle_uart_config_t *)(arg))->timeout;
        config.baud_rate = ((middle_uart_config_t *)(arg))->baud_rate;
        config.stop_bits = ((middle_uart_config_t *)(arg))->stop_bits;
        config.parity = ((middle_uart_config_t *)(arg))->parity;
        config.data_bits = ((middle_uart_config_t *)(arg))->data_bits;
    } 

    return os_device_control(uart_device, OS_DEVICE_CTRL_CONFIG, &config);
}

static int uart_ctrl(void *device, int cmd, void* arg)
{
    os_device_t *uart_device = NULL;
    
    uart_device = os_device_find(((device_info_t *)device)->owner.name);
    if (!uart_device) 
    {
        mp_err("Find UART device failed! ");
        return MP_ERROR;
    }

    switch (cmd)
    {
    case MP_UART_SET_DEAULT_CFG:
    case MP_UART_SET_USR_CFG:
        uart_set_cfg(uart_device, cmd, arg);
        break;
    }

    return MP_EOK;
}

STATIC struct operate usr_uart_ops = {
    .open = mpy_usr_driver_open,
    .read = uart_read,
    .write = uart_write,
    .ioctl = uart_ctrl,
    .close = mpy_usr_driver_close,
};

static int uart_register(void)
{
    MP_SIMILAR_DEVICE_REGISTER(MICROPYTHON_MACHINE_UART_PRENAME, DEV_BUS, &usr_uart_ops);
    return MP_EOK;
}

OS_DEVICE_INIT(uart_register, OS_INIT_SUBLEVEL_LOW);

#endif // MICROPY_PY_MACHINE_UART

