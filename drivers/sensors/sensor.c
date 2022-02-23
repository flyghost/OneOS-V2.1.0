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
 * @file        sensor.c
 *
 * @brief       This file provides functions for sensor.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <os_mutex.h>
#include "sensor.h"

#define DRV_EXT_TAG "sensor"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>

#include <string.h>

static char *const sensor_name_str[] =
{
    "none",
    "acce_", /* Accelerometer     */
    "gyro_", /* Gyroscope         */
    "mag_",  /* Magnetometer      */
    "temp_", /* Temperature       */
    "humi_", /* Relative Humidity */
    "baro_", /* Barometer         */
    "li_",   /* Ambient light     */
    "pr_",   /* Proximity         */
    "hr_",   /* Heart Rate        */
    "tvoc_", /* TVOC Level        */
    "noi_",  /* Noise Loudness    */
    "step_", /* Step sensor       */
    "forc_", /* Force sensor      */
    "alti_", /* Altitude sensor   */
    "ir_",   /* ir intensity sensor   */
};

void os_sensor_cb(os_sensor_t sen)
{
    if (sen->irq_handle != OS_NULL)
    {
        sen->irq_handle(sen);
    }

    if (sen->data_len > 0)
    {
        sen->parent.rx_count = sen->data_len / sizeof(struct os_sensor_data);
        os_device_recv_notify(&sen->parent);
    }
    else if (sen->config.mode == OS_SENSOR_MODE_INT)
    {
        sen->parent.rx_count = 1;
        os_device_recv_notify(&sen->parent);
    }
    else if (sen->config.mode == OS_SENSOR_MODE_FIFO)
    {
        sen->parent.rx_count = sen->info.fifo_max;
        os_device_recv_notify(&sen->parent);
    }
}

static void irq_callback(void *args)
{
    os_sensor_t sensor = (os_sensor_t)args;

    os_sensor_cb(sensor);
}

OS_USED static os_err_t os_sensor_irq_init(os_sensor_t sensor)
{
    if (sensor->config.irq_pin.pin == OS_PIN_NONE)
    {
        return OS_EINVAL;
    }

    os_pin_mode(sensor->config.irq_pin.pin, sensor->config.irq_pin.mode);

    if (sensor->config.irq_pin.mode == PIN_MODE_INPUT_PULLDOWN)
    {
        os_pin_attach_irq(sensor->config.irq_pin.pin, PIN_IRQ_MODE_RISING, irq_callback, (void *)sensor);
    }
    else if (sensor->config.irq_pin.mode == PIN_MODE_INPUT_PULLUP)
    {
        os_pin_attach_irq(sensor->config.irq_pin.pin, PIN_IRQ_MODE_FALLING, irq_callback, (void *)sensor);
    }
    else if (sensor->config.irq_pin.mode == PIN_MODE_INPUT)
    {
        os_pin_attach_irq(sensor->config.irq_pin.pin, PIN_IRQ_MODE_RISING_FALLING, irq_callback, (void *)sensor);
    }

    os_pin_irq_enable(sensor->config.irq_pin.pin, OS_TRUE);

    LOG_I(DBG_EXT_TAG,"interrupt init success");

    return 0;
}

static os_err_t os_sensor_init(os_device_t *dev)
{
    os_sensor_t sensor = (os_sensor_t)dev;
    OS_ASSERT(dev != OS_NULL);
    os_err_t res = OS_EOK;
    
    if (sensor->ops->control != OS_NULL)
    {
        /* If polling mode is supported, configure it to polling mode */
        sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_MODE, (void *)OS_SENSOR_MODE_POLLING);
    }
    sensor->config.mode = OS_SENSOR_MODE_POLLING;

    /* Configure power mode to normal mode */
    if (sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_POWER, (void *)OS_SENSOR_POWER_NORMAL) == OS_EOK)
    {
        sensor->config.power = OS_SENSOR_POWER_NORMAL;
    }

    return res;
}

static os_err_t os_sensor_deinit(os_device_t *dev)
{
    os_sensor_t sensor = (os_sensor_t)dev;

    OS_ASSERT(dev != OS_NULL);

    /* Configure power mode to power down mode */
    if (sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_POWER, (void *)OS_SENSOR_POWER_DOWN) == OS_EOK)
    {
        sensor->config.power = OS_SENSOR_POWER_DOWN;
    }

    /* Sensor disable interrupt */
    if (sensor->config.irq_pin.pin != OS_PIN_NONE)
    {
        os_pin_irq_enable(sensor->config.irq_pin.pin, OS_FALSE);
    }

    return OS_EOK;
}

static os_size_t os_sensor_read(os_device_t *dev, os_off_t pos, void *buf, os_size_t len)
{
    os_sensor_t sensor = (os_sensor_t)dev;
    os_size_t   result = 0;
    OS_ASSERT(dev != OS_NULL);

    if (buf == NULL || len == 0)
    {
        return 0;
    }

    /* The buffer is not empty. Read the data in the buffer first */
    if (sensor->data_len > 0)
    {
        if (len > sensor->data_len / sizeof(struct os_sensor_data))
        {
            len = sensor->data_len / sizeof(struct os_sensor_data);
        }

        memcpy(buf, sensor->data_buf, len * sizeof(struct os_sensor_data));

        /* Clear the buffer */
        sensor->data_len = 0;
        result           = len;
    }
    else
    {
        /* If the buffer is empty read the data */
        result = sensor->ops->fetch_data(sensor, buf, len);
    }

    return result;
}

static os_err_t os_sensor_control(os_device_t *dev, int cmd, void *args)
{
    os_sensor_t sensor = (os_sensor_t)dev;
    os_err_t    result = OS_EOK;
    OS_ASSERT(dev != OS_NULL);

    switch (cmd)
    {
    case OS_SENSOR_CTRL_GET_ID:
        if (args)
        {
            sensor->ops->control(sensor, OS_SENSOR_CTRL_GET_ID, args);
        }
        break;
    case OS_SENSOR_CTRL_GET_INFO:
        if (args)
        {
            memcpy(args, &sensor->info, sizeof(struct os_sensor_info));
        }
        break;
    case OS_SENSOR_CTRL_SET_RANGE:

        /* Configuration measurement range */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_RANGE, args);
        if (result == OS_EOK)
        {
            sensor->config.range = (os_int32_t)args;
            LOG_D(DBG_EXT_TAG, "set range %d", sensor->config.range);
        }
        break;
    case OS_SENSOR_CTRL_SET_ODR:

        /* Configuration data output rate */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_ODR, args);
        if (result == OS_EOK)
        {
            sensor->config.odr = (os_uint32_t)args & 0xFFFF;
            LOG_D(DBG_EXT_TAG, "set odr %d", sensor->config.odr);
        }
        break;
    case OS_SENSOR_CTRL_SET_POWER:

        /* Configuration sensor power mode */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SET_POWER, args);
        if (result == OS_EOK)
        {
            sensor->config.power = (os_uint32_t)args & 0xFF;
            LOG_D(DBG_EXT_TAG, "set power mode code:", sensor->config.power);
        }
        break;
    case OS_SENSOR_CTRL_SELF_TEST:

        /* Device self-test */
        result = sensor->ops->control(sensor, OS_SENSOR_CTRL_SELF_TEST, args);
        break;
    default:
        return OS_ERROR;
    }

    return result;
}

const static struct os_device_ops os_sensor_ops =
{
    .init    = os_sensor_init,
    .deinit  = os_sensor_deinit,
    .read    = os_sensor_read,
    .control = os_sensor_control,
};

int os_hw_sensor_register(os_sensor_t sensor, const char *name, void *data)
{
    os_int8_t    result;
    os_device_t *device;
    OS_ASSERT(sensor != OS_NULL);

    char *sensor_name = OS_NULL, *device_name = OS_NULL;

    /* Add a type name for the sensor device */
    sensor_name = sensor_name_str[sensor->info.type];
    device_name = (char *)os_calloc(1, strlen(sensor_name) + 1 + strlen(name));
    if (device_name == OS_NULL)
    {
        LOG_E(DBG_EXT_TAG,"device_name calloc failed!");
        return OS_ERROR;
    }

    memcpy(device_name, sensor_name, strlen(sensor_name) + 1);
    strcat(device_name, name);

    device = &sensor->parent;

    device->ops = &os_sensor_ops;
    
    device->type        = OS_DEVICE_TYPE_SENSOR;
    device->user_data   = data;

    result = os_device_register(device, device_name);
    if (result != OS_EOK)
    {
        os_free(device_name);
        LOG_E(DBG_EXT_TAG,"os_sensor register err code: %d", result);
        return result;
    }

    os_free(device_name);
    LOG_I(DBG_EXT_TAG,"os_sensor init success");
    return OS_EOK;
}

