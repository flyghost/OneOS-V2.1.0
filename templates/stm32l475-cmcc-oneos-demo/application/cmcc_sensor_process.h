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
 * @file        cmcc_sensor_process.h
 *
 * @brief       This file provides sensor function declaration and data definition.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMCC_SENSOR_PROCESS_H__
#define __CMCC_SENSOR_PROCESS_H__
#include <os_task.h>
#include <stdint.h>
#include <device.h>

typedef struct
{
    float    aht10_data_temp;
    float    aht10_data_humi;
    float    ap3216_data_als;
    uint16_t ap3216_data_ps;
    uint8_t  reserved;
} cmcc_sensor_data_t;


/* sensor data handle */
void cmcc_sensor_data_read(void *arg);
cmcc_sensor_data_t *cmcc_sensor_data_result_get(void);
void cmcc_sensor_data_upload(cmcc_sensor_data_t *in_data, cmcc_sensor_data_t *out_data);

/* init */
void cmcc_sensor_init(void);

#endif
