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
 * @file        spi_core.c
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
#include "os_memory.h"
#include <device.h>
#include <string.h>
#include "spi.h"
#include "pin.h"

extern os_err_t os_spi_bus_device_init(struct os_spi_bus *bus, const char *name);
extern os_err_t os_spidev_device_init(struct os_spi_device *dev, const char *name);

os_err_t os_spi_bus_register(struct os_spi_bus *bus, const char *name, const struct os_spi_ops *ops)
{
    os_err_t result;

    result = os_spi_bus_device_init(bus, name);
    if (result != OS_EOK)
        return result;

    /* Initialize mutex lock */
    os_mutex_init(&(bus->lock), name, OS_FALSE);
    /* Set ops */
    bus->ops = ops;
    /* Initialize owner */
    bus->owner = OS_NULL;
    /* Set bus mode */
    bus->mode = OS_SPI_BUS_MODE_SPI;

    return OS_EOK;
}

os_err_t os_spi_bus_attach_device(struct os_spi_device *device, const char *name, const char *bus_name, os_base_t cs_pin)
{
    os_err_t     result;
    os_device_t *bus;

    /* Get physical spi bus */
    bus = os_device_find(bus_name);
    if (bus != OS_NULL && bus->type == OS_DEVICE_TYPE_SPIBUS)
    {
        device->bus = (struct os_spi_bus *)bus;

        /* Initialize spidev device */
        result = os_spidev_device_init(device, name);
        if (result != OS_EOK)
            return result;

        memset(&device->config, 0, sizeof(device->config));
        device->cs_pin = cs_pin;

        return OS_EOK;
    }

    /* Not found the host bus */
    return OS_ERROR;
}

os_err_t os_hw_spi_device_attach(const char *bus_name, const char *device_name, os_base_t cs_pin)
{
    os_err_t ret = OS_EOK;

    struct os_spi_device *spi_device = (struct os_spi_device *)os_calloc(1, sizeof(struct os_spi_device));
    OS_ASSERT(spi_device != OS_NULL);

    os_pin_mode(cs_pin, PIN_MODE_OUTPUT);
    os_pin_write(cs_pin, PIN_HIGH);

    ret = os_spi_bus_attach_device(spi_device, device_name, bus_name, cs_pin);

    return ret;
}

os_err_t os_spi_configure(struct os_spi_device *device, struct os_spi_configuration *cfg)
{
    os_err_t result;

    OS_ASSERT(device != OS_NULL);

    /* Set configuration */
    device->config.data_width = cfg->data_width;
    device->config.mode       = cfg->mode & OS_SPI_MODE_MASK;
    device->config.max_hz     = cfg->max_hz;

    if (device->bus != OS_NULL)
    {
        result = os_mutex_lock(&(device->bus->lock), OS_WAIT_FOREVER);
        if (result == OS_EOK)
        {
            if (device->bus->owner == device)
            {
                device->bus->ops->configure(device, &device->config);
            }

            /* Release lock */
            os_mutex_unlock(&(device->bus->lock));
        }
    }

    return OS_EOK;
}

os_err_t os_spi_send_then_send(struct os_spi_device *device,
                               const void           *send_buf1,
                               os_size_t             send_length1,
                               const void           *send_buf2,
                               os_size_t             send_length2)
{
    os_err_t              result;
    struct os_spi_message message;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    result = os_mutex_lock(&(device->bus->lock), OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        if (device->bus->owner != device)
        {
            /* Not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == OS_EOK)
            {
                /* Set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* Configure SPI bus failed */
                result = OS_EIO;
                goto __exit;
            }
        }

        /* Send data1 */
        message.send_buf   = send_buf1;
        message.recv_buf   = OS_NULL;
        message.length     = send_length1;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = OS_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = OS_EIO;
            goto __exit;
        }

        /* Send data2 */
        message.send_buf   = send_buf2;
        message.recv_buf   = OS_NULL;
        message.length     = send_length2;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = OS_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = OS_EIO;
            goto __exit;
        }

        result = OS_EOK;
    }
    else
    {
        return OS_EIO;
    }

__exit:
    os_mutex_unlock(&(device->bus->lock));

    return result;
}

os_err_t os_spi_send_then_recv(struct os_spi_device *device,
                               const void           *send_buf,
                               os_size_t             send_length,
                               void                 *recv_buf,
                               os_size_t             recv_length)
{
    os_err_t              result;
    struct os_spi_message message;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    result = os_mutex_lock(&(device->bus->lock), OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        if (device->bus->owner != device)
        {
            /* Not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == OS_EOK)
            {
                /* Set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* Configure SPI bus failed */
                result = OS_EIO;
                goto __exit;
            }
        }

        /* Send data */
        message.send_buf   = send_buf;
        message.recv_buf   = OS_NULL;
        message.length     = send_length;
        message.cs_take    = 1;
        message.cs_release = 0;
        message.next       = OS_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = OS_EIO;
            goto __exit;
        }

        /* Recv data */
        message.send_buf   = OS_NULL;
        message.recv_buf   = recv_buf;
        message.length     = recv_length;
        message.cs_take    = 0;
        message.cs_release = 1;
        message.next       = OS_NULL;

        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            result = OS_EIO;
            goto __exit;
        }

        result = OS_EOK;
    }
    else
    {
        return OS_EIO;
    }

__exit:
    os_mutex_unlock(&(device->bus->lock));

    return result;
}

os_size_t os_spi_transfer(struct os_spi_device *device, const void *send_buf, void *recv_buf, os_size_t length)
{
    os_err_t              result;
    struct os_spi_message message;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    result = os_mutex_lock(&(device->bus->lock), OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        if (device->bus->owner != device)
        {
            /* Not the same owner as current, re-configure SPI bus */
            result = device->bus->ops->configure(device, &device->config);
            if (result == OS_EOK)
            {
                /* Set SPI bus owner */
                device->bus->owner = device;
            }
            else
            {
                /* Configure SPI bus failed */
                os_set_errno(OS_EIO);
                result = 0;
                goto __exit;
            }
        }

        /* Initial message */
        message.send_buf   = send_buf;
        message.recv_buf   = recv_buf;
        message.length     = length;
        message.cs_take    = 1;
        message.cs_release = 1;
        message.next       = OS_NULL;

        /* Transfer message */
        result = device->bus->ops->xfer(device, &message);
        if (result == 0)
        {
            os_set_errno(OS_EIO);
            goto __exit;
        }
    }
    else
    {
        os_set_errno(OS_EIO);

        return 0;
    }

__exit:
    os_mutex_unlock(&(device->bus->lock));

    return result;
}

struct os_spi_message *os_spi_transfer_message(struct os_spi_device *device, struct os_spi_message *message)
{
    os_err_t               result;
    struct os_spi_message *index;

    OS_ASSERT(device != OS_NULL);

    /* Get first message */
    index = message;
    if (index == OS_NULL)
        return index;

    result = os_mutex_lock(&(device->bus->lock), OS_WAIT_FOREVER);
    if (result != OS_EOK)
    {
        os_set_errno(OS_EBUSY);

        return index;
    }

    /* Reset errno */
    os_set_errno(OS_EOK);

    /* Configure SPI bus */
    if (device->bus->owner != device)
    {
        /* Not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->config);
        if (result == OS_EOK)
        {
            /* Set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* Configure SPI bus failed */
            os_set_errno(OS_EIO);
            goto __exit;
        }
    }

    /* Transmit each SPI message */
    while (index != OS_NULL)
    {
        /* Transmit SPI message */
        result = device->bus->ops->xfer(device, index);
        if (result == 0)
        {
            os_set_errno(OS_EIO);
            break;
        }

        index = index->next;
    }

__exit:
    /* Release bus lock */
    os_mutex_unlock(&(device->bus->lock));

    return index;
}

/**
 ***********************************************************************************************************************
 * @brief           This function takes SPI bus.
 *
 * @param[in]       device          The SPI device attached to SPI bus.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_spi_take_bus(struct os_spi_device *device)
{
    os_err_t result = OS_EOK;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    result = os_mutex_lock(&(device->bus->lock), OS_WAIT_FOREVER);
    if (result != OS_EOK)
    {
        os_set_errno(OS_EBUSY);

        return OS_EBUSY;
    }

    /* Reset errno */
    os_set_errno(OS_EOK);

    /* Configure SPI bus */
    if (device->bus->owner != device)
    {
        /* Not the same owner as current, re-configure SPI bus */
        result = device->bus->ops->configure(device, &device->config);
        if (result == OS_EOK)
        {
            /* Set SPI bus owner */
            device->bus->owner = device;
        }
        else
        {
            /* Configure SPI bus failed */
            os_set_errno(OS_EIO);
            /* Release lock */
            os_mutex_unlock(&(device->bus->lock));

            return OS_EIO;
        }
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function releases SPI bus.
 *
 * @param[in]       device          The SPI device attached to SPI bus.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_spi_release_bus(struct os_spi_device *device)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);
    OS_ASSERT(device->bus->owner == device);

    /* Release lock */
    os_mutex_unlock(&(device->bus->lock));

    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           This function take SPI device (takes CS of SPI device).
 *
 * @param[in]       device          The SPI device attached to SPI bus.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 * @retval          others          Fail.
 ***********************************************************************************************************************
 */
os_err_t os_spi_take(struct os_spi_device *device)
{
    os_err_t              result;
    struct os_spi_message message;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    memset(&message, 0, sizeof(message));
    message.cs_take = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function releases SPI device (releases CS of SPI device).
 *
 * @param[in]       device          The SPI device attached to SPI bus.
 *
 * @return          The operation status.
 * @retval          OS_EOK          Successful.
 ***********************************************************************************************************************
 */
os_err_t os_spi_release(struct os_spi_device *device)
{
    os_err_t              result;
    struct os_spi_message message;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(device->bus != OS_NULL);

    memset(&message, 0, sizeof(message));
    message.cs_release = 1;

    result = device->bus->ops->xfer(device, &message);

    return result;
}
