#include "state_machine.h"




/**
 *********************************************************************************************************
 *                                      state machine data structure
 *********************************************************************************************************
*/
typedef enum state_{
    CMD_NULL,
    CMD_1,
    CMD_2,
    CMD_3,
    CMD_4,
    CMD_5
}Mstate_t;


typedef enum result_state{
    RESULT_CHECK = 0,
    RESULT_NO_CHECK
}result_s;



uint8_t g_last_id= 0;
uint8_t g_statemachine_type = SM_FUNCTIONNUM;
jos_list_t G_sm_prior_list[SM_MAX_PRIORITY]; // SM_MAX_PRIORITY 个优先级链表




/**
 *********************************************************************************************************
 *                                      state machine function prototype
 *********************************************************************************************************
*/


/**
 *********************************************************************************************************
 *                                      find the location of the task
 *
 * @description: This function will find the location of the task.
 *
 * @param      : list:  Linked list to be looked up   
 *               node:  Node to be found
 *
 * @returns    : Find the previous node of the node to be found
 *********************************************************************************************************
*/
static jos_list_t * find_self_site(jos_list_t * list, jos_list_t * node)
{
    jos_list_t * temp = list->next;
    if (mpy_os_cycle_list_is_end(list, list)) {
        return list;
    }
	
    while(MEMBER(temp) < MEMBER(node)){
        if (mpy_os_cycle_list_is_end(list, temp)){
            return temp;
        }
        temp = temp->next;
    }
    return temp;
}
#if 0
/**
 *********************************************************************************************************
 *                                      获取尾节点
 *
 * @description: 这个函数用来获取链表的尾节点。
 *
 * @param      : list:  链表头节点    
 *
 * @returns    : 尾节点
 *********************************************************************************************************
*/
static jos_list_t * get_list_tail(jos_list_t * list)
{
    jos_list_t * temp = list;
    while(!mpy_os_list_is_end(list)) temp = temp->next;
    return temp;
}

/**
 *********************************************************************************************************
 *                                      分割链表
 *
 * @description: 这个函数用来分割需要比较的链表。
 *
 * @param      : low:  小节点    
 *               high: 大节点
 *
 * @returns    : 基准位置
 *********************************************************************************************************
*/
static jos_list_t * list_partition(jos_list_t * low, jos_list_t *high)
{
    jos_list_t *pivotNode = low;  // 基准节点

    //从表的两端交替地向中间扫描
    while(MEMBER(low) < MEMBER(high)){
        //从high 节点向前搜索，至多到low+1 节点位置。将比基准节点小的交换到低端 
        while (MEMBER(high) >= MEMBER(pivotNode)){
            high = high->prev;
        }
        while (MEMBER(low) <= MEMBER(pivotNode)){
            low = low->next;
        }
        os_list_swap(low, high);
    }
    os_list_swap(low, pivotNode);

    return low;
}

/**
 *********************************************************************************************************
 *                                      快排序
 *
 * @description: 这个函数用来对链表进行快排序。
 *               1. 选择一个基准元素,通常选择第一个元素或者最后一个元素。
 *               2. 通过一趟排序讲待排序的记录分割成独立的两部分，其中一部分记录的元素值均比基准元素值小。
 *                  另一部分记录的 元素值比基准值大。
 *               3. 此时基准元素在其排好序后的正确位置
 *               4. 然后分别对这两部分记录用同样的方法继续进行排序，直到整个序列有序。
 *
 * @param      : low:  小节点    
 *               high: 大节点
 *
 * @returns    : 基准位置
 *********************************************************************************************************
*/
static bool_t quick_sort(jos_list_t * low, jos_list_t *high)
{
    jos_list_t *pivotNode = NULL;
    if (MEMBER(low) < MEMBER(high)){
        pivotNode  = list_partition(low, high);  //将表一分为二  
        quick_sort(low, pivotNode->prev);        //递归对低子表递归排序  
		quick_sort(pivotNode->next, high);       //递归对高子表递归排序  
    } 
    return true;
}

/**
 *********************************************************************************************************
 *                                      快排序
 *
 * @description: 这个函数用来对链表进行快排序。
 *
 * @param      : head:  需要排序的链表    
 *
 * @returns    : 操作结果，真或假
 *********************************************************************************************************
*/
bool_t quickSort(jos_list_t * head)
{
	if (head == NULL){
        return false;
    }
		
	jos_list_t *low = head;
	jos_list_t *high = get_list_tail(head);
    
	quick_sort(low,high);
	return true;
}
#endif  
/**
 *********************************************************************************************************
 *                                      check task ID
 *
 * @description: This function check task ID is exist.
 *
 * @param      : id : task id
 *
 * @returns    : none
 *********************************************************************************************************
*/
bool_t check_id(uint8_t id)
{
    state_machine_t *pos, *m;
    for(int i = 0; i < SM_MAX_PRIORITY; i++){
        mpy_list_for_each_entry_safe(pos, m, &G_sm_prior_list[i], list){
            if (pos->id == id){
                return ams_false; 
            }
        }
    }
    return ams_true;
}

/**
 *********************************************************************************************************
 *                                      set task ID
 *
 * @description: This function create a task id.
 *
 * @param      : none
 *
 * @returns    : none
 *********************************************************************************************************
*/
uint8_t sm_creat_id(void)
{
    uint8_t id = ++g_last_id;
    if (check_id(id)){
        return id;
    } 
    return ams_false;
}

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
state_machine_t *sm_create_task(uint16_t priority, uint8_t index, uint8_t type, void *sm_node)
{
    int size = sizeof(state_machine_t); 
    state_machine_t *node = (state_machine_t *)sm_calloc(1,size);
    if (node == SM_NULL || (type == SM_LIST_TASK && sm_node == SM_NULL)) {
        return SM_NULL;
    }
	sm_memset(node,0, size);
    node->priority = priority;
    node->index = index;
    
	SET_SM_TYPE(node, g_statemachine_type);
    SET_TASK_TYPE(node, type);
	if (type == SM_LIST_TASK){
		CLEAN_LIST_TASK_FLAG(node);
		if (sm_node == SM_NULL){
			sm_err("the parameter is wrong! \n");
			goto FAILED_CREATE;
		}
	}
	
	node->reserved = (sm_node == SM_SELF_NODE)? node : sm_node;
	if (GET_SM_TYPE(node)){			// state machine ?  
		node->id = sm_creat_id(); 	// yes, set id. if not set it at [sm_response_func_register] function
		if (!node->id){
			sm_err("failed to create state machine node id! \n");
			goto FAILED_CREATE;
		}
    }
	//sm_log("[sm_create_task] [addr:%p| id: %d]\n", node, node->id);
    return node;

FAILED_CREATE:
	sm_free(node);
	return SM_NULL;
}


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
bool_t sm_response_func_register(state_machine_t *sm_node,  uint16_t fun_num, fun_t func)
{
    jos_list_t *site = NULL;
    if (sm_node == NULL || sm_node->priority >= SM_MAX_PRIORITY){
        return ams_false;
    }
    if (GET_SM_TYPE(sm_node)){
        sm_node->state = fun_num;
    } else {
        sm_node->id = fun_num;
    }
    sm_node->func = func;
    SET_RUN_FLAG(sm_node);
    site = find_self_site(&G_sm_prior_list[sm_node->priority], &sm_node->list);
    if (site == NULL){
        return ams_false;
    }
    mpy_os_list_add_next(&sm_node->list, site);
    return ams_true;
}

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
bool_t sm_set_priority(state_machine_t *sm_node, uint16_t priority)
{
    jos_list_t *site = NULL;
    if (sm_node == NULL){
        return ams_false;
    }
    mpy_os_list_del_init(&sm_node->list);
    sm_node->priority = priority;
    site = find_self_site(&G_sm_prior_list[sm_node->priority], &sm_node->list);
    mpy_os_list_add_next(&G_sm_prior_list[sm_node->priority], site);
    return ams_true;
}

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
int sm_system_init(void)
{
    for (int i = 0; i < SM_MAX_PRIORITY; i++){
        mpy_os_list_init(&G_sm_prior_list[i]);
	}
	return 0;
}



/**
*********************************************************************************************************
*                                      list all registered task id
*
* @Description: This function list all registered task id
*
* @Returns    : 0
*********************************************************************************************************
*/
uint32_t sm_listall(void)
{
	state_machine_t *pos, *m;
	sm_log("task list:\n");
	for (int i = 0; i < SM_MAX_PRIORITY; i++){
		mpy_list_for_each_entry_safe(pos, m, &G_sm_prior_list[i], list){
			sm_log("task (id:0x%x | priority:%d | index:%d | addr:%p)\n", pos->id, i, pos->index, pos);
		}
	}
  return 0;
}

/**
 *********************************************************************************************************
 *                                      find task
 *
 * @description: This function find task.
 *
 * @param      : id : task id
 *
 * @returns    : task node or null
 *********************************************************************************************************
*/
state_machine_t *find_task(uint16_t id)
{
    state_machine_t *pos, *m;
    for (int i = 0; i < SM_MAX_PRIORITY; i++)
    {
        mpy_list_for_each_entry_safe(pos, m, &G_sm_prior_list[i], list) 
        {
        //sm_log("[find_task]: index:%d ; id:%d ; pos->id:%d!!!\n",i, id, pos->id);
            if (pos->id == id)
            {
                return pos;
            }
        }
    }
    return NULL;
}

/**
 *********************************************************************************************************
 *                                      run task
 *
 * @description: This function run list task.
 *
 * @param      : task : list task.
 *
 * @returns    : Successfully, retrun null, if not return self node,(restart list task, return the 
 *               first list task node)
 *********************************************************************************************************
*/
static state_machine_t *run_task(state_machine_t *task)
{
    sm_log("(id : %d | index : %d)\n", task->id, task->index);
    if (GET_LIST_TASK_FLAG(task))
    {  //restart list tasks
        CLEAN_LIST_TASK_FLAG(task);
        return task->reserved;
    }
    
    task->result = task->func(task);
    if ((task->index != 0) && (task->result != SM_EOK))
    { // run failed, retry!
        task->counter ++;
        if (task->counter >= MAX_RETRY_CALL_FUNC)
        {
            sm_err("Excessive repetition! restart list tasks!\n");

            SET_LIST_TASK_FLAG(task);
            task->counter = 0;
            sm_mdelay(5);
        } 
        else 
        {
            CLEAN_LIST_TASK_FLAG(task);
            sm_mdelay(10);
        }
        return task;    
    } 
    else 
    {
        task->counter = 0;
        CLEAN_RUN_FLAG(task); //delete for function can re-entry
    }
    return NULL;
}


/**
 *********************************************************************************************************
 *                                      run sequence task
 *
 * @description: The funtion run list task.
 *
 * @param      : task : list task.
 *
 * @returns    : none
 *********************************************************************************************************
*/
static bool_t list_task_run(state_machine_t *task)
{
    state_machine_t *m, *pos = task;
	
start:
	pos = run_task(pos);
	if (pos != NULL){ // run failed, restart list tasks or itself.
		goto start;
	}
	
    mpy_list_for_each_entry_safe(pos, m, &task->list, list){
		
		pos = run_task(pos);
		if (pos != NULL){ // run failed, restart list tasks.
			goto start;
		}
		if (mpy_os_cycle_list_is_end(&task->list, &m->list)){
			break;
		}
    }
        
    return ams_true;
}

/**
 *********************************************************************************************************
 *                                      run normal task
 *
 * @description: This function run normal task.
 *
 * @param      : task : normal task
 *
 * @returns    : none
 *********************************************************************************************************
*/
static bool_t normal_task_run(state_machine_t *task)
{
    if (GET_RUN_FLAG(task)){ 
        task->result = task->func(task);

        //CLEAR_RUN_FLAG(task);///delete for function can re-entry
        return ams_true;
    }

    return ams_false;
}

/**
 *********************************************************************************************************
 *                                      state machine running 
 *
 * @description: This function run state machine.
 *
 * @param      : sm_type		:	the type of state machine 
 *				 recv_msg_fun	:	receive message (function number) interface
 *
 * @returns    : none
 *********************************************************************************************************
*/
int sm_system_run(int sm_type, fun_i_0_t recv_msg_fun)
{
    state_machine_t *node;
	int fun_num = -1;    
    
	g_statemachine_type = sm_type;
    while(1)
    {		
		
		fun_num = recv_msg_fun();
		if(fun_num == SM_ERROR){
			sm_mdelay(5);
			continue;
		}

		sm_log("function number:%d!!!\n", fun_num);
 
		node = find_task(fun_num);
		if (!node) {
			sm_mdelay(5);
			continue;
		}

		if (GET_TASK_TYPE(node)){
			sm_log("list_task_run !!!\n");
			list_task_run(node);
		} else {
			sm_log("normal_task_run !!!\n");
			normal_task_run(node);
		}
		
		sm_mdelay(5);
    }
}

