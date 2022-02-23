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
 * @file        onetls_lib.h
 *
 * @brief       onetls_lib header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_LIB_H__
#define __ONETLS_LIB_H__
#include "onetls.h"
#include "onetls_util.h"
#include "onetls_handshake.h"
#include "onetls_crypto.h"

enum {
    ONETLS_EPOCH_FOR_CH_SH_HRR  = 0,
    ONETLS_EPOCH_FOR_EARLY_DATA = 1,
    ONETLS_EPOCH_FOR_HS_MSG     = 2,
    ONETLS_EPOCH_FOR_APP_DATA   = 3,
};

#define ONETLS_BUFFERED_FLIGHT_NUM 4
#define ONETLS_MAX_CID_LEN 256

typedef struct {
    uint32_t life_time;
    uint32_t age_add;
    uint32_t max_early_data;
    uint8_t  master_key[ONETLS_MAX_MD_LEN];
    uint16_t cipher_id;
    uint16_t ticket_len;
    uint8_t  ticket[0];
} onetls_ticket;

typedef union {
    uint8_t seqence_number[8];
} onetls_record_number;


struct st_onetls_ctx {
    uint8_t     mode;
    uint8_t     state;
    uint16_t    shutdown;

    uint32_t socket_timeout;   // 这个是socket的超时间

    // 握手管理结构体
    onetls_handshake *handshake;
    struct {
        uint8_t *client;
        uint32_t client_len;

        uint8_t *server;
        uint32_t server_len;
    } cid;

    struct {
        uint8_t  send_msg[ONETLS_MAX_RECORD_PACKET_LEN];
        uint8_t  send_msg_type;
        uint32_t send_msg_len;

        uint8_t  recv_msg[ONETLS_MAX_RECORD_PACKET_LEN];
        uint8_t  recv_msg_type;
        uint32_t recv_msg_len;
        uint32_t recv_msg_cursor;

        uint8_t  send_buffer[ONETLS_MAX_RECORD_PACKET_LEN];
        uint32_t send_buffer_cursor;
        uint32_t send_buffer_len;

        uint8_t  recv_buffer[ONETLS_MAX_RECORD_PACKET_LEN];
        uint32_t recv_buffer_cursor;
        uint32_t recv_buffer_len;
    };

    struct {
        int recv_fd;
        int send_fd;
    };

    struct {
        uint32_t hint_len;
        uint8_t *hint;
        onetls_psk_callback psk_cb;
        onetls_nst_callback nst_cb;
    };

    struct {
        const uint8_t *data;
        uint32_t data_len;
        uint32_t data_send_len;
    } early_data;

    struct {
        const onetls_cipher *cipher;
        uint8_t exporter_master_secret[ONETLS_MAX_MD_LEN];
        uint8_t resume_master_secret[ONETLS_MAX_MD_LEN];
        uint8_t master_secret[ONETLS_MAX_MD_LEN];
        uint8_t c_app_secret[ONETLS_MAX_MD_LEN];
        uint8_t s_app_secret[ONETLS_MAX_MD_LEN];

        psa_key_handle_t en_ctx;
        psa_key_handle_t de_ctx;

        // 序列号
        onetls_record_number wr_seq;
        uint8_t wr_iv[ONETLS_MAX_IV_LEN];

        onetls_record_number rd_seq;
        uint8_t rd_iv[ONETLS_MAX_IV_LEN];

    } security;

    struct {
        uint32_t age_add;
        uint32_t life_time;
        uint32_t max_early_data;
        const onetls_cipher *cipher;
        uint8_t master_key[ONETLS_MAX_MD_LEN];
        uint8_t *ticket;
        uint32_t ticket_len;
    } resumption;
};
uint32_t onetls_fflush_ex(onetls_ctx* ctx, uint8_t reset);
void onetls_shutdown_ex(onetls_ctx *ctx);
void onetls_reset_buffer(onetls_ctx *ctx, uint8_t sending);
void onetls_reset_msg_buffer(onetls_ctx *ctx, uint8_t sending);

uint32_t onetls_send_data_out(onetls_ctx *ctx, const uint8_t *out, uint32_t out_len, uint32_t *send_len);
uint32_t onetls_recv_data_in(onetls_ctx *ctx, uint8_t *in, uint32_t in_len, uint32_t *recv_len);
#endif

