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
 * @file        mo_auto_probe.h
 *
 * @brief       module link kit common api declaration
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-22   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#ifndef __MO_AUTO_PROBE_H__
#define __MO_AUTO_PROBE_H__

#include <os_stddef.h>
#include <oneos_config.h>

#ifdef MOLINK_USING_AUTO_PROBE_OPS

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef os_err_t (*mo_misc_fn_t)(void);

/**
 ***********************************************************************************************************************
 * @struct      mo_auto_probe_info_t
 *
 * @brief       struct that holds molink module auto probe export information
 ***********************************************************************************************************************
 */
typedef struct mo_auto_probe_info
{
    const char *name;               /* export module name */
    const char *at_cmd;             /* export module indentify command */
    const char *identifier;         /* export module identifier */
    const char *device_name;        /* export module device name */
    const int   uart_baud_rate;     /* export module device baud rate */
    const mo_misc_fn_t misc_fn;     /* export module device misc func, for module init preparation */
    const os_init_fn_t init_fn;     /* export module init func */
} mo_auto_probe_info_t;

#define MO_MODULE_EXPORT(name, at_cmd, identifier, device_name, uart_baud_rate, misc_fn, init_fn)  \
    OS_USED static const mo_auto_probe_info_t _mo_module_##name OS_SECTION("MolinkTab") =          \
    {                                                                                              \
        #name,                                                                                     \
        at_cmd,                                                                                    \
        identifier,                                                                                \
        device_name,                                                                               \
        uart_baud_rate,                                                                            \
        misc_fn,                                                                                   \
        init_fn,                                                                                   \
    };

/* export this function in case users want to manually start this session */
extern os_err_t mo_auto_probe(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MOLINK_USING_AUTO_PROBE_OPS */

#endif /* __MO_AUTO_PROBE_H__ */
