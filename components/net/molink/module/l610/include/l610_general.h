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
 * @file        l610_general.h
 *
 * @brief       l610 module link kit general api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-12   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __L610_GENERAL_H__
#define __L610_GENERAL_H__

#include "mo_general.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef L610_USING_GENERAL_OPS

os_err_t l610_at_test(mo_object_t *self);
os_err_t l610_get_imei(mo_object_t *self, char *value, os_size_t len);
os_err_t l610_get_imsi(mo_object_t *self, char *value, os_size_t len);
os_err_t l610_get_iccid(mo_object_t *self, char *value, os_size_t len);
os_err_t l610_get_cfun(mo_object_t *self, os_uint8_t *fun_lvl);
os_err_t l610_set_cfun(mo_object_t *self, os_uint8_t fun_lvl);
os_err_t l610_get_firmware_version(mo_object_t *self, mo_firmware_version_t *version);

#endif /* L610_USING_GENERAL_OPS */

#ifdef __cplusplus
}
#endif

#endif /* __L610_GENERAL_H__ */
