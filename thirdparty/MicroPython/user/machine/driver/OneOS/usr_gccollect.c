/*
 * gccollect.c
 *
 *  Created on: 2019å¹?æœ?3æ—?
 *      Author: admin
 */
/* get sp for gc_collect_root
 * *
 * */
#include <stdlib.h>
#include <stdio.h>
#include "py/mpstate.h"
#include "py/gc.h"
#include "usr_misc.h"
#include <os_memory.h>
#include <os_task.h>

#if defined(__CC_ARM)
extern void push_common_registers(void);
extern void pop_common_registers(void);
#endif
void gc_collect(void) 
{
    gc_dump_info();
    gc_collect_start();
    
#if defined(__CC_ARM)
    push_common_registers();
#endif
    gc_collect_root((void**)TASK_SP_REGS, (size_t)(((mp_uint_t)MP_STATE_THREAD(stack_top) - (mp_uint_t)TASK_SP_REGS) / 4));
#if defined(__CC_ARM)
    pop_common_registers();
#endif

#if MICROPY_PY_THREAD
    // trace root pointers from any threads
    mp_thread_gc_others();
#endif

    gc_collect_end();
    gc_dump_info();
}

/**
*********************************************************************************************************
* @brief                                 malloc heap memory for micropython 
*
* @description	This function malloc memory as heap for micropython.
*
* @param	  	size_or_addr : The size of memory need to allocate or the ram address for mapping.
*
* @param		flag		 : the type of malloc memory
*								MP_HEAP_ALLOC: malloc memory from system memory management.  
*								MP_HEAP_RAM_ADDR: Direct memory mapping form ram
*
* @return	  :	memory address  
*********************************************************************************************************
*/
void * mp_heap_malloc(mp_size_t size_or_addr, mp_heap_flag_t flag)
{
	if (flag == MP_HEAP_ALLOC){
		return usr_malloc(size_or_addr);
	} 
	return (void *)size_or_addr;
}

/**
*********************************************************************************************************
* @brief                                 free micropython heap memory
*
* @description This function free micropython heap memory.
*
* @param	   addr	: The address of memory need to free only if the flag equal to MP_HEAP_ALLOC.
*
* @param	   flag	: The way of malloc memory, this is the same as memory allocation. 
*					  MP_HEAP_ALLOC: malloc memory from system memory management. 
*					  MP_HEAP_RAM_ADDR: Direct memory mapping form ram.
*
* @return	  :	none
*********************************************************************************************************
*/
void mp_heap_free(void * addr, mp_heap_flag_t flag)
{
	if (flag == MP_HEAP_ALLOC){
		free(addr);
	} 
	return ;
}
