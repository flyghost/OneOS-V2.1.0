#include "mpconfigport.h"
#ifdef MICROPY_USING_AMS
#include <sys/unistd.h>
#include <sys/socket.h>
#include <vfs_fs.h>
#include "board.h"
#include "vfs_posix.h"
#include "ams.h"
#include "usr_misc.h"

#ifndef  MICROPY_USING_FILESYSTEM
#error "Application management system(AMS) need filesystem, open filesystem please!"
#else
#include <os_util.h>
#endif

int set_ams_file_lseek(int fd, off_t offset, int whence)
{
    switch (whence)
    {
    case AMS_SEEK_SET:
        return lseek((int)fd, (off_t)offset, SEEK_SET);
    case AMS_SEEK_CUR:
        return lseek((int)fd, (off_t)offset, SEEK_CUR);
    case AMS_SEEK_END:
        return lseek((int)fd, (off_t)offset, SEEK_END);
    }

    return -1;
}

static int check_ams_file_stat(const char *filepath)
{
    struct stat rst = {0};
    return stat(filepath, &rst);
}

static int set_ams_file_open(const char *file_name, uint32_t ams_flag)
{
    uint32_t os_flag = 0 ;
    switch (ams_flag)
    {
    case AMS_FILE_FLAG_RDONLY:
        os_flag = O_RDONLY;
        break;
    case AMS_FILE_FLAG_WR_AND_CR:
        os_flag = (O_WRONLY | O_CREAT);
        break;
    default :
        mp_err("Open file[%s] failed, flag is [%d].", file_name, ams_flag);
        break;
    }

    return open(file_name, os_flag);
}

static void register_ams_file_api(void)
{
    struct ams_file_fun *file_fun = ams_port_get_file_structure();

    file_fun->check_stat    = (fun_i_1_t)check_ams_file_stat;
    file_fun->unlink        = (fun_i_1_t)unlink;
    file_fun->open          = (fun_i_2_t)set_ams_file_open;
    file_fun->close         = (fun_i_1_t)close;
    file_fun->read          = (fun_i_3_t)read;
    file_fun->write         = (fun_i_3_t)write;
    file_fun->lseek         = (fun_i_3_t)set_ams_file_lseek;
    file_fun->fsync         = (fun_i_1_t)vfs_fsync;
}

int set_ams_readdir(DIR *dp, ams_dirent_t *ams_dir)
{
    struct dirent *dirp = readdir(dp);
    
    if (dirp == NULL || ams_dir == NULL)
    {
        return MP_ERROR;
    }
    
    ams_dir->d_type = dirp->d_type;
    strcpy(ams_dir->d_name, dirp->d_name);

    return MP_EOK;
}

static uint32_t get_ams_dir_type(uint32_t os_dir_type)
{
    AMS_DIR_TYPE_T ams_dir = AMS_DIR_TYPE_ERROR;
    
    switch (os_dir_type)
    {
    case DT_DIR:
        ams_dir = AMS_DIR_TYPE_DIR;
        break;
    case DT_REG:
        ams_dir = AMS_DIR_TYPE_REG;
        break;
    }

    return ams_dir;
}

static void register_ams_dir_api(void)
{
    struct ams_dir_fun *dir_fun = ams_port_get_dir_structure();

    dir_fun->readdir =(fun_i_2_t)set_ams_readdir;
    dir_fun->opendir =(fun_p_1_t)opendir;
    dir_fun->closedir =(fun_i_1_t)closedir;
    dir_fun->mkdir =(fun_i_2_t)mkdir;
    dir_fun->chdir =(fun_i_1_t)chdir;
    dir_fun->get_dir_type = (fun_i_1_t)get_ams_dir_type;
}

int set_ams_socket_timeout(int socket_id, uint32_t timeout)
{
    struct timeval recv_time = {
        .tv_sec = timeout, 
        .tv_usec = 0
    };
    
    return setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, &recv_time, sizeof(recv_time));
}

char *get_ams_server_addr(void *arg)
{
    return AMS_SERVER_IP_ADDR;
}

int set_ams_connect(int sock, const char *saddr, uint32_t port)
{
    struct hostent *host;
    struct sockaddr_in server_addr;

    host = gethostbyname(saddr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0) 
    {
        mp_err(" socket:%d --> Connect fail!", sock);
        return MP_ERROR;
    }

    return MP_EOK;
}

static int turn_amsdomain_2_osdomain(int ams_domain_type)
{
    switch (ams_domain_type)
    {
    case AMS_SOCK_DOMAIN_V4:
        return AF_INET;
    case AMS_SOCK_DOMAIN_V6:
        return AF_INET6;
    case AMS_SOCK_DOMAIN_UNSPEC:
        return AF_UNSPEC;
    default :
        mp_err("Create socket failed, domain type[%d].", ams_domain_type);
        return -1;
    }
}

static int turn_amsprot_2_osprot(int ams_prot_type)
{
    switch (ams_prot_type)
    {
    case AMS_SOCK_PROT_TCP:
        return SOCK_STREAM;
    case AMS_SOCK_PROT_UDP:
        return SOCK_DGRAM;
    case AMS_SOCK_PROT_RAW:
        return SOCK_RAW;
    default :
        mp_err("Create socket failed, protocol type[%d].", ams_prot_type);
        return -1;
    }
}

int set_ams_socket(int ams_domain_type, int ams_prot_type, int protocol)
{
    int os_domain_type = turn_amsdomain_2_osdomain(ams_domain_type);
    int os_prot_type = turn_amsprot_2_osprot(ams_prot_type);

    if (os_domain_type < 0 || os_prot_type < 0)
    {
        return -1;
    }

    return socket(os_domain_type, os_prot_type, 0);
}

static void register_ams_socket_api(void)
{
    struct ams_socket_fun *socket_fun = ams_port_get_socket_structure();

    socket_fun->recv            =(fun_i_4_t)recv;
    socket_fun->send            =(fun_i_4_t)send;
    socket_fun->ams_connect     =(fun_i_3_t)set_ams_connect;
    socket_fun->socket          =(fun_i_3_t)set_ams_socket;
    socket_fun->closesocket     =(fun_i_1_t)closesocket;
    socket_fun->set_timeout     =(fun_i_2_t)set_ams_socket_timeout;
    socket_fun->get_server_addr =(fun_p_1_t)get_ams_server_addr;

    return;
}

void *create_ams_timer(const char *name, fun_0_1_t function, 
                            void *parameter, uint32_t timeout, uint8_t flag)
{
    uint8_t os_flag = 0;
    
    switch (flag)
    {
    case AMS_TIMER_FLAG_PERIODIC:
        os_flag = OS_TIMER_FLAG_PERIODIC;
        break;
    case AMS_TIMER_FLAG_ONESHOT:
        os_flag = OS_TIMER_FLAG_ONE_SHOT;
        break;
    default:
        mp_err("Create AMS timer failed, flag is %d.", flag);
        return NULL;
    }

    return os_timer_create(name, function, parameter, timeout, os_flag);
}

static void register_ams_misc_api(void)
{
    struct ams_misc_fun *misc_fun = ams_port_get_misc_structure();
    misc_fun->reboot = HAL_NVIC_SystemReset;
    misc_fun->sm_fun->calloc = (fun_p_2_t)os_calloc;
    misc_fun->sm_fun->free = os_free;
    misc_fun->sm_fun->mdelay = (fun_i_1_t)os_task_tsleep;

    misc_fun->sm_fun->format = os_vsnprintf;
    misc_fun->create_task = (fun_p_6_t)os_task_create;
    misc_fun->run_task = (fun_i_1_t)os_task_startup;
    misc_fun->delete_task = (fun_i_1_t)usr_task_delete;

    misc_fun->create_timer = (fun_p_5_t)create_ams_timer;
    misc_fun->run_timer = (fun_i_1_t)os_timer_start;
    misc_fun->stop_timer = (fun_i_1_t)os_timer_stop;
    misc_fun->get_imei = usr_misc_get_imei;
    misc_fun->get_imsi = usr_misc_get_imsi;
    misc_fun->get_iccid = usr_misc_get_iccid;

    return;
}

static void register_ams_entry_param(void)
{
    ams_entry_param_t *ams_entry = ams_get_entry_data_structure();
    
    ams_entry->ams_priority = AMS_TASK_PRIOIRTY;
    ams_entry->ams_stack_size =AMS_TASK_STACK_SIZE;
    ams_entry->runner_priority = MICROPY_RUNNER_PRIOIRTY;
    ams_entry->runner_stack_size = MICROPY_RUNNER_STACK_SIZE;
    ams_entry->mpy_entry = mpy_file_entry;

    return;
}

void init_ams(void)
{
    register_ams_entry_param();
    register_ams_misc_api();
    register_ams_socket_api();
    register_ams_dir_api();
    register_ams_file_api();

    return;
}

#endif


