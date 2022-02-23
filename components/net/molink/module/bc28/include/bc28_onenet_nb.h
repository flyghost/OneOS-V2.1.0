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
 * @file        bc28_onenet_nb.h
 *
 * @brief       bc28 module link kit onenet nb api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __BC28_ONENET_H__
#define __BC28_ONENET_H__

#include "mo_onenet_nb.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef BC28_USING_ONENET_NB_OPS

os_err_t bc28_onenetnb_get_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_set_config(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_create(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_delete(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);

os_err_t bc28_onenetnb_addobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_delobj(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_open(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_close(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);

os_err_t bc28_onenetnb_discoverrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_observersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_readrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_writersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_executersp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_parameterrsp(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);

os_err_t bc28_onenetnb_notify(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);
os_err_t bc28_onenetnb_update(mo_object_t *module, os_int32_t timeout, void *resp, const char *format, va_list args);

os_err_t bc28_onenetnb_cb_register(mo_object_t *module, mo_onenet_cb_t user_callbacks);

#endif /* BC28_USING_ONENET_NB_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BC28_ONENET_H__ */
