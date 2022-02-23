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
 * @file        sensor_light_test.c
 *
 * @brief       The test file for light sensor.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_clock.h>
#include <stdio.h>
#include <shell.h>
#include <sensors/sensor.h>

static int sensor_light_test(int argc, char *argv[])
{
    int  i;
    char sensor_name[24];
    struct os_sensor_data sensor_data;

    if (argc != 2)
    {
        os_kprintf("usage:sensor_light_test <sensor_name>\r\n");
        return -1;
    }

    snprintf(sensor_name, sizeof(sensor_name) - 1, "li_%s", argv[1]);

    os_device_t *sensor = os_device_find(sensor_name);
    OS_ASSERT(sensor != NULL);
    os_device_open(sensor);

    struct os_sensor_info sensor_info;
    os_device_control(sensor, OS_SENSOR_CTRL_GET_INFO, &sensor_info);

    for (i = 0; i < 10; i++)
    {
        os_device_read_nonblock(sensor, 0, &sensor_data, sizeof(struct os_sensor_data));

        if (sensor_info.unit == OS_SENSOR_UNIT_MLUX)
        {
            os_kprintf("sensor light (%d.%03d)\r\n", sensor_data.data.light / 1000, sensor_data.data.light % 1000);
        }
        else if (sensor_info.unit == OS_SENSOR_UNIT_LUX)
        {
            os_kprintf("sensor light (%d)\r\n", sensor_data.data.light);
        }
        else if (sensor_info.unit == OS_SENSOR_UNIT_RAW)
        {
            os_kprintf("sensor light raw(%d) mV\r\n", sensor_data.data.light);
        }
        else
        {
            os_kprintf("invalid unit\r\n");
        }

        os_task_msleep(1000);
    }

    return 0;
}
SH_CMD_EXPORT(sensor_light_test, sensor_light_test, "sensor_light_test");
