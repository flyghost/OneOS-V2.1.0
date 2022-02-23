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
 * @file        shell_process.c
 *
 * @brief       Process shell command.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <oneos_config.h>
#include <shell.h>
#include <os_util.h>
#include <os_memory.h>
#include <os_assert.h>
#include <os_errno.h>
#include <string.h>

#ifdef OS_USING_VFS
#include <unistd.h>
#include <dirent.h>
#endif

#include "shell_internal.h"

#ifdef OS_USING_SHELL

static os_int32_t sh_split(char *cmd, os_size_t length, char *argv[SHELL_ARG_MAX])
{
    char      *ptr;
    os_size_t  position;
    os_size_t  argc;
    os_size_t  i;

    ptr      = cmd;
    position = 0;
    argc     = 0;

    while (position < length)
    {
        /* Strip bank and tab */
        while (((*ptr == ' ') || (*ptr == '\t')) && position < length)
        {
            *ptr = '\0';
            ptr++;
            position++;
        }

        if (position >= length)
        {
            break;
        }

        if (argc >= SHELL_ARG_MAX)
        {
            os_kprintf("Too many args ! We only Use:\r\n");
            for(i = 0; i < argc; i++)
            {
                os_kprintf("%s ", argv[i]);
            }
            os_kprintf("\r\n");
            
            break;
        }
        
        /* Handle string */
        if (*ptr == '"')
        {
            ptr++;
            position++;
            argv[argc] = ptr;
            argc++;

            /* Skip this string */
            while (('"' != *ptr) && (position < length))
            {
                if ('\\' == *ptr)
                {
                    if ('"' == *(ptr + 1))
                    {
                        ptr++;
                        position++;
                    }
                }
                
                ptr++;
                position++;
            }
            
            if (position >= length)
            {
                break;
            }
            
            /* Skip '"' */
            *ptr = '\0';
            ptr++;
            position++;
        }
        else
        {
            argv[argc] = ptr;
            argc++;

            while ((*ptr != ' ') && (*ptr != '\t') && (position < length))
            {
                ptr++;
                position++;
            }
            
            if (position >= length)
            {
                break;
            }
        }
    }

    return argc;
}

static cmd_function_t sh_get_cmd_func(char *cmd, os_int32_t size)
{
    sh_cmd_entry_t *syscall_tmp;
    sh_cmd_entry_t *table_begin;
    sh_cmd_entry_t *table_end;
    cmd_function_t  cmd_func;
    os_uint16_t     cmd_prefix_len;

    cmd_func = OS_NULL;

    sh_get_cmd_table(&table_begin, &table_end);
    cmd_prefix_len = strlen("__cmd_");

    for (syscall_tmp = table_begin; syscall_tmp < table_end; SH_NEXT_CMD_ENTRY(syscall_tmp))
    {
        if (strncmp(syscall_tmp->name, "__cmd_", cmd_prefix_len))
        {
            continue;
        }

        if (!strncmp(&syscall_tmp->name[cmd_prefix_len], cmd, size)
            && (syscall_tmp->name[cmd_prefix_len + size] == '\0'))
        {
            cmd_func = (cmd_function_t)syscall_tmp->func;
            break;
        }
    }

    return cmd_func;
}

static os_err_t sh_do_exec_cmd(char *cmd, os_size_t length, os_err_t *retp)
{
    os_int32_t      argc;
    os_size_t       cmd0_size;
    cmd_function_t  cmd_func;
    char           *argv[SHELL_ARG_MAX];
    os_err_t        ret;

    OS_ASSERT(cmd);
    OS_ASSERT(retp);

    ret       = OS_EOK;
    cmd0_size = 0U;

    /* Find the size of first command */
    while ((cmd[cmd0_size] != ' ') && (cmd[cmd0_size] != '\t') && (cmd0_size < length))
    {
        cmd0_size++;
    }
    
    if (0U == cmd0_size)
    {
        os_kprintf("cmd 0 size \r\n");
        ret = OS_ERROR;
    }
    else
    {
        (void)memset(argv, 0, sizeof(argv));
        cmd_func = sh_get_cmd_func(cmd, cmd0_size);
        argc     = sh_split(cmd, length, argv);

        if ((OS_NULL != cmd_func) && (0 != argc))
        {
            *retp = cmd_func(argc, argv); 
        }
        else
        {
            ret = OS_ERROR;
        }
    }
    
    return ret;
}

/**
 ***********************************************************************************************************************
 * @brief           Execute command.
 *
 * @param[in]       cmd             Command string, including command name and command arguments.         
 * @param[in]       length          Command string length.
 *
 * @return          Execute command result.
 * @retval          OS_EOK          Execute command success.
 * @retval          else            Execute command failed.
 ***********************************************************************************************************************
 */
os_err_t sh_do_exec(char *cmd, os_size_t length)
{
    char     *tcmd;
    os_err_t  cmd_ret;
    os_err_t  ret;

    ret = OS_EOK;

    /* Strim the beginning of command */
    while (((*cmd  == ' ') || (*cmd == '\t')) && (length > 0))
    {
        cmd++;
        length--;
    }

    if (length > 0)
    {
        ret = sh_do_exec_cmd(cmd, length, &cmd_ret);
        if (OS_EOK == ret)
        {
            ret = cmd_ret;
        }
        else
        {   
            /* Truncate the cmd at the first space. */
            tcmd = cmd;
            while ((*tcmd != ' ') && (*tcmd != '\0'))
            {
                tcmd++;
            }
            *tcmd = '\0';
            
            os_kprintf("%s: command not found.\r\n", cmd);
        }
    }
    
    return ret;
}

os_err_t sh_exec(const char *cmd)
{
    char     *cmd_str;
    os_size_t cmd_length;
    os_err_t  ret;
    
    OS_ASSERT(cmd != OS_NULL);

    ret        = OS_EOK;
    cmd_length = strlen(cmd);

    /* 1 is used to reserve space for '\0' */
    cmd_str = (char *)os_malloc(cmd_length + 1);
    if (OS_NULL == cmd_str)
    {
        ret = OS_ENOMEM;
    }
    else
    {
        (void)strcpy(cmd_str, cmd);

        /* Here, the length of cmd is not contain '\0' */
        ret = sh_do_exec(cmd_str, cmd_length);

        os_free(cmd_str);
        cmd_str = OS_NULL;
    }
    
    return ret;
}

static os_int32_t sh_string_same_part_length(const char *cmd_str1, const char *cmd_str2)
{
    const char *cmd_str_tmp;

    cmd_str_tmp = cmd_str1;
    while ((*cmd_str_tmp != 0) && (*cmd_str2 != 0) && (*cmd_str_tmp == *cmd_str2))
    {
        cmd_str_tmp++;
        cmd_str2++;
    }

    return (cmd_str_tmp - cmd_str1);
}

#ifdef OS_USING_VFS
static void sh_do_auto_complete_path(char *path, os_size_t size)
{
#define FULL_PATH_LEN_MAX       256

    DIR           *dir = OS_NULL;
    struct dirent *dirent = OS_NULL;
    char          *full_path;
    char          *ptr;
    char          *index;
    char          *end_addr = path + size;

    OS_ASSERT(OS_NULL != path);

    full_path = (char *)os_malloc(FULL_PATH_LEN_MAX);
    if (OS_NULL == full_path)
    {
        /* Out of memory */
        os_kprintf("Malloc memory of full_path failed\r\n");
        return;
    }

    memset(full_path, 0, FULL_PATH_LEN_MAX);

    if (*path != '/')
    {
        getcwd(full_path, FULL_PATH_LEN_MAX);
        
        if (full_path[strlen(full_path) - 1] != '/')
        {
            strcat(full_path, "/");
        }
    }
    else
    {
        *full_path = '\0';
    }
    
    index = OS_NULL;
    ptr   = path;

    for (;;)
    {
        if (*ptr == '/')
        {
            index = ptr + 1;
        }
        
        if (!(*ptr))
        {
            break;
        }
        
        ptr++;
    }

    if (OS_NULL == index)
    {
        index = path;
    }

    if (OS_NULL != index)
    {
        char *dest;

        dest = index;

        /* Fill the parent path */
        ptr = full_path;
        while (*ptr)
        {
            ptr++;
        }

        for (index = path; index != dest;)
        {
            *ptr++ = *index++;
        }
        *ptr = '\0';

        dir = opendir(full_path);
        if (OS_NULL == dir)
        {
            /* Open directory failed! */
            os_free(full_path);
            return;
        }

        /* Restore the index position */
        index = dest;
    }

    /* Auto complete the file or directory name */
    if ('\0' == *index)
    {
        /* Display all of files and directories */
        for (;;)
        {
            dirent = readdir(dir);
            if (OS_NULL == dirent)
            {
                break;
            }
            
            os_kprintf("%s\r\n", dirent->d_name);
        }
    }
    else
    {
        os_size_t length;
        os_size_t min_length;

        min_length = 0;
        for (;;)
        {
            dirent = readdir(dir);
            if (OS_NULL == dirent)
            {
                break;
            }
            
            /* Matched the prefix string */
            if (!strncmp(index, dirent->d_name, strlen(index)))
            {
                os_kprintf("%s\r\n", dirent->d_name);
            
                if (0 == min_length)
                {
                    min_length = strlen(dirent->d_name);

                    /* Save dirent name */
                    strncpy(full_path, dirent->d_name, FULL_PATH_LEN_MAX);
                }

                length = sh_string_same_part_length(dirent->d_name, full_path);

                if (length < min_length)
                {
                    min_length = length;
                }
            }
        }

        if (min_length)
        {
            if ((index + min_length + 1) >= end_addr)
            {
                min_length = end_addr - index - 1;
            }
            memcpy(index, full_path, min_length);
            index[min_length] = '\0';
        }
    }

    closedir(dir);
    os_free(full_path);

#undef FULL_PATH_LEN_MAX
}

static os_bool_t sh_auto_complete_path(char *prefix, os_size_t size)
{
    char        *ptr;
    const char  *end_addr;
    os_bool_t    is_complete_path;

    is_complete_path = OS_FALSE;

    end_addr = prefix + size;
    ptr      = prefix + strlen(prefix);

    /* Check whether a space in the prefix */
    while (ptr != prefix)
    {
        if (*ptr == ' ')
        {
            is_complete_path = OS_TRUE;
            sh_do_auto_complete_path(ptr + 1, end_addr - (ptr + 1));

            break;
        }

        ptr--;
    }

    return is_complete_path;
}
#endif /* OS_USING_VFS */

/**
 ***********************************************************************************************************************
 * @brief           Auto complete the path name or command name.
 *
 * @param[in,out]   prefix          The prefix of path or command.
 * @param[in]       size            The size of buffer.
 * 
 * @return          None.
 ***********************************************************************************************************************
 */
void sh_auto_complete(char *prefix, os_size_t size)
{
    os_int32_t      length;
    os_int32_t      min_length;
    sh_cmd_entry_t *syscall_tmp;
    sh_cmd_entry_t *syscall_begin;
    sh_cmd_entry_t *syscall_end;
    const char     *name_ptr;
    const char     *cmd_name;
    os_bool_t       is_complete_path;
    os_bool_t       is_null_str;

    os_kprintf("\r\n");

    is_complete_path = OS_FALSE;
    is_null_str      = OS_FALSE;

    if (*prefix == '\0')
    {
        sh_exec("help");
        is_null_str = OS_TRUE;
    }

#ifdef OS_USING_VFS
    is_complete_path = sh_auto_complete_path(prefix, size);
#endif

    if ((OS_FALSE == is_null_str) && (OS_FALSE == is_complete_path))
    {
        min_length = 0;
        name_ptr   = OS_NULL;

        /* Checks in internal command */
        sh_get_cmd_table(&syscall_begin, &syscall_end);
    
        for (syscall_tmp = syscall_begin; syscall_tmp < syscall_end; SH_NEXT_CMD_ENTRY(syscall_tmp))
        {
            /* Skip shell function */
            if (strncmp(syscall_tmp->name, "__cmd_", 6))
            {
                continue;
            }
            
            cmd_name = &syscall_tmp->name[6];
            if (!strncmp(prefix, cmd_name, strlen(prefix)))
            {
                if (0 == min_length)
                {
                    /* Set name_ptr */
                    name_ptr = cmd_name;
                    
                    /* Set initial length */
                    min_length = strlen(name_ptr);

                    os_kprintf("\r\n");
                    os_kprintf("%s\r\n", cmd_name);
                }
                else
                {
                    length = sh_string_same_part_length(name_ptr, cmd_name);
                    if (length < min_length)
                    {
                        min_length = length;
                    }
                    
                    os_kprintf("%s\r\n", cmd_name);
                }
            }
        }

        /* Auto complete string */
        if (OS_NULL != name_ptr)
        {
            min_length = (min_length > size) ? size : min_length;
            strncpy(prefix, name_ptr, min_length);
        }
    }
    
    os_kprintf("%s%s", sh_get_prompt(), prefix);
    
    return;
}
#endif /* OS_USING_SHELL */

