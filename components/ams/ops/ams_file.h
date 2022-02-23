#ifndef __AMS_FILE_H__
#define __AMS_FILE_H__

#define AMS_PACKAGE_LEN             32

typedef int (*ams_file_thd_func)(char *);
 
typedef struct ams_script_cfg {
    char *ext_name;
    ams_file_thd_func thd_entry;
} ams_script_cfg_t;


#define AMS_FILE_ROOT_PATH          "/"


typedef struct ams_jz_file_info
{
    uint8_t file_num;
    uint8_t name_len;
    uint32_t content_len;
    char content_md5[33];
    char file_name[256];
} ams_jz_file_info_t;


/*                     file operations                       */
int ams_check_file_exist(const char * file_path);
int ams_is_single_file(const char *main_class);
int ams_save_app_file(int sock, int app_length, char *file_name);
int ams_unpack_app(const char *jz_file);
char *ams_scan_files(void);
int ams_delete_directory(const char * dir_name);
void ams_run_file_thd_entry(void *argument);
char *ams_get_file_ext_name(void);
extern int mpy_file_entry(char *script_file);
extern int js_file_entry(char *script_file);
int ams_get_auto_app_path(char *script_name);
int ams_set_auto_app_path(char *app_name, char *main_class);
void ams_del_auto_app_path(const char *app_name);
int ams_get_vm_status(void);
int ams_mount_file_system(char *file_dev_name);




/*                     file operations                       */



#endif /* __AMS_FILE_H__ */
