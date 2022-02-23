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
 * @file        aht10.c
 *
 * @brief       This file provides functions for aht10.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_memory.h>
#include <sensors/sensor.h>
#include <math.h>

#define DBG_EXT_TAG "sensor.sensirion.aht10"
#include <dlog.h>

#define AHT10_CALIBRATION_CMD 0xE1 /* Calibration cmd for measuring */
#define AHT10_NORMAL_CMD      0xA8 /* Normal cmd */
#define AHT10_GET_DATA        0xAC /* Get data cmd */

typedef struct
{
    struct os_sensor_device sensor;
    struct os_i2c_client    i2c;
    os_uint8_t              id;

    union
    {
        float tempreture;
        float humidity;
    } value;
    os_mutex_t *lock;
} aht10_info_t;

static unsigned char aht10_read_user_reg(aht10_info_t *aht10, os_uint8_t *buf, os_uint8_t len)
{
    return os_i2c_client_read(&aht10->i2c, 0, 0, buf, len);
}

static unsigned char aht10_write_user_reg(aht10_info_t *aht10, os_uint8_t reg, os_uint8_t *data)
{
    return os_i2c_client_write(&aht10->i2c, reg, 1, data, 2);
}

static os_err_t calibration_enabled(aht10_info_t *aht10)
{
    os_uint8_t val = 0;

    aht10_read_user_reg(aht10, &val, 1);

    if ((val & 0x68) == 0x08)
        return OS_EOK;
    else
        return OS_ERROR;
}

static aht10_info_t *aht10_init(const char *bus_name, os_uint16_t addr)
{
    os_uint8_t    temp[2] = {0, 0};
    aht10_info_t *aht10   = OS_NULL;

    aht10 = os_calloc(1, sizeof(aht10_info_t));
    if (aht10 == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht10 amlloc faile");
        return OS_NULL;
    }

    LOG_I(DBG_EXT_TAG,"aht10:[%s][0x%02x]", bus_name, addr);

    aht10->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (aht10->i2c.bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht10 i2c invalid.");
        os_free(aht10);
        return OS_NULL;
    }

    aht10->lock = os_mutex_create("mutex_aht10", OS_FALSE);
    if (aht10->lock == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"Can't create mutex for aht10 device on '%s' ", bus_name);
        os_free(aht10);
        return OS_NULL;
    }

    aht10->i2c.client_addr = addr;

    aht10_write_user_reg(aht10, AHT10_NORMAL_CMD, temp);
    os_task_msleep(500); /* At least 300 ms */

    temp[0] = 0x08;
    temp[1] = 0x00;
    aht10_write_user_reg(aht10, AHT10_CALIBRATION_CMD, temp);
    os_task_msleep(450); /* At least 300 ms */

    return aht10;
}

static os_size_t aht10_temp_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    aht10_info_t          *aht10 = OS_NULL;
    struct os_sensor_data *data  = OS_NULL;
    os_uint8_t             temp[6];
    os_err_t               result;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_TEMP);
    OS_ASSERT(buf);

    aht10 = (aht10_info_t *)sensor;
    data  = (struct os_sensor_data *)buf;

    result = os_mutex_lock(aht10->lock, OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        os_uint8_t cmd[2] = {0, 0};
        aht10_write_user_reg(aht10, AHT10_GET_DATA, cmd);

        result = calibration_enabled(aht10);
        if (result != OS_EOK)
        {
            LOG_E(DBG_EXT_TAG,"The aht10 is under an abnormal status. Please try again");
        }
        else
        {
            aht10_read_user_reg(aht10, temp, 6);
            /* Sensor temperature converse to reality */
            aht10->value.tempreture =
                (((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50) * 1000;
        }
    }
    else
    {
        LOG_E(DBG_EXT_TAG,"The aht10 could not respond temperature measurement at this time. Please try again");
    }
    os_mutex_unlock(aht10->lock);

    data->type      = sensor->info.type;
    data->data.temp = aht10->value.tempreture;
    data->timestamp = os_sensor_get_ts();
    return 0;
}

static os_err_t aht10_temp_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t      result = OS_EOK;
    aht10_info_t *aht10  = (aht10_info_t *)sensor;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(os_uint8_t *)args = aht10->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops aht10_temp_ops =
{
    aht10_temp_fetch_data,
    aht10_temp_control,
};

static int os_hw_aht10_temp_init(void)
{
    os_int8_t     result;
    aht10_info_t *aht10;

    aht10 = aht10_init(OS_AHT10_I2C_BUS_NAME, OS_AHT10_I2C_ADDR);
    if (aht10 == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht10 temp init failed.");
        goto __exit;
    }

    /* Temp */
    aht10->sensor.info.type       = OS_SENSOR_CLASS_TEMP;
    aht10->sensor.info.vendor     = OS_SENSOR_VENDOR_SENSIRION;
    aht10->sensor.info.model      = "aht10";
    aht10->sensor.info.unit       = OS_SENSOR_UNIT_MDCELSIUS;
    aht10->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    aht10->sensor.info.range_max  = 100000;
    aht10->sensor.info.range_min  = 0;
    aht10->sensor.info.period_min = 300;
    aht10->sensor.ops             = &aht10_temp_ops;

    result = os_hw_sensor_register(&aht10->sensor, "aht10", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG,"device temp register err code: %d", result);
        goto __exit;
    }

    struct os_sensor_data sensor_data;
    aht10_temp_fetch_data(&aht10->sensor, &sensor_data, sizeof(sensor_data));

    LOG_I(DBG_EXT_TAG,"aht10 temp init success");
    return OS_EOK;

__exit:
    if (aht10)
        os_free(aht10);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_aht10_temp_init, OS_INIT_SUBLEVEL_LOW);

static os_size_t aht10_humi_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    aht10_info_t          *aht10 = OS_NULL;
    struct os_sensor_data *data  = OS_NULL;
    os_uint8_t             humi[6];
    os_err_t               result;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_HUMI);
    OS_ASSERT(buf);

    aht10 = (aht10_info_t *)sensor;
    data  = (struct os_sensor_data *)buf;

    result = os_mutex_lock(aht10->lock, OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        os_uint8_t cmd[2] = {0, 0};
        aht10_write_user_reg(aht10, AHT10_GET_DATA, cmd);

        result = calibration_enabled(aht10);
        if (result != OS_EOK)
        {
            LOG_E(DBG_EXT_TAG,"The aht10 is under an abnormal status. Please try again");
        }
        else
        {
            aht10_read_user_reg(aht10, humi, 6);
            /* Sensor humidity converse to reality */
            aht10->value.humidity = (humi[1] << 12 | humi[2] << 4 | (humi[3] & 0xf0) >> 4) * 100.0 / (1 << 20) *1000;
        }
    }
    else
    {
        LOG_E(DBG_EXT_TAG,"The aht10 could not respond humidity measurement at this time. Please try again");
    }
    os_mutex_unlock(aht10->lock);

    data->type      = sensor->info.type;
    data->data.humi = aht10->value.humidity;
    data->timestamp = os_sensor_get_ts();

    return 0;
}

static os_err_t aht10_humi_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t      result = OS_EOK;
    aht10_info_t *aht10  = (aht10_info_t *)sensor;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(os_uint8_t *)args = aht10->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops aht10_humi_ops =
{
    aht10_humi_fetch_data,
    aht10_humi_control,
};

static int os_hw_aht10_humi_init(void)
{
    os_int8_t     result;
    aht10_info_t *aht10;

    aht10 = aht10_init(OS_AHT10_I2C_BUS_NAME, OS_AHT10_I2C_ADDR);
    if (aht10 == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht10 baro init failed.");
        goto __exit;
    }

    /* Baro */
    aht10->sensor.info.type       = OS_SENSOR_CLASS_HUMI;
    aht10->sensor.info.vendor     = OS_SENSOR_VENDOR_SENSIRION;
    aht10->sensor.info.model      = "aht10";
    aht10->sensor.info.unit       = OS_SENSOR_UNIT_MPERMILLAGE;
    aht10->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    aht10->sensor.info.range_max  = 100000;
    aht10->sensor.info.range_min  = 0;
    aht10->sensor.info.period_min = 300;
    aht10->sensor.ops             = &aht10_humi_ops;

    result = os_hw_sensor_register(&aht10->sensor, "aht10", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG,"device baro register err code: %d", result);
        goto __exit;
    }

    struct os_sensor_data sensor_data;
    aht10_humi_fetch_data(&aht10->sensor, &sensor_data, sizeof(sensor_data));

    LOG_I(DBG_EXT_TAG,"aht10 baro init success");
    return OS_EOK;

__exit:
    if (aht10)
        os_free(aht10);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_aht10_humi_init, OS_INIT_SUBLEVEL_LOW);
