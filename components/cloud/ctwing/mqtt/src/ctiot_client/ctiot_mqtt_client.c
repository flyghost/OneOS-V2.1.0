#include "ctiot_mqtt_client.h"
#include "ctiot_log.h"
#include "MQTTClient.h"
#include "cJSON.h"
#include "ctiot_os.h"

//#include "MQTTLinux.h"
#include "mqtt_os.h"
#include "ctiot_memory.h"

#define MQTT_VERSION_3_1 (3)
#define MQTT_VERSION_3_1_1 (4)

#define VARIABLE_SIZE (4 + 1)

#define MQTT_TIME_BUF_LEN 11

#define STRING_MAX_LEN 256

#define IS_VALID_NAME_LEN(name) (strlen((name)) <= STRING_MAX_LEN)

SDK_U8  globalPingTimes = 0;  

typedef enum{
    CTIOT_INT,
    CTIOT_DOUBLE,
    CTIOT_FLOAT,
    CTIOT_STR,
    CTIOT_ENUM,
    CTIOT_BOOL,
    CTIOT_DATE,
}CTIOT_PARA_TYPE;
typedef struct
{
    int max;
    int min;
    int val;
}CTIOT_INT_ITEM;

typedef struct
{
    double max;
    double min;
    double val;
}CTIOT_DOUBLE_ITEM;

typedef struct
{
    float max;
    float min;
    float val;
}CTIOT_FLOAT_ITEM;

typedef struct
{
    int maxLen;
    int minLen;
    char *val;
}CTIOT_STR_ITEM;

typedef struct
{
    int maxLen;
    int minLen;
    int val;
}CTIOT_ENUM_ITEM;

typedef struct
{
    int max;
    int min;
    int val;
}CTIOT_BOOL_ITEM;

typedef struct
{
    int max;
    int min;
    long long val;
}CTIOT_DATE_ITEM;

typedef struct
{
    CTIOT_PARA_TYPE ctiotParaType;
    char *paraName;
	union{
        CTIOT_STR_ITEM ctiotStr;
        CTIOT_FLOAT_ITEM ctiotFloat;
        CTIOT_DOUBLE_ITEM ctiotDouble;
        CTIOT_DATE_ITEM ctiotDate;
        CTIOT_INT_ITEM ctiotInt;
        CTIOT_BOOL_ITEM ctiotBool;
        CTIOT_ENUM_ITEM ctiotEnum;
    }u;
}CTIOT_PARAM_ITEMS;

#define MAX_PARA_COUNT 10

typedef struct ctiot_mqtt_para{
    int count;
    CTIOT_PARAM_ITEMS paraList[MAX_PARA_COUNT];
}CTIOT_MQTT_PARA;

typedef void (* CTIOT_CB)(MessageData *md);

struct mqtt_client_tag_s
{
	mqtt_device_info_s device_info;
	MQTTClient client;
	mqtt_param_s params;
	void *ctiotCbFunc;
	char *sub_topic;
	uint8_t init_flag;
	uint8_t reserve[3];
};

static uint8_t g_mqtt_sendbuf[MQTT_SENDBUF_SIZE];

/* reserve 1 byte for string end 0 for jason */
static uint8_t g_mqtt_readbuf[MQTT_READBUF_SIZE + 1];

mqtt_client_s g_mqtt_client;

cJSON_Hooks g_cJSON_Hooks = {CTIOT_MALLOC,CTIOT_FREE};

Network n;

static int ctiot_mqtt_publish_msg(mqtt_client_s *phandle, const char *msg,  uint32_t msg_len, mqtt_qos_e qos, char* topic);

char *ctiot_strdup(const char *ch)
{
    char *copy;
    size_t length;

    if(NULL == ch)
        return NULL;

    length = strlen(ch);
    OS_GET_MEM(copy,char,length + 1);
    if(NULL == copy)
        return NULL;
    memcpy(copy, ch, length);
    copy[length] = '\0';

    return copy;
}

static void ctiot_mqtt_register_cbs(void *cbs)
{
	#ifdef NEED_DN_DATA
	CTIOT_CB_FUNC *p = cbs;
	
	if(NULL == cbs)
		return;
	
    g_mqtt_client.ctiotCbFunc = p;
	#else
	return;
	#endif
}


static void ctiot_mqtt_free_params(mqtt_param_s *param)
{
    OS_PUT_MEM(param->server_ip);
    OS_PUT_MEM(param->server_port);
}

static int ctiot_mqtt_check_param(const mqtt_param_s *param)
{
    if ((param->server_ip == NULL)
        || (param->server_port == NULL)
        || (param->info.security_type >= MQTT_SECURITY_TYPE_MAX))
    {
        CTIOT_LOG(LOG_FATAL, "invalid param, sec type %d", param->info.security_type);
        return CTIOT_ARG_INVALID;
    }

    return CTIOT_OK;
}

static int ctiot_mqtt_dup_param(mqtt_param_s *dest, const mqtt_param_s *src)
{
    memset(dest, 0, sizeof(*dest));

    dest->info.security_type = src->info.security_type;
    dest->info.u.ca.ca_crt = src->info.u.ca.ca_crt; /* add by OneOS Team to support TLS */
    dest->info.u.ca.ca_len = src->info.u.ca.ca_len; /* add by OneOS Team to support TLS */

    dest->server_ip = ctiot_strdup(src->server_ip);
    if(NULL == dest->server_ip)
    {
        CTIOT_LOG(LOG_FATAL, "ctiot_strdup NULL");
        return CTIOT_MALLOC_FAILED;
    }

    dest->server_port = ctiot_strdup(src->server_port);
    if(NULL == dest->server_port)
    {
        CTIOT_LOG(LOG_FATAL, "ctiot_strdup NULL");
        goto mqtt_param_dup_failed;
    }

    return CTIOT_OK;

mqtt_param_dup_failed:
    ctiot_mqtt_free_params(dest);
    return CTIOT_MALLOC_FAILED;
}

static void ctiot_mqtt_free_device_info(mqtt_device_info_s *info)
{

    OS_PUT_MEM(info->password);
    if(MQTT_STATIC_CONNECT == info->connection_type)
    {
        OS_PUT_MEM(info->u.s_info.deviceid);
    }
}

static int ctiot_mqtt_check_device_info(const mqtt_device_info_s *info)
{
    if((info->connection_type >= MQTT_MAX_CONNECTION_TYPE)
        || (info->codec_mode >= MQTT_MAX_CODEC_MODE)
        || (NULL == info->password)
        || (!IS_VALID_NAME_LEN(info->password)))
    {
        CTIOT_LOG(LOG_FATAL, "invalid device info con_type %d codec_mode %d ",
            info->connection_type, info->codec_mode);
        return CTIOT_ARG_INVALID;
    }

    if ((info->connection_type == MQTT_STATIC_CONNECT)
        && ((NULL == info->u.s_info.deviceid)
        || (!IS_VALID_NAME_LEN(info->u.s_info.deviceid))))
    {
        CTIOT_LOG(LOG_FATAL, "invalid static device info con_type %d codec_mode %d",
            info->connection_type, info->codec_mode);
        return CTIOT_ARG_INVALID;
    }
    else if(info->connection_type != MQTT_STATIC_CONNECT)
    {
        CTIOT_LOG(LOG_FATAL, "invalid connection type, con_type %d ",
            info->connection_type);
    }

    return CTIOT_OK;

}

static int ctiot_mqtt_dup_device_info(mqtt_device_info_s *dest, const mqtt_device_info_s *src)
{
    memset(dest, 0, sizeof(*dest));
    dest->connection_type = src->connection_type;
    dest->codec_mode = src->codec_mode;
    dest->password = ctiot_strdup(src->password);
    if (NULL == dest->password)
    {
        CTIOT_LOG(LOG_INFO, "ctiot_strdup fail");
        return CTIOT_MALLOC_FAILED;
    }

    if(MQTT_STATIC_CONNECT == src->connection_type)
    {
        dest->u.s_info.deviceid = ctiot_strdup(src->u.s_info.deviceid);
        if (NULL == dest->u.s_info.deviceid)
        {
            CTIOT_LOG(LOG_FATAL, "ctiot_strdup fail");
            goto MALLOC_FAIL;
        }
    }

    return CTIOT_OK;

MALLOC_FAIL:
    ctiot_mqtt_free_device_info(dest);
    return CTIOT_MALLOC_FAILED;
}

static bool ctiot_mqtt_is_connectting_with_deviceid(const mqtt_client_s* handle)
{
    return (MQTT_STATIC_CONNECT == handle->device_info.connection_type);
}

static void ctiot_mqtt_destroy_data_connection_info(MQTTPacket_connectData *data)
{
    OS_PUT_MEM(data->clientID.cstring);
    OS_PUT_MEM(data->password.cstring);
}



static int ctiot_mqtt_get_connection_info(mqtt_client_s* handle, MQTTPacket_connectData *data)
{
    char *password;

    if (ctiot_mqtt_is_connectting_with_deviceid(handle))
    {
        if (handle->device_info.connection_type == MQTT_STATIC_CONNECT)
        {
            password = handle->device_info.password;
        }
        CTIOT_LOG(LOG_INFO, "try static connect");
    }

    data->clientID.cstring = ctiot_strdup(handle->device_info.u.s_info.deviceid);

    if (data->clientID.cstring == NULL)
    {
        return CTIOT_MALLOC_FAILED;
    }

    data->username.cstring = data->clientID.cstring;
    data->password.cstring = ctiot_strdup(password);

    if (data->password.cstring == NULL)
    {
        return CTIOT_ERR;
    }

    CTIOT_LOG(LOG_FATAL, "send user %s client %s", data->username.cstring,
                data->clientID.cstring);

    return CTIOT_OK;
}

static int ctiot_mqtt_modify_payload(void *md)
{
    char *end = ((char *)((MessageData *)md)->message->payload) + ((MessageData *)md)->message->payloadlen;
    static uint32_t callback_err;

    if ((end >= (char *)g_mqtt_readbuf) && (end < (char *)(g_mqtt_readbuf + sizeof(g_mqtt_readbuf))))
    {
         *end = '\0';
         return CTIOT_OK;
    }

    CTIOT_LOG(LOG_ERR, "not expect msg callback err, pl %p, len %ld, err num %ld", ((MessageData *)md)->message->payload, ((MessageData *)md)->message->payloadlen, ++callback_err);

    return CTIOT_ERR;
}

static void ctiot_mqtt_disconnect( MQTTClient *client, Network *n)
{
    if (MQTTIsConnected(client))
    {
        (void)MQTTDisconnect(client);
        CTIOT_LOG(LOG_INFO, "MQTT disconnect"); /* add by OneOS Team */
    }
    NetworkDisconnect(n);

    //CTIOT_LOG(LOG_ERR, "mqtt_disconnect"); /* mask by OneOS Team */
}

static inline void ctiot_mqtt_inc_fail_cnt(int32_t *conn_failed_cnt)
{
    if(*conn_failed_cnt < MQTT_CONN_FAILED_MAX_TIMES)
    {
        (*conn_failed_cnt)++;
    }
}

static void ctiot_mqtt_proc_connect_err( MQTTClient *client, Network *n, int32_t *conn_failed_cnt)
{
    ctiot_mqtt_inc_fail_cnt(conn_failed_cnt);
    ctiot_mqtt_disconnect(client, n);
}

//static mqtt_security_info_s *mqtt_get_security_info(void)
//{
//    mqtt_client_s* handle = &g_mqtt_client;
//    return &handle->params.info;
//}

int ctiot_mqtt_init(const mqtt_param_s *params,void *callback_struct, mqtt_client_s **phandle)
{
    cJSON_InitHooks(&g_cJSON_Hooks);
    if (params == NULL || phandle == NULL
        || ctiot_mqtt_check_param(params) != CTIOT_OK)
    {
        CTIOT_LOG(LOG_FATAL, "Invalid args");
        return CTIOT_ARG_INVALID;
    }

    if (g_mqtt_client.init_flag)
    {
        CTIOT_LOG(LOG_FATAL, "mqtt reinit");
        return CTIOT_ERR;
    }

    memset(&g_mqtt_client, 0, sizeof(g_mqtt_client));

    if (CTIOT_OK != ctiot_mqtt_dup_param(&(g_mqtt_client.params), params))
    {
        return CTIOT_MALLOC_FAILED;
    }

    *phandle = &g_mqtt_client;

    ctiot_mqtt_register_cbs(callback_struct);

    g_mqtt_client.init_flag = true;

    return CTIOT_OK;
}

int  ctiot_mqtt_login(const mqtt_device_info_s* device_info, mqtt_client_s* handle)
{
    MQTTClient *client = NULL;
    mqtt_param_s *params;
    int rc;
    int32_t conn_failed_cnt = 0;
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    int result = CTIOT_ERR;

    if (NULL == handle)
    {
        CTIOT_LOG(LOG_FATAL, "handle null");
        return CTIOT_ARG_INVALID;
    }

    if((device_info == NULL)
        || (ctiot_mqtt_check_device_info(device_info) != CTIOT_OK))
    {
        CTIOT_LOG(LOG_FATAL, "parameter invalid");
        result = CTIOT_ARG_INVALID;
        goto  ctiot_login_quit;
    }

    client = &(handle->client);
    params = &(handle->params);

    rc = ctiot_mqtt_dup_device_info(&(handle->device_info), device_info);
    if (rc != CTIOT_OK)
    {
        goto  ctiot_login_quit;
    }

    //NetworkInit(&n);
    /* Modify by OneOS Team to support TLS */
#ifdef CTIOT_MQTT_USING_TLS
    NetworkInit(&n, params->server_ip, atoi(params->server_port), params->info.u.ca.ca_crt);
#else
    NetworkInit(&n, params->server_ip, atoi(params->server_port), NULL);
#endif

    memset(client, 0x0, sizeof(MQTTClient));
    MQTTClientInit(client, &n, MQTT_COMMAND_TIMEOUT_MS, g_mqtt_sendbuf, MQTT_SENDBUF_SIZE, g_mqtt_readbuf, MQTT_READBUF_SIZE);

    data.willFlag = 0;
    data.MQTTVersion = MQTT_VERSION_3_1_1;
    data.keepAliveInterval = MQTT_KEEPALIVE_INTERVAL_S;
    data.cleansession = true;

    while(true)
    {
        if(conn_failed_cnt > 0)
        {
            CTIOT_LOG(LOG_INFO, "reconnect delay : %d", conn_failed_cnt);
            OS_SLEEP(MQTT_CONN_FAILED_BASE_DELAY  << conn_failed_cnt);
        }

        //rc = NetworkConnect(&n, params->server_ip, atoi(params->server_port));
        rc = NetworkConnect(&n); /* Modify by OneOS Team to support TLS */
        if(rc != 0)
        {
            CTIOT_LOG(LOG_ERR, "NetworkConnect fail: %d", rc);
            ctiot_mqtt_inc_fail_cnt(&conn_failed_cnt);
            NetworkDisconnect(&n);
            continue;
        }

        if(ctiot_mqtt_get_connection_info(handle, &data) != CTIOT_OK)
        {
            CTIOT_LOG(LOG_ERR, "get mqtt connection info failed");
            ctiot_mqtt_destroy_data_connection_info(&data);
            ctiot_mqtt_proc_connect_err(client, &n, &conn_failed_cnt);
            continue;
        }

        rc = MQTTConnect(client, &data);
        ctiot_mqtt_destroy_data_connection_info(&data);
        CTIOT_LOG(LOG_DEBUG, "MQTT CONNACK : %d", rc);
        if(MQTT_SUCCESS != rc)
        {
            CTIOT_LOG(LOG_ERR, "MQTTConnect failed %d", rc);
            ctiot_mqtt_proc_connect_err(client, &n, &conn_failed_cnt);
            continue;
        }

        #ifdef NEED_DN_DATA
        if(CTIOT_OK != ctiot_mqtt_subscribe(handle))       
        {
            CTIOT_LOG(LOG_ERR, "mqtt_subscribe_topic failed");
            ctiot_mqtt_proc_connect_err(client, &n, &conn_failed_cnt);
            continue;
        }
        else
        {
            return rc;
        }
		#else
			return rc;
		#endif
    }
ctiot_login_quit:
    ctiot_mqtt_free_params(&(handle->params));
    ctiot_mqtt_free_device_info(&(handle->device_info));
    handle->init_flag = false;
    return result;
}


CTIOT_STATUS ctiot_handleMessagr(mqtt_client_s* phandle)
{
    int rc = 0;
    if(MQTTIsConnected(&(phandle->client)))
    {
        rc = MQTTYield(&(phandle->client), MQTT_EVENTS_HANDLE_PERIOD_MS);
        if (rc >= 0)
        {
            return CTIOT_OK;
        }
        else
        {
            return CTIOT_YIELD_ERROR;
        }
    }
    else{
        return CTIOT_ERR;
    }
}

CTIOT_STATUS ctiot_mqtt_logout(mqtt_client_s* phandle)
{
    CTIOT_STATUS ret = CTIOT_OK;
    ctiot_mqtt_disconnect(&(phandle->client), &n);
    MQTTClientDeInit(&(phandle->client)); /* Add by OneOS Team, to adapt OneOS */
    ctiot_mqtt_free_params(&(phandle->params));
    ctiot_mqtt_free_device_info(&(phandle->device_info));
    phandle->init_flag = false;
    return ret;
}



int ctiot_mqtt_isconnected(mqtt_client_s* phandle)
{
    if (NULL == phandle)
    {
        CTIOT_LOG(LOG_ERR, "invalid args");
        return false;
    }
    return ctiot_mqtt_is_connectting_with_deviceid(phandle) && MQTTIsConnected(&(phandle->client));
}

static CTIOT_MSG_STATUS ctiot_mqtt_validate(CTIOT_MQTT_PARA para)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    int i = 0;
    for(;i < para.count;i++){
        switch(para.paraList[i].ctiotParaType){
            case CTIOT_INT:
			{
                if(para.paraList[i].u.ctiotInt.val < para.paraList[i].u.ctiotInt.min || para.paraList[i].u.ctiotInt.val > para.paraList[i].u.ctiotInt.max){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_DOUBLE:
			{
                if(para.paraList[i].u.ctiotDouble.val < para.paraList[i].u.ctiotDouble.min || para.paraList[i].u.ctiotDouble.val > para.paraList[i].u.ctiotDouble.max){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_FLOAT:
			{
                if(para.paraList[i].u.ctiotFloat.val < para.paraList[i].u.ctiotFloat.min || para.paraList[i].u.ctiotFloat.val > para.paraList[i].u.ctiotFloat.max){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_STR:
			{
                if(para.paraList[i].u.ctiotStr.val != NULL && (strlen(para.paraList[i].u.ctiotStr.val) < para.paraList[i].u.ctiotStr.minLen || strlen(para.paraList[i].u.ctiotStr.val) > para.paraList[i].u.ctiotStr.maxLen)){
                    return CTIOT_PARA_ERROR;
                }
                break;
			}
            case CTIOT_ENUM:
			{
                break;
			}
            case CTIOT_BOOL:
			{
                break;
			}
            case CTIOT_DATE:
			{
                break;
			}
            default:
				return CTIOT_TYPE_ERROR;
        }
    }
    return ret;
}

static CTIOT_MSG_STATUS ctiot_mqtt_msg_encode(CTIOT_MQTT_PARA para,char** payload)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    cJSON *root = cJSON_CreateObject();
    cJSON *node = NULL;
    int i = 0;
	
    if(root == NULL)
    {
        goto failed;
    }
    
    for(;i < para.count;i++){
        switch(para.paraList[i].ctiotParaType){
            case CTIOT_INT:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotInt.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(root,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_DOUBLE:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotDouble.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(root,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_FLOAT:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotFloat.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(root,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_STR:
			{
				if(para.paraList[i].u.ctiotStr.val == NULL)
				{
					node = cJSON_CreateNull();
					if (node == NULL)
					{
						goto failed;
					}
				}
				else
				{
					node = cJSON_CreateString(para.paraList[i].u.ctiotStr.val);
					if(node == NULL)
					{
						goto failed;
					}
				}
                cJSON_AddItemToObjectCS(root,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_ENUM:
            {
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotEnum.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(root,para.paraList[i].paraName,node);
                break;
            }
            case CTIOT_BOOL:
			{
                node = cJSON_CreateBool(para.paraList[i].u.ctiotBool.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(root,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_DATE:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotDate.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(root,para.paraList[i].paraName,node);
                break;
			}
            default:
			{
                if(root){
                    cJSON_Delete(root);
                }
                return CTIOT_TYPE_ERROR;
			}
        }
    }
    (*payload) = cJSON_Print(root);
    if((*payload) == NULL)
    {
        goto failed;
    }
    cJSON_Delete(root);
    CTIOT_LOG(LOG_DEBUG, "report:%s!",(*payload));

    return ret;
failed:
    cJSON_Delete(root);
    //return CTIOT_MALLOC_FAILED;
    return CTIOT_OTHER_ERROR;
}

static CTIOT_MSG_STATUS ctiot_mqtt_msg_response_encode(CTIOT_MQTT_PARA para,int taskId,char** payload)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    cJSON *node = NULL;
    cJSON *root = cJSON_CreateObject();
    int i = 0;
    
    if(root == NULL)
    {
        return CTIOT_OTHER_ERROR;			
    }
    cJSON_AddItemToObjectCS(root,"taskId",cJSON_CreateNumber(taskId));

    cJSON *paylaodItem = cJSON_CreateObject();
    if(paylaodItem == NULL)
    {
        goto failed;
    }
    
    for(;i < para.count;i++){
        switch(para.paraList[i].ctiotParaType){
            case CTIOT_INT:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotInt.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(paylaodItem,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_DOUBLE:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotDouble.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(paylaodItem,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_FLOAT:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotFloat.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(paylaodItem,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_STR:
			{
                node = cJSON_CreateString(para.paraList[i].u.ctiotStr.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(paylaodItem,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_ENUM:
            {
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotEnum.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(paylaodItem,para.paraList[i].paraName,node);
                break;
            }
            case CTIOT_BOOL:
			{
                node = cJSON_CreateBool(para.paraList[i].u.ctiotBool.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(paylaodItem,para.paraList[i].paraName,node);
                break;
			}
            case CTIOT_DATE:
			{
                node = cJSON_CreateNumber(para.paraList[i].u.ctiotDate.val);
                if(node == NULL)
                {
                    goto failed;
                }
                cJSON_AddItemToObjectCS(paylaodItem,para.paraList[i].paraName,node);
                break;
			}
            default:
            {
                if(root){
                    cJSON_Delete(root);
                }
                if(paylaodItem){
                    cJSON_Delete(paylaodItem);
                }
                return CTIOT_TYPE_ERROR;
            }
        }
    }
    cJSON_AddItemToObjectCS(root,"resultPayload",paylaodItem);
    (*payload) = cJSON_Print(root);
    if((*payload) == NULL)
    {
        goto failed;
    }
    cJSON_Delete(root);
    CTIOT_LOG(LOG_DEBUG, "response:%s!",(*payload));
    return ret;
failed:
    cJSON_Delete(root);
		//return CTIOT_MALLOC_FAILED;
    return CTIOT_OTHER_ERROR;
}

/*
#define cJSON_False  (1 << 0)
#define cJSON_True   (1 << 1)
#define cJSON_NULL   (1 << 2)
#define cJSON_Number (1 << 3)
#define cJSON_String (1 << 4)
#define cJSON_Array  (1 << 5)
*/
#if (DEVICE_TRANSPARENT == 0)
static CTIOT_MQTT_PARA *ctiot_mqtt_json_parsing(char *json)
{
    CTIOT_MQTT_PARA *para = NULL;
    OS_GET_MEM(para,char,sizeof(CTIOT_MQTT_PARA));
    if(para == NULL)
    {
        return NULL;
    }
    int count = 0;
    cJSON *root = cJSON_Parse(json);
    if(root == NULL)
    {
        return NULL;
    }
    cJSON *payloadItem = cJSON_GetObjectItem(root,"payload");
    cJSON *taskIdItem = cJSON_GetObjectItem(root,"taskId");

    para->paraList[count].ctiotParaType = CTIOT_INT;
    para->paraList[count].u.ctiotInt.val = taskIdItem->valueint;
    para->paraList[count].paraName = "taskId";
    count ++;

    cJSON *c = payloadItem->child;
    while(c != NULL)
    {
        switch(c->type){
            case cJSON_False:
            case cJSON_True:
                para->paraList[count].ctiotParaType = CTIOT_BOOL;
                para->paraList[count].u.ctiotBool.val = c->valueint;
                OS_GET_MEM(para->paraList[count].paraName,char,strlen(c->string)+1);
                if(para->paraList[count].paraName == NULL)
                {
                    goto failed;
                }
                strcpy(para->paraList[count].paraName,c->string);
                break;
            case cJSON_Number:
                para->paraList[count].ctiotParaType = CTIOT_DOUBLE;
                para->paraList[count].u.ctiotDouble.val = c->valuedouble;
                OS_GET_MEM(para->paraList[count].paraName,char,strlen(c->string)+1);
                if(para->paraList[count].paraName == NULL)
                {
                    goto failed;
                }
                strcpy(para->paraList[count].paraName,c->string);
                break;
            case cJSON_String:
                para->paraList[count].ctiotParaType = CTIOT_STR;
                OS_GET_MEM(para->paraList[count].u.ctiotStr.val,char,strlen(c->valuestring)+1);
                if(para->paraList[count].u.ctiotStr.val == NULL)
                {
                    goto failed;
                }
                strcpy(para->paraList[count].u.ctiotStr.val,c->valuestring);
                OS_GET_MEM(para->paraList[count].paraName,char,strlen(c->string)+1);
                if(para->paraList[count].paraName == NULL)
                {
                    OS_PUT_MEM(para->paraList[count].u.ctiotStr.val);
                    goto failed;
                }
                strcpy(para->paraList[count].paraName,c->string);
                break;
            case cJSON_NULL:
                break;
        }
        count ++;
        //CTIOT_LOG(LOG_DEBUG, "response: %s!",c->string);
        c = c->next;
        //CTIOT_LOG(LOG_DEBUG, "response: %d!",count);
    }
    para->count = count;
    cJSON_Delete(root);
    return para;
failed:
    cJSON_Delete(root);
    int i = 0; 
    while (i < count) 
    {
        OS_PUT_MEM(para->paraList[i].paraName);
        if(para->paraList[i].ctiotParaType == CTIOT_STR)
        {
            OS_PUT_MEM(para->paraList[count].u.ctiotStr.val);
        }
        i++; 
    }
    OS_PUT_MEM(para);
    return NULL;
}
#endif

CTIOT_MSG_STATUS ctiot_mqtt_msg_publish(char *topic,mqtt_qos_e qos,char* payload)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;

    CTIOT_LOG(LOG_DEBUG, "ctiot_mqtt_msg_publish:%s  !", payload);
    if (payload == NULL)
    {
        return CTIOT_PUBLISH_ERROR;
    }
    if (strlen(payload) > (MQTT_SENDBUF_SIZE - 2 - strlen(topic) - 2 - 10)) //2为MQTT头，TOPIC，2为报文序列号，10作为额外保护
    {
        return CTIOT_PUBLISH_ERROR;
    }
    //CTIOT_LOG(LOG_DEBUG, "ctiot_mqtt_msg_publish:%s !", payload);

    int rc = ctiot_mqtt_publish_msg(&g_mqtt_client, payload, strlen(payload), qos, topic);
    //OS_PUT_MEM(payload);
    if (rc != 0)
    {
        ret = CTIOT_PUBLISH_ERROR;
    }
    return ret;
}

static int ctiot_mqtt_publish_msg(mqtt_client_s *phandle, const char *msg,  uint32_t msg_len, mqtt_qos_e qos, char* topic)
{
    MQTTMessage message;
    int rc;

    if ((phandle == NULL) || (msg == NULL) || (msg_len <= 0)
        || (qos >= MQTT_QOS_MAX))
    {
        CTIOT_LOG(LOG_FATAL, "Parameter invalid");
        return CTIOT_ARG_INVALID;
    }

    if (!ctiot_mqtt_isconnected(phandle))
    {
        CTIOT_LOG(LOG_FATAL, "not connected");
        return CTIOT_ERR;
    }

    if (topic == NULL)
    {
        return CTIOT_MALLOC_FAILED;
    }
    memset(&message, 0, sizeof(message));
    message.qos = (enum QoS)qos;
    message.payload = (void *)msg;
    message.payloadlen = strlen(msg);
    rc = MQTTPublish(&phandle->client, topic, &message);
    if (rc != MQTT_SUCCESS)
    {
        CTIOT_LOG(LOG_FATAL, "MQTTPublish fail,rc %d", rc);
        return CTIOT_ERR;
    }
    else
    {
        CTIOT_LOG(LOG_INFO, "MQTTPublish payload %s", msg);
    }
    return CTIOT_OK;
}



CTIOT_MSG_STATUS ctiot_mqtt_encode_data_report_service_datareport(DATA_REPORT_SERVICE_DATAREPORT* para,char** payload)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    CTIOT_MQTT_PARA  mqttPara = { 0 };

    mqttPara.count = 3;
    mqttPara.paraList[0].ctiotParaType = CTIOT_FLOAT;
    mqttPara.paraList[0].paraName = "property_temperaturedata";
    mqttPara.paraList[0].u.ctiotFloat.min = 0.000000;
    mqttPara.paraList[0].u.ctiotFloat.max = 100.000000;
    mqttPara.paraList[0].u.ctiotFloat.val = para->property_temperaturedata;

    mqttPara.paraList[1].ctiotParaType = CTIOT_FLOAT;
    mqttPara.paraList[1].paraName = "property_humiditydata";
    mqttPara.paraList[1].u.ctiotFloat.min = 0.000000;
    mqttPara.paraList[1].u.ctiotFloat.max = 100.000000;
    mqttPara.paraList[1].u.ctiotFloat.val = para->property_humiditydata;

    mqttPara.paraList[2].ctiotParaType = CTIOT_BOOL;
    mqttPara.paraList[2].paraName = "property_motordata";
    mqttPara.paraList[2].u.ctiotBool.val = para->property_motordata;

    ret = ctiot_mqtt_validate(mqttPara);
    if (ret != CTIOT_SUCCESS)
    {
        return ret;
    }

    ret = ctiot_mqtt_msg_encode(mqttPara, payload);

    return ret;
}


CTIOT_MSG_STATUS ctiot_mqtt_encode_event_report_service_eventreport(EVENT_REPORT_SERVICE_EVENTREPORT* para,char** payload)
{
    CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
    CTIOT_MQTT_PARA  mqttPara = { 0 };

    mqttPara.count = 3;
    mqttPara.paraList[0].ctiotParaType = CTIOT_FLOAT;
    mqttPara.paraList[0].paraName = "property_temperaturedata";
    mqttPara.paraList[0].u.ctiotFloat.min = 0.000000;
    mqttPara.paraList[0].u.ctiotFloat.max = 100.000000;
    mqttPara.paraList[0].u.ctiotFloat.val = para->property_temperaturedata;

    mqttPara.paraList[1].ctiotParaType = CTIOT_FLOAT;
    mqttPara.paraList[1].paraName = "property_humiditydata";
    mqttPara.paraList[1].u.ctiotFloat.min = 0.000000;
    mqttPara.paraList[1].u.ctiotFloat.max = 100.000000;
    mqttPara.paraList[1].u.ctiotFloat.val = para->property_humiditydata;

    mqttPara.paraList[2].ctiotParaType = CTIOT_BOOL;
    mqttPara.paraList[2].paraName = "property_motordata";
    mqttPara.paraList[2].u.ctiotBool.val = para->property_motordata;

    ret = ctiot_mqtt_validate(mqttPara);
    if (ret != CTIOT_SUCCESS)
    {
        return ret;
    }

    ret = ctiot_mqtt_msg_encode(mqttPara, payload);

    return ret;
}


static void ctiot_mqtt_cmd_dn_service_cmddn_entry(MessageData *md)
{ 
	if ((md == NULL) || (md->message == NULL) || (ctiot_mqtt_modify_payload(md) != CTIOT_OK)) 
	{ 
		CTIOT_LOG(LOG_FATAL, "null point"); 
		return; 
	} 
 
	char *payload = md->message->payload;

#if (DEVICE_TRANSPARENT == 1)
	CTIOT_CB_FUNC *ctiot_callback = g_mqtt_client.ctiotCbFunc;
	if (ctiot_callback != NULL && ctiot_callback->ctiot_mqtt_cmd_dn_service_cmddn != NULL) 
	{
		CTIOT_LOG(LOG_INFO, "receive msg: %s", payload);
		ctiot_callback->ctiot_mqtt_cmd_dn_service_cmddn(payload);
	}
#else
	CMD_DN_SERVICE_CMDDN para = { 0 }; 
	CTIOT_CB_FUNC *ctiot_callback = g_mqtt_client.ctiotCbFunc;
 
	if (ctiot_callback != NULL && ctiot_callback->ctiot_mqtt_cmd_dn_service_cmddn != NULL) 
	{ 
		CTIOT_LOG(LOG_INFO, "receive msg: %s", payload);
		CTIOT_MQTT_PARA* paraHead = ctiot_mqtt_json_parsing(payload); 
		if (paraHead == NULL)
		{
			return;
		}
		int i = 0; 
		while (i < paraHead->count) 
		{ 
			char* paraname = paraHead->paraList[i].paraName; 
			if (strcmp(paraname, "taskId") == 0) 
			{ 
				para.taskId = (int)paraHead->paraList[i].u.ctiotInt.val; 
			} 
			else if(strcmp(paraname,"property_temperaturedata")==0)
			{
				para.property_temperaturedata = (float)paraHead->paraList[i].u.ctiotDouble.val;
			}
			else if(strcmp(paraname,"property_humiditydata")==0)
			{
				para.property_humiditydata = (float)paraHead->paraList[i].u.ctiotDouble.val;
			}
			else if(strcmp(paraname,"property_motordata")==0)
			{
				para.property_motordata = (int)paraHead->paraList[i].u.ctiotDouble.val;
			}
            
			//OS_PUT_MEM(paraname); /* Modify by OneOS Team */
			if (i > 0)
			{
				OS_PUT_MEM(paraname);/* paraHead->paraList[0].paraName = "taskId" is not from OS_GET_MEM, do not use OS_PUT_MEM */
			}
			i++; 
		} 
		ctiot_callback->ctiot_mqtt_cmd_dn_service_cmddn(&para); 

		OS_PUT_MEM(paraHead);
	}
#endif

}

CTIOT_MSG_STATUS ctiot_mqtt_encode_cmd_response_service_cmddnresponse(CMD_RESPONSE_SERVICE_CMDDNRESPONSE* para,char** payload)
{
	CTIOT_MSG_STATUS ret = CTIOT_SUCCESS;
	CTIOT_MQTT_PARA  mqttPara = { 0 };
	mqttPara.count = 3;
	mqttPara.paraList[0].ctiotParaType = CTIOT_FLOAT;
	mqttPara.paraList[0].paraName = "property_temperaturedata";
	mqttPara.paraList[0].u.ctiotFloat.min = 0.000000;
	mqttPara.paraList[0].u.ctiotFloat.max = 100.000000;
	mqttPara.paraList[0].u.ctiotFloat.val = para->property_temperaturedata;

	mqttPara.paraList[1].ctiotParaType = CTIOT_FLOAT;
	mqttPara.paraList[1].paraName = "property_humiditydata";
	mqttPara.paraList[1].u.ctiotFloat.min = 0.000000;
	mqttPara.paraList[1].u.ctiotFloat.max = 100.000000;
	mqttPara.paraList[1].u.ctiotFloat.val = para->property_humiditydata;

	mqttPara.paraList[2].ctiotParaType = CTIOT_BOOL;
	mqttPara.paraList[2].paraName = "property_motordata";
	mqttPara.paraList[2].u.ctiotBool.val = para->property_motordata;



	ret = ctiot_mqtt_validate(mqttPara);
	if (ret != CTIOT_SUCCESS)
	{
		return ret;
	}

	ret = ctiot_mqtt_msg_response_encode(mqttPara, para->taskId, payload);

	return ret;
}


int ctiot_mqtt_subscribe (void* mhandle) 
{ 
	struct mqtt_client_tag_s *handle = mhandle;
	char *topic; 
	CTIOT_CB topic_callback = NULL; 
	int rc; 
	
	if (handle->sub_topic) 
	{ 
		(void)MQTTSetMessageHandler(&handle->client, handle->sub_topic, NULL); 
		OS_PUT_MEM(handle->sub_topic); 
		handle->sub_topic = NULL; 
	} 

#if (DEVICE_TRANSPARENT == 1)
	topic = "device_control";
#else
	topic = "service_cmddn";
#endif
	topic_callback = ctiot_mqtt_cmd_dn_service_cmddn_entry;
	(void)MQTTSetMessageHandler(&handle->client, topic, topic_callback);
	CTIOT_LOG(LOG_INFO, "subcribe static topic: %s", topic);

	rc = MQTT_SUCCESS; 
	return rc;
}

