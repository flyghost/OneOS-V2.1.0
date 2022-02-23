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
 * @file        aht20.c
 *
 * @brief       This file provides functions for aht20.
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

#define DBG_EXT_TAG "sensor.sensirion.aht20"
#include <dlog.h>

#if 0
#define AHT20_CALIBRATION_CMD 0xE1 /* INITIAL */
#define AHT20_NORMAL_CMD      0xA8 /* Normal cmd */
#define AHT20_GET_DATA        0xAC /* Start Measure */
#define AHT20_SOFTWARE_RESET  0xBA /* Software Reset */
#else
#define AHT20_CALIBRATION_CMD 0x71 /* Calibration cmd for measuring */
#define AHT20_NORMAL_CMD      0xBE /* Normal cmd */
#define AHT20_GET_DATA        0xAC /* START MEASURE */
#endif

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
} aht20_info_t;

static unsigned char aht20_read_user_reg(aht20_info_t *aht20, os_uint8_t *buf, os_uint8_t len)
{
    return os_i2c_client_read(&aht20->i2c, 0, 0, buf, len);
}

static unsigned char aht20_write_user_reg(aht20_info_t *aht20, os_uint8_t reg, os_uint8_t *data)
{
    return os_i2c_client_write(&aht20->i2c, reg, 1, data, 2);
}

static unsigned char aht20_write_user_reg_with_length(aht20_info_t *aht20, os_uint8_t reg, os_uint8_t *data, os_uint8_t len)
{
    return os_i2c_client_write(&aht20->i2c, reg, 1, data, len);
}

static os_err_t calibration_enabled(aht20_info_t *aht20)
{
    os_uint8_t val = 0;
    os_uint8_t reg = 0;

//    aht20_read_user_reg(aht20, &val, 1);
    reg = 0x00;
    os_i2c_client_read(&aht20->i2c, reg, 1, &val, 1);

    if ((val & 0x68) == 0x08)
        return OS_EOK;
    else
        return OS_ERROR;
}


#define ATH20_SLAVE_ADDRESS    0x38		/* I2C从机地址 */

//****************************************
// 定义 AHT20 内部地址
//****************************************
#define	INIT		    0xBE	//初始化
#define	SoftReset		0xBA	//软复位
#define	StartTest		0xAC	//开始测试
uint8_t count;
static aht20_info_t *aht20_init(const char *bus_name, os_uint16_t addr)
{
    uint8_t tmp[10];
    aht20_info_t *aht20   = OS_NULL;
    
    os_err_t result = 0;

    aht20 = os_calloc(1, sizeof(aht20_info_t));
    if (aht20 == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht20 amlloc faile");
        return OS_NULL;
    }

    LOG_I(DBG_EXT_TAG,"aht20:[%s][0x%02x]", bus_name, addr);

    aht20->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (aht20->i2c.bus == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht20 i2c invalid.");
        os_free(aht20);
        return OS_NULL;
    }

    aht20->lock = os_mutex_create("mutex_aht20", OS_FALSE);
    if (aht20->lock == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"Can't create mutex for aht20 device on '%s' ", bus_name);
        os_free(aht20);
        return OS_NULL;
    }

    aht20->i2c.client_addr = ATH20_SLAVE_ADDRESS;
    
    
    os_task_msleep(40);

    tmp[0] = 0x08;
    tmp[1] = 0x00;
    aht20_write_user_reg(aht20, INIT, tmp);
    os_task_msleep(500);
    os_uint8_t reg = 0;
    os_i2c_client_write(&aht20->i2c, reg, 1, NULL, 0);
    
    count = 0;
    result = calibration_enabled(aht20);
    while(result != OS_EOK)//需要等待状态字status的Bit[3]=1时才去读数据。如果Bit[3]不等于1 ，发软件复位0xBA给AHT10，再重新初始化AHT10，直至Bit[3]=1
    {
        aht20_write_user_reg_with_length(aht20, SoftReset, tmp , 0);
        os_task_msleep(200);

        aht20_write_user_reg(aht20, INIT, tmp);

        count++;
        if(count >= 10)
            return 0;
        os_task_msleep(500);
        result = calibration_enabled(aht20);
    }
    
    return aht20;
}

static os_size_t aht20_temp_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    aht20_info_t          *aht20 = OS_NULL;
    struct os_sensor_data *data  = OS_NULL;
    os_uint8_t             temp[6];
    os_err_t               result;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_TEMP);
    OS_ASSERT(buf);

    aht20 = (aht20_info_t *)sensor;
    data  = (struct os_sensor_data *)buf;

    result = os_mutex_lock(aht20->lock, OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        os_uint8_t cmd[2] = {0, 0};
        aht20_write_user_reg(aht20, AHT20_GET_DATA, cmd);

        result = calibration_enabled(aht20);
        if (result != OS_EOK)
        {
            LOG_E(DBG_EXT_TAG,"The aht20 is under an abnormal status. Please try again");
            os_mutex_unlock(aht20->lock);
            return 0;
        }
        else
        {
            aht20_read_user_reg(aht20, temp, 6);
            /* Sensor temperature converse to reality */
            aht20->value.tempreture =
                (((temp[3] & 0xf) << 16 | temp[4] << 8 | temp[5]) * 200.0 / (1 << 20) - 50) * 1000;
        }
    }
    else
    {
        LOG_E(DBG_EXT_TAG,"The aht20 could not respond temperature measurement at this time. Please try again");
        return 0;
    }
    os_mutex_unlock(aht20->lock);

    data->type      = sensor->info.type;
    data->data.temp = aht20->value.tempreture;
    data->timestamp = os_sensor_get_ts();
    return sizeof(struct os_sensor_data);
}

static os_err_t aht20_temp_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t      result = OS_EOK;
    aht20_info_t *aht20  = (aht20_info_t *)sensor;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(os_uint8_t *)args = aht20->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops aht20_temp_ops =
{
    aht20_temp_fetch_data,
    aht20_temp_control,
};

static int os_hw_aht20_temp_init(void)
{
    os_int8_t     result;
    aht20_info_t *aht20;

    aht20 = aht20_init(OS_AHT20_I2C_BUS_NAME, OS_AHT20_I2C_ADDR);
    if (aht20 == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht20 temp init failed.");
        goto __exit;
    }

    /* Temp */
    aht20->sensor.info.type       = OS_SENSOR_CLASS_TEMP;
    aht20->sensor.info.vendor     = OS_SENSOR_VENDOR_SENSIRION;
    aht20->sensor.info.model      = "aht20";
    aht20->sensor.info.unit       = OS_SENSOR_UNIT_MDCELSIUS;
    aht20->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    aht20->sensor.info.range_max  = 100000;
    aht20->sensor.info.range_min  = 0;
    aht20->sensor.info.period_min = 300;
    aht20->sensor.ops             = &aht20_temp_ops;

    result = os_hw_sensor_register(&aht20->sensor, "aht20", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG,"device temp register err code: %d", result);
        goto __exit;
    }

    struct os_sensor_data sensor_data;
    aht20_temp_fetch_data(&aht20->sensor, &sensor_data, sizeof(sensor_data));

    LOG_I(DBG_EXT_TAG,"aht20 temp init success");
    return OS_EOK;

__exit:
    if (aht20)
        os_free(aht20);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_aht20_temp_init, OS_INIT_SUBLEVEL_LOW);

static os_size_t aht20_humi_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    aht20_info_t          *aht20 = OS_NULL;
    struct os_sensor_data *data  = OS_NULL;
    os_uint8_t             humi[6];
    os_err_t               result;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_HUMI);
    OS_ASSERT(buf);

    aht20 = (aht20_info_t *)sensor;
    data  = (struct os_sensor_data *)buf;

    result = os_mutex_lock(aht20->lock, OS_WAIT_FOREVER);
    if (result == OS_EOK)
    {
        os_uint8_t cmd[2] = {0, 0};
        aht20_write_user_reg(aht20, AHT20_GET_DATA, cmd);

        result = calibration_enabled(aht20);
        if (result != OS_EOK)
        {
            LOG_E(DBG_EXT_TAG,"The aht20 is under an abnormal status. Please try again");
            os_mutex_unlock(aht20->lock);
            return 0;
        }
        else
        {
            aht20_read_user_reg(aht20, humi, 6);
            /* Sensor humidity converse to reality */
            aht20->value.humidity = (humi[1] << 12 | humi[2] << 4 | (humi[3] & 0xf0) >> 4) * 100.0 / (1 << 20) *1000;
        }
    }
    else
    {
        LOG_E(DBG_EXT_TAG,"The aht20 could not respond humidity measurement at this time. Please try again");
        return 0;
    }
    os_mutex_unlock(aht20->lock);

    data->type      = sensor->info.type;
    data->data.humi = aht20->value.humidity;
    data->timestamp = os_sensor_get_ts();

    return sizeof(struct os_sensor_data);
}

static os_err_t aht20_humi_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t      result = OS_EOK;
    aht20_info_t *aht20  = (aht20_info_t *)sensor;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(os_uint8_t *)args = aht20->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops aht20_humi_ops =
{
    aht20_humi_fetch_data,
    aht20_humi_control,
};

static int os_hw_aht20_humi_init(void)
{
    os_int8_t     result;
    aht20_info_t *aht20;

    aht20 = aht20_init(OS_AHT20_I2C_BUS_NAME, OS_AHT20_I2C_ADDR);
    if (aht20 == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"aht20 baro init failed.");
        goto __exit;
    }

    /* Baro */
    aht20->sensor.info.type       = OS_SENSOR_CLASS_HUMI;
    aht20->sensor.info.vendor     = OS_SENSOR_VENDOR_SENSIRION;
    aht20->sensor.info.model      = "aht20";
    aht20->sensor.info.unit       = OS_SENSOR_UNIT_MPERMILLAGE;
    aht20->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    aht20->sensor.info.range_max  = 100000;
    aht20->sensor.info.range_min  = 0;
    aht20->sensor.info.period_min = 300;
    aht20->sensor.ops             = &aht20_humi_ops;

    result = os_hw_sensor_register(&aht20->sensor, "aht20", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_EXT_TAG,"device baro register err code: %d", result);
        goto __exit;
    }

    struct os_sensor_data sensor_data;
    aht20_humi_fetch_data(&aht20->sensor, &sensor_data, sizeof(sensor_data));

    LOG_I(DBG_EXT_TAG,"aht20 baro init success");
    return OS_EOK;

__exit:
    if (aht20)
        os_free(aht20);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_aht20_humi_init, OS_INIT_SUBLEVEL_LOW);
