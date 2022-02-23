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
 * @file        air723ug_ping.h
 *
 * @brief       air723ug module link kit ping api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-03-25   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __AIR723UG_PING_H__
#define __AIR723UG_PING_H__

#include "mo_ping.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef AIR723UG_USING_PING_OPS

os_err_t air723ug_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp);

#endif /* AIR723UG_USING_PING_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __AIR723UG_PING_H__ */
