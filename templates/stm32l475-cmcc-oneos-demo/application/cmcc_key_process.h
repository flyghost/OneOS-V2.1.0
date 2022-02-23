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
 * @file        cmcc_key_process.h
 *
 * @brief       This file provides key function declaration.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __CMCC_KEY_PROCESS_H__
#define __CMCC_KEY_PROCESS_H__
#include <os_task.h>
#include <stdint.h>

#define CMCC_BEEP_TASK_STACK_SIZE 512
#define EVENT_BEEP 16

void cmcc_key_init(void);
void cmcc_led_init(void);
void cmcc_buzzer_init(void);

#endif
