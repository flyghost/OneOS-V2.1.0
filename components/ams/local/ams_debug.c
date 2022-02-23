#include <shell.h>
#include "core/ams_core.h"
#include "core/ams_task.h"
#include "ops/ams_file.h"
#include "ops/ams_net.h"
#include "ops/ams_protocol.h"
#include "ams_local.h"
#include "ams_debug.h"

static void ams_dbg_run_app(char **argv);
static ams_debug_info_t g_ams_dbg[ams_dbg_type_max] = 
{
    {"-help", "Show help information of show_ams command.", "dbg_ams -help", NULL},
    {"-task", "Show internal task information of ams.", "dbg_ams -task", NULL},
    {"-net", "Show network information of ams.", "dbg_ams -net", NULL},
    {"-list", "Show APP file list in local storage.", "dbg_ams -list", NULL},
    {"-run", "Run APP file.", "dbg_ams -run appname appmainclass", NULL},
    {"-del", "Delete APP file.", "dbg_ams -del appname", NULL},
};

static void ams_show_helpinfo(void)
{
    int i = 0;

    os_kprintf("Command support: \r\n");
    for (i = 0; i < ams_dbg_type_max; i++)
    {
        os_kprintf("    %-*s", 5, g_ams_dbg[i].key_str);
        os_kprintf("[%-*s]: ", 15, g_ams_dbg[i].example);
        os_kprintf("    %-*s\r\n", 5, g_ams_dbg[i].explain);
    }

    return;
}

static void ams_dbg_run_app(char **argv)
{
    ams_start_local_app(argv[2], argv[3]);
    return;
}

static void ams_dbg_del_app(char **argv)
{
    ams_del_local_app(argv[2]);
    return;
}

static void ams_dbg_register_dbg_func(void)
{
    g_ams_dbg[ams_dbg_type_help].func.show_func = ams_show_helpinfo;
    g_ams_dbg[ams_dbg_type_task].func.show_func = ams_show_taskinfo;
    g_ams_dbg[ams_dbg_type_net].func.show_func = ams_show_netinfo;
    g_ams_dbg[ams_dbg_type_list].func.show_func = ams_show_applist;
    g_ams_dbg[ams_dbg_type_run].func.operate_func = ams_dbg_run_app;
    g_ams_dbg[ams_dbg_type_del].func.operate_func = ams_dbg_del_app;
    return;
}

int ams_dbg(int argc, char **argv)
{
    int i = 0;
    
    if (argc < 2)
    {
        ams_show_helpinfo();
        return AMS_OK;
    }
    ams_dbg_register_dbg_func();
    for (i = 0; i < ams_dbg_type_max; i++)
    {
        if (argc == 2 && g_ams_dbg[i].func.show_func != NULL)
        {
            if (strcmp(argv[1], g_ams_dbg[i].key_str) == 0)
            {
                g_ams_dbg[i].func.show_func();
                return AMS_OK;
            }
        }
        else if (g_ams_dbg[i].func.operate_func != NULL)
        {
            if (strcmp(argv[1], g_ams_dbg[i].key_str) == 0)
            {
                g_ams_dbg[i].func.operate_func(argv);
                return AMS_OK;
            }
        }
    }

    return AMS_ERROR;
}
SH_CMD_EXPORT(dbg_ams, ams_dbg, "Debug cmd of AMS components.");

