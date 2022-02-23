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
 * @file        bh1750.c
 *
 * @brief       This file provides functions for bh1750.
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

#define BH1750_Addr 0x46
#define BH1750_ON   0x01
#define BH1750_CON  0x10
#define BH1750_ONE  0x20
#define BH1750_RSET 0x07

typedef struct
{
    struct os_sensor_device sensor;
    struct os_i2c_client    i2c;
    os_uint8_t              id;
} bh1750_info_t;

static void cmd_write_bh1750(bh1750_info_t *bh1750, unsigned int cmd)
{
    os_i2c_client_write(&bh1750->i2c, cmd, 1, NULL, 0);
}

static void start_bh1750(bh1750_info_t *bh1750)
{
    cmd_write_bh1750(bh1750, BH1750_ON);   /* Power on */
    cmd_write_bh1750(bh1750, BH1750_RSET); /* Clear */
    cmd_write_bh1750(bh1750, BH1750_ONE);  /* H resolution mode once, at least 120ms */
}

static float read_bh1750(bh1750_info_t *bh1750)
{
    float         value_lx;
    unsigned char buff[2];

    os_i2c_client_read(&bh1750->i2c, 0, 0, buff, 2);
    value_lx = ((buff[0] << 8) | buff[1]) / 1.2;

    LOG_I(DBG_TAG,"BH1750 DATA is: %d LUX", (int)value_lx);

    return value_lx;
}

static os_size_t bh1750_fetch_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    bh1750_info_t *bh1750 = NULL;
    struct os_sensor_data *data = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_LIGHT);
    OS_ASSERT(buf);

    bh1750 = (bh1750_info_t *)sensor;
    data   = (struct os_sensor_data *)buf;

    data->type       = sensor->info.type;
    data->data.light = read_bh1750(bh1750);
    data->timestamp  = os_sensor_get_ts();

    /* Start next convert */
    start_bh1750(bh1750);

    return 0;
}

static os_err_t bh1750_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t result = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = 0x12;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops bh1750_ops =
{
    bh1750_fetch_data,
    bh1750_control
};

static bh1750_info_t *bh1750_Init(const char *bus_name, os_uint16_t addr)
{
    bh1750_info_t *bh1750 = NULL;

    LOG_I(DBG_TAG,"bh1750:[%s][0x%02x]", bus_name, addr);

    bh1750 = os_calloc(1, sizeof(bh1750_info_t));
    if (bh1750 == OS_NULL)
    {
        LOG_E(DBG_TAG,"bh1750 amlloc faile");
        return NULL;
    }

    bh1750->i2c.bus = os_i2c_bus_device_find(bus_name);
    if (bh1750->i2c.bus == NULL)
    {
        LOG_E(DBG_TAG,"bh1750 i2c invalid.");
        os_free(bh1750);
        return NULL;
    }

    bh1750->i2c.client_addr = addr;

    start_bh1750(bh1750);

    return bh1750;
}

static int os_hw_bh1750_init(void)
{
    os_int8_t      result;
    bh1750_info_t *bh1750 = NULL;

    bh1750 = bh1750_Init(OS_BH1750_I2C_BUS_NAME, OS_BH1750_I2C_ADDR);
    if (bh1750 == NULL)
    {
        LOG_E(DBG_TAG,"bh1750 init failed.");
        goto __exit;
    }

    bh1750->sensor.info.type       = OS_SENSOR_CLASS_LIGHT;
    bh1750->sensor.info.vendor     = OS_SENSOR_VENDOR_ADI;
    bh1750->sensor.info.model      = "bh1750";
    bh1750->sensor.info.unit       = OS_SENSOR_UNIT_LUX;
    bh1750->sensor.info.intf_type  = OS_SENSOR_INTF_I2C;
    bh1750->sensor.info.range_max  = 10000;
    bh1750->sensor.info.range_min  = 0;
    bh1750->sensor.info.period_min = 300;
    bh1750->sensor.ops             = &bh1750_ops;

    result = os_hw_sensor_register(&bh1750->sensor, "bh1750", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG,"device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_TAG,"BH1750 init success");
    return OS_EOK;

__exit:
    if (bh1750)
        os_free(bh1750);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_bh1750_init, OS_INIT_SUBLEVEL_LOW);
