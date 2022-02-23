/*
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
 * @file        agent_tiny_data_def.h
 *
 * @brief       Data format of APP message defined on cloud platform
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __AGENT_TINY_DATA_DEF_H_
#define __AGENT_TINY_DATA_DEF_H_
#include <os_mutex.h>
#include "los_typedef_adapter.h"

#define MQ_MAX_MSG 2
/*  app_message_id_t :
    The message ID which defined in the codec plug-in of cloud platform. */
typedef enum
{
    APP_MESSAGE_POST_ATTR_ID              = 0,
    APP_MESSAGE_CMD_CONTROL_LIGHT_ID      = 1,
    APP_MESSAGE_RESPONSE_CONTROL_LIGHT_ID = 2,
    APP_MESSAGE_CMD_CONTROL_MOTOR_ID      = 3,
    APP_MESSAGE_RESPONSE_CONTROL_MOTOR_ID = 4
} app_message_id_t;

/*  app_message_type_t :
    user message type, which definition on huawei-cloud platform object model. */
typedef enum
{
    APP_MESSAGE_NULL,
    APP_MESSAGE_REQUEST,
    APP_MESSAGE_RESPONSE,
    APP_MESSAGE_MAX
} app_message_type_t;

/*  app_data_t:
    User app data */
typedef struct
{
    struct os_mutex   *mutex;
    app_message_type_t type;
    size_t             len;
    void              *buff;
} app_data_t;

/*  app_attr_data_t :
    It is the same as the attribute definition of huawei-cloud platform object model. */
#pragma pack(1)
typedef struct
{
    UINT8  message_id;
    UINT8  temperature;
    UINT8  humidity;
    UINT16 luminance;
} app_attr_data_t;
/*  app_cmd_controlLight_t :
    It is the same as the command definition of huawei-cloud platform object model. */
typedef struct
{
    struct __app_light_cmd__
    {
        UINT8 message_id;
        INT16 mid;
        char  cmd_light[3];
    } cmd;
    struct __app_light_response__
    {
        UINT8 message_id;
        INT16 mid;
        UINT8 err_code;
        UINT8 state_light;
    } resp;
} app_cmd_controlLight_t;
/*  app_cmd_controlMotor_t :
    It is the same as the command definition of huawei-cloud platform object model. */
typedef struct
{
    struct __app_motor_cmd__
    {
        UINT8 message_id;
        INT16 mid;
        char  cmd_motor[3];
    } cmd;
    struct __app_motor_response__
    {
        UINT8 message_id;
        INT16 mid;
        UINT8 err_code;
        UINT8 state_motor;
    } resp;
} app_cmd_controlMotor_t;
#pragma pack()

#endif
