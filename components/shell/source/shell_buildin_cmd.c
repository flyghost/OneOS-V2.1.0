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
 * @file        shell_buildin_cmd.c
 *
 * @brief       Buildin command of shell.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#include <os_util.h>
#include <os_errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef OS_USING_SHELL
#include <shell.h>
#include "shell_internal.h"

#ifdef OS_USING_SYS_HEAP

static os_err_t sh_set_prompt(os_int32_t argc, char **argv)
{
    os_err_t ret;

    ret = OS_EOK;

    if (argc != 2)
    {
        os_kprintf("Wrong parameter, usage:\r\n");
        os_kprintf("set_prompt <prompt string>\r\n");
        
        ret = OS_EINVAL;
    }
    else
    {
        ret = sh_do_set_prompt(argv[1]);
    }

    return ret;
}
SH_CMD_EXPORT(set_prompt, sh_set_prompt, "Set shell prompt");

#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           Provide help information.
 *
 * @param[in]       argc            Command arguments count.
 * @param[in]       argv            Command arguments
 *
 * @return          The state of executting command.
 * @retval          OS_EOK          Execute command success.
 * @retval          else            Execute command failed.
 ***********************************************************************************************************************
 */
static os_err_t sh_help(os_int32_t argc, char **argv)
{
    sh_cmd_entry_t *table_entry;
    sh_cmd_entry_t *table_begin;
    sh_cmd_entry_t *table_end;

    OS_UNREFERENCE(argc);
    OS_UNREFERENCE(argv);

    os_kprintf("OneOS shell commands:\r\n");

    sh_get_cmd_table(&table_begin, &table_end);

    for (table_entry = table_begin; table_entry < table_end; SH_NEXT_CMD_ENTRY(table_entry))
    {
        if (strncmp(table_entry->name, "__cmd_", 6))
        {
            continue;
        }

#if defined(SHELL_USING_DESCRIPTION)
        os_kprintf("%-16s - %s\r\n", &table_entry->name[6], table_entry->desc);
#else
        os_kprintf("%s\r\n", &table_entry->name[6]);
#endif
    }

    return OS_EOK;
}
SH_CMD_EXPORT(help, sh_help, "Obtain help of commands");

#endif /* OS_USING_SHELL */

