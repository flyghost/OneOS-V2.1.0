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
 * @file        adxl345.c
 *
 * @brief       This file provides functions for adxl345.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <arch_interrupt.h>
#include <device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_util.h>
#include <os_memory.h>
#include <string.h>
#include <stdio.h>
#include <drv_gpio.h>
#include <sensors/sensor.h>

#define DBG_TAG "sensor.adi.adxl345"
#include <drv_log.h>

#define DATA_FORMAT_REG 0x31
#define POWER_CTL       0x2d
#define INT_ENABLE      0x2e
#define BW_RATE         0x2c
#define OFSX            0x1e
#define OFSY            0x1f
#define OFSZ            0x20

typedef struct
{
    struct os_sensor_device sensor;
    struct os_i2c_client    i2c;
    os_uint8_t              id;

    os_int32_t incidence_X;
    os_int32_t incidence_Y;
    os_int32_t incidence_Z;
} adxl345_info_t;

static os_err_t adxl345_write(adxl345_info_t *adxl345, uint8_t Reg, uint8_t Value)
{
    return os_i2c_client_write_byte(&adxl345->i2c, Reg, 1, Value);
}

static uint8_t adxl345_read(adxl345_info_t *adxl345, uint8_t Reg)
{
    return os_i2c_client_read_byte(&adxl345->i2c, Reg, 1);
}

static os_err_t adxl345_read_buff(adxl345_info_t *adxl345, uint8_t Reg, uint8_t *Buffer, uint16_t Length)
{
    return os_i2c_client_read(&adxl345->i2c, Reg, 1, Buffer, Length);
}

static adxl345_info_t *adxl345_init(const char *bus_name, os_uint16_t addr)
{
    adxl345_info_t *adxl345 = NULL;
    unsigned char   devid   = 0;

    adxl345 = os_calloc(1, sizeof(adxl345_info_t));
    if (adxl345 == OS_NULL)
    {
        LOG_E(DBG_TAG,"adxl345 amlloc faile");
        return NULL;
    }

    LOG_I(DBG_TAG,"adxl345:[%s][0x%02x]", bus_name, addr);

    adxl345->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (adxl345->i2c.bus == NULL)
    {
        LOG_E(DBG_TAG,"adxl345 i2c invalid.");
        os_free(adxl345);
        return NULL;
    }

    adxl345->i2c.client_addr = addr;

    os_task_msleep(1);

    /* Read ID */
    devid = adxl345_read(adxl345, 0);
    LOG_I(DBG_TAG,"adxl345 devid:0x%02x", devid);
    if (devid != 0xe5)
    {
        LOG_E(DBG_TAG,"adxl345 devid invalid.");
        os_free(adxl345);
        return NULL;
    }

    adxl345->id = devid;

    os_task_msleep(1);

    /* Low level interrupt output, output data right aligned */
    adxl345_write(adxl345, DATA_FORMAT_REG, 0x2B);
    os_task_msleep(1);

    /* The data output speed is 100Hz */
    adxl345_write(adxl345, BW_RATE, 0x0A);
    os_task_msleep(1);

    /* Link enabled, measure mode */
    adxl345_write(adxl345, POWER_CTL, 0x28);
    os_task_msleep(1);

    /* Not using interrupts */
    adxl345_write(adxl345, INT_ENABLE, 0);
    os_task_msleep(1);

    /* Offset 0 on the X axis */
    adxl345_write(adxl345, OFSX, 0);
    os_task_msleep(1);

    /* Offset 0 on the Y axis */
    adxl345_write(adxl345, OFSY, 0);
    os_task_msleep(1);

    /* Offset 0 on the Z axis */
    adxl345_write(adxl345, OFSZ, 0);
    os_task_msleep(1);

    return adxl345;
}

static void adxl345_get_value(adxl345_info_t *adxl345)
{
    unsigned char dataTemp[6];

    /* Read the original acceleration value(4mg/LSB) */
    dataTemp[0] = 0x32;
    adxl345_read_buff(adxl345, 0x32, dataTemp, 6);

    adxl345->incidence_X = (short)(dataTemp[0] + ((unsigned short)dataTemp[1] << 8));
    adxl345->incidence_Y = (short)(dataTemp[2] + ((unsigned short)dataTemp[3] << 8));
    adxl345->incidence_Z = (short)(dataTemp[4] + ((unsigned short)dataTemp[5] << 8));

    /* Each LSB stands for 3.9 mG */
    adxl345->incidence_X = adxl345->incidence_X * 3.9;
    adxl345->incidence_Y = adxl345->incidence_Y * 3.9;
    adxl345->incidence_Z = adxl345->incidence_Z * 3.9;

    LOG_I(DBG_TAG,"ADX345-X is : %d mG", adxl345->incidence_X);
    LOG_I(DBG_TAG,"ADX345-Y is : %d mG", adxl345->incidence_Y);
    LOG_I(DBG_TAG,"ADX345-Z is : %d mG", adxl345->incidence_Z);
}

static os_size_t adxl345_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    adxl345_info_t *adxl345 = NULL;
    struct os_sensor_data *data = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_ACCE);
    OS_ASSERT(buf);

    adxl345 = (adxl345_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    adxl345_get_value(adxl345);

    data->type        = sensor->info.type;
    data->data.acce.x = adxl345->incidence_X;
    data->data.acce.y = adxl345->incidence_Y;
    data->data.acce.z = adxl345->incidence_Z;
    data->timestamp   = os_sensor_get_ts();

    return 0;
}

static os_err_t adxl345_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t        result  = OS_EOK;
    adxl345_info_t *adxl345 = (adxl345_info_t *)sensor;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = adxl345->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops adxl345_ops =
{
    adxl345_fetch_data,
    adxl345_control
};

static int os_hw_adxl345_init(void)
{
    os_int8_t       result;
    adxl345_info_t *adxl345;

    adxl345 = adxl345_init(OS_ADXL345_I2C_BUS_NAME, OS_ADXL345_I2C_ADDR);
    if (adxl345 == NULL)
    {
        LOG_E(DBG_TAG,"adxl345 init failed.");
        goto __exit;
    }

    adxl345->sensor.info.type       = OS_SENSOR_CLASS_ACCE;
    adxl345->sensor.info.vendor     = OS_SENSOR_VENDOR_ADI;
    adxl345->sensor.info.model      = "adxl345";
    adxl345->sensor.info.unit       = OS_SENSOR_UNIT_MG;
    adxl345->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    adxl345->sensor.info.range_max  = 16000;
    adxl345->sensor.info.range_min  = -16000;
    adxl345->sensor.info.period_min = 5;
    adxl345->sensor.ops             = &adxl345_ops;

    result = os_hw_sensor_register(&adxl345->sensor, "adxl345", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_TAG,"adxl345 init success");
    return OS_EOK;

__exit:
    if (adxl345)
        os_free(adxl345);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_adxl345_init, OS_INIT_SUBLEVEL_LOW);

