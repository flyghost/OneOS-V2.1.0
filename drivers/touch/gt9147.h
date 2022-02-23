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
 * @file        gt9147.h
 *
 * @brief       gt9147
 *
 * @details     gt9147
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __GT9147_H_
#define __GT9147_H_
#include <os_task.h>
#include "touch/touch.h"

#define GTP_ADDR_LENGTH       (2)
#define GT9147_MAX_TOUCH      (5)
#define GT9147_POINT_INFO_NUM (8)

#define GT9147_ADDRESS_HIGH (OS_GT9147_I2C_ADDR_HIGH)
#define GT9147_ADDRESS_LOW  (OS_GT9147_I2C_ADDR_LOW)

#define GT9147_COMMAND (0x8040)
#define GT9147_CONFIG  (0x8047)

#define GT9XX_PRODUCT_ID   (0x8140)
#define GT9147_READ_STATUS (0x814E)

#define GT9147_POINT1_REG (0x814F)
#define GT9147_POINT2_REG (0X8157)
#define GT9147_POINT3_REG (0X815F)
#define GT9147_POINT4_REG (0X8167)
#define GT9147_POINT5_REG (0X816F)

#define GT9147_CHECK_SUM (0X80FF)

#endif
