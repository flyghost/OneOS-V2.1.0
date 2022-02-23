#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <fal/fal.h>
#include <vfs_fs.h>
#include "os_memory.h"
#include "vfs_posix.h"
#include "core/ams_core.h"
#include "ams_file.h"
#include "core/ams_md5.h"
#ifdef AMS_USING_PLATFORM
#include <sys/socket.h>
#endif

/* encapsulate functions of file system */
/*                     file operations                       */
#define AMS_AUTO_START_CFG_PATH     "/ams_auto_cfg.ini"
static char g_ams_file_list[512] = {0};

static ams_script_cfg_t g_ams_script_cfg[AMS_APP_TYPE_MAX] = {
        {".py", mpy_file_entry},
        {".js", NULL}, /* TO-DO */
 };
static int g_ams_vm_status = AMS_OK;
 
static int ams_file_check_stat(const char *filepath)
{
    struct stat rst = {0};
    return stat(filepath, &rst);
}

int ams_check_file_exist(const char * file_path)
{
    int ret = AMS_ERROR;
    
    ret = ams_file_check_stat(file_path);
    if(ret != AMS_OK)
    {
        ams_log("The file[%s] may not be existed.", file_path);
    }

    return ret;
}

int ams_is_single_file(const char *main_class)
{
    return ((strncmp(main_class , "null" , strlen("null")) == 0) ? 1 : 0);
}

int ams_save_app_file(int sock, int app_length, char *file_name)
{
    char *recv_data = NULL;
    int fd = -1;
    int totalRecv = 0;
    int diff = 0, bytes_received = 0;
    int ret = AMS_ERROR;

    if(ams_check_file_exist(file_name) == AMS_OK) 
    {
        ams_log("App exist, delete it first!!!");
        unlink(file_name);
    }

    ams_log("Open file[%s].",file_name);
    fd = open(file_name, O_WRONLY | O_CREAT); 
    if (fd < 0)
    {
        ams_err("Open file[%s] failed[%d].", file_name, fd);
        return AMS_ERROR;
    }

    recv_data = ams_calloc(1, AMS_PACKAGE_LEN + 1);
    if (recv_data == NULL) 
    {
        ams_err(" No memory!");
        goto __exit;
    }
    memset(recv_data , 0 , AMS_PACKAGE_LEN + 1);
    while(totalRecv < app_length) 
    {
        diff = app_length - totalRecv;
        ams_log("diff =%d, applen =%d, totalRecv=%d.", diff, app_length, totalRecv);
#ifdef AMS_USING_PLATFORM
        bytes_received = recv(sock, 
                                recv_data, 
                                ((diff >= AMS_PACKAGE_LEN) ? AMS_PACKAGE_LEN : diff), 
                                0);
#endif
        if (bytes_received <= 0) 
        {
            ams_err("Received[%d] error, close the socket.", sock);
            goto __exit;
        }

        totalRecv += bytes_received;
        if (write(fd, recv_data, bytes_received) < 0)
        {
            ams_err("write error, close the socket."); 
            goto __exit;
        }
    }
    vfs_fsync(fd);
    ams_log(" install app finish!!!");
    ret = AMS_OK;
__exit:
    if (fd)
    {
        close(fd); 
    }
    ams_free(recv_data);
    return ret;
}

char *ams_scan_files(void)
{
    DIR *dir = NULL;
    struct dirent *dir_param = NULL;

    dir = opendir(AMS_FILE_ROOT_PATH);
    if (dir == NULL)
    {
        ams_err("Open path[%s] failed.", AMS_FILE_ROOT_PATH);
        return NULL;
    }
    memset(g_ams_file_list, 0, sizeof(g_ams_file_list));
    while((dir_param = readdir(dir)) != NULL) 
    {
        ams_log("Dir[type=%d, name:%s].", dir_param->d_type, dir_param->d_name);
        if (dir_param->d_type == DT_DIR)
        {
            continue;
        }

        if((strlen(dir_param->d_name) - strlen(ams_get_file_ext_name())) != 32)
        { // the length of md5 value is 32
            continue;
        }
        if(strlen(g_ams_file_list) != 0)
        {
            strcat(g_ams_file_list, ";");
        }
        strcat(g_ams_file_list , dir_param->d_name);
        /* if too many app name need to collect, we must consider the head len(20) of send info */
        if (strlen(g_ams_file_list) + strlen(ams_get_file_ext_name()) + 32 >= 512 - 20)
        {
            ams_err("File list already full!");
            break;
        }
    }
    ams_log("Scan file finished.");
    closedir(dir);
    
    return g_ams_file_list;
}

int ams_delete_directory(const char * dir_name)
{
    int ret = 0;
    DIR *dir = NULL;
    char tmp[64] = {0};
    struct dirent *dir_param = NULL;

    sprintf(tmp, "/%s", dir_name);
    dir = opendir(tmp);
    if (NULL == dir)
    {
        ams_err("Open dir: %s failed.", tmp);
        return AMS_ERROR;
    }
    
    while((dir_param = readdir(dir)) != NULL)
    {
        if (!dir_param->d_name[0])
        {
            break;
        }
        if (dir_param->d_name[0] == '.')
        {
            continue;
        }

        memset(tmp, 0, sizeof(tmp));
        sprintf(tmp, "/%s/%s", dir_name, dir_param->d_name);
        ams_log("File in DIR is [%s].", tmp);
        ret = unlink(tmp);
        if(ret != AMS_OK)
        {
            ams_err("Delete dir: %s failed[%d].", tmp, ret);
        }
    }
    closedir(dir);

    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp , "/%s", dir_name);
    ams_log("File in DIR is [%s]\n", tmp);
    if((ret = unlink(tmp)) != AMS_OK) 
    {
        ams_err("Delete dir: %s failed[%d].", tmp, ret);
        return AMS_ERROR;
    } 
    return AMS_OK;
}

static void ams_read_jz_file(int jzfile_fd, ams_jz_file_info_t *pack)
{
    uint8_t tmplen[5];

    read(jzfile_fd , &pack->name_len, 1);
    read(jzfile_fd , pack->file_name, pack->name_len);
    read(jzfile_fd , &tmplen , 4);
    read(jzfile_fd , pack->content_md5 , 32);

    pack->content_len = (tmplen[3] << 0) | (tmplen[2] << 8) | (tmplen[1] << 16) | (tmplen[0] << 24);
    ams_log("Jzfile info:{name: %s, namelen:%d, contentlen: %d, contentmd5: %s}.", 
            pack->file_name, pack->name_len, pack->content_len, pack->content_md5);
    return;
}

static char *ams_get_file_md5(const char *file_name)
{
    static char md5str[33];
    MD5_CTX md5;
    unsigned char tmp[40];
    unsigned char decrypt[16];
    int fd = 0;
    int len = 0;
    int totalRead = 0 ,curRead = 0, diff_len;
    
    fd = open(file_name, O_RDONLY); 
    if (fd < 0)
    {
        ams_err("Open file[%s] failed[%d].", file_name, fd);
        return NULL;
    }

    len = lseek(fd , 0, SEEK_END);
    if (len <= 0)
    {
        ams_err("Lseek file[%s] length failed.", file_name);
        close(fd); 
        return NULL;
    }
    ams_log("The len: %d", len);
    lseek(fd , 0, SEEK_SET);

    MD5Init(&md5);
    while (totalRead < len) 
    {
        memset(tmp, 0 , sizeof(tmp));
        diff_len = len - totalRead;
        curRead = read(fd, &tmp[0], ((diff_len >= 32) ? 32 : diff_len));
        MD5Update(&md5, &tmp[0], curRead);
        totalRead += curRead;
    }

    close(fd); 
    MD5Final(decrypt, &md5); 
    memset(md5str , 0 , sizeof(md5str));
    for (int i=0; i<16; i++) 
    {
        memset(tmp, 0, sizeof(tmp));
        sprintf((char *)tmp, "%02x", decrypt[i]);
        strcat(md5str, (char *)tmp);
    }
    ams_log("Mdstr is %s.", md5str);
    return md5str;
}

static int ams_unpack_jz_file(int jz_fd, const char *jz_file, int file_num)
{
    ams_jz_file_info_t jz_file_info = {0};
    char inner_file_path[128] = {0};
    int inner_file_fd = -1;
    char tmp[2] = {0};
    int total_read = 0;
    
    while (file_num)
    {
        memset(&jz_file_info, 0, sizeof(ams_jz_file_info_t));
        ams_read_jz_file(jz_fd, &jz_file_info);

        memset(inner_file_path, 0, 128);
        sprintf(inner_file_path, AMS_FILE_ROOT_PATH"%s/%s", jz_file, jz_file_info.file_name);
        inner_file_fd = open(inner_file_path, (O_WRONLY | O_CREAT));
        if (inner_file_fd < 0)
        {
            ams_err("Open file[%s] failed.", inner_file_path);
            return AMS_ERROR;
        }

        total_read = 0;
        while (total_read < jz_file_info.content_len)
        {
            memset(tmp , 0 , sizeof(tmp));
            read(jz_fd, tmp, 1);
            write(inner_file_fd, tmp, 1);
            total_read++;
        }
        vfs_fsync(inner_file_fd);
        close(inner_file_fd);

        if (strncmp(ams_get_file_md5(inner_file_path), 
                    jz_file_info.content_md5, 
                    strlen(jz_file_info.content_md5)) == 0)
        {
            ams_log("verify [file: %s, md5: %s] success!", jz_file_info.file_name, jz_file_info.content_md5);
        }
        else
        {
            ams_err("verify [file: %s , md5: %s] failed!", jz_file_info.file_name, jz_file_info.content_md5);
            return AMS_ERROR;
        }
        file_num--;
    }
    
    return AMS_OK;
}

int ams_unpack_app(const char *jz_file)
{
    uint8_t file_num = 0;
    char jz_file_path[40] = {0};
    int jz_fd = -1;
    int ret = AMS_ERROR;
    
    if (ams_check_file_exist(jz_file) == AMS_OK)
    {
        ams_delete_directory(jz_file);
    }
    
    if (mkdir(jz_file, 0777) == AMS_OK)
    {
        sprintf(jz_file_path, AMS_FILE_ROOT_PATH"%s.jz", jz_file);
        jz_fd = open(jz_file_path, O_RDONLY);
        if (jz_fd < 0)
        {
            ams_err("Open jz file[%s] failed[%d].", jz_file_path, jz_fd);
            return AMS_ERROR;
        }

        (void)read(jz_fd, &file_num, 1);
        if (file_num == 0)
        {
            ams_err("File in jz file[%s] is none.", jz_file_path);
            close(jz_fd);
            return AMS_ERROR;
        }
        ams_log("File in jz file[%s] is %d.", jz_file_path, file_num);
        ret = ams_unpack_jz_file(jz_fd, jz_file, file_num);
    }
    else
    {
        ams_err("Make dir[%s] failed.", jz_file);
    }

    close(jz_fd);
    return ret;
}

void ams_run_file_thd_entry(void *argument)
{
    g_ams_vm_status = g_ams_script_cfg[AMS_APP_TYPE_CHOICE].thd_entry((char *)argument);
    while(1)
    {
        os_task_msleep(5000);
    }
}

char *ams_get_file_ext_name(void)
{
    return g_ams_script_cfg[AMS_APP_TYPE_CHOICE].ext_name;
}

int ams_get_vm_status(void)
{
    return g_ams_vm_status;
}

int ams_get_auto_app_path(char *script_name)
{
    int fd = 0;
    int name_len = 0;
    
    if (ams_check_file_exist(AMS_AUTO_START_CFG_PATH) != AMS_OK)
    {
        ams_log("No need to auto start.");
        return AMS_ERROR;
    }

    strcat(script_name, "/");
    fd = open(AMS_AUTO_START_CFG_PATH, O_RDONLY);
    if (fd < 0)
    {
        ams_err("Open file[%s] failed.", AMS_AUTO_START_CFG_PATH);
        return AMS_ERROR;
    }
    name_len = lseek(fd , 0 , SEEK_END);
    lseek(fd , 0 , SEEK_SET);
    read(fd, &script_name[1], name_len);
    close(fd);

    /* 如果是单文件,则需要添加后缀名 */
    if (name_len <= (AMS_APP_NAME_LEN - 1))
    {
        strcat(script_name, ams_get_file_ext_name());
    }

    return AMS_OK;
}

int ams_set_auto_app_path(char *app_name, char *main_class)
{
    int fd = 0;
    if (ams_check_file_exist(AMS_AUTO_START_CFG_PATH) == AMS_OK)
    {
        unlink(AMS_AUTO_START_CFG_PATH);
    }
    
    fd = open(AMS_AUTO_START_CFG_PATH, O_WRONLY | O_CREAT);
    if (fd < 0)
    {
        ams_err("Open file[%s] failed.", AMS_AUTO_START_CFG_PATH);
        return AMS_ERROR;
    }
    
    write(fd, app_name, strlen(app_name));
    if (!ams_is_single_file(main_class))
    {
        write(fd, "/", strlen("/"));
        write(fd, main_class, strlen(main_class));
    }
    fsync(fd);
    close(fd);
    return AMS_OK;
}

void ams_del_auto_app_path(const char *app_name)
{
    char tmp[AMS_APP_NAME_LEN] = {0};
    int fd = 0;

    if(ams_check_file_exist(AMS_AUTO_START_CFG_PATH) != AMS_OK)
    {
        return;
    }

    fd = open(AMS_AUTO_START_CFG_PATH, O_RDONLY); 
    if (fd >= 0) 
    {
        read(fd, tmp, AMS_APP_NAME_LEN - 1);
    }
    close(fd); 

    if(strncmp(app_name , tmp , AMS_APP_NAME_LEN - 1) == 0) 
    {
        unlink(AMS_AUTO_START_CFG_PATH);
    }
    
    return;
}

int ams_mount_file_system(char *file_dev_name)
{
    int32_t ret = AMS_ERROR;
    
#ifdef AMS_FS_DEVICE_TYPE_CHOICE_FLASH
    if (fal_blk_device_create(file_dev_name))
    {
        ams_log("Create a block device on the %s partition of flash successful.", file_dev_name);
    }
    else
    {
        ams_log("Can't create a block device on '%s' partition.", file_dev_name);
        return ret;
    }
#else
    os_device_t *device = NULL;
    
    device = os_device_find(file_dev_name);
    if (NULL == device)
    {
        ams_err("Find device: %s failed.", file_dev_name);
        return AMS_ERROR;
    }

    ret = os_device_open(device);
    if (!ret)
    {
        ams_err("Open device: %s failed[%d].", file_dev_name, ret);
        return AMS_ERROR;
    }
#endif

    ret = vfs_mount(file_dev_name, "/", "fat", 0, 0);
    if (AMS_OK != ret)
    {
        ams_err("Mount fs on dev: %s failed.", file_dev_name);
    }

    return ret;
}


