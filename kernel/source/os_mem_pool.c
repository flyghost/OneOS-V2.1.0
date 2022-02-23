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
 * @file        os_mem_pool.c
 *
 * @brief       This file implements the memory pool functions.
 *
 * @revision
 * Date         Author          Notes
 * 2020-11-18   OneOS team      First Version
 ***********************************************************************************************************************
 */

#include <string.h>
#include <os_memory.h>
#include <os_errno.h>
#include <os_clock.h>
#include <os_spinlock.h>
#include "os_kernel_internal.h"

#define MP_TAG          "MP"

#ifdef OS_USING_MP_CHECK_TAG
struct mp_check_tag
{
    os_uint32_t magic_number:31;
    os_uint32_t used_flag:1;
};

#define MP_MAGIC_NUMBER         0x504F4F4C
#define MP_TAG_SIZE             sizeof(struct mp_check_tag)
#define MP_SIZE_WITH_TAG(size)  (size + MP_TAG_SIZE)
#define MP_PTR_WITHOUT_TAG(mem) ((void *)((char*)mem + MP_TAG_SIZE))
#define MP_PTR_WITH_TAG(mem)    ((void *)((char*)mem - MP_TAG_SIZE))
#define MP_SET_BLK_TAG(blk)  do {                                   \
    ((struct mp_check_tag *)mem)->magic_number = MP_MAGIC_NUMBER;   \
    ((struct mp_check_tag *)mem)->used_flag = OS_TRUE;              \
} while(0)
#define MP_BLK_TAG_OK(blk) ((((struct mp_check_tag *)mem)->magic_number == MP_MAGIC_NUMBER) && (((struct mp_check_tag *)mem)->used_flag == OS_TRUE))
#define MP_GET_ALIGN_BLK_SIZE(blk_size)     OS_ALIGN_UP(MP_SIZE_WITH_TAG(blk_size), OS_ALIGN_SIZE)
#else
#define MP_TAG_SIZE             0
#define MP_GET_ALIGN_BLK_SIZE(blk_size)     OS_ALIGN_UP(blk_size, OS_ALIGN_SIZE)
#endif

struct blk_head
{
    struct blk_head *next;
};

static os_list_node_t gs_os_mp_resource_list_head = OS_LIST_INIT(gs_os_mp_resource_list_head);
static OS_DEFINE_SPINLOCK(gs_os_mp_resource_list_lock);

static os_err_t _k_mp_add_resourcelist(os_mp_t *mp, os_uint8_t object_alloc_type)
{
    os_list_node_t *pos;
    os_mp_t        *item_mp;
    os_err_t        ret;

    ret = OS_EOK;

    if (OS_KOBJ_ALLOC_TYPE_STATIC == object_alloc_type)
    {
        os_spin_lock(&gs_os_mp_resource_list_lock);
        os_list_for_each(pos, &gs_os_mp_resource_list_head)
        {
            item_mp = os_list_entry(pos, os_mp_t, resource_node);
            if (item_mp == mp)
            {
                os_spin_unlock(&gs_os_mp_resource_list_lock);

                OS_KERN_LOG(KERN_ERROR, MP_TAG, "The mp(addr: %p) already exist", item_mp);
                ret = OS_EINVAL;
                break;
            }
        }

        if (OS_EOK == ret)
        {
            os_list_add_tail(&gs_os_mp_resource_list_head, &mp->resource_node);
            os_spin_unlock(&gs_os_mp_resource_list_lock);
        }
    }
    else
    {
        os_spin_lock(&gs_os_mp_resource_list_lock);
        os_list_add_tail(&gs_os_mp_resource_list_head, &mp->resource_node);
        os_spin_unlock(&gs_os_mp_resource_list_lock);
    }

    return ret;
}

static void _k_mp_init_free_list(os_mp_t *mp)
{
    struct blk_head *blk;
    os_uint32_t      i;

    mp->free_list = mp->start_addr;

    blk = (struct blk_head *)mp->start_addr;
    for (i = 0; i < (mp->blk_total_num - 1); i++)
    {
        blk->next = (struct blk_head *)((char *)blk + mp->blk_size);
        blk = blk->next;
    }
    blk->next = OS_NULL;
}

static void _k_mp_init(os_mp_t        *mp,
                           const char *name,
                           void       *start,
                           os_size_t   size,
                           os_size_t   blk_size,
                           os_uint16_t object_alloc_type)
{
    if (OS_NULL != name)
    {
        strncpy(&mp->name[0], name, OS_NAME_MAX);
        mp->name[OS_NAME_MAX] = '\0';
    }
    else
    {
        mp->name[0] = '\0';
    }

    mp->start_addr    = start;
    mp->size          = size;
    mp->blk_size      = blk_size;
    mp->blk_total_num = (size / blk_size);
    mp->blk_free_num  = mp->blk_total_num;

    _k_mp_init_free_list(mp);
    os_list_init(&mp->task_list_head);

    mp->object_alloc_type = object_alloc_type;
    mp->object_inited     = OS_KOBJ_INITED;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will initialize a memory pool object.
 *
 * @param[in]       mp              The pointer to memory pool object.
 * @param[in]       name            The name of memory pool.
 * @param[in]       start           The start address of memory pool.
 * @param[in]       size            The total size of memory pool.
 * @param[in]       blk_size        The size for each block.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mp_init(os_mp_t *mp, const char *name, void  *start, os_size_t size, os_size_t blk_size)
{
    void    *start_addr;
    os_err_t ret;

    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(mp);
    OS_ASSERT(start);
    OS_ASSERT(size >= OS_ALIGN_SIZE);
    OS_ASSERT(blk_size > 0);

    ret = OS_EOK;

    start_addr = (void *)OS_ALIGN_UP((os_ubase_t)start, OS_ALIGN_SIZE);
    size       = OS_ALIGN_DOWN((os_ubase_t)start + size - (os_ubase_t)start_addr, OS_ALIGN_SIZE);
    blk_size   = MP_GET_ALIGN_BLK_SIZE(blk_size);
    if (0 != (size / blk_size))
    {
        ret = _k_mp_add_resourcelist(mp, OS_KOBJ_ALLOC_TYPE_STATIC);

        if (OS_EOK == ret)
        {
            _k_mp_init(mp, name, start_addr, size, blk_size, OS_KOBJ_ALLOC_TYPE_STATIC);
        }
    }
    else
    {
        ret =  OS_EINVAL;
    }

    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will deinitialize a memory pool object.
 *
 * @param[in]       mp              The pointer to memory pool object.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mp_deinit(os_mp_t *mp)
{
    OS_KERNEL_INIT();

    OS_ASSERT(mp);
    OS_ASSERT(OS_KOBJ_INITED == mp->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_STATIC == mp->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());

    os_spin_lock(&gs_os_mp_resource_list_lock);
    os_list_del(&mp->resource_node);
    os_spin_unlock(&gs_os_mp_resource_list_lock);

    OS_KERNEL_ENTER();
    mp->object_inited = OS_KOBJ_DEINITED;
    k_cancle_all_blocked_task(&mp->task_list_head);
    OS_KERNEL_EXIT_SCHED();

    return OS_EOK;
}

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           This function will create a memory pool object from system memory heap.
 *
 * @param[in]       name            The name of memory pool.
 * @param[in]       blk_count       The count of blocks in memory pool.
 * @param[in]       blk_size        The size for each block.
 *
 * @return          The pointer to the created memory pool object.
 * @retval          pointer         If operation successful.
 * @retval          OS_NULL         Error occurred.
 ***********************************************************************************************************************
 */
os_mp_t *os_mp_create(const char *name, os_size_t blk_count, os_size_t blk_size)
{
    os_mp_t   *mp;
    os_size_t  size;
    void      *start_addr;
    os_err_t   ret;

    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());
    OS_ASSERT((blk_count > 1) && (blk_size > 0));

    ret = OS_EOK;

    mp = (os_mp_t *)OS_KERNEL_MALLOC(sizeof(os_mp_t));
    if (mp)
    {
        blk_size = MP_GET_ALIGN_BLK_SIZE(blk_size);
        size = blk_size * blk_count;
        start_addr = OS_KERNEL_MALLOC(size);

        if (start_addr)
        {
            ret = _k_mp_add_resourcelist(mp, OS_KOBJ_ALLOC_TYPE_DYNAMIC);
            if (OS_EOK == ret)
            {
                _k_mp_init(mp, name, start_addr, size, blk_size, OS_KOBJ_ALLOC_TYPE_DYNAMIC);
            }
            else
            {
                OS_KERNEL_FREE(start_addr);
                OS_KERNEL_FREE(mp);
                mp = OS_NULL;
            }
        }
        else
        {
            OS_KERNEL_FREE(mp);
            mp = OS_NULL;
        }
    }

    return mp;
}

/**
 ***********************************************************************************************************************
 * @brief           Destory an memory pool object created from system memory heap.
 *
 * @param[in]       mp              The memory pool to destroy.
 *
 * @return          The operation result.
 * @retval          OS_EOK          If the operation successful.
 * @retval          else            Error code.
 ***********************************************************************************************************************
 */
os_err_t os_mp_destroy(os_mp_t *mp)
{
    OS_KERNEL_INIT();

    OS_ASSERT(OS_NULL != mp);
    OS_ASSERT(OS_KOBJ_INITED == mp->object_inited);
    OS_ASSERT(OS_KOBJ_ALLOC_TYPE_DYNAMIC == mp->object_alloc_type);
    OS_ASSERT(OS_FALSE == os_is_irq_active());
    OS_ASSERT(OS_FALSE == os_is_irq_disabled());
    OS_ASSERT(OS_FALSE == os_is_schedule_locked());

    os_spin_lock(&gs_os_mp_resource_list_lock);
    os_list_del(&mp->resource_node);
    os_spin_unlock(&gs_os_mp_resource_list_lock);

    OS_KERNEL_ENTER();
    mp->object_inited = OS_KOBJ_DEINITED;
    k_cancle_all_blocked_task(&mp->task_list_head);
    OS_KERNEL_EXIT_SCHED();

    OS_KERNEL_FREE(mp->start_addr);
    OS_KERNEL_FREE(mp);

    return OS_EOK;
}
#endif

/**
 ***********************************************************************************************************************
 * @brief           This function allocates a free block from memory pool.
 *
 * @param[in]       mp              The pointer to memory pool object.
 * @param[in]       timeout         Waiting time (in clock ticks).
 *
 * @return          The pointer to allocated memory or OS_NULL if no free memory was found.
 ***********************************************************************************************************************
 */
void *os_mp_alloc(os_mp_t *mp, os_tick_t timeout)
{
    void      *mem;
    os_task_t *current_task;
    os_tick_t  tick_before;
    os_tick_t  tick_elapse;
    os_err_t   ret;

    OS_KERNEL_INIT();

    OS_ASSERT(mp);
    OS_ASSERT(OS_KOBJ_INITED == mp->object_inited);
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_active()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_irq_disabled()));
    OS_ASSERT((OS_NO_WAIT == timeout) || (OS_FALSE == os_is_schedule_locked()));
    OS_ASSERT((timeout < (OS_TICK_MAX / 2)) || (OS_WAIT_FOREVER == timeout));

    mem = OS_NULL;
    ret = OS_EOK;

    OS_KERNEL_ENTER();
    current_task = k_task_self();

    while(!mp->free_list)
    {
        if (OS_NO_WAIT == timeout)
        {
            OS_KERNEL_EXIT();
            ret = OS_ENOMEM;
            break;
        }

        tick_before = os_tick_get();
        k_block_task(&mp->task_list_head, current_task, timeout, OS_TRUE);
        OS_KERNEL_EXIT_SCHED();

        if (OS_EOK != current_task->switch_retval)
        {
            ret = OS_ERROR;
            break;
        }

        OS_KERNEL_ENTER();
        if (OS_WAIT_FOREVER != timeout)
        {
            tick_elapse = os_tick_get() - tick_before;
            timeout = (tick_elapse >= timeout) ? OS_NO_WAIT : (timeout - tick_elapse);
        }
    }

    if (OS_EOK == ret)
    {
        mem = mp->free_list;
        mp->free_list = ((struct blk_head *)mem)->next;
        mp->blk_free_num--;

        OS_KERNEL_EXIT();

#ifdef OS_USING_MP_CHECK_TAG
        MP_SET_BLK_TAG(mem);
        mem = MP_PTR_WITHOUT_TAG(mem);
#endif
    }
    return mem;
}

/**
 ***********************************************************************************************************************
 * @brief           This function frees a memory block allocated by os_mp_alloc().
 *
 * @param[in]       mp              The pointer to memory pool object.
 * @param[in]       mem             The pointer to memory block.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void os_mp_free(os_mp_t *mp, void *mem)
{
    os_task_t *task;

    OS_KERNEL_INIT();

    OS_ASSERT(mp);
    OS_ASSERT(OS_KOBJ_INITED == mp->object_inited);
    OS_ASSERT(mem);
    OS_ASSERT((mem >= mp->start_addr) && ((char *)mem < ((char *)mp->start_addr + mp->size)));

#ifdef OS_USING_MP_CHECK_TAG
    mem = MP_PTR_WITH_TAG(mem);
    OS_ASSERT(MP_BLK_TAG_OK(mem));
#endif
    OS_ASSERT((((char *)mem - (char *)mp->start_addr) % mp->blk_size) == 0);

    OS_KERNEL_ENTER();

    ((struct blk_head *)mem)->next = mp->free_list;
    mp->free_list = mem;
    mp->blk_free_num++;

    if (!os_list_empty(&mp->task_list_head))
    {
        task = os_list_first_entry(&mp->task_list_head, os_task_t, task_node);
        k_unblock_task(task);
        if (task->state & OS_TASK_STATE_READY)
        {
            OS_KERNEL_EXIT_SCHED();
        }
        else
        {
            OS_KERNEL_EXIT();
        }
    }
    else
    {
        OS_KERNEL_EXIT();
    }
}

/**
 ***********************************************************************************************************************
 * @brief           This function get a memory pool information.
 *
 * @param[in]       mp              The pointer to memory pool object.
 * @param[in,out]   info            The pointer to save the info.
 *
 * @return          None
 ***********************************************************************************************************************
 */
void os_mp_info(os_mp_t *mp, os_mpinfo_t *info)
{
    OS_KERNEL_INIT();

    OS_ASSERT(mp);
    OS_ASSERT(info);

    OS_KERNEL_ENTER();
    info->blk_size = mp->blk_size - MP_TAG_SIZE;
    info->blk_total_num = mp->blk_total_num;
    info->blk_free_num = mp->blk_free_num;
    OS_KERNEL_EXIT();
}

#if defined(OS_USING_SHELL) && defined(OS_USING_SYS_HEAP)
#include <shell.h>

#define SH_SHOW_TASK_CNT_MAX    5

typedef struct
{
    os_mp_t     *mp;
    os_uint32_t  blk_total_num;
    os_uint32_t  blk_free_num;
    os_uint32_t  blk_size;

    os_uint16_t  block_task_count;
    os_task_t   *block_task[SH_SHOW_TASK_CNT_MAX];
}sh_mp_info_t;

os_err_t sh_show_mempool_info(os_int32_t argc, char **argv)
{
    os_mp_t       *iter_mp;
    sh_mp_info_t  *mp_info;
    os_uint16_t    mp_count;
    os_uint16_t    mp_index;
    char           name[OS_NAME_MAX + 1];
    os_uint16_t    len;
    os_int32_t     ret;

    OS_KERNEL_INIT();

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("%-*s %-10s %-10s %-10s %-10s\r\n", OS_NAME_MAX, "MemPool", "TotalCount", "FreeCount", "PerSize", "Block Task");

    len = OS_NAME_MAX;
    while (len--)
    {
        os_kprintf("-");
    }
    os_kprintf(" ---------- ---------- ---------- ----------\r\n");

    os_spin_lock(&gs_os_mp_resource_list_lock);
    mp_count = os_list_len(&gs_os_mp_resource_list_head);
    os_spin_unlock(&gs_os_mp_resource_list_lock);

    mp_info = OS_NULL;
    ret     = OS_EOK;

    if (0U != mp_count)
    {
        mp_info = (sh_mp_info_t *)os_malloc(sizeof(sh_mp_info_t) * mp_count);
        if (OS_NULL == mp_info)
        {
            ret = OS_ENOMEM;
        }
    }

    if ((OS_EOK == ret) && (OS_NULL != mp_info))
    {
        memset((void *)mp_info, 0, sizeof(sh_mp_info_t) * mp_count);

        os_spin_lock(&gs_os_mp_resource_list_lock);

        mp_index = 0;
        os_list_for_each_entry(iter_mp, &gs_os_mp_resource_list_head, os_mp_t, resource_node)
        {
            OS_KERNEL_ENTER();

            mp_info[mp_index].mp   = iter_mp;
            mp_info[mp_index].blk_total_num = iter_mp->blk_total_num;
            mp_info[mp_index].blk_free_num  = iter_mp->blk_free_num;
            mp_info[mp_index].blk_size  = iter_mp->blk_size;

            mp_info[mp_index].block_task_count = os_list_len(&iter_mp->task_list_head);
            k_get_blocked_task(&iter_mp->task_list_head, mp_info[mp_index].block_task, SH_SHOW_TASK_CNT_MAX);

            OS_KERNEL_EXIT();

            mp_index++;
            if (mp_index == mp_count)
            {
                break;
            }
        }

        os_spin_unlock(&gs_os_mp_resource_list_lock);

        if (mp_index < mp_count)
        {
            mp_count = mp_index;
        }

        for (mp_index = 0U; mp_index < mp_count; mp_index++)
        {
            if (OS_KOBJ_INITED != mp_info[mp_index].mp->object_inited)
            {
                continue;
            }

            strncpy(name, mp_info[mp_index].mp->name, OS_NAME_MAX);
            name[OS_NAME_MAX] = '\0';

            os_kprintf("%-*s %-10u %-10u %-10u %-10u:",
                       OS_NAME_MAX,
                       (name[0] != '\0') ? name : "-",
                       mp_info[mp_index].blk_total_num,
                       mp_info[mp_index].blk_free_num,
                       mp_info[mp_index].blk_size - MP_TAG_SIZE,
                       mp_info[mp_index].block_task_count);

            k_show_blocked_task(mp_info[mp_index].block_task,
                                SH_SHOW_TASK_CNT_MAX,
                                mp_info[mp_index].block_task_count);
            os_kprintf("\r\n");
        }

        os_free(mp_info);
        mp_info = OS_NULL;
    }

    return ret;
}
SH_CMD_EXPORT(show_mempool, sh_show_mempool_info, "Show mempool information");
#endif

