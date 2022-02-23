#include "os_memory.h"
#include "core/ams_core.h"
#include "ams_protocol.h"

/* protocal between the platform and terminal, encoding and decoding */

void ams_encode_device_info(int reconnect_count, char *cmd)
{
    char tmp[32] = {0};
    char imei[AMS_IMEI_NAME_LEN + 1] = {0};
    char imsi[AMS_IMSI_NAME_LEN + 1] = {0};
    char iccid[AMS_ICCID_NAME_LEN + 1] = {0};
    
    strcat(cmd , "[REPORTC]"); 

    strcat(cmd , AMS_CODE_SEPARATOR);
    AMS_STORE_MSG(cmd, tmp, "%d", AMS_PROTOCOL_RESP_REGISTER);

    //add separtor
    strcat(cmd, AMS_CODE_SEPARATOR);
        
    //add cmd body
    strcat(cmd, "DEVICE_ID=");

    strcat(cmd, (ams_get_imei(imei, sizeof(imei)) != AMS_ERROR) ? imei :"000000000000000");  
    strcat(cmd, "_");
    
    strcat(cmd, (ams_get_imsi(imsi, sizeof(imsi)) != AMS_ERROR) ? imsi :"000000000000000");  
    strcat(cmd, "_");
    
    strcat(cmd, (ams_get_iccid(iccid, sizeof(iccid)) != AMS_ERROR)? iccid :"00000000000000000000");   

    AMS_STORE_MSG(cmd, tmp , ";CONN_COUNT=%d" , reconnect_count);

    //add line end
    strcat(cmd , AMS_LINE_END);

    return;
}


void ams_encode_response_result(const char *uni_id, int protocol, const char* val, char *result)
{
    char tmp[32] = {0};

    AMS_ADD_REPORT_MSG_HEAD(result, uni_id, "%d", protocol, tmp);

    if (val != NULL)
    {
        strcat(result, AMS_CODE_SEPARATOR);
        strcat(result, val);
    }

    strcat(result, AMS_LINE_END);
    return;
}

#define AMS_MSG_TYPE_HASH_MASK        (0xff)

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
* @note         : djb2 algorithm; see http://www.cse.yorku.ca/~oz/hash.html
 *********************************************************************************************************
*/
int ams_decode_msg_type(const char *buf, uint32_t buf_size)
{
    uint32_t hash = 5381;
    
    for (const char *top = buf + buf_size; buf < top; buf++) 
    {
        hash = ((hash << 5) + hash) ^ (*buf); // hash * 33 ^ data
    }
    // Make sure that valid hash is never zero, zero means "hash not computed"
    return (hash ? hash & AMS_MSG_TYPE_HASH_MASK : 1);
}

static int ams_find_msg_offset(const char *key, const char *msg, int *spos , int *epos)
{
    int bfind = 0 ;
    int idx1, idx2;

    for (idx1 = 0; idx1 < strlen(msg) ; idx1++) 
    {
        if(strncmp(&(msg[idx1]), key, strlen(key)) == 0) 
        {
            bfind = 1;
            *spos = idx1;
            break;
        }
    }

    if(!bfind)
    {
        return AMS_ERROR;
    }
    
    bfind = 0 ;
    for (idx2 = idx1; idx2 < strlen(msg); idx2++)
    {
        if(msg[idx2] == ',')
        {
            //find sub string end pos;
            bfind = 1;
            *epos = idx2;
            break;
        }
    }

    if(!bfind) {
        *epos = idx2;
    }

    return AMS_OK;

}

static void ams_decode_msg_body(const char *key, const char *msg, char *result)
{
    int start = 0;
    int end   = 0;
    
    if(ams_find_msg_offset(key, msg, &start , &end) != AMS_ERROR)
    {
        strncpy(result , &(msg[start + strlen(key)]), end -(start + strlen(key)));
        ams_log("result: %s", result);
    }
    else
    {
        ams_err("Decode msg[%s] failed.", msg);
    }

    return;
}

static void ams_decode_msg_head(const char *msg, char *result)
{
    int id_str_len = strlen(AMS_KEY_UNIQUE_ID);
    int head_len = AMS_PROTOCOL_TYPE_LEN + id_str_len;
    
    if (strncmp(&(msg[AMS_PROTOCOL_TYPE_LEN]), AMS_KEY_UNIQUE_ID, id_str_len) == 0)
    {
        strncpy(result, &(msg[head_len]), AMS_UNIQUE_ID_LEN - 1);
    }

    return;
}

void *ams_parse_heartbeat_cmd(const char *msg)
{
    char *unique_id = NULL;

    AMS_CALLOC_MEM_FOR_PARSE_MSG(unique_id, AMS_UNIQUE_ID_LEN, "heartbeat");
    ams_decode_msg_head(msg, unique_id);
    
    ams_log("unique_id : %s", unique_id);
    return (void *)unique_id;
}

void *ams_parse_list_app_cmd(const char *msg)
{
    char *unique_id = NULL;

    AMS_CALLOC_MEM_FOR_PARSE_MSG(unique_id, AMS_UNIQUE_ID_LEN, "list");
    ams_decode_msg_head(msg, unique_id);
    
    ams_log("unique_id : %s", unique_id);
    return (void *)unique_id;
}

void *ams_parse_stop_app_cmd(const char *msg)
{
    char *unique_id = NULL;

    AMS_CALLOC_MEM_FOR_PARSE_MSG(unique_id, AMS_UNIQUE_ID_LEN, "stop");
    ams_decode_msg_head(msg, unique_id);

    ams_log("unique_id : %s.", unique_id);
    return (void *)unique_id;
}

void *ams_parse_report_app_cmd(const char *msg)
{
    char *unique_id = NULL;

    AMS_CALLOC_MEM_FOR_PARSE_MSG(unique_id, AMS_UNIQUE_ID_LEN, "report");
    ams_decode_msg_head(msg, unique_id);

    ams_log("unique_id : %s.", unique_id);
    return (void *)unique_id;
}


void *ams_parse_delete_app_cmd(const char *msg)
{
    ams_delete_app_param_t *param = NULL;
    
    AMS_CALLOC_MEM_FOR_PARSE_MSG(param, sizeof(ams_delete_app_param_t), "delete");
    ams_decode_msg_head(msg, param->unique_id);
    ams_decode_msg_body(AMS_KEY_APP_NAME, msg, param->app_name);
    
    ams_log("appname : %s", param->app_name);
    return (void *)param;
}
/*
    |socket://|[ip:]|[port/]|[dowload url]
*/
/*
    socket://111.10.38.128:28766/https://oneos-platform-oss-tesh-hlang.oss-cn-hangzhou.aliyuncs.com/iotoneos/data/file/upload/b/e/c/203599d5
*/
void *ams_parse_download_app_cmd(const char *msg)
{
    char tmp[1] = {0};
    char str_port[6] = {0};
    char app_len_buf[8] = {0};
    int idx1, idx2;
    ams_download_app_param_t *param = NULL;
    
    AMS_CALLOC_MEM_FOR_PARSE_MSG(param, sizeof(ams_download_app_param_t), "download");
    ams_decode_msg_head(msg, param->unique_id);
    ams_decode_msg_body(AMS_KEY_APP_NAME, msg, param->app_name);
    ams_decode_msg_body(AMS_KEY_APP_URL, msg, param->app_url);
    ams_decode_msg_body(AMS_KEY_MAIN_CLASS, msg, param->main_class);
    ams_decode_msg_body(AMS_KEY_AUTO_START, msg, tmp);
    if (tmp[0] >= '0' && tmp[0] <= '9')
    {
        param->auto_start = tmp[0] - '0';
    }

    ams_decode_msg_body(AMS_KEY_START_NOW, msg, tmp);
    if (tmp[0] >= '0' && tmp[0] <= '9')
    {
        param->start_now = tmp[0] - '0';
    }

    ams_decode_msg_body(AMS_KEY_APP_LEN, msg, app_len_buf);
    for(int idx = 0; idx < strlen(app_len_buf); idx++) 
    {
        param->app_len = param->app_len * 10 + (app_len_buf[idx] - '0');
    }

    for (idx1 = 9 ; idx1 < strlen(param->app_url) ; idx1++) 
    {
        if(param->app_url[idx1] == ':')
        {
            break;
        }
    }
    strncpy(param->saddr , &param->app_url[9] , idx1 - 9);
    param->saddr[idx1 - 9] = '\0';
    idx1++;
    for(idx2 = idx1 ; idx2 < strlen(param->app_url); idx2++)
    {
        if(param->app_url[idx2] == '/')
        {
            break;
        }
    }
    strncpy(str_port , &param->app_url[idx1] , idx2 - idx1);
    for(int i = 0 ; i < strlen(str_port); i++)
    {
        param->port = param->port * 10 + (str_port[i]-'0');
    }
    memset(param->payload, 0, sizeof(param->payload));
    strcat(param->payload, "GET ");
    ams_log("app_url: %s.", &param->app_url[idx2 + 1]);
    strncpy(&param->payload[4] , &param->app_url[idx2 + 1] , strlen(param->app_url) - idx2);
    strcat(param->payload , "\n");
    
    ams_log("appname: %s, mainclass: %s, applen : %d, saddr: %s, port: %d.", 
                    param->app_name, param->main_class, param->app_len,
                    param->saddr, param->port);
    ams_log("payload: %s.", param->payload);
    return (void *)param;
}

void *ams_parse_start_app_cmd(const char *msg)
{
    ams_start_app_param_t *param = NULL;
    
    AMS_CALLOC_MEM_FOR_PARSE_MSG(param, sizeof(ams_start_app_param_t), "start");
    
    ams_decode_msg_head(msg, param->unique_id);
    ams_decode_msg_body(AMS_KEY_APP_NAME, msg, param->app_name);
    ams_decode_msg_body(AMS_KEY_MAIN_CLASS, msg, param->main_class);
    
    ams_log("unique_id: %s, app_name: %s, main_class : %s", 
                param->unique_id, param->app_name, param->main_class);
    return (void *)param;
}

static ams_parse_result_t   g_ams_parse_result[AMS_NORMAL_TASK_MAX] = {
                        {AMS_DOWNLAPP_HASH, ams_parse_download_app_cmd},
                        {AMS_STARTAPP_HASH, ams_parse_start_app_cmd},
                        {AMS_RLISTAPP_HASH, ams_parse_list_app_cmd},
                        {AMS_RSTOPAPP_HASH, ams_parse_stop_app_cmd},
                        {AMS_HEARTBEA_HASH, ams_parse_heartbeat_cmd},
                        {AMS_RDELEAPP_HASH, ams_parse_delete_app_cmd},
                        {AMS_RUNNGAPP_HASH, ams_parse_report_app_cmd},
                        {AMS_RESETSYS_HASH, NULL},
};

int ams_parse_msg(const char *msg_buf, ams_task_param_t *result)
{
    int msg_type_hash = -1;
    int i = 0;
    
    msg_type_hash = ams_decode_msg_type(msg_buf, AMS_PROTOCOL_TYPE_LEN);
    ams_log("The hash of msg type is: 0x%02x.", msg_type_hash);
    for (i = 0; i < AMS_NORMAL_TASK_MAX; i++)
    {
        if (g_ams_parse_result[i].hash_id == msg_type_hash)
        {
            result->id = i;
            if (g_ams_parse_result[i].func != NULL)
            {
                result->param = g_ams_parse_result[i].func(msg_buf);
                ams_log("Get a param[%p]", result->param);
            }
            break;
        }
    }
    
    if (i == AMS_NORMAL_TASK_MAX)
    {
        ams_err("Map msg type[0x%02x] to task type failed.", msg_type_hash);
        return AMS_ERROR;
    }
    

    return AMS_OK;
}


