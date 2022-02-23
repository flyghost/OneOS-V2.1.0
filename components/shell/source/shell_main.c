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
 * @file        shell_main.c
 *
 * @brief       Main routine of shell.
 *
 * @revision
 * Date         Author          Notes
 * 2020-04-10   OneOS Team      First version.
 ***********************************************************************************************************************
 */

#include <os_errno.h>
#include <os_task.h>
#include <arch_interrupt.h>
#include <shell.h>
#include <os_util.h>
#include <os_memory.h>
#include <os_assert.h>
#include <device.h>
#include <string.h>
#include <stdio.h>

#include <os_sem.h>
#include <console.h>

#if defined(OS_USING_VFS)
#include <unistd.h>
#endif

#include "shell_internal.h"

#ifdef OS_USING_SHELL

#if (SHELL_TASK_PRIORITY > OS_TASK_PRIORITY_MAX - 1)
#error "Shell task priority is greater than or equal to OS_TASK_PRIORITY_MAX"
#endif

static OS_TASK_STACK_DEFINE(shell_stack, SHELL_TASK_STACK_SIZE);
static os_task_t          gs_shell_task = {0};

static shell_ctrl_info_t  gs_shell = {0};
static char              *gs_shell_prompt_custom = OS_NULL;

#ifdef OS_USING_SYS_HEAP
/**
 ***********************************************************************************************************************
 * @brief           Set prompt for shell.
 *
 * @param[in]       prompt          New prompt string.
 *
 * @return          Set prompt result.
 * @retval          OS_EOK          Set prompt success.
 * @retval          else            Set prompt failed.
 ***********************************************************************************************************************
 */
os_err_t sh_do_set_prompt(const char *prompt)
{
    os_err_t ret;

    ret = OS_EOK;

    if (gs_shell_prompt_custom)
    {
        os_free(gs_shell_prompt_custom);
        gs_shell_prompt_custom = OS_NULL;
    }

    /* Strdup */
    if (prompt)
    {
        gs_shell_prompt_custom = os_malloc(strlen(prompt) + 1);
        if (gs_shell_prompt_custom)
        {
            (void)strcpy(gs_shell_prompt_custom, prompt);
        }
        else
        {
            os_kprintf("Malloc prompt failed\r\n");
            ret = OS_ENOMEM;
        }
    }

    return ret;
}
#endif /* OS_USING_SYS_HEAP */

/**
 ***********************************************************************************************************************
 * @brief           Get prompt of shell.
 *
 * @param           None.
 *
 * @return          The prompt string.
 ***********************************************************************************************************************
 */
const char *sh_get_prompt(void)
{
#define _PROMPT     "sh"

    os_int32_t  resv_space;
    static char s_shell_prompt[SHELL_PROMPT_SIZE + 1];

    /* check prompt mode */
    if (!gs_shell.enable_prompt)
    {
        s_shell_prompt[0] = '\0';
    }
    else
    {
#if defined(OS_USING_VFS) && defined(OS_USING_LIBC_ADAPTER)
        resv_space = strlen(" ") + strlen(">") + 1;  
#else
        resv_space = strlen(">") + 1;
#endif

        OS_ASSERT(sizeof(s_shell_prompt) > resv_space);

        (void)memset(s_shell_prompt, 0, sizeof(s_shell_prompt));

        if (gs_shell_prompt_custom)
        {
            (void)strncpy(s_shell_prompt, gs_shell_prompt_custom, sizeof(s_shell_prompt) - resv_space);
        }
        else
        {
            (void)strncpy(s_shell_prompt, _PROMPT, sizeof(s_shell_prompt) - resv_space);
        }
       
#if defined(OS_USING_VFS) && defined(OS_USING_LIBC_ADAPTER)
        (void)strcat(s_shell_prompt, " ");

        resv_space = resv_space - strlen(" ");

        /* Get current working directory */
        getcwd(&s_shell_prompt[strlen(s_shell_prompt)], sizeof(s_shell_prompt) - strlen(s_shell_prompt) - resv_space);
#endif

        (void)strcat(s_shell_prompt, ">");
    }
    
    return s_shell_prompt;
}

static os_err_t sh_rx_ind(os_device_t *dev, struct os_device_cb_info *info)
{
    /* Release semaphore to let shell task rx data */
    (void)os_sem_post(&gs_shell.rx_sem);;

    return OS_EOK;
}

static os_err_t sh_do_connect_console(shell_ctrl_info_t *shell_info)
{
    os_device_t *console;
    os_err_t     ret;

    ret     = OS_EOK;
    console = os_console_get_device();

    if (OS_NULL == console)
    {
        ret = OS_ENODEV;
    }
    else
    {
        if (OS_NULL != shell_info->device)
        {
            ret = os_device_control(shell_info->device, OS_DEVICE_CTRL_RM_CB, &shell_info->cb_info);
            if (OS_EOK != ret)
            {
                OS_ASSERT_EX(0, "Why control console device(%s) failed?", device_name(shell_info->device));
            }
        }

        /* Clear line buffer before switch to new device */
        (void)memset(shell_info->line, 0, sizeof(shell_info->line));
        shell_info->line_curpos   = 0;
        shell_info->line_position = 0;
        shell_info->device = console;

        ret = os_device_control(shell_info->device, OS_DEVICE_CTRL_SET_CB, &shell_info->cb_info);
        if (OS_EOK != ret)
        {
            OS_ASSERT_EX(0, "Why control console device(%s) failed?", device_name(shell_info->device));
        }
    }

    return ret;
}

static void sh_connect_console(shell_ctrl_info_t *shell_info)
{
    os_err_t ret;

    while (1)
    {
        ret = sh_do_connect_console(shell_info);
        if (OS_EOK != ret)
        {
            (void)os_task_msleep(100U);
            continue;
        }
        else
        {
            break;
        }
    }

    return;
}

static char sh_get_char(shell_ctrl_info_t *shell_info)
{
    os_size_t size;
    char      ch;
    os_err_t  ret;

    while (1)
    {
        ret = OS_EOK;

        (void)os_mutex_lock(&shell_info->dev_mutex, OS_WAIT_FOREVER);
        
        if (OS_NULL != shell_info->device)
        {
            size = os_device_read_nonblock(shell_info->device, -1, &ch, 1U);
            if (1U != size)
            {
                ret = OS_EEMPTY;
            }
        }
        else
        {
            ret = OS_ENODEV;
        }
        
        (void)os_mutex_unlock(&shell_info->dev_mutex);

        if (OS_EOK != ret)
        {
            (void)os_sem_wait(&shell_info->rx_sem, OS_WAIT_FOREVER);
        }
        else
        {
            break;
        }
    }
    
    return ch;   
}

#ifdef SHELL_USING_HISTORY
static void sh_print_history(const char *history)
{
    os_kprintf("\033[2K\r");
    os_kprintf("%s%s", sh_get_prompt(), history);
    
    return;
}

static void sh_push_history(shell_ctrl_info_t *shell_info)
{
    if (0 != shell_info->line_position)
    {
        /* Push history */
        if (shell_info->history_count >= SHELL_HISTORY_LINES)
        {
            /* If current cmd is same as last cmd, don't push */
            if (strncmp(&shell_info->cmd_history[SHELL_HISTORY_LINES - 1][0], &shell_info->line[0], SHELL_CMD_SIZE + 1))
            {
                /* Move history */
                os_int32_t index;
                
                for (index = 0; index < SHELL_HISTORY_LINES - 1; index++)
                {
                    (void)strcpy(&shell_info->cmd_history[index][0], 
                                 &shell_info->cmd_history[index + 1][0]);
                }
                
                (void)strcpy(&shell_info->cmd_history[SHELL_HISTORY_LINES - 1][0], 
                            shell_info->line);

                /* It's the maximum history */
                shell_info->history_count = SHELL_HISTORY_LINES;
            }
        }
        else
        {
            /* If current cmd is same as last cmd, don't push */
            if ((0 == shell_info->history_count)
                || strncmp(&shell_info->cmd_history[shell_info->history_count - 1][0], &shell_info->line[0], SHELL_CMD_SIZE + 1))
            {
                (void)strncpy(&shell_info->cmd_history[shell_info->history_count][0], shell_info->line, shell_info->line_position);

                /* increase count and set current history position */
                shell_info->history_count++;
            }
        }
    }
    
    shell_info->current_history = shell_info->history_count;

    return;
}

static void sh_handle_up_key(shell_ctrl_info_t *shell_info)
{
    if (shell_info->current_history > 0)
    {
        shell_info->current_history--;
        sh_print_history(&shell_info->cmd_history[shell_info->current_history][0]);
    }

    return;
}

static void sh_handle_down_key(shell_ctrl_info_t *shell_info)
{
    if (shell_info->history_count > 0)
    {
        if (shell_info->current_history < shell_info->history_count)
        {
            shell_info->current_history++;
        }
        if (shell_info->current_history < shell_info->history_count)
        {
            sh_print_history(&shell_info->cmd_history[shell_info->current_history][0]);
        }
        else
        {
            sh_print_history(&shell_info->line[0]);
        }
    }

    return;
}

static void sh_copy_history_cmd(shell_ctrl_info_t *shell_info)
{
    if (shell_info->current_history < shell_info->history_count)
    {
        (void)strcpy(&shell_info->line[0], &shell_info->cmd_history[shell_info->current_history][0]);
        shell_info->line_position = strlen(shell_info->line);
        shell_info->line_curpos = strlen(shell_info->line);
        shell_info->current_history = shell_info->history_count;
    }
}
#endif /* SHELL_USING_HISTORY */

#ifdef SHELL_USING_AUTH
static os_err_t sh_set_password(const char *password)
{
    os_ubase_t irq_save;
    os_size_t  pw_len;

    pw_len = strlen(password);

    if ((pw_len < SHELL_PASSWORD_MIN) || (pw_len > SHELL_PASSWORD_MAX))
    {
        os_kprintf("Invalid password length(%u) of shell\r\n", pw_len);
        return OS_EINVAL;
    }
    
    irq_save = os_irq_lock();
    (void)strncpy(gs_shell.password, password, SHELL_PASSWORD_MAX);
    os_irq_unlock(irq_save);

    return OS_EOK;
}

static void sh_wait_auth(shell_ctrl_info_t *shell_info)
{
    char      ch;
    os_bool_t input_shell;
    char      password[SHELL_PASSWORD_MAX];
    os_size_t cur_pos;

    (void)memset(password, 0, sizeof(password));
    input_shell = OS_FALSE;
    cur_pos     = 0;

    /* Password not set */
    if (!strlen(shell_info->password))
    {
        return;
    }
    
    while (1)
    {
        os_kprintf("Password for login: ");
        
        while (!input_shell)
        {
            while (1)
            {
                /* read one character from device */
                ch = sh_get_char(shell_info);

                if ((ch >= ' ') && (ch <= '~') && (cur_pos < SHELL_PASSWORD_MAX))
                {
                    /* Change the printable characters to '*' */
                    os_kprintf("*");
                    password[cur_pos++] = ch;
                }
                else if ((ch == '\b') && (cur_pos > 0))
                {
                    /* Backspace */
                    password[cur_pos] = '\0';
                    cur_pos--;
                    os_kprintf("\b \b");
                }
                else if ((ch == '\r') || (ch == '\n'))
                {
                    os_kprintf("\r\n");
                    input_shell = OS_TRUE;
                    break;
                }
            }
        }
        
        if (!strncmp(shell_info->password, password, SHELL_PASSWORD_MAX))
        {
            return;
        }
        else
        {
            /* Authentication failed, delay 2S for retry */
            (void)os_task_msleep(2000);
            
            os_kprintf("Sorry, try again.\r\n");
            
            cur_pos = 0;
            input_shell = OS_FALSE;
            (void)memset(password, 0, sizeof(password));
        }
    }
}

static void sh_auth(shell_ctrl_info_t *shell_info)
{
    os_err_t ret;

    /* Set the default password when the password isn't setting */
    if (!strlen(shell_info->password) && strlen(SHELL_DEFAULT_PASSWORD))
    {
        ret = sh_set_password(SHELL_DEFAULT_PASSWORD);
        if (OS_EOK != ret)
        {
            os_kprintf("Shell password set failed.\r\n");
        }
    }
    
    /* Waiting authenticate success */
    sh_wait_auth(shell_info);

    return;
}
#endif /* SHELL_USING_AUTH */

static void sh_handle_left_key(shell_ctrl_info_t *shell_info)
{   
#ifdef SHELL_USING_HISTORY
    sh_copy_history_cmd(shell_info);
#endif
    if (shell_info->line_curpos)
    {
        os_kprintf("\b");
        shell_info->line_curpos--;
    }

    return;
}

static void sh_handle_right_key(shell_ctrl_info_t *shell_info)
{
#ifdef SHELL_USING_HISTORY
    sh_copy_history_cmd(shell_info);
#endif
    if (shell_info->line_curpos < shell_info->line_position)
    {
        os_kprintf("%c", shell_info->line[shell_info->line_curpos]);
        shell_info->line_curpos++;
    }

    return;
}

static void sh_handle_tab_key(shell_ctrl_info_t *shell_info)
{
#ifdef SHELL_USING_HISTORY
    sh_copy_history_cmd(shell_info);
#endif
    sh_auto_complete(&shell_info->line[0], sizeof(shell_info->line));

    /* Re-calculate position */
    shell_info->line_curpos   = strlen(shell_info->line);
    shell_info->line_position = strlen(shell_info->line);
    
    return;
}

static void sh_handle_backspace_key(shell_ctrl_info_t *shell_info)
{
    os_uint8_t pos;
    
#ifdef SHELL_USING_HISTORY
    sh_copy_history_cmd(shell_info);
#endif

    if (shell_info->line_curpos > 0U)
    {
        shell_info->line_position--;
        shell_info->line_curpos--;

        if (shell_info->line_position > shell_info->line_curpos)
        {
            (void)strcpy(&shell_info->line[shell_info->line_curpos], &shell_info->line[shell_info->line_curpos + 1U]);

            os_kprintf("\b%s  \b", &shell_info->line[shell_info->line_curpos]);

            /* Move the cursor to the origin position */
            for (pos = shell_info->line_curpos; pos <= shell_info->line_position; pos++)
            {
                os_kprintf("\b");
            }
        }
        else
        {
            os_kprintf("\b \b");
            shell_info->line[shell_info->line_position] = '\0';
        }
    }
    
    return;
}

static void sh_handle_enter_key(shell_ctrl_info_t *shell_info)
{
#ifdef SHELL_USING_HISTORY
    sh_copy_history_cmd(shell_info);
    sh_push_history(shell_info);
#endif

    if (shell_info->enable_echo)
    {
        os_kprintf("\r\n");
    }

    (void)sh_do_exec(shell_info->line, shell_info->line_position);
    
    os_kprintf("%s", sh_get_prompt());
    
    (void)memset(shell_info->line, 0, sizeof(shell_info->line));
    shell_info->line_curpos   = 0U;
    shell_info->line_position = 0U;
    
    return;
}

static os_bool_t sh_handle_control_key(shell_ctrl_info_t *shell_info, char ch)
{
    os_bool_t is_direction_key;
    os_bool_t is_char_handled;
    
    is_direction_key = OS_TRUE;
    is_char_handled  = OS_TRUE;

    /*
     * Direction key.
     * up key   : 0x1b 0x5b 0x41
     * down key : 0x1b 0x5b 0x42
     * right key: 0x1b 0x5b 0x43
     * left key : 0x1b 0x5b 0x44
     */
    if (ch == (char)0x1b)
    {
        shell_info->stat = SHELL_WAIT_SPEC_KEY;
    }
    else if (shell_info->stat == SHELL_WAIT_SPEC_KEY)
    {
        if (ch == (char)0x5b)
        {
            shell_info->stat = SHELL_WAIT_FUNC_KEY;
        }
        else
        {
            shell_info->stat = SHELL_WAIT_NORMAL;
            is_direction_key = OS_FALSE;
        }
    }
    else if (shell_info->stat == SHELL_WAIT_FUNC_KEY)
    {
        shell_info->stat = SHELL_WAIT_NORMAL;

        if (ch == (char)0x41)     /* Up key */
        {
#ifdef SHELL_USING_HISTORY
            sh_handle_up_key(shell_info);
#endif
        }
        else if (ch == (char)0x42)    /* Down key */
        {
#ifdef SHELL_USING_HISTORY
            sh_handle_down_key(shell_info);
#endif
        }
        else if (ch == (char)0x44) /* Left key */
        {
            sh_handle_left_key(shell_info);
        }
        else if (ch == (char)0x43) /* Right key */
        {
            sh_handle_right_key(shell_info);
        }
        else
        {
            is_direction_key = OS_FALSE;
        }
    }
    else
    {
        is_direction_key = OS_FALSE;
    }

    if (OS_FALSE == is_direction_key)
    {
        if ((ch == '\0') || (ch == (char)0xff))           /* Received null or error */
        {
            /* Discard, do nothing. */
            ;
        }
        else if (ch == '\t')                        /* Handle tab key */
        {
            sh_handle_tab_key(shell_info);
        }
        else if ((ch == (char)0x7f) || (ch == (char)0x08))      /* Handle backspace key */
        {
            sh_handle_backspace_key(shell_info);
        }
        else if ((ch == '\r') || (ch == '\n'))
        {
            sh_handle_enter_key(shell_info);
        }
        else
        {
            is_char_handled = OS_FALSE;
        }
    }

    return is_char_handled;
}

static void sh_handle_normal_character(shell_ctrl_info_t *shell_info, char ch)
{
    os_uint8_t pos;
    
#ifdef SHELL_USING_HISTORY
    sh_copy_history_cmd(shell_info);
#endif

    if (shell_info->line_position < SHELL_CMD_SIZE)
    {
        /* Normal character */  
        if (shell_info->line_curpos < shell_info->line_position)
        {
            (void)memmove(&shell_info->line[shell_info->line_curpos + 1],
                          &shell_info->line[shell_info->line_curpos],
                          shell_info->line_position - shell_info->line_curpos);

            shell_info->line[shell_info->line_curpos] = ch;
            
            if (shell_info->enable_echo)
            {
                os_kprintf("%s", &shell_info->line[shell_info->line_curpos]);
            }
            
            /* Move the cursor to new position */
            for (pos = shell_info->line_curpos; pos < shell_info->line_position; pos++)
            {
                os_kprintf("\b");
            }
        }
        else
        {
            shell_info->line[shell_info->line_position] = ch;
            if (shell_info->enable_echo)
            {
                os_kprintf("%c", ch);
            }
        }

        shell_info->line_position++;
        shell_info->line_curpos++;
    }
    else
    {
        /* It's a large line, discard it, do nothing. */
        ;
    }

    return;
}

static void sh_task_entry(void *arg)
{
    shell_ctrl_info_t *shell_info;
    os_bool_t is_char_handled;
    char      ch;

    OS_ASSERT(OS_NULL != arg);

    shell_info = (shell_ctrl_info_t *)arg;

    sh_connect_console(shell_info);

#ifdef SHELL_USING_AUTH
    sh_auth(shell_info);
#endif

    os_kprintf("\r\n");
    os_kprintf(sh_get_prompt());

    while (1)
    {
        ch = sh_get_char(shell_info);

        is_char_handled = sh_handle_control_key(shell_info, ch);
        if (OS_FALSE == is_char_handled)
        {
            sh_handle_normal_character(shell_info, ch);
        }
    }
}

#if defined(__ICCARM__) || defined(__ICCRX__)   /* for IAR compiler */
#pragma section="FSymTab"
#endif

static os_err_t sh_system_init(void)
{
    os_err_t ret;

#if defined(__CC_ARM) || defined(__CLANG_ARM)                   /* ARM C Compiler */
    extern const int FSymTab$$Base;
    extern const int FSymTab$$Limit;
    sh_init_cmd_table(&FSymTab$$Base, &FSymTab$$Limit);
    
#elif defined(__ICCARM__) || defined(__ICCRX__)                /* for IAR Compiler */
    sh_init_cmd_table(__section_begin("FSymTab"), __section_end("FSymTab"));
    
#elif defined(__GNUC__)                                         /* GNU GCC Compiler */
    extern const int __fsymtab_start;
    extern const int __fsymtab_end;
    sh_init_cmd_table(&__fsymtab_start, &__fsymtab_end);
    
#else
    #error "Not supported the tool chain."
#endif
 
    ret = os_task_init(&gs_shell_task,
                       SHELL_TASK_NAME,
                       sh_task_entry,
                       &gs_shell,
                       OS_TASK_STACK_BEGIN_ADDR(shell_stack),
                       OS_TASK_STACK_SIZE(shell_stack),
                       SHELL_TASK_PRIORITY);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Init shell task failed, ret(%d)", ret);
    }

    ret = os_sem_init(&gs_shell.rx_sem, "shrx", 0, OS_SEM_MAX_VALUE);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Init shrx semaphore failed, ret(%d)", ret);
    }

    ret = os_mutex_init(&gs_shell.dev_mutex, "shdev", OS_FALSE);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Init shdev mutex failed, ret(%d)", ret);
    }

    gs_shell.device        = OS_NULL;
    gs_shell.cb_info.type  = OS_DEVICE_CB_TYPE_RX;
    gs_shell.cb_info.cb    = sh_rx_ind;
    gs_shell.cb_info.data  = OS_NULL;
    gs_shell.cb_info.size  = 0;

    gs_shell.stat          = SHELL_WAIT_NORMAL;
    gs_shell.enable_prompt = OS_TRUE;

#ifndef SHELL_ECHO_DISABLE_DEFAULT
    gs_shell.enable_echo   = OS_TRUE;
#else
    gs_shell.enable_echo   = OS_FALSE;
#endif

    ret = os_task_startup(&gs_shell_task);
    if (OS_EOK != ret)
    {
        OS_ASSERT_EX(0, "Startup shell task failed, ret(%d)", ret);
    }

    return ret;
}
OS_CMPOENT_INIT(sh_system_init, OS_INIT_SUBLEVEL_LOW);

void sh_disconnect_console(void)
{ 
    os_err_t ret;

    (void)os_mutex_lock(&gs_shell.dev_mutex, OS_WAIT_FOREVER);

    if (OS_NULL != gs_shell.device)
    {
        ret = os_device_control(gs_shell.device, OS_DEVICE_CTRL_RM_CB, &gs_shell.cb_info);
        if (OS_EOK != ret)
        {
            OS_ASSERT_EX(0, "Why control console device(%s) failed?", device_name(gs_shell.device));
        }
        
        gs_shell.device = OS_NULL;
    }
    
    (void)os_mutex_unlock(&gs_shell.dev_mutex);

    return;
}

void sh_reconnect_console(void)
{
    os_err_t ret;

    ret = sh_do_connect_console(&gs_shell);
    if (OS_EOK != ret)
    {
        os_kprintf("Reconnect console failed.\r\n");
    }

    return;
}
#endif /* OS_USING_SHELL */

