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
 * @file        select_test.c
 *
 * @brief       The test file for select.
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <drv_cfg.h>
#include <os_clock.h>
#include <os_memory.h>
#include <shell.h>
#include <unistd.h>
#include <vfs_select.h>
#include <serial/serial.h>
#include <fcntl.h>

static unsigned char send_buff[OS_SERIAL_RX_BUFSZ];

static int select_test(int argc, char *argv[])
{
    int ret;
    int i, j, loops = 1;
    int fd;
    const char *dev_name;

    if (argc != 2 && argc != 3)
    {
        os_kprintf("usage: select_test <dev> [times]\r\n");
        os_kprintf("       select_test /dev/uart2 (default 1 times)\r\n");
        os_kprintf("       select_test /dev/uart2 10000\r\n");
        return -1;
    }

    if (argc == 3)
    {
        loops = strtol(argv[2], NULL, 0);
    }

    int wait_ms = 3500;

    int            nfds;
    fd_set         readfds;
    fd_set         writefds;
    fd_set         exceptfds;
    struct timeval timeout;
    int            rand_send_cnt, send_cnt, recv_cnt, recv_total_cnt = 0;
    unsigned char *recv_buff = os_calloc(1, OS_SERIAL_RX_BUFSZ);

    uint16_t tx_crc = 0;
    uint16_t rx_crc = 0;

    OS_ASSERT(recv_buff);

    dev_name = argv[1];
    fd       = open(dev_name, O_RDWR);

    os_kprintf("dev:%s, fd:%d\r\n", dev_name, fd);

    timeout.tv_sec  = wait_ms / 1000;
    timeout.tv_usec = (wait_ms % 1000) * 1000;

    for (i = 0; i < loops; i++)
    {
        for (j = 0; j < sizeof(send_buff); j++)
        {
            send_buff[j] = rand();
        }

        tx_crc = crc16(tx_crc, send_buff, sizeof(send_buff));

        send_cnt = 0;

        while (send_cnt < sizeof(send_buff))
        {
            FD_ZERO(&readfds);
            FD_ZERO(&writefds);
            FD_ZERO(&exceptfds);

            FD_SET(fd, &readfds);
            FD_SET(fd, &writefds);
            FD_SET(fd, &exceptfds);

            nfds = fd + 1;

            os_kprintf("<%d>before select %d/%d\r\n", os_tick_get(), i, loops);

            ret = select(nfds, &readfds, &writefds, &exceptfds, &timeout);

            os_kprintf("<%d>after select, ret:%d\r\n", os_tick_get(), ret);

            if (ret == 0)
            {
                os_kprintf("<%d>select timeout, no exception\r\n", os_tick_get());
                continue;
            }

            /* Read */
            if (FD_ISSET(fd, &readfds))
            {
                os_kprintf("<%d>rx ready, recv...\r\n", os_tick_get());
                recv_cnt = read(fd, recv_buff, OS_SERIAL_RX_BUFSZ);
                recv_total_cnt += recv_cnt;
                os_kprintf("<%d>recv %d, total %d :\r\n", os_tick_get(), recv_cnt, recv_total_cnt);
                hex_dump(recv_buff, recv_cnt);
                rx_crc = crc16(rx_crc, recv_buff, recv_cnt);
            }
            else
            {
                os_kprintf("<%d>wait rx ready timeout.\r\n", os_tick_get());
            }

            /* Write */
            if (FD_ISSET(fd, &writefds))
            {
                os_kprintf("<%d>tx ready, send...\r\n", os_tick_get());
                rand_send_cnt = (sizeof(send_buff) - send_cnt) * (rand() & 0xff) / 256 + 1;
                rand_send_cnt = min(sizeof(send_buff) - send_cnt, rand_send_cnt);
                send_cnt += write(fd, send_buff + send_cnt, rand_send_cnt);
                os_kprintf("<%d>send %d/%d\r\n", os_tick_get(), send_cnt, sizeof(send_buff));
            }
            else
            {
                os_kprintf("<%d>wait tx ready timeout.\r\n", os_tick_get());
            }

            /* Exception */
            if (FD_ISSET(fd, &exceptfds))
            {
                os_kprintf("<%d>except occur.\r\n", os_tick_get());
            }
            else
            {
                os_kprintf("<%d>no except.\r\n", os_tick_get());
            }
        }

        /* last package */
        while (1)
        {
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            timeout.tv_sec  = 0;
            timeout.tv_usec = 50 * 1000;
            
            ret = select(fd + 1, &readfds, NULL, NULL, &timeout);
            
            if (!FD_ISSET(fd, &readfds))
                break;

            recv_cnt = read(fd, recv_buff, OS_SERIAL_RX_BUFSZ);
            recv_total_cnt += recv_cnt;
            os_kprintf("<%d>recv %d, total %d :\r\n", os_tick_get(), recv_cnt, recv_total_cnt);
            hex_dump(recv_buff, recv_cnt);
            rx_crc = crc16(rx_crc, recv_buff, recv_cnt);
        }
    }

    close(fd);

    os_free(recv_buff);

    os_kprintf("\r\n");
    os_kprintf("stat:\r\n");
    os_kprintf("    send size %d\r\n", sizeof(send_buff) * loops);
    os_kprintf("    recv size:%d\r\n", recv_total_cnt);
    os_kprintf("    %s tx_crc:%04x, rx_crc:%04x\r\n", (tx_crc == rx_crc) ? "success" : "failed", tx_crc, rx_crc);

    return 0;
}
SH_CMD_EXPORT(select_test, select_test, "select_test");
