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
 * @file        spi_dev.c
 *
 * @brief       This file provides functions for spi.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_task.h>
#include <os_errno.h>
#include <device.h>
#include "spi.h"

static os_size_t _spi_bus_device_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_spi_bus *bus;

    bus = (struct os_spi_bus *)dev;
    OS_ASSERT(bus != OS_NULL);
    OS_ASSERT(bus->owner != OS_NULL);

    return os_spi_transfer(bus->owner, OS_NULL, buffer, size);
}

static os_size_t _spi_bus_device_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_spi_bus *bus;

    bus = (struct os_spi_bus *)dev;
    OS_ASSERT(bus != OS_NULL);
    OS_ASSERT(bus->owner != OS_NULL);

    return os_spi_transfer(bus->owner, buffer, OS_NULL, size);
}

static os_err_t _spi_bus_device_control(os_device_t *dev, int cmd, void *args)
{
    /* TODO: add control command handle */
    switch (cmd)
    {
    case 0: /* Set device */
        break;
    case 1:
        break;
    }

    return OS_EOK;
}

const static struct os_device_ops spi_bus_ops = 
{
    .read    = _spi_bus_device_read,
    .write   = _spi_bus_device_write,
    .control = _spi_bus_device_control
};

os_err_t os_spi_bus_device_init(struct os_spi_bus *bus, const char *name)
{
    struct os_device *device;
    OS_ASSERT(bus != OS_NULL);

    device = &bus->parent;

    /* Set device type */
    device->type = OS_DEVICE_TYPE_SPIBUS;
    /* Initialize device interface */
    device->ops = &spi_bus_ops;

    /* Register to device manager */
    return os_device_register(device, name);
}

/* SPI Dev device interface */
static os_size_t _spidev_device_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t size)
{
    struct os_spi_device *device;

    device = (struct os_spi_device *)dev;
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    return os_spi_transfer(device, OS_NULL, buffer, size);
}

static os_size_t _spidev_device_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t size)
{
    struct os_spi_device *device;

    device = (struct os_spi_device *)dev;
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    return os_spi_transfer(device, buffer, OS_NULL, size);
}

static os_err_t _spidev_device_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t ret = OS_EOK;
    struct os_spi_message *message;
    struct os_spi_device *device;

    device = (struct os_spi_device *)dev;
    
    switch (cmd)
    {
    case OS_SPI_DEVICE_CTRL_RW:
        message = (struct os_spi_message *)args;
        ret = os_spi_transfer(device, message->send_buf, message->recv_buf, message->length);
        break;
    case 1:
        break;
    default:
        break;
    }

    return ret;
}

const static struct os_device_ops spi_device_ops = 
{
    .read    = _spidev_device_read,
    .write   = _spidev_device_write,
    .control = _spidev_device_control
};

os_err_t os_spidev_device_init(struct os_spi_device *dev, const char *name)
{
    struct os_device *device;
    OS_ASSERT(dev != OS_NULL);

    device = &(dev->parent);

    /* Set device type */
    device->type = OS_DEVICE_TYPE_SPIDEVICE;
    device->ops  = &spi_device_ops;

    /* Register to device manager */
    return os_device_register(device, name);
}
