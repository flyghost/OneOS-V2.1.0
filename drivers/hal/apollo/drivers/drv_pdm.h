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
 * @file        drv_pdm.h
 *
 * @brief       This file provides operation functions declaration for pdm.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __PDM_H_
#define __PDM_H_

#include <os_types.h>

int        os_pdm_init(void);
os_uint8_t am_pdm_data_get(os_uint8_t *buff, os_uint16_t size);
void       am_pdm_start(void);
void       am_pdm_stop(void);

#endif /* __PDM_H_ */
