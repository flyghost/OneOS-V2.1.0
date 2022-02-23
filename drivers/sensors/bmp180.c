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
 * @file        bmp180.c
 *
 * @brief       This file provides functions for bmp180.
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
#include <os_workqueue.h>
#include <sensors/sensor.h>
#include <math.h>

#define DBG_TAG "sensor.adi.bmp180"
#include <dlog.h>

typedef struct
{
    struct os_sensor_device sensor_temp;
    struct os_sensor_device sensor_baro;
    struct os_sensor_device sensor_alti;
    struct os_i2c_client    i2c;
    os_uint8_t              id;

    os_timer_t timer;

    short          AC1;
    short          AC2;
    short          AC3;
    unsigned short AC4;
    unsigned short AC5;
    unsigned short AC6;
    short          B1;
    short          B2;
    short          MB;
    short          MC;
    short          MD;
    long           UT;
    long           UP;
    long           X1;
    long           X2;
    long           X3;
    long           B3;
    unsigned long  B4;
    long           B5;
    long           B6;
    long           B7;
    long           p;
    long           Temp;
    float          altitude;
} bmp180_info_t;

static short BMP_ReadTwoByte(bmp180_info_t *bmp180, uint8_t ReadAddr)
{
    short data;

    os_i2c_client_read(&bmp180->i2c, ReadAddr, 1, (os_uint8_t *)&data, 2);

    data = ((data >> 8) & 0xff) | ((data << 8) & 0xff00);

    return data;
}

static void BMP_ReadCalibrationData(bmp180_info_t *bmp180)
{
    bmp180->AC1 = BMP_ReadTwoByte(bmp180, 0xAA);
    bmp180->AC2 = BMP_ReadTwoByte(bmp180, 0xAC);
    bmp180->AC3 = BMP_ReadTwoByte(bmp180, 0xAE);
    bmp180->AC4 = BMP_ReadTwoByte(bmp180, 0xB0);
    bmp180->AC5 = BMP_ReadTwoByte(bmp180, 0xB2);
    bmp180->AC6 = BMP_ReadTwoByte(bmp180, 0xB4);
    bmp180->B1  = BMP_ReadTwoByte(bmp180, 0xB6);
    bmp180->B2  = BMP_ReadTwoByte(bmp180, 0xB8);
    bmp180->MB  = BMP_ReadTwoByte(bmp180, 0xBA);
    bmp180->MC  = BMP_ReadTwoByte(bmp180, 0xBC);
    bmp180->MD  = BMP_ReadTwoByte(bmp180, 0xBE);
}

static long BMP_Read_UT(bmp180_info_t *bmp180)
{
    long temp = 0;

    os_i2c_client_write_byte(&bmp180->i2c, 0xf4, 1, 0x2e);

    os_task_msleep(5);
    temp = (long)BMP_ReadTwoByte(bmp180, 0xF6);
    return temp;
}

static long BMP_Read_UP(bmp180_info_t *bmp180)
{
    long pressure = 0;

    os_i2c_client_write_byte(&bmp180->i2c, 0xf4, 1, 0x34);

    os_task_msleep(5);
    pressure = (long)BMP_ReadTwoByte(bmp180, 0xF6);
    return pressure & 0x0000FFFF;
}

static void BMP_UncompemstatedToTrue(bmp180_info_t *bmp180)
{
    bmp180->UT = BMP_Read_UT(bmp180); /* First read error */
    bmp180->UT = BMP_Read_UT(bmp180); /* Second read to correct the parameters */
    bmp180->UP = BMP_Read_UP(bmp180);

    bmp180->X1   = ((bmp180->UT - bmp180->AC6) * bmp180->AC5) >> 15;
    bmp180->X2   = (((long)bmp180->MC) << 11) / (bmp180->X1 + bmp180->MD);
    bmp180->B5   = bmp180->X1 + bmp180->X2;
    bmp180->Temp = (bmp180->B5 + 8) >> 4;

    bmp180->B6 = bmp180->B5 - 4000;
    bmp180->X1 = ((long)bmp180->B2 * (bmp180->B6 * bmp180->B6 >> 12)) >> 11;
    bmp180->X2 = ((long)bmp180->AC2) * bmp180->B6 >> 11;
    bmp180->X3 = bmp180->X1 + bmp180->X2;

    bmp180->B3 = ((((long)bmp180->AC1) * 4 + bmp180->X3) + 2) / 4;
    bmp180->X1 = ((long)bmp180->AC3) * bmp180->B6 >> 13;
    bmp180->X2 = (((long)bmp180->B1) * (bmp180->B6 * bmp180->B6 >> 12)) >> 16;
    bmp180->X3 = ((bmp180->X1 + bmp180->X2) + 2) >> 2;
    bmp180->B4 = ((long)bmp180->AC4) * (unsigned long)(bmp180->X3 + 32768) >> 15;
    bmp180->B7 = ((unsigned long)bmp180->UP - bmp180->B3) * 50000;

    if (bmp180->B7 < 0x80000000)
    {
        bmp180->p = (bmp180->B7 * 2) / bmp180->B4;
    }
    else
    {
        bmp180->p = (bmp180->B7 / bmp180->B4) * 2;
    }

    bmp180->X1 = (bmp180->p >> 8) * (bmp180->p >> 8);
    bmp180->X1 = (((long)bmp180->X1) * 3038) >> 16;
    bmp180->X2 = (-7357 * bmp180->p) >> 16;

    bmp180->p = bmp180->p + ((bmp180->X1 + bmp180->X2 + 3791) >> 4);

    bmp180->altitude = 44330 * (1 - pow(((bmp180->p) / 101325.0), (1.0 / 5.255)));
}

static void bmp180_get_data_work(void *parameter)
{
    bmp180_info_t *bmp180 = (bmp180_info_t *)parameter;

    BMP_ReadCalibrationData(bmp180);
    BMP_UncompemstatedToTrue(bmp180);

    LOG_I(DBG_TAG,"BMP180_ID = 0X%X\t\r\n"
              "BMP180_temp = %d.%dC\t\r\n"
              "BMP180_Pressure = %ldPa\t\r\n"
              "BMP180_Altitude = %dm\r\n",
              bmp180->id,
              bmp180->Temp / 10,
              bmp180->Temp % 10,
              bmp180->p,
              (int)bmp180->altitude);
}

static bmp180_info_t *bmp180_init(const char *bus_name, os_uint16_t addr)
{
    bmp180_info_t *bmp180 = NULL;
    unsigned char  devid  = 0;

    bmp180 = os_calloc(1, sizeof(bmp180_info_t));
    if (bmp180 == OS_NULL)
    {
        LOG_E(DBG_TAG,"bmp180 amlloc faile");
        return NULL;
    }

    LOG_I(DBG_TAG,"bmp180:[%s][0x%02x]", bus_name, addr);

    bmp180->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (bmp180->i2c.bus == NULL)
    {
        LOG_E(DBG_TAG,"bmp180 i2c invalid.");
        return NULL;
    }

    bmp180->i2c.client_addr = addr;

    devid = os_i2c_client_read_byte(&bmp180->i2c, 0xd0, 1);
    LOG_I(DBG_TAG,"bmp180 devid:0x%02x", devid);
    if (devid != 0x55)
    {
        LOG_E(DBG_TAG,"bmp180 devid invalid.");
        os_free(bmp180);
        return NULL;
    }

    bmp180->id = devid;

    os_timer_init(&bmp180->timer,
                  "bmp180_timer",
                  bmp180_get_data_work,
                  bmp180,
                  OS_TICK_PER_SECOND,
                  OS_TIMER_FLAG_PERIODIC);

    os_timer_start(&bmp180->timer);

    return bmp180;
}

static bmp180_info_t *sensor_to_bmp180(struct os_sensor_device *sensor)
{
    bmp180_info_t *bmp180 = NULL;

    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_TEMP || sensor->info.type == OS_SENSOR_CLASS_BARO ||
              sensor->info.type == OS_SENSOR_CLASS_ALTI);

    if (sensor->info.type == OS_SENSOR_CLASS_TEMP)
    {
        bmp180 = container_of(sensor, bmp180_info_t, sensor_temp);
    }
    else if (sensor->info.type == OS_SENSOR_CLASS_BARO)
    {
        bmp180 = container_of(sensor, bmp180_info_t, sensor_baro);
    }
    else if (sensor->info.type == OS_SENSOR_CLASS_ALTI)
    {
        bmp180 = container_of(sensor, bmp180_info_t, sensor_alti);
    }
    else
    {
        LOG_E(DBG_TAG,"bmp180 type invalid.");
        return NULL;
    }

    return bmp180;
}

static os_size_t bmp180_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    bmp180_info_t *bmp180 = NULL;
    struct os_sensor_data *data = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_TEMP || sensor->info.type == OS_SENSOR_CLASS_BARO ||
              sensor->info.type == OS_SENSOR_CLASS_ALTI);
    OS_ASSERT(buf);

    bmp180 = sensor_to_bmp180(sensor);
    data   = (struct os_sensor_data *)buf;

    data->type      = sensor->info.type;
    data->timestamp = os_sensor_get_ts();

    if (sensor->info.type == OS_SENSOR_CLASS_TEMP)
    {
        data->data.temp = bmp180->Temp;
    }
    else if (sensor->info.type == OS_SENSOR_CLASS_BARO)
    {
        data->data.baro = bmp180->p;
    }
    else if (sensor->info.type == OS_SENSOR_CLASS_ALTI)
    {
        data->data.alti = bmp180->altitude;
    }
    else
    {
        LOG_E(DBG_TAG,"bmp180 type invalid.");
        return 0;
    }

    return 0;
}

static os_err_t bmp180_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t       result = OS_EOK;
    bmp180_info_t *bmp180 = sensor_to_bmp180(sensor);

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = bmp180->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops bmp180_ops =
{
    bmp180_fetch_data,
    bmp180_control
};

static int os_hw_bmp180_init(void)
{
    os_int8_t      result;
    bmp180_info_t *bmp180 = NULL;

    bmp180 = bmp180_init(OS_BMP180_I2C_BUS_NAME, OS_BMP180_I2C_ADDR);
    if (bmp180 == NULL)
    {
        LOG_E(DBG_TAG,"bmp180 init failed.");
        goto __exit;
    }

    /* Temp */
    bmp180->sensor_temp.info.type       = OS_SENSOR_CLASS_TEMP;
    bmp180->sensor_temp.info.vendor     = OS_SENSOR_VENDOR_ADI;
    bmp180->sensor_temp.info.model      = "bmp180";
    bmp180->sensor_temp.info.unit       = OS_SENSOR_UNIT_DCELSIUS;
    bmp180->sensor_temp.info.intf_type  = OS_SENSOR_INTF_I2C;
    bmp180->sensor_temp.info.range_max  = 1000;
    bmp180->sensor_temp.info.range_min  = 0;
    bmp180->sensor_temp.info.period_min = 300;

    bmp180->sensor_temp.ops = &bmp180_ops;

    result = os_hw_sensor_register(&bmp180->sensor_temp, "bmp180", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    /* Baro */
    bmp180->sensor_baro.info.type       = OS_SENSOR_CLASS_BARO;
    bmp180->sensor_baro.info.vendor     = OS_SENSOR_VENDOR_ADI;
    bmp180->sensor_baro.info.model      = "bmp180";
    bmp180->sensor_baro.info.unit       = OS_SENSOR_UNIT_PA;
    bmp180->sensor_baro.info.intf_type  = OS_SENSOR_INTF_I2C;
    bmp180->sensor_baro.info.range_max  = 1000000;
    bmp180->sensor_baro.info.range_min  = 0;
    bmp180->sensor_baro.info.period_min = 300;

    bmp180->sensor_baro.ops = &bmp180_ops;

    result = os_hw_sensor_register(&bmp180->sensor_baro, "bmp180", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    /* Altitude */
    bmp180->sensor_alti.info.type       = OS_SENSOR_CLASS_ALTI;
    bmp180->sensor_alti.info.vendor     = OS_SENSOR_VENDOR_ADI;
    bmp180->sensor_alti.info.model      = "bmp180";
    bmp180->sensor_alti.info.unit       = OS_SENSOR_UNIT_M;
    bmp180->sensor_alti.info.intf_type  = OS_SENSOR_INTF_I2C;
    bmp180->sensor_alti.info.range_max  = 10000;
    bmp180->sensor_alti.info.range_min  = 0;
    bmp180->sensor_alti.info.period_min = 300;

    bmp180->sensor_alti.ops = &bmp180_ops;

    result = os_hw_sensor_register(&bmp180->sensor_alti, "bmp180", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_TAG,"BMP180 init success");
    return OS_EOK;

__exit:
    if (bmp180)
        os_free(bmp180);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_bmp180_init, OS_INIT_SUBLEVEL_LOW);

