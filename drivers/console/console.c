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
 * @file        console.c
 *
 * @brief       this file implements console
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2020-10-29    OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <board.h>
#include <os_memory.h>
#include <driver.h>
#include <string.h>

extern void __os_hw_console_output(char *str);

static os_device_t *gs_console_device = OS_NULL;

/**
 ***********************************************************************************************************************
 * @brief           This function returns the device using in console.
 *
 * @param           No parameter.
 *
 * @return          The device using in console.
 * @retval          OS_NULL         There is no device in console.
 * @retval          else            The device pointer using in console.
 ***********************************************************************************************************************
 */
os_device_t *os_console_get_device(void)
{
    return gs_console_device;
}

/**
 ***********************************************************************************************************************
 * @brief           This function will set a device as console device.
 *
 * @details         After setting a device to console, all output of os_kprintf will be redirected to this new device.
 *
 *
 * @param[in]       name            The name of new console device.
 *
 * @return          The old console device pointer.
 * @retval          OS_NULL         Before setting a new device to console, there is no device in console. 
 * @retval          else            Return the old console device pointer.
 ***********************************************************************************************************************
 */
os_device_t *os_console_set_device(const char *name)
{
    os_device_t *new;
    os_device_t *old;
    
    /* Save old device */
    old = gs_console_device;

    /* Find new console device */
    new = os_device_find(name);
    if (OS_NULL != new)
    {
        if (OS_NULL != gs_console_device)
        {
            /* Close old console device */
            os_device_close(gs_console_device);
        }

        /* Set new console device */
        os_device_open(new);

        gs_console_device = new;
    }

    return old;
}

OS_WEAK void __os_hw_console_output(char *str)
{
    
}

void os_hw_console_output(char *log_buff)
{
    os_device_t *console = os_console_get_device();
    
    if (console == OS_NULL)
    {
        __os_hw_console_output(log_buff);
    }
    else
    {
        int count = 0;
        int send_index = 0;
        int size  = strlen(log_buff);
        
        while (send_index < size)
        {
            count = os_device_write_nonblock(console, 0, log_buff + send_index, size - send_index);
            if (count < 0)
            {
                return;
            }
            
            send_index += count;
        }
    }
    return;
}
    
os_err_t os_console_init(void)
{
    os_console_set_device(OS_CONSOLE_DEVICE_NAME);
    return OS_EOK;
}

OS_PREV_INIT(os_console_init, OS_INIT_SUBLEVEL_MIDDLE);

