/**
 *********************************************************************************************************
 *                                                Micropython
 *
 *                              (c) Copyright CMCC IOT All Rights Reserved
 *
 * @file	: model_core.h
 * @brief	: 核心头文件，设备与总线模型的核心数据结构
 * @author	: 周子涵
 * @version : V1.00.00
 * @date	: 2019年9月17日
 * 
 * The license and distribution terms for this file may be found in the file LICENSE in this distribution
*********************************************************************************************************
*/

#ifndef __MODEL__CORE_H__
#define __MODEL__CORE_H__

#include "model_list.h"
#include <stdint.h>





/*
*********************************************************************************************************
*                                        核心数据结构
*********************************************************************************************************
*/
struct core{
    int             type;                  /*类型                                                       */ 
    int             flag;                  /*标志                                                       */
    char            *name;                 /*名称                                                       */
    mpy_os_list_t   list;                  /*链表                                                       */
};

#endif
