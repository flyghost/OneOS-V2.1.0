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
 * @file        i2c.h
 *
 * @brief       this file implements i2c related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __I2C_H__
#define __I2C_H__

#include <os_task.h>
#include <device.h>
#include <os_mutex.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OS_I2C_WR          0x0000
#define OS_I2C_RD          (1u << 0)
#define OS_I2C_ADDR_10BIT  (1u << 2) /* this is a ten bit chip address */
#define OS_I2C_NO_START    (1u << 4)
#define OS_I2C_IGNORE_NACK (1u << 5)
#define OS_I2C_NO_READ_ACK (1u << 6) /* when I2C reading, we do not ACK */

#define OS_I2C_DEV_MEM_MODE         0
#define OS_I2C_DEV_MASTER_MODE      1
#define OS_I2C_DEV_SLAVE_MODE       2

#define OS_I2C_DEV_CTRL_10BIT   IOC_I2CBUS(0x00)
#define OS_I2C_DEV_CTRL_ADDR    IOC_I2CBUS(0x01)
#define OS_I2C_DEV_CTRL_TIMEOUT IOC_I2CBUS(0x02)
#define OS_I2C_DEV_CTRL_RW      IOC_I2CBUS(0x03)

struct os_i2c_msg
{
    os_uint16_t addr;
    os_uint16_t flags;
    os_uint16_t len;
    os_uint8_t *buf;
};

struct os_i2c_priv_data
{
    struct os_i2c_msg *msgs;
    os_size_t          number;
};

struct os_i2c_bus_device
{
    struct os_device                    parent;
    const struct os_i2c_bus_device_ops *ops;
    os_uint16_t                         addr;
    os_uint16_t                         flags;
    struct os_mutex                     lock;
    os_uint32_t                         timeout;
    os_uint32_t                         retries;
    void *                              priv;
};

struct os_i2c_client
{
    struct os_i2c_bus_device *bus;
    os_uint16_t               client_addr;
};

struct os_i2c_bus_device_ops
{
    os_size_t (*i2c_transfer)(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num);
    os_size_t (*i2c_slave_transfer)(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num);
    os_err_t (*i2c_bus_control)(struct os_i2c_bus_device *bus, void *arg);
};

void i2c_delay(os_uint32_t us);
void                      os_hw_i2c_isr(struct os_i2c_bus_device *i2c, int event);
os_err_t                  os_i2c_bus_device_register(struct os_i2c_bus_device *device, const char *bus_name, void *data);
struct os_i2c_bus_device *os_i2c_bus_device_find(const char *bus_name);
os_size_t                 os_i2c_transfer(struct os_i2c_bus_device *bus, struct os_i2c_msg msgs[], os_uint32_t num);
os_err_t   os_i2c_client_write(struct os_i2c_client *client,
                               os_uint32_t           cmd,
                               os_uint8_t            cmd_len,
                               os_uint8_t           *buff,
                               os_uint32_t           len);
os_err_t   os_i2c_client_read(struct os_i2c_client *client,
                              os_uint32_t           cmd,
                              os_uint8_t            cmd_len,
                              os_uint8_t           *buff,
                              os_uint32_t           len);
os_err_t   os_i2c_client_write_byte(struct os_i2c_client *client, os_uint32_t cmd, os_uint8_t cmd_len, os_uint8_t data);
os_uint8_t os_i2c_client_read_byte(struct os_i2c_client *client, os_uint32_t cmd, os_uint8_t cmd_len);
os_size_t os_i2c_master_send(struct os_i2c_bus_device *bus,
                             os_uint16_t               addr,
                             os_uint16_t               flags,
                             const os_uint8_t         *buf,
                             os_uint32_t               count);
os_size_t os_i2c_master_recv(struct os_i2c_bus_device *bus,
                             os_uint16_t               addr,
                             os_uint16_t               flags,
                             os_uint8_t               *buf,
                             os_uint32_t               count);						 
#ifdef __cplusplus
}
#endif

#endif
