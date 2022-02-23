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
 * @file        pulse_encoder_test.c
 *
 * @brief       The test file for pulse encoder.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <device.h>
#include "drv_gpio.h"
#include <os_errno.h>


#ifdef OS_USING_SHELL
#include <shell.h>
#endif

int pulse_encoder_sample(int argc, char **argv)
{
    os_err_t     ret  = OS_EOK;
    os_int32_t count = 0;
    
    char *dev_name;
    os_pulse_encoder_device_t *pulse_encoder_dev = OS_NULL;
    
    if (argc != 2)
    {
        os_kprintf("usage: pulse_encoder_sample <dev>\r\n");
        os_kprintf("       pulse_encoder_sample encoder_tim1\r\n");
        return -1;
    }
    
    dev_name = argv[1];
    pulse_encoder_dev = (os_pulse_encoder_device_t *)os_device_find(dev_name);
    if (pulse_encoder_dev == OS_NULL)
    {
        os_kprintf("pulse encoder sample run failed! can't find %s device!\r\n", dev_name);
        return OS_ERROR;
    }

    ret = os_pulse_encoder_enable(pulse_encoder_dev);
    
    for (int i = 0; i <= 10; i++)
    {
        os_task_msleep(500);
        ret = os_pulse_encoder_read(pulse_encoder_dev, &count);
        os_kprintf("get count %d\r\n", count);
    }

    return ret;
}
/* Export to msh command list */
SH_CMD_EXPORT(pulse_encoder_sample, pulse_encoder_sample, "pulse_encoder_sample");
