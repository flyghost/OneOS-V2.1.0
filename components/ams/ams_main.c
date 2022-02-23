/* main thread */
#include <unistd.h>
#include "core/ams_core.h"
#include "core/ams_task.h"
#include "ops/ams_file.h"
#include "ops/ams_net.h"
#include "ops/ams_protocol.h"

static int                  g_ams_reconnect = 0;
static char                 g_ams_running_app_name[AMS_APP_NAME_LEN] = {0};
static char                 g_ams_task_file_path[128] = {0};

#define SET_TASK_RESULT(result, unique_id, resp, resp_buf)                 \
    do {                                                        \
        ams_task_result_t *task_res = (void *)result;           \
        if (task_res != NULL)                                   \
        {                                                       \
            task_res->uni_id = unique_id;                       \
            task_res->res_type = resp;                          \
            task_res->res_buf = resp_buf;                       \
        }                                                       \
    } while(0)

/**
 *********************************************************************************************************
 *                            [thread area] create and run thread
 *********************************************************************************************************
*/

static int ams_run_script(char *file_path)
{
    int ret = AMS_ERROR;
    char *fname = file_path;
    int fname_len = strlen(file_path) + 1;
    
    memset(g_ams_task_file_path, 0, sizeof(g_ams_task_file_path));
    strcpy(g_ams_task_file_path, file_path); /* need put heap or global mem into task */
    ams_log("Start a new thread to run script[%s].", g_ams_task_file_path);
    ret = ams_startup_thread(AMS_THD_RUNNER, ams_run_file_thd_entry, (void *)g_ams_task_file_path);
    ams_log("Create ams APP runner thread %s.", (ret == AMS_ERROR) ? "failed" : "success");
    if (ret == AMS_OK)
    {
        memset(g_ams_running_app_name, 0, AMS_APP_NAME_LEN);
        if (*fname == '/')
        {
            fname += 1;
        }
        strncpy(g_ams_running_app_name, fname, AMS_APP_NAME_LEN - 1);
    }
    return ret;
}

static int ams_send_device_info(int sock)
{
    int ret = AMS_ERROR;
    char cmd[AMS_ENCODE_BUFF_LEN] = {0};
    
    /* package device info */
    ams_encode_device_info(g_ams_reconnect, cmd);
    g_ams_reconnect++;
    ams_log("Encode cmd is[%s]", cmd);
    
    /* send info */
    ret = ams_send(sock, cmd, strlen(cmd));
    if (ret < 0)
    {
        ams_err("Send cmd[%s] failed[%d].", cmd, ret);
        return AMS_ERROR;
    }

    return AMS_OK;
}

static void ams_send_cmd_result(void *result)
{
    ams_task_result_t *task_res = (ams_task_result_t *)result;
    char cmd_result[AMS_ENCODE_BUFF_LEN] = {0};
    int main_sock = -1;

    main_sock = ams_get_main_socket();
    if (main_sock < -1)
    {
        ams_err("Socket it not create.");
        return;
    }

    ams_log("Ams [unique id: %s, protocol type: %d]", task_res->uni_id, task_res->res_type);
    ams_encode_response_result(task_res->uni_id, task_res->res_type, task_res->res_buf, cmd_result);
    ams_log("Send cmd[%s]", cmd_result);
    if (ams_send(main_sock, cmd_result, strlen(cmd_result)) < 0)
    {
        ams_err("Send[%s] error, close the main socket[%d].", cmd_result, main_sock);
    }

    return;
}

static int ams_register_device(void *param, void *result)
{
    int ret = AMS_ERROR;
    int main_sock = -1;
    
    /* create socket */
    ret = ams_connet_platform(&main_sock, (15 * AMS_RECV_MSG_TIMEOUT_ONE_MINUTE), NULL, 0);
    if (ret == AMS_ERROR)
    {
        return AMS_ERROR;
    }
    
    /* send info */
    ret = ams_send_device_info(main_sock);
    if (ret == AMS_ERROR)
    {
        ams_err("Send device information to platform failed.");
        ams_close_socket(main_sock);
        main_sock = -1;
    }
    ams_set_main_socket(main_sock);
    return ret;
}

static int ams_download_app(void *param, void *result)
{
    int ret = AMS_ERROR;
    int resp = AMS_PROTOCOL_RESP_INSTALLOK;
    int down_sock = -1;
    char file_name[128] = {0};
    ams_download_app_param_t *cmd_param = (ams_download_app_param_t *)param;

    if (cmd_param == NULL)
    {
        ams_err("Internal error, no parameters of the download command.");
        resp = AMS_PROTOCOL_FAIL_DOWNLOAD;
        goto __exit;
    }

    /* 1.create a socket to receive file */
    ret = ams_connet_platform(&down_sock, 1, cmd_param->saddr, cmd_param->port);
    if (ret != AMS_OK)
    {
        ams_err("Connect socket for download app failed.");
        resp = AMS_PROTOCOL_FAIL_DOWNLOAD;
        goto __exit;
    }
    if (ams_send(down_sock, cmd_param->payload, strlen(cmd_param->payload)) <= 0) 
    {
        ams_err("send error, close the socket.");
        resp = AMS_PROTOCOL_FAIL_DOWNLOAD;
        goto __exit;
    }
    
    (ams_is_single_file(cmd_param->main_class) ? 
        sprintf(file_name , "/%s%s", cmd_param->app_name, ams_get_file_ext_name()) : 
        sprintf(file_name , "/%s%s" , cmd_param->app_name, ".jz"));

    ret = ams_save_app_file(down_sock, cmd_param->app_len, file_name);
    if (ret != AMS_OK)
    {
        ams_err("Save file failed when download app.");
        resp = AMS_PROTOCOL_FAIL_DOWNLOAD;
        goto __exit;
    }
    /* 2.if not a signle file, unpack them and get the main file name. */
    if (!ams_is_single_file(cmd_param->main_class))
    {
        if (ams_unpack_app(cmd_param->app_name) == AMS_ERROR)
        {
            ams_err("Failed to unpack app !!!");
            resp = AMS_PROTOCOL_FAIL_DOWNLOAD;
            goto __exit;
        }
        memset(file_name, 0, sizeof(file_name));
        sprintf(file_name , "/%s/%s", cmd_param->app_name, cmd_param->main_class);
    }
    
    if (cmd_param->auto_start)
    {
        if (ams_set_auto_app_path(cmd_param->app_name, cmd_param->main_class) == AMS_ERROR)
        {   
            ams_err("Failed to set auto file !!!");
            resp = AMS_PROTOCOL_FAIL_DOWNLOAD;
            goto __exit;
        }
    }
    
    /* 5.return the last result of app running. */
    if (cmd_param->start_now)
    {
        ret = ams_run_script(file_name);
        ams_task_result_t start_result = {0};
        start_result.uni_id = cmd_param->unique_id;
        start_result.res_type = (ret == AMS_OK) ? AMS_PROTOCOL_RESP_APPSTARTOK : AMS_PROTOCOL_APPSTARTERROR;;
        ams_send_cmd_result(&start_result);
    }
    
__exit:
    SET_TASK_RESULT(result, cmd_param->unique_id, resp, NULL);
    if (down_sock >= 0)
    {
        ams_close_socket(down_sock);
    }

    return ret;
}

int ams_start_app(void *param, void *result)
{
    int resp = AMS_PROTOCOL_RESP_APPSTARTOK;
    char file_name[128] = {0};
    ams_start_app_param_t *cmd_param = (ams_start_app_param_t *)param;

    if (cmd_param == NULL)
    {
        ams_err("Internal error, no parameters of the start command.");
        resp = AMS_PROTOCOL_APPSTARTERROR;
        goto __exit;
    }

    if(ams_is_single_file(cmd_param->main_class))
    {
        sprintf(file_name , "/%s%s", cmd_param->app_name, ams_get_file_ext_name());
    }
    else
    {
        sprintf(file_name , "/%s/%s" , cmd_param->app_name, cmd_param->main_class);
    }
    
    ams_log("Begin start app[%s].", file_name);
    if (ams_check_file_exist(file_name) == AMS_ERROR) 
    {
        ams_err("App file[%s] is not exist.", file_name);
        resp = AMS_PROTOCOL_APPNOTEXIST;
        goto __exit;
    }
    
    if (ams_run_script(file_name) != AMS_OK) 
    {
        ams_err("App[%s] running error.", file_name);
        resp = AMS_PROTOCOL_APPSTARTERROR;
    }
__exit:
    SET_TASK_RESULT(result, cmd_param->unique_id, resp, NULL);
    return ((resp == AMS_PROTOCOL_APPSTARTERROR) ? AMS_ERROR : AMS_OK);
}

int ams_list_app(void *param, void *result)
{
    char *file_list = NULL;
    const char *unique_id = (const char *)param;
        
    if (unique_id == NULL)
    {
        ams_err("Internal error, no parameters of the start command.");
        return AMS_ERROR;
    }

    file_list = ams_scan_files();
    if ((file_list == NULL) || (strlen(file_list) == 0))
    {
        SET_TASK_RESULT(result, unique_id, AMS_PROTOCOL_RESP_APPLIST, "<No applications found>");
    }
    else
    {
        SET_TASK_RESULT(result, unique_id, AMS_PROTOCOL_RESP_APPLIST, file_list);
    }
    ams_log("Finished report-list-command[%s] !", file_list);
    return AMS_OK;
}

static int ams_stop_app(void *param, void *result)
{
    char *unique_id = (char *)param;
    int ret = AMS_ERROR;
    
    if (unique_id == NULL)
    {
        ams_err("Internal error, no parameters of the stop command.");
        return AMS_ERROR;
    }

    ret = ams_stop_thread(AMS_THD_RUNNER);
    if (ret != AMS_OK)
    {
        ams_err("Internal error, stop thread failed.");
        return AMS_ERROR;
    }
    memset(g_ams_running_app_name, 0, sizeof(g_ams_running_app_name));
    SET_TASK_RESULT(result, unique_id, AMS_PROTOCOL_RESP_APPFINISH, "0");
    return AMS_OK;
}

static int ams_resp_heartbeat(void *param, void *result)
{
    const char *unique_id = (const char *)param;
    
    if (unique_id == NULL)
    {
        ams_err("Internal error, no parameters of the heartbeat command.");
        return AMS_ERROR;
    }

    SET_TASK_RESULT(result, unique_id, AMS_PROTOCOL_RESP_HEARTBEAT, NULL);
    return AMS_OK;
}

int ams_delete_app(void *param, void *result)
{
    char tmp[40] = {0};
    int ret = AMS_ERROR;
    int resp = AMS_ERROR;
    ams_delete_app_param_t *cmd_param = (ams_delete_app_param_t *)param;

    if (cmd_param == NULL)
    {
        ams_err("Internal error, no parameters of the delete command.");
        return AMS_ERROR;
    }
    
    sprintf(tmp, "/%s.jz", cmd_param->app_name);
    if(ams_check_file_exist(tmp) == AMS_OK) 
    {
        //1.delete sub directory
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%s", cmd_param->app_name);
        if(ams_check_file_exist(tmp) == AMS_OK)
        {
            ams_delete_directory(tmp);
        }
        //2.delete jz file
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%s.jz", cmd_param->app_name);
    } 
    else 
    {
        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "%s%s", cmd_param->app_name, ams_get_file_ext_name());
    }

    if(ams_check_file_exist(tmp) != AMS_OK)
    {
        ams_log("App is not exist.");
        resp = AMS_PROTOCOL_APPNOTEXIST;
        goto  __exit;
    }
    
    ret = unlink(tmp) ;
    ((ret == AMS_OK) ? (resp = AMS_PROTOCOL_RESP_DELETEOK) : (resp = AMS_PROTOCOL_DELETEFAIL));
    ams_log("Finished[%d] deleted-app-command.", ret);

    /* 检查auto file中是否是要删除的文件，如果是，则要删除auto file     */
    ams_del_auto_app_path(cmd_param->app_name);
__exit:
    SET_TASK_RESULT(result, cmd_param->unique_id, resp, NULL);
    return ret;
}

static int ams_report_app(void *param, void *result)
{
    char *unique_id = (char *)param;

    if (ams_get_vm_status == AMS_OK)
    {
        memset(g_ams_running_app_name, 0, sizeof(g_ams_running_app_name));
    }
    
    if (strlen(g_ams_running_app_name) != 0)
    {
        SET_TASK_RESULT(result, unique_id, AMS_PROTOCOL_RESP_RUNNINGAPPLIST, g_ams_running_app_name);
    }
    else
    {
        SET_TASK_RESULT(result, unique_id, AMS_PROTOCOL_RESP_RUNNINGAPPLIST, "EMPTY_LIST");
    }
    ams_log("Finished report-running-app-command!"); 
    return AMS_OK;
}

static int ams_reset(void *param, void *result)
{
    ams_log("Reboot.");
    ams_reset_board();
    return AMS_OK;
}

static int ams_recv_msg(ams_task_param_t *recv_res)
{
    char single_msg;
    int off = 0;
    char msg_buf[AMS_RECV_MSG_MAX_LEN] = {0};
    int main_sock = 0;

    main_sock = ams_get_main_socket();
    if (main_sock == -1)
    {   /* connection hasnot been established yet. */
        recv_res->id = AMS_SYSTEM_TASK_REGISTER;
        return AMS_OK;
    }
    
    while (off < AMS_RECV_MSG_MAX_LEN)
    {
        if (ams_recv(main_sock, &single_msg, 1) != 1)
        {   /* recv timeout, reconnected again. */
            ams_close_socket(main_sock);
            ams_set_main_socket(-1);
            recv_res->id = AMS_SYSTEM_TASK_REGISTER;
            return AMS_OK;
        }

        if (single_msg == 0x0a)
        {
            if (off < 35)
            {   /* smaller than 35, is an unvalable command.*/
                memset(msg_buf, 0, off);
                off = 0;
                continue;
            }
            break;
        }

        msg_buf[off] = single_msg;
        off++;
    }
    ams_log("Recv end, msg len[%d].", off);

    return ams_parse_msg(msg_buf, recv_res);
}

void ams_start_task_list(void)
{
    ams_init_task_list();

    ams_log("start");
    
    (void)ams_add_system_task(AMS_SYSTEM_TASK_REGISTER, ams_register_device);

    (void)ams_add_normal_task(AMS_NORMAL_TASK_DOWNLOAD_APP, ams_download_app);

    (void)ams_add_normal_task(AMS_NORMAL_TASK_START_APP, ams_start_app);

    (void)ams_add_normal_task(AMS_NORMAL_TASK_LIST_APP, ams_list_app);

    (void)ams_add_normal_task(AMS_NORMAL_TASK_STOP_APP, ams_stop_app);
    
    (void)ams_add_normal_task(AMS_NORMAL_TASK_HEARTBEAT, ams_resp_heartbeat);
    
    (void)ams_add_normal_task(AMS_NORMAL_TASK_DELETE_APP, ams_delete_app);

    (void)ams_add_normal_task(AMS_NORMAL_TASK_REPORT_APP, ams_report_app);

    (void)ams_add_normal_task(AMS_NORMAL_TASK_RESET_SYS, ams_reset);
    
    ams_gister_send_func(ams_send_cmd_result);
    
    ams_run_task_list(ams_recv_msg);
    
}

static void ams_main_entry(void *parameter)
{
    char script_name[AMS_APP_NAME_LEN + AMS_MAIN_CLASS_LEN + 1] = {0};
    
    /* todo: some initialize work */
    
    if (ams_get_auto_app_path(script_name) == AMS_OK)
    {
        ams_run_script(script_name);
    }
#ifdef AMS_USING_PLATFORM
    ams_start_task_list();
#endif 
    return;
}

int ams_start(void)
{ 
    int ret = AMS_ERROR;
#if 0
    /* mount file system */
    ret = ams_mount_file_system(AMS_FS_DEVICE_NAME);
    if (ret == AMS_ERROR)
    {
        ams_err("Mount file system failed.");
        return ret;
    }
#endif
    ret = ams_startup_thread(AMS_THD_MAIN, ams_main_entry, NULL);
    ams_log("Create ams main thread %s.", (ret == AMS_ERROR) ? "failed" : "success");
    
    return ret;
}
OS_APP_INIT(ams_start, OS_INIT_SUBLEVEL_LOW);



