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
 * @file        main.c
 *
 * @brief       User application entry
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>
#include <sdram_port.h>
#include "ui.h"
#include "airconditioner_ui.h"

//zhw debug
int g_user_task_end = 0;
int g_push_button_0_val = 0;
unsigned char *log_buff = NULL;

static void read_push_button_0(void)
{
	os_pin_mode(key_table[0].pin, key_table[0].mode);
	g_push_button_0_val = os_pin_read(key_table[0].pin);
	os_kprintf("Pushed button %d.\n", g_push_button_0_val);
}

static void user_task(void *parameter)
{
    int i = 0;
	read_push_button_0();
    for (i = 0; i < led_table_size; i++)
    {
        os_pin_mode(led_table[i].pin, PIN_MODE_OUTPUT);
		os_pin_write(led_table[i].pin, !led_table[i].active_level);
    }
#if 1
    while (!g_user_task_end)
    {
        if (g_push_button_0_val == 0) //pushed key
        {
            for (i = 0; i < led_table_size; i++)
            {
                os_pin_write(led_table[i].pin, led_table[i].active_level);
                os_task_msleep(200);

                os_pin_write(led_table[i].pin, !led_table[i].active_level);
                os_task_msleep(200);
            }
        }
        else
        os_task_msleep(1000);
    }
    os_task_msleep(2000);
    for (i = 0; i < led_table_size; i++)
    {
            os_pin_write(led_table[i].pin, led_table[i].active_level);
    }
    // i = 0;
    // while(1)
    // {
    //     os_task_msleep(1000);
    //     ui_set_temperature(i);
    //     ui_set_binary_switch(i);
    //     i ++;
    // }		
#endif

}

int main(void)
{
    os_task_t *task;
    task = os_task_create("user", user_task, NULL, 2048, 3);
    OS_ASSERT(task);
    os_task_startup(task);

#ifdef USE_SDRAM_HEAP
    log_buff = sdram_alloc(128 * 1024);
    os_kprintf("Log buffer inited.\r\n");
    OS_ASSERT(NULL != log_buff);
#else

#endif

#ifdef OCF_USE_GUI
    os_task_msleep(5000);
    show();
#endif

    return 0;
}


#include <shell.h>
#include <os_task.h>
#include <mo_api.h>
#include <serial.h>

static struct serial_configure uart_config = OS_SERIAL_CONFIG_DEFAULT;

os_err_t test_func(int argc, char **argv)
{

    os_device_t *device = os_device_find("uart3");

    if (OS_NULL == device)
    {
        os_kprintf("Auto create failed, Can not find ESP8266 interface device!\r\n");
        return OS_ERROR;
    }

    uart_config.baud_rate = 115200;

    os_device_control(device, OS_DEVICE_CTRL_CONFIG, &uart_config);
    mo_parser_config_t parser_config = {.parser_name = "8266",
                                        .parser_device = device,
                                        .recv_buff_len = 1024};

    mo_object_t *obj = mo_create("8266", MODULE_TYPE_ESP8266, &parser_config);

    if (OS_NULL == obj)
    {
        os_kprintf("creat failed!\r\n");
        return OS_ERROR;
    }
    return OS_EOK;
}
SH_CMD_EXPORT(test, test_func, "open 8266");
