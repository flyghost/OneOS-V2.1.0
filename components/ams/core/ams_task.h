#ifndef __AMS_TASK_H__
#define __AMS_TASK_H__
#include "core/ams_core.h"

typedef enum AMS_TASK_PRIORITY {
    AMS_TASK_PRIORITY_HIGH = 0,
    AMS_TASK_PRIORITY_NORMAL,
    AMS_TASK_PRIORITY_MAX,
} AMS_TASK_PRIORITY_T;

typedef enum AMS_TASK_ID {
    AMS_NORMAL_TASK_DOWNLOAD_APP = 0,
    AMS_NORMAL_TASK_START_APP,
    AMS_NORMAL_TASK_LIST_APP,
    AMS_NORMAL_TASK_STOP_APP,
    AMS_NORMAL_TASK_HEARTBEAT,
    AMS_NORMAL_TASK_DELETE_APP,
    AMS_NORMAL_TASK_REPORT_APP,
    AMS_NORMAL_TASK_RESET_SYS,
    AMS_NORMAL_TASK_MAX,
    AMS_SYSTEM_TASK_REGISTER,
} AMS_TASK_ID_T;


/*                     task list                      */
typedef struct ams_task_list_node {
    struct ams_task_list_node *prev;
    struct ams_task_list_node *next;
} ams_task_list_t;

static inline void ams_task_list_add_tail(ams_task_list_t *head, ams_task_list_t *entry)
{
    head->prev->next = entry;
    entry->prev = head->prev;
    head->prev = entry;
    entry->next = head;

    return;
}

typedef     int (* task_fun_t)(void *, void *);
typedef     void (* task_send_fun_t)(void *);
typedef enum AMS_TASK_STATUS_TYPE {
    AMS_TASK_STATUS_IDLE = 0,
    AMS_TASK_STATUS_BUSY,
    AMS_TASK_STATUS_ERROR,
} AMS_TASK_STATUS_TYPE_T;

#define AMS_TASK_RUN(stat)                        \
    do {                                                        \
        stat = AMS_TASK_STATUS_BUSY;                          \
    } while(0)

#define AMS_TASK_STOP(stat)                        \
    do {                                                        \
        stat = AMS_TASK_STATUS_IDLE;                          \
    } while(0)
    
#define AMS_MAX_RETRY_CALL_FUNC     10

typedef struct ams_task_result {
    const char *uni_id;       //处理命令后，需要带上unique id表明是哪个命令
    int32_t res_type;   //处理命令的结果
    char *res_buf;      //处理命令的结果需要带上的buf，如list，则需要带上文件列表
} ams_task_result_t;

typedef struct ams_task {
    ams_task_list_t     list;           // list member   
    uint8_t             id;             // task id
    uint8_t             failed_times;   // task处理失败次数
    uint8_t             priority;       // 优先级，用于处理不同序列任务
    uint8_t             status;         // 状态，当task处理空闲状态时才处理
    task_fun_t          func;           // task处理函数
    ams_task_result_t   result;         // task处理结果
    task_send_fun_t     send_func;      // task结果发送
} ams_task_t;

typedef struct ams_task_param {
    uint8_t     id;
    void        *param;
} ams_task_param_t;

typedef     int (* recv_msg_func_t)(ams_task_param_t *param);

void ams_start_task_list(void);

int ams_add_normal_task(uint8_t task_id, task_fun_t func);
int ams_add_system_task(uint8_t task_id, task_fun_t func);
void ams_init_task_list(void);
int ams_run_task_list(recv_msg_func_t recv_fun);
void ams_show_taskinfo(void);
void ams_gister_send_func(task_send_fun_t func);


/*                     task list                      */

#endif /* __AMS_TASK_H__ */

