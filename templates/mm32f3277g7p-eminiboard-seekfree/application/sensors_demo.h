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
 * @file        sensors_demo.h
 *
 * @brief       sensors_demo
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __SENSOR_DEMO_H__
#define __SENSOR_DEMO_H__

#include <os_types.h>

#define SENSOR_ADC_NAME                         "adc1"
#define SENSOR_NCP18XH103F03RB_ADC_CHANNEL      0
#define SENSOR_GL5516_ADC_CHANNEL               1

#define SENSOR_NCP18XH103F03RB_V_REF            3300 
#define SENSOR_NCP18XH103F03RB_R_REF            (10*1000)
#define SENSOR_NCP18XH103F03RB_R_TYPICAL        (10*1000)
#define SENSOR_NCP18XH103F03RB_TEMP_TYPICAL     (25 + 273.15) 
#define SENSOR_NCP18XH103F03RB_B_VALUE          3380
#define SENSOR_GL5516_V_REF                     3300
#define SENSOR_GL5516_R_REF                     (10*1000)
#define SENSOR_GL5516_R_TYPICAL                 (10*1000)                //typical value at 10 lux in datasheet
#define SENSOR_GL5516_R_K_TEMP                  0.003
#define SENSOR_GL5516_R_B_TEMP                  1.04

#define SENSOR_DATA_MAX_NUM                     10
#define SENSOR_DATA_GET_PERIOD                  1000

os_err_t sensors_output_get(char *temp_buff, char *light_buff, os_uint32_t size);
int sensors_demo_init(void);
#endif
