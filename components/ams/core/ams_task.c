#include <string.h>
#include <os_memory.h>
#include <os_task.h>
#include <os_list.h>
#include "ams_task.h"

/* sth about the ams task, like app running machine et. */

ams_task_list_t g_ams_task_list[AMS_TASK_PRIORITY_MAX];
task_send_fun_t g_ams_send_result = NULL;

static void ams_register_task(ams_task_t *task)
{
    ams_task_list_add_tail(&g_ams_task_list[task->priority], &task->list);
    return;
}

static int ams_add_task(uint8_t periority, uint8_t task_id, task_fun_t func)
{
    ams_task_t *task = NULL;
    
    task = (ams_task_t *)ams_calloc(1, sizeof(ams_task_t));
    if (task == NULL)
    {
        ams_err("No memory for task[periority: %d, id: %d], size[%d].", periority, task_id, sizeof(ams_task_t));
        return AMS_ERROR;
    }
    memset(task, 0, sizeof(ams_task_t));

    task->priority = periority;
    task->id = task_id;
    task->func = func;
    task->status = AMS_TASK_STATUS_IDLE;

    ams_register_task(task);
    return AMS_OK;
}

int ams_add_system_task(uint8_t task_id, task_fun_t func)
{
    return ams_add_task(AMS_TASK_PRIORITY_HIGH, task_id, func);
}

int ams_add_normal_task(uint8_t task_id, task_fun_t func)
{
    return ams_add_task(AMS_TASK_PRIORITY_NORMAL, task_id, func);
}

void ams_init_task_list(void)
{
    int i = 0;
    
    for (i = 0; i < AMS_TASK_PRIORITY_MAX; i++)
    {
        g_ams_task_list[i].next = g_ams_task_list[i].prev = &g_ams_task_list[i];
    }
    
    return;
}

static ams_task_t *ams_find_task(uint16_t task_id)
{
    ams_task_t *pos, *m;
    int i = 0;
    
    for (i = 0; i < AMS_TASK_PRIORITY_MAX; i++)
    {
        os_list_for_each_entry_safe(pos, m, &g_ams_task_list[i], ams_task_t, list) 
        {
            //ams_log("[find_task]: list_id: %d, task_id: %d, pos->id: %d\n", i, task_id, pos->id);
            if (pos->id == task_id)
            {
                return pos;
            }
        }
    }
    return NULL;
}

static void ams_run_system_task(ams_task_t *task, void *param)
{
    int i = 0;
    
    if (task->status != AMS_TASK_STATUS_IDLE)
    {
        ams_err("System task[id: %d] is not idle.", task->id);
        return;
    }
    AMS_TASK_RUN(task->status);
    task->failed_times = 0;
    task->result.res_type = task->func(param, NULL);
    if (task->result.res_type != AMS_OK)
    {
        ams_log("Try system task[id: %d] again.", task->id);
        for (i = 0; i < AMS_MAX_RETRY_CALL_FUNC; i++)
        {
            task->result.res_type = task->func(param, NULL);
            if (task->result.res_type == AMS_OK)
            {
                AMS_TASK_STOP(task->status);
                return;
            }
            task->failed_times = i;
            ams_log("Try system task[id: %d] again[times: %d].", task->id, task->failed_times);
            os_task_tsleep(100);
        }
        ams_err("Too many system task[id: %d] failures.", task->id);
    }
    AMS_TASK_STOP(task->status);
    return;
}

static void ams_run_normal_task(ams_task_t *task, void *param)
{
    /* no re-try */
    memset(&task->result, 0, sizeof(task->result));
    (void)task->func(param, (void *)(&task->result));
    if (task->result.uni_id != NULL)
    {
        /* send result */
        g_ams_send_result(&task->result);
    }
    ams_log("Run normal task[id: %d, result: %d] over.", task->id, task->result.res_type);
    return;
}

void ams_gister_send_func(task_send_fun_t func)
{
    g_ams_send_result = func;
    return;
}

int ams_run_task_list(recv_msg_func_t recv_fun)
{
    ams_task_t *task = NULL;
    ams_task_param_t param = {0};
    int ret = AMS_ERROR;
    
    while(1)
    {
        if (param.param != NULL)
        {
            ams_log("Free[%p].", param.param);
            ams_free(param.param);
        }
        
        memset(&param, 0, sizeof(ams_task_param_t));
        ret = recv_fun(&param);
        if(ret == AMS_ERROR)
        {
            os_task_tsleep(5);
            continue;
        }

        ams_log("start find task: %d.", param.id);

        task = ams_find_task(param.id);
        if (NULL == task) 
        {
            ams_log("Find task: %d failed.", param.id);
            os_task_tsleep(5);
            continue;
        }
        
        if (task->priority == AMS_TASK_PRIORITY_HIGH)
        {
            ams_log("It is a system task.\n");
            ams_run_system_task(task, param.param);
        } 
        else 
        {
            ams_log("It is a normal task.\n");
            ams_run_normal_task(task, param.param);
        }
        
        os_task_tsleep(5);
    }
}

void ams_show_taskinfo(void)
{
#ifdef AMS_USING_PLATFORM
    int i = 0;
    ams_task_t *pos, *m;
    for (i = 0; i < AMS_TASK_PRIORITY_MAX; i++)
    {
        os_list_for_each_entry_safe(pos, m, &g_ams_task_list[i], ams_task_t, list) 
        {
            //ams_log("[find_task]: list_id: %d, task_id: %d, pos->id: %d\n", i, task_id, pos->id);
            os_kprintf("task[id: %d, priority: %d, status: %d, current_result: %d, failed_times: %d]\r\n", 
                    pos->id, pos->priority, pos->status, pos->result, pos->failed_times);
        }
    }
#endif
}

