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
 * @file        photo_diode.c
 *
 * @brief       This file provides functions for photo diode.
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

#define DBG_TAG "sensor.photodiode"
#include <dlog.h>

typedef struct
{
    struct os_sensor_device sensor;
    os_device_t    *adc;
    os_uint32_t    channel;
} photo_diode_info_t;

os_int32_t photo_diode_read_ambient_light(photo_diode_info_t *photo_diode)
{
    os_int32_t brightness = 0; /* default error data */
    os_int32_t read_data;

    OS_ASSERT(photo_diode);

    os_device_read_nonblock(photo_diode->adc, photo_diode->channel, &read_data, sizeof(read_data));

    if(read_data >= photo_diode->sensor.info.range_min && read_data <= photo_diode->sensor.info.range_max)
    {
        brightness = read_data;
    }
    else
    {
        LOG_W(DBG_TAG, "data beyond range[%d].",read_data);
    }
    
    return brightness;
}


static os_size_t photo_diode_fetch_light_data(struct os_sensor_device *sensor, void *buf, os_size_t len)
{
    photo_diode_info_t *photo_diode = NULL;
    struct os_sensor_data *data    = NULL;

    OS_ASSERT(sensor);
    OS_ASSERT(sensor->info.type == OS_SENSOR_CLASS_LIGHT);
    OS_ASSERT(buf);

    photo_diode = (photo_diode_info_t *)sensor;
    data    = (struct os_sensor_data *)buf;

    data->type       = sensor->info.type;
    data->timestamp  = os_sensor_get_ts();
    data->data.light = photo_diode_read_ambient_light(photo_diode);

    return 0;
}


static os_err_t photo_diode_light_control(struct os_sensor_device *sensor, int cmd, void *args)
{
    os_err_t result = OS_EOK;

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        *(uint8_t *)args = 0x00;
        break;
    default:
        return OS_ERROR;
    }
    return result;
}

static struct os_sensor_ops photo_diode_light_ops = {photo_diode_fetch_light_data, photo_diode_light_control};


static photo_diode_info_t *photo_diode_init(const char *adc_dev, os_uint32_t channel)
{
    photo_diode_info_t *photo_diode = NULL;
    

    LOG_I(DBG_TAG, "photo_diode:[%s][0x%02x]", adc_dev, channel);

    photo_diode = os_calloc(1, sizeof(photo_diode_info_t));
    if (photo_diode == OS_NULL)
    {
        LOG_E(DBG_TAG, "photo_diode amlloc failed");
        return NULL;
    }
    
    photo_diode->adc = os_device_find(adc_dev);
    if (photo_diode->adc == NULL)
    {
        LOG_E(DBG_TAG, "photo_diode adc invalid.");
        os_free(photo_diode);
        return NULL;
    }
    
    os_device_open(photo_diode->adc);
    if (os_device_control(photo_diode->adc, OS_ADC_CMD_ENABLE, OS_NULL) != OS_EOK)
    {
        LOG_E(DBG_TAG, "photo_diode open failed! \r\n");
        return NULL;
    }
    
    photo_diode->channel = channel;

    return photo_diode;
}

static int os_hw_photo_diode_light_init(void)
{
    os_int8_t       result;
    photo_diode_info_t *photo_diode = NULL;

    photo_diode = photo_diode_init(OS_PHOTO_DIODE_ADC_NAME, OS_PHOTO_DIODE_ADC_CHANNEL);

    if (photo_diode == NULL)
    {
        LOG_E(DBG_TAG, "photo_diode init failed.");
        goto __exit;
    }

    photo_diode->sensor.info.type       = OS_SENSOR_CLASS_LIGHT;
    photo_diode->sensor.info.vendor     = OS_SENSOR_VENDOR_UNKNOWN;
    photo_diode->sensor.info.model      = "photo_diode";
    photo_diode->sensor.info.unit       = OS_SENSOR_UNIT_RAW;
    photo_diode->sensor.info.intf_type  = OS_SENSOR_INTF_ADC;
    photo_diode->sensor.info.range_max  = 3300;
    photo_diode->sensor.info.range_min  = 0;
    photo_diode->sensor.info.period_min = 300;
    photo_diode->sensor.ops             = &photo_diode_light_ops;

    result = os_hw_sensor_register(&photo_diode->sensor, "photo_diode", OS_NULL);
    if (result != OS_EOK)
    {
        LOG_E(DBG_TAG, "device register err code: %d", result);
        goto __exit;
    }

    LOG_I(DBG_TAG, "PHOTO_DIODE init success");
    LOG_W(DBG_TAG, "sensor data type is adc sampling voltage in unit mV.");
    return OS_EOK;

__exit:
    if (photo_diode)
        os_free(photo_diode);
    return OS_ERROR;
}

OS_DEVICE_INIT(os_hw_photo_diode_light_init, OS_INIT_SUBLEVEL_LOW);
