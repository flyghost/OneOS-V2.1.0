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
 * @file        qspi_core.c
 *
 * @brief       This file provides functions for qspi.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "spi.h"

os_err_t os_qspi_configure(struct os_qspi_device *device, struct os_qspi_configuration *cfg)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(cfg != OS_NULL);

    struct os_qspi_device *qspi_device = (struct os_qspi_device *)device;
    os_err_t result = OS_EOK;

    /* Copy configuration items */
    qspi_device->config.parent.mode       = cfg->parent.mode;
    qspi_device->config.parent.max_hz     = cfg->parent.max_hz;
    qspi_device->config.parent.data_width = cfg->parent.data_width;
    qspi_device->config.parent.reserved   = cfg->parent.reserved;
    qspi_device->config.medium_size       = cfg->medium_size;
    qspi_device->config.ddr_mode          = cfg->ddr_mode;
    qspi_device->config.qspi_dl_width     = cfg->qspi_dl_width;

    result = os_spi_configure(&device->parent, &cfg->parent);

    return result;
}

os_err_t os_qspi_bus_register(struct os_spi_bus *bus, const char *name, const struct os_spi_ops *ops)
{
    os_err_t result = OS_EOK;

    result = os_spi_bus_register(bus, name, ops);
    if (result == OS_EOK)
    {
        /* Set SPI bus to qspi modes */
        bus->mode = OS_SPI_BUS_MODE_QSPI;
    }

    return result;
}

os_size_t os_qspi_transfer_message(struct os_qspi_device *device, struct os_qspi_message *message)
{
    os_err_t result;

    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(message != OS_NULL);

    result = os_mutex_lock(&(device->parent.bus->lock), OS_WAIT_FOREVER);
    if (result != OS_EOK)
    {
        os_set_errno(OS_EBUSY);

        return 0;
    }

    /* Reset errno */
    os_set_errno(OS_EOK);

    /* Configure SPI bus */
    if (device->parent.bus->owner != &device->parent)
    {
        /* Not the same owner as current, re-configure SPI bus */
        result = device->parent.bus->ops->configure(&device->parent, &device->parent.config);
        if (result == OS_EOK)
        {
            /* Set SPI bus owner */
            device->parent.bus->owner = &device->parent;
        }
        else
        {
            /* Configure SPI bus failed */
            os_set_errno(OS_EIO);
            goto __exit;
        }
    }

    /* Transmit each SPI message */

    result = device->parent.bus->ops->xfer(&device->parent, &message->parent);
    if (result == 0)
    {
        os_set_errno(OS_EIO);
    }

__exit:
    /* Release bus lock */
    os_mutex_unlock(&(device->parent.bus->lock));

    return result;
}

os_err_t os_qspi_send_then_recv(struct os_qspi_device *device,
                                const void            *send_buf,
                                os_size_t              send_length,
                                void                  *recv_buf,
                                os_size_t              recv_length)
{
    OS_ASSERT(send_buf);
    OS_ASSERT(recv_buf);
    OS_ASSERT(send_length != 0);

    struct os_qspi_message message;
    unsigned char *ptr = (unsigned char *)send_buf;
    os_size_t count = 0;
    os_err_t result = 0;

    message.instruction.content    = ptr[0];
    message.instruction.qspi_lines = 1;
    count++;

    /* Get address */
    if (send_length > 1)
    {
        if (device->config.medium_size > 0x1000000 && send_length >= 5)
        {
            /* Medium size greater than 16Mb, address size is 4 Byte */
            message.address.content = (ptr[1] << 24) | (ptr[2] << 16) | (ptr[3] << 8) | (ptr[4]);
            message.address.size    = 32;
            count += 4;
        }
        else if (send_length >= 4)
        {
            /* Address size is 3 Byte */
            message.address.content = (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
            message.address.size    = 24;
            count += 3;
        }
        else
        {
            return OS_ERROR;
        }
        message.address.qspi_lines = 1;
    }
    else
    {
        /* No address stage */
        message.address.content    = 0;
        message.address.qspi_lines = 0;
        message.address.size       = 0;
    }

    message.alternate_bytes.content    = 0;
    message.alternate_bytes.size       = 0;
    message.alternate_bytes.qspi_lines = 0;

    /* Set dummy cycles */
    if (count != send_length)
    {
        message.dummy_cycles = (send_length - count) * 8;
    }
    else
    {
        message.dummy_cycles = 0;
    }

    /* Set recv buf and recv size */
    message.parent.recv_buf   = recv_buf;
    message.parent.send_buf   = OS_NULL;
    message.parent.length     = recv_length;
    message.parent.cs_take    = 1;
    message.parent.cs_release = 1;

    message.qspi_data_lines = 1;

    result = os_qspi_transfer_message(device, &message);
    if (result == 0)
    {
        result = OS_EIO;
    }
    else
    {
        result = recv_length;
    }

    return result;
}

/**
 ***********************************************************************************************************************
 * @brief           This function can send data to QSPI device.
 *
 * @param[in]       device       The QSPI device attached to QSPI bus.
 * @param[in]       send_buf     The buffer to be transmitted to QSPI device.
 * @param[in]       length       The number of data to be transmitted.
 *
 * @return          The status of transmit.
 * @retval          OS_ERROR     Generic ERROR.
 * @retval          OS_EIO       IO ERROR.
 * @retval          length       The length of send buf.
 ***********************************************************************************************************************
 */
os_err_t os_qspi_send(struct os_qspi_device *device, const void *send_buf, os_size_t length)
{
    OS_ASSERT(send_buf);
    OS_ASSERT(length != 0);

    struct os_qspi_message message;
    char      *ptr    = (char *)send_buf;
    os_size_t  count  = 0;
    os_err_t   result = 0;

    message.instruction.content    = ptr[0];
    message.instruction.qspi_lines = 1;
    count++;

    /* Get address */
    if (length > 1)
    {
        if (device->config.medium_size > 0x1000000 && length >= 5)
        {
            /* Medium size greater than 16Mb, address size is 4 Byte */
            message.address.content    = (ptr[1] << 24) | (ptr[2] << 16) | (ptr[3] << 8) | (ptr[4]);
            message.address.size       = 32;
            message.address.qspi_lines = 1;
            count += 4;
        }
        else if (length >= 4)
        {
            /* Address size is 3 Byte */
            message.address.content    = (ptr[1] << 16) | (ptr[2] << 8) | (ptr[3]);
            message.address.size       = 24;
            message.address.qspi_lines = 1;
            count += 3;
        }
        else
        {
            return OS_ERROR;
        }
    }
    else
    {
        /* No address stage */
        message.address.content    = 0;
        message.address.qspi_lines = 0;
        message.address.size       = 0;
    }

    message.alternate_bytes.content    = 0;
    message.alternate_bytes.size       = 0;
    message.alternate_bytes.qspi_lines = 0;

    message.dummy_cycles = 0;

    /* Determine if there is data to send */
    if (length - count > 0)
    {
        message.qspi_data_lines = 1;
    }
    else
    {
        message.qspi_data_lines = 0;
    }

    /* Set send buf and send size */
    message.parent.send_buf   = ptr + count;
    message.parent.recv_buf   = OS_NULL;
    message.parent.length     = length - count;
    message.parent.cs_take    = 1;
    message.parent.cs_release = 1;

    result = os_qspi_transfer_message(device, &message);
    if (result == 0)
    {
        result = OS_EIO;
    }
    else
    {
        result = length;
    }

    return result;
}
