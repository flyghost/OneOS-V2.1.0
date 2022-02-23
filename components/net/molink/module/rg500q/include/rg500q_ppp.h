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
 * @file        rg500q_ppp.h
 *
 * @brief       rg500q module link kit ppp api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __RG500Q_PPP_H__
#define __RG500Q_PPP_H__

#include "mo_ppp.h"
#include "os_mb.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef RG500Q_USING_PPP_OPS

/* rg500q needs more than 1s around +++ input */
#define RG500Q_PPP_EXIT_DELAY_MS (1200)

os_err_t rg500q_ppp_init(mo_object_t *module);
os_err_t rg500q_ppp_dial(mo_object_t *module);
os_err_t rg500q_ppp_exit(mo_object_t *module);
os_err_t rg500q_ppp_startup(mo_object_t *module);
// os_err_t rg500q_ppp_shutdown(mo_object_t *module);

#endif /* RG500Q_USING_PPP_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RG500Q_PPP_H__ */
