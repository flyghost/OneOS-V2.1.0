/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-08     flybreak     the first version
 * 2020-03-20     OneOS Team   adapt the code to OneOS
 */

#include <os_memory.h>
#include <string.h>
#include <stdint.h>
#include "../inc/sensor_inven_mpu6xxx.h"

#define DRV_EXT_LVL         LOG_LVL_DEBUG
#define DRV_EXT_TAG         "sensor.inven.mpu6xxx" 
#define DBG_TAG             "sensor.inven.mpu6xxx"
#include <drv_log.h>

static struct mpu6xxx_device *mpu_dev;

static os_err_t _mpu6xxx_init(struct os_sensor_intf *intf)
{
    os_uint8_t  i2c_addr = (os_uint32_t)(intf->user_data) & 0xff;

    mpu_dev = mpu6xxx_init(intf->dev_name, i2c_addr);

    if (mpu_dev == OS_NULL)
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t _mpu6xxx_set_range(os_sensor_t sensor, os_int32_t range)
{
    if (sensor->info.type == OS_SENSOR_CLASS_ACCE)
    {
        uint8_t range_ctr;

        if (range < 2000)
            range_ctr = MPU6XXX_ACCEL_RANGE_2G;
        else if (range < 4000)
            range_ctr = MPU6XXX_ACCEL_RANGE_4G;
        else if (range < 8000)
            range_ctr = MPU6XXX_ACCEL_RANGE_8G;
        else
            range_ctr = MPU6XXX_ACCEL_RANGE_16G;

        LOG_D(DBG_TAG, "acce set range %d", range_ctr);

        return mpu6xxx_set_param(mpu_dev, MPU6XXX_ACCEL_RANGE, range_ctr);
    }
    else if (sensor->info.type == OS_SENSOR_CLASS_GYRO)
    {
        uint8_t range_ctr;

        if (range < 250000UL)
            range_ctr = MPU6XXX_GYRO_RANGE_250DPS;
        else if (range < 500000UL)
            range_ctr = MPU6XXX_GYRO_RANGE_500DPS;
        else if (range < 1000000UL)
            range_ctr = MPU6XXX_GYRO_RANGE_1000DPS;
        else
            range_ctr = MPU6XXX_GYRO_RANGE_2000DPS;

        LOG_D(DBG_TAG, "gyro set range %d", range);

        return mpu6xxx_set_param(mpu_dev, MPU6XXX_GYRO_RANGE, range_ctr);
    }
    return OS_EOK;
}

static os_err_t _mpu6xxx_acc_set_mode(os_sensor_t sensor, os_uint8_t mode)
{
    if (mode == OS_SENSOR_MODE_POLLING)
    {
        LOG_D(DBG_TAG, "set mode to POLLING");
    }
    else
    {
        LOG_D(DBG_TAG, "Unsupported mode, code is %d", mode);
        return OS_ERROR;
    }
    return OS_EOK;
}

static os_err_t _mpu6xxx_set_power(os_sensor_t sensor, os_uint8_t power)
{
    static uint8_t ref_count = 0;

    if (power == OS_SENSOR_POWER_DOWN)
    {
        if (ref_count > 0)
        {
            ref_count --;
        }
        if (ref_count == 0)
        {
            LOG_D(DBG_TAG, "set power down");
            return mpu6xxx_set_param(mpu_dev, MPU6XXX_SLEEP, MPU6XXX_SLEEP_ENABLE);
        }
        return OS_EOK;
    }
    else if (power == OS_SENSOR_POWER_NORMAL)
    {
        ref_count ++;
        LOG_D(DBG_TAG, "set power normal");
        return mpu6xxx_set_param(mpu_dev, MPU6XXX_SLEEP, MPU6XXX_SLEEP_DISABLE);
    }
    else
    {
        LOG_W(DBG_TAG, "Unsupported mode, code is %d", power);
        return OS_ERROR;
    }
}

static os_size_t _mpu6xxx_polling_get_data(os_sensor_t sensor, struct os_sensor_data *data)
{
    if (sensor->info.type == OS_SENSOR_CLASS_ACCE)
    {
        struct mpu6xxx_3axes acce;
        if (mpu6xxx_get_accel(mpu_dev, &acce) != OS_EOK)
        {
            return 0;
        }

        data->type = OS_SENSOR_CLASS_ACCE;
        data->data.acce.x = acce.x;
        data->data.acce.y = acce.y;
        data->data.acce.z = acce.z;
        data->timestamp = os_sensor_get_ts();
    }
    else if (sensor->info.type == OS_SENSOR_CLASS_GYRO)
    {
        struct mpu6xxx_3axes gyro;
        if (mpu6xxx_get_gyro(mpu_dev, &gyro) != OS_EOK)
        {
            return 0;
        }

        data->type = OS_SENSOR_CLASS_GYRO;
        data->data.gyro.x = gyro.x * 100;
        data->data.gyro.y = gyro.y * 100;
        data->data.gyro.z = gyro.z * 100;
        data->timestamp = os_sensor_get_ts();
    }
    else if (sensor->info.type == OS_SENSOR_CLASS_TEMP)
    {
        float temp;
        if (mpu6xxx_get_temp(mpu_dev, &temp) != OS_EOK)
        {
            return 0;
        }

        data->type = OS_SENSOR_CLASS_GYRO;
        data->data.temp = temp * 1000;
        data->timestamp = os_sensor_get_ts();
    }
    return 1;
}

static os_size_t mpu6xxx_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    OS_ASSERT(buf);

    if (sensor->config.mode == OS_SENSOR_MODE_POLLING)
    {
        return _mpu6xxx_polling_get_data(sensor, buf);
    }
    else
        return 0;
}

static os_err_t mpu6xxx_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t result = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = mpu_dev->id;
        break;
    case OS_SENSOR_CTRL_SET_RANGE:
        result = _mpu6xxx_set_range(sensor, (os_int32_t)args);
        break;
    case OS_SENSOR_CTRL_SET_ODR:
        result = OS_EINVAL;
        break;
    case OS_SENSOR_CTRL_SET_MODE:
        result = _mpu6xxx_acc_set_mode(sensor, (os_uint32_t)args & 0xff);
        break;
    case OS_SENSOR_CTRL_SET_POWER:
        result = _mpu6xxx_set_power(sensor, (os_uint32_t)args & 0xff);
        break;
    case OS_SENSOR_CTRL_SELF_TEST:
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops sensor_ops =
{
    mpu6xxx_fetch_data,
    mpu6xxx_control
};

int os_hw_mpu6xxx_init(const char *name, struct os_sensor_config *cfg)
{
    os_int8_t result;
    os_sensor_t sensor = OS_NULL;

    result = _mpu6xxx_init(&cfg->intf);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG, "_mpu6xxx init err code: %d", result);
        return OS_ERROR;
    }

#ifdef PKG_USING_MPU6XXX_ACCE
    /* accelerometer sensor register */
    {
        sensor = os_calloc(1, sizeof(struct os_sensor_device));
        OS_ASSERT(sensor != OS_NULL);

        sensor->info.type       = OS_SENSOR_CLASS_ACCE;
        sensor->info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
        sensor->info.model      = "mpu6xxx_acc";
        sensor->info.unit       = OS_SENSOR_UNIT_MG;
        #if defined(OS_MPU6XXX_BUS_SPI)
        sensor->info.intf_type  = OS_SENSOR_INTF_SPI;
        #else
        sensor->info.intf_type  = OS_SENSOR_INTF_I2C;
        #endif
        sensor->info.range_max  = 16000;
        sensor->info.range_min  = 2000;
        sensor->info.period_min = 5;

        memcpy(&sensor->config, cfg, sizeof(struct os_sensor_config));
        sensor->ops = &sensor_ops;

        result = os_hw_sensor_register(sensor, name, OS_NULL);
        OS_ASSERT(result == OS_EOK);
    }
#endif
#ifdef PKG_USING_MPU6XXX_GYRO
    /* gyroscope sensor register */
    {
        sensor = os_calloc(1, sizeof(struct os_sensor_device));
        OS_ASSERT(sensor != OS_NULL);

        sensor->info.type       = OS_SENSOR_CLASS_GYRO;
        sensor->info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
        sensor->info.model      = "mpu6xxx_gyro";
        sensor->info.unit       = OS_SENSOR_UNIT_MDPS;
        #if defined(OS_MPU6XXX_BUS_SPI)
        sensor->info.intf_type  = OS_SENSOR_INTF_SPI;
        #else
        sensor->info.intf_type  = OS_SENSOR_INTF_I2C;
        #endif
        sensor->info.range_max  = 2000000;
        sensor->info.range_min  = 250000;
        sensor->info.period_min = 5;

        memcpy(&sensor->config, cfg, sizeof(struct os_sensor_config));
        sensor->ops = &sensor_ops;

        result = os_hw_sensor_register(sensor, name, OS_NULL);
        OS_ASSERT(result == OS_EOK);
    }
#endif
#ifdef PKG_USING_MPU6XXX_TEMP
    /* temperature sensor register */
    {
        sensor = os_calloc(1, sizeof(struct os_sensor_device));
        OS_ASSERT(sensor != OS_NULL);

        sensor->info.type       = OS_SENSOR_CLASS_TEMP;
        sensor->info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
        sensor->info.model      = "mpu6xxx_temp";
        sensor->info.unit       = OS_SENSOR_UNIT_MDCELSIUS;
        #if defined(OS_MPU6XXX_BUS_SPI)
        sensor->info.intf_type  = OS_SENSOR_INTF_SPI;
        #else
        sensor->info.intf_type  = OS_SENSOR_INTF_I2C;
        #endif
        sensor->info.range_max  = 50000;
        sensor->info.range_min  = 0;
        sensor->info.period_min = 5;

        memcpy(&sensor->config, cfg, sizeof(struct os_sensor_config));
        sensor->ops = &sensor_ops;

        result = os_hw_sensor_register(sensor, name, OS_NULL);
        OS_ASSERT(result == OS_EOK);
    }
#endif

    LOG_I(DBG_TAG, "sensor init success");
    return OS_EOK;
}
