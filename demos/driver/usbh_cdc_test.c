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
 * @file        serial_test.c
 *
 * @brief       The test file for serial.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_clock.h>
#include <os_memory.h>
#include <os_sem.h>
#include <stdint.h>
#include <stdlib.h>
#include <shell.h>
#include <serial/serial.h>

os_device_t   *g_usbh_cdc_cdc;

static void usbh_read_task(void *parameter)
{
    unsigned char rx_buff[32];
    int i;
    int           rx_cnt;
    while (1)
    {
        rx_cnt = os_device_read_block( g_usbh_cdc_cdc, 0, rx_buff, 32);
        /* wait rx complete */
        if(rx_cnt > 0)
        {
            for (i=0; i<rx_cnt; i++)
            os_kprintf("%c", rx_buff[i]);
        }
        else
            break;
    }
}

static int usbh_cdc_open(int argc, char *argv[])
{
    os_device_t   *usbh_cdc;
    os_task_t *task_handle = NULL;
    
    if (argc != 1)
    {
        os_kprintf("usage: usbh_cdc_open\r\n");
        return -1;
    }

    usbh_cdc = os_device_find("usbh_cdc_hs");
    OS_ASSERT(usbh_cdc);

    os_device_open(usbh_cdc);
    g_usbh_cdc_cdc = usbh_cdc;
    
    task_handle = os_task_create("usbh_read_task", usbh_read_task, NULL, 512, 3);

    if (task_handle == NULL)
    {
        os_kprintf("usbh_read_task create fail!\r\n");
        return -1;
    }
    os_task_msleep(200);
    os_task_startup(task_handle);
    
    return 0;
}
SH_CMD_EXPORT(usbh_cdc_open, usbh_cdc_open, "usbh_cdc_open");


int usbh_cdc_close()
{    
    if (g_usbh_cdc_cdc != NULL)
    {        
        os_device_close(g_usbh_cdc_cdc);
        g_usbh_cdc_cdc = NULL;
    }
    os_task_msleep(200);

    
    return 0;
}



unsigned char tx_buff[32];
static int usbh_cdc_send(int argc, char *argv[])
{
    int           tx_cnt;
    int cmd_length = 0;
    
    
    if (argc != 2)
    {
        os_kprintf("usage: usbh_cdc_send <cmd> \r\n");
        os_kprintf("       usbh_cdc_send AT\r\n");
        return -1;
    }
    
    if( g_usbh_cdc_cdc<0 )
    {
		os_kprintf( "%s:%d open usb dce failed.", __MODULE__, __LINE__ );
        return -1;
	}
    cmd_length = strlen(argv[1]);
    memcpy(tx_buff, argv[1], cmd_length);
    memcpy(tx_buff + cmd_length, "\r\n", strlen("\r\n"));
    /* Tx */
    tx_cnt=os_device_write_nonblock( g_usbh_cdc_cdc, 0, tx_buff, cmd_length + 2);

    os_kprintf("tx_cnt = %d\r\n",tx_cnt);
    return 0;
}
SH_CMD_EXPORT(usbh_cdc_send, usbh_cdc_send, "usbh_cdc_send");

extern int ec20_auto_create(void);
static int ec20_create(int argc, char *argv[])
{
    ec20_auto_create();
    return OS_EOK;
}
SH_CMD_EXPORT(ec20_create, ec20_create, "ec20_create");


#include "device.h"
#include <console.h>
#include "usbh_hid_mouse.h"

os_err_t usbh_cdc_notify_callback(os_device_t *dev, os_ubase_t event, os_ubase_t args)
{
    if(strcmp(dev->name, "usbh_cdc_hs"))
    {
        return OS_EOK;
    }
    switch(event)
    {
        case OS_DEV_OPS_TYPE_REGISTER:
            os_kprintf("%s register: %d\r\n", dev->name, args);
            usbh_cdc_open(1, NULL);
            break;
        case OS_DEV_OPS_TYPE_UNREGISTER:
            os_kprintf("%s unregister: %d\r\n", dev->name, args);
            usbh_cdc_close();
            break;
        case OS_DEV_OPS_TYPE_OPEN:
            os_kprintf("%s open: %d\r\n", dev->name, args);
            break;
        case OS_DEV_OPS_TYPE_CLOSE:
            os_kprintf("%s close: %d\r\n", dev->name, args);
            break;
        case OS_DEV_OPS_TYPE_READ_BLOCK:
            os_kprintf("%s read block: %d\r\n", dev->name, args);
            break;
        case OS_DEV_OPS_TYPE_READ_NONBLOCK:
            os_kprintf("%s read nonblock: %d\r\n", dev->name, args);
            break;
        case OS_DEV_OPS_TYPE_WRITE_BLOCK:
            os_kprintf("%s write block: %d\r\n", dev->name, args);
            break;
        case OS_DEV_OPS_TYPE_WRITE_NONBLOCK:
            os_kprintf("%s write nonblock: %d\r\n", dev->name, args);
            break;
        case OS_DEV_OPS_TYPE_CONTROL:
            os_kprintf("%s control: %d\r\n", dev->name, args);
            break;
        default:
            break;
    }
    return OS_EOK;
}

os_err_t usbh_cdc_init(void)
{
    os_device_notify_register(OS_NULL, device_ops_notify_callback, OS_NULL);
    
    return OS_EOK;
}

OS_DEVICE_INIT(usbh_cdc_init, OS_INIT_SUBLEVEL_HIGH);


