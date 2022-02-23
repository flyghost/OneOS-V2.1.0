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
 * @file        mo_lib.h
 *
 * @brief       module link kit lib api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_LIB_H__
#define __MO_LIB_H__

#include <os_types.h>
#include <os_assert.h>
#include <os_stddef.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void bytes_to_hexstr(const char *source, char *dest, os_size_t source_size);
void hexstr_to_bytes(const char *source, char *dest, os_size_t source_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MO_LIB_H__ */
