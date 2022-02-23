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
 * @file        e7025_general.h
 *
 * @brief       e7025 module link kit general api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-05   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __E7025_GENERAL_H__
#define __E7025_GENERAL_H__

#include "mo_general.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef E7025_USING_GENERAL_OPS

os_err_t e7025_at_test(mo_object_t *self);
os_err_t e7025_get_imei(mo_object_t *self, char *value, os_size_t len);
os_err_t e7025_get_imsi(mo_object_t *self, char *value, os_size_t len);
os_err_t e7025_get_iccid(mo_object_t *self, char *value, os_size_t len);
os_err_t e7025_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl);
os_err_t e7025_set_cfun(mo_object_t *self, os_uint8_t fun_lvl);

#endif /* E7025_USING_GENERAL_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __E7025_GENERAL_H__ */
