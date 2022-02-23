#ifndef __DATA_NODE_H__
#define __DATA_NODE_H__

#include <os_task.h>
#include "os_types.h"
struct rt_data_node
{
    char *data_ptr;
    os_uint32_t data_size;
};

struct rt_data_node_list
{
    struct rt_data_node *node;
    os_uint32_t size;
    os_uint32_t read_index, write_index;
    os_uint32_t data_offset;
    void (*read_complete)(struct rt_data_node *node, void *user_data);
    void *user_data;
};

int rt_data_node_init(struct rt_data_node_list **node_list, os_uint32_t size);
int rt_data_node_is_empty(struct rt_data_node_list *node_list);
int rt_data_node_write(struct rt_data_node_list *node_list, void *buffer, os_uint32_t size);
int rt_data_node_read(struct rt_data_node_list *node_list, void *buffer, os_uint32_t size);
void rt_data_node_empty(struct rt_data_node_list *node_list);

#endif
