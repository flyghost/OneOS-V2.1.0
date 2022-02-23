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
 * @file        commandline_adapter.h
 *
 * @brief       huawei cloud sdk file "commandline.h" adaptation
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __COMMANDLINE_ADAPTER_H__
#define __COMMANDLINE_ADAPTER_H__

#include <stdio.h>
#include "los_config_adapter.h"

void output_buffer(FILE *stream, uint8_t *buffer, int length, int indent);

#endif /* __COMMANDLINE_ADAPTER_H__ */
