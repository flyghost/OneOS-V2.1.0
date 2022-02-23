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
 * @file        sensors_demo.c
 *
 * @brief       sensors_demo
 *
 * @revision
 * Date         Author          Notes
 * 2021-05-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <sensors_demo.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <os_memory.h>
#include <os_clock.h>
#include <shell.h>
#include <drv_cfg.h>

#define DRV_EXT_LVL DBG_EXT_INFO
#define DRV_EXT_TAG "demo.sensor"
#include <drv_log.h>

struct sensor_adc_info
{
    os_uint32_t         voltage;
    float               resister;
    float               value;
};


struct sensor_demo_info
{
    os_device_t            *snesor_adc;
    struct sensor_adc_info  temp[SENSOR_DATA_MAX_NUM];
    struct sensor_adc_info  light[SENSOR_DATA_MAX_NUM];
    os_uint8_t              index;
    os_uint8_t              err_count;
    
    char                    temp_output_buf[10][10];
    char                    light_output_buf[100];
    os_uint16_t             output_write_index;
    os_uint16_t             output_read_index;
};

struct sensor_demo_info snesor_demo;

static float sensor_get_temp(struct sensor_demo_info *snesor_demo, os_uint32_t voltage)
{
    float       ref = 0;
    float       resister = 0;
    float       value = 0;
    //T1 =ln(Rt/R)/B+1/T2
    ref = ((float)voltage) / SENSOR_NCP18XH103F03RB_V_REF;
    resister = (float)((1 - ref) / ref * SENSOR_NCP18XH103F03RB_R_REF);
    
    ref = log(resister / SENSOR_NCP18XH103F03RB_R_TYPICAL) / SENSOR_NCP18XH103F03RB_B_VALUE; 
    value = 1 / (ref + 1 / SENSOR_NCP18XH103F03RB_TEMP_TYPICAL) - 273.15;
    
    snesor_demo->temp[snesor_demo->index].voltage  = voltage;
    snesor_demo->temp[snesor_demo->index].resister = resister;
    snesor_demo->temp[snesor_demo->index].value    = value;
    
    return value;
}

static float sensor_get_light(struct sensor_demo_info *snesor_demo, os_uint32_t voltage)
{
    float       ref = 0;
    float       k_temp = 0;

    float       resister = 0;
    float       value = 0;
    //T1 =ln(Rt/R)/B+1/T2
    if (snesor_demo->temp[snesor_demo->index].value > 20)
        k_temp = 1.1;
    else
        k_temp = SENSOR_GL5516_R_K_TEMP * snesor_demo->temp[snesor_demo->index].value + SENSOR_GL5516_R_B_TEMP;

    ref = ((float)voltage) / SENSOR_NCP18XH103F03RB_V_REF;
    resister = (float)((1 - ref) / ref * SENSOR_NCP18XH103F03RB_R_REF);
    
    value = pow(10, (2 * (log10(SENSOR_GL5516_R_TYPICAL * k_temp) - log10(resister * k_temp)) + 1));
    
    snesor_demo->light[snesor_demo->index].voltage  = voltage;
    snesor_demo->light[snesor_demo->index].resister = resister;
    snesor_demo->light[snesor_demo->index].value    = value;
    
    return value;
}

const char *temp_output_format  = "%5.2f";
const char *light_output_format = "%5.2f;";
static void sensors_output_updata(struct sensor_demo_info *snesor_demo)
{
    os_uint8_t i = 0;
    os_ubase_t level;
    char char_buf[10];
    
    level = os_irq_lock();
    for (i = 0; i < SENSOR_DATA_MAX_NUM; i++)
    {
        snprintf(snesor_demo->temp_output_buf[i], 10, temp_output_format, snesor_demo->temp[i].value);
        snprintf(char_buf, 6, light_output_format, snesor_demo->light[i].value);
        strcat(snesor_demo->light_output_buf, char_buf);
    }
    snesor_demo->output_write_index++;
    os_irq_unlock(level);
}

OS_UNUSED os_err_t sensors_output_get(char *temp_buff, char *light_buff, os_uint32_t size)
{
    os_ubase_t level;
    if (size < 100)
        return OS_ERROR;
    if (snesor_demo.output_write_index == snesor_demo.output_read_index)
        return OS_EEMPTY;
    level = os_irq_lock();
    memcpy(temp_buff, snesor_demo.temp_output_buf, 100);
    memcpy(light_buff, snesor_demo.light_output_buf, 100);
    snesor_demo.output_read_index = snesor_demo.output_write_index;
    os_irq_unlock(level);
    return OS_EOK;
}

static void sensors_demo_task(void *parameter)
{
    os_uint32_t temp_voltage, light_voltage;
    
    snesor_demo.snesor_adc = os_device_find(SENSOR_ADC_NAME);
    if (snesor_demo.snesor_adc == OS_NULL)
    {
        LOG_E(DRV_EXT_TAG, "cannot find %s!", SENSOR_ADC_NAME);
        return;
    }
    if (os_device_open(snesor_demo.snesor_adc) != OS_EOK)
    {
        LOG_E(DRV_EXT_TAG, "cannot open %s!", SENSOR_ADC_NAME);
        return;
    }
    os_device_control(snesor_demo.snesor_adc, OS_ADC_CMD_ENABLE, OS_NULL);
    
    while (1)
    {
        if ((os_device_read_nonblock(snesor_demo.snesor_adc, SENSOR_GL5516_ADC_CHANNEL, &temp_voltage, sizeof(os_uint32_t)) > 0)
            && (os_device_read_nonblock(snesor_demo.snesor_adc, SENSOR_NCP18XH103F03RB_ADC_CHANNEL, &light_voltage, sizeof(os_uint32_t)) > 0))
        {
            sensor_get_temp(&snesor_demo, temp_voltage);
            sensor_get_light(&snesor_demo, light_voltage);

            if (snesor_demo.index == SENSOR_DATA_MAX_NUM - 1)
            {
                snesor_demo.index = 0;
                sensors_output_updata(&snesor_demo);
            }
            else
                snesor_demo.index++;
        }
        else
        {
            snesor_demo.err_count++;
            LOG_E(DRV_EXT_TAG, "get sensors data failed!");
        }
        os_task_msleep(SENSOR_DATA_GET_PERIOD);
    }
}

int sensors_demo_init(void)
{
    os_task_t *task;

    task = os_task_create("sensors_demo", sensors_demo_task, NULL, 1024, 3);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}

SH_CMD_EXPORT(sensors_demo, sensors_demo_init, "test sensors_demo");
