/**
 ***********************************************************************************************************************
 * Copyright (c) <2019-2020> <China Mobile Communications Group Co.,Ltd> All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * @file        cJSON_port.c
 *
 * @brief       This file is used to port cJSON
 *
 * Change Logs:
 * Date         Author          Notes
 * 2020-04-21   OneOS Group     First Version
 ***********************************************************************************************************************
 */

#ifdef PKG_USING_CJSON

#include <os_stddef.h>
#include <os_memory.h>
#include <os_errno.h>

#include "cJSON.h"

int cJSON_hook_init(void)
{
    cJSON_Hooks cJSON_hook;

    cJSON_hook.malloc_fn = (void *(*)(size_t sz))os_malloc;
    cJSON_hook.free_fn = os_free;

    cJSON_InitHooks(&cJSON_hook);

    return OS_EOK;
}
OS_CMPOENT_INIT(cJSON_hook_init);

#endif /* PKG_USING_CJSON */
