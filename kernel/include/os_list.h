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
 * @file        os_list.h
 *
 * @brief       Provide double list and single list functions
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-13   OneOS Team      First version
 ***********************************************************************************************************************
 */

#ifndef __OS_LIST_H__
#define __OS_LIST_H__

#include "os_types.h"
#include "os_stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 ***********************************************************************************************************************
 * @struct      os_list_node
 *
 * @brief       Double List structure
 ***********************************************************************************************************************
 */
struct os_list_node
{
    struct os_list_node *next;      /* Point to next node */
    struct os_list_node *prev;      /* point to previous node */
};
typedef struct os_list_node os_list_node_t;

/**
 ***********************************************************************************************************************
 * @brief           Initialize a list head node or entry node.
 *
 * @param[in]       node            List head node or entry node to be initialized.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_init(os_list_node_t *node)
{
    node->next = node;
    node->prev = node;
}

/**
 ***********************************************************************************************************************
 * @brief           Add a new entry to list.
 *
 * @details         Insert a new entry after the specified head.
 *
 * @param[in]       head            List head to add it after.
 * @param[in]       entry           New entry to be added.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_add(os_list_node_t *head, os_list_node_t *entry)
{
    head->next->prev = entry;
    entry->next = head->next;

    head->next = entry;
    entry->prev = head;
    
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Add a new entry to list.
 *
 * @details         Insert a new entry before the specified head.
 *
 * @param[in]       head            List head to add it before.
 * @param[in]       entry           New entry to be added.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_add_tail(os_list_node_t *head, os_list_node_t *entry)
{ 
    head->prev->next = entry;
    entry->prev = head->prev;

    head->prev = entry;
    entry->next = head;
    
    return;
}

OS_INLINE void __os_list_del(os_list_node_t *prev, os_list_node_t *next)
{
    next->prev = prev;
    prev->next = next;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete entry from list
 *
 * @param[in]       entry           The entry to delete from the list.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_del(os_list_node_t *entry)
{
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
 
    entry->next = entry;
    entry->prev = entry;

    return;
}

OS_INLINE void __os_list_del_entry(os_list_node_t *entry)
{
    __os_list_del(entry->prev, entry->next);
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Deletes entry from list and reinitialize it.
 *
 * @param[in]       entry           The entry to delete from the list.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_del_init(os_list_node_t *entry)
{
    __os_list_del_entry(entry);
    os_list_init(entry);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete from one list and add as another's head.
 *
 * @param[in]       head            The head that will precede our entry.
 * @param[in]       entry           The entry to move.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_move(os_list_node_t *head, os_list_node_t *entry)
{
    __os_list_del_entry(entry);
    os_list_add(head, entry);

	return;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete from one list and add as another's tail.
 *
 * @param[in]       head            The head that will follow our entry.
 * @param[in]       entry           The entry to move.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_move_tail(os_list_node_t *head, os_list_node_t *entry)
{
    __os_list_del_entry(entry);
    os_list_add_tail(head, entry);

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Tests whether a list is empty.
 *
 * @param[in]       head            The list to test.
 *
 * @return          Whether a list is empty.
 * @retval          OS_TRUE         The list is empty.
 * @retval          OS_FALSE        The list is not empty.
 ***********************************************************************************************************************
 */
OS_INLINE os_bool_t os_list_empty(const os_list_node_t *head)
{
    return (head->next == head);
}

/**
 ***********************************************************************************************************************
 * @brief           Calculate entry count of a list.
 *
 * @param[in]       head            The list to calculate entry count.
 *
 * @return          Entry count of the list.
 ***********************************************************************************************************************
 */
OS_INLINE os_uint32_t os_list_len(const os_list_node_t *head)
{
    os_uint32_t len;
    const os_list_node_t *node;

    len = 0;
    node = head;
       
    while (node->next != head)
    {
        node = node->next;
        len ++;
    }

    return len;
}

OS_INLINE void __os_list_splice(os_list_node_t *head, os_list_node_t *list)
{
    os_list_node_t *first;
    os_list_node_t *last;
    os_list_node_t *at;

    first = list->next;
    last = list->prev;
    at = head->next;
    
    first->prev = head;
    head->next = first;
    
    last->next = at;
    at->prev = last;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Join two lists.
 *
 * @param[in]       head            The place to add it in the second list.
 * @param[in]       list            The new list to add.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_splice(os_list_node_t *head, os_list_node_t *list)
{
    if (!os_list_empty(list))
    {
        __os_list_splice(head, list);
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Join two lists and reinitialise the emptied list.
 *
 * @attention       The list at @list is reinitialised
 *
 * @param[in]       head            The place to add it in the second list.
 * @param[in]       list            The new list to add.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_list_splice_init(os_list_node_t *head, os_list_node_t *list)
{
    if (!os_list_empty(list))
    {
        __os_list_splice(head, list);
        os_list_init(list);
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Return first entry of a list.
 *
 * @param[in]       head            The list to find first entry.
 *
 * @return          The first entry.
 * @retval          OS_NULL         The list is empty, not found first entry.
 * @retval          else            The first entry of the list.
 ***********************************************************************************************************************
 */
OS_INLINE os_list_node_t *os_list_first(os_list_node_t *head)
{
    if (head->next == head)
    {
        return OS_NULL;
    }
    
    return head->next;
}

/**
 ***********************************************************************************************************************
 * @brief           Return last entry of a list.
 *
 * @param[in]       head            The list to find last entry.
 *
 * @return          The last entry.
 * @retval          OS_NULL         The list is empty, not found last entry.
 * @retval          else            The last entry of the list.
 ***********************************************************************************************************************
 */
OS_INLINE os_list_node_t *os_list_tail(os_list_node_t *head)
{
    os_list_node_t *node;

    if (head->next == head)
    {
        return OS_NULL;
    }
    
    node = head; 
    while (node->next != head)
    {
        node = node->next;
    }

    return node;
}

/* Initialize a double list, this micro is only as right value */
#define OS_LIST_INIT(name)          {&(name), &(name)}

/**
 ***********************************************************************************************************************
 * @def         os_list_entry
 *
 * @brief       Get the struct for this entry
 *
 * @param       ptr             The &struct os_list_node pointer.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the list_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_list_entry(ptr, type, member)                                    \
    os_container_of(ptr, type, member)

/**
 ***********************************************************************************************************************
 * @def         os_list_first_entry
 *
 * @brief       Get the first entry from a list
 *
 * @attention   That list is expected to be not empty.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the list_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_list_first_entry(head, type, member)                             \
    os_list_entry((head)->next, type, member)

/**
 ***********************************************************************************************************************
 * @def         os_list_first_entry_or_null
 *
 * @brief       Get the first entry from a list
 *
 * @attention   That list may be empty. If empty, return OS_NULL.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the list_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_list_first_entry_or_null(head, type, member)                     \
    (!os_list_empty(head) ? os_list_first_entry(head, type, member) : OS_NULL)


/**
 ***********************************************************************************************************************
 * @def         os_list_tail_entry
 *
 * @brief       Get the last entry from a list
 *
 * @attention   That list is expected to be not empty.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the list_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_list_tail_entry(head, type, member)                              \
    os_list_entry(os_list_tail(head), type, member)

/**
 ***********************************************************************************************************************
 * @def         os_list_tail_entry_or_null
 *
 * @brief       Get the last entry from a list
 *
 * @attention   That list may be empty. If empty, return OS_NULL.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the list_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_list_tail_entry_or_null(head, type, member)                      \
    (!os_list_empty(head) ? os_list_tail_entry(head, type, member) : OS_NULL)

/**
 ***********************************************************************************************************************
 * @def         os_list_for_each
 *
 * @brief       Iterate over a list
 *
 * @param       pos             The &struct os_list_node to use as a loop counter.
 * @param       head            The head for your list.
 ***********************************************************************************************************************
 */
#define os_list_for_each(pos, head)                                         \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 ***********************************************************************************************************************
 * @def         os_list_for_each_safe
 *
 * @brief       Iterate over a list safe against removal of list entry
 *
 * @param       pos             The &struct os_list_node to use as a loop counter.
 * @param       n               Another &struct os_list_node to use as temporary storage
 * @param       head            The head for your list.
 ***********************************************************************************************************************
 */
#define os_list_for_each_safe(pos, n, head)                                 \
    for (pos = (head)->next, n = pos->next;                                 \
         pos != (head);                                                     \
         pos = n, n = pos->next)

/**
 ***********************************************************************************************************************
 * @def         os_list_for_each_entry
 *
 * @brief       Iterate over list of given type
 *
 * @param       pos             The type * to use as a loop counter.
 * @param       head            The head for your list.
 * @param       type            The type of the struct same as pos(loop counter).
 * @param       member          The name of the os_list_node within the struct.
 ***********************************************************************************************************************
 */
#define os_list_for_each_entry(pos, head, type, member)                     \
    for (pos = os_list_entry((head)->next, type, member);                   \
         &pos->member != (head);                                            \
         pos = os_list_entry(pos->member.next, type, member))

/**
 ***********************************************************************************************************************
 * @def         os_list_for_each_entry_safe
 *
 * @brief       Iterate over list of given type safe against removal of list entry.
 *
 * @param       pos             The type * to use as a loop counter.
 * @param       n               Another type * to use as temporary storage.
 * @param       head            The head for your list.
 * @param       type            The type of the struct same as pos(loop counter).
 * @param       member          The name of the os_list_node within the struct.
 ***********************************************************************************************************************
 */
#define os_list_for_each_entry_safe(pos, n, head, type, member)             \
    for (pos = os_list_entry((head)->next, type, member),                   \
         n = os_list_entry(pos->member.next, type, member);                 \
         &pos->member != (head);                                            \
         pos = n, n = os_list_entry(n->member.next, type, member))


/**
 ***********************************************************************************************************************
 * @struct      os_slist_node
 *
 * @brief       Single List structure
 ***********************************************************************************************************************
 */
struct os_slist_node
{
    struct os_slist_node *next;         /* Point to next node */
};
typedef struct os_slist_node os_slist_node_t;


/**
 ***********************************************************************************************************************
 * @brief           Initialize a single list
 *
 * @param[in]       node            The single list to be initialized
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_slist_init(os_slist_node_t *node)
{
    node->next = OS_NULL;
    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Add a new entry to front of a list.
 *
 *
 * @param[in]       head            List head.
 * @param[in]       entry           New entry to be added.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_slist_add(os_slist_node_t *head, os_slist_node_t *entry)
{
    entry->next = head->next;
    head->next = entry;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Add a new entry to tail of a list.
 *
 *
 * @param[in]       head            List head.
 * @param[in]       entry           New entry to be added.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_slist_add_tail(os_slist_node_t *head, os_slist_node_t *entry)
{
    os_slist_node_t *node;

    node = head;
    while (node->next)
    {
        node = node->next;
    }

    /* append the node to the tail */
    node->next = entry;
    entry->next = OS_NULL;

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Delete entry from list
 *
 * @param[in]       head            List head.
 * @param[in]       entry           The entry to delete from the list.
 *
 * @return          No return value.
 ***********************************************************************************************************************
 */
OS_INLINE void os_slist_del(os_slist_node_t *head, os_slist_node_t *entry)
{
    os_slist_node_t *node;

    node = head;
    while (node->next && (node->next != entry))
    {
        node = node->next;
    }
    
    /* del entry node */
    if (node->next != OS_NULL)
    {
        node->next = node->next->next;
    }

    return;
}

/**
 ***********************************************************************************************************************
 * @brief           Calculate entry count of a list.
 *
 * @param[in]       head            The list to calculate entry count.
 *
 * @return          Entry count of the list.
 ***********************************************************************************************************************
 */
OS_INLINE os_uint32_t os_slist_len(const os_slist_node_t *head)
{
    os_uint32_t len;
    const os_slist_node_t *node;

    len = 0;
    node = head->next;
    
    while (node != OS_NULL)
    {
        node = node->next;
        len++;
    }

    return len;
}

/**
 ***********************************************************************************************************************
 * @brief           Return first entry of a list.
 *
 * @param[in]       head            The list to find first entry.
 *
 * @return          The first entry.
 * @retval          OS_NULL         The list is empty, not found first entry.
 * @retval          else            The first entry of the list.
 ***********************************************************************************************************************
 */
OS_INLINE os_slist_node_t *os_slist_first(os_slist_node_t *head)
{
    return head->next;
}

/**
 ***********************************************************************************************************************
 * @brief           Return last entry of a list.
 *
 * @param[in]       head            The list to find last entry.
 *
 * @return          The last entry.
 * @retval          OS_NULL         The list is empty, not found last entry.
 * @retval          else            The last entry of the list.
 ***********************************************************************************************************************
 */
OS_INLINE os_slist_node_t *os_slist_tail(os_slist_node_t *head)
{
    os_slist_node_t *node;

    if (head->next == OS_NULL)
    {
        return OS_NULL;
    }
    
    node = head; 
    while (node->next)
    {
        node = node->next;
    }

    return node;
}

/**
 ***********************************************************************************************************************
 * @brief           Return next entry of current entry.
 *
 * @param[in]       node            Current entry.
 *
 * @return          The next entry of current entry.
 * @retval          OS_NULL         The current entry is last, not found next entry.
 * @retval          else            The next entry of current entry.
 ***********************************************************************************************************************
 */
OS_INLINE os_slist_node_t *os_slist_next(os_slist_node_t *node)
{
    return node->next;
}

/**
 ***********************************************************************************************************************
 * @brief           Tests whether a list is empty.
 *
 * @param[in]       head            The list to test.
 *
 * @return          Whether a list is empty.
 * @retval          OS_TRUE         The list is empty.
 * @retval          OS_FALSE        The list is not empty.
 ***********************************************************************************************************************
 */
OS_INLINE os_bool_t os_slist_empty(os_slist_node_t *head)
{
    return (head->next == OS_NULL);
}

/* Initialize a single list, this micro is only as right value */
#define OS_SLIST_INIT(name)         {OS_NULL}

/**
 ***********************************************************************************************************************
 * @def         os_slist_entry
 *
 * @brief       Get the struct for this entry
 *
 * @param       ptr             The &struct os_slist_node pointer.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the slist_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_slist_entry(ptr, type, member)                                   \
    os_container_of(ptr, type, member)

/**
 ***********************************************************************************************************************
 * @def         os_slist_for_each
 *
 * @brief       Iterate over a list
 *
 * @param       pos             The &struct os_slist_node to use as a loop counter.
 * @param       head            The head for your list.
 ***********************************************************************************************************************
 */
#define os_slist_for_each(pos, head)                                        \
    for (pos = (head)->next; pos != OS_NULL; pos = pos->next)

/**
 ***********************************************************************************************************************
 * @def         os_slist_for_each_safe
 *
 * @brief       Iterate over a list safe against removal of list entry
 *
 * @param       pos             The &struct os_slist_node to use as a loop counter.
 * @param       n               Another &struct os_slist_node to use as temporary storage
 * @param       head            The head for your list.
 ***********************************************************************************************************************
 */
#define os_slist_for_each_safe(pos, n, head)                                \
    for (pos = (head)->next, n = (pos != OS_NULL) ? pos->next : OS_NULL;    \
         pos != OS_NULL;                                                    \
         pos = n, n = (pos != OS_NULL) ? pos->next : OS_NULL)

/**
 ***********************************************************************************************************************
 * @def         os_slist_first_entry
 *
 * @brief       Get the first entry from a list
 *
 * @attention   That list is expected to be not empty.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the slist_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_slist_first_entry(head, type, member)                            \
    os_slist_entry((head)->next, type, member)

/**
 ***********************************************************************************************************************
 * @def         os_slist_first_entry_or_null
 *
 * @brief       Get the first entry from a list
 *
 * @attention   That list may be empty. If empty, return OS_NULL.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the slist_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_slist_first_entry_or_null(head, type, member)                    \
	(!os_slist_empty(head) ? os_slist_first_entry(head, type, member) : OS_NULL)


/**
 ***********************************************************************************************************************
 * @def         os_slist_tail_entry
 *
 * @brief       Get the last entry from a list
 *
 * @attention   That list is expected to be not empty.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the slist_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_slist_tail_entry(head, type, member)                             \
    os_slist_entry(os_slist_tail(head), type, member)

/**
 ***********************************************************************************************************************
 * @def         os_slist_tail_entry_or_null
 *
 * @brief       Get the last entry from a list
 *
 * @attention   That list may be empty. If empty, return OS_NULL.
 *
 * @param       head            The list head to take the entry from.
 * @param       type            The type of the struct this is embedded in.
 * @param       member          The name of the list_struct within the struct.
 ***********************************************************************************************************************
 */
#define os_slist_tail_entry_or_null(head, type, member)                     \
	(!os_slist_empty(head) ? os_slist_tail_entry(head, type, member) : OS_NULL)

#ifdef __cplusplus
}
#endif

#endif /* __OS_LIST_H__ */

