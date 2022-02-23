#ifndef __AMS_SHOW_H__
#define __AMS_SHOW_H__

typedef enum ams_dbg_type {
    ams_dbg_type_help = 0,
    ams_dbg_type_task,
    ams_dbg_type_net,
    ams_dbg_type_list,
    ams_dbg_type_run,
    ams_dbg_type_del,
    ams_dbg_type_max,
} ams_dbg_type_e;

typedef void (*ams_dbg_show_func)(void);
typedef void (*ams_dbg_operate_func)(char **);

typedef struct ams_debug_info {
    char                    *key_str;
    char                    *explain;
    char                    *example;
    union {
        ams_dbg_show_func       show_func;
        ams_dbg_operate_func    operate_func;
    } func;
} ams_debug_info_t;

void ams_start_local_app(char *app_name, char *main_file);
void ams_del_local_app(char *app_name);

#endif /* __AMS_SHOW_H__ */

