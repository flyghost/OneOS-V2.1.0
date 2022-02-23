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
 * @file        mo_common.h
 *
 * @brief       module link kit common api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_COMMON_H__
#define __MO_COMMON_H__

#include "mo_object.h"

#ifdef NET_USING_MOLINK

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

mo_object_t *mo_get_by_name(const char *name);
mo_object_t *mo_get_default(void);
void         mo_set_default(mo_object_t *self);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NET_USING_MOLINK */

#endif /* __MO_COMMON_H__ */
