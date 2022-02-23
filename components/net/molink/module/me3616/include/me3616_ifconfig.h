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
 * @file        me3616_ifconfig.h
 *
 * @brief       me3616 module link kit ifconfig api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-12-1   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __ME3616_IFCONFIG_H__
#define __ME3616_IFCONFIG_H__

#include "mo_ifconfig.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef ME3616_USING_IFCONFIG_OPS

os_err_t me3616_ifconfig(mo_object_t *self);
os_err_t me3616_get_ipaddr(mo_object_t *self, char ip[]);

#endif /* ME3616_USING_IFCONFIG_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ME3616_IFCONFIG_H__ */


