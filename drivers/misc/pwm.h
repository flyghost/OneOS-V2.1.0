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
 * @file        pwm.h
 *
 * @brief       this file implements pwm related definitions and declarations
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-02-20    OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_PWM_H_INCLUDE__
#define __DRV_PWM_H_INCLUDE__

#include <os_assert.h>
#include <os_errno.h>
#include <device.h>

#define OS_PWM_CMD_ENABLE           IOC_PWM(0x00)
#define OS_PWM_CMD_DISABLE          IOC_PWM(0x01)
#define OS_PWM_CMD_SET_PERIOD       IOC_PWM(0x02)
#define OS_PWM_CMD_SET_PULSE        IOC_PWM(0x03)

struct os_pwm_configuration
{
    os_uint32_t channel; 
    os_uint32_t period;  
    os_uint32_t pulse;   
};

struct os_pwm_device
{
    struct os_device         parent;
    const struct os_pwm_ops *ops;
    os_uint32_t period;
    os_uint32_t max_value;
};

struct os_pwm_ops
{
    os_err_t (*enabled)(struct os_pwm_device *dev, os_uint32_t channel, os_bool_t enable);
    os_err_t (*set_period)(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t period);
    os_err_t (*set_pulse)(struct os_pwm_device *dev, os_uint32_t channel, os_uint32_t pulse);
    os_err_t (*control)(struct os_pwm_device *dev, int cmd, void *args);
};

typedef struct os_pwm_device os_pwm_device_t;

os_err_t os_pwm_enable(struct os_pwm_device *pwm, os_uint32_t channel);
os_err_t os_pwm_disable(struct os_pwm_device *pwm, os_uint32_t channel);
os_err_t os_pwm_set_pulse(struct os_pwm_device *pwm, os_uint32_t channel, os_uint32_t pulse);
os_err_t os_pwm_set_period(struct os_pwm_device *pwm, os_uint32_t channel, os_uint32_t period);
os_err_t os_device_pwm_register(struct os_pwm_device    *device,
                                const char              *name);

#endif /* __DRV_PWM_H_INCLUDE__ */
