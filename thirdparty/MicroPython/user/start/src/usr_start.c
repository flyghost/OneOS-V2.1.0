#include "usr_misc.h"
#include "shell.h"
#include "string.h"
#include "mpconfigport.h"

#if MICROPY_PY_THREAD
#include "mpthreadport.h"
#endif

#ifdef MICROPY_USING_FILESYSTEM
#include <vfs_fs.h>
#include "vfs_fs.h"
#include "vfs_posix.h"
#include "usr_general.h"
#endif
#ifdef MICROPY_USING_AMS
#include "ams.h"
#ifndef  MICROPY_USING_FILESYSTEM
#error "Application management system(AMS) need filesystem, open filesystem please!"
#endif
#else
#include <os_util.h>
#endif


//#if SHELL_TASK_STACK_SIZE < 2336
//#error "SHELL_TASK_STACK_SIZE need more than 2336 bytes if use microPython"
//#endif


os_err_t usr_task_delete(os_task_t *task)
{
	os_err_t ret = os_task_destroy(task);
	micropy_file_exit();
	return ret;
}

#ifdef MICROPY_USING_FILESYSTEM
#define MICROPY_OPEN_BLOCK_DEVICE_TIMES     5
#ifdef BOARD_ATK_APOLLO
#include <fal/fal.h>
static int mp_fal_mount(const char *part_name, const char *mount_point, const char *fs_type)
{
    int32_t ret = MP_ERROR;
    
    if (fal_blk_device_create(part_name))
    {
        mp_log("Create a block device on the %s partition of flash successful.\r\n", part_name);
    }
    else
    {
        mp_log("Can't create a block device on '%s' partition.\r\n", part_name);
        return ret;
    }
    
    ret = vfs_mount(part_name, mount_point, fs_type, 0, 0);
    if (ret == 0)
    {
        mp_log("filesystem mount successful.\r\n");
    }
    else
    {
        mp_log("filesystem mount fail.\r\n");
        
        ret = vfs_mkfs(fs_type ,part_name);
        if(ret != 0)
        {
            mp_err("Failed to make file system!");
        }

        ret = vfs_mount(part_name, mount_point, fs_type, 0, 0);
        if(ret != 0)
        {
            mp_err("Failed to mount file system!");
        }
    }
    
    return ret;
}
#endif

static int fs_dev_link(int argc, char **argv)
{
    char *file_sys_device =  MICROPY_FS_DEVICE_NAME;

    if (argc == 2)
    {
        file_sys_device = argv[1];
        mp_log("file_sys_device:%s", file_sys_device);
    }
#ifdef BOARD_ATK_APOLLO
    mp_fal_mount(file_sys_device, "/", "fat");
#else
    int ret = 0;
    int i = 0;
    /* --todo--规避： 探索者407有时会出现挂文件系统时，sd0还未注册的情况         */
    for (i = 0; i < MICROPY_OPEN_BLOCK_DEVICE_TIMES; i++)
    {
        ret = mpy_usr_driver_open(file_sys_device);
        if (ret == 0)
        {
            mp_log("Try[%d] open device[%s] success.", i, file_sys_device);
            break;
        }
        os_task_tsleep(100);
    }
    
    if (ret != 0)
    {
        mp_err("Failed to open device[%s].", file_sys_device);
        return OS_ERROR;
    }
    
    /* Mount the file system from tf card(sd0) or internal flash(W25Q64) */
    ret = vfs_mount(file_sys_device, "/", "fat", 0, 0);
    if (ret != 0)
    {
        mp_log("For the first time mount file system on device[%s] failed, try again.", 
                    file_sys_device);
        os_task_tsleep(1);
        ret = vfs_mkfs("fat" ,file_sys_device);
        if(ret != 0)
        {
            mp_err("Failed to make file system!\n");
            return OS_ERROR;
        }
        
        ret = vfs_mount(file_sys_device, "/", "fat", 0, 0);
        if(ret != 0)
        {
            mp_err("Failed to mount file system!\n");
            return OS_ERROR;
        }
    }
 #endif
    mp_log("File system initialized!");

    os_task_tsleep(500);

    return OS_EOK;
}
#endif

static int register_mpy_log_api(void)
{
#ifdef MICROPY_USING_AMS
	struct ams_misc_fun *misc_fun = ams_port_get_misc_structure();
	misc_fun->sm_fun->print = os_kprintf;
	#ifdef MICROPY_USING_DEBUG_MODE
	misc_fun->sm_fun->model = MODEL_LOG_BASE;
	#else
	misc_fun->sm_fun->model = MODEL_LOG_DEBUG;
	#endif
#else
	struct model_misc_fun * misc_api =  model_get_misc_structure();
	misc_api->print = os_kprintf;
	#ifdef MICROPY_USING_DEBUG_MODE
	misc_api->model = MODEL_LOG_BASE;
	#else
	misc_api->model = MODEL_LOG_DEBUG;
	#endif
#endif
	return 0;
}
OS_PREV_INIT(register_mpy_log_api, OS_INIT_SUBLEVEL_LOW);

static int micropy_start(void)
{
    int ret = MP_EOK;
#ifdef MICROPY_USING_AMS
    init_ams();
    ret = start_ams_component();
    mp_log("ams_device:%s, ret: %d",  mp_misc_get_dev_name(AMS_DEVICE_ID), ret);
#endif

#ifdef MICROPY_USING_FILESYSTEM
    ret = fs_dev_link(0, NULL);
#endif

    return ret;
}
OS_APP_INIT(micropy_start, OS_INIT_SUBLEVEL_LOW);

os_task_t *g_micropy_task = NULL;
char * g_script_name[40] = {0};

static int save_file(char *file)
{
	if(!file){
		return MP_ERROR;
	}
	memset(g_script_name, 0, strlen((const char *)g_script_name));
	memcpy(g_script_name, file, strlen(file));
	return MP_EOK;
}


int run_mpy(int argc, char **argv)
{
    char *file = NULL;

    if (argc == 1)
    {
#ifdef MICROPYTHON_USING_REPL
        mpy_repl_entry();
#else
        mp_err("The microPython repl mode is closed.");
#endif
        return MP_EOK;
    }
    
    file = argv[1];
    if (strncmp(file, "stop", 4) == 0)
    {
        if (g_micropy_task)
        {
            usr_task_delete(g_micropy_task);
        } 
        else
        {
            mp_log("No app running!");
        }

        return MP_EOK;
    }

    save_file(file);
    g_micropy_task = os_task_create("mpy-task", 
                                    (fun_0_1_t)mpy_file_entry, 
                                    g_script_name,  
                                    8192, 
                                    19); /* tshell的优先级是20, 这里调成19，可以解决数据读、写 */
    if (!g_micropy_task)
    {
        mp_err("Failed to create micropython task !");
        return MP_ERROR;
    }
    os_task_startup(g_micropy_task);

    return MP_EOK;
}

SH_CMD_EXPORT(mpy, run_mpy, "Run/stop python file or enter MicroPython repl");

OS_PREV_INIT(mpycall_device_list_init, OS_INIT_SUBLEVEL_MIDDLE);







