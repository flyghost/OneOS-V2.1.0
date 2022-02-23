#ifndef __AMS_PROTOCOL_H__
#define __AMS_PROTOCOL_H__
#include <stdio.h>
#include <string.h>
#include "core/ams_core.h"
#include "core/ams_task.h"

/* protocol: "type"+"unique id"+"message" */
/* example: [STARTAPP]UNIQUEID=xx,APPNAME=xx,MAINCLASS=null */
/* type: [STARTAPP], 10 Bytes; 
   unique id: UNIQUEID=xx, 9 + 16 Bytes; 
   message: APPNAME=xx,MAINCLASS=null */

#define AMS_PROTOCOL_TYPE_LEN       10
#define AMS_UNIQUE_ID_LEN           17

#define AMS_STARTAPP_HASH   (0xc2)
#define AMS_HEARTBEA_HASH   (0xa7)
#define AMS_DOWNLAPP_HASH   (0x7c)
#define AMS_RLISTAPP_HASH   (0xb2)
#define AMS_RSTOPAPP_HASH   (0x88)
#define AMS_RDELEAPP_HASH   (0x98)
#define AMS_RUNNGAPP_HASH   (0xe2)
#define AMS_RESETSYS_HASH   (0xc7)
typedef void *(*ams_msg_parse_func)(const char *);

typedef struct ams_parse_result {
    uint32_t hash_id;
    ams_msg_parse_func func;
} ams_parse_result_t;


#define AMS_EMPTY_PARSE_FUNC    ()

#define AMS_ENCODE_BUFF_LEN         512
#define AMS_LINE_END                "\n"
#define AMS_CODE_SEPARATOR          ","

#define AMS_KEY_UNIQUE_ID           "UNIQUEID="
#define AMS_KEY_APP_NAME            "APPNAME="
#define AMS_KEY_MAIN_CLASS          "MAINCLASS="
#define AMS_KEY_APP_URL             "APPURL="
#define AMS_KEY_AUTO_START          "AUTOSTART="
#define AMS_KEY_START_NOW           "STARTNOW="
#define AMS_KEY_APP_LEN             "APPLEN="

#define AMS_STORE_MSG(buf, tmp, type, src)                  \
    do {                                                    \
        memset(tmp, 0, sizeof(tmp));                        \
        sprintf(tmp, type, src);                            \
        strcat(buf, tmp);                                   \
    } while (0)

#define AMS_ADD_REPORT_MSG_HEAD(cmd, id, type, msg, tmp)                \
    do {                                                                \
        strcat(cmd , "[REPORTC]");                                      \
        if(strlen(id) != 0)                                         \
        {                                                               \
            strcat(cmd , id);                                       \
        }                                                               \
        strcat(cmd , AMS_CODE_SEPARATOR);                               \
        AMS_STORE_MSG(cmd, tmp, type, msg);                             \
    } while(0);


typedef enum AMS_PROTOCOL_TYPE {
    /*COMMAND CODE*/
    AMS_PROTOCOL_DOWNLOAD       = 30,
    AMS_PROTOCOL_START          = 31,
    AMS_PROTOCOL_LIST           = 32,
    AMS_PROTOCOL_STOP           = 33,
    AMS_PROTOCOL_HEARTBEAT      = 34,
    AMS_PROTOCOL_DELETE         = 35,
    AMS_PROTOCOL_RUNNING        = 36,
    AMS_PROTOCOL_RESET_JVM      = 37,

    /*REPORT CODE*/
    AMS_PROTOCOL_RESP_DOWNLOAD    = 60,
    AMS_PROTOCOL_RESP_APPEXIST    = 61,
    AMS_PROTOCOL_RESP_INSTALLOK   = 62,
    AMS_PROTOCOL_RESP_APPSTARTOK  = 63,
    AMS_PROTOCOL_RESP_HEARTBEAT   = 64,
    AMS_PROTOCOL_RESP_REGISTER    = 65,
    AMS_PROTOCOL_RESP_APPLIST     = 66,
    AMS_PROTOCOL_RESP_APPFINISH   = 67,
    AMS_PROTOCOL_RESP_DELETEOK    = 68,
    AMS_PROTOCOL_RESP_RUNNINGAPPLIST = 69,

    /*ERROR REPORT CODE*/
    AMS_PROTOCOL_FAIL_DOWNLOAD = 100,
    AMS_PROTOCOL_INSTALLFAIL   = 101,
    AMS_PROTOCOL_APPSTARTERROR = 102,
    AMS_PROTOCOL_APPNOTEXIST   = 103,
    AMS_PROTOCOL_DELETEFAIL    = 104,
    AMS_PROTOCOL_APPNOTFINISH  = 105,
}AMS_PROTOCOL_TYPE_T;

typedef struct ams_download_app_param {
    unsigned int   auto_start;
    unsigned int   start_now;
    unsigned int   app_len;
    unsigned int   port;
    char unique_id[AMS_UNIQUE_ID_LEN];
    char app_name[AMS_APP_NAME_LEN];
    char main_class[AMS_MAIN_CLASS_LEN];
    char app_url[AMS_APP_URL_LEN];
    char saddr[32];
    char payload[160];
} ams_download_app_param_t;

typedef struct ams_start_app_param {
    char unique_id[AMS_UNIQUE_ID_LEN];
    char app_name[AMS_APP_NAME_LEN];
    char main_class[AMS_MAIN_CLASS_LEN];
} ams_start_app_param_t;

typedef struct ams_delete_app_param {
    char unique_id[AMS_UNIQUE_ID_LEN];
    char app_name[AMS_APP_NAME_LEN];
} ams_delete_app_param_t;

#define AMS_CALLOC_MEM_FOR_PARSE_MSG(param, param_size, param_info)                 \
    do {                                                                \
        param = ams_calloc(1, param_size);                           \
        if (param == NULL)                                      \
        {                                                       \
            ams_err("No memory[%d] for cmd[%s].", param_size, param_info);                \
            return NULL;                                             \
        }                                                       \
        ams_log("Alloc a memory[%d] for cmd[%s].", param_size, param_info);                \
        memset(param, 0, param_size);                           \
    } while(0)

void ams_encode_device_info(int reconnect_count, char *cmd);

void ams_encode_response_result(const char *uni_id, int protocol, const char* val, char *result);

int ams_decode_msg_type(const char *buf, uint32_t buf_size);

int ams_parse_msg(const char *msg_buf, ams_task_param_t *result);


#endif /* __AMS_PROTOCOL_H__ */


