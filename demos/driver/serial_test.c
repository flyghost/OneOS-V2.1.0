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

static int32_t  rx_thread_status = 0;
static int32_t  rx_total_cnt     = 0;
static uint16_t rx_crc           = 0;

static void rx_thread(void *parameter)
{
    int            rx_cnt;
    unsigned char *rx_buff = os_calloc(1, OS_SERIAL_RX_BUFSZ);
    os_device_t   *uart = (os_device_t *)parameter;

    OS_ASSERT(rx_buff);

    /* set device timeout */
    os_ubase_t timeout = OS_TICK_PER_SECOND;
    os_device_control(uart, OS_DEVICE_CTRL_SET_RX_TIMEOUT, &timeout);
    os_device_control(uart, OS_DEVICE_CTRL_SET_TX_TIMEOUT, &timeout);

    while (rx_thread_status == 0)
    {        
        rx_cnt = os_device_read_block(uart, 0, rx_buff, OS_SERIAL_RX_BUFSZ);
        if (rx_cnt <= 0)
            continue;

        rx_total_cnt += rx_cnt;
        os_kprintf("<%d>rx: %d/%d\r\n", os_tick_get(), rx_cnt, rx_total_cnt);
        hex_dump(rx_buff, rx_cnt);
        rx_crc = crc16(rx_crc, rx_buff, rx_cnt);
    }

    /* last pack */
    rx_cnt = os_device_read_block(uart, 0, rx_buff, OS_SERIAL_RX_BUFSZ);
    if (rx_cnt > 0)
    {
        rx_total_cnt += rx_cnt;
        os_kprintf("<%d>rx: %d/%d\r\n", os_tick_get(), rx_cnt, rx_total_cnt);
        hex_dump(rx_buff, rx_cnt);
        rx_crc = crc16(rx_crc, rx_buff, rx_cnt);
    }

    os_free(rx_buff);

    rx_thread_status = 2;
}

static int serial_block_test(int argc, char *argv[])
{
    int            ret, i, j, loops = 1;
    uint32_t       tx_index, rand_tx_cnt;
    unsigned char *tx_buff = os_calloc(1, OS_SERIAL_RX_BUFSZ);
    os_device_t   *uart;
    uint16_t       tx_crc = 0;

    OS_ASSERT(tx_buff);

    if (argc != 2 && argc != 3)
    {
        os_kprintf("usage: serial_block_test <dev> [times]\r\n");
        os_kprintf("       serial_block_test uart1 (default 1 times)\r\n");
        os_kprintf("       serial_block_test uart1 10000\r\n");
        return -1;
    }

    uart = os_device_find(argv[1]);
    OS_ASSERT(uart);

    if (argc == 3)
    {
        loops = strtol(argv[2], OS_NULL, 0);
    }

    os_device_open(uart);

    /* uart config */
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_2000000;
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
    if (ret != 0)
    {
        os_kprintf("serial baud 115200\r\n");
        config.baud_rate = BAUD_RATE_115200;
        ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
        if (ret != 0)
        {
            os_kprintf("serial control fail %d\r\n", ret);
            return -1;
        }
    }

    rx_thread_status = 0;
    rx_total_cnt     = 0;
    rx_crc           = 0;

    os_task_t *task = os_task_create("rx_thread", rx_thread, uart, 512, 4);
    OS_ASSERT(task);
    os_task_startup(task);

    for (i = 0; i < loops; i++)
    {
        tx_index = 0;

        for (j = 0; j < OS_SERIAL_RX_BUFSZ; j++)
        {
            tx_buff[j] = rand();
        }
        os_task_msleep(50);

        tx_crc = crc16(tx_crc, tx_buff, OS_SERIAL_RX_BUFSZ);

        while (tx_index < OS_SERIAL_RX_BUFSZ)
        {
            rand_tx_cnt = (OS_SERIAL_RX_BUFSZ - tx_index) * (rand() & 0xff) / 256 + 1;
            rand_tx_cnt = min(OS_SERIAL_RX_BUFSZ - tx_index, rand_tx_cnt);
            tx_index += os_device_write_block(uart, 0, tx_buff + tx_index, rand_tx_cnt);

            os_kprintf("<%d>uart tx loop:%d/%d, buff:%d/%d\r\n", os_tick_get(), i, loops, tx_index, OS_SERIAL_RX_BUFSZ);
        }
    }

    /* wait rx thread exit */
    os_task_msleep(300);
    rx_thread_status = 1;
    while (rx_thread_status != 2)
    {
        os_kprintf("wait rx thread exit..\r\n");
        os_task_msleep(300);
    }

    os_device_close(uart);

    os_kprintf("\r\n");
    os_kprintf("stat:\r\n");
    os_kprintf("    tx size %d\r\n", OS_SERIAL_RX_BUFSZ * loops);
    os_kprintf("\r\n");
    os_kprintf("    rx size %d\r\n", rx_total_cnt);
    os_kprintf("\r\n");
    os_kprintf("    %s tx_crc:%04x, rx_crc:%04x\r\n", (tx_crc == rx_crc) ? "success" : "failed", tx_crc, rx_crc);

    os_free(tx_buff);

    return 0;
}
SH_CMD_EXPORT(serial_block_test, serial_block_test, "serial_block_test");

static os_sem_t rx_sem;
static os_sem_t tx_sem;

static os_err_t rx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    os_sem_post(&rx_sem);
    return 0;
}

static os_err_t tx_done(os_device_t *uart, struct os_device_cb_info *info)
{
    os_sem_post(&tx_sem);
    return 0;
}

static int serial_nonblock_test(int argc, char *argv[])
{
    int           ret;
    int           tx_cnt;
    int           rx_cnt;
    unsigned char rx_buff[32];
    os_device_t  *uart;
    const char   *dev_name;

    if (argc != 2)
    {
        os_kprintf("usage: serial_nonblock_test <dev> \r\n");
        os_kprintf("       serial_nonblock_test uart2 \r\n");
        return -1;
    }

    dev_name = argv[1];

    uart = os_device_find(dev_name);
    OS_ASSERT(uart);

    /* open serial device */
    os_device_open(uart);

    /* uart config */
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_115200;
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
    if (ret != 0)
    {
        os_kprintf("serial control fail %d\r\n", ret);
        return -1;
    }

    /* nonblock callback sem */
    os_sem_init(&rx_sem, "nblk_rx", 0, 1);
    os_sem_init(&tx_sem, "nblk_tx", 0, 1);

    /* nonblock callback */
    struct os_device_cb_info cb_info;
    
    cb_info.type = OS_DEVICE_CB_TYPE_TX;
    cb_info.cb   = tx_done;
    os_device_control(uart, OS_DEVICE_CTRL_SET_CB, &cb_info);

    cb_info.type = OS_DEVICE_CB_TYPE_RX;
    cb_info.cb   = rx_done;
    os_device_control(uart, OS_DEVICE_CTRL_SET_CB, &cb_info);

    os_task_msleep(1);

    /* tx */
    tx_cnt = os_device_write_nonblock(uart, 0, "Hello World!\r\n", sizeof("Hello World!\r\n"));

    /* wait tx complete */
    os_sem_wait(&tx_sem, OS_WAIT_FOREVER);

    /* wait rx complete */
    os_sem_wait(&rx_sem, OS_WAIT_FOREVER);
    
    rx_cnt = os_device_read_nonblock(uart, 0, rx_buff, sizeof(rx_buff));

    os_kprintf("tx_cnt: %d, rx_cnt: %d\r\n", tx_cnt, rx_cnt);

    if (rx_cnt == sizeof("Hello World!\r\n"))
    {
        os_kprintf("rx buff:%s\r\n", rx_buff);
    }
    else
    {
        os_kprintf("rx failed\r\n");
    }

    os_device_close(uart);

    os_sem_deinit(&rx_sem);
    os_sem_deinit(&tx_sem);

    return 0;
}
SH_CMD_EXPORT(serial_nonblock_test, serial_nonblock_test, "serial_nonblock_test");

static int serial_rx_test(int argc, char *argv[])
{
    int            ret;
    int            rx_cnt;
    unsigned char *rx_buff;
    os_device_t   *uart;
    const char    *dev_name;

    if (argc != 2)
    {
        os_kprintf("usage: serial_rx_test <dev> \r\n");
        os_kprintf("       serial_rx_test uart2 \r\n");
        return -1;
    }

    dev_name = argv[1];

    uart = os_device_find(dev_name);
    OS_ASSERT(uart);

    os_device_open(uart);

    /* uart config */
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_115200;
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
    if (ret != 0)
    {
        os_kprintf("serial control fail %d\r\n", ret);
        return -1;
    }

    rx_buff = os_calloc(1, 128);
    OS_ASSERT(rx_buff);

    while (1)
    {
        do
        {
            rx_cnt = os_device_read_nonblock(uart, 0, rx_buff, 128);

            if (rx_cnt == 0)
                break;

            if (rx_cnt < 0)
                goto end;

            os_kprintf("rx_cnt: %d\r\n", rx_cnt);
            
            hex_dump(rx_buff, rx_cnt);
        } while (1);
    }

end:
    os_free(rx_buff);
    os_device_close(uart);

    return 0;
}
SH_CMD_EXPORT(serial_rx_test, serial_rx_test, "serial_rx_test");

static int serial_tx_test(int argc, char *argv[])
{
    int            ret;
    int            count;
    os_device_t   *uart;
    const char    *dev_name;

    if (argc != 2)
    {
        os_kprintf("usage: serial_tx_test <dev> \r\n");
        os_kprintf("       serial_tx_test uart2 \r\n");
        return -1;
    }

    dev_name = argv[1];

    uart = os_device_find(dev_name);
    OS_ASSERT(uart);

    os_device_open(uart);

    /* uart config */
    struct serial_configure config = OS_SERIAL_CONFIG_DEFAULT;
    config.baud_rate = BAUD_RATE_115200;
    ret = os_device_control(uart, OS_DEVICE_CTRL_CONFIG, &config);
    if (ret != 0)
    {
        os_kprintf("serial control fail %d\r\n", ret);
        return -1;
    }

    count = 0;
    do
    {
        count += os_device_write_nonblock(uart, 0, "hello world!\r\n", sizeof("hello world!\r\n") - count);

        if (count < 0)
            break;
        
    } while (count < sizeof("hello world!\r\n"));

    os_device_close(uart);

    return 0;
}
SH_CMD_EXPORT(serial_tx_test, serial_tx_test, "serial_tx_test");

