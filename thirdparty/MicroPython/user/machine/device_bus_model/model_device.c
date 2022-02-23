/**
 *********************************************************************************************************
 *                                                Micropython
 *
 *                              (c) Copyright CMCC IOT All Rights Reserved
 *
 * @file	: model_device.c
 * @brief	: 设备管理系统
 * @author	: 周子涵
 * @version : V1.00.00
 * @date	: 2019年9月17日
 * 
 * The license and distribution terms for this file may be found in the file LICENSE in this distribution
*********************************************************************************************************
*/

#include "model_device.h"
#include <stdio.h>
#include <string.h>

#include <os_util.h>


static mpy_os_list_t device_list_head;

/**
*********************************************************************************************************
*                                      初始化设备链表
*
* @Description: 初始化设备链表，优先调用。
*
* @Arguments  : 
*
* @Returns    : 结构体的首地址
*********************************************************************************************************
*/
int mpycall_device_list_init(void)
{
    mpy_os_list_init(&device_list_head);
	return 0;
}

/**
 ********************************************************************************************************
 *                                      获取设备链表头节点
 *
 * @description: 调用此函数,获取设备链表头节点
 *
 * @param      : device      :设备结构体指针
 *
 * @returns    : 无
 ********************************************************************************************************
 */
mpy_os_list_t * get_list_head(void)
{
	return &device_list_head;
}
/**
*********************************************************************************************************
*                                      添加设备
*
* @Description: 调用此函数向设备管理系统中添加设备。
*
* @Arguments  : device      :设备结构体指针
*
* @Returns    : 无
*
* @note 	  : device_list_head链表是环形双向链表
*********************************************************************************************************
*/
void mpycall_device_add(device_info_t *device)
{
	mpy_os_list_insert_after(&device_list_head, &device->owner.list);   
}

/**
*********************************************************************************************************
*                                      添加一批设备
*
* @Description: 调用此函数向设备管理系统中添加添加一批设备,这一批设备在一个设备链表中。
*
* @Arguments  : device      :设备链表中的第一个设备结构体指针
*
* @Returns    : 无
*
* @note 	  : device_list_head链表是环形双向链表
*********************************************************************************************************
*/
void mpycall_device_add_list(device_info_t *device)
{
	mpy_os_list_cycle_merge(&device_list_head, &device->owner.list);
}
	

/**
*********************************************************************************************************
*                                      删除设备
*
* @Description: 调用此函数，向设备管理系统中删除目标设备。
*
* @Arguments  : device      :设备结构体指针
*
* @Returns    : -1 :failed to remove device. 0 : succeed to remove target.
*********************************************************************************************************
*/
int mpycall_device_remove(device_info_t *device)
{
    if (device == NULL){
        printf("the device pointer is NULL.");
        return -1;
    }
	
    device_info_t *dev = mpycall_device_find(device->owner.name);
    if (dev != NULL)
    {
        mpy_os_list_remove(&dev->owner.list);
    } else {
        printf(" the device is not in the DEVICE MANAGEMENT SYSTEM!");
    }
    return 0;
}

/**
*********************************************************************************************************
*                                      查找设备
*
* @Description: 调用此函数，在设备管理系统中查找设备。
*
* @Arguments  : name      :设备名称
*
* @Returns    : 成功返回设备结构体指针，失败返回空
*********************************************************************************************************
*/
device_info_t *mpycall_device_find(char *name)
{
    struct core *pos = NULL, *n = NULL;

    mpy_list_for_each_entry_safe(pos, n, &device_list_head, list)
	{
        if (strcmp(pos->name, name) == 0){
			return  list_entry(pos, device_info_t, owner);
        }
    }

	return NULL;
}

/**
*********************************************************************************************************
*                                      查找操作系统中的设备
*
* @Description: 调用此函数，查找操作系统中的设备。
*
* @Arguments  : name      :设备名称
*
* @Returns    : 成功返回设备结构体指针，失败返回空
*********************************************************************************************************
*/
device_info_t *mpycall_device_find_os_device(char *prename, void *fun)
{
	return 0;
}


void mpycall_device_register(device_info_t *device)
{
		
}

void mpycall_device_unregister(device_info_t *device)
{
    
}

/**
*********************************************************************************************************
*                                      获取设备
*
* @Description: 调用此函数，获取设备表中当前设备。
*
* @Arguments  : dev      	:当前设备地址
*
* @Returns    : 成功返回设备结构体指针，失败返回空
*********************************************************************************************************
*/
device_info_t * mpycall_get_device(mpy_os_list_t *list)
{
	struct core  *owner_t;
	owner_t = list_entry(list, struct core, list);
	return  list_entry(owner_t, device_info_t, owner);
}

/**
*********************************************************************************************************
*                                      获取下一个设备
*
* @Description: 调用此函数，获取设备表中当前设备的下一个设备。
*
* @Arguments  : dev      	:当前设备地址
*
* @Returns    : 成功返回设备结构体指针，失败返回空
*********************************************************************************************************
*/
device_info_t * mpycall_get_next_dev(device_info_t *dev)
{
	return mpycall_get_device(dev->owner.list.next);
}

/**
*********************************************************************************************************
*                                      获取上一个设备
*
* @Description: 调用此函数，获取设备表中当前设备的上一个设备。
*
* @Arguments  : dev      	:当前设备地址
*
* @Returns    : 成功返回设备结构体指针，失败返回空
*********************************************************************************************************
*/
device_info_t * mpycall_get_prev_dev(device_info_t *dev)
{
	struct core  *owner_t;
	device_info_t *prev;
	owner_t = list_entry(dev->owner.list.prev, struct core, list);
	prev = list_entry(owner_t, device_info_t, owner);
	return prev;
}

/**
*********************************************************************************************************
*                                      列出设备中的
*
* @Description: 调用此函数，设备管理系统中的所有设备。
*
* @Returns    : 0
*********************************************************************************************************
*/
uint32_t mpycall_device_listall(void)
{
	mpy_os_list_t * head;
	struct core * nodehead;
	head = &device_list_head;
	
	for(;head->next != &device_list_head;)
	{
		head = head->next;
		nodehead = list_entry(head ,struct core, list);
		os_kprintf("%s\r\n", nodehead->name);
	}
	return 0;
}
