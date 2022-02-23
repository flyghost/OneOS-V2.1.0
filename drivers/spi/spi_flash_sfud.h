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
 * @file        spi_flash_sfud.h
 *
 * @brief       This file provides functions declaration for spi flash sfud.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef _SPI_FLASH_SFUD_H_
#define _SPI_FLASH_SFUD_H_

#include <os_task.h>
#include <device.h>
#include "./sfud/inc/sfud.h"
#include "spi_flash.h"

os_spi_flash_device_t os_sfud_flash_probe(const char *spi_flash_dev_name, const char *spi_dev_name);

os_err_t os_sfud_flash_delete(os_spi_flash_device_t spi_flash_dev);

sfud_flash_t os_sfud_flash_find(const char *spi_dev_name);

sfud_flash_t os_sfud_flash_find_by_dev_name(const char *flash_dev_name);

#endif /* _SPI_FLASH_SFUD_H_ */
