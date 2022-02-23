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
 * @file        spi_tfcard.c
 *
 * @brief       This file provides functions for spi tfcard.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include "drv_spi.h"
#include "spi_msd.h"
#include "drv_gpio.h"

static int os_hw_spi_tfcard(void)
{
    os_hw_spi_device_attach(BSP_SDCARD_SPI_DEV, "spi_tfcard", BSP_SDCARD_SPI_CS_PIN);
    msd_init("sd0", "spi_tfcard");
    return OS_EOK;
}
OS_DEVICE_INIT(os_hw_spi_tfcard, OS_INIT_SUBLEVEL_HIGH);
