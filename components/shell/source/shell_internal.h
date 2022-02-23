/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        shell_internal.h
 *
 * @brief       Internal header file for shell.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __SHELL_INTERNAL_H__
#define __SHELL_INTERNAL_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <device.h>
#include <os_sem.h>
#include <os_mutex.h>

#define SHELL_OPTION_ECHO               0x01

enum sh_input_stat
{
    SHELL_WAIT_NORMAL,
    SHELL_WAIT_SPEC_KEY,
    SHELL_WAIT_FUNC_KEY
};

typedef os_err_t (*cmd_function_t)(os_int32_t argc, char **argv);


/**
 ***********************************************************************************************************************
 * @struct      shell_ctrl_info
 *
 * @brief       Shell control block information.
 ***********************************************************************************************************************
 */
struct shell_ctrl_info
{
    os_sem_t            rx_sem;                             /* Semaphore use for receiving character */

    enum sh_input_stat  stat;                               /* Receive character state */

    os_bool_t           enable_echo;                        /* Whether enable echo */
    os_bool_t           enable_prompt;                      /* Whether enable prompt */

#ifdef SHELL_USING_HISTORY
    os_uint16_t         current_history;                    /* Current history index */
    os_uint16_t         history_count;                      /* The count for histories */
    char                cmd_history[SHELL_HISTORY_LINES][SHELL_CMD_SIZE + 1];   /* History buffer */
#endif

    char                line[SHELL_CMD_SIZE + 1];           /* Command line buffer */
    os_uint8_t          line_position;                      /* Current input character position */
    os_uint8_t          line_curpos;                        /* Current cursor position. */

    os_device_t              *device;                       /* Console device. */
    struct os_device_cb_info  cb_info;
    os_mutex_t                dev_mutex;

#ifdef SHELL_USING_AUTH
    char                password[SHELL_PASSWORD_MAX + 1];   /* Password string */
#endif
};
typedef struct shell_ctrl_info shell_ctrl_info_t; 

extern void        sh_init_cmd_table(const void *begin, const void *end);
extern void        sh_get_cmd_table(sh_cmd_entry_t **table_begin, sh_cmd_entry_t **table_end);
extern void        sh_auto_complete(char *prefix, os_size_t size);
extern const char *sh_get_prompt(void);
extern os_err_t    sh_do_set_prompt(const char *prompt);
extern os_err_t    sh_do_exec(char *cmd, os_size_t length);


#endif /* __SHELL_INTERNAL_H__ */

