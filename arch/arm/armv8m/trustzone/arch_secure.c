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
 * @file        arch_secure.c
 *
 * @brief       This file provides trustzone functions related to the ARMv8-M architecture.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-12   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_stddef.h>
#include <os_types.h>
#include <os_util.h>
#include <os_errno.h>

#define ARCH_TZ_OK    1
#define ARCH_TZ_FAIL  0

#define ARCH_TZ_CONTEXT_INIT_ID    0x0001
#define ARCH_TZ_CONTEXT_ALLOC_ID   0x0002
#define ARCH_TZ_CONTEXT_FREE_ID    0x0003

/* cmsis RTOS Thread Context Management for Armv8-M TrustZone */
extern os_uint32_t TZ_InitContextSystem_S(void);
extern os_uint32_t TZ_AllocModuleContext_S (os_uint32_t module);
extern os_uint32_t TZ_FreeModuleContext_S(os_uint32_t mem_id);
extern os_uint32_t TZ_LoadContext_S(os_uint32_t mem_id);
extern os_uint32_t TZ_StoreContext_S(os_uint32_t mem_id);

extern os_uint32_t arch_trustzone_call(os_uint32_t id, os_uint32_t arg0, os_uint32_t arg1, os_uint32_t arg2);

volatile os_uint32_t _g_arch_tz_context_memory_id = 0;

/**
 ***********************************************************************************************************************
 * @brief           Initialize the trustzone context memory system.
 *
 * @param           None
 *
 * @return          OS_EOK          Initialize successfully.
 *                  OS_ERROR        Initialize failed.
 ***********************************************************************************************************************
 */
os_err_t _arch_tz_init(void)
{
    static os_bool_t init_flag = OS_FALSE;
    os_uint32_t ret = OS_EOK;

    if (OS_TRUE == init_flag)
    {
        return ret;
    }

    if (ARCH_TZ_OK != arch_trustzone_call(ARCH_TZ_CONTEXT_INIT_ID, 0, 0, 0))
    {
        ret = OS_ERROR;
    }

    init_flag = OS_TRUE;
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Store secure context.
 *
 * @param[in]       mem_id          TrustZone memory slot id
 *
 * @return          OS_EOK          Store successfully.
 *                  OS_ERROR        Store failed.
 ***********************************************************************************************************************
 */
os_err_t _arch_tz_context_store(os_uint32_t mem_id)
{
    os_uint32_t ret = OS_EOK;

    if (ARCH_TZ_OK != TZ_StoreContext_S(mem_id))
    {
        ret = OS_ERROR;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Load secure context.
 *
 * @param[in]       mem_id          TrustZone memory slot id
 *
 * @return          OS_EOK          Load successfully.
 *                  OS_ERROR        Load failed.
 ***********************************************************************************************************************
 */
os_err_t _arch_tz_context_load(os_uint32_t mem_id)
{
    os_uint32_t ret = OS_EOK;

    if (ARCH_TZ_OK != TZ_LoadContext_S(mem_id))
    {
        ret = OS_ERROR;
    }

    return ret;
}

os_uint32_t _arch_tz_svc_handle(os_uint32_t tz_call_id, os_uint32_t arg0, os_uint32_t arg1, os_uint32_t arg2)
{
    os_uint32_t ret = ARCH_TZ_FAIL;

    switch (tz_call_id)
    {
    case ARCH_TZ_CONTEXT_INIT_ID:
        ret = TZ_InitContextSystem_S();
        break;

    case ARCH_TZ_CONTEXT_ALLOC_ID:
        _g_arch_tz_context_memory_id = TZ_AllocModuleContext_S(arg0);
        if (0 == _g_arch_tz_context_memory_id)
        {
            os_kprintf("arch trustzone alloc stack fail\r\n");
            ret = ARCH_TZ_FAIL;
        }
        else
        {
            ret = TZ_LoadContext_S(_g_arch_tz_context_memory_id);
        }
        break;

    case ARCH_TZ_CONTEXT_FREE_ID:
        ret = TZ_FreeModuleContext_S(_g_arch_tz_context_memory_id);
        _g_arch_tz_context_memory_id = 0;
        break;

    default:
        os_kprintf("arch trustzone call unsupport:%d\r\n", tz_call_id);
        ret = ARCH_TZ_FAIL;
        break;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Allocate context memory for calling secure software modules in TrustZone.
 *
 * @param[in]       module         identifies software modules called from non-secure mode
 *
 * @return          OS_EOK          Allocate successfully.
 *                  OS_ERROR        Allocate failed.
 ***********************************************************************************************************************
 */
os_err_t os_arch_tz_context_alloc(os_uint32_t module)
{
    os_uint32_t ret = OS_EOK;

    if (OS_EOK != _arch_tz_init())
    {
        ret = OS_ERROR;
    }
		
    if (ARCH_TZ_OK != arch_trustzone_call(ARCH_TZ_CONTEXT_ALLOC_ID, module, 0, 0))
    {
        ret = OS_ERROR;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Free context memory that was previously allocated.
 *
 * @param           None
 *
 * @return          OS_EOK          Free successfully.
 *                  OS_ERROR        Free failed.
 ***********************************************************************************************************************
 */
os_err_t os_arch_tz_context_free(void)
{
    os_uint32_t ret = OS_EOK;
    if (ARCH_TZ_OK != arch_trustzone_call(ARCH_TZ_CONTEXT_FREE_ID, 0, 0, 0))
    {
        ret = OS_ERROR;
    }

    return ret;
}