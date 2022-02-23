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
 * @file        mo_factory.h
 *
 * @brief       module link kit factory mode api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_FACTORY_H__
#define __MO_FACTORY_H__

#include "mo_object.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef mo_object_t *(*mo_create_fn)(const char *name, void *parser_config);
typedef os_err_t     (*mo_destory_fn)(mo_object_t *self);

mo_object_t *mo_create(const char *name, mo_type_t type, void *parser_config);
os_err_t     mo_destroy(mo_object_t *self, mo_type_t type);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MO_FACTORY_H__ */
