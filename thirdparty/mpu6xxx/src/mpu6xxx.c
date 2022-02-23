/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-10-23     flybreak     the first version
 * 2020-03-20     OneOS Team   adapt the code to OneOS
 */

#include <os_task.h>
#include <drv_cfg.h>
#include <os_memory.h>
#include <shell.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define DRV_EXT_LVL         LOG_LVL_DEBUG
#define DRV_EXT_TAG         "mpu6xxx" 
#define DBG_TAG             "mpu6xxx" 
#include <drv_log.h>

#include "../inc/mpu6xxx.h"
#include "../inc/mpu6xxx_reg.h"

#define MPU6XXX_ACCEL_SEN     (16384)
#define MPU6XXX_GYRO_SEN      (1310)

#define MPU60X0_SPI_MAX_SPEED (1000 * 1000)
#define MPU60X0_TEMP_SEN      (340)
#define MPU60X0_TEMP_OFFSET   (36.5)

#define MPU6500_TEMP_SEN      (333.87)
#define MPU6500_TEMP_OFFSET   (21)

#define MPU9250_TEMP_SEN      (333.87)
#define MPU9250_TEMP_OFFSET   (21)

#define ICM2060X_TEMP_SEN     (326.8)
#define ICM2060X_TEMP_OFFSET  (25)

/**
 * This function writes the value of the register for mpu6xxx
 *
 * @param dev the pointer of device driver structure
 * @param reg the register for mpu6xxx
 * @param data value to write
 *
 * @return the writing status, OS_EOK reprensents  writing the value of the register successfully.
 */
static os_err_t mpu6xxx_write_reg(struct mpu6xxx_device *dev, os_uint8_t reg, os_uint8_t data)
{
    os_int8_t res = 0;
#ifdef OS_USING_I2C
    struct os_i2c_msg msgs;
    os_uint8_t buf[2] = {reg, data};
#endif
    if (dev->bus->type == OS_DEVICE_TYPE_I2CBUS)
    {
#ifdef OS_USING_I2C
        msgs.addr  = dev->i2c_addr;    /* slave address */
        msgs.flags = OS_I2C_WR;        /* write flag */
        msgs.buf   = buf;              /* Send data pointer */
        msgs.len   = 2;

        if (os_i2c_transfer((struct os_i2c_bus_device *)dev->bus, &msgs, 1) == 1)
        {
            res = OS_EOK;
        }
        else
        {
            res = OS_ERROR;
        }
#endif
    }
    else
    {
#ifdef OS_USING_SPI
        res = os_spi_send_then_send((struct os_spi_device *)dev->bus, &reg, 1, &data, 1);
#endif
    }
    return res;
}

/**
 * This function reads the value of registers for mpu6xxx
 *
 * @param dev the pointer of device driver structure
 * @param reg the register for mpu6xxx
 * @param len number of register
 * @param buf read data pointer
 *
 * @return the reading status, OS_EOK reprensents  reading the value of registers successfully.
 */
static os_err_t mpu6xxx_read_regs(struct mpu6xxx_device *dev, os_uint8_t reg, os_uint8_t len, os_uint8_t *buf)
{
    os_int8_t res = 0;
#ifdef OS_USING_I2C
    struct os_i2c_msg msgs[2];
#endif
#ifdef OS_USING_SPI
    os_uint8_t tmp;
#endif
    if (dev->bus->type == OS_DEVICE_TYPE_I2CBUS)
    {
#ifdef OS_USING_I2C
        msgs[0].addr  = dev->i2c_addr;    /* Slave address */
        msgs[0].flags = OS_I2C_WR;        /* Write flag */
        msgs[0].buf   = &reg;             /* Slave register address */
        msgs[0].len   = 1;                /* Number of bytes sent */

        msgs[1].addr  = dev->i2c_addr;    /* Slave address */
        msgs[1].flags = OS_I2C_RD;        /* Read flag */
        msgs[1].buf   = buf;              /* Read data pointer */
        msgs[1].len   = len;              /* Number of bytes read */

        if (os_i2c_transfer((struct os_i2c_bus_device *)dev->bus, msgs, 2) == 2)
        {
            res = OS_EOK;
        }
        else
        {
            res = OS_ERROR;
        }
#endif
    }
    else
    {
#ifdef OS_USING_SPI
        //The first bit of the first byte contains the Read/Write bit and indicates the Read (1) or Write (0) operation.
        tmp = reg | 0x80;

        res = os_spi_send_then_recv((struct os_spi_device *)dev->bus, &tmp, 1, buf, len);
#endif
    }
    return res;
}

/**
 * This function writes a bit value of registers for mpu6xxx
 *
 * @param dev the pointer of device driver structure
 * @param reg the register for mpu6xxx
 * @param bit the position of the register
 * @param data value to write
 *
 * @return the writing status, OS_EOK reprensents  writing a bit value of registers successfully.
 */
static os_err_t mpu6xxx_write_bit(struct mpu6xxx_device *dev, os_uint8_t reg, os_uint8_t bit, os_uint8_t data)
{
    os_uint8_t byte;
    os_err_t res;

    res = mpu6xxx_read_regs(dev, reg, 1, &byte);
    if (res != OS_EOK)
    {
        return res;
    }

    byte = (data != 0) ? (byte | (1 << bit)) : (byte & ~(1 << bit));

    return mpu6xxx_write_reg(dev, reg, byte);
}

/**
 * This function reads a bit value of registers for mpu6xxx
 *
 * @param dev the pointer of device driver structure
 * @param reg the register for mpu6xxx
 * @param bit the position of the register
 * @param data read data pointer
 *
 * @return the reading status, OS_EOK reprensents  reading a bit value of registers successfully.
 */
static os_err_t mpu6xxx_read_bit(struct mpu6xxx_device *dev, os_uint8_t reg, os_uint8_t bit, os_uint8_t *data)
{
    os_uint8_t byte;
    os_err_t res;

    res = mpu6xxx_read_regs(dev, reg, 1, &byte);
    if (res != OS_EOK)
    {
        return res;
    }

    *data = byte & (1 << bit);

    return OS_EOK;
}

/**
 * This function writes multi-bit value of registers for mpu6xxx
 *
 * @param dev the pointer of device driver structure
 * @param reg the register for mpu6xxx
 * @param start_bit the start position of the register
 * @param len number of bits to write
 * @param data value to write
 *
 * @return the writing status, OS_EOK reprensents  writing multi-bit value of registers successfully.
 */
static os_err_t mpu6xxx_write_bits(struct mpu6xxx_device *dev, os_uint8_t reg, os_uint8_t start_bit, os_uint8_t len, os_uint8_t data)
{
    os_uint8_t byte, mask;
    os_err_t res;

    res = mpu6xxx_read_regs(dev, reg, 1, &byte);
    if (res != OS_EOK)
    {
        return res;
    }

    mask = ((1 << len) - 1) << (start_bit - len + 1);
    data <<= (start_bit - len + 1); // shift data into correct position
    data &= mask; // zero all non-important bits in data
    byte &= ~(mask); // zero all important bits in existing byte
    byte |= data; // combine data with existing byte

    return mpu6xxx_write_reg(dev, reg, byte);
}

/**
 * This function reads multi-bit value of registers for mpu6xxx
 *
 * @param dev the pointer of device driver structure
 * @param reg the register for mpu6xxx
 * @param start_bit the start position of the register
 * @param len number of bits to write
 * @param data read data pointer
 *
 * @return the reading status, OS_EOK reprensents  reading multi-bit value of registers successfully.
 */
static os_err_t mpu6xxx_read_bits(struct mpu6xxx_device *dev, os_uint8_t reg, os_uint8_t start_bit, os_uint8_t len, os_uint8_t *data)
{
    os_uint8_t byte, mask;
    os_err_t res;

    res = mpu6xxx_read_regs(dev, reg, 1, &byte);
    if (res != OS_EOK)
    {
        return res;
    }

    mask = ((1 << len) - 1) << (start_bit - len + 1);
    byte &= mask;
    byte >>= (start_bit - len + 1);
    *data = byte;

    return OS_EOK;
}

/**
 * This function gets the raw data of the accelerometer
 *
 * @param dev the pointer of device driver structure
 * @param accel the pointer of 3axes structure for receive data
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
static os_err_t mpu6xxx_get_accel_raw(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *accel)
{
    os_uint8_t buffer[6];
    os_err_t res;

    res = mpu6xxx_read_regs(dev, MPU6XXX_RA_ACCEL_XOUT_H, 6, buffer);
    if (res != OS_EOK)
    {
        return res;
    }

    accel->x = ((os_uint16_t)buffer[0] << 8) + buffer[1];
    accel->y = ((os_uint16_t)buffer[2] << 8) + buffer[3];
    accel->z = ((os_uint16_t)buffer[4] << 8) + buffer[5];

    return OS_EOK;
}

/**
 * This function gets the raw data of the gyroscope
 *
 * @param dev the pointer of device driver structure
 * @param gyro the pointer of 3axes structure for receive data
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
static os_err_t mpu6xxx_get_gyro_raw(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *gyro)
{
    os_uint8_t buffer[6];
    os_err_t res;

    res = mpu6xxx_read_regs(dev, MPU6XXX_RA_GYRO_XOUT_H, 6, buffer);
    if (res != OS_EOK)
    {
        return res;
    }

    gyro->x = ((os_uint16_t)buffer[0] << 8) + buffer[1];
    gyro->y = ((os_uint16_t)buffer[2] << 8) + buffer[3];
    gyro->z = ((os_uint16_t)buffer[4] << 8) + buffer[5];

    return OS_EOK;
}

/**
 * This function gets the raw data of the temperature
 *
 * @param dev the pointer of device driver structure
 * @param temp read data pointer
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
static os_err_t mpu6xxx_get_temp_raw(struct mpu6xxx_device *dev, os_int16_t *temp)
{
    os_uint8_t buffer[2];
    os_err_t res;

    res = mpu6xxx_read_regs(dev, MPU6XXX_RA_TEMP_OUT_H, 2, buffer);
    if (res != OS_EOK)
    {
        return res;
    }

    *temp = ((os_uint16_t)buffer[0] << 8) + buffer[1];

    return OS_EOK;
}

/**
 * This function gets mpu6xxx parameters.
 *
 * @param dev the pointer of device driver structure
 * @param cmd Configuration item
 * @param param read data pointer
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
static os_err_t mpu6xxx_get_param(struct mpu6xxx_device *dev, enum mpu6xxx_cmd cmd, os_uint16_t *param)
{
    os_uint8_t data = 0;
    os_err_t res = OS_EOK;

    OS_ASSERT(dev);

    switch (cmd)
    {
    case MPU6XXX_GYRO_RANGE:  /* Gyroscope full scale range */
        res = mpu6xxx_read_bits(dev, MPU6XXX_RA_GYRO_CONFIG, MPU6XXX_GCONFIG_FS_SEL_BIT, MPU6XXX_GCONFIG_FS_SEL_LENGTH, &data);
        *param = data;
        break;
    case MPU6XXX_ACCEL_RANGE: /* Accelerometer full scale range */
        res = mpu6xxx_read_bits(dev, MPU6XXX_RA_ACCEL_CONFIG, MPU6XXX_ACONFIG_AFS_SEL_BIT, MPU6XXX_ACONFIG_AFS_SEL_LENGTH, &data);
        *param = data;
        break;
    case MPU6XXX_DLPF_CONFIG: /* Digital Low Pass Filter */
        res = mpu6xxx_read_bits(dev, MPU6XXX_RA_CONFIG, MPU6XXX_CFG_DLPF_CFG_BIT, MPU6XXX_CFG_DLPF_CFG_LENGTH, &data);
        *param = data;
        break;
    case MPU6XXX_SAMPLE_RATE: /* Sample Rate */
        /* Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV) */
        res = mpu6xxx_read_bits(dev, MPU6XXX_RA_CONFIG, MPU6XXX_CFG_DLPF_CFG_BIT, MPU6XXX_CFG_DLPF_CFG_LENGTH, &data);
        if (res != OS_EOK)
        {
            break;
        }

        if (data == 0 || data == 7) /* dlpf is disable */
        {
            res = mpu6xxx_read_regs(dev, MPU6XXX_RA_SMPLRT_DIV, 1, &data);
            *param = 8000 / (data + 1);
        }
        else /* dlpf is enable */
        {
            res = mpu6xxx_read_regs(dev, MPU6XXX_RA_SMPLRT_DIV, 1, &data);
            *param = 1000 / (data + 1);
        }
        break;
    case MPU6XXX_SLEEP: /* sleep mode */
        res = mpu6xxx_read_bit(dev, MPU6XXX_RA_PWR_MGMT_1, MPU6XXX_PWR1_SLEEP_BIT, &data);
        *param = data;
        break;
    }

    return res;
}

/**
 * This function set mpu6xxx parameters.
 *
 * @param dev the pointer of device driver structure
 * @param cmd Configuration item
 * @param param Configuration item parameter
 *
 * @return the setting status, OS_EOK reprensents  setting the parameter successfully.
 */
os_err_t mpu6xxx_set_param(struct mpu6xxx_device *dev, enum mpu6xxx_cmd cmd, os_uint16_t param)
{
    os_uint8_t data = 0;
    os_err_t res = OS_EOK;

    OS_ASSERT(dev);

    switch (cmd)
    {
    case MPU6XXX_GYRO_RANGE:  /* Gyroscope full scale range */
        res = mpu6xxx_write_bits(dev, MPU6XXX_RA_GYRO_CONFIG, MPU6XXX_GCONFIG_FS_SEL_BIT, MPU6XXX_GCONFIG_FS_SEL_LENGTH, param);
        dev->config.gyro_range = param;
        break;
    case MPU6XXX_ACCEL_RANGE: /* Accelerometer full scale range */
        res = mpu6xxx_write_bits(dev, MPU6XXX_RA_ACCEL_CONFIG, MPU6XXX_ACONFIG_AFS_SEL_BIT, MPU6XXX_ACONFIG_AFS_SEL_LENGTH, param);
        dev->config.accel_range = param;
        break;
    case MPU6XXX_DLPF_CONFIG: /* Digital Low Pass Filter */
        res = mpu6xxx_write_bits(dev, MPU6XXX_RA_CONFIG, MPU6XXX_CFG_DLPF_CFG_BIT, MPU6XXX_CFG_DLPF_CFG_LENGTH, param);
        break;
    case MPU6XXX_SAMPLE_RATE: /* Sample Rate â€”â€?16-bit unsigned value.
                                 Sample Rate = [1000 -  4]HZ when dlpf is enable
                                 Sample Rate = [8000 - 32]HZ when dlpf is disable */

        //Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
        res = mpu6xxx_read_bits(dev, MPU6XXX_RA_CONFIG, MPU6XXX_CFG_DLPF_CFG_BIT, MPU6XXX_CFG_DLPF_CFG_LENGTH, &data);
        if (res != OS_EOK)
        {
            break;
        }

        if (data == 0 || data == 7) /* dlpf is disable */
        {
            if (param > 8000)
                data = 0;
            else if (param < 32)
                data = 0xFF;
            else
                data = 8000 / param - 1;
        }
        else /* dlpf is enable */
        {
            if (param > 1000)
                data = 0;
            else if (param < 4)
                data = 0xFF;
            else
                data = 1000 / param - 1;
        }
        res = mpu6xxx_write_reg(dev, MPU6XXX_RA_SMPLRT_DIV, data);
        break;
    case MPU6XXX_SLEEP: /* Configure sleep mode */
        res = mpu6xxx_write_bit(dev, MPU6XXX_RA_PWR_MGMT_1, MPU6XXX_PWR1_SLEEP_BIT, param);
        break;
    }

    return res;
}

/**
* This function gets the data of the accelerometer, unit: mg
 *
 * @param dev the pointer of device driver structure
 * @param accel the pointer of 3axes structure for receive data
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
os_err_t mpu6xxx_get_accel(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *accel)
{
    struct mpu6xxx_3axes tmp;
    os_uint16_t sen;
    os_err_t res;

    res = mpu6xxx_get_accel_raw(dev, &tmp);
    if (res != OS_EOK)
    {
        return res;
    }

    sen = MPU6XXX_ACCEL_SEN >> dev->config.accel_range;

    accel->x = (os_int32_t)tmp.x * 1000 / sen;
    accel->y = (os_int32_t)tmp.y * 1000 / sen;
    accel->z = (os_int32_t)tmp.z * 1000 / sen;

    return OS_EOK;
}

/**
* This function gets the data of the gyroscope, unit: deg/10s
 *
 * @param dev the pointer of device driver structure
 * @param gyro the pointer of 3axes structure for receive data
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
os_err_t mpu6xxx_get_gyro(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *gyro)
{
    struct mpu6xxx_3axes tmp;
    os_uint16_t sen;
    os_err_t res;

    res = mpu6xxx_get_gyro_raw(dev, &tmp);
    if (res != OS_EOK)
    {
        return res;
    }

    sen = MPU6XXX_GYRO_SEN >> dev->config.gyro_range;

    gyro->x = (os_int32_t)tmp.x * 100 / sen;
    gyro->y = (os_int32_t)tmp.y * 100 / sen;
    gyro->z = (os_int32_t)tmp.z * 100 / sen;

    return OS_EOK;
}

/**
 * This function gets the data of the temperature, unit: Centigrade
 *
 * @param dev the pointer of device driver structure
 * @param temp read data pointer
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
os_err_t mpu6xxx_get_temp(struct mpu6xxx_device *dev, float *temp)
{
    os_int16_t tmp;
    os_err_t res;

    res = mpu6xxx_get_temp_raw(dev, &tmp);
    if (res != OS_EOK)
    {
        return res;
    }

    if (dev->id == MPU6050_WHO_AM_I)
    {
        /* mpu60x0: Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity)/340 + 36.5 */
        *temp = (double)tmp / MPU60X0_TEMP_SEN + MPU60X0_TEMP_OFFSET;
    }
    else if (dev->id == MPU6500_WHO_AM_I)
    {
        /* mpu6500:  ((TEMP_OUT â€?RoomTemp_Offset)/Temp_Sensitivity)+ 21degC */
        *temp = (double)tmp / MPU6500_TEMP_SEN + MPU6500_TEMP_OFFSET;
    }
    else if (dev->id == MPU9250_WHO_AM_I)
    {
        /* mpu9250:  ((TEMP_OUT â€?RoomTemp_Offset)/Temp_Sensitivity)+ 21degC */
        *temp = (double)tmp / MPU9250_TEMP_SEN + MPU9250_TEMP_OFFSET;
    }
    else
    {
        /* icm2060x: ((TEMP_OUT â€?RoomTemp_Offset)/Temp_Sensitivity)+ 25degC */
        *temp = (double)tmp / ICM2060X_TEMP_SEN + ICM2060X_TEMP_OFFSET;
    }

    return OS_EOK;
}

/**
 * This function initialize the mpu6xxx device.
 *
 * @param dev_name the name of transfer device
 * @param param the i2c device address for i2c communication, OS_NULL for spi
 *
 * @return the pointer of device driver structure, OS_NULL reprensents  initialization failed.
 */
struct mpu6xxx_device *mpu6xxx_init(const char *dev_name, os_uint8_t param)
{
    struct mpu6xxx_device *dev = OS_NULL;
    os_uint8_t reg = 0xFF, res = OS_EOK;

    OS_ASSERT(dev_name);

    dev = os_calloc(1, sizeof(struct mpu6xxx_device));
    if (dev == OS_NULL)
    {
        LOG_E(DBG_TAG, "Can't allocate memory for mpu6xxx device on '%s' ", dev_name);
        goto __exit;
    }

    dev->bus = os_device_find(dev_name);
    if (dev->bus == OS_NULL)
    {
        LOG_E(DBG_TAG, "Can't find device:'%s'", dev_name);
        goto __exit;
    }

    if (dev->bus->type == OS_DEVICE_TYPE_I2CBUS)
    {
        if (param != 0/*OS_NULL*/)
        {
            dev->i2c_addr = param;
        }
        else
        {
            /* find mpu6xxx device at address: 0x68 */
            dev->i2c_addr = MPU6XXX_ADDRESS_AD0_LOW;
            if (mpu6xxx_read_regs(dev, MPU6XXX_RA_WHO_AM_I, 1, &reg) != OS_EOK)
            {
                /* find mpu6xxx device at address 0x69 */
                dev->i2c_addr = MPU6XXX_ADDRESS_AD0_HIGH;
                if (mpu6xxx_read_regs(dev, MPU6XXX_RA_WHO_AM_I, 1, &reg) != OS_EOK)
                {
                    LOG_E(DBG_TAG, "Can't find device at '%s'!", dev_name);
                    goto __exit;
                }
            }
            LOG_D(DBG_TAG, "Device i2c address is:'0x%x'!", dev->i2c_addr);
        }
    }
    else if (dev->bus->type == OS_DEVICE_TYPE_SPIBUS)
    {
#if defined(OS_USING_SPI) && defined(OS_MPU6XXX_BUS_SPI)
        struct os_spi_configuration cfg;

        os_hw_spi_device_attach(dev_name, OS_MPU6XXX_SPI_ATTACH_NAME, OS_MPU6XXX_CS_PIN);
        dev->bus = os_device_find(OS_MPU6XXX_SPI_ATTACH_NAME);
        cfg.data_width = 8;
        cfg.mode = OS_SPI_MASTER | OS_SPI_MODE_0 | OS_SPI_MSB;
        cfg.max_hz = MPU60X0_SPI_MAX_SPEED; /* Set spi max speed */

        os_spi_configure((struct os_spi_device *)dev->bus, &cfg);
#endif
    }
    else
    {
        LOG_E(DBG_TAG, "Unsupported device:'%s'!", dev_name);
        goto __exit;
    }

    if (mpu6xxx_read_regs(dev, MPU6XXX_RA_WHO_AM_I, 1, &reg) != OS_EOK)
    {
        LOG_E(DBG_TAG, "Failed to read device id!");
        goto __exit;
    }

    dev->id = reg;

    switch (dev->id)
    {
    case MPU6050_WHO_AM_I:
        LOG_I(DBG_TAG, "Find device: mpu6050!");
        break;
    case MPU6500_WHO_AM_I:
        LOG_I(DBG_TAG, "Find device: mpu6500!");
        break;
    case MPU9250_WHO_AM_I:
        LOG_I(DBG_TAG, "Find device: mpu9250!");
        break;
    case ICM20608G_WHO_AM_I:
    case ICM20608D_WHO_AM_I:
        LOG_I(DBG_TAG, "Find device: icm20608!");
        break;
    case ICM20602_WHO_AM_I:
        LOG_I(DBG_TAG, "Find device: icm20602!");
        break;
    case 0xFF:
        LOG_E(DBG_TAG, "No device connection!");
        goto __exit;
    default:
        LOG_W(DBG_TAG, "Unknown device id: 0x%x!", reg);
    }

    res += mpu6xxx_get_param(dev, MPU6XXX_ACCEL_RANGE, &dev->config.accel_range);
    res += mpu6xxx_get_param(dev, MPU6XXX_GYRO_RANGE, &dev->config.gyro_range);

    res += mpu6xxx_write_bits(dev, MPU6XXX_RA_PWR_MGMT_1, MPU6XXX_PWR1_CLKSEL_BIT, MPU6XXX_PWR1_CLKSEL_LENGTH, MPU6XXX_CLOCK_PLL_XGYRO);
    res += mpu6xxx_set_param(dev, MPU6XXX_GYRO_RANGE, MPU6XXX_GYRO_RANGE_250DPS);
    res += mpu6xxx_set_param(dev, MPU6XXX_ACCEL_RANGE, MPU6XXX_ACCEL_RANGE_2G);
    res += mpu6xxx_set_param(dev, MPU6XXX_SLEEP, MPU6XXX_SLEEP_DISABLE);

    if (res == OS_EOK)
    {
        LOG_I(DBG_TAG, "Device init succeed!");
    }
    else
    {
        LOG_W(DBG_TAG, "Error in device initialization!");
    }
    return dev;

__exit:
    if (dev != OS_NULL)
    {
        os_free(dev);
    }
    return OS_NULL;
}

/**
 * This function releases memory
 *
 * @param dev the pointer of device driver structure
 */
void mpu6xxx_deinit(struct mpu6xxx_device *dev)
{
    OS_ASSERT(dev);

    os_free(dev);
}

OS_UNUSED static void mpu6xxx_test(int argc, char **argv)
{
    static struct mpu6xxx_device *dev = OS_NULL;

    /* If the number of arguments less than 2 */
    if (argc < 2)
    {
        os_kprintf("\n");
        os_kprintf("mpu6xxx [OPTION] [PARAM]\n");
        os_kprintf("         probe <dev_name>      Probe mpu6xxx by given name\n");
        os_kprintf("         sr <var>              Set sample rate to var\n");
        os_kprintf("                               var = [1000 -  4] when dlpf is enable\n");
        os_kprintf("                               var = [8000 - 32] when dlpf is disable\n");
        os_kprintf("         gr <var>              Set gyro range to var\n");
        os_kprintf("                               var = [0 - 3] means [250 - 2000DPS]\n");
        os_kprintf("         ar <var>              Set accel range to var\n");
        os_kprintf("                               var = [0 - 3] means [2 - 16G]\n");
        os_kprintf("         sleep <var>           Set sleep status\n");
        os_kprintf("                               var = 0 means disable, = 1 means enable\n");
        os_kprintf("         read [num]            read [num] times mpu6xxx\n");
        os_kprintf("                               num default 5\n");
        return ;
    }
    else if (!strcmp(argv[1], "read"))
    {
        struct mpu6xxx_3axes accel, gyro;
        float temp;
        uint16_t num = 5;

        if (dev == OS_NULL)
        {
            os_kprintf("Please probe mpu6xxx first!\n");
            return ;
        }
        if (argc == 3)
        {
            num = atoi(argv[2]);
        }

        while (num --)
        {
            mpu6xxx_get_accel(dev, &accel);
            mpu6xxx_get_gyro(dev, &gyro);
            mpu6xxx_get_temp(dev, &temp);

            os_kprintf("accel.x = %4d, accel.y = %4d, accel.z = %4d ", accel.x, accel.y, accel.z);
            os_kprintf("gyro.x = %4d gyro.y = %4d, gyro.z = %4d, ", gyro.x, gyro.y, gyro.z);
            os_kprintf("temp = %d.%d\n", (int)(temp * 100) / 100, (int)(temp * 100) % 100);

            os_task_msleep(100);
        }
    }
    else if (argc == 3)
    {
        if (!strcmp(argv[1], "probe"))
        {
            if (dev)
            {
                mpu6xxx_deinit(dev);
            }
            dev = mpu6xxx_init(argv[2], 0/*OS_NULL*/);
        }
        else if (dev == OS_NULL)
        {
            os_kprintf("Please probe mpu6xxx first!\n");
            return ;
        }
        else if (!strcmp(argv[1], "sr"))
        {
            mpu6xxx_set_param(dev, MPU6XXX_SAMPLE_RATE, atoi(argv[2]));
        }
        else if (!strcmp(argv[1], "sleep"))
        {
            mpu6xxx_set_param(dev, MPU6XXX_SLEEP, atoi(argv[2]));
        }
        else if (!strcmp(argv[1], "gr"))
        {
            mpu6xxx_set_param(dev, MPU6XXX_GYRO_RANGE, atoi(argv[2]));
        }
        else if (!strcmp(argv[1], "ar"))
        {
            mpu6xxx_set_param(dev, MPU6XXX_ACCEL_RANGE, atoi(argv[2]));
        }
        else
        {
            os_kprintf("Unknown command, please enter 'mpu6xxx' get help information!\n");
        }
    }
    else
    {
        os_kprintf("Unknown command, please enter 'mpu6xxx' get help information!\n");
    }
}
//#ifdef OS_USING_SHELL
    SH_CMD_EXPORT(mpu6xxx, mpu6xxx_test, "mpu6xxx sensor function");
//#endif
