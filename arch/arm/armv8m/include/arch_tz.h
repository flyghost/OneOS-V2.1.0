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
 * @file        arch_tz.h
 *
 * @brief       This file provides external declarations of architecture-related functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#ifndef __ARCH_TZ_H__
#define __ARCH_TZ_H__

#ifdef __cplusplus
    extern "C" {
#endif

#ifdef ARCH_ARM_CORTEX_M33_TZ
extern os_err_t    os_arch_tz_context_alloc(os_uint32_t module);
extern os_err_t    os_arch_tz_context_free(void);
#endif

#ifdef __cplusplus
    }
#endif

#endif /* __OS_ARCH_HW_H__ */

