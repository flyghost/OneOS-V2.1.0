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
 * @file        cmsis_mp.c
 *
 * @brief       Implementation of CMSIS-RTOS API v2 mempool function.
 *
 * @revision
 * Date         Author          Notes
 * 2021-04-06   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <os_assert.h>
#include <os_errno.h>
#include <os_util.h>
#include <string.h>
#include <arch_interrupt.h>
#include <os_mq.h>

#include "cmsis_internal.h"

#ifdef OS_USING_MEM_POOL
osMemoryPoolId_t osMemoryPoolNew(uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr)
{
    char                name[OS_NAME_MAX];
    void               *mp_addr;
    os_uint32_t         mp_size;
    mempool_cb_t       *mempool_cb;
    static os_uint16_t  memory_pool_number = 1U;
    os_err_t ret;

    if ((0U == block_count) || (0U == block_size))
    {
        return OS_NULL;
    }

    /* OneOS object's name can't be NULL */
    if ((OS_NULL == attr) || (OS_NULL == attr->name))
    {
        os_snprintf(name, sizeof(name), "mp%02d", memory_pool_number++);
    }
    else
    {
        os_snprintf(name, sizeof(name), "%s", attr->name);
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->cb_mem))
    {
        mempool_cb = os_malloc(sizeof(mempool_cb_t));
        if (OS_NULL == mempool_cb)
        {
            return (osMemoryPoolId_t)OS_NULL;
        }
        memset(mempool_cb, 0, sizeof(mempool_cb_t));
        mempool_cb->flags |= SYS_MALLOC_CTRL_BLK;
    }
    else
    {
        if (attr->cb_size >= sizeof(mempool_cb_t))
        {
            mempool_cb = attr->cb_mem;
            mempool_cb->flags = 0;
        }
        else
        {
            return (osMemoryPoolId_t)OS_NULL;
        }
    }

    if ((OS_NULL == attr) || (OS_NULL == attr->mp_mem))
    {
        block_size = OS_ALIGN_UP(block_size, OS_ALIGN_SIZE);
        mp_size    = block_size  * block_count;

        mp_addr = os_malloc(mp_size);
        if (OS_NULL == mp_addr)
        {
            if (mempool_cb->flags & SYS_MALLOC_CTRL_BLK)
            {
                os_free(mempool_cb);
            }

            return (osMemoryPoolId_t)OS_NULL;
        }
        mempool_cb->flags |= SYS_MALLOC_MEM;
        mempool_cb->start_addr = mp_addr;
    }
    else
    {
        mp_addr = (void *)(attr->mp_mem);
        mp_size = attr->mp_size;
    }
    strncpy(&mempool_cb->name[0], name, OS_NAME_MAX);
    mempool_cb->id = IdMemoryPool;

    ret = os_mp_init(&mempool_cb->mp, name, mp_addr, mp_size, block_size);
    if (OS_EOK != ret)
    {
        if (mempool_cb->flags & SYS_MALLOC_MEM)
        {
            os_free(mempool_cb->start_addr);
        }

        if (mempool_cb->flags & SYS_MALLOC_CTRL_BLK)
        {
             os_free(mempool_cb);
        }

        return (osMemoryPoolId_t)OS_NULL;
    }

    return (osMemoryPoolId_t)mempool_cb;
}

const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return OS_NULL;
    }

    return mempool_cb->name;
}
void *osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return OS_NULL;
    }

    return os_mp_alloc(&mempool_cb->mp, timeout);
}

osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void *block)
{
    mempool_cb_t *mempool_cb;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return osErrorParameter;
    }

    os_mp_free(&mempool_cb->mp, block);

    return osOK;
}

uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;
    os_mpinfo_t os_mpinfo;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return 0U;
    }
    os_mp_info(&mempool_cb->mp, &os_mpinfo);

    return os_mpinfo.blk_total_num;
}

uint32_t osMemoryPoolGetBlockSize(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;
    os_mpinfo_t os_mpinfo;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return 0U;
    }
    os_mp_info(&mempool_cb->mp, &os_mpinfo);

    return os_mpinfo.blk_size;
}

uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id)
{
    os_size_t     used_blocks;
    mempool_cb_t *mempool_cb;
    os_mpinfo_t os_mpinfo;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return 0U;
    }
    os_mp_info(&mempool_cb->mp, &os_mpinfo);
    used_blocks = os_mpinfo.blk_total_num - os_mpinfo.blk_free_num;

    return (uint32_t)used_blocks;
}

uint32_t osMemoryPoolGetSpace(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;
    os_mpinfo_t os_mpinfo;

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return 0U;
    }
    os_mp_info(&mempool_cb->mp, &os_mpinfo);
    return os_mpinfo.blk_free_num;
}

osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id)
{
    mempool_cb_t *mempool_cb;
    os_err_t ret;
    osStatus_t status;

    if (OS_TRUE == os_is_irq_active())
    {
        return  osErrorISR;
    }

    mempool_cb = (mempool_cb_t *)mp_id;

    if ((OS_NULL == mempool_cb) || (IdMemoryPool != mempool_cb->id))
    {
        return osErrorParameter;
    }
    mempool_cb->id = IdInvalid;
    ret = os_mp_deinit(&mempool_cb->mp);
    if (OS_EOK == ret)
    {
        status = osOK;
    }
    else
    {
        status = osErrorResource;
    }

    if (mempool_cb->flags & SYS_MALLOC_MEM)
    {
        os_free(mempool_cb->start_addr);
    }

    if (mempool_cb->flags & SYS_MALLOC_CTRL_BLK)
    {
        os_free(mempool_cb);
    }

    return status;
}

#endif /* OS_USING_MEM_POOL */

