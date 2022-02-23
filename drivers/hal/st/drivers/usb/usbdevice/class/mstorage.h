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
 * @file        mstorage.h
 *
 * @brief       This file provides mstorage struct definition.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MSTORAGE_H__
#define __MSTORAGE_H__

#include <os_task.h>

#pragma pack(1)

struct umass_descriptor
{
#ifdef OS_USB_DEVICE_COMPOSITE
    struct uiad_descriptor iad_desc;
#endif
    struct uinterface_descriptor intf_desc;
    struct uendpoint_descriptor  ep_out_desc;
    struct uendpoint_descriptor  ep_in_desc;
};
typedef struct umass_descriptor *umass_desc_t;

struct capacity_data
{
    os_uint8_t LastLogicalBlockAddress[4];
    os_uint8_t BlockLengthInBytes[4];
};

struct request_sense_data
{
    os_uint8_t ErrorCode : 7;
    os_uint8_t Valid : 1;
    os_uint8_t Reserved1;
    os_uint8_t SenseKey : 4;
    os_uint8_t Reserved2 : 4;
    os_uint8_t Information[4];
    os_uint8_t AdditionalSenseLength;
    os_uint8_t Reserved3[4];
    os_uint8_t AdditionalSenseCode;
    os_uint8_t AdditionalSenseCodeQualifier;
    os_uint8_t Reserved4[4];
} request_sense_data_t;

#pragma pack()

#endif
