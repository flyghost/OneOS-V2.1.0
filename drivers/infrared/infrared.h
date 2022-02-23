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
 * @file        infrared.h
 *
 * @brief       This file provides.infrared functions declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __INFRARED_H__
#define __INFRARED_H__

#include <os_task.h>
#include <os_list.h>
#include <os_sem.h>
#include <device.h>

struct os_infrared_info
{
    os_uint8_t addr;
    os_uint8_t data;
    os_uint8_t times;
};

struct os_infrared_device
{
    struct os_device parent;

    int      tx_pin;
    int      rx_pin;

    int         rx_status;
    os_uint64_t rx_time_stamp;
    os_uint16_t rx_addr, rx_addr_offset;
    os_uint16_t rx_data, rx_data_offset;

    int info_head, info_tail;
    struct os_infrared_info info[8];

    os_list_node_t list;
};

#define OS_INFRARED_INVALIDE_PIN (-1)

os_err_t os_infrared_register_device(const char *name, struct os_infrared_device *device);

#endif
