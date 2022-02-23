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
 * @file        adc_test.c
 *
 * @brief       The test file for adc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <shell.h>
#include <stdlib.h>

int adc_sample(int argc, char **argv)
{
    os_uint32_t adc_channel;
    os_int32_t  adc_databuf;
    os_err_t    ret = OS_EOK;

    os_device_t *adc_dev;
    
    if (argc != 3)
    {
        os_kprintf("usage: adc_sample name  channel\r\n");
        os_kprintf("       adc_sample adc1     0\r\n");
        return -1;
    }

    /* find device */
    adc_dev = os_device_find(argv[1]);
    if (adc_dev == OS_NULL)
    {
        os_kprintf("adc device not find! \r\n");
        return -1;
    }
    adc_channel = strtol(argv[2], OS_NULL, 0);;
    /* open adc */
    os_device_open(adc_dev);
    ret = os_device_control(adc_dev, OS_ADC_CMD_ENABLE, OS_NULL);
    if (ret != OS_EOK)
    {
        os_kprintf("adc device cannot enable! \r\n");
        os_device_close(adc_dev);
        return -1;
    }
    
    for (int i = 0; i < 10;i++)
    {
        ret = os_device_read_nonblock(adc_dev, adc_channel, &adc_databuf, sizeof(adc_databuf));
        if (ret == sizeof(adc_databuf))
        {
            os_kprintf("%s channel %s voltage value: %d\r\n", argv[1] ,argv[2], adc_databuf);
        }
        os_task_msleep(100);
    }
    
    os_device_control(adc_dev, OS_ADC_CMD_DISABLE, OS_NULL);
    os_device_close(adc_dev);
    return ret;
}

SH_CMD_EXPORT(adc_sample, adc_sample, "test set adc");
