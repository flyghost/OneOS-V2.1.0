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
 * @file        n58_ping.h
 *
 * @brief       n58 module link kit ping api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-30   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __N58_PING_H__
#define __N58_PING_H__

#include "mo_ping.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef N58_USING_PING_OPS

os_err_t n58_ping(mo_object_t *self, const char *host, os_uint16_t len, os_uint32_t timeout, struct ping_resp *resp);

#endif /* N58_USING_PING_OPS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __N58_PING_H__ */
