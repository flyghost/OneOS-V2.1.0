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
 * @file        wiresharkdump.h
 *
 * @brief       Dump data to pc and save them to a file what wireshark can read.
 *
 * @details
 *
 * @revision
 * Date          Author          Notes
 * 2021-06-07    OneOS Team      First Version
 ***********************************************************************************************************************
 */
#ifndef __WIRESHARK_DUMP_H__
#define __WIRESHARK_DUMP_H__

#include <device.h>

typedef enum wsk_dump_dir
{
    wsk_dump_dir_o = 0,
    wsk_dump_dir_i = 1
} wsk_dump_dir_t;

struct wsk_ret;
typedef int (*func_gen_data_t)(void *paras, struct wsk_ret *rets);

typedef struct wsk_bt_hci_paras
{
    char type;
    char *pkt;
    int len;
} wsk_bt_hci_paras_t;

typedef struct wsk_eth_paras
{
    char *pkt;
    int len;
} wsk_eth_paras_t;

typedef struct wsk_ret
{
    unsigned char *buf;
    unsigned int tol_len;
} wsk_ret_t;

void wsk_dump_init(os_device_t *dev);

int wsk_bt_hci_hexdump(void *paras_p, wsk_ret_t *rets);

int wsk_eth_hexdump(void *paras_p, wsk_ret_t *rets);

int wsk_hexdump(wsk_dump_dir_t dir, func_gen_data_t func, void *paras);

int wsk_frame_tx_get_rest_len(unsigned int drv_fifo_len);

void wsk_frame_tx_next_slice(unsigned int drv_fifo_len);

void wsk_frame_tx_done(void);

int wsk_dump_sta_check(void);

#endif
