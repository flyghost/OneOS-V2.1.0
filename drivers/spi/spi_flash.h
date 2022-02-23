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
 * @file        spi_flash.h
 *
 * @brief       This file provides struct definition
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef SPI_FLASH_H__
#define SPI_FLASH_H__

#include <os_mutex.h>
#include <device.h>
#include <mtd/mtd.h>

struct spi_flash_device
{
    os_mtd_device_t         mtd_dev;
    struct os_spi_device   *os_spi_device;
    struct os_mutex         lock;
    void                   *user_data;
};

typedef struct spi_flash_device *os_spi_flash_device_t;

#endif
