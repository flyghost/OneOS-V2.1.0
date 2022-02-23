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
 * @file        sdmmc_test.c
 *
 * @brief       The test file for sdmmc.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <arch_interrupt.h>
#include <device.h>
#include <shell.h>
#include <os_errno.h>
#include <board.h>
#include <drv_log.h>
#include <vfs_fs.h>

/*Set the NRF_CE_PIN8*/

int sdmmc_test(void)
{
    /* Mount the file system from tf card */
    if (vfs_mount("sd0", "/", "fat", 0, 0) == 0)
    {
        os_kprintf("Filesystem initialized!\r\n");
        return OS_EOK;
    }
    else
    {
        os_kprintf("Failed to initialize filesystem!\r\n");
        return OS_ERROR;
    }

}
SH_CMD_EXPORT(sdmmc_test, sdmmc_test, "sdmmc_test");
