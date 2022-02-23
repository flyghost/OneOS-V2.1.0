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
 * @file        drv_easyflash.c
 *
 * @brief       This file implements gpio driver for stm32.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */
 
 #include <stdio.h>
#include <board.h>
#include <os_memory.h>
#include <easyflash.h>

#ifdef OS_USING_SHELL
#include <drv_log.h>
#include <shell.h>
#endif


#ifdef OS_USING_SHELL

static os_err_t ef_init(void)
{
    os_kprintf("\r\n");
    
    if( easyflash_init() != EF_NO_ERR)
    {
        os_kprintf("Init easeflash failed.\r\n");
        return OS_ERROR;
    }
    
    os_kprintf("Init easeflash successfuuly.\r\n");
    
    return OS_EOK;
}
SH_CMD_EXPORT(ef_init, ef_init, "init easy-flash");

static os_err_t ef_env_print(void)
{
    os_kprintf("\r\n");
    
    ef_print_env();
    
    return OS_EOK;
}
SH_CMD_EXPORT(ef_print, ef_env_print, "print all ef env");

uint32_t get_ef_key_len(const char *key)
{
    return strlen(key);
}

uint32_t get_ef_value_len(const char *value)
{
    return strlen(value);
}

static os_err_t ef_set(os_int32_t argc, char **argv)
{    
    os_kprintf("\r\n");
    
    if ((argc == 1) || (argc == 2) || (argc > 3))
    {      
        os_kprintf("ef_setenv [key] [value]\r\n");
        return OS_EOK;    
    }
    else if (argc == 3) 
    {
        uint32_t key_len = get_ef_key_len(argv[1]);
        uint32_t value_len = get_ef_key_len(argv[2]);
        if (key_len < 4)
        {
            os_kprintf("length of [key] must over 4 bytes.\r\n");
            return OS_EOK;         
        }
        else if (value_len < 4)
        {
            os_kprintf("length of [value] must over 4 bytes.\r\n");
            return OS_EOK;          
        }
        else
        {
            if (ef_set_env(argv[1], argv[2]) != EF_NO_ERR)
            {
                os_kprintf("set easy-flash failed.\r\n");
                return OS_ERROR;
            }
        }
    }

    os_kprintf("set easy-flash successfully.\r\n");
    
    return OS_EOK;
}
SH_CMD_EXPORT(ef_set, ef_set, "set value of a specified env");

static os_err_t ef_delete(os_int32_t argc, char **argv)
{    
    os_kprintf("\r\n");

    if ((argc == 1) || (argc > 2))
    {      
        os_kprintf("ef_delete_env [key]\r\n");
        return OS_EOK;    
    }
    else if (argc == 2)
    {
        uint32_t key_len = get_ef_key_len(argv[1]); 
        if (key_len < 4)
        {
            os_kprintf("length of [key] must over 4 bytes.\r\n");
            return OS_EOK;         
        }   
        else
        {
            if (ef_set_env(argv[1], NULL) != EF_NO_ERR)
            {
                os_kprintf("delete easy-flash failed.\r\n");
                return OS_ERROR;
            }
        }        
    }
    
    os_kprintf("delete easy-flash successfully.\r\n");
    
    return OS_EOK;
}
SH_CMD_EXPORT(ef_delete, ef_delete, "delete a specified env");

static os_err_t ef_reset(void)
{
    os_kprintf("\r\n");
    
    if (ef_env_set_default() != EF_NO_ERR)
    {
        os_kprintf("reset easy-flash failed.\r\n");
        return OS_ERROR;
    }
    
    os_kprintf("reset easy-flash successfully.\r\n");
    
    return OS_EOK;
}
SH_CMD_EXPORT(ef_reset, ef_reset, "reset env as default");

#endif

