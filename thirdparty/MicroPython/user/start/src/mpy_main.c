#include <board.h>
#include <shell.h>
#include <os_memory.h>
#include "py/compile.h"
#include "py/repl.h"
#include "py/gc.h"
#include "py/stackctrl.h"
#include "lib/mp-readline/readline.h"
#include "lib/utils/pyexec.h"
#include "middlelib.h"

#include "string.h"
#include "usr_stdio.h"
#include "usr_misc.h"


#define MICROPY_FILE_ENTRY()                            \
    do {                                                \
        if(g_mpy_file_counter)                          \
        {                                               \
            mp_err("micropython file is running!");   \
            return MP_ERROR;                            \
        }                                               \
        g_mpy_file_counter++;                           \
        mp_log("mpy counter: %d", g_mpy_file_counter);\
    } while(0)

#define MICROPY_FILE_EXIT()                             \
    do {                                                \
        g_mpy_file_counter--;                           \
        if (g_mpy_file_counter < 0)                     \
        {                                               \
            g_mpy_file_counter = 0;                     \
        }                                               \
        mp_log("mpy counter: %d", g_mpy_file_counter);\
    } while (0)


#ifdef MICROPY_VFS_FAT
#include <vfs_posix.h>
#endif



static void *g_stack_top = NULL;
static char *g_heap = NULL;
static	int8_t g_mpy_file_counter = 0;
static uint8_t g_repl_entry_status = 0;
static void *mpy_main_thd_id = NULL;

void * mpy_get_main_thd_id(void)
{
    return mpy_main_thd_id;
}

static uint8_t get_repl_entry_status(void)
{
    return g_repl_entry_status;
}

static void set_repl_entry_status(uint8_t value)
{
    g_repl_entry_status = value;
    return;
}

mp_import_stat_t mp_import_stat(const char *path) 
{
#ifdef MICROPY_VFS_FAT
    struct stat stat;
    if (vfs_stat(path, &stat) == 0) 
    {
        return (S_ISDIR(stat.st_mode) ? MP_IMPORT_STAT_DIR : MP_IMPORT_STAT_FILE);
    } 
    else 
    {
        return MP_IMPORT_STAT_NO_EXIST;
    }
#else
    return MP_IMPORT_STAT_NO_EXIST;
#endif
}


void nlr_jump_fail(void *val) {
    while (1);
}

void NORETURN __fatal_error(const char *msg) {
    while (1);
}




static int add_path_to_sys(char * name)
{
    int site = reverse_find_char(name, '/');
    
    if (site >= 35)
    {
        mp_err("the length of folder name is to long!");
        return MP_ERROR;
    }
    
    if (site == -1)
    {
        return MP_ERROR;
    }
    
    char tmp[35]={0};
    strncpy(tmp , name , site);
    (void) mp_obj_list_append(mp_sys_path, mp_obj_new_str(tmp, strlen(tmp)));
    return MP_EOK;
}

static int mpy_init(void)
{
	int stack_dummy;
	g_stack_top = (void *) &stack_dummy;
	mpy_main_thd_id = (void *)os_task_self();
#if MICROPY_PY_THREAD
	void *stack_addr = mp_thread_get_stack_addr();
	mp_thread_init(stack_addr, ((u32_t)g_stack_top - (u32_t)stack_addr) / 4);
	
#endif
	
	mp_stack_set_top(g_stack_top);
	mp_stack_set_limit(mp_thread_get_stack_size() - 1024);

    mp_log("Start alloc memory size: %d", MICROPY_HEAP_SIZE);
	//g_heap = mp_heap_malloc((size_t)MICROPY_HEAP_ADDR, MP_HEAP_RAM_ADDR);
	g_heap = os_malloc(MICROPY_HEAP_SIZE);
	if(!g_heap){
		 mp_err("g_heap malloc failed.");
		 return MP_ERROR;
	}
	gc_init(g_heap, g_heap + MICROPY_HEAP_SIZE);

	mp_init();
	mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_path), 0);
	(void) mp_obj_list_append(mp_sys_path, MP_OBJ_NEW_QSTR(MP_QSTR_));
	mp_obj_list_init(MP_OBJ_TO_PTR(mp_sys_argv), 0);
	
	readline_init0();
	return MP_EOK;
}

int mpy_deinit(void)
{
	gc_sweep_all();
	mp_deinit();
#if MICROPY_PY_THREAD
	mp_thread_deinit();
#endif

	//mp_heap_free(g_heap, MP_HEAP_RAM_ADDR);
	free(g_heap);
	return MP_EOK;
}

static int repl_running(void)
{
    set_repl_entry_status(1);
    /* hold the console terminal */
    sh_disconnect_console();

    usr_getchar_init();
    
    for (;;)
    {
        if (pyexec_mode_kind == PYEXEC_MODE_RAW_REPL) 
        {
            if (pyexec_raw_repl() != 0) 
            {
                break;
            }
        } 
        else if (pyexec_friendly_repl() != 0) 
        {
            break;
        }
    }

    usr_getchar_deinit();

    /* release the console terminal */
    sh_reconnect_console();
    set_repl_entry_status(0);
    return MP_EOK;
}

static int mpy_file_running(void* argument)
{
    if (!argument)
    {
        return MP_ERROR;
    }
#ifndef MICROPYTHON_USING_UOS
    mp_error("Please enable uos module in sys module option first.\n");
#else
    (void) add_path_to_sys(argument);
    (void) pyexec_file(argument);
#endif
    return MP_EOK;
}

int mpy_repl_entry(void)
{
    if (get_repl_entry_status())
    {
        mp_log("The microPython repl has already been open.");
        mp_raise_msg(&mp_type_RuntimeError, " Iterative repl unsupported");
        return MP_EOK;
    }
    (void)mpy_init();
    (void)repl_running();
    (void)mpy_deinit();
    return MP_EOK;
}

uint8_t micropy_file_exit(void)
{
	MICROPY_FILE_EXIT();
	return g_mpy_file_counter;
}

uint8_t micropy_file_state(void)
{
    return g_mpy_file_counter;
}

int mpy_file_entry(void* argument)
{
    char * file = argument;
    
    if (!file)
    {
        return MP_ERROR;
    }
    // check file
    int length = strlen(argument);
    if ((strncmp(file+length-3,".py", 3) !=0) && (strncmp(file+length-4,".mpy", 4) !=0))
    {
        mp_err("file is wrong!");
        return MP_ERROR;
    }

    MICROPY_FILE_ENTRY();
    (void) mpy_init();
    (void) mpy_file_running(argument);
    (void) mpy_deinit();
    MICROPY_FILE_EXIT();
    return MP_EOK;
}


