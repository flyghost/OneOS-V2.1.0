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
 * @file        drv_smbus.h
 *
 * @brief       This file provides operation functions declaration for smbus.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __SMBUS_H_
#define __SMBUS_H_

#include <os_types.h>

int os_hw_smbus_init(void);
os_uint8_t am_smbus_tx_then_tx(os_uint8_t SlaveAddress, os_uint8_t command, os_uint8_t *pBuffer, os_uint16_t bytesNumber);
os_uint8_t am_smbus_tx_then_rx(os_uint8_t SlaveAddress, os_uint8_t command, os_uint8_t *pBuffer, os_uint16_t bytesNumber);
void am_smbus_scl_high(void);
void am_smbus_scl_low(void);

#endif /* __SMBUS_H_ */
