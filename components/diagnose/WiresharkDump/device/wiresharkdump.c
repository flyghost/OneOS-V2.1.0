/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
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
 * @file        wiresharkdump.c
 *
 * @brief       Dump data to pc and save them to a file that wireshark can read
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2021-06-07    OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <drv_cfg.h>
#include <os_clock.h>
#include <os_memory.h>
#include <arch_interrupt.h>
#include <os_mq.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <serial/serial.h>
#include "misc_evt.h"
#include "wiresharkdump.h"

// WSK_LOG_BUF_NUM should be 1<<n
#define WSK_LOG_BUF_NUM 0x20
#define WSK_LOG_BUF_NUM_MASK WSK_LOG_BUF_NUM - 1
struct wsk_log_stru
{
    os_device_t *dev;
    unsigned char *(buf_p[WSK_LOG_BUF_NUM]);
    unsigned short buf_len[WSK_LOG_BUF_NUM];
    unsigned short buf_tx_len;
    unsigned char buf_w;
    unsigned char buf_r;
    unsigned char overflow;
};
static struct wsk_log_stru s_wsk_log_stru;

#define WSK_LOG_FIFO_FULL() (WSK_LOG_BUF_NUM_MASK == ((s_wsk_log_stru.buf_w + WSK_LOG_BUF_NUM - s_wsk_log_stru.buf_r) & WSK_LOG_BUF_NUM_MASK))
#define WSK_LOG_FIFO_EMPTY() (s_wsk_log_stru.buf_w == s_wsk_log_stru.buf_r)

static int wsk_evt_deal(os_uint32_t arg);

int wsk_bt_hci_hexdump(void *paras_p, wsk_ret_t *rets)
{
    wsk_bt_hci_paras_t *paras = (wsk_bt_hci_paras_t *)paras_p;

    char type = paras->type;
    char *pkt = paras->pkt;
    int len = paras->len;

    unsigned int tol_len = 5 + sizeof(os_tick_t) + (len + 1);
    unsigned char *buf = os_malloc(tol_len);
    OS_ASSERT(buf != OS_NULL);

    unsigned int pos = 4 + sizeof(os_tick_t);
    buf[pos] = type & 0x0F;

    memcpy(&buf[5 + sizeof(os_tick_t)], pkt, len);

    rets->buf = buf;
    rets->tol_len = tol_len;

    return 0;
}

int wsk_eth_hexdump(void *paras_p, wsk_ret_t *rets)
{
    wsk_eth_paras_t *paras = (wsk_eth_paras_t *)paras_p;

    char *pkt = paras->pkt;
    int len = paras->len;

    unsigned int tol_len = 5 + sizeof(os_tick_t) + len;
    unsigned char *buf = os_malloc(tol_len);
    OS_ASSERT(buf != OS_NULL);

    memcpy(&buf[4 + sizeof(os_tick_t)], pkt, len);

    rets->buf = buf;
    rets->tol_len = tol_len;

    return 0;
}

// dir:0-Out 1-In
int wsk_hexdump(wsk_dump_dir_t dir, func_gen_data_t func, void *paras)
{
    if (!s_wsk_log_stru.dev)
    {
        return 0;
    }

    os_tick_t time = (1000 / OS_TICK_PER_SECOND) * os_tick_get();

    wsk_ret_t rets;
    func(paras, &rets);

    unsigned char *buf = rets.buf;
    unsigned int tol_len = rets.tol_len;
    unsigned int frame_data_len = tol_len - 3;
    unsigned int pos;

    pos = 0;
    buf[pos++] = 0xAA;
    buf[pos++] = frame_data_len & 0xFF;
    buf[pos++] = (frame_data_len >> 8) & 0xFF;
    memcpy(&buf[pos], (char *)&time, sizeof(time));
    pos += sizeof(time);
    buf[pos] = dir;
    buf[tol_len - 1] = 0x55;

    os_base_t level = os_irq_lock();
    int empty = WSK_LOG_FIFO_EMPTY();

    int buf_w = s_wsk_log_stru.buf_w;
    s_wsk_log_stru.buf_w++;
    s_wsk_log_stru.buf_w &= WSK_LOG_BUF_NUM_MASK;

    s_wsk_log_stru.overflow = s_wsk_log_stru.overflow || WSK_LOG_FIFO_EMPTY();

    s_wsk_log_stru.buf_p[buf_w] = buf;
    s_wsk_log_stru.buf_len[buf_w] = tol_len;
    os_irq_unlock(level);

    if (empty)
    {
        int buf_r = s_wsk_log_stru.buf_r;
        s_wsk_log_stru.buf_tx_len = s_wsk_log_stru.buf_len[buf_r];
        os_device_write_nonblock(s_wsk_log_stru.dev, 0, s_wsk_log_stru.buf_p[buf_r], s_wsk_log_stru.buf_tx_len);
    }
    return 0;
}

int wsk_frame_tx_get_rest_len(unsigned int drv_fifo_len)
{
    return s_wsk_log_stru.buf_tx_len - drv_fifo_len;
}

void wsk_frame_tx_next_slice(unsigned int drv_fifo_len)
{
    s_wsk_log_stru.buf_tx_len -= drv_fifo_len;
    int buf_tx_len = s_wsk_log_stru.buf_tx_len;
    int buf_r = s_wsk_log_stru.buf_r;
    int buf_pos = s_wsk_log_stru.buf_len[buf_r] - buf_tx_len;
    os_device_write_nonblock(s_wsk_log_stru.dev, 0, &s_wsk_log_stru.buf_p[buf_r][buf_pos], buf_tx_len);
}

void wsk_frame_tx_done(void)
{
    misc_evt_t send_evt;
    send_evt.handle = wsk_evt_deal;
    send_evt.arg = (unsigned int)(s_wsk_log_stru.buf_p[s_wsk_log_stru.buf_r]);
    if (OS_EFULL == os_mq_send(misc_evt_mq_get(), &send_evt, sizeof(misc_evt_t), OS_NO_WAIT))
    {
        OS_ASSERT(0);
    }

    // os_base_t level = os_irq_lock();
    s_wsk_log_stru.buf_r++;
    s_wsk_log_stru.buf_r &= WSK_LOG_BUF_NUM_MASK;
    // os_irq_unlock(level);

    int buf_r = s_wsk_log_stru.buf_r;
    s_wsk_log_stru.buf_tx_len = s_wsk_log_stru.buf_len[buf_r];
    if (!WSK_LOG_FIFO_EMPTY())
    {
        os_device_write_nonblock(s_wsk_log_stru.dev, 0, s_wsk_log_stru.buf_p[buf_r], s_wsk_log_stru.buf_len[buf_r]);
    }
}

int wsk_dump_sta_check(void)
{
    if (s_wsk_log_stru.overflow)
    {
        s_wsk_log_stru.overflow = 0;
        return 1;
    }
    return 0;
}

static int wsk_evt_deal(os_uint32_t arg)
{
    unsigned int *buf_r_p = (unsigned int *)arg;
    os_free(buf_r_p);
    return 0;
}

void wsk_dump_init(os_device_t *dev)
{
    s_wsk_log_stru.dev = dev;
}
