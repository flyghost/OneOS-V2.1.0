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
 * @file        adc.h
 *
 * @brief       this file implements adc related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __ADC_H__
#define __ADC_H__
#include <os_assert.h>
#include <os_errno.h>
#include <device.h>


#define OS_ADC_EVENT_CONVERT_DONE           0x00

#define OS_ADC_CMD_ENABLE   IOC_MISCELLANEOUS(1)
#define OS_ADC_CMD_DISABLE  IOC_MISCELLANEOUS(2)

#define OS_ADC_ENABLE       0x01
#define OS_ADC_DISABLE      0x00

struct os_adc_device
{
    struct os_device         parent;
    const struct os_adc_ops *ops;
    os_int32_t  ref_hight;
    os_int32_t  ref_low;
    os_uint32_t max_value;

    os_uint32_t mult;       /* adc voltage(mV) = value * mult >> shift */
    os_uint32_t shift;
    
};
typedef struct os_adc_device os_adc_device_t;

struct os_adc_ops
{
    os_err_t (*adc_enabled)(struct os_adc_device *dev, os_bool_t enable);
    os_err_t (*adc_control)(struct os_adc_device *dev, int cmd, void *arg);
    os_err_t (*adc_read)(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buffer);
};

void os_hw_adc_isr(struct os_adc_device *dev, int event);
os_err_t os_adc_read(struct os_adc_device *dev, os_uint32_t channel, os_int32_t *buffer);
os_err_t os_adc_enable(struct os_adc_device *dev);
os_err_t os_adc_disable(struct os_adc_device *dev);
os_err_t os_hw_adc_register(struct os_adc_device *device, const char *name, void *data);
#endif /* __ADC_H__ */
