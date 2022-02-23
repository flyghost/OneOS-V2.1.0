#ifndef __USR_MISC_H
#define __USR_MISC_H

#include "mpconfigport.h"
#include <model_device.h>
#include <model_log.h>

#define usr_malloc(sz)      os_malloc(sz)



typedef enum mp_heap{
	MP_HEAP_ALLOC,
	MP_HEAP_RAM_ADDR
}mp_heap_flag_t;




struct similar_device{
	const char *name;
	mpy_os_list_t *head;
	device_info_t *tail;
};

/**
 *********************************************************************************************************
 *                                      micropython define log macro
 *********************************************************************************************************
*/



/**
 *********************************************************************************************************
 *                                      Inherited the log macro from  model-log  
 *********************************************************************************************************
*/
#define usr_kprintf(msg ,...)    model_kprintf(msg, ##__VA_ARGS__)	


#define mp_log(msg, ...)	MODEL_LOG("[MICROPYTHON]", msg, ##__VA_ARGS__)
#define mp_err(msg, ...)	MODEL_ERR("[MICROPYTHON]", msg, ##__VA_ARGS__)

/**
*********************************************************************************************************
* @brief                                 分配堆空间
*
* @description: 调用此函数，为MicroPython分配堆空间。
*
* @param	  :	size_or_addr	需要分配的空间大小，或者RAM区域地址
*
*				flag			分配堆空间的方式
*								MP_HEAP_ALLOC: 调用malloc分配空间
*								MP_HEAP_RAM_ADDR：RAM地址设置方式，直接指定RAM区域为堆MicroPython
*
* @return	  :	MicroPython堆空间地址
*********************************************************************************************************
*/
void * mp_heap_malloc(mp_size_t size_or_addr, mp_heap_flag_t flag);

/**
*********************************************************************************************************
* @brief                                 释放堆空间
*
* @description: 调用此函数，为MicroPython释放堆空间。
*
* @param	  :	addr		需要分配的空间大小，或者RAM区域地址
*
*				flag		目标堆空间当时创建的方式
*							MP_HEAP_ALLOC: 调用malloc分配空间
*							MP_HEAP_RAM_ADDR：RAM地址设置方式，直接指定RAM区域为堆MicroPython
*
* @return	  :	MicroPython堆空间地址
*********************************************************************************************************
*/
void mp_heap_free(void * addr, mp_heap_flag_t flag);




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
void mp_free_list(mpy_os_list_t *head);


/**
*********************************************************************************************************
* @brief                                 查找操作系统底层同类设备
*
* @description: 调用此函数，查找操作系统底层同类设备。
*
* @param	  :	dev_type		设备类型，用以判断同类	
*
* @return	  :	设备链表
*
* @note		  : 设备链表是双向环形链表
*********************************************************************************************************
*/
struct similar_device * mp_misc_find_similar_device(const char *prename);

extern int mpy_repl_entry(void);
extern int mpy_file_entry(void* argument);
extern uint8_t micropy_file_exit(void);
extern os_err_t usr_task_delete(os_task_t *task);
extern void *os_get_current_task_sp(void);
#define TASK_SP_SELF        (os_task_self()->stack_top)
#define TASK_SP_REGS        (os_get_current_task_sp())

#ifdef MICROPY_USING_AMS

enum {
#define DEVICE_INFO(id, name, imei)     id,
#include "usr_deviceid.h"
#undef DEVICE_INFO
	DEVICE_MAX,
};

#define AMS_DEVICE_ID   DEV_66_6



char * mp_misc_get_dev_name(int x);
void *usr_misc_get_imei(void);
void *usr_misc_get_imsi(void);
void *usr_misc_get_iccid(void);
#endif

#endif

