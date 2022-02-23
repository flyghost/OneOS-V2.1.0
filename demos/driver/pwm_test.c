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
 * @file        pwm_test.c
 *
 * @brief       The test file for pwm.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <device.h>
#include <os_clock.h>
#include <os_memory.h>
#include <stdlib.h>
#include <shell.h>
#include <timer/clocksource.h>

static void pwm_pin_callback(void *args)
{
    int pin = (int)(unsigned long)args;

//    os_kprintf("<%d>   <%d>----------------------pin:%d value:%d\r\n", (int)os_tick_get(), (int)(os_clocksource_gettime() & 0x7fffffff), pin, os_pin_read(pin));
}

int pwm_sample(int argc, char **argv)
{
    os_uint32_t pin;
    
    char *dev_name;
    
    os_device_t *pwm_dev = OS_NULL;
    
    struct os_pwm_configuration *config;
    
    config = os_calloc(1, sizeof(struct os_pwm_configuration));
    if (argc < 3)
    {
        os_kprintf("usage: pwm_sample <dev> <channel> [period(ns)] [duty(ns) def 1000000] [pin]\r\n");
        os_kprintf("       pwm_sample pwm_tim1 1 default: 5000000, 1000000\r\n");
        os_kprintf("       pwm_sample pwm_tim1 1 5000000 1000000 0x3e\r\n");
        os_kprintf("       pwm_sample pwm_tim1 1 1000000000 1000000000 0x3e\r\n");
        os_kprintf("       pwm_sample pwm_tim1 1 1000000000 0 0x3e\r\n");
        return -1;
    }
    
    dev_name = argv[1];
    config->channel  = strtol(argv[2], OS_NULL, 0);

    if (argc > 3)
    {
        config->period = strtol(argv[3], OS_NULL, 0);
    }

    if (argc > 4)
    {
        config->pulse = strtol(argv[4], OS_NULL, 0);
    }

    if (argc > 5)
    {
        pin = strtol(argv[5], OS_NULL, 0);
        
        os_pin_mode(pin, PIN_MODE_INPUT_PULLUP);
        os_pin_attach_irq(pin, PIN_IRQ_MODE_RISING_FALLING, pwm_pin_callback, (void *)pin);
        os_pin_irq_enable(pin, PIN_IRQ_ENABLE);
    }
    
    pwm_dev = os_device_find(dev_name);
    if (pwm_dev == OS_NULL)
    {
        os_kprintf("pwm sample run failed! can't find %s device!\r\n", dev_name);
        return OS_ERROR;
    }

    os_device_control(pwm_dev, OS_PWM_CMD_SET_PERIOD, config);
    
    os_device_control(pwm_dev, OS_PWM_CMD_SET_PULSE, config);

    os_device_control(pwm_dev, OS_PWM_CMD_ENABLE, config);

    os_task_msleep(10000);
    
    os_device_control(pwm_dev, OS_PWM_CMD_DISABLE, config);
    
    return 0;
}

SH_CMD_EXPORT(pwm_sample, pwm_sample, "test_pwm_sample!");

