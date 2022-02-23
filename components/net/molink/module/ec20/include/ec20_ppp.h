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
 * @file        ec20_ppp.h
 *
 * @brief       ec20 module link kit ppp api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __EC20_PPP_H__
#define __EC20_PPP_H__

#include "mo_ppp.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef EC20_USING_PPP_OPS

os_err_t ec20_ppp_init(mo_object_t *module);
os_err_t ec20_ppp_dial(mo_object_t *module);
os_err_t ec20_ppp_exit(mo_object_t *module);

#endif /* EC20_USING_PPP_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __EC20_PPP_H__ */
