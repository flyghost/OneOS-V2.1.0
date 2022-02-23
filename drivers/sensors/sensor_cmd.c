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
 * @file        sensor_cmd.c
 *
 * @brief       This file provides command for sensor.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_memory.h>
#include <shell.h>
#include <os_sem.h>
#include "sensor.h"

#ifdef OS_USING_SENSOR_CMD

#define DRV_EXT_TAG "sensor.cmd"
#define DRV_EXT_LVL DBG_EXT_INFO
#include <drv_log.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static os_sem_t *sensor_rx_sem = OS_NULL;

static void sensor_show_data(os_size_t num, os_sensor_t sensor, struct os_sensor_data *sensor_data)
{
    switch (sensor->info.type)
    {
    case OS_SENSOR_CLASS_ACCE:
        LOG_I(DBG_TAG,"num:%3d, x:%5d, y:%5d, z:%5d mg, timestamp:%5d",
                  num,
                  sensor_data->data.acce.x,
                  sensor_data->data.acce.y,
                  sensor_data->data.acce.z,
                  sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_GYRO:
        LOG_I(DBG_TAG,"num:%3d, x:%8d, y:%8d, z:%8d dps, timestamp:%5d",
                  num,
                  sensor_data->data.gyro.x / 1000,
                  sensor_data->data.gyro.y / 1000,
                  sensor_data->data.gyro.z / 1000,
                  sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_MAG:
        LOG_I(DBG_TAG,"num:%3d, x:%5d, y:%5d, z:%5d mGauss, timestamp:%5d",
                  num,
                  sensor_data->data.mag.x,
                  sensor_data->data.mag.y,
                  sensor_data->data.mag.z,
                  sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_HUMI:
        LOG_I(DBG_TAG,"num:%3d, humi:%3d.%d%%, timestamp:%5d",
                  num,
                  sensor_data->data.humi / 10,
                  sensor_data->data.humi % 10,
                  sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_TEMP:
        LOG_I(DBG_TAG,"num:%3d, temp:%3d.%dC, timestamp:%5d",
                  num,
                  sensor_data->data.temp / 10,
                  sensor_data->data.temp % 10,
                  sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_BARO:
        LOG_I(DBG_TAG,"num:%3d, press:%5d pa, timestamp:%5d", num, sensor_data->data.baro, sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_STEP:
        LOG_I(DBG_TAG,"num:%3d, step:%5d, timestamp:%5d", num, sensor_data->data.step, sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_PROXIMITY:
        LOG_I(DBG_TAG,"num:%3d, distance:%5d, timestamp:%5d", num, sensor_data->data.proximity, sensor_data->timestamp);
        break;
    case OS_SENSOR_CLASS_FORCE:
        LOG_I(DBG_TAG,"num:%3d, force:%5d, timestamp:%5d", num, sensor_data->data.force, sensor_data->timestamp);
        break;
    default:
        break;
    }
}

static os_err_t rx_callback(os_device_t *dev, struct os_device_cb_info *info)
{
    os_sem_post(sensor_rx_sem);
    return 0;
}

static void sensor_fifo_rx_entry(void *parameter)
{
    os_device_t           *dev    = (os_device_t *)parameter;
    os_sensor_t            sensor = (os_sensor_t)parameter;
    struct os_sensor_data *data   = OS_NULL;
    struct os_sensor_info  info;
    os_size_t              res, i;

    os_device_control(dev, OS_SENSOR_CTRL_GET_INFO, &info);

    data = (struct os_sensor_data *)os_calloc(1, sizeof(struct os_sensor_data) * info.fifo_max);
    if (data == OS_NULL)
    {
        LOG_E(DBG_TAG,"Memory allocation failed!");
    }

    while (1)
    {
        os_sem_wait(sensor_rx_sem, OS_WAIT_FOREVER);

        res = os_device_read_nonblock(dev, 0, data, info.fifo_max);
        for (i = 0; i < res; i++)
        {
            sensor_show_data(i, sensor, &data[i]);
        }
    }
}

static void sensor_fifo_test(int argc, char **argv)
{
    static os_task_t *tid1 = OS_NULL;
    os_device_t      *dev  = OS_NULL;
    os_sensor_t       sensor;

    dev = os_device_find(argv[1]);
    if (dev == OS_NULL)
    {
        LOG_E(DBG_TAG,"Can't find device:%s", argv[1]);
        return;
    }
    sensor = (os_sensor_t)dev;

    if (os_device_open(dev, OS_DEVICE_FLAG_FIFO_RX) != OS_EOK)
    {
        LOG_E(DBG_TAG,"open device failed!");
        return;
    }

    if (sensor_rx_sem == OS_NULL)
    {
        sensor_rx_sem = os_sem_create("sen_rx_sem", 0, OS_IPC_FLAG_FIFO);
    }
    else
    {
        LOG_E(DBG_TAG,"The thread is running, please reboot and try again");
        return;
    }

    tid1 = os_task_create("sen_rx_thread", sensor_fifo_rx_entry, sensor, 1024, 15, 5);

    if (tid1 != OS_NULL)
        os_task_startup(tid1);

    struct os_device_cb_info cb_info = 
    {
        .type = OS_DEVICE_CB_TYPE_RX,
        .cb   = rx_callback,
    };

    os_device_control(dev, OS_DEVICE_CTRL_SET_CB, &cb_info);

    os_device_control(dev, OS_SENSOR_CTRL_SET_ODR, (void *)20);
}
SH_CMD_EXPORT(sensor_fifo, sensor_fifo_test, "Sensor fifo mode test function");

static void sensor_irq_rx_entry(void *parameter)
{
    os_device_t *dev    = (os_device_t *)parameter;
    os_sensor_t  sensor = (os_sensor_t)parameter;
    struct os_sensor_data data;
    os_size_t    res, i = 0;

    while (1)
    {
        os_sem_wait(sensor_rx_sem, OS_WAIT_FOREVER);

        res = os_device_read_nonblock(dev, 0, &data, 1);
        if (res == 1)
        {
            sensor_show_data(i++, sensor, &data);
        }
    }
}

static void sensor_int_test(int argc, char **argv)
{
    static os_task_t *tid1 = OS_NULL;
    os_device_t      *dev  = OS_NULL;
    os_sensor_t       sensor;

    dev = os_device_find(argv[1]);
    if (dev == OS_NULL)
    {
        LOG_E(DBG_TAG,"Can't find device:%s", argv[1]);
        return;
    }
    sensor = (os_sensor_t)dev;

    if (sensor_rx_sem == OS_NULL)
    {
        sensor_rx_sem = os_sem_create("sen_rx_sem", 0, OS_IPC_FLAG_FIFO);
    }
    else
    {
        LOG_E(DBG_TAG,"The thread is running, please reboot and try again");
        return;
    }

    tid1 = os_task_create("sen_rx_thread", sensor_irq_rx_entry, sensor, 1024, 15, 5);

    if (tid1 != OS_NULL)
        os_task_startup(tid1);

    struct os_device_cb_info cb_info = 
    {
        .type = OS_DEVICE_CB_TYPE_RX,
        .cb   = rx_callback,
    };

    os_device_control(dev, OS_DEVICE_CTRL_SET_CB, &cb_info);

    if (os_device_open(dev) != OS_EOK)
    {
        LOG_E(DBG_TAG,"open device failed!");
        return;
    }
    os_device_control(dev, OS_SENSOR_CTRL_SET_ODR, (void *)20);
}
SH_CMD_EXPORT(sensor_int, sensor_int_test, "Sensor interrupt mode test function");

static void sensor_polling_test(int argc, char **argv)
{
    uint16_t              num = 10;
    os_device_t          *dev = OS_NULL;
    os_sensor_t           sensor;
    struct os_sensor_data data;
    os_size_t             res, i;

    dev = os_device_find(argv[1]);
    if (dev == OS_NULL)
    {
        LOG_E(DBG_TAG,"Can't find device:%s", argv[1]);
        return;
    }
    if (argc > 2)
        num = atoi(argv[2]);

    sensor = (os_sensor_t)dev;

    if (os_device_open(dev) != OS_EOK)
    {
        LOG_E(DBG_TAG,"open device failed!");
        return;
    }
    os_device_control(dev, OS_SENSOR_CTRL_SET_ODR, (void *)100);

    for (i = 0; i < num; i++)
    {
        res = os_device_read_nonblock(dev, 0, &data, 1);
        if (res != 1)
        {
            LOG_E(DBG_TAG,"read data failed!size is %d", res);
        }
        else
        {
            sensor_show_data(i, sensor, &data);
        }
        os_task_msleep(100);
    }
    os_device_close(dev);
}
SH_CMD_EXPORT(sensor_polling, sensor_polling_test, "Sensor polling mode test function");

static void sensor_test(int argc, char **argv)
{
    static os_device_t   *dev = OS_NULL;
    struct os_sensor_data data;
    os_size_t             res, i;

    /* If the number of arguments less than 2 */
    if (argc < 2)
    {
        os_kprintf("\r\n");
        os_kprintf("sensor  [OPTION] [PARAM]\r\n");
        os_kprintf("         probe <dev_name>      Probe sensor by given name\r\n");
        os_kprintf("         info                  Get sensor info\r\n");
        os_kprintf("         sr <var>              Set range to var\r\n");
        os_kprintf("         sm <var>              Set work mode to var\r\n");
        os_kprintf("         sp <var>              Set power mode to var\r\n");
        os_kprintf("         sodr <var>            Set output date rate to var\r\n");
        os_kprintf("         read [num]            Read [num] times sensor\r\n");
        os_kprintf("                               num default 5\r\n");
        return;
    }
    else if (!strcmp(argv[1], "info"))
    {
        struct os_sensor_info info;
        if (dev == OS_NULL)
        {
            LOG_W(DBG_TAG,"Please probe sensor device first!");
            return;
        }
        os_device_control(dev, OS_SENSOR_CTRL_GET_INFO, &info);
        switch (info.vendor)
        {
        case OS_SENSOR_VENDOR_UNKNOWN:
            os_kprintf("vendor    :unknown vendor\r\n");
            break;
        case OS_SENSOR_VENDOR_STM:
            os_kprintf("vendor    :STMicroelectronics\r\n");
            break;
        case OS_SENSOR_VENDOR_BOSCH:
            os_kprintf("vendor    :Bosch\r\n");
            break;
        case OS_SENSOR_VENDOR_INVENSENSE:
            os_kprintf("vendor    :Invensense\r\n");
            break;
        case OS_SENSOR_VENDOR_SEMTECH:
            os_kprintf("vendor    :Semtech\r\n");
            break;
        case OS_SENSOR_VENDOR_GOERTEK:
            os_kprintf("vendor    :Goertek\r\n");
            break;
        case OS_SENSOR_VENDOR_MIRAMEMS:
            os_kprintf("vendor    :MiraMEMS\r\n");
            break;
        case OS_SENSOR_VENDOR_DALLAS:
            os_kprintf("vendor    :Dallas\r\n");
            break;
        }
        os_kprintf("model     :%s\r\n", info.model);
        switch (info.unit)
        {
        case OS_SENSOR_UNIT_NONE:
            os_kprintf("unit      :none\r\n");
            break;
        case OS_SENSOR_UNIT_MG:
            os_kprintf("unit      :mG\r\n");
            break;
        case OS_SENSOR_UNIT_MDPS:
            os_kprintf("unit      :mdps\r\n");
            break;
        case OS_SENSOR_UNIT_MGAUSS:
            os_kprintf("unit      :mGauss\r\n");
            break;
        case OS_SENSOR_UNIT_LUX:
            os_kprintf("unit      :lux\r\n");
            break;
        case OS_SENSOR_UNIT_CM:
            os_kprintf("unit      :cm\r\n");
            break;
        case OS_SENSOR_UNIT_PA:
            os_kprintf("unit      :pa\r\n");
            break;
        case OS_SENSOR_UNIT_PERMILLAGE:
            os_kprintf("unit      :permillage\r\n");
            break;
        case OS_SENSOR_UNIT_DCELSIUS:
            os_kprintf("unit      :Celsius\r\n");
            break;
        case OS_SENSOR_UNIT_HZ:
            os_kprintf("unit      :HZ\r\n");
            break;
        case OS_SENSOR_UNIT_ONE:
            os_kprintf("unit      :1\r\n");
            break;
        case OS_SENSOR_UNIT_BPM:
            os_kprintf("unit      :bpm\r\n");
            break;
        case OS_SENSOR_UNIT_MM:
            os_kprintf("unit      :mm\r\n");
            break;
        case OS_SENSOR_UNIT_MN:
            os_kprintf("unit      :mN\r\n");
            break;
        }
        os_kprintf("range_max :%d\r\n", info.range_max);
        os_kprintf("range_min :%d\r\n", info.range_min);
        os_kprintf("period_min:%dms\r\n", info.period_min);
        os_kprintf("fifo_max  :%d\r\n", info.fifo_max);
    }
    else if (!strcmp(argv[1], "read"))
    {
        uint16_t num = 5;

        if (dev == OS_NULL)
        {
            LOG_W(DBG_TAG,"Please probe sensor device first!");
            return;
        }
        if (argc == 3)
        {
            num = atoi(argv[2]);
        }

        for (i = 0; i < num; i++)
        {
            res = os_device_read_nonblock(dev, 0, &data, 1);
            if (res != 1)
            {
                LOG_E(DBG_TAG,"read data failed!size is %d", res);
            }
            else
            {
                sensor_show_data(i, (os_sensor_t)dev, &data);
            }
            os_task_msleep(100);
        }
    }
    else if (argc == 3)
    {
        if (!strcmp(argv[1], "probe"))
        {
            os_uint8_t reg = 0xFF;
            if (dev)
            {
                os_device_close(dev);
            }

            dev = os_device_find(argv[2]);
            if (dev == OS_NULL)
            {
                LOG_E(DBG_TAG,"Can't find device:%s", argv[1]);
                return;
            }
            if (os_device_open(dev) != OS_EOK)
            {
                LOG_E(DBG_TAG,"open device failed!");
                return;
            }
            os_device_control(dev, OS_SENSOR_CTRL_GET_ID, &reg);
            LOG_I(DBG_TAG,"device id: 0x%x!", reg);
        }
        else if (dev == OS_NULL)
        {
            LOG_W(DBG_TAG,"Please probe sensor first!");
            return;
        }
        else if (!strcmp(argv[1], "sr"))
        {
            os_device_control(dev, OS_SENSOR_CTRL_SET_RANGE, (void *)atoi(argv[2]));
        }
        else if (!strcmp(argv[1], "sm"))
        {
            os_device_control(dev, OS_SENSOR_CTRL_SET_MODE, (void *)atoi(argv[2]));
        }
        else if (!strcmp(argv[1], "sp"))
        {
            os_device_control(dev, OS_SENSOR_CTRL_SET_POWER, (void *)atoi(argv[2]));
        }
        else if (!strcmp(argv[1], "sodr"))
        {
            os_device_control(dev, OS_SENSOR_CTRL_SET_ODR, (void *)atoi(argv[2]));
        }
        else
        {
            LOG_W(DBG_TAG,"Unknown command, please enter 'sensor' get help information!");
        }
    }
    else
    {
        LOG_W(DBG_TAG,"Unknown command, please enter 'sensor' get help information!");
    }
}
SH_CMD_EXPORT(sensor, sensor_test, "sensor test function");

#endif
