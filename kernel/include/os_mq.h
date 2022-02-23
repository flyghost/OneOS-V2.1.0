/**
 ***********************************************************************************************************************
 * Copyright (c) 2020-2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        os_mq.h
 *
 * @brief       Header file for message queue interface.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-14   OneOS Team      First Version
 * 2021-01-08   OneOS team      Refactor header file for message queue interface interface.
 ***********************************************************************************************************************
 */

#ifndef __OS_MQ_H__
#define __OS_MQ_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_list.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef OS_USING_MESSAGEQUEUE

#define OS_MQ_WAKE_TYPE_PRIO                0x55
#define OS_MQ_WAKE_TYPE_FIFO                0xAA

/**
 ***********************************************************************************************************************
 * @struct      struct os_mq_msg
 *
 * @brief       This is message header that is placed at the head of the message.
 ***********************************************************************************************************************
 */
struct os_mq_msg
{
    struct os_mq_msg *next;             /* Point to the next message */
    os_size_t         msg_len;          /* message size */
};
typedef struct os_mq_msg os_mq_msg_t;
typedef struct os_mq_msg os_mq_msg_hdr_t;

/**
 ***********************************************************************************************************************
 * @struct      struct os_mq
 *
 * @brief       Define control block of message queue.
 ***********************************************************************************************************************
 */
struct os_mq
{
    void          *msg_pool;                /* Point to message pool. */
    os_mq_msg_t   *msg_queue_head;          /* Point to first entry of message queue. */
    os_mq_msg_t   *msg_queue_tail;          /* Point to last entry of message queue. */
    os_mq_msg_t   *msg_queue_free;          /* Point to first entry of free message resource. */    

    os_list_node_t send_task_list_head;     /* Sender tasks blocked on this messages queue. */
    os_list_node_t recv_task_list_head;     /* Receiver tasks blocked on this messages queue. */

    os_list_node_t resource_node;           /* Node in resource list */

    os_size_t      max_msg_size;            /* Max message size of each message. */
    os_uint16_t    queue_depth;             /* Maximum number of messages that can be put into message queue. */
    os_uint16_t    entry_count;             /* Number of messages queued. */

    os_uint8_t     object_inited;           /* Indicates whether object is inited or deinited, value is
                                               OS_KOBJ_INITED or OS_KOBJ_DEINITED */
    os_uint8_t     object_alloc_type;       /* Indicates whether object is allocated dynamically or statically, 
                                               value is OS_KOBJ_ALLOC_TYPE_STATIC or OS_KOBJ_ALLOC_TYPE_DYNAMIC */
    os_uint8_t     wake_type;               /* The type to wake up blocking tasks, value is OS_MQ_WAKE_TYPE_PRIO
                                               or OS_MQ_WAKE_TYPE_FIFO */
    
    char           name[OS_NAME_MAX + 1];   /* Message queue name. */
};
typedef struct os_mq os_mq_t;


#ifdef OS_USING_SYS_HEAP
extern os_mq_t  *os_mq_create(const char *name, os_size_t msg_size, os_size_t max_msgs);
extern os_err_t  os_mq_destroy(os_mq_t *mq);
#endif /* OS_USING_SYS_HEAP */

/*
 * OS_MQ_POOL_DEFINE is used to define message queue's memory pool and the address is aligned.
 * OS_MQ_POOL_ADDR is used to get address of message queue's memory pool.
 * OS_MQ_POOL_SIZE is used to get size of message queue's memory pool.
 *
 * These macros are used when creating a message queue with the function os_mq_init().
 */
#define OS_MQ_POOL_DEFINE(name, msg_count, msg_size) OS_ALIGN(OS_ALIGN_SIZE) os_uint8_t     \
        pool_##name[(sizeof(os_mq_msg_hdr_t) + OS_ALIGN_UP(msg_size, OS_ALIGN_SIZE)) * msg_count]
#define OS_MQ_POOL_ADDR(name)   ((void *)pool_##name)
#define OS_MQ_POOL_SIZE(name)   sizeof(pool_##name)

extern os_err_t    os_mq_init(os_mq_t    *mq,
                              const char *name,
                              void       *msg_pool,
                              os_size_t   msg_pool_size,
                              os_size_t   msg_size);          
extern os_err_t    os_mq_deinit(os_mq_t *mq);

extern os_err_t    os_mq_send(os_mq_t *mq, void *buffer, os_size_t buff_size, os_tick_t timeout);
extern os_err_t    os_mq_send_urgent(os_mq_t *mq, void *buffer, os_size_t buff_size, os_tick_t timeout);

extern os_err_t    os_mq_recv(os_mq_t    *mq,
                              void       *buffer,
                              os_size_t   buff_size,
                              os_tick_t   timeout,
                              os_size_t  *recv_msg_size);

extern os_err_t    os_mq_set_wake_type(os_mq_t *mq, os_uint8_t wake_type);
extern void        os_mq_reset(os_mq_t *mq);
extern os_bool_t   os_mq_is_empty(os_mq_t *mq);
extern os_bool_t   os_mq_is_full(os_mq_t *mq);
extern os_uint16_t os_mq_get_queue_depth(os_mq_t *mq);
extern os_uint16_t os_mq_get_used_entry_count(os_mq_t *mq);
extern os_uint16_t os_mq_get_unused_entry_count(os_mq_t *mq);

#endif /* OS_USING_MESSAGEQUEUE */

#ifdef __cplusplus
}
#endif

#endif /* __OS_MQ_H__ */

