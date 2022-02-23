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
 * @file        soft_i2c_bus.c
 *
 * @brief       this file implements i2c-bit-bus related functions
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_cfg.h>
#include <os_clock.h>
#include <os_memory.h>
#include <timer/clocksource.h>
#include <drv_log.h>

#define DBG_TAG "I2C"
#ifdef OS_I2C_BITOPS_DEBUG
#define DRV_EXT_LVL DBG_LOG
#else
#define DRV_EXT_LVL DBG_EXT_INFO
#endif

struct soft_i2c_bus
{
    struct os_i2c_bus_device i2c_bus;
    struct soft_i2c_config   config;
};

#define SET_SCL_OUTPUT(bus)    os_pin_mode(bus->config.scl, PIN_MODE_OUTPUT_OD)
#define SET_SDA_OUTPUT(bus)    os_pin_mode(bus->config.sda, PIN_MODE_OUTPUT_OD)
#define SET_SCL_INPUT(bus)     os_pin_mode(bus->config.scl, PIN_MODE_INPUT)
#define SET_SDA_INPUT(bus)     os_pin_mode(bus->config.sda, PIN_MODE_INPUT)

#define SET_SDA(bus, val) os_pin_write(bus->config.sda, val)
#define SET_SCL(bus, val) os_pin_write(bus->config.scl, val)
#define GET_SDA(bus)      os_pin_read(bus->config.sda)
#define GET_SCL(bus)      os_pin_read(bus->config.scl)

#define SDA_L(bus) SET_SDA(bus, 0)
#define SDA_H(bus) SET_SDA(bus, 1)
#define SCL_L(bus) SET_SCL(bus, 0)
//#define SCL_H(bus) SET_SCL(bus, 1)

#define SOFT_I2C_BUS_TIMEOUT_MS (100)

#ifndef SOFT_I2C_BUS_DELAY_US
#define SOFT_I2C_BUS_DELAY_US   (10)
#endif

static void soft_i2c_delay(os_uint32_t us)
{
    os_clocksource_ndelay(us * 1000);
}

static os_err_t SCL_H(struct soft_i2c_bus *bus)
{
    SET_SCL(bus, 1);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    return OS_EOK;
}

static os_err_t soft_i2c_bus_unlock(struct soft_i2c_bus *bus)
{
    os_int32_t i = 0;

    SET_SDA_INPUT(bus);

    if (PIN_LOW == GET_SDA(bus))
    {
        while (i++ < 9)
        {
            SET_SCL(bus, PIN_HIGH);
            soft_i2c_delay(100);
            SET_SCL(bus, PIN_LOW);
            soft_i2c_delay(100);
        }
    }
    if (PIN_LOW == GET_SDA(bus))
    {
        SET_SDA_OUTPUT(bus);
        SDA_H(bus);
        return OS_ERROR;
    }
    SET_SDA_OUTPUT(bus);
    SDA_H(bus);
    return OS_EOK;
}

static void i2c_start(struct soft_i2c_bus *bus)
{
    SCL_H(bus);
    SDA_H(bus);

    SET_SCL_OUTPUT(bus);
    SET_SDA_OUTPUT(bus);

    SDA_L(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    SCL_L(bus);
}

static void i2c_restart(struct soft_i2c_bus *bus)
{
    SDA_H(bus);
    SCL_H(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    SDA_L(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    SCL_L(bus);
}

static void i2c_stop(struct soft_i2c_bus *bus)
{
    SDA_L(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    SCL_H(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    SDA_H(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US * 2);
    
    os_pin_mode(bus->config.scl, PIN_MODE_DISABLE);
    os_pin_mode(bus->config.sda, PIN_MODE_DISABLE);
}


OS_INLINE os_bool_t i2c_waitack(struct soft_i2c_bus *bus)
{
    os_bool_t ack;

    SDA_H(bus);
    SET_SDA_INPUT(bus);

    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);

    if (SCL_H(bus) < 0)
    {
        LOG_W(DBG_TAG,"wait ack timeout");
        SET_SDA_OUTPUT(bus);
        SDA_H(bus);
        return OS_ETIMEOUT;
    }

    ack = !GET_SDA(bus); /* ACK : SDA pin is pulled low */
    LOG_D(DBG_TAG, "%s", ack ? "ACK" : "NACK");

    SCL_L(bus);
    SET_SDA_OUTPUT(bus);
    SDA_H(bus);
    return ack;
}

static os_int32_t i2c_writeb(struct os_i2c_bus_device *i2c_bus, os_uint8_t data)
{
    os_int32_t i;
    os_uint8_t bit;

    struct soft_i2c_bus *bus = (struct soft_i2c_bus *)i2c_bus;

    for (i = 7; i >= 0; i--)
    {
        SCL_L(bus);
        bit = (data >> i) & 1;
        SET_SDA(bus, bit);
        soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
        if (SCL_H(bus) < 0)
        {
            LOG_D(DBG_TAG, "i2c_writeb: 0x%02x, "
                      "wait scl pin high timeout at bit %d",
                      data,
                      i);

            return OS_ETIMEOUT;
        }
    }
    SCL_L(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);

    return i2c_waitack(bus);
}

static os_int32_t i2c_readb(struct os_i2c_bus_device *i2c_bus)
{
    os_uint8_t i;
    os_uint8_t data = 0;

    struct soft_i2c_bus *bus  = (struct soft_i2c_bus *)i2c_bus;
    
    SET_SDA_INPUT(bus);

    SDA_H(bus);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    for (i = 0; i < 8; i++)
    {
        data <<= 1;

        if (SCL_H(bus) < 0)
        {
            LOG_D(DBG_TAG, "i2c_readb: wait scl pin high "
                      "timeout at bit %d",
                      7 - i);
            SET_SDA_OUTPUT(bus);
            SDA_H(bus);
            return OS_ETIMEOUT;
        }

        if (GET_SDA(bus))
            data |= 1;
        SCL_L(bus);
        soft_i2c_delay(SOFT_I2C_BUS_DELAY_US * 2);
    }
    
    SET_SDA_OUTPUT(bus);
    SDA_H(bus);
    return data;
}

static os_size_t i2c_send_bytes(struct os_i2c_bus_device *bus, struct os_i2c_msg *msg)
{
    os_int32_t        ret;
    os_size_t         bytes       = 0;
    const os_uint8_t *ptr         = msg->buf;
    os_int32_t        count       = msg->len;
    os_uint16_t       ignore_nack = msg->flags & OS_I2C_IGNORE_NACK;

    while (count > 0)
    {
        ret = i2c_writeb(bus, *ptr);

        if ((ret > 0) || (ignore_nack && (ret == 0)))
        {
            count--;
            ptr++;
            bytes++;
        }
        else if (ret == 0)
        {
            LOG_D(DBG_TAG, "send bytes: NACK.");
            return 0;
        }
        else
        {
            LOG_E(DBG_TAG,"send bytes: error %d", ret);
            return ret;
        }
    }

    return bytes;
}

static os_err_t i2c_send_ack_or_nack(struct os_i2c_bus_device *i2c_bus, int ack)
{
    struct soft_i2c_bus *bus = (struct soft_i2c_bus *)i2c_bus;

    if (ack)
        SET_SDA(bus, 0);
    soft_i2c_delay(SOFT_I2C_BUS_DELAY_US);
    if (SCL_H(bus) < 0)
    {
        LOG_E(DBG_TAG,"ACK or NACK timeout.");

        return OS_ETIMEOUT;
    }
    SCL_L(bus);

    return OS_EOK;
}

static os_size_t i2c_recv_bytes(struct os_i2c_bus_device *bus, struct os_i2c_msg *msg)
{
    os_int32_t        val;
    os_int32_t        bytes = 0; /* actual bytes */
    os_uint8_t       *ptr   = msg->buf;
    os_int32_t        count = msg->len;
    const os_uint32_t flags = msg->flags;

    while (count > 0)
    {
        val = i2c_readb(bus);
        if (val >= 0)
        {
            *ptr = val;
            bytes++;
        }
        else
        {
            break;
        }

        ptr++;
        count--;

        LOG_D(DBG_TAG, "recieve bytes: 0x%02x, %s",
                  val,
                  (flags & OS_I2C_NO_READ_ACK) ? "(No ACK/NACK)" : (count ? "ACK" : "NACK"));

        if (!(flags & OS_I2C_NO_READ_ACK))
        {
            val = i2c_send_ack_or_nack(bus, count);
            if (val < 0)
                return val;
        }
    }

    return bytes;
}

static os_int32_t i2c_send_address(struct os_i2c_bus_device *i2c_bus, os_uint8_t addr, os_int32_t retries)
{
    os_int32_t i;
    os_err_t   ret = 0;

    struct soft_i2c_bus *bus = (struct soft_i2c_bus *)i2c_bus;

    for (i = 0; i <= retries; i++)
    {
        ret = i2c_writeb(i2c_bus, addr);
        if (ret == 1 || i == retries)
            break;
        LOG_D(DBG_TAG, "send stop condition");
        i2c_stop(bus);
        soft_i2c_delay(SOFT_I2C_BUS_DELAY_US * 2);
        LOG_D(DBG_TAG, "send start condition");
        i2c_start(bus);
    }

    return ret;
}

static os_err_t i2c_bit_send_address(struct os_i2c_bus_device *bus, struct os_i2c_msg *msg)
{
    os_uint16_t          flags       = msg->flags;
    os_uint16_t          ignore_nack = msg->flags & OS_I2C_IGNORE_NACK;

    os_uint8_t addr1, addr2;
    os_int32_t retries;
    os_err_t   ret;

    retries = ignore_nack ? 0 : bus->retries;

    if (flags & OS_I2C_ADDR_10BIT)
    {
        addr1 = 0xf0 | ((msg->addr >> 7) & 0x06);
        addr2 = msg->addr & 0xff;

        LOG_D(DBG_TAG, "addr1: %d, addr2: %d", addr1, addr2);

        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
        {
            LOG_W(DBG_TAG,"NACK: sending first addr");

            return OS_EIO;
        }

        ret = i2c_writeb(bus, addr2);
        if ((ret != 1) && !ignore_nack)
        {
            LOG_W(DBG_TAG,"NACK: sending second addr");

            return OS_EIO;
        }
        if (flags & OS_I2C_RD)
        {
            LOG_D(DBG_TAG, "send repeated start condition");
            i2c_restart((struct soft_i2c_bus *)bus);
            addr1 |= 0x01;
            ret = i2c_send_address(bus, addr1, retries);
            if ((ret != 1) && !ignore_nack)
            {
                LOG_E(DBG_TAG,"NACK: sending repeated addr");

                return OS_EIO;
            }
        }
    }
    else
    {
        /* 7-bit addr */
        addr1 = msg->addr << 1;
        if (flags & OS_I2C_RD)
            addr1 |= 1;
        ret = i2c_send_address(bus, addr1, retries);
        if ((ret != 1) && !ignore_nack)
            return OS_EIO;
    }

    return OS_EOK;
}

static os_size_t i2c_bit_transfer(struct os_i2c_bus_device *i2c_bus, struct os_i2c_msg msgs[], os_uint32_t num)
{
    struct os_i2c_msg    *msg;
    struct soft_i2c_bus  *bus = (struct soft_i2c_bus *)i2c_bus;
    os_int32_t            i, ret;
    os_uint16_t           ignore_nack;

    LOG_D(DBG_TAG, "send start condition");
    i2c_start(bus);
    for (i = 0; i < num; i++)
    {
        msg         = &msgs[i];
        ignore_nack = msg->flags & OS_I2C_IGNORE_NACK;
        if (!(msg->flags & OS_I2C_NO_START))
        {
            if (i)
            {
                i2c_restart(bus);
            }
            ret = i2c_bit_send_address(i2c_bus, msg);
            if ((ret != OS_EOK) && !ignore_nack)
            {
                LOG_D(DBG_TAG, "receive NACK from device addr 0x%02x msg %d", msgs[i].addr, i);
                goto out;
            }
        }
        if (msg->flags & OS_I2C_RD)
        {
            ret = i2c_recv_bytes(i2c_bus, msg);
            if (ret >= 1)
                LOG_D(DBG_TAG, "read %d byte%s", ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = OS_EIO;
                goto out;
            }
        }
        else
        {
            ret = i2c_send_bytes(i2c_bus, msg);
            if (ret >= 1)
                LOG_D(DBG_TAG, "write %d byte%s", ret, ret == 1 ? "" : "s");
            if (ret < msg->len)
            {
                if (ret >= 0)
                    ret = OS_ERROR;
                goto out;
            }
        }
    }
    ret = i;

out:
    LOG_D(DBG_TAG, "send stop condition");
    i2c_stop(bus);

    return ret;
}

static const struct os_i2c_bus_device_ops i2c_bit_bus_ops =
{
    .i2c_transfer       = i2c_bit_transfer,
    .i2c_slave_transfer = OS_NULL,
    .i2c_bus_control    = OS_NULL,
};

static int soft_i2c_bus_probe(const os_driver_info_t *drv, const os_device_info_t *dev)
{
    struct soft_i2c_bus     *bus;
    struct soft_i2c_config  *config;

    config = (struct soft_i2c_config *)dev->info;
    OS_ASSERT(config != OS_NULL);

    bus = os_calloc(1, sizeof(struct soft_i2c_bus));
    OS_ASSERT(bus != OS_NULL);

    bus->config      = *config;
    bus->i2c_bus.ops = &i2c_bit_bus_ops;
    
    os_i2c_bus_device_register(&bus->i2c_bus, dev->name, &bus->i2c_bus);

    soft_i2c_bus_unlock(bus);

    LOG_D(DBG_TAG, "software simulation %s init done, pin scl: %d, pin sda %d",
              dev->name,
              config->scl,
              config->sda);

    return 0;
}

OS_DRIVER_INFO soft_i2c_bus_driver = {
    .name   = "soft_i2c_bus",
    .probe  = soft_i2c_bus_probe,
};

OS_DRIVER_DEFINE(soft_i2c_bus_driver, PREV, OS_INIT_SUBLEVEL_HIGH);

