#ifndef _AMS_PORT_H_
#define	_AMS_PORT_H_


#include "state_machine.h"
#define AMS_SERVER_PORT         28765
typedef sm_err_t		ams_err_t;
typedef sm_size_t		ams_size_t;
typedef signed long 	ams_off_t;
typedef void *			ams_thread_t;
typedef void *			ams_timer_t;

#define AMS_TIMER_FLAG_ONESHOT      0X1
#define AMS_TIMER_FLAG_PERIODIC     0x2     /* Periodic timer. */
#define AMS_TIMER_FLAG_SOFT_TIMER   0x4     /* Soft timer,the timer's callback function will be called in timer task. */

#define AMS_FILE_FLAG_RDONLY    0
#define AMS_FILE_FLAG_WR_AND_CR 1 

typedef enum AMS_DIR_TYPE {
    AMS_DIR_TYPE_DIR = 0,   /* directory */
    AMS_DIR_TYPE_REG,       /* regular file */
    AMS_DIR_TYPE_ERROR,     /* wrong type */
} AMS_DIR_TYPE_T;

typedef enum AMS_SOCK_PROT_TYPE {
    AMS_SOCK_PROT_TCP = 0,
    AMS_SOCK_PROT_UDP,
    AMS_SOCK_PROT_RAW,
} AMS_SOCK_PROT_TYPE_T;

typedef enum AMS_SOCK_DOMAIN_TYPE {
    AMS_SOCK_DOMAIN_V4 = 0,
    AMS_SOCK_DOMAIN_V6,
    AMS_SOCK_DOMAIN_UNSPEC,
} AMS_SOCK_DOMAIN_TYPE_T;



struct ams_sockaddr
{
    uint8_t sa_len;
    uint8_t sa_family;
    char    sa_data[14];
};

typedef struct ams_entry_param
{
    uint32_t    runner_stack_size; 
    uint32_t    runner_priority;
    uint32_t    ams_stack_size;
    uint32_t    ams_priority; 
    fun_i_1_t   mpy_entry;	//micropython entry -> micropy_entry(mpy_script);
} ams_entry_param_t;

#define AMS_EOK         SM_EOK
#define AMS_ERROR       SM_ERROR
#define AMS_WAITING_NO  OS_WAITING_NO
#define AMS_NULL        SM_NULL

typedef enum AMS_SEEK_WHENCE_TYPE {
    AMS_SEEK_SET = 0,
    AMS_SEEK_CUR = 1,
    AMS_SEEK_END = 2,
} AMS_SEEK_WHENCE_TYPE_T;


/**
 *********************************************************************************************************
 *                                      Inherited the log macro from  model-log  
 *********************************************************************************************************
*/
#define ams_kprintf(msg ,...)	model_kprintf(msg, ##__VA_ARGS__)


#define ams_log(msg, ...)	MODEL_LOG("[AMS]", msg, ##__VA_ARGS__)
#define ams_err(msg, ...)	MODEL_ERR("[AMS]", msg, ##__VA_ARGS__)


/**
 *********************************************************************************************************
 *                                      thread interface
 *********************************************************************************************************
*/
ams_thread_t ams_thread_create(const char *name,
                            fun_0_1_t entry,
                             void       *parameter,
                             uint32_t stack_size,
                             uint8_t  priority,
                             uint32_t tick);

ams_err_t ams_thread_startup(ams_thread_t thread);
ams_err_t ams_thread_delete(ams_thread_t thread);

#define ams_thread_mdelay(ms) sm_mdelay(ms)

/**
 *********************************************************************************************************
 *                                      timer interface
 *********************************************************************************************************
*/
ams_timer_t ams_timer_create(const char *name, 
                             fun_0_1_t 	entry, 
                             void       *parameter, 
                             uint32_t   time, 
                             uint32_t    flag);
int32_t ams_timer_start(ams_timer_t timer);
int32_t ams_timer_stop(ams_timer_t timer);							 

/**
 *********************************************************************************************************
 *                                      ascii c interface
 *********************************************************************************************************
*/
void *ams_memset(void*s,int c,size_t n);
char *ams_strcat(char * dest, const char * src);
char *ams_strncpy(char *dest, const char *src, size_t n);
int ams_strncmp(const char *cs,const char *ct, size_t count);
size_t ams_strlen(const char *s);
char *ams_strcpy(char *dest, const char *src);
int  ams_sprintf(char *buf, const char *format, ...);



/**
 *********************************************************************************************************
 *                                      system interface
 *********************************************************************************************************
*/
void *ams_calloc(ams_size_t count, ams_size_t size);
void ams_free(void *ptr);



/**
 *********************************************************************************************************
 *                                      file system interface
 *********************************************************************************************************
*/
int ams_open(const char *file, int flags);
int ams_close(int d);

int ams_read(int fd, void *buf, size_t len);
int ams_write(int fd, const void *buf, size_t len);
ams_off_t ams_lseek(int fd, ams_off_t offset, int whence);
int ams_fsync(int fildes);
int ams_f_check_stat(const char *path);
int ams_f_unlink (const char* path);


int ams_dir_readdir(void *dp, void *ams_dirent);
void* ams_dir_opendir(const char *path);
int ams_dir_closedir(void *dp);
int ams_dir_mkdir   (const char* path);
int ams_dir_chdir   (const char* path);
char *ams_dir_readdir_name(void *dp);
int ams_get_dir_type(uint32_t os_dir_type);



/**
 *********************************************************************************************************
 *                                      socket interface
 *********************************************************************************************************
*/
int ams_recv(int s, void *mem, size_t len, int flags);
int ams_send(int socket, const void *data, size_t size, int flags);
int ams_connect(int socket, const char *saddr, unsigned int port);
int ams_socket(int domain, int type, int protocol);
int ams_closesocket(int socket);
int ams_set_timeout(int fd, unsigned int timeout);
char *ams_getserveraddr(void *arg);

void ams_destroy_sockaddr(void *sockaddr);

int get_imei(void);
int get_imsi(void);
int get_iccid(void);


void ams_reboot(void);




struct _ams_struct
{
	int cmd;
	int reconnect_count;
	char _body[600];
	char _type;
	short _typeID;
	char _uniqueID[17];
	
	
	char imei[32];  //15
	char imsi[32];  //15
	char iccid[32]; //

	char mpyrunning[33];

	//downlaod app
	char appname[33];
	char appurl[196];
	char _autostart[1];
	unsigned int   autostart;
	char _startnow[1];
	unsigned int   startnow;
	char mainclass[64];
	char _applen[8];
	unsigned int   applen;

	unsigned int   forceInstall;
};

struct ams_file_fun
{
    fun_i_1_t   check_stat;
    fun_i_1_t   unlink;
    fun_i_2_t   open;
    fun_i_1_t   close;
    fun_i_3_t   read;
    fun_i_3_t   write;
    fun_i_3_t   lseek;
    fun_i_1_t   fsync;
};

typedef struct ams_dirent {
    unsigned char  d_type;
    char           d_name[256];
} ams_dirent_t;

struct ams_dir_fun
{
    fun_i_2_t   readdir;
    fun_p_1_t   opendir;
    fun_i_1_t   closedir;
    fun_i_2_t   mkdir;
    fun_i_1_t   chdir;
    fun_i_1_t   get_dir_type;
};

struct ams_socket_fun
{
    fun_i_4_t recv;
    fun_i_4_t send;
    fun_i_3_t ams_connect;
    fun_i_3_t socket;
    fun_i_1_t closesocket;
    fun_i_2_t   set_timeout; 
    fun_p_1_t get_server_addr;
};

struct ams_misc_fun{
	fun_0_0_t	reboot;
	fun_p_6_t 	create_task;
	fun_i_1_t	run_task;
	fun_i_1_t	delete_task;
	fun_p_0_t	get_imei;
	fun_p_0_t	get_imsi;
	fun_p_0_t	get_iccid;
	fun_p_5_t	create_timer;
	fun_i_1_t	run_timer;
	fun_i_1_t	stop_timer;
	
	struct model_misc_fun *sm_fun;
};


struct ams_file_fun *ams_port_get_file_structure(void);

struct ams_dir_fun *ams_port_get_dir_structure(void);

struct ams_socket_fun *ams_port_get_socket_structure(void);

struct ams_misc_fun *ams_port_get_misc_structure(void);

ams_entry_param_t *ams_get_entry_data_structure(void);

int start_ams_component(void);

void init_ams(void);

#endif

