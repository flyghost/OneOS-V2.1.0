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
 * @file        hwcrypto.c
 *
 * @brief       This file provides interfaces for hardware crypto.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <hwcrypto/hwcrypto.h>
#include <os_errno.h>
#include <os_memory.h>
#include <string.h>
#include <os_assert.h>
#include <driver.h>

/**
 ***********************************************************************************************************************
 * @brief           Set crypto context type.
 *
 * @attention       Direct calls are not recommended.
 *
 * @param[in,out]   ctx             Crypto context.
 * @param[in]       type            Context Type.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 * @retval          OS_EINVAL       Fail. ctx is NULL.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_set_type(struct os_hwcrypto_ctx *ctx, hwcrypto_type type)
{
    if (ctx)
    {
        /* Is it the same category? */
        if ((ctx->type & HWCRYPTO_MAIN_TYPE_MASK) == (type & HWCRYPTO_MAIN_TYPE_MASK))
        {
            ctx->type = type;
            return OS_EOK;
        }
        /* Context is empty type */
        else if (ctx->type == HWCRYPTO_TYPE_NULL)
        {
            ctx->type = type;
            return OS_EOK;
        }
        else
        {
            return OS_ERROR;
        }
    }
    return OS_EINVAL;
}

/**
***********************************************************************************************************************
* @brief           Reset crypto context.
*
* @attention       Direct calls are not recommended.
*
* @param[in,out]   ctx             Pointer to crypto context.
*
* @return          None.
***********************************************************************************************************************
*/
void os_hwcrypto_ctx_reset(struct os_hwcrypto_ctx *ctx)
{
    if (ctx && ctx->device->ops->reset)
    {
        ctx->device->ops->reset(ctx);
    }
}

/**
 ***********************************************************************************************************************
 * @brief           Initialise crypto context.
 *
 * @attention       Direct calls are not recommended.
 *
 * @param[in,out]   ctx             Crypto context.
 * @param[in]       device          Crypto devce.
 * @param[in]       type            Context Type.
 *
 * @return
 * @retval          OS_EOK          Succeed.
 * @retval          OS_ERROR        Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_ctx_init(struct os_hwcrypto_ctx *ctx, struct os_hwcrypto_device *device, hwcrypto_type type)
{
    os_err_t err;

    /* Setting context type */
    os_hwcrypto_set_type(ctx, type);
    ctx->device = device;
    /* Create hardware context */
    err = ctx->device->ops->create(ctx);
    if (err != OS_EOK)
    {
        return err;
    }
    return OS_EOK;
}

/**
 ***********************************************************************************************************************
 * @brief           Create crypto context.
 *
 * @param[in]       device          Crypto devce.
 * @param[in]       type            Context Type.
 * @param[in]       obj_size        Size of context.
 *
 * @return
 * @retval          OS_NULL         Fail.
 * @retval          Others          Pointer to crypto context.
 ***********************************************************************************************************************
 */
struct os_hwcrypto_ctx *
os_hwcrypto_ctx_create(struct os_hwcrypto_device *device, hwcrypto_type type, os_uint32_t obj_size)
{
    struct os_hwcrypto_ctx *ctx;
    os_err_t                err;

    /* Parameter checking */
    if (device == OS_NULL || obj_size < sizeof(struct os_hwcrypto_ctx))
    {
        return OS_NULL;
    }
    ctx = os_calloc(1, obj_size);
    if (ctx == OS_NULL)
    {
        return ctx;
    }
    memset(ctx, 0, obj_size);
    /* Init context */
    err = os_hwcrypto_ctx_init(ctx, device, type);
    if (err != OS_EOK)
    {
        os_free(ctx);
        ctx = OS_NULL;
    }
    return ctx;
}

/**
 ***********************************************************************************************************************
 * @brief           Destroy crypto context.
 *
 * @param[in]       ctx          Crypto context.
 *
 * @return          None.
 ***********************************************************************************************************************
 */
void os_hwcrypto_ctx_destroy(struct os_hwcrypto_ctx *ctx)
{
    if (ctx == OS_NULL)
    {
        return;
    }
    /* Destroy hardware context */
    if (ctx->device->ops->destroy)
    {
        ctx->device->ops->destroy(ctx);
    }
    /* Free the resources */
    os_free(ctx);
}

/**
 ***********************************************************************************************************************
 * @brief           Copy crypto context.
 *
 * @param[in,out]   des            Destination crypto context(copy to).
 * @param[in]       src            Source crypto context(copy from).
 *
 * @return
 * @retval          OS_EOK         Succeed.
 * @retval          Others         Fail.
 ***********************************************************************************************************************
 */
os_err_t os_hwcrypto_ctx_cpy(struct os_hwcrypto_ctx *des, const struct os_hwcrypto_ctx *src)
{
    if (des == OS_NULL || src == OS_NULL)
    {
        return OS_EINVAL;
    }

    /* The equipment is different or of different types and cannot be copied */
    if (des->device != src->device || (des->type & HWCRYPTO_MAIN_TYPE_MASK) != (src->type & HWCRYPTO_MAIN_TYPE_MASK))
    {
        return OS_EINVAL;
    }
    des->type = src->type;
    /* Calling Hardware Context Copy Function */
    return src->device->ops->copy(des, src);
}

/**
 ***********************************************************************************************************************
 * @brief           Get ID of crypto device.
 *
 * @param[in]       device          Crypto device.
 *
 * @return
 * @retval          0               Fail.
 * @retval          Others          ID of crypto device.
 ***********************************************************************************************************************
 */
os_uint64_t os_hwcrypto_id(struct os_hwcrypto_device *device)
{
    if (device)
    {
        return device->id;
    }
    return 0;
}

/**
***********************************************************************************************************************
* @brief           Register crypto device.
*
* @param[in]       device          Crypto device.
* @param[in]       name            Device name.
*
* @return
* @retval          OS_EOK          Succeed.
* @retval          Others          Fail.
***********************************************************************************************************************
*/
os_err_t os_hwcrypto_register(struct os_hwcrypto_device *device, const char *name)
{
    OS_ASSERT(device != OS_NULL);
    OS_ASSERT(name != OS_NULL);
    OS_ASSERT(device->ops != OS_NULL);
    OS_ASSERT(device->ops->create != OS_NULL);
    OS_ASSERT(device->ops->destroy != OS_NULL);
    OS_ASSERT(device->ops->copy != OS_NULL);
    OS_ASSERT(device->ops->reset != OS_NULL);

    device->parent.ops  = OS_NULL;
    device->parent.type = OS_DEVICE_TYPE_MISCELLANEOUS;

    return os_device_register(&device->parent, name);
}

