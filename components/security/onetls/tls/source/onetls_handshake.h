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
 * @file        onetls_handshake.h
 *
 * @brief       onetls_handshake header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_HANDSHAKE_H__
#define __ONETLS_HANDSHAKE_H__
#include "onetls.h"
#include "onetls_crypto.h"

// 报文类型
#define ONETLS_MT_CCS              0x14
#define ONETLS_MT_ALERT            0x15
#define ONETLS_MT_HANDSHAKE        0x16
#define ONETLS_MT_APPLICATION_DATA 0x17

/* From RFC 8446: https://tools.ietf.org/html/rfc8446#appendix-B.3 */
#define ONETLS_HELLO_REQUEST              0
#define ONETLS_CLIENT_HELLO               1
#define ONETLS_SERVER_HELLO               2
#define ONETLS_SERVER_NEW_SESSION_TICKET  4
#define ONETLS_CLIENT_END_OF_EARLY_DATA   5
#define ONETLS_ENCRYPTED_EXTENSIONS       8
#define ONETLS_CERTIFICATE               11
#define ONETLS_SERVER_KEY                12
#define ONETLS_CERT_REQ                  13
#define ONETLS_SERVER_HELLO_DONE         14
#define ONETLS_CERT_VERIFY               15
#define ONETLS_CLIENT_KEY                16
#define ONETLS_FINISHED                  20
#define ONETLS_SERVER_CERT_STATUS        22
#define ONETLS_SERVER_SESSION_LOOKUP     23
#define ONETLS_KEY_UPDATE                24
#define ONETLS_MESSAGE_HASH              254

/* RFC 5246 7.4.1.2 */
#define ONETLS_SESSION_ID_MAX_LEN 32

// TLS的版本状态，当前不支持配置，因为只支持最高的TLS 1.3
#define ONETLS_VERSION_TLS_12     0x0303
#define ONETLS_VERSION_TLS_13     0x0304

#define ONETLS_MAX_PSK_LEN 256

typedef uint32_t(*onetls_state_func)(onetls_ctx*);

// TLS状态机
enum en_onetls_state {
    ONETLS_STATE_INIT = 0,
    ONETLS_STATE_SEND_CNT_HELLO,
    ONETLS_STATE_SEND_EARLY_DATA,
    ONETLS_STATE_RECV_SVR_HELLO,
    ONETLS_STATE_RECV_SVR_EE,
    ONETLS_STATE_RECV_SVR_FINISH,
    ONETLS_STATE_SEND_EOE,
    ONETLS_STATE_SEND_CNT_FINISH,
    ONETLS_STATE_PRE_OK,
    ONETLS_STATE_OK,
};

typedef struct {
    uint8_t in_use;
    uint32_t length;
    uint8_t *data;
} onetls_flight;

typedef struct {
    uint8_t random_c[32];
    uint8_t random_s[32];

    uint16_t c_message_seq;
    uint16_t s_message_seq;

    uint8_t *cookie;
    uint16_t cookie_len;

    uint8_t session_id_len;
    uint8_t *session_id;

    uint8_t handshake_secret[ONETLS_MAX_MD_LEN];
    uint8_t c_handshake_secret[ONETLS_MAX_MD_LEN];
    uint8_t s_handshake_secret[ONETLS_MAX_MD_LEN];
    uint8_t c_finish_key[ONETLS_MAX_MD_LEN];
    uint8_t s_finish_key[ONETLS_MAX_MD_LEN];

    struct {
        uint8_t hrr:1;
        uint8_t early_data_ee:1;
    } msg_tag;

    struct {    // 计算摘要时，做的一些缓存，后续可能用得到
        psa_hash_operation_t hash_ctx;
        uint8_t ch[ONETLS_MAX_MD_LEN];
        uint8_t ch_to_sh[ONETLS_MAX_MD_LEN];
        uint8_t ch_to_sf[ONETLS_MAX_MD_LEN];
        uint8_t ch_to_cf[ONETLS_MAX_MD_LEN];
    } digest;

    struct {    // dh参数相关
        uint32_t server_public_key_len;
        uint8_t *server_public;

        uint16_t server_selected_id; // 服务的选择的id

        uint8_t              key_share_num;
        onetls_key_share    *key_share_all;

        // 协商完之后，最终的dh共享参数
        uint32_t dh_key_len;
        uint8_t *dh_key;
    } key_share;

    struct {    // 存放各种secret信息，各个长度，由算法决定
        uint8_t server_selected_psk;
        uint8_t early_secret[2][ONETLS_MAX_MD_LEN];
        uint32_t early_secret_hash[2];
        uint8_t early_traffic_secret[ONETLS_MAX_MD_LEN];
        const char *label[2];
        uint16_t binder_len;
        uint16_t identity_len;
    };
} onetls_handshake;

void onetls_new_handshake(onetls_ctx *ctx);
void onetls_del_handshake(onetls_ctx *ctx);
uint32_t onetls_negoatiate(onetls_ctx *ctx);
uint32_t onetls_init_state(onetls_ctx *ctx);
uint32_t onetls_send_client_hello(onetls_ctx *ctx);
uint32_t onetls_send_early_data_process(onetls_ctx *ctx);
uint32_t onetls_recv_server_hello(onetls_ctx *ctx);
uint32_t onetls_recv_server_ee(onetls_ctx *ctx);
uint32_t onetls_recv_server_finish(onetls_ctx *ctx);
uint32_t onetls_send_client_eoe(onetls_ctx *ctx);
uint32_t onetls_send_client_finish(onetls_ctx *ctx);
uint32_t onetls_pre_ok(onetls_ctx *ctx);
uint32_t onetls_deal_post_handshake(onetls_ctx *ctx);
uint32_t onetls_send_key_update(onetls_ctx* ctx, uint8_t k);
uint32_t onetls_recv_key_update(onetls_ctx* ctx, uint8_t *packet, uint32_t len);

uint32_t onetls_get_handshake_digest(onetls_ctx *ctx, uint8_t *out);
void onetls_release_handshake_key_share(onetls_ctx *ctx);
#endif
