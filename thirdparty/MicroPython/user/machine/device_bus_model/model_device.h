/**
 *********************************************************************************************************
 *                                                Micropython
 *
 *                              (c) Copyright CMCC IOT All Rights Reserved
 *
 * @file	: model_device.h
 * @brief	: 设备头文件，设备模型的相关数据结构
 * @author	: 周子涵
 * @version : V1.00.00
 * @date	: 2019年9月17日
 * 
 * The license and distribution terms for this file may be found in the file LICENSE in this distribution
*********************************************************************************************************
*/

#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "model_core.h"


#ifdef __cplusplus
extern "C" {
#endif



/*
*********************************************************************************************************
*                                        设备 数据结构 
*********************************************************************************************************
*/



/*
*********************************************************************************************************
*                                        设备类型宏
*********************************************************************************************************
*/

#define DEV_BUS      0x00000001
#define DEV_KEY      0x00000002
#define DEV_SENSOR   0x00000004
#define DEV_DAC      0x00000008
#define DEV_PANEL    0X00000010
#define DEV_FLASH    0x00000020
#define DEV_MOTOR    0x00000040
#define DEV_TIMER    0x00000080
#define DEV_HUMITURE 0x00000100
#define DEV_BATTERY  0x00000200
#define DEV_WDT      0x00000400
#define DEV_LIGHT    0x00000800
#define DEV_LED      0x00001000
#define DEV_SIX_AXIS 0x00002000
#define DEV_BEEP	 0x00004000
#define DEV_AUDIO	 0x00008000
#define DEV_RTC	 	 0x00010000
#define DEV_PM	 	 0x00020000

/*
enum device_type
{
     DEV_BUS    = 0x0001;
     DEV_KEY    = 0x0002;
     DEV_SENSOR = 0x0004;
     DEV_TP     = 0x0008;
     DEV_PANEL  = 0X0010;
     DEV_FLASH  = 0x0020;
}
*/
/*
enum device_flag
{
    OPEN_KEY = 0x0001,
    DEVICE_SENSOR = 0x0002,
    DEVICE_TP = 0x0004,
    DEVICE_PANEL = 0X0008,
    
};

enum device_ctrl
{
    DEVICE_SUSPEND=0,
    DEVICE_RESUME,
    DEVICE_PROBE,
};
*/

/*
 ********************************************************************************************************
 *                                        设备操作接口
 ********************************************************************************************************
 */
struct operate
{
    int (*open)(const char *dev_name);
    int (*close)(const char *dev_name);
    #if 0 // It's no longer used in micropython
    int (*remove)(void *device);
    int (*suspend)(void *device, void *msg);
    int (*resume)(void *device);
    void (*callback)(void *device, void *arg);
    #endif
    int (*ioctl)(void *device, int cmd, void *arg);
    int (*read)(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize);
    int (*write)(const char *dev_name, uint32_t offset, void *buf, uint32_t bufsize); 
};

/*
 ********************************************************************************************************
 *                                        设备描述
 ********************************************************************************************************
 */
struct model_device_info
{
	struct core  owner;  /*设备所有者，设备管理核心                                   */
    int type;            /*0x0000 0000 包含设备类型与bus类型 DEVICE_TP<< 16 | I2C_BUS */
    int32_t id;          /*设备id                                                    */
    uint8_t open_flag;   /* 打开标志                                                 */
    uint8_t ref_count;   /* 被应用次数                                               */
    void *bus;           /*总线指针，包括i2c ，spi, uart等                            */
    struct operate *ops; /*设备通用操作函数指针                                       */
    void *other;         /*用户预留                                                  */
};
typedef struct model_device_info device_info_t;

/*
 ********************************************************************************************************
 *                                        device manager function
 ********************************************************************************************************
 */



/**
*********************************************************************************************************
*                                      添加设备
*
* @Description: 调用此函数向设备管理系统中添加设备。
*
* @Arguments  : device      :设备结构体指针
*
* @Returns    : 无
*********************************************************************************************************
*/
void mpycall_device_add(device_info_t *device);

/**
*********************************************************************************************************
*                                      添加一批设备
*
* @Description: 调用此函数向设备管理系统中添加添加一批设备,这一批设备在一个设备链表中。
*
* @Arguments  : device      :设备链表中的第一个设备结构体指针
*
* @Returns    : 无
*********************************************************************************************************
*/
void mpycall_device_add_list(device_info_t *device);

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
int mpycall_device_remove(device_info_t *device);


void mpycall_device_register(device_info_t *device);

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
device_info_t *mpycall_device_find(char *name);

/**
*********************************************************************************************************
*                                      初始化设备链表
*
* @Description: 初始化设备链表，优先调用。
*
* @Returns    : 结构体的首地址
*********************************************************************************************************
*/
int mpycall_device_list_init(void);

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
device_info_t * mpycall_get_device(mpy_os_list_t *list);
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
device_info_t * mpycall_get_next_dev(device_info_t *dev);

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
device_info_t * mpycall_get_prev_dev(device_info_t *dev);

/**
*********************************************************************************************************
*                                      列出设备中的
*
* @Description: 调用此函数，设备管理系统中的所有设备。
*
* @Returns    : 0
*********************************************************************************************************
*/
uint32_t mpycall_device_listall(void);

mpy_os_list_t * get_list_head(void);

/**
 *********************************************************************************************************
 * @brief                                 设备链表循环操作
 *
 * @description: 调用此函数，查找操作系统底层同类设备。
 *
 * @param	  :	pos			设备指针，中间使用变量
 *
 *				first		第一个设备的地址
 *
 *				head		链表头节点
 *
 * @return	  :	设备链表
 *
 * @note		  : 设备链表是双向环形链表
 *********************************************************************************************************
*/
#define DEV_LIST_LOOP(pos, first, head) 												\
	for (pos = first; &pos->owner.list != head;											\
		 pos = list_entry(mpy_list_next_entry(&(pos)->owner, list), typeof(*pos), owner))



/**
 *********************************************************************************************************
 * @brief                                 设备链表循环操作
 *
 * @description: 调用此函数，查找操作系统底层同类设备。
 *
 * @param	  :	pos			设备指针，中间使用变量
 *
 *				first		第一个设备的地址
 *
 *				head		链表头节点
 *
 * @return	  :	设备链表
 *
 * @note		  : 设备链表是双向环形链表
 *********************************************************************************************************
*/
#define DEV_LIST_LOOP_REVERSE(pos, tail, head) 											\
	for (pos = tail; &pos->owner.list != head;											\
		 pos = list_entry(mpy_list_prev_entry(&(pos)->owner, list), typeof(*pos), owner))


#ifdef __cplusplus
}
#endif

#endif
