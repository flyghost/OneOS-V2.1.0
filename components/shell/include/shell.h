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
 * @file        shell.h
 *
 * @brief       Header file for SHELL.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#ifndef __SHELL_H__
#define __SHELL_H__

#include <oneos_config.h>
#include <os_types.h>
#include <os_stddef.h>
#include <os_util.h>

#ifdef OS_USING_SHELL

typedef os_err_t (*sh_cmd_func_t)(os_int32_t argc, char **argv);

struct sh_cmd_entry
{
    const char*     name;       /* The name of system call */

#if defined(SHELL_USING_DESCRIPTION)
    const char*     desc;       /* Description of system call */
#endif

    sh_cmd_func_t   func;       /* The function address of system call */
};
typedef struct sh_cmd_entry sh_cmd_entry_t;

#if (defined(__GNUC__) && defined(ARCH_X86_64))
    extern sh_cmd_entry_t *shell_cmd_entry_next(sh_cmd_entry_t *entry);
    #define SH_NEXT_CMD_ENTRY(index)  index=shell_cmd_entry_next(index)
#else
    #define SH_NEXT_CMD_ENTRY(index)  index++
#endif

#ifdef SHELL_USING_DESCRIPTION
#define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
    const char __fsym_##cmd##_name[] = #cmd;                            \
    const char __fsym_##cmd##_desc[] = desc;                            \
    OS_USED const sh_cmd_entry_t __fsym_##cmd OS_SECTION("FSymTab") =   \
    {                                                                   \
        __fsym_##cmd##_name,                                            \
        __fsym_##cmd##_desc,                                            \
        (sh_cmd_func_t)func                                             \
    };
#else
#define SH_FUNCTION_EXPORT_CMD(func, cmd, desc)                         \
    const char __fsym_##cmd##_name[] = #cmd;                            \
    OS_USED const sh_cmd_entry_t __fsym_##cmd OS_SECTION("FSymTab")=    \
    {                                                                   \
        __fsym_##cmd##_name,                                            \
        (sh_cmd_func_t)func                                             \
    };
#endif /* SHELL_USING_DESCRIPTION */

/**
 ***********************************************************************************************************************
 * @def         SH_CMD_EXPORT
 *
 * @brief       This macro exports a system function with an command name to shell.
 *
 * @param       cmd             The name of command.
 * @param       func            The name of function.
 * @param       desc            The description of command, which will show in help.
 ***********************************************************************************************************************
 */
#define SH_CMD_EXPORT(cmd, func, desc)      SH_FUNCTION_EXPORT_CMD(func, __cmd_##cmd, desc)

extern void     sh_disconnect_console(void);
extern void     sh_reconnect_console(void);
extern os_err_t sh_exec(const char *cmd);

#else
#define SH_CMD_EXPORT(cmd, func, desc)
#endif

#endif /* __SHELL_H__ */

