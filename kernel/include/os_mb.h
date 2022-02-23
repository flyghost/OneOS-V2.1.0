/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        os_mb.h
 *
 * @brief       Header file for mailbox interface.
 *
 * @revision
 * Date         Author          Notes
 * 2021-01-08   OneOS team      First version.
 ***********************************************************************************************************************
 */

#ifndef __OS_MB_H__
#define __OS_MB_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_MAILBOX

#define OS_MB_WAKE_TYPE_PRIO                0x55
#define OS_MB_WAKE_TYPE_FIFO                0xAA

/**
 ***********************************************************************************************************************
 * @struct      struct os_mb
 *
 * @brief       Define control block of mailbox.
 ***********************************************************************************************************************
 */
struct os_mb
{
    void          *mail_pool;                   /* The address that doesn't do alignment for mail pool */
    void          *mail_pool_align;             /* Aligned address of mail pool. */

    os_list_node_t send_task_list_head;         /* Sender tasks blocked on this mailbox. */
    os_list_node_t recv_task_list_head;         /* Receiver tasks blocked on this mailbox. */
    
    os_list_node_t resource_node;               /* Node in resource list */

    os_uint16_t    capacity;                    /* Max number of mails for this mailbox. */
    os_uint16_t    entry_count;                 /* Numbers of mails into mailbox. */
    os_uint16_t    read_index;                  /* Read index of mail pool. */
    os_uint16_t    write_index;                 /* Write index of mail pool. */

    os_uint8_t     object_inited;               /* Indicates whether object is inited or deinited, value is
                                                   OS_KOBJ_INITED or OS_KOBJ_DEINITED */
    os_uint8_t     object_alloc_type;           /* Indicates whether object is allocated dynamically or statically, 
                                                   value is OS_KOBJ_ALLOC_TYPE_STATIC or OS_KOBJ_ALLOC_TYPE_DYNAMIC */
    os_uint8_t     wake_type;                   /* The type to wake up blocking tasks, value is OS_MB_WAKE_TYPE_PRIO
                                                   or OS_MB_WAKE_TYPE_FIFO */

    char           name[OS_NAME_MAX + 1];       /* Mailbox name. */
};
typedef struct os_mb os_mb_t;


#ifdef OS_USING_SYS_HEAP
extern os_mb_t  *os_mb_create(const char *name, os_size_t max_mails);
extern os_err_t  os_mb_destroy(os_mb_t *mb);
#endif /* OS_USING_SYS_HEAP */                                

/*
 * OS_MB_POOL_DEFINE is used to define mailbox's memory pool and the address is aligned.
 * OS_MB_POOL_ADDR is used to get address of mailbox's memory pool.
 * OS_MB_POOL_SIZE is used to get size of mailbox's memory pool.
 *
 * These macros are used when creating a mailbox with the function os_mb_init().
 */
#define OS_MB_POOL_DEFINE(name, mail_count)     os_ubase_t pool_##name[mail_count]
#define OS_MB_POOL_ADDR(name)                   ((void *)pool_##name)
#define OS_MB_POOL_SIZE(name)                   sizeof(pool_##name)

extern os_err_t    os_mb_init(os_mb_t *mb, const char *name, void *mail_pool, os_size_t mail_pool_size); 
extern os_err_t    os_mb_deinit(os_mb_t *mb);

extern os_err_t    os_mb_send(os_mb_t *mb, os_ubase_t value, os_tick_t timeout);
extern os_err_t    os_mb_recv(os_mb_t *mb, os_ubase_t *value, os_tick_t timeout);

extern os_err_t    os_mb_set_wake_type(os_mb_t *mb, os_uint8_t wake_type);
extern void        os_mb_reset(os_mb_t *mb);
extern os_bool_t   os_mb_is_empty(os_mb_t *mb);
extern os_bool_t   os_mb_is_full(os_mb_t *mb);
extern os_uint16_t os_mb_get_capacity(os_mb_t *mb);
extern os_uint16_t os_mb_get_used_entry_count(os_mb_t *mb);
extern os_uint16_t os_mb_get_unused_entry_count(os_mb_t *mb);

#endif /* OS_USING_MAILBOX */
    
#ifdef __cplusplus
}
#endif

#endif /* __OS_MB_H__ */

