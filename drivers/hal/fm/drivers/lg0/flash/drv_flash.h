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
 * @file        drv_flash.h
 *
 * @brief       The head file of flash drv
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __DRV_FLASH_H__
#define __DRV_FLASH_H__

#include <board.h>
#include "fm33lg0xx_fl_flash.h"


#ifdef __cplusplus
extern "C" {
#endif

int fm33_flash_read(os_uint32_t addr, os_uint8_t *buf, os_size_t size);
int fm33_flash_write(os_uint32_t addr, const os_uint8_t *buf, os_size_t size);
int fm33_flash_erase(os_uint32_t addr, os_size_t size);

#ifdef __cplusplus
}
#endif

#endif  /* __DRV_FLASH_H__ */
