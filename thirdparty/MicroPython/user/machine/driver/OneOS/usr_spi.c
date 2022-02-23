/*
 * This file is paos of the MicroPython project, http://micropython.org/
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
 * all copies or substantial poosions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PAosICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TOos OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include "py/runtime.h"
#include <os_stddef.h>

#if (MICROPY_PY_MACHINE_SPI)
#include "modmachine.h"
#include "usr_misc.h"
#include "spi.h"
#include "usr_spi.h"
#include <device.h>
#include "usr_general.h"

static int spi_read(const char *spi_dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    struct os_spi_device *spi_device  = NULL;
    
    spi_device = (struct os_spi_device *)os_device_find(spi_dev_name);
    if(NULL == spi_device)
    {
        mp_err("spi_read find name error.");
        return -1;
    }

    return os_spi_transfer(spi_device, NULL, buf, bufsize);
}

static int spi_write(const char *spi_dev_name, uint32_t offset, void *buf, uint32_t bufsize)
{
    struct os_spi_device *spi_device  = NULL;
    
    spi_device = (struct os_spi_device *)os_device_find(spi_dev_name);
    if (NULL == spi_device)
    {
        mp_err("spi_write find name error.");
        return -1;
    }

    return os_spi_transfer(spi_device, buf, NULL, bufsize);
}

static int spi_config(char *spi_dev_name, void* arg)
{
    struct os_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.max_hz = 40000;
    int mode = MP_SPI_MSB;
    struct os_spi_device *spi_device  = NULL;
    
    spi_device = (struct os_spi_device *)os_device_find(spi_dev_name);
    if (spi_device == NULL)
    {
        mp_err("test_spi find device error\n");
        return -1;
    }

    if(arg != NULL)
    {
      cfg.max_hz = ((int*)arg)[0];
      cfg.data_width = ((int*)arg)[1];
      mode = ((int*)arg)[2];
    }

    if((mode &(~MP_SPI_MASK)) == MP_SPI_MSB)
    {
      mode &= MP_SPI_MASK;
      if(mode == MP_SPI_MODE_0)
      {
          cfg.mode = OS_SPI_MODE_0 | OS_SPI_MSB;
      }
      else if(mode == MP_SPI_MODE_1)
      {
          cfg.mode = OS_SPI_MODE_1 | OS_SPI_MSB;
      }
      else if(mode == MP_SPI_MODE_2)
      {
          cfg.mode = OS_SPI_MODE_2 | OS_SPI_MSB;
      }
      else if(mode == MP_SPI_MODE_3)
      {
          cfg.mode = OS_SPI_MODE_2 | OS_SPI_MSB;
      }
      else
      {
          return -1;
      }
    }
    else
    {
      mode &= MP_SPI_MASK;
      if(mode == MP_SPI_MODE_0)
      {
          cfg.mode = OS_SPI_MODE_0 | OS_SPI_LSB;
      }
      else if(mode == MP_SPI_MODE_1)
      {
          cfg.mode = OS_SPI_MODE_1 | OS_SPI_LSB;
      }
      else if(mode == MP_SPI_MODE_2)
      {
          cfg.mode = OS_SPI_MODE_2 | OS_SPI_LSB;
      }
      else if(mode == MP_SPI_MODE_3)
      {
          cfg.mode = OS_SPI_MODE_2 | OS_SPI_LSB;
      }
      else
      {
          return -1;
      }
    }

    
    os_spi_configure(spi_device, &cfg);

    return 0;

}

static int spi_attach_dev(machine_spi_obj_t *self)
{
    int ret = MP_ERROR;
    struct os_spi_device *spi_device  = NULL;

    mp_log("Attach spi dev: %s.", self->spi_dev_name);
    spi_device = (struct os_spi_device *)os_device_find(self->spi_dev_name);
    if (NULL != spi_device)
    {
        mp_log("SPI device[%s] has already attached.", self->spi_dev_name);
        return 0;
    }

    ret = os_hw_spi_device_attach(self->spi_bus->owner.name, self->spi_dev_name, self->cs_pin_num);
    mp_log("Attach spi device[%s] to bus[%s] through pin[%d], result[%d].", 
                self->spi_dev_name, self->spi_bus->owner.name, self->cs_pin_num, ret);
    return ret;
}

static int spi_send_recv(char *spi_dev_name , machine_spi_dev_msg_buf_t *msg_buf)
{
    struct os_spi_device *spi_device = NULL;
    
    spi_device = (struct os_spi_device *)os_device_find(spi_dev_name);
    if (spi_device == NULL)
    {
      mp_err("test_spi find device error\n");
      return -1;
    }
    
    return os_spi_transfer(spi_device, msg_buf->send_buf, msg_buf->recv_buf, msg_buf->length);
}

static int spi_ioctl(void *device, int cmd, void* arg)
{
    int ret = MP_ERROR;
    
    switch (cmd)
    {
        case SPI_CONFIG_CMD:
            ret = spi_config(device, arg);
            break;
        case SPI_DEV_ATTACH_CMD:
            ret = spi_attach_dev((machine_spi_obj_t *)device);
            break;
        case SPI_DEV_SEND_RECV_CMD:
            ret = spi_send_recv((char *)device, (machine_spi_dev_msg_buf_t *)arg);
            break;
        default:
            break;
    }
    return ret;
}

STATIC struct operate spi_ops = {
    .read  =  spi_read,
    .write =  spi_write,
    .ioctl =  spi_ioctl,
};


static int spi_bus_register(void)
{
    MP_SIMILAR_DEVICE_REGISTER(MICROPYTHON_MACHINE_SPI_PRENAME, DEV_BUS, &spi_ops);
    return MP_EOK;
}
OS_DEVICE_INIT(spi_bus_register, OS_INIT_SUBLEVEL_LOW);



#endif

