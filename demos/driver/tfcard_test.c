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
 * @file        tfcard_test.c
 *
 * @brief       
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_memory.h>
#include <string.h>
#include <shell.h>
#include <string.h>
#include <stdlib.h>
#include <driver.h>
#include <fal/fal.h>

static int fal_sd_read_page(fal_flash_t *flash, os_uint32_t page_addr, os_uint8_t *buff, os_uint32_t page_nr)
{
    os_device_t *dev = flash->priv;

    os_device_read_nonblock(dev, page_addr, buff, page_nr);
    return 0;
}

static int fal_sd_write_page(fal_flash_t *flash, os_uint32_t page_addr, const os_uint8_t *buff, os_uint32_t page_nr)
{
    os_device_t *dev = flash->priv;

    os_device_write_nonblock(dev, page_addr, buff, page_nr);
    return 0;
}

static int fal_sd_erase_block(fal_flash_t *flash, os_uint32_t page_addr, os_uint32_t page_nr)
{
    int i, ret;
    os_uint8_t *buff = os_calloc(1, flash->page_size);
    os_device_t *dev = flash->priv;

    memset(buff, 0xff, flash->page_size);

    for (i = 0; i < page_nr; i ++)
    {
        ret = os_device_write_nonblock(dev, (page_addr + i), buff, 1);
        if (ret != 1)
        {
            return ret;
        }
    }

    return 0;
}


static int fal_sd(void)
{
    const char  *flash_name = "sd0block";

    os_device_t *dev = os_device_find("sd0block");

    OS_ASSERT(dev);

    os_device_open(dev);

    fal_flash_dynamic_t *fal_flash = os_calloc(1, sizeof(fal_flash_dynamic_t));

    if (fal_flash == OS_NULL)
    {
        os_kprintf("fal nand mem leak %s.\r\n", flash_name);
        return -1;
    }

    memcpy(fal_flash->flash.name,
           flash_name,
           min(FAL_DEV_NAME_MAX - 1, strlen(flash_name)));
    fal_flash->flash.name[min(FAL_DEV_NAME_MAX - 1, strlen(flash_name))] = 0;

    fal_flash->flash.capacity   = 10 * 1024 * 1024;
    fal_flash->flash.block_size = 512;
    fal_flash->flash.page_size  = 512;

    fal_flash->flash.ops.init        = OS_NULL;
    fal_flash->flash.ops.read_page   = fal_sd_read_page,
    fal_flash->flash.ops.write_page  = fal_sd_write_page,
    fal_flash->flash.ops.erase_block = fal_sd_erase_block,

    fal_flash->flash.priv = dev;

    return fal_dynamic_flash_register(fal_flash);
}
OS_DEVICE_INIT(fal_sd, OS_INIT_SUBLEVEL_MIDDLE);
