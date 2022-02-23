#include <device.h>
#include <string.h>
#include "usr_misc.h"
#include <os_memory.h>



#ifndef MICROPY_USING_AMS
struct model_misc_fun g_misc_api;
#else



static const char *g_dev_imei[]={
#define DEVICE_INFO(id, name, imei)     imei,
#include "usr_deviceid.h"
#undef DEVICE_INFO
};


#define DEVICE_IMEI(x)	g_dev_imei[x]


static const char * g_ams_dev_name[] ={   
#define DEVICE_INFO(id, name, imei)     name,
#include "usr_deviceid.h"
#undef DEVICE_INFO   
};



char * mp_misc_get_dev_name(int x)
{
	return (char *)g_ams_dev_name[x];
}
#endif

/**
*********************************************************************************************************
* @brief                                 释放整个链表
*
* @description: 调用此函数，释放整个链表。
*
* @param	  :	head		链表的头节点
*
* @return	  :	无 
*********************************************************************************************************
*/
void mp_free_list(mpy_os_list_t *head)
{
	mpy_os_list_t *tail = mpy_os_list_find_tail((mpy_os_list_t *)head);
	if (head != NULL){
		mpy_os_list_destroy(head, tail, os_free);
	}
}

void dev_free(device_info_t *dev)
{	
	mpy_os_list_del_init(&dev->owner.list);
	if (dev->owner.name){
		os_free(dev->owner.name);
	}
	os_free(dev);
}


void mp_misc_free_dev_list(mpy_os_list_t *head, mpy_os_list_t *tail)
{
	device_info_t *dev;
	mpy_os_list_t *node;
	for (node = tail->prev; node != head; node = node->prev)
	{
		dev = list_entry(list_entry(node->next, struct core, list) , device_info_t, owner);
		dev_free(dev);
	}
}



/**
*********************************************************************************************************
* @brief                                 查找操作系统底层同类设备 
*
* @description: 调用此函数，查找操作系统底层同类设备。
*
* @param	  :	prename		前缀名，用以判断同类	
*
* @return	  :	设备链表
*
* @note		  : 设备链表是双向环形链表
*********************************************************************************************************
*/
#if 0
device_info_t * mp_misc_find_similar_device(char *prename)
{
	uint8_t name_len=0;
	os_object_info_t *info;
	os_object_t 	*pos = NULL, *n = NULL;
	mpy_os_list_t *head =  get_list_head();
	device_info_t   *node = NULL, *sentinel = NULL;
	int first_into_flag = 1;
	
	os_enter_critical();
	
    info = os_object_get_info(OS_OBJECT_DEVICE);
	mpy_list_for_each_entry_safe(pos, n, &info->object_list, list)
    {
		if (0 == strncmp(pos->name, prename, strlen(prename)))
        {
			name_len = strlen(pos->name);
			node = os_malloc(sizeof(device_info_t));
			if (!node){ 
				goto malloc_error;
			}
			node->owner.name = os_malloc(name_len + 1);
			if (!node->owner.name){
				goto malloc_error;
			}
			strncpy(node->owner.name, pos->name, name_len);
			node->owner.name[name_len] = '\0';
			mpy_os_list_insert_before(head, &node->owner.list);
			if (first_into_flag){
				first_into_flag = 0;
				sentinel = node;
			}
        }
	}
	os_exit_critical();
	return sentinel;
	
malloc_error:
		mp_misc_free_dev_list(head);
		os_exit_critical();
		os_kprintf("[mp_misc_find_os_device] Falied to malloc memory!\n");
		return NULL;
}
#else

os_err_t find_device_callback(os_device_t *dev, void *param)
{
	uint8_t name_len=0;
	device_info_t   *node = NULL;

	struct similar_device **s_dev = param;

	if (0 == strncmp(dev->name, (*s_dev)->name, strlen((*s_dev)->name)))
	{
		node = os_malloc(sizeof(device_info_t));
		if (!node){
			goto malloc_error;
		}
		name_len = strlen(dev->name);
		
		node->owner.name = os_malloc(name_len + 1);
		if (!node->owner.name){
				goto malloc_error;
		}
		strncpy(node->owner.name, dev->name, name_len);
		node->owner.name[name_len] = '\0';
		
		mpy_os_list_insert_before((*s_dev)->head, &node->owner.list);
		if ((*s_dev)->head == get_list_head()){
			(*s_dev)->tail = node;
		}
		(*s_dev)->head = &node->owner.list;
	}
	return OS_EOK;
	
malloc_error:
		mp_misc_free_dev_list((*s_dev)->head, &((*s_dev)->tail->owner.list));
		mp_err("[mp_misc_find_os_device] Falied to malloc memory!\n");
		return OS_ERROR;
}


struct similar_device * mp_misc_find_similar_device(const char *prename)
{

	struct similar_device *dev= os_malloc(sizeof(struct similar_device));
	if (!dev){
	    mp_err("Failed to allocate memory!");
		return NULL;
	}
	dev->name = prename;
	dev->head = get_list_head();
	dev->tail = OS_NULL;
	os_device_for_each(find_device_callback, &dev);

	dev->head = dev->head->prev; // contain itself
	return dev;
}

#ifndef MICROPY_USING_AMS
struct model_misc_fun *model_get_misc_structure(void)
{
	return &g_misc_api;
}
#else

/**
 *********************************************************************************************************
 *                                      get device id (imei, imsi, iccid)
 *********************************************************************************************************
*/
void *usr_misc_get_imei(void)
{
	return (void *)(DEVICE_IMEI(AMS_DEVICE_ID));
}

void *usr_misc_get_imsi(void)
{
	return "460042370500473";
}

void *usr_misc_get_iccid(void)
{
	return "89860403101892378494";
}
#endif

#endif
