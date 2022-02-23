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
 * @file        dac_test.c
 *
 * @brief       The test file for dac.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <shell.h>
#include <stdlib.h>
#include <drv_cfg.h>

static int dac(int argc, char **argv)
{
    char *dev_name;
    int   channel;
    int   voltage;
    
    os_device_t *dac;

    if (argc != 4)
    {
        os_kprintf("usage: dac <dev> <channel> <voltage(mV)>\r\n");
        os_kprintf("       dac dac 1 1500\r\n");
        return -1;
    }

    dev_name = argv[1];
    channel  = strtol(argv[2], OS_NULL, 0);
    voltage  = strtol(argv[3], OS_NULL, 0);

    dac = os_device_find(dev_name);
    if (dac == OS_NULL)
    {
        os_kprintf("invalid adc device %s.\r\n", dev_name);
        return -2;
    }

    os_kprintf("dac device %s, %d, %d.\r\n", dev_name, channel, voltage);

    os_device_open(dac);
    os_device_control(dac, OS_DAC_CMD_ENABLE, (void *)channel);
    os_device_write_nonblock(dac, channel, &voltage, sizeof(voltage));
    os_device_close(dac);
    return 0;
}

SH_CMD_EXPORT(dac, dac, "dac_output");

static int dac_off(int argc, char **argv)
{
    char *dev_name;
    int   channel;
    
    os_device_t *dac;

    if (argc != 3)
    {
        os_kprintf("usage: dac_off <dev> <channel>\r\n");
        os_kprintf("       dac_off dac 1\r\n");
        return -1;
    }

    dev_name = argv[1];
    channel  = strtol(argv[2], OS_NULL, 0);

    dac = os_device_find(dev_name);
    if (dac == OS_NULL)
    {
        os_kprintf("invalid adc device %s.\r\n", dev_name);
        return -2;
    }

    os_kprintf("turn off dac device %s.\r\n", dev_name);
    os_device_control(dac, OS_DAC_CMD_DISABLE, (void *)channel);
    return 0;
}

SH_CMD_EXPORT(dac_off, dac_off, "turn off dac");


