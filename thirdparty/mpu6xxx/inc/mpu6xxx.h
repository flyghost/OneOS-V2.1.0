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

#ifndef MPU6XXX_H_
#define MPU6XXX_H_

#include <os_task.h>
#include <device.h>

/* Accelerometer full scale range */
enum mpu6xxx_accel_range
{
    MPU6XXX_ACCEL_RANGE_2G  = 0, // ±2G
    MPU6XXX_ACCEL_RANGE_4G  = 1, // ±4G
    MPU6XXX_ACCEL_RANGE_8G  = 2, // ±8G
    MPU6XXX_ACCEL_RANGE_16G = 3  // ±16G
};

/* Gyroscope full scale range */
enum mpu6xxx_gyro_range
{
    MPU6XXX_GYRO_RANGE_250DPS  = 0, // ±250°/s
    MPU6XXX_GYRO_RANGE_500DPS  = 1, // ±500°/s
    MPU6XXX_GYRO_RANGE_1000DPS = 2, // ±1000°/s
    MPU6XXX_GYRO_RANGE_2000DPS = 3  // ±2000°/s
};

/* Digital Low Pass Filter parameters */
enum mpu6xxx_dlpf
{
    MPU6XXX_DLPF_DISABLE = 0, //256HZ
    MPU6XXX_DLPF_188HZ = 1,
    MPU6XXX_DLPF_98HZ  = 2,
    MPU6XXX_DLPF_42HZ  = 3,
    MPU6XXX_DLPF_20HZ  = 4,
    MPU6XXX_DLPF_10HZ  = 5,
    MPU6XXX_DLPF_5HZ   = 6
};

/* sleep mode parameters */
enum mpu6xxx_sleep
{
    MPU6XXX_SLEEP_DISABLE = 0,
    MPU6XXX_SLEEP_ENABLE  = 1
};

/* Supported configuration items */
enum mpu6xxx_cmd
{
    MPU6XXX_GYRO_RANGE,  /* Gyroscope full scale range */
    MPU6XXX_ACCEL_RANGE, /* Accelerometer full scale range */
    MPU6XXX_DLPF_CONFIG, /* Digital Low Pass Filter */
    MPU6XXX_SAMPLE_RATE, /* Sample Rate —�?16-bit unsigned value.
                            Sample Rate = [1000 -  4]HZ when dlpf is enable
                            Sample Rate = [8000 - 32]HZ when dlpf is disable */
    MPU6XXX_SLEEP        /* Sleep mode */
};

/* 3-axis data structure */
struct mpu6xxx_3axes
{
    os_int16_t x;
    os_int16_t y;
    os_int16_t z;
};

/* mpu6xxx config structure */
struct mpu6xxx_config
{
    os_uint16_t accel_range;
    os_uint16_t gyro_range;
};

/* mpu6xxx device structure */
struct mpu6xxx_device
{
    os_device_t *bus;
    os_uint8_t id;
    os_uint8_t i2c_addr;
    struct mpu6xxx_config config;
};

/**
 * This function initialize the mpu6xxx device.
 *
 * @param dev_name the name of transfer device
 * @param param the i2c device address for i2c communication, OS_NULL for spi
 *
 * @return the pointer of device driver structure, OS_NULL reprensents  initialization failed.
 */
struct mpu6xxx_device *mpu6xxx_init(const char *dev_name, os_uint8_t param);

/**
 * This function releases memory
 *
 * @param dev the pointer of device driver structure
 */
void mpu6xxx_deinit(struct mpu6xxx_device *dev);

/**
 * This function set mpu6xxx parameters.
 *
 * @param dev the pointer of device driver structure
 * @param cmd Configuration item
 * @param param Configuration item parameter
 *
 * @return the setting status, OS_EOK reprensents  setting the parameter successfully.
 */
os_err_t mpu6xxx_set_param(struct mpu6xxx_device *dev, enum mpu6xxx_cmd cmd, os_uint16_t param);

/**
* This function gets the data of the accelerometer, unit: mg
 *
 * @param dev the pointer of device driver structure
 * @param accel the pointer of 3axes structure for receive data
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
os_err_t mpu6xxx_get_accel(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *accel);

/**
* This function gets the data of the gyroscope, unit: deg/10s
 *
 * @param dev the pointer of device driver structure
 * @param gyro the pointer of 3axes structure for receive data
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
os_err_t mpu6xxx_get_gyro(struct mpu6xxx_device *dev, struct mpu6xxx_3axes *gyro);

/**
 * This function gets the data of the temperature, unit: Centigrade
 *
 * @param dev the pointer of device driver structure
 * @param temp read data pointer
 *
 * @return the reading status, OS_EOK reprensents  reading the data successfully.
 */
os_err_t mpu6xxx_get_temp(struct mpu6xxx_device *dev, float *temp);

#endif
