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
 * @file        flash_spi.c
 *
 * @brief       This file provides functions for spi flash init.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <drv_spi.h>
#include <drv_cfg.h>
#include <arch_interrupt.h>

#include "spi_flash.h"
#include "spi_flash_sfud.h"
#include <drv_log.h>

static int os_hw_spi_flash_with_sfud_init(void)
{
    /* SPI */
    os_hw_spi_device_attach(OS_SPI_FLASH_BUS_NAME, OS_EXTERN_FLASH_BUS_NAME, OS_SPI_FLASH_CS_PIN);

    /* Init*/
    if (OS_NULL == os_sfud_flash_probe(OS_EXTERN_FLASH_DEV_NAME, OS_EXTERN_FLASH_BUS_NAME))
    {
        return OS_ERROR;
    }

    return OS_EOK;
}
OS_DEVICE_INIT(os_hw_spi_flash_with_sfud_init, OS_INIT_SUBLEVEL_HIGH);
