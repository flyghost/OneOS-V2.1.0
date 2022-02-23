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
 * @file        a7600x_netserv.h
 *
 * @brief       A7600X module link kit netservice api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __A7600X_NETSERV_H__
#define __A7600X_NETSERV_H__

#include "mo_netserv.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef A7600X_USING_NETSERV_OPS

os_err_t a7600x_set_attach(mo_object_t *self, os_uint8_t attach_stat);
os_err_t a7600x_get_attach(mo_object_t *self, os_uint8_t *attach_stat);
os_err_t a7600x_set_reg(mo_object_t *self, os_uint8_t reg_n);
os_err_t a7600x_get_reg(mo_object_t *self, eps_reg_info_t *info);
os_err_t a7600x_set_cgact(mo_object_t *self, os_uint8_t cid, os_uint8_t act_stat);
os_err_t a7600x_get_cgact(mo_object_t *self, os_uint8_t *cid, os_uint8_t *act_stat);
os_err_t a7600x_get_csq(mo_object_t *self, os_uint8_t *rssi, os_uint8_t *ber);
os_err_t a7600x_get_radio(mo_object_t *self, radio_info_t *radio_info);
os_err_t a7600x_get_cell_info(mo_object_t *self, onepos_cell_info_t* onepos_cell_info);

#endif /* A7600X_USING_NETSERV_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __A7600X_NETSERV_H__ */
