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
 * @file        i2c_dev.c
 *
 * @brief       this file implements i2c_dev related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <device.h>
#include <drv_cfg.h>
#include <os_clock.h>
#include <dlog.h>

#ifdef OS_USING_I2C

#define DBG_EXT_TAG "I2C"

void os_hw_i2c_isr(struct os_i2c_bus_device *i2c, int event)
{   

}

/**
 ***********************************************************************************************************************
 * @brief           use i2c device to transfer message
 *
 * @param[in]       bus             pointer of i2c device
 * @param[in]       msgs[]          message struct
 * @param[in]       num             number of message
 *
 * @return          os_size_t
 * @retval          > 0             number of transfer bytes
 * @retval          0               transfer fail
 ***********************************************************************************************************************
 */
os_size_t os_i2c_transfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
    os_size_t ret;

    if (bus->ops->i2c_transfer)
    {
#ifdef OS_I2C_DEBUG
        for (ret = 0; ret < num; ret++)
        {
            LOG_D(DBG_EXT_TAG, "msgs[%d] %c, addr=0x%02x, len=%d",
                  ret,
                  (msgs[ret].flags & OS_I2C_RD) ? 'R' : 'W',
                  msgs[ret].addr,
                  msgs[ret].len);
        }
#endif

        os_mutex_lock(&bus->lock, OS_WAIT_FOREVER);
        ret = bus->ops->i2c_transfer(bus, msgs, num);
        os_mutex_unlock(&bus->lock);

        return ret;
    }
    else
    {
        LOG_E(DBG_EXT_TAG, "I2C bus operation not supported");

        return 0;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           i2c client write data with cmd
 *
 * @param[in]       client          pointer of i2c client
 * @param[in]       cmd             command
 * @param[in]       cmd_len         command length
 * @param[in]       buff            pointer of data to send
 * @param[in]       len             data length
 *
 * @return          os_err_t
 * @retval          OS_EOK          write successfully
 * @retval          OS_ERROR        write failed
 ***********************************************************************************************************************
 */
os_err_t os_i2c_client_write(struct os_i2c_client *client,
                             os_uint32_t           cmd,
                             os_uint8_t            cmd_len,
                             os_uint8_t           *buff,
                             os_uint32_t           len)
{
    int               i = 0;
    struct os_i2c_msg msgs[2];

    if (cmd_len != 0)
    {
        msgs[i].addr  = client->client_addr;
        msgs[i].flags = OS_I2C_WR;
        msgs[i].buf   = (os_uint8_t *)&cmd;
        msgs[i].len   = cmd_len;
        i++;
    }

    if (len != 0)
    {
        msgs[i].addr  = client->client_addr;
        msgs[i].flags = OS_I2C_WR | OS_I2C_NO_START;
        msgs[i].buf   = buff;
        msgs[i].len   = len;
        i++;
    }

    OS_ASSERT(i == 1 || i == 2);

    if (os_i2c_transfer(client->bus, msgs, i) == i)
    {
        return OS_EOK;
    }
    else
    {
        return OS_ERROR;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           i2c client read data with cmd
 *
 * @param[in]       client          pointer of i2c client
 * @param[in]       cmd             command
 * @param[in]       cmd_len         command length
 * @param[out]      buff            pointer of data use to store
 * @param[in]       len             data length
 *
 * @return          os_err_t
 * @retval          OS_EOK          read successfully
 * @retval          OS_ERROR        read failed
 ***********************************************************************************************************************
 */
os_err_t
os_i2c_client_read(struct os_i2c_client *client, os_uint32_t cmd, os_uint8_t cmd_len, os_uint8_t *buff, os_uint32_t len)
{
    int               i = 0;
    struct os_i2c_msg msgs[2];

    if (cmd_len != 0)
    {
        msgs[i].addr  = client->client_addr;
        msgs[i].flags = OS_I2C_WR;
        msgs[i].buf   = (os_uint8_t *)&cmd;
        msgs[i].len   = cmd_len;
        i++;
    }

    if (len != 0)
    {
        msgs[i].addr  = client->client_addr;
        msgs[i].flags = OS_I2C_RD;
        msgs[i].buf   = buff;
        msgs[i].len   = len;
        i++;
    }

    OS_ASSERT(i == 1 || i == 2);   

    if (os_i2c_transfer(client->bus, msgs, i) == i)
    {
        return OS_EOK;
    }
    else
    {
        return OS_ERROR;
    }
}

/**
 ***********************************************************************************************************************
 * @brief           i2c client write one byte data with cmd
 *
 * @param[in]       client          pointer of i2c client
 * @param[in]       cmd             command
 * @param[in]       cmd_len         command length
 * @param[in]       data            data to write
 *
 * @return          os_err_t
 * @retval          OS_EOK          write successfully
 * @retval          OS_ERROR        write failed
 ***********************************************************************************************************************
 */
os_err_t os_i2c_client_write_byte(struct os_i2c_client *client, os_uint32_t cmd, os_uint8_t cmd_len, os_uint8_t data)
{
    return os_i2c_client_write(client, cmd, cmd_len, &data, 1);
}

/**
 ***********************************************************************************************************************
 * @brief           i2c client rad one byte data with cmd
 *
 * @param[in]       client          pointer of i2c client
 * @param[in]       cmd             command
 * @param[in]       cmd_len         command length
 *
 * @return          os_uint8_t
 * @retval          value           read data
 ***********************************************************************************************************************
 */
os_uint8_t os_i2c_client_read_byte(struct os_i2c_client *client, os_uint32_t cmd, os_uint8_t cmd_len)
{
    os_uint8_t value = 0;
    os_i2c_client_read(client, cmd, cmd_len, &value, 1);
    return value;
}

/**
 ***********************************************************************************************************************
 * @brief           i2c device send data with address in master mode
 *
 * @param[in]       bus             pointer of i2c device
 * @param[in]       addr            address
 * @param[in]       flags           i2c transfer flags
 * @param[in]       buf             pointer of data to send
 * @param[in]       count           data length
 *
 * @return          os_size_t
 * @retval          count           bytes number of transfer data
 * @retval          else            error code
 ***********************************************************************************************************************
 */
os_size_t os_i2c_master_send(struct os_i2c_bus_device *bus,
                             os_uint16_t               addr,
                             os_uint16_t               flags,
                             const os_uint8_t         *buf,
                             os_uint32_t               count)
{
    os_err_t          ret;
    struct os_i2c_msg msg;

    msg.addr  = addr;
    msg.flags = flags;
    msg.len   = count;
    msg.buf   = (os_uint8_t *)buf;

    ret = os_i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}
                             
/**
 ***********************************************************************************************************************
 * @brief           i2c device read data with address in master mode
 *
 * @param[in]       bus             pointer of i2c device
 * @param[in]       addr            address
 * @param[in]       flags           i2c transfer flags
 * @param[in]       buf             pointer of data use to sotre
 * @param[in]       count           read data length
 *
 * @return          os_size_t
 * @retval          count           bytes number of read data
 * @retval          else            error code
 ***********************************************************************************************************************
 */
os_size_t os_i2c_master_recv(struct os_i2c_bus_device *bus,
                             os_uint16_t               addr,
                             os_uint16_t               flags,
                             os_uint8_t               *buf,
                             os_uint32_t               count)
{
    os_err_t          ret;
    struct os_i2c_msg msg;
    OS_ASSERT(bus != OS_NULL);

    msg.addr  = addr;
    msg.flags = flags | OS_I2C_RD;
    msg.len   = count;
    msg.buf   = buf;

    ret = os_i2c_transfer(bus, &msg, 1);

    return (ret > 0) ? count : ret;
}

static os_size_t i2c_bus_device_read(os_device_t *dev, os_off_t pos, void *buffer, os_size_t count)
{
    os_uint16_t addr = 0;
    os_uint16_t flags = 0;
    
    struct os_i2c_bus_device *bus = (struct os_i2c_bus_device *)dev->user_data;

    OS_ASSERT(bus != OS_NULL);
    OS_ASSERT(buffer != OS_NULL);

    LOG_D(DBG_EXT_TAG, "I2C bus dev [%s] reading %u bytes.", device_name(dev), count);

    addr  = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    return os_i2c_master_recv(bus, addr, flags, (os_uint8_t *)buffer, count);
}

static os_size_t i2c_bus_device_write(os_device_t *dev, os_off_t pos, const void *buffer, os_size_t count)
{
    os_uint16_t addr = 0;
    os_uint16_t flags = 0;
    
    struct os_i2c_bus_device *bus = (struct os_i2c_bus_device *)dev->user_data;
    
    OS_ASSERT(bus != OS_NULL);
    OS_ASSERT(buffer != OS_NULL);

    LOG_D(DBG_EXT_TAG, "I2C bus dev [%s] writing %u bytes.", device_name(dev), count);

    addr  = pos & 0xffff;
    flags = (pos >> 16) & 0xffff;

    return os_i2c_master_send(bus, addr, flags, (const os_uint8_t *)buffer, count);

}

static os_err_t i2c_bus_device_control(os_device_t *dev, int cmd, void *args)
{
    os_err_t                  ret;
   
    struct os_i2c_priv_data  *priv_data;
    struct os_i2c_bus_device *bus = (struct os_i2c_bus_device *)dev->user_data;

    OS_ASSERT(bus != OS_NULL);

    switch (cmd)
    {
    case OS_I2C_DEV_CTRL_10BIT:
        bus->flags |= OS_I2C_ADDR_10BIT;
        break;
    case OS_I2C_DEV_CTRL_ADDR:
        bus->addr = *(os_uint16_t *)args;
        break;
    case OS_I2C_DEV_CTRL_TIMEOUT:
        bus->timeout = *(os_uint32_t *)args;
        break;
    case OS_I2C_DEV_CTRL_RW:
        priv_data = (struct os_i2c_priv_data *)args;
        ret       = os_i2c_transfer(bus, priv_data->msgs, priv_data->number);
        if (ret != priv_data->number)
        {
            return OS_EIO;
        }
        break;
    default:
        break;
    }

    return OS_EOK;
}

struct os_i2c_bus_device *os_i2c_bus_device_find(const char *bus_name)
{
    struct os_i2c_bus_device *bus;

    os_device_t *dev = os_device_find(bus_name);

    if (dev == OS_NULL || dev->type != OS_DEVICE_TYPE_I2CBUS)
    {
        LOG_E(DBG_EXT_TAG, "I2C bus %s not exist", bus_name);

        return OS_NULL;
    }

    bus = (struct os_i2c_bus_device *)dev->user_data;

    return bus;
}

const static struct os_device_ops i2c_ops = {
    .read    = i2c_bus_device_read,
    .write   = i2c_bus_device_write,
    .control = i2c_bus_device_control
};

/**
 ***********************************************************************************************************************
 * @brief           init i2c device
 *
 * @param[in]       bus             pointer of i2c device
 * @param[in]       name            pointer of i2c device name
 *
 * @return          os_err_t
 * @retval          OS_EOK          run successfully
 ***********************************************************************************************************************
 */
os_err_t os_i2c_bus_device_register(struct os_i2c_bus_device *device, const char *bus_name, void *data)
{  
    OS_ASSERT(device != OS_NULL);

    if (device->timeout == 0) 
    {
        device->timeout = OS_TICK_PER_SECOND;
    }
    
    /* set device type */
    device->parent.type = OS_DEVICE_TYPE_I2CBUS;
    /* initialize device interface */
    device->parent.ops = &i2c_ops;
    device->parent.user_data = data;

    os_mutex_init(&device->lock, "i2c_bus_lock", OS_FALSE);

    /* register to device manager */
    os_device_register(&device->parent, bus_name);

    return OS_EOK;
}

#endif
