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
 * @file        flash_qspi.c
 *
 * @brief       This file provides functions for qspi flash init.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_qspi.h>
#include <drv_cfg.h>
#include <arch_interrupt.h>

#include "spi_flash.h"
#include "spi_flash_sfud.h"

char w25qxx_read_status_register2(struct os_qspi_device *device)
{
    /* 0x35 read status register2 */
    char instruction = 0x35, status;

    os_qspi_send_then_recv(device, &instruction, 1, &status, 1);

    return status;
}

void w25qxx_write_enable(struct os_qspi_device *device)
{
    /* 0x06 write enable */
    char instruction = 0x06;

    os_qspi_send(device, &instruction, 1);
}

void w25qxx_enter_qspi_mode(struct os_qspi_device *device)
{
    char status = 0;
    /* 0x38 enter qspi mode */
    char instruction          = 0x38;
    char write_status2_buf[2] = {0};

    /* 0x31 write status register2 */
    write_status2_buf[0] = 0x31;

    status = w25qxx_read_status_register2(device);
    if (!(status & 0x02))
    {
        status |= 1 << 1;
        w25qxx_write_enable(device);
        write_status2_buf[1] = status;
        os_qspi_send(device, &write_status2_buf, 2);
        os_qspi_send(device, &instruction, 1);
        os_kprintf("flash already enter qspi mode\r\n");
        os_task_msleep(10);
    }
}

static int os_hw_qspi_flash_with_sfud_init(void)
{
    stm32_qspi_bus_attach_device(OS_QSPI_FLASH_BUS_NAME,
                                 OS_EXTERN_FLASH_BUS_NAME,
                                 0,
                                 4,
                                 w25qxx_enter_qspi_mode,
                                 OS_NULL);

    /* Init w25q128 */
    if (OS_NULL == os_sfud_flash_probe(OS_EXTERN_FLASH_DEV_NAME, OS_EXTERN_FLASH_BUS_NAME))
    {
        return OS_ERROR;
    }

    return OS_EOK;
}

OS_DEVICE_INIT(os_hw_qspi_flash_with_sfud_init, OS_INIT_SUBLEVEL_HIGH);
