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
 * @file        adc_dac_test.c
 *
 * @brief       The test file for adc and dac.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <stdlib.h>
#include <device.h>
#include <unistd.h>
#include <drv_log.h>
#include <drv_cfg.h>

#ifdef OS_USING_SHELL
#include <shell.h>
#endif

/*
*   adc and dac should config one channel,and connet together;
*/
int adc_dac_test(int argc, char **argv)
{
    int   adc_channel;
    int   dac_channel;
    int   voltage_output = 0;
    int   voltage_input = 0;
    char *adc_name;
    char *dac_name;
    os_err_t ret = OS_EOK;

    os_device_t *adc;    
    os_device_t *dac;
    
    if (argc != 5)
    {
        os_kprintf("usage: adc_dac_test adc_dev <dac_channel> dac_dev <dac_channel>\r\n");
        os_kprintf("       adc_dac_test  adc1         0           dac1         1 \r\n");
        return -1;
    }
    
    adc_name = argv[1];
    dac_name = argv[3];
    adc_channel = strtol(argv[2], OS_NULL, 0);
    dac_channel = strtol(argv[4], OS_NULL, 0);

    adc = os_device_find(adc_name);
    if (adc == OS_NULL)
    {
        os_kprintf("invalid adc device %s.\r\n", adc_name);
        return -2;
    }
    
    os_device_open(adc);

    dac = os_device_find(dac_name);
    if (dac == OS_NULL)
    {
        os_device_close(adc);
        os_kprintf("invalid dac device %s.\r\n", dac_name);
        return -2;
    }

    os_device_open(dac);

    os_device_control(adc, OS_ADC_CMD_ENABLE, OS_NULL);
    os_device_control(dac, OS_DAC_CMD_ENABLE, (void *)dac_channel);
    
    while (voltage_output <= 3300)
    {
        os_device_write_nonblock(dac, dac_channel, &voltage_output, sizeof(voltage_output));
        os_task_msleep(10);
        ret = os_device_read_nonblock(adc, adc_channel, &voltage_input, sizeof(voltage_input));
        os_kprintf("dac output %d mv. adc input %d mv\r\n", voltage_output, voltage_input);
        voltage_output += 550;
        os_task_msleep(400);
    }
    os_device_close(adc);
    os_device_close(dac);
    return ret;
}

/* Export to msh command list */
SH_CMD_EXPORT(adc_dac_test, adc_dac_test, "test adc and dac");
