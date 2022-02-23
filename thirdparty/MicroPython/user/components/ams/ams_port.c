#include <string.h>
#include "ams_port.h"

/*
 *********************************************************************************************************
 *                                        macro area
 *********************************************************************************************************
 */

#define AMS_BUFFER_SIZE		512

#define __use_test_deviceID__




extern struct _ams_struct ams;

/*
 *********************************************************************************************************
 *                                        Global variable area
 *********************************************************************************************************
 */

struct ams_file_fun         g_file_fun;

struct ams_dir_fun          g_dir_fun;

struct ams_socket_fun       g_socket_fun;

struct ams_misc_fun         g_self_misc_fun;

static ams_entry_param_t    g_entry ={0};


#if defined(RT_USING_DEVICE) && defined(RT_USING_CONSOLE)
extern  rt_device_t _console_device ;
#endif


/**
 *********************************************************************************************************
 *                                      function prototype
 *********************************************************************************************************
*/


/**
 *********************************************************************************************************
 *                                      get device id (imei, imsi, iccid)
 *********************************************************************************************************
*/
int get_imei(void)
{

	char * imei = g_self_misc_fun.get_imei();
	if (!imei || strlen(imei) != 15){
		return AMS_ERROR;
	}
	ams_strcpy((char*)ams.imei , imei); 
	return AMS_EOK;
}

int get_imsi(void)
{
	char * imsi = g_self_misc_fun.get_imsi();
	if (!imsi || strlen(imsi) != 15){
		return AMS_ERROR;
	}
	ams_strcpy((char*)ams.imsi , imsi);
	return AMS_EOK;
}

int get_iccid(void)
{
	char * iccid = g_self_misc_fun.get_iccid();
	if (!iccid || strlen(iccid) != 20){
		return AMS_ERROR;
	}		
	ams_strcpy((char*)ams.iccid , iccid);
	return AMS_EOK;
}

void ams_reboot(void)
{
	g_self_misc_fun.reboot();
}


/**
 *********************************************************************************************************
 *                                      thread interface
 *********************************************************************************************************
*/
ams_thread_t ams_thread_create(const char *name,
                              fun_0_1_t   entry,
                              void        *parameter,
                              uint32_t    stack_size,
                              uint8_t     priority,
                              uint32_t    tick)
{
	return g_self_misc_fun.create_task((void *)name, entry, parameter, (void *)stack_size, (void *)((uint32_t)priority), (void *)tick);
}


ams_err_t ams_thread_startup(ams_thread_t thread)
{
	return g_self_misc_fun.run_task(thread);
}


ams_err_t ams_thread_delete(ams_thread_t thread)
{
	return g_self_misc_fun.delete_task(thread);
}

/**
 *********************************************************************************************************
 *                                      timer interface
 *********************************************************************************************************
*/

ams_timer_t ams_timer_create(const char *name, 
                             fun_0_1_t 	entry, 
                             void       *parameter, 
                             uint32_t   time, 
                             uint32_t    flag)
{
	return g_self_misc_fun.create_timer((void *)name, entry, parameter, (void *)time, (void *)flag);
}

int32_t ams_timer_start(ams_timer_t timer)
{
	return g_self_misc_fun.run_timer(timer);
}

int32_t ams_timer_stop(ams_timer_t timer)
{
	return g_self_misc_fun.stop_timer(timer);
}



/**
 *********************************************************************************************************
 *                                      newlib c interface
 *********************************************************************************************************
*/
void *ams_memset(void*s,int c,size_t n)
{
	return memset(s,c,n);
}

char *ams_strcat(char * dest, const char * src)
{
	return strcat(dest,src);
}

char *ams_strncpy(char *dest, const char *src, size_t n)
{
	return strncpy(dest , src , n);
}

int ams_strncmp(const char *cs,const char *ct, size_t count)
{
	return strncmp(cs,ct,count);
}

size_t ams_strlen(const char *s)
{
	return strlen(s);
}

char *ams_strcpy(char *dest, const char *src)
{
	return strcpy(dest,src);
}

int  ams_sprintf(char *buf, const char *format, ...)
{
    int n;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    n = g_self_misc_fun.sm_fun->format(buf, AMS_BUFFER_SIZE, format, arg_ptr);
    va_end(arg_ptr);

    return n;	
}

void *ams_calloc(ams_size_t count, ams_size_t size)
{
	return sm_calloc(count, size);
}

void ams_free(void *ptr)
{
	sm_free(ptr);
}

/**
 *********************************************************************************************************
 *                                      file interface function
 *********************************************************************************************************
*/
int ams_f_check_stat(const char *path)
{
    return g_file_fun.check_stat((void *)path);
}

int ams_f_unlink (const char *path)
{
	return g_file_fun.unlink((void *)path);
}

int ams_open(const char *file, int flags)
{
	return g_file_fun.open((void *)file, (void *)flags);
}

int ams_close(int d)
{
	return g_file_fun.close((void *)d);
}

int ams_read(int fd, void *buf, size_t len)
{
	return g_file_fun.read((void *)fd, buf, (void *)len);
}

int ams_write(int fd, const void *buf, size_t len)
{
	return  g_file_fun.write((void *)fd, (void *)buf, (void *)len);
}

ams_off_t ams_lseek(int fd, ams_off_t offset, int whence)
{
	return g_file_fun.lseek((void *)fd, (void *)offset, (void *)whence);
}

int ams_fsync(int fildes)
{
	return g_file_fun.fsync((void *)fildes);
}

/**
 *********************************************************************************************************
 *                                      Directory api
 *********************************************************************************************************
*/
int ams_dir_readdir(void *dp, void *ams_dirent)
{
    return g_dir_fun.readdir(dp, ams_dirent);
}

void* ams_dir_opendir(const char *path)
{
    return g_dir_fun.opendir((void *)path);
}

int ams_dir_closedir(void *dp)
{
	return g_dir_fun.closedir(dp);
}

int ams_dir_mkdir(const char* path)
{
	int mode = 0777;
	return g_dir_fun.mkdir((void *)path, (void *)mode);
}

int ams_dir_chdir(const char* path)
{
	return g_dir_fun.chdir((void *)path);
}

int ams_get_dir_type(uint32_t os_dir_type)
{
    return g_dir_fun.get_dir_type((void *)os_dir_type);
}

/**
 *********************************************************************************************************
 *                                      socket interface
 *********************************************************************************************************
*/
int ams_recv(int s, void *mem, size_t len, int flags)
{
    return g_socket_fun.recv((void *)s, mem, (void *)len, (void *)flags);
}

int ams_send(int socket, const void *data, size_t size, int flags)
{
    return g_socket_fun.send((void *)socket, (void *)data, (void *)size, (void *)flags);
}

int ams_connect(int socket, const char *saddr, unsigned int port)
{
    return g_socket_fun.ams_connect((void *)socket, (void *)saddr, (void *)port);
}

int ams_socket(int domain, int type, int protocol)
{
    return g_socket_fun.socket((void *)domain, (void *)type, (void *)protocol);
}

int ams_closesocket(int socket)
{
    return g_socket_fun.closesocket((void *)socket);
}

char *ams_getserveraddr(void *arg)
{
    return g_socket_fun.get_server_addr(arg);
}

void ams_destroy_sockaddr(void *sockaddr)
{
    if (sockaddr == NULL)
    {
        ams_free(sockaddr);
    }

    return;
}

int ams_set_timeout(int fd, unsigned int timeout)
{
    return g_socket_fun.set_timeout((void *)fd, (void *)timeout);
}

/**
 *********************************************************************************************************
 *                      misc struct
 *********************************************************************************************************
*/


/**
 *********************************************************************************************************
 *                                  function structure channel interface(Connected to the outside world)
 *********************************************************************************************************
*/
struct ams_file_fun *ams_port_get_file_structure(void)
{
	return &g_file_fun;
}

struct ams_dir_fun *ams_port_get_dir_structure(void)
{
	return &g_dir_fun;
}

struct ams_socket_fun *ams_port_get_socket_structure(void)
{
	return &g_socket_fun;
}

struct ams_misc_fun *ams_port_get_misc_structure(void)
{
	g_self_misc_fun.sm_fun = model_get_misc_structure();
	return &g_self_misc_fun;
}

ams_entry_param_t *ams_get_entry_data_structure(void)
{
    return &g_entry;
}


