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
 * @file        mo_ppp.h
 *
 * @brief       module ppp api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-13   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_PPP_H__
#define __MO_PPP_H__

#include "mo_object.h"

#ifdef MOLINK_USING_PPP_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 ***********************************************************************************************************************
 * @struct      mo_ppp_ops_t
 *
 * @brief       molink module ppp ops table
 ***********************************************************************************************************************
 */
typedef struct mo_ppp_ops
{
    os_err_t (*ppp_init)(mo_object_t *module);
    os_err_t (*ppp_dial)(mo_object_t *module);
    os_err_t (*ppp_exit)(mo_object_t *module);
   
} mo_ppp_ops_t;

os_err_t mo_ppp_init(mo_object_t *module);
os_err_t mo_ppp_dial(mo_object_t *module);
os_err_t mo_ppp_exit(mo_object_t *module);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_PPP_OPS */

#endif /* __MO_PPP_H__ */
