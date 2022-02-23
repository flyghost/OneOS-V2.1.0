#include <string.h>
#include "core/ams_core.h"
#include "ops/ams_protocol.h"
#include "ops/ams_file.h"
#include "ams_local.h"

/* Get the local APP for display */
char *ams_get_local_file(void)
{
    /* TODO -- the local file is */
    return ams_scan_files();
}

void ams_start_local_app(char *app_name, char *main_file)
{
    ams_start_app_param_t cmd_param = {0};

    if ((strlen(app_name) > (AMS_APP_NAME_LEN - 1)) || 
        (strlen(main_file) > (AMS_MAIN_CLASS_LEN - 1)))
    {
        ams_err("Input is invalid.");
        return;
    }
    strncpy(cmd_param.app_name, app_name, strlen(app_name));
    if (main_file == NULL)
    {
        strcpy(cmd_param.main_class, "null");
    }
    else
    {
        strncpy(cmd_param.main_class, main_file, strlen(main_file));
    }
    
    ams_start_app(&cmd_param, NULL);
    return;
}

void ams_del_local_app(char *app_name)
{
    ams_delete_app_param_t cmd_param = {0};
    
    strncpy(cmd_param.app_name, app_name, AMS_APP_NAME_LEN);
    ams_delete_app(&cmd_param, NULL);
    return;
}

void ams_show_applist(void)
{
    os_kprintf("%s\r\n", ams_scan_files());
}

