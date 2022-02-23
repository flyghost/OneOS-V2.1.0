#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include <stdlib.h>
#include <stdint.h>

#include "model_list.h"
#include "model_log.h"
/**
 *********************************************************************************************************
 *                                       state machine data structure
 *********************************************************************************************************
*/
#define SM_EOK			0
#define SM_ERROR		-1

#define SM_NULL			((void *)0)
#define SM_SELF_NODE	((void *)1)


#define SM_HASH_MASK  	(0xff)

typedef unsigned long	sm_size_t;
typedef signed int		sm_err_t;
typedef mpy_os_list_t jos_list_t;

typedef enum bool_{
	ams_false,
	ams_true
}bool_t;

typedef enum sm_type{
    SM_FUNCTIONNUM = 0,
    SM_STATE
}sm_type_t;


typedef enum sm_task_type{
    SM_NORMAL_TASK = 0,
    SM_LIST_TASK
}sm_task_type_t;


typedef fun_i_1_t fun_t;


typedef struct state_machine_module{
	jos_list_t  list;      // list member   
    uint16_t    id;        // sequence task number(unique identifier),It could be function number
    uint8_t     index;     // sequence number. It is used to process sequential processes, Such as registration sequence.
						   //  The responses in the sequence flow are of the same priority.
    uint8_t     result;    // Result, Used for complete state detection
    uint16_t    flag;      /* flag, 
                             first bit ：the flag of task type.     0 is normal task; 1: sequence task
                             second bit：the flag of state machine. 0：function number - response; 1：status — response
                             third bit ：running flag
                             第四位：用于判断是否启用结果检测功能。异步处理时，需要使用此标志
                           */
    uint16_t    state;     // 状态机的状态
    uint8_t     priority;  // 优先级，用于处理不同序列任务
	uint8_t		counter;   // 计数器，记录错误次数
	uint16_t	other;	   // 预留
    uint32_t    *param;    // func的第一个参数 
    void        *reserved; // 预留， 可以用作 func的第二个参数
    fun_t       func;      // 响应函数
}state_machine_t;


#define SM_MAX_PRIORITY         4
#define MAX_RETRY_CALL_FUNC     10

/**
 *********************************************************************************************************
 *                                      state machine  macro function
 *********************************************************************************************************
*/
#define MEMBER(x) (list_entry(x, state_machine_t, list)->index)   // 获取以os_list_t为类型的成员list的结构体的index成员
#define MEMBER_X(x, a) (list_entry(x, state_machine_t, list)->a)   // 获取以os_list_t为类型的成员list的结构体的index成员

#define SET_TASK_TYPE(sm_node, t)  		((sm_node)->flag |= ((t) << 0))
#define GET_TASK_TYPE(sm_node)     		((sm_node)->flag & (1 << 0))
		
#define SET_SM_TYPE(sm_node, t)    		((sm_node)->flag |= ((t) << 1))
#define GET_SM_TYPE(sm_node)       		((sm_node)->flag & (1 << 1))
		
#define SET_RUN_FLAG(sm_node)      		((sm_node)->flag |= (1 << 2))		// set run-flag and function is no longer run
#define GET_RUN_FLAG(sm_node)      		((sm_node)->flag & (1 << 2))		// get run-flag
#define CLEAN_RUN_FLAG(sm_node)    		((sm_node)->flag &= (~(1 << 2)))	// clean run-flag and function can re-entry
		
#define SET_ASYNC_FLAG(sm_node)    		((sm_node)->flag |= (1 << 3))
#define GET_ASYNC_FLAG(sm_node)    		((sm_node)->flag & (1 << 3))
#define CLEAN_ASYNC_FLAG(sm_node)  		((sm_node)->flag &= (~(1 << 3)))

#define SET_LIST_TASK_FLAG(sm_node) 	((sm_node)->flag |= (1 << 4))		// set the bit, reexecute list tasks
#define GET_LIST_TASK_FLAG(sm_node) 	((sm_node)->flag & (1 << 4))		// get list_task_flag
#define CLEAN_LIST_TASK_FLAG(sm_node)  	((sm_node)->flag &= (~(1 << 4)))	// clean list_task_flag

/**
 *********************************************************************************************************
 *                                      state machine define log macro
 *********************************************************************************************************
*/


/**
 *********************************************************************************************************
 *                                      Inherited the log macro from  model-log  
 *********************************************************************************************************
*/
#define sm_log(msg, ...)	MODEL_LOG("[STATMEMACHINE]", msg, ##__VA_ARGS__)
#define sm_err(msg, ...)	MODEL_ERR("[STATMEMACHINE]", msg, ##__VA_ARGS__)

/**
 *********************************************************************************************************
 *                                      state machine interface statement
 *********************************************************************************************************
*/



/**
 *********************************************************************************************************
 *                                      create response list task
 *
 * @description: This function will create a response list task. The type is statemachine, set id, 
 *               if not (is command machine), when run regesiter function to set id (command number)   
 *
 * @param      : priority: the priority of response function
 *               index   : The index of response function
 *               type    : The type of task
 *				 sm_node : The first node of task list, Used when restarting sequence tasks.
 *
 * @returns    : return the task pointer or null
 *********************************************************************************************************
 */
state_machine_t *sm_create_task(uint16_t priority, uint8_t index, uint8_t type, void *sm_node);

/**
 *********************************************************************************************************
 *                                      Register response function
 *
 * @description: This function registers response function.
 *
 * @param      : sm_node : the node of statemachine
 *               fun_num : function noumber
 *               func    ：response function
 *
 * @returns    : Operation status。succeed: true; failed: false
 *********************************************************************************************************
*/
bool_t sm_response_func_register(state_machine_t *sm_node,  uint16_t fun_num, fun_t func);

/**
 *********************************************************************************************************
 *                                      set response level
 *
 * @description: This function set response function level.  Removes task node from current priority list, 
 * 				 then sets priority, and inserts it into the corresponding priority chain. 
 *
 * @param      : sm_node : statemachine node
 *               priority: response function level
 *
 * @returns    : Operation status。succeed: true; failed: false
 *********************************************************************************************************
*/
bool_t sm_set_priority(state_machine_t *sm_node, uint16_t priority);


/**
 *********************************************************************************************************
 *                                      Initialize state machine system 
 *
 * @description: This function initialize state machine system
 *
 * @param      : none
 *
 * @returns    : none
 *********************************************************************************************************
*/
int sm_system_init(void);


/**
 *********************************************************************************************************
 *                                      state machine running 
 *
 * @description: call the function to run state machine.
 *
 * @param      : sm_type		:	the type of state machine 
 *				 recv_msg_fun	:	receive message (function number) interface
 *
 * @returns    : none
 *********************************************************************************************************
*/
int sm_system_run(int sm_type, fun_i_0_t recv_msg_fun);

/**
*********************************************************************************************************
*                                      list all registered task id
*
* @Description: This function list all registered task id
*
* @Returns    : 0
*********************************************************************************************************
*/
uint32_t sm_listall(void);

/**
 *********************************************************************************************************
 *                                      state machine memory interface
 *********************************************************************************************************
*/
void sm_free(void *ptr);
void *sm_calloc(sm_size_t count, sm_size_t size);

void *sm_memset(void*s,int c,size_t n);

sm_err_t sm_mdelay(int32_t ms);

/**
 *********************************************************************************************************
 *                                      compute string hash
 *
 * @description: call the function to compute string hash
 *
 * @param      : data : source
 *
 *               len  : the length of data
 *
 * @returns    : hash
 *
* @note	   	   : djb2 algorithm; see http://www.cse.yorku.ca/~oz/hash.html
 *********************************************************************************************************
*/
uint32_t sm_compute_hash(const char *data, size_t len);


#endif
