#ifndef __AMS_CORE_H__
#define __AMS_CORE_H__
#include <stdint.h>
#include <os_task.h>
#include <os_util.h>
#include "oneos_config.h"

#ifndef NULL    /* ANSI C #defines NULL everywhere. */
#define NULL    0
#endif


typedef enum {
    AMS_ERROR = -1,
    AMS_OK,
} AMS_ERROR_CODE;

typedef enum AMS_THREAD_TYPE {
    AMS_THD_MAIN = 0,
    AMS_THD_RUNNER,
    AMS_THD_MAX,
} AMS_THREAD_TYPE_T;

#ifdef  AMS_APP_TYPE_CHOICE_JS
#define AMS_APP_TYPE_CHOICE     AMS_APP_JAVA_SCRIPT,
#endif

#ifdef AMS_APP_TYPE_CHOICE_PY
#define AMS_APP_TYPE_CHOICE     AMS_APP_MICROPYTHON
#endif

#ifndef AMS_APP_TYPE_CHOICE
#define AMS_APP_TYPE_CHOICE     AMS_APP_MICROPYTHON
#endif

#ifdef AMS_USING_DEBUG_MODE
#define AMS_APP_DEBUG_FLAG      0
#else
#define AMS_APP_DEBUG_FLAG      1
#endif
/*                     ams log                */
typedef enum {
    AMS_DBG_LEVEL_DBG = 1,
    AMS_DBG_LEVEL_ERR,
} AMS_DBG_LEVEL;

#define ams_model_printf(debug_lvl, msg, ...)                                               \
    do {                                                                                    \
        if (debug_lvl == AMS_DBG_LEVEL_ERR || debug_lvl > AMS_APP_DEBUG_FLAG) {            \
            os_kprintf(msg"\r\n", ##__VA_ARGS__);                             \
        }                                                                                   \
    }while(0);

#define ams_log(msg, ...)       ams_model_printf(AMS_DBG_LEVEL_DBG, "[AMS][LOG][%s|%d]"msg, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ams_err(msg, ...)       ams_model_printf(AMS_DBG_LEVEL_ERR, "[AMS][ERROR][%s|%d]"msg, __FUNCTION__, __LINE__, ##__VA_ARGS__)
/*                     ams log                */

/*                     ams encap os functions                */
#define ams_calloc   os_calloc

#define ams_free     os_free

typedef enum AMS_APP_TYPE {
    AMS_APP_MICROPYTHON = 0,
    AMS_APP_JAVA_SCRIPT,
    AMS_APP_TYPE_MAX,
} AMS_APP_TYPE_T;

typedef struct ams_thd_ctrl_entry {
    os_task_t *thread;
    char *name;
    uint32_t stack_size;
    uint8_t priority;
} ams_thd_ctrl_entry_t;

int ams_startup_thread(AMS_THREAD_TYPE_T thd_type, void (*entry)(void *arg), void *arg);

int asm_stop_thread(AMS_THREAD_TYPE_T thd_type);

/*                     ams encap os functions                */

/*                     get device info                       */
#define AMS_IMEI_NAME_LEN           15
#define AMS_IMSI_NAME_LEN           15
#define AMS_ICCID_NAME_LEN          20
#define AMS_APP_NAME_LEN            33
#define AMS_MAIN_CLASS_LEN          33
#define AMS_APP_URL_LEN             256

typedef int   (*ams_get_devinfo_func)(void *, char *, int);

int ams_get_imei(char *imei, int len);
int ams_get_imsi(char *imsi, int len);
int ams_get_iccid(char *iccid, int len);
void ams_reset_board(void);


/*                     get device info                       */


/*             external functions               */
typedef uint8_t (*ams_env_unlock_func)(void);
typedef uint8_t (*ams_app_running_states)(void);
typedef int (*ams_env_deinit_func)(void);

typedef struct ams_env_set {
    ams_env_unlock_func app_unlock;
    ams_app_running_states app_state;
    ams_env_deinit_func deinit;
} ams_env_set_t;

extern uint8_t micropy_file_exit(void);
extern int mpy_deinit(void);
extern uint8_t micropy_file_state(void);

int ams_stop_thread(AMS_THREAD_TYPE_T thd_type);

/*             external functions end               */


#endif /* __AMS_CORE_H__ */

