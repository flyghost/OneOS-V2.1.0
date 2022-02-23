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
 * @file        ak8963.c
 *
 * @brief       This file provides functions for ak8963.
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

#define DBG_TAG "sensor.adi.ak8963"
#include <dlog.h>

#define AK8963_ID 0X48 /* Device ID of AK8963 */

/* The internal register */
#define MAG_WIA   0x00 /* Device ID register address for AK8963 */
#define MAG_CNTL1 0X0A
#define MAG_CNTL2 0X0B

#define MAG_XOUT_L 0X03
#define MAG_XOUT_H 0X04
#define MAG_YOUT_L 0X05
#define MAG_YOUT_H 0X06
#define MAG_ZOUT_L 0X07
#define MAG_ZOUT_H 0X08

typedef struct
{
    struct os_sensor_device sensor;
    struct os_i2c_client    i2c;
    os_uint8_t              id;

    short mx;
    short my;
    short mz;
} ak8963_info_t;

#ifdef OS_USING_MPU9250

#define MPU_INTBP_CFG_REG 0X37

static void mpu9250_bypass(void)
{
    struct os_i2c_client i2c;

    i2c.bus = os_i2c_bus_device_find(OS_MPU6XXX_BUS_NAME);
    if (i2c.bus == NULL)
    {
        LOG_E(DBG_TAG,"ak8963(mpu9250) i2c invalid.");
        return;
    }

    i2c.client_addr = OS_MPU6XXX_ADDR;

    /* Open bypass mode to read the magnetometer directly */
    os_i2c_client_write_byte(&i2c, MPU_INTBP_CFG_REG, 1, 0x82);
}
#endif

static ak8963_info_t *ak8963_Init(const char *bus_name, os_uint16_t addr)
{
    ak8963_info_t *ak8963 = NULL;
    unsigned char  devid  = 0;

    LOG_I(DBG_TAG,"ak8963:[%s][0x%02x]", bus_name, addr);

    ak8963 = os_calloc(1, sizeof(ak8963_info_t));
    if (ak8963 == OS_NULL)
    {
        LOG_E(DBG_TAG,"ak8963 amlloc faile");
        return NULL;
    }

    ak8963->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (ak8963->i2c.bus == NULL)
    {
        LOG_E(DBG_TAG,"ak8963 i2c invalid.");
        os_free(ak8963);
        return NULL;
    }

    ak8963->i2c.client_addr = addr;

#ifdef OS_USING_MPU9250
    mpu9250_bypass();
#endif

    devid = os_i2c_client_read_byte(&ak8963->i2c, MAG_WIA, 1);

    LOG_I(DBG_TAG,"ak8963 devid:0x%02x", devid);
    if (devid != AK8963_ID)
    {
        LOG_E(DBG_TAG,"ak8963 devid invalid:0x%02x", devid);
        os_free(ak8963);
        return NULL;
    }

    ak8963->id = devid;

    /* Set AK8963 as a single measurement mode */
    os_i2c_client_write_byte(&ak8963->i2c, MAG_CNTL1, 1, 0x11);

    return ak8963;
}

static uint8_t ak8963_get_value(ak8963_info_t *ak8963)
{
    uint8_t buf[6];
    uint8_t ret;

    ret = os_i2c_client_read(&ak8963->i2c, MAG_XOUT_L, 1, buf, 6);

    /* Reset to single measurement mode after each read */
    os_i2c_client_write_byte(&ak8963->i2c, MAG_CNTL1, 1, 0x11);

    if (ret != 0)
    {
        LOG_E(DBG_TAG,"invalid value.");
        return ret;
    }

    ak8963->mx = ((uint16_t)buf[1] << 8) | buf[0];
    ak8963->my = ((uint16_t)buf[3] << 8) | buf[2];
    ak8963->mz = ((uint16_t)buf[5] << 8) | buf[4];

    LOG_D(DBG_TAG, "(%d, %d, %d).", (int)ak8963->mx, (int)ak8963->my, (int)ak8963->mz);

    return ret;
}

static os_size_t ak8963_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    ak8963_info_t *ak8963 = NULL;
    struct os_sensor_data *data = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_MAG);
    OS_ASSERT(buf);

    ak8963 = (ak8963_info_t *)sensor;
    data   = (struct os_sensor_data *)buf;

    ak8963_get_value(ak8963);

    data->type       = sensor->info.type;
    data->data.mag.x = ak8963->mx;
    data->data.mag.y = ak8963->my;
    data->data.mag.z = ak8963->mz;
    data->timestamp  = os_sensor_get_ts();

    return 0;
}

static os_err_t ak8963_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    ak8963_info_t *ak8963 = (ak8963_info_t *)sensor;
    os_err_t       result = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = ak8963->id;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops ak8963_ops =
{
    ak8963_fetch_data,
    ak8963_control
};

static int os_hw_ak8963_init(void)
{
    os_int8_t      result;
    ak8963_info_t *ak8963;

    ak8963 = ak8963_Init(OS_AK8963_I2C_BUS_NAME, OS_AK8963_I2C_ADDR);
    if (ak8963 == NULL)
    {
        LOG_E(DBG_TAG,"ak8963 init failed.");
        goto __exit;
    }

    ak8963->sensor.info.type       = OS_SENSOR_CLASS_MAG;
    ak8963->sensor.info.vendor     = OS_SENSOR_VENDOR_INVENSENSE;
    ak8963->sensor.info.model      = "ak8963";
    ak8963->sensor.info.unit       = OS_SENSOR_UNIT_MG;
    ak8963->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    ak8963->sensor.info.range_max  = 16000;
    ak8963->sensor.info.range_min  = -16000;
    ak8963->sensor.info.period_min = 5;
    ak8963->sensor.ops             = &ak8963_ops;

    result = os_hw_sensor_register(&ak8963->sensor, "ak8963", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_TAG,"ak8963 init success");
    return OS_EOK;

__exit:
    if (ak8963)
        os_free(ak8963);
    return OS_ERROR;
}

OS_INIT_EXPORT(os_hw_ak8963_init, "6", OS_INIT_SUBLEVEL_LOW);
