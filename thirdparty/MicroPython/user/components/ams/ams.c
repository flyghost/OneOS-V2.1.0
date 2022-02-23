#include <string.h>
#include "ams.h"

/*
*********************************************************************************************************
*                                        macro area
*********************************************************************************************************
*/

#define PACKAGELEN 			32


#define SCRIPT_NAME_SIZE  	128


#define INTERNAL_LOG_EN             1

#define internal_log(level, msg, ...)   \
    do                                  \
    {                                   \
        if (level)                      \
        {                               \
            ams_log(msg"\r\n", ##__VA_ARGS__);\
        }                               \
    } while(0)

/*
*********************************************************************************************************
*                                        define hash value macro 
*********************************************************************************************************
*/
#define STARTAPP_HASH	(0xc2)
#define CHECKCON_HASH	(0xa7)
#define DOWNLAPP_HASH	(0x7c)
#define RLISTAPP_HASH	(0xb2)
#define RSTOPAPP_HASH	(0x88)
#define RDELEAPP_HASH	(0x98)
#define RUNNGAPP_HASH	(0xe2)
#define RESETJVM_HASH	(0xc7)

#define EMPTY_FUNC_PARAM(param, param1)

#define PARSER_MSG_PATH(value, id, func) 	\
	case value:{							\
		ams._typeID = id;					\
		if (0 != func())					\
        {                                   \
            ams._typeID = AMS_ERROR;        \
        }                                   \
		break;								\
	}
	
/**
 *********************************************************************************************************
 *                                      store message
 *
 * @description: This function store the type(string) of source data into buffer.
 *
 * @param      : buf	: data will be push into
 * 
 *				 temp	: intermediate variable

 *				 type	：data type, it's is string.

 *				 src	：source
 *
 * @returns    : none
 *********************************************************************************************************
*/
#define STORE_MSG(buf, temp, type, src)					\
	ams_memset(temp, 0 , sizeof(temp));					\
	ams_sprintf(temp , type , src);						\
	ams_strcat(buf  , temp)

#define NORMAL_STORE_MSG(buf, type, src, ...) 			\
	ams_memset(buf, 0, sizeof(buf));					\
	ams_sprintf(buf, type, src, ##__VA_ARGS__);			\
/**
 *********************************************************************************************************
 *                                      create and register normal task
 *
 * @description: This function store the type(string) of source data into buffer.
 *
 * @param      : task		: task handle
 * 
 *				 fun_num	: function number

 *				 func		：function will be register.

 *				 msg		：describe information
 *
 * @returns    : none
 *********************************************************************************************************
*/
#define CREATE_AND_REGISTER_TASK(task, priority, index, type, node, fun_num, func, msg)		\
	state_machine_t *task = sm_create_task(priority, index, type, node);					\
    if (!task){																				\
		ams_err("failed to create state machine list %s!", #task);						\
    }																						\
    if (! sm_response_func_register(task,  fun_num, func)){									\
		ams_err("Failed to register ams %s !", msg);										\
    }

#define CREATE_AND_REGISTER_NORMAL_TASK(task, fun_num, func, msg)							\
	CREATE_AND_REGISTER_TASK(task, NORMAL_TASK_PRIORITY, 0, SM_NORMAL_TASK, SM_NULL, fun_num, func, msg)
	
	
#define CREATE_AND_REGISTER_LIST_TASK(l_task, index, node, fun_num, func, msg)				\
	CREATE_AND_REGISTER_TASK(l_task, LIST_TASK_PRIPORTY, index, SM_LIST_TASK, node, fun_num, func, msg)
	
/*
*********************************************************************************************************
*                                        define parse commond macro 
*********************************************************************************************************
*/
#define GET_START_STR(buf, stack) 									 \
	if(buf[0] >= '0' && buf[0] <= '9'){								 \
			stack = buf[0] - '0';									 \
	}					 
					 
#define GET_APP_LENGTH(buf, stack)									 \
	stack = 0 ;														 \
	for(int idx=0; idx < strlen(buf);idx++) {						 \
		stack = stack * 10 + buf[idx] - '0';						 \
	}	
	
#define PARSE_SUB_MSG(buf, key, func, stack)                                \
    do {                                                                    \
        int startPos = 0;                                                   \
        int endPos   = 0;                                                    \
        if(get_sub_message(key, &startPos , &endPos) != AMS_ERROR)          \
        {                                                                   \
            if (ams_strncmp(key, "APPURL=", ams_strlen("APPURL=")) == 0)    \
            {                                                               \
                if (endPos-(startPos + ams_strlen(key)) >= 196)             \
                {                                                           \
                    ams_err("Url is too long[%d].", (endPos-(startPos + ams_strlen(key))));                            \
                    return -1;                                                  \
                }                                                           \
            }                                                               \
            ams_memset(buf , 0 , sizeof(buf));                              \
            ams_strncpy(buf , &(ams._body[startPos + ams_strlen(key)]) ,    \
            endPos-(startPos + ams_strlen(key)));                           \
            func(buf, stack);                                               \
        }                                                                   \
    } while(0)

#define ADD_REPORT_MSG_HEAD(cmd, id, type, msg, tmp)                          \
    do {                                                            \
        ams_strcat(cmd , "[REPORTC]"); /* begin        */               \
        if(ams_strlen(id) != 0)                                         \
        {       /* device id    */                                      \
            ams_strcat(cmd , id);                                       \
        }                                                               \
        ams_strcat(cmd , SEPARATOR);   /* add separtor */               \
        STORE_MSG(cmd, tmp, type, msg);/* add code */                   \
    } while(0);

/*
*********************************************************************************************************
*                                        date type area
*********************************************************************************************************
*/
typedef unsigned int time_t;

#define ONE_MINUTE					 60*1000
#define SOCKET_TIMEOUT				 15*ONE_MINUTE

#define AMS_RECV_MSG_WAIT_FOREVER     0xffffffffU
#define AMS_RECV_MSG_WAIT_TIMEOUT     (5 * ONE_MINUTE)

/*
*********************************************************************************************************
*                                        Global variable area
*********************************************************************************************************
*/
struct _ams_struct ams;


char script_name[SCRIPT_NAME_SIZE];	

int g_socket_fd = 0xff;
int g_skstate = SOCKET_NONE;

static ams_thread_t g_ams_thread;
static ams_thread_t g_mpy_thread;

static ams_timer_t	g_network_timer;
int g_network_timer_lock = 0;


uint8_t	g_virtualmachine_status = 0;

/*
*********************************************************************************************************
*                                        Function declaration area
*********************************************************************************************************
*/
static void setup_mpy_thread(char *mpy_script);
static void statemachine_start(void *parameter);
static int check_file_exist(const char * filepath);

static int replace_mpy(char *mpy_script);

static int check_socket_connect(void);
static int register_to_server(void);
static void reset_main_socket(void * caller);

static void start_socket_timeout_timer(void);
/*
*********************************************************************************************************
*                                        Function prototype area
*********************************************************************************************************
*/

/**
 *********************************************************************************************************
 *                                      get server host
 *
 * @description: This function get server host. Prevent cleartext string.
 *
 * @param      : none
 *
 * @returns    : server host
 *********************************************************************************************************
*/

char * file_md5_verify(const char * name)
{
    MD5_CTX md5;
    static char md5str[33];
    unsigned char tmp[40];
    unsigned char decrypt[16];  
    int fd = 0;
    int len = 0;
    int totalRead = 0 ,curRead = 0, diff_len;
    
    fd = ams_open(name, AMS_FILE_FLAG_RDONLY); 
    if (fd < 0)
    {
        ams_err("Open file[%s] failed[%d].", name, fd);
        return NULL;
    }

    len = ams_lseek(fd , 0 , AMS_SEEK_END);
    if (len <= 0)
    {
        ams_err("Lseek file[%s] length failed.", name);
        return NULL;
    }
    ams_log("The len: %d", len);
    ams_lseek(fd , 0 , AMS_SEEK_SET);

    MD5Init(&md5);
    while (totalRead < len) 
    {
        memset(tmp, 0 , sizeof(tmp));
        diff_len = len - totalRead;
        curRead = ams_read(fd, &tmp[0], diff_len >= 32 ? 32 : diff_len);
        MD5Update(&md5,&tmp[0],curRead);
        totalRead += curRead;
    }

    ams_close(fd); 
    MD5Final(decrypt , &md5); 
    ams_memset(md5str , 0 , sizeof(md5str));
    for (int i=0;i<16;i++) 
    {
        STORE_MSG(md5str, (char *)tmp, "%02x", decrypt[i]);
    }
    ams_log("Mdstr is %s.", md5str);
    return md5str;
}


char* scan_python_files(void)
{
    void *dir = NULL;
    ams_dirent_t dirp = {0};

    static char list[512] = {0};
    char tmp[40] = {0};
    char autostart[33] = {0};

    ams_log("start");

    if (check_file_exist(("/cfg.ini")) == AMS_EOK)
    {
        //load autostart file name
        int fd = ams_open("/cfg.ini", AMS_FILE_FLAG_RDONLY); 
        if (fd >= 0) 
        {
            ams_read(fd, autostart, 32);
        }
        ams_close(fd); 
        ams_sprintf(list , "%s[AUTOSTART]", autostart);
    }

    if((dir = ams_dir_opendir("/")) == AMS_NULL) 
    {
        return AMS_NULL;
    }
    while(!ams_dir_readdir(dir, &dirp)) 
    {
        //ams_log("length=%d, type=%d, name:%s\n", dirp->d_namlen, dirp->d_type, dirp->d_name);
        if (ams_get_dir_type(dirp.d_type) == AMS_DIR_TYPE_DIR)
        {
            continue;
        }

        NORMAL_STORE_MSG(tmp, "%s", dirp.d_name);

        if(ams_strlen(tmp) != 35)
        { // the length of md5 value is 32, ".py"
            continue;
        }
        tmp[32]  = 0 ;
        if(ams_strncmp(autostart , tmp , 32) == 0)
        {
            continue;
        }
        if(ams_strlen(list))
        {
            ams_strcat(list , ";");
        }
        ams_strcat(list ,  tmp);

    }
    ams_log("end");
    ams_dir_closedir(dir);
    return list;
}

static int check_file_exist(const char * filepath)
{
    int ret = AMS_ERROR;
    
    if (filepath == NULL)
    {
        return AMS_ERROR;
    }

    ret = ams_f_check_stat(filepath);
    if(ret != AMS_EOK)
    {
        ams_log("The file[%s] may not be existed.", filepath);
        return AMS_ERROR;
    }

    return AMS_EOK;
}

static int check_autostart_file(const char * name)
{
	char tmp[40] = {0};
	int fd = 0;
	
	if(check_file_exist("/cfg.ini") != AMS_EOK)
	{
		return AMS_ERROR;
	}

	fd = ams_open("/cfg.ini", AMS_FILE_FLAG_RDONLY); 
	if (fd >= 0) {
		ams_read(fd, tmp, 32);
	}
	ams_close(fd); 

	if(ams_strlen(tmp) != 32 || ams_strncmp(name , tmp , 32) != 0 ) {
		return AMS_ERROR;
	}		
	return AMS_EOK;
}

static int app_is_single_file(void)
{
	return ((ams_strncmp(ams.mainclass , "null" , ams_strlen("null")) == 0) ? 1 : 0);
}

static int delete_directory(const char * dirname)
{
    int ret = 0;
    void *dir = NULL;
    char tmp[64] = {0};
    ams_dirent_t dirp = {0};

    ams_sprintf(tmp, "/%s", dirname);
    dir = ams_dir_opendir(tmp);
    if (AMS_NULL == dir)
    {
        ams_err("Open dir: %s failed.", tmp);
        return AMS_ERROR;
    }
    
    while(!ams_dir_readdir(dir, &dirp))
    {
        if (!dirp.d_name[0])
        {
            break;
        }
        if (dirp.d_name[0] == '.')
        {
            continue;
        }
        NORMAL_STORE_MSG(tmp, "/%s/%s", dirname, dirp.d_name);
        ams_log("file in DIR is [%s]\n", tmp);
        ret = ams_f_unlink(tmp);
        if(ret != AMS_EOK)
        {
            ams_err("Delete dir: %s failed[%d].", tmp, ret);
        }
    }
    ams_dir_closedir(dir);

    NORMAL_STORE_MSG(tmp , "/%s", dirname);
    ams_log("file in DIR is [%s]\n", tmp);
    if((ret = ams_f_unlink(tmp)) != AMS_EOK) 
    {
        ams_err("Delete dir: %s failed[%d].", tmp, ret);
        return AMS_ERROR;
    } 
    ams_log("[%s] success!",tmp);
    return AMS_EOK;
}

static void read_jzfile(int jzfile_fd, struct _ams_unpack_struct *pack)
{
    unsigned char tmplen[5];
    
    ams_memset(&pack->namelen , 0 , sizeof(pack->namelen));
    ams_memset(pack->name , 0 , sizeof(pack->name));
    ams_memset(&pack->contentlen , 0 , sizeof(pack->contentlen));
    ams_memset(pack->contentmd5 , 0 , sizeof(pack->contentmd5));
    ams_memset(tmplen , 0 , sizeof(tmplen));

    ams_read(jzfile_fd , &pack->namelen, 1);
    ams_read(jzfile_fd , pack->name, pack->namelen);
    ams_read(jzfile_fd , &tmplen , 4);
    ams_read(jzfile_fd , pack->contentmd5 , 32);

    pack->contentlen = (tmplen[3] << 0) | (tmplen[2] << 8) | (tmplen[1] << 16) | (tmplen[0] << 24);
    ams_log("Jzfile info:{name: %s, namelen:%d, contentlen: %d, contentmd5: %s}", 
            pack->name, pack->namelen, pack->contentlen, pack->contentmd5);
    return;
}

static int unpack_app(void)
{
    struct _ams_unpack_struct pack;

    //1.delete sub directory
    if(check_file_exist(ams.appname) == AMS_EOK) 
    {
        delete_directory(ams.appname);
    }

    //2.create new directory
    if (ams_dir_mkdir(ams.appname)== AMS_EOK)
    {
        char tmpjzfile[40] = {0};
        ams_sprintf(tmpjzfile, "/%s.jz", ams.appname);

        int fd = ams_open(tmpjzfile, AMS_FILE_FLAG_RDONLY);
        if (fd < 0)
        {
            return AMS_ERROR;
        }

        ams_read(fd, &pack.filenum, 1);
        int totalRead = 0 ;
        int fdpy = 0;
        char tmpPyName[128];
        char tmp[2];

        ams_log("Total numble of files in zip: %d.", pack.filenum);
        while (pack.filenum) 
        {
            read_jzfile(fd, &pack);

            totalRead = 0 ;
            ams_memset(tmpPyName, 0, sizeof(tmpPyName));
            ams_sprintf(tmpPyName, "/%s/%s",ams.appname, pack.name);

            fdpy = ams_open(tmpPyName, AMS_FILE_FLAG_WR_AND_CR);
            if (fdpy < 0)
            {
                ams_close(fd);
                ams_err("Open file[%s] failed.", tmpPyName);
                return AMS_ERROR;
            }
            while (totalRead < pack.contentlen)
            {
                ams_memset(tmp , 0 , sizeof(tmp));
                ams_read(fd , tmp, 1); 
                ams_write(fdpy, tmp, 1);
                totalRead += 1;
            }

            ams_fsync(fdpy);
            ams_close(fdpy); 

            if(ams_strncmp(file_md5_verify(tmpPyName), pack.contentmd5 , ams_strlen(pack.contentmd5)) == 0)
            {
                ams_log("file:%s , md5:[%s] verify success!",pack.name , pack.contentmd5);
            } 
            else 
            {
                ams_err("file:%s , md5:[%s] verify failed!",pack.name , pack.contentmd5);
                ams_close(fd);
                return AMS_ERROR;
            }
            pack.filenum--;
        }
        ams_close(fd); 
        }
    else 
    {
        return AMS_ERROR;
    }

    return AMS_EOK;
}


static int response_cmd_with_value(const char *id, int cmdtype, const char* val)
{
    char cmd[256] = {0};
    char tmp[32] = {0};

    ADD_REPORT_MSG_HEAD(cmd, id, "%d", cmdtype, tmp);

    if (val != NULL)
    {
        ams_strcat(cmd , SEPARATOR);
        ams_strcat(cmd , val);
    }

    //add line end
    ams_strcat(cmd , LINE_END);

    internal_log(INTERNAL_LOG_EN, " send cmd:[%s]", cmd);

    if (ams_send(g_socket_fd , cmd , ams_strlen(cmd) , 0) < 0 ) 
    {
        ams_err("send error, close the socket.");
        return AMS_ERROR;
    } 

    return AMS_EOK;
}



/**
 *********************************************************************************************************
 *                            [commond area] Implement download application command
 *********************************************************************************************************
*/


static int start_download_socket(void)
{
    char saddr[32] = {0};
    char sport[6] = {0};
    unsigned int port=0;
    char payload[160] = {0};
    int idx1, idx2;
    int sock = -1;

    for (idx1 = 9 ; idx1 < ams_strlen(ams.appurl) ; idx1++) 
    {
        if(ams.appurl[idx1] == ':')
        {
            break;
        }
    }

    //ams_memset(saddr , 0 ,sizeof(saddr));
    ams_strncpy(saddr , &ams.appurl[9] , idx1 - 9);
    idx1++;
    for(idx2 = idx1 ; idx2 < ams_strlen(ams.appurl) ; idx2++)
    {
        if(ams.appurl[idx2] == '/')
        {
            break;
        }
    }
    //ams_memset(sport , 0 , sizeof(sport));
    ams_strncpy(sport , &ams.appurl[idx1] , idx2 - idx1);
    for(int i=0 ; i< ams_strlen(sport) ; i++)
    {
        port = port*10 + (sport[i]-'0');
    }
    //ams_memset(payload , 0 , sizeof(payload));
    ams_strcat(payload , "GET ");
    ams_strncpy(&payload[4] , &ams.appurl[idx2 + 1] , ams_strlen(ams.appurl) - idx2);
    ams_strcat(payload , "\n");

    internal_log(INTERNAL_LOG_EN, "saddr:[%s] | sport:[%s] | port:%d | payload:[%s]\n",
                                    saddr, sport, port, payload);

    sock = ams_socket(AMS_SOCK_DOMAIN_V4, AMS_SOCK_PROT_TCP, 0);
    if (sock < 0) 
    {
        ams_err("Socket error");
        goto __exit;
    } 

    (void)ams_set_timeout(sock, 1);
    internal_log(INTERNAL_LOG_EN, "set sockect[%d] time out\n", sock);
    if (ams_connect(sock, saddr, port) < 0) 
    {
        ams_err(" socket:%d --> Connect fail!", sock);
        goto __exit;
    }

    if (ams_send(sock, payload, ams_strlen(payload), 0) <= 0) 
    {
        ams_err("send error, close the socket.");
        goto __exit;
    }
    return sock;

__exit:
    if (sock >= 0)
    {
        ams_closesocket(sock);
    }
    return AMS_ERROR;
}

static int recv_and_save_data(int sock, char *file_name)
{
    int fd = 0, diff_len = 0;
    char *recv_data = NULL;
    int responseError = AMS_EOK;
    int totalRecv = 0, bytes_received = 0;

    if (sock == AMS_ERROR)
    {
        ams_err(" Parameter is error, socket[%d]!", sock);
        return AMS_ERROR;
    }

    recv_data = ams_calloc(1, PACKAGELEN+1);
    if (recv_data == AMS_NULL) 
    {
        ams_err(" No memory!");
        return AMS_ERROR;
    }

    ams_sprintf(file_name , app_is_single_file()? "/%s.py" : "/%s.jz" , ams.appname);
    if(check_file_exist(file_name) == AMS_EOK) 
    {
        ams_f_unlink(file_name) ;
        ams_log("App exist, delete it first!!!");
    }   

    fd = ams_open(file_name, AMS_FILE_FLAG_WR_AND_CR); 
    if (fd < 0)
    {
        ams_err("Open file[%s] failed[%d].", file_name, fd);
        responseError = AMS_ERROR;
        goto __exit;
    }

    while(totalRecv < ams.applen) 
    {
        ams_memset(recv_data , 0 , PACKAGELEN+1);
        diff_len = ams.applen - totalRecv;
        //ams_log("diff_len =%d, ams.applen =%d, totalRecv=%d\n", diff_len, ams.applen, totalRecv);
        bytes_received = ams_recv(sock, recv_data, (diff_len >= PACKAGELEN)? PACKAGELEN : diff_len, 0);
        if (bytes_received <= 0) 
        {
            ams_err("received error, close the socket.");
            goto __exit;
        }

        totalRecv += bytes_received;

        if (ams_write(fd, recv_data, bytes_received) < 0)
        {
            ams_err("write error, close the socket."); 
            responseError = AMS_ERROR;
            goto __exit;
        }
    }
    ams_fsync(fd);

    ams_log(" install app finish!!!");
__exit:
    ams_closesocket(sock);
    if (fd)
    {
        ams_close(fd); 
    }
    ams_free(recv_data);
    return responseError;
}

static int command_download_app(void *param)
{
    ams_log("--------->(address: 0x%p) : start download app!", command_download_app);

    internal_log(INTERNAL_LOG_EN, "appurl:%s | mainclass:%s applen:%d ", 
                            ams.appurl, 
                            ams.mainclass, 
                            ams.applen);

    int responseError = AMS_EOK;
    char tmp[128] = {0};

    responseError = recv_and_save_data(start_download_socket(), tmp);
    if (responseError == AMS_ERROR)
    {
        ams_err("Failed to receive file from server !!!");
        goto __exit;
    }

    if (!app_is_single_file())
    {
        if(unpack_app()== AMS_ERROR) 
        { //jz ,unpack it now!
            ams_err("Failed to unpack app !!!");
            goto __exit;
        }
    }

    if (response_cmd_with_value(ams._uniqueID , RESPCODE_INSTALLOK, NULL) < 0)
    {
        responseError = AMS_ERROR;
    }

    if(ams.autostart) //need start when power on
    {
        if (check_file_exist("/cfg.ini") == AMS_EOK) 
        {
            ams_f_unlink("/cfg.ini");
        }

        ams_memset(tmp , 0 , sizeof(tmp));
        ams_strcat(tmp , "/cfg.ini" );
        int fcfg  = ams_open(tmp, AMS_FILE_FLAG_WR_AND_CR); 
        if (fcfg >= 0) 
        {
            ams_write(fcfg, ams.appname, strlen(ams.appname));

            if (!app_is_single_file()) 
            {
                ams_write(fcfg, "/", strlen("/"));
                ams_write(fcfg, ams.mainclass, strlen(ams.mainclass));
            }
            ams_fsync(fcfg);
        }
        ams_close(fcfg); 
    }

    if (ams.startnow) //need run app  at once
    {
        ams_memset(tmp , 0 , sizeof(tmp));

        if (app_is_single_file()) 
        {
            ams_sprintf(tmp , "/%s.py" , ams.appname);
        } 
        else 
        {
            ams_sprintf(tmp , "/%s/%s" , ams.appname , ams.mainclass);
        }

        if (response_cmd_with_value(ams._uniqueID , (replace_mpy(tmp) == AMS_EOK) ? RESPCODE_APPSTARTOK : RESPCODE_APPSTARTERROR, NULL) <0)
        {
            responseError = AMS_ERROR;
        }
    }

__exit:
    
    if (responseError == AMS_ERROR)
    {
        if(response_cmd_with_value(ams._uniqueID , RESPCODE_FAIL_DOWNLOAD, NULL)<0)
        {
            ams_err("Failed to report  download-app-command status!!!");
        }
        reset_main_socket(command_download_app);
    }
    ams_log("finished download-app-command!!!");

    return responseError;	
}

/**
 *********************************************************************************************************
 *                            [commond area] Implement start application command
 *********************************************************************************************************
*/
static int command_start_app(void *param)
{
    ams_log("--------->(address: 0x%p) :", command_start_app);
    ams_log("appName:%s | mainclass:%s ",ams.appname, ams.mainclass);

    int response_type = RESPCODE_APPSTARTOK;
    ams_memset(script_name , 0 , SCRIPT_NAME_SIZE);

    if (app_is_single_file()) 
    {
        ams_sprintf(script_name , "/%s.py" , ams.appname);
    } 
    else 
    {
        ams_sprintf(script_name , "/%s/%s" , ams.appname , ams.mainclass);
    }

    ams_log("Script name is [%s]!!!", script_name);
    if (check_file_exist(script_name) == AMS_ERROR) 
    {
        response_type = RESPCODE_APPNOTEXIST;
        goto _error;
    }   

    if (replace_mpy(script_name) != AMS_EOK) 
    {
        response_type = RESPCODE_APPSTARTERROR;
    }

    _error:
    if (response_cmd_with_value(ams._uniqueID , response_type, NULL)<0)
    {
        reset_main_socket(command_start_app);
    }
    ams_log("Finished start-app-command!!!");
    return ((response_type == RESPCODE_APPSTARTOK) ? (AMS_EOK): (AMS_ERROR));
}

/**
 *********************************************************************************************************
 *                            [commond area] Implement report application list command
 *********************************************************************************************************
*/
static int command_report_list(void *param)
{
    //find app root directory files number;
    char cmd[512] = {0};
    char tmp[32] = {0};
    char *listbody;

    ams_log("--------->(address: 0x%p)", command_report_list);
    ADD_REPORT_MSG_HEAD(cmd, ams._uniqueID, "%d", RESPCODE_APPLIST, tmp);

    ams_strcat(cmd , SEPARATOR);

    listbody = scan_python_files();

    ams_strcat(cmd, (ams_strlen(listbody)== 0)? "<No applications found>": listbody);

    //add line end
    ams_strcat(cmd , LINE_END);

    //send cmd to server
    internal_log(INTERNAL_LOG_EN, "send cmd:[%s]",cmd); 

    if (ams_send(g_socket_fd , cmd , strlen(cmd) , 0)<0)
    {
        ams_err("send error, close the socket.");
        reset_main_socket(command_report_list);
        return AMS_ERROR;
    }
    ams_log("Finished report-list-command !");
    return AMS_EOK;
}

/**
 *********************************************************************************************************
 *                            [commond area] Implement stop application command
 *********************************************************************************************************
*/
static int command_stop_app(void *param)
{
	ams_log("--------->(address: 0x%p) :", command_stop_app);
	//ams_thread_mdelay(3000);
	if(g_mpy_thread == AMS_NULL) {
		ams_log("command_stop_app No running App");	
		
	} else if ((ams_thread_delete(g_mpy_thread)) == AMS_EOK){
		g_mpy_thread = AMS_NULL;
		ams_memset(ams.mpyrunning , 0 , sizeof(ams.mpyrunning));
		//g_micro.stream_exit(); 
	} else {
		ams_err("stop App error!");	
		return AMS_ERROR;
	}
	
	if(response_cmd_with_value(ams._uniqueID , RESPCODE_APPFINISH , "0")<0){
		reset_main_socket(command_stop_app);
		return AMS_ERROR;
	}
	ams_log("Finished stop-app-command !");
	return AMS_EOK;
}

/**
 *********************************************************************************************************
 *                            [commond area] Implement heart beat command
 *********************************************************************************************************
*/
static int command_heart_beat(void *param)
{
    char cmd[256] = {0};
    char tmp[32] = {0};

    ams_log("[address: 0x%p] Heartbeat!", command_heart_beat);

    ADD_REPORT_MSG_HEAD(cmd, ams._uniqueID, "%d", RESPCODE_HEARTBEAT, tmp);

    //add line end
    ams_strcat(cmd , LINE_END);
    //send cmd to server
    internal_log(INTERNAL_LOG_EN, "respone Heartbeat send cmd:[%s]",cmd);
    if (ams_send(g_socket_fd , cmd , strlen(cmd) , 0)<0)
    {
        ams_err("send error, close the socket.");
        reset_main_socket(command_heart_beat);
        return AMS_ERROR;
    }
    ams_log("Finished sending heart beat !");
    return AMS_EOK;
}

/**
 *********************************************************************************************************
 *                            [commond area] Implement delete application command
 *********************************************************************************************************
*/
static int command_delete_app(void *param)
{
	ams_log("[address: 0x%p] Delete app", command_delete_app);
	char tmp[40] = {0};

	ams_sprintf(tmp, "/%s.jz", ams.appname);
	
	if(check_file_exist(tmp) == AMS_EOK) {
		//1.delete sub directory
		NORMAL_STORE_MSG(tmp , "%s" , ams.appname);
		if(check_file_exist(tmp) == AMS_EOK) {
			delete_directory(tmp);
		}
		//2.delete jz file
		NORMAL_STORE_MSG(tmp , "/%s.jz" , ams.appname);
	} else {
		NORMAL_STORE_MSG(tmp , "/%s.py" , ams.appname);			
	}
	
	if(check_file_exist(tmp) != AMS_EOK){
		ams_log("App NOT exist!!!");
		if(response_cmd_with_value(ams._uniqueID , RESPCODE_APPNOTEXIST, NULL)<0){
			reset_main_socket(command_delete_app);
		}
		return AMS_ERROR;
	}
	
	int ret = ams_f_unlink(tmp) ;
	if( ret == AMS_EOK)
	{
		if(response_cmd_with_value(ams._uniqueID , RESPCODE_DELETEOK, NULL)<0){
			reset_main_socket(command_delete_app);
		}
		if(check_autostart_file(ams.appname)) {
			if(ams_f_unlink("/cfg.ini") == AMS_EOK)
				ams_log(" Delete autostart cfg ini files!!!");				
		}			
	} else {
		ams_err(" delete file error! ret:%d",ret);	
		if(response_cmd_with_value(ams._uniqueID , RESPCODE_DELETEFAIL, NULL)<0){
			reset_main_socket(command_delete_app);
		}
		return AMS_ERROR;
	}
	ams_log("finished deleted-app-command !!!");
	return AMS_EOK;
}

/**
 *********************************************************************************************************
 *                            [commond area] Implement report running application command
 *********************************************************************************************************
*/
static int command_report_running_app(void *param)
{
    //find app root directory files number;
    char cmd[256] = {0};
    char tmp[32] = {0};
    uint8_t runningapp_length = 0;

    ams_log("[address: 0x%p] report running app", command_report_running_app);
    ADD_REPORT_MSG_HEAD(cmd, ams._uniqueID, "%d", RESPCODE_RUNNINGAPPLIST, tmp);

    //add separtor
    ams_strcat(cmd , SEPARATOR);

    runningapp_length = ams_strlen(ams.mpyrunning);
    if (g_virtualmachine_status == AMS_EOK)
    {
        ams_memset(ams.mpyrunning, 0, runningapp_length);
        runningapp_length = 0;
    }

    //add cmd body
    ams_strcat(cmd, (runningapp_length == 32) ? ams.mpyrunning : "EMPTY_LIST");

    //add line end
    ams_strcat(cmd , LINE_END);

    //send cmd to server
    internal_log(INTERNAL_LOG_EN, "respone Report RunningApp List send cmd:[%s]", cmd);   

    if (ams_send(g_socket_fd , cmd , strlen(cmd) , 0) < 0)
    {
        ams_err("send error, close the socket.");
        reset_main_socket(command_report_running_app);
        return AMS_ERROR;
    }
    
    ams_log("Finished report-running-app-command!"); 
    return AMS_EOK;
}

/**
 *********************************************************************************************************
 *                            [commond area] Implement restart virtual machine command
 *********************************************************************************************************
*/
static int command_reset_vm(void *param)
{
	ams_log("reboot now!!!");	
	ams_reboot();
	return AMS_EOK;
}

/**
 *********************************************************************************************************
 *                            [parser area] Parse massage
 *********************************************************************************************************
*/
static int get_sub_message(char * key , int *spos , int *epos)
{
	int bfind = 0 ;
	int idx1,idx2;

	for(idx1=0; idx1 < strlen(ams._body) ; idx1++) {
		if(ams_strncmp(&(ams._body[idx1]), key , ams_strlen(key)) == 0) {
			bfind = 1;
			*spos = idx1;
			break;
		}
	}

	if(!bfind){
		return AMS_ERROR;
	}
	bfind = 0 ;
	for(idx2 = idx1; idx2 < ams_strlen(ams._body); idx2++)	{
		if(ams._body[idx2] == ',' )	{
			//find sub string end pos;
			bfind = 1;
			*epos = idx2;
			break;
		}
	}
	
	if(!bfind) {
		*epos = idx2;
	}

	return AMS_EOK;
}


static int parse_start_app_cmd(void)
{
    PARSE_SUB_MSG(ams.appname,   "APPNAME=",   EMPTY_FUNC_PARAM, AMS_NULL);
    PARSE_SUB_MSG(ams.mainclass, "MAINCLASS=", EMPTY_FUNC_PARAM, AMS_NULL);
    
    internal_log(INTERNAL_LOG_EN, "ams.appname : %s, ams.mainclass : %s\n", ams.appname, ams.mainclass);
    return 0;
}


static int parse_download_app_cmd(void)
{
    PARSE_SUB_MSG(ams.appname,      "APPNAME=",     EMPTY_FUNC_PARAM,   AMS_NULL);
    PARSE_SUB_MSG(ams.appurl,       "APPURL=",      EMPTY_FUNC_PARAM,   AMS_NULL);
    PARSE_SUB_MSG(ams._autostart,   "AUTOSTART=",   GET_START_STR,      ams.autostart);
    PARSE_SUB_MSG(ams._startnow,    "STARTNOW=",    GET_START_STR,      ams.startnow);
    PARSE_SUB_MSG(ams.mainclass,    "MAINCLASS=",   EMPTY_FUNC_PARAM,   AMS_NULL);
    PARSE_SUB_MSG(ams._applen,      "APPLEN=",      GET_APP_LENGTH,     ams.applen);

    internal_log(INTERNAL_LOG_EN, "ams.appname : %s, ams.mainclass : %s, ams.appurl : %s, ams._applen : %s\n", 
                    ams.appname, ams.mainclass, ams.appurl, ams._applen);
    return 0;
}


static int parse_delete_app_cmd(void)
{
    PARSE_SUB_MSG(ams.appname, "APPNAME=", EMPTY_FUNC_PARAM, AMS_NULL);
    internal_log(INTERNAL_LOG_EN, "ams.appname : %s\n", ams.appname);
    return 0;
}

static int parse_stub(void)
{
    return 0;
}

static int  parse_message(void)
{
    
    switch(sm_compute_hash(&(ams._body[0]), 10))
    {
        PARSER_MSG_PATH(STARTAPP_HASH, COMMAND_START_APP,       parse_start_app_cmd);
        PARSER_MSG_PATH(DOWNLAPP_HASH, COMMAND_DOWNLOAD_APP,    parse_download_app_cmd);
        PARSER_MSG_PATH(RDELEAPP_HASH, COMMAND_DELETE_APP,      parse_delete_app_cmd);
        PARSER_MSG_PATH(RLISTAPP_HASH, COMMAND_LIST_APP,        parse_stub);
        PARSER_MSG_PATH(RSTOPAPP_HASH, COMMAND_STOP_APP,        parse_stub);
        PARSER_MSG_PATH(RUNNGAPP_HASH, COMMAND_RUNNING_APP,     parse_stub);
        PARSER_MSG_PATH(RESETJVM_HASH, COMMAND_RESET_JVM,       parse_stub);
        PARSER_MSG_PATH(CHECKCON_HASH, COMMAND_HEARTBEAT,       parse_stub);

        default:
        {
            ams._typeID = AMS_ERROR;
            //ams_err("unkonw command! body : %s\n", &(ams._body[0]));
        }
    }

    
    internal_log(INTERNAL_LOG_EN, "body : %s\n\n\n", &(ams._body[0]));
    if (ams_strncmp(&(ams._body[10]) , "UNIQUEID=" , 9) == 0 ) 
    {
        ams_strncpy(ams._uniqueID , &(ams._body[19]) , 16);
    }


    return ams._typeID;
}

static int receive_message(void)
{
    unsigned char b;
    int off = 0;
    int maxlen = 600;

    (void)ams_set_timeout(g_socket_fd, AMS_RECV_MSG_WAIT_TIMEOUT);
    ams_memset(ams._body , 0 , sizeof(ams._body));
    while(off < maxlen)
    {
        if (ams_recv(g_socket_fd , &b , 1 , 0) != 1)
        {
            if (check_socket_connect())
            {
                g_skstate = SOCKET_START;       // restart network
                return LIST_TASK_CREATE_SOCKET; // enter network sequence tasks.
            }
            //			if (!g_network_timer_lock){
            //				g_network_timer_lock = 1;	// enable restart network 
            //			}
                ams_thread_mdelay(50);
                continue; 
        }
        if (b == 0x0a)
        {
            break;
        }
        ams._body[off] = b;
        off++;
    }
    internal_log(INTERNAL_LOG_EN, "receive_message: [%s] , off:%d\r\n\n", ams._body, off);
    //	if (g_network_timer_lock){
    //		g_network_timer_lock = 0;	// desable restart network 
    //	}
    (void)ams_set_timeout(g_socket_fd, 0);
    return parse_message();
}



/**
 *********************************************************************************************************
 *                            [network area] Implement connect server function
 *********************************************************************************************************
*/
static int create_socket(void *sm)
{
    if ((g_skstate != SOCKET_ERR) && (g_skstate != SOCKET_START))
    {
        return AMS_ERROR;
    }
    ams_log(" start network task!!!");

    if((g_socket_fd == 0xff) && ((g_socket_fd = ams_socket(AMS_SOCK_DOMAIN_V4, AMS_SOCK_PROT_TCP, 0))) < 0)
    {
        ams_err("%s: failed to create socket!", TAG);
        reset_main_socket(create_socket);
        return AMS_ERROR; 
    } 
    else 
    {
        ams_log("(%s | socket:%d): created socket successfully!", TAG, g_socket_fd); 
        g_skstate = SOCKET_CREATE;
    }   

    return AMS_EOK;
}

static int connet_server(void *sm)
{
    if(ams_connect(g_socket_fd, ams_getserveraddr(AMS_NULL), AMS_SERVER_PORT) < 0) 
    {
        ams_err("%s:ConnectError!",TAG);
        reset_main_socket(connet_server);

        return AMS_ERROR;
    } 
    else 
    {
        ams_log("(%s | socket:%d): connected server successfully!", TAG, g_socket_fd);
        g_skstate = SOCKET_CONNET;
    }
    
    return AMS_EOK;
}

static int register_device(void *sm)
{
    if (register_to_server() < 0)
    {
        ams_err("%s:registerError, try again!", TAG);
        reset_main_socket(register_device);

        return AMS_ERROR;
    } 

    ams_log("%s: Register to server successfully!", TAG);
    return AMS_EOK;
}


static int register_to_server(void)
{
    char cmd[256] = {0};
    char tmp[32] = {0};
    ams_strcat(cmd , "[REPORTC]"); 

    ams_strcat(cmd , SEPARATOR);  
    STORE_MSG(cmd, tmp, "%d", RESPCODE_REGISTER);

    //add separtor
    ams_strcat(cmd , SEPARATOR);
    	
    //add cmd body
    ams_strcat(cmd  , "DEVICE_ID=");

    ams_strcat(cmd  , (get_imei() != AMS_ERROR)? ams.imei :"000000000000000");	

    ams_strcat(cmd  , "_");
    ams_strcat(cmd  , (get_imsi() != AMS_ERROR)? ams.imsi :"000000000000000");	

    ams_strcat(cmd  , "_");
    ams_strcat(cmd  , (get_iccid() != AMS_ERROR)? ams.iccid :"00000000000000000000");	

    STORE_MSG(cmd, tmp , ";CONN_COUNT=%d" , ams.reconnect_count);

    //add line end
    ams_strcat(cmd , LINE_END);
    //send cmd to server
    ams.reconnect_count++;
    internal_log(INTERNAL_LOG_EN, "reg2Server send cmd:[%s]",cmd);
    if(ams_send(g_socket_fd , cmd , ams_strlen(cmd) , 0) < 0) 
    {
        ams_err("send error, close the socket.");
        return AMS_ERROR;
    }
    return AMS_EOK;
}

static int check_socket_connect(void)
{
    return (g_socket_fd == 0xff || g_skstate != SOCKET_CONNET);
}


static void reset_main_socket(void * caller)
{
    ams_log("The caller is 0x%p ", caller);
    ams_closesocket(g_socket_fd);
    g_socket_fd = 0xff;
    g_skstate = SOCKET_ERR;
}

/**
 *********************************************************************************************************
 *                            [script area] operate python script 
 *********************************************************************************************************
*/
static int replace_mpy(char *mpy_script)
{
	//ams_thread_mdelay(2500);
	if(g_mpy_thread != AMS_NULL){
		if((ams_thread_delete(g_mpy_thread)) != AMS_EOK){
			ams_err("%s:Delete mpy_thread error!",TAG);
			return AMS_ERROR;
		}
		ams_log("%s:Delete mpy_thread ok!",TAG);
	} else {
		ams_log("waiting for start py thread!!!");
	}
	
	ams_thread_mdelay(100);

	ams_log("%s:run the new mpy script!",TAG);
	setup_mpy_thread(mpy_script);	
	
	return AMS_EOK;
}

static void start_py_by_config_file(void)
{
    int fd = 0;
    int len = 0;

    ams_memset(script_name , 0 , sizeof(script_name));
    ams_strcat(script_name,"/");

    fd = ams_open("/cfg.ini", AMS_FILE_FLAG_RDONLY); 

    if (fd >= 0) 
    {
        len = ams_lseek(fd , 0 , AMS_SEEK_END);
        ams_lseek(fd , 0 , AMS_SEEK_SET);
        ams_read(fd, &script_name[1], len);
    }
    ams_close(fd); 

    if (len <= 32)
    {
        ams_strcat(script_name , ".py");
    }
    ams_log("power on ,autostart run file [%s]", script_name);
    if (ams_strlen(script_name) < 32)
    {
        ams_log("cfg.ini empty!!!");
    } 
    else 
    {
        setup_mpy_thread(script_name);
    }
}

/**
 *********************************************************************************************************
 *                            [statemachine area] create and regster task 
 *********************************************************************************************************
*/

static void statemachine_start(void *parameter)
{
    ams_log("start");
    
    // create list task
    CREATE_AND_REGISTER_LIST_TASK(l_task1, AMS_NET_LIST_TASK_1, SM_SELF_NODE,
                                    LIST_TASK_CREATE_SOCKET, create_socket, "CreateSocket");
    CREATE_AND_REGISTER_LIST_TASK(l_task2, AMS_NET_LIST_TASK_2, l_task1, 
                                    LIST_TASK_CONNECT, connet_server, "ConnectServer");
    CREATE_AND_REGISTER_LIST_TASK(l_task3, AMS_NET_LIST_TASK_3, l_task1, 
                                    LIST_TASK_REGISTER, register_device, "RegisterDevice");
    // create normal task
    CREATE_AND_REGISTER_NORMAL_TASK(task1, COMMAND_DOWNLOAD_APP,
                                        command_download_app, "DownloadApp");
    CREATE_AND_REGISTER_NORMAL_TASK(task2, COMMAND_START_APP, 
                                        command_start_app, "StartApp");
    CREATE_AND_REGISTER_NORMAL_TASK(task3, COMMAND_LIST_APP, 
                                        command_report_list, "ReportList");
    CREATE_AND_REGISTER_NORMAL_TASK(task4, COMMAND_STOP_APP,
                                        command_stop_app, "StopApp");
    CREATE_AND_REGISTER_NORMAL_TASK(task5, COMMAND_HEARTBEAT,
                                        command_heart_beat, "Heartbeat");
    CREATE_AND_REGISTER_NORMAL_TASK(task6, COMMAND_DELETE_APP,
                                        command_delete_app, "DeleteAPP");
    CREATE_AND_REGISTER_NORMAL_TASK(task7, COMMAND_RUNNING_APP,
                                        command_report_running_app, "ReportRunningApp");
    CREATE_AND_REGISTER_NORMAL_TASK(task8, COMMAND_RESET_JVM,
                                        command_reset_vm, "ResetJVM");

    //sm_listall();	 //only for test show all register function!

    sm_system_run(SM_FUNCTIONNUM, receive_message);
}

static void socket_timeout(void *param)
{
    reset_main_socket(socket_timeout);
    ams_log("trigger socke timer!");
}

static void start_socket_timeout_timer(void)
{
    g_network_timer = ams_timer_create("socket_timer", 
                                        socket_timeout, 
                                        AMS_NULL, 
                                        SOCKET_TIMEOUT, 
                                        AMS_TIMER_FLAG_PERIODIC);
    if (!g_network_timer)
    {
        ams_err("Created timer falied!");
        return ;
    }

    ams_timer_start(g_network_timer);
}

/**
 *********************************************************************************************************
 *                            [thread area] create and run thread
 *********************************************************************************************************
*/
static void ams_thread_entry(void *parameter)
{
    g_skstate = SOCKET_START;
    g_network_timer_lock = 0;
    
    if (check_file_exist("/cfg.ini") == AMS_EOK) 
    {
        start_py_by_config_file();
    }

    start_socket_timeout_timer();

    sm_system_init();

    /* the server address is only used in connection, 
        and can be gotten by saddr and server port.
    */
    statemachine_start(AMS_NULL);
    return;
}

static void micropy_thread_entry(void *mpy_script)
{
    ams_entry_param_t *ams_entry = ams_get_entry_data_structure();
    
    ams_log("-------->> mpy_script:%s", (char *)mpy_script);

    char * fname = mpy_script;
    ams_memset(ams.mpyrunning , 0  ,sizeof(ams.mpyrunning));

    if (*fname == '/')
    {
        fname += 1;
    }
    
    strncpy(ams.mpyrunning, fname, 32);
    ams_log("mpyrunning name :%s",ams.mpyrunning);
    g_virtualmachine_status = (uint8_t)ams_entry->mpy_entry(mpy_script);
    while(1) 
    {
        ams_thread_mdelay(5000);
    }
}

static void setup_mpy_thread(char *mpy_script)
{
    ams_entry_param_t *ams_entry = ams_get_entry_data_structure();
    
    g_mpy_thread = ams_thread_create("mpy_thread", micropy_thread_entry, mpy_script, 
                                        ams_entry->runner_stack_size, ams_entry->runner_priority, 10);
    if (g_mpy_thread == AMS_NULL) 
    {
        ams_err("%s: setup myp thread failed", TAG);
    } 
    else 
    {
        ams_thread_startup(g_mpy_thread);
        ams_log("start mpy_thread successfully!");
    }

    return;
}

int start_ams_component(void)
{ 
    int ret = AMS_ERROR;
    ams_entry_param_t *ams_entry = ams_get_entry_data_structure();

    g_ams_thread = ams_thread_create("ams_thread", ams_thread_entry, AMS_NULL, 
                                        ams_entry->ams_stack_size, ams_entry->ams_priority, 10);
    if (g_ams_thread == AMS_NULL) 
    {
        ret = AMS_ERROR;
        ams_err("%s:setup ams thread failed", TAG);
    } 
    else 
    {
        ret = AMS_EOK;
        ams_thread_startup(g_ams_thread);
    }

    return ret;
}


