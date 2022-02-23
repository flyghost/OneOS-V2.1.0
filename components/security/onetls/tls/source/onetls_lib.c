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
 * @file        onetls_lib.c
 *
 * @brief       onetls_lib functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "onetls_lib.h"
#include "onetls_alert.h"
#include "onetls_socket.h"
#include "onetls_hkdf.h"

const char *onetls_version(void)
{
    static const char *version = "onetls v1.0.1 built at "__DATE__" "__TIME__;
    return version;
}

onetls_ctx *onetls_new_ctx(uint8_t mode)
{
    onetls_ctx *ctx = (onetls_ctx *)onetls_malloc(sizeof(onetls_ctx));
    if (ctx == NULL) {
        onetls_check_errlog(ONETLS_MALLOC_FAIL, "onetls_malloc");
        return NULL;
    }
    memset(ctx, 0, sizeof(onetls_ctx));
    onetls_crypto_init();   // 内部会判断是否已经初始化
    ctx->mode = mode;
    ctx->state = ONETLS_STATE_INIT;
   
    ctx->recv_fd = -1;
    ctx->send_fd = -1;

    ctx->socket_timeout = ONETLS_TLS_SOCKET_POLL_DEFAULT_TIMEOUT;

   
    return ctx;
}

void onetls_del_ctx(onetls_ctx *ctx)
{
    onetls_shutdown(ctx);

    onetls_free(ctx->resumption.ticket);
    onetls_free(ctx->hint);
    onetls_free(ctx->cid.client);
    onetls_free(ctx->cid.server);

    onetls_crypto_delete_ctx(&(ctx->security.en_ctx));
    onetls_crypto_delete_ctx(&(ctx->security.de_ctx));
    
    onetls_free(ctx);
}

void onetls_shutdown(onetls_ctx *ctx)
{
    if (ctx->state != ONETLS_STATE_INIT) {
        onetls_notify_close(ctx);
    }
    ctx->shutdown = ONETLS_CLOSED;
    onetls_shutdown_ex(ctx);
}

void onetls_shutdown_ex(onetls_ctx *ctx)
{
    onetls_del_handshake(ctx);
    ctx->state = ONETLS_STATE_INIT;
    memset(&(ctx->early_data), 0, sizeof(ctx->early_data));
}

void onetls_set_socket(onetls_ctx *ctx, int recv_fd, int send_fd)
{
    ctx->recv_fd = recv_fd;
    ctx->send_fd = send_fd;    
}

void onetls_set_psk_callback(onetls_ctx *ctx, onetls_psk_callback cb)
{
    ctx->psk_cb = cb;
}

void onetls_set_nst_callback(onetls_ctx *ctx, onetls_nst_callback cb)
{
    ctx->nst_cb = cb;
}

uint32_t onetls_set_ticket_ex(onetls_ctx *ctx, onetls_ticket *ticket_info)
{
    ctx->resumption.cipher = onetls_get_cipher_byid(ticket_info->cipher_id);
    if (ctx->resumption.cipher == NULL) {
        onetls_check_errlog(ONETLS_CONFIG_INVALID_TICKET, "cipher[%d]", ticket_info->cipher_id);
        return ONETLS_CONFIG_INVALID_TICKET;
    }

    memcpy(ctx->resumption.master_key, ticket_info->master_key, ONETLS_MAX_MD_LEN);
    ctx->resumption.life_time = ticket_info->life_time;
    ctx->resumption.age_add = ticket_info->age_add;
    ctx->resumption.max_early_data = ticket_info->max_early_data;
    ctx->resumption.ticket_len = ticket_info->ticket_len;
    ctx->resumption.ticket = onetls_malloc(ticket_info->ticket_len);
    if (ctx->resumption.ticket == NULL) {
        onetls_check_errlog(ONETLS_CONFIG_INVALID_TICKET, "ticket_len[%d]", ticket_info->ticket_len);
        return ONETLS_CONFIG_INVALID_TICKET;
    }
    memcpy(ctx->resumption.ticket, ticket_info->ticket, ticket_info->ticket_len);
    return ONETLS_SUCCESS;
}

uint32_t onetls_set_ticket(onetls_ctx *ctx, const uint8_t *ticket, uint32_t ticket_len)
{
    onetls_ticket *ticket_info = (onetls_ticket*)ticket;
    if ((ticket_info == NULL) || ((ticket_info->ticket_len + sizeof(onetls_ticket)) != ticket_len)) {
        onetls_check_errlog(ONETLS_CONFIG_INVALID_TICKET, "onetls_set_ticket");
        return ONETLS_CONFIG_INVALID_TICKET;
    }
    
    uint32_t ret = onetls_set_ticket_ex(ctx, ticket_info);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_set_ticket_ex");
        onetls_free(ctx->resumption.ticket);
        memset(&(ctx->resumption), 0, sizeof(ctx->resumption));
        return ONETLS_CONFIG_INVALID_TICKET;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_set_outband_psk_hint(onetls_ctx *ctx, uint8_t *hint, uint32_t hint_len)
{
    if ((hint == NULL) || (hint_len == 0)) {
        return ONETLS_CONFIG_INVALID_PSK_HINT;
    }
    
    // 先释放之前的。
    if (ctx->hint != NULL) {
        onetls_free(ctx->hint);
        ctx->hint_len = 0;
    }
    
    ctx->hint = (uint8_t *)onetls_malloc(hint_len);
    if (ctx->hint == NULL) {
        return ONETLS_NULL_PTR;
    }
    memcpy(ctx->hint, hint, hint_len);
    ctx->hint_len = hint_len;
    return ONETLS_SUCCESS;
}

uint32_t onetls_send_early_data(onetls_ctx *ctx, const uint8_t *out, uint32_t out_len, uint32_t *send_len)
{
    if ((out_len > ctx->resumption.max_early_data) ||
        (ctx->resumption.ticket == NULL) ||
        (ctx->state >= ONETLS_STATE_RECV_SVR_FINISH)) {
        return ONETLS_HANDSHAKE_EARLY_DATA;
    }

    ctx->early_data.data = out;
    ctx->early_data.data_len = out_len;
    ctx->early_data.data_send_len = 0;

    uint32_t ret = onetls_connect(ctx);
    
    *send_len = ctx->early_data.data_send_len;
    
    ctx->early_data.data = NULL;
    ctx->early_data.data_len = 0;
    ctx->early_data.data_send_len = 0;
    return ret;
}

uint32_t onetls_connect(onetls_ctx *ctx)
{
    onetls_new_handshake(ctx);

    ctx->shutdown = 0;  // 不管之前是啥状态，再次连接。不能直接shutdown
    uint32_t ret = onetls_negoatiate(ctx);
    if (ret == ONETLS_SOCKET_TRYAGAIN) {
        return ret;
    }
    onetls_del_handshake(ctx);

    // 状态要进入初始状态
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_negoatiate failed!");
        ctx->state = ONETLS_STATE_INIT;
    }
    return ret;
}

void onetls_reset_buffer(onetls_ctx *ctx, uint8_t sending)
{
    if (sending) {
        memset(ctx->send_buffer, 0, ctx->send_buffer_len);
        ctx->send_buffer_cursor = 0;
        ctx->send_buffer_len = 0;
    } else {
        memset(ctx->recv_buffer, 0, ctx->recv_buffer_len);
        ctx->recv_buffer_len = 0;
        ctx->recv_buffer_cursor = 0;        
    }
}

void onetls_reset_msg_buffer(onetls_ctx *ctx, uint8_t sending)
{
    if (sending) {
        memset(ctx->send_msg, 0, ctx->send_msg_len);
        ctx->send_msg_len = 0;
        ctx->send_msg_type = 0;
    } else {
        memset(ctx->recv_msg, 0, ctx->recv_msg_len);
        ctx->recv_msg_len = 0;
        ctx->recv_msg_type = 0;
        ctx->recv_msg_cursor = 0;
    }
}

uint32_t onetls_socket_timeout(onetls_ctx* ctx, uint32_t ms)
{
    ctx->socket_timeout = ms;
    return ONETLS_SUCCESS;
}

uint32_t onetls_key_update(onetls_ctx* ctx, uint8_t k)
{
    if (ctx->state != ONETLS_STATE_OK) {
        return ONETLS_SYS_INVALID_STATE;
    }

    uint32_t ret = onetls_send_key_update(ctx, k);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_fflush(onetls_ctx* ctx)
{
    uint32_t ret = ONETLS_SUCCESS;
    uint32_t send_len = 0;
    while (ctx->send_buffer_len > ctx->send_buffer_cursor) {
        ret = onetls_sock_send(ctx->send_fd, 
                               ctx->send_buffer + ctx->send_buffer_cursor,
                               ctx->send_buffer_len - ctx->send_buffer_cursor,
                               &send_len,
                               ctx->socket_timeout);
        ctx->send_buffer_cursor += send_len;
        if ((ret != ONETLS_SOCKET_TRYAGAIN) && (ret != ONETLS_SUCCESS)) {
            onetls_check_errlog(ret, "onetls_sock_send");
            return ret;
        }
    }
    onetls_reset_buffer(ctx, 1);
    return ONETLS_SUCCESS;
}

uint32_t onetls_check_data_channel(onetls_ctx *ctx, const uint8_t *data, uint32_t data_len, uint32_t *result_len)
{
    *result_len = 0;
    if ((data == NULL) || (data_len == 0)) {
        return ONETLS_INVALID_PARA;
    }
    // 如果状态不正常了。禁止收发
    if (ctx->state != ONETLS_STATE_OK) {
        return ONETLS_SYS_INVALID_STATE;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_recv(onetls_ctx *ctx, uint8_t *in, uint32_t in_len, uint32_t *recv_len)
{
    uint32_t ret = onetls_check_data_channel(ctx, in, in_len, recv_len);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    }
    return onetls_recv_data_in(ctx, in, in_len, recv_len);
}

uint32_t onetls_send(onetls_ctx *ctx, const uint8_t *out, uint32_t out_len, uint32_t *send_len)
{
    uint32_t ret = onetls_check_data_channel(ctx, out, out_len, send_len);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    }
    return onetls_send_data_out(ctx, out, out_len, send_len);
}

uint32_t onetls_send_data_out(onetls_ctx *ctx, const uint8_t *out, uint32_t out_len, uint32_t *send_len)
{
    uint32_t ret = 0;
    uint32_t offset = 0;
    uint32_t length = 0;

    onetls_fflush(ctx);
    while (offset < out_len) {
        length = out_len - offset;

        onetls_reset_msg_buffer(ctx, 1);
        memcpy(ctx->send_msg, out + offset, length);
        ctx->send_msg_len = length;
        ctx->send_msg_type = ONETLS_MT_APPLICATION_DATA;
        ret = onetls_send_record(ctx);        
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_send_record");
            return ret;
        }
        offset += length;
        ret = onetls_fflush(ctx);
        if (ret != ONETLS_SUCCESS) {
            return ret;
        }
        *send_len += length;
    }
    return ONETLS_SUCCESS;    
}

uint32_t onetls_pending(onetls_ctx* ctx)
{
    if ((ctx->state < ONETLS_STATE_OK)) {
        uint32_t ret = onetls_sock_test(ctx->recv_fd, ONETLS_SOCK_RD, 0);
        return ret ? 0 : 1;
    }
    return ctx->recv_msg_len - ctx->recv_msg_cursor;
}

uint32_t onetls_recv_data_in(onetls_ctx *ctx, uint8_t *in, uint32_t in_len, uint32_t *recv_len)
{
    uint32_t ret = 0;
    while (ctx->recv_msg_len <= ctx->recv_msg_cursor) {
        if (ctx->shutdown) {
            return ctx->shutdown;
        }
        /*
            先看缓冲区有没有数据
            没有数据就收一包上来
            如果是握手数据就对外返回重试
            如果是其他数据就报错
        */

        ret = onetls_recv_record(ctx);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_recv_record");
            onetls_reset_msg_buffer(ctx, 0);
            return ret;
        }

        if (ctx->recv_msg_type == ONETLS_MT_HANDSHAKE) {
            ret = onetls_deal_post_handshake(ctx);
            onetls_reset_msg_buffer(ctx, 0);
            if (ret != ONETLS_SUCCESS) {
                onetls_check_errlog(ret, "onetls_deal_post_handshake");
                return ret;
            }
            return ONETLS_SOCKET_TRYAGAIN;
        }

        if (ctx->recv_msg_type != ONETLS_MT_APPLICATION_DATA) {
            onetls_reset_msg_buffer(ctx, 0);
            return ONETLS_SYS_UNEXPECTED_MESSAGE;
        }
    }

    *recv_len = onetls_min(ctx->recv_msg_len - ctx->recv_msg_cursor, in_len);
    memcpy(in, ctx->recv_msg + ctx->recv_msg_cursor, *recv_len);
    
    // 偏移
    ctx->recv_msg_cursor += *recv_len;

    return ret;    
}

uint32_t onetls_get_export_key(onetls_ctx *ctx, const char *label, const uint8_t *in, uint32_t in_len, uint8_t *out, uint32_t out_size, uint32_t *out_len)
{
    uint32_t md_size = 0;
    uint32_t hash_id = 0;
    uint32_t ret = 0;
    uint8_t hash[ONETLS_MAX_MD_LEN] = { 0 };
    uint8_t export_secret[ONETLS_MAX_MD_LEN] = { 0 };

    if (ctx->state != ONETLS_STATE_OK) {
        return ONETLS_SYS_INVALID_STATE;
    }

    hash_id = ctx->security.cipher->hash_alg;  
    md_size = onetls_hash_size(hash_id);
    if ((out == NULL) || (out_size < md_size)) {
        return ONETLS_INVALID_PARA;
    }

    onetls_hash(hash_id, NULL, 0, hash, &ret);
    ret = onetls_hkdf_expand_label(hash_id,
                                   ctx->security.exporter_master_secret, md_size, 
                                   ONETLS_LABEL_EXP_MASTER_KEY, strlen(ONETLS_LABEL_EXP_MASTER_KEY), 
                                   hash, md_size, 
                                   export_secret, md_size);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_hkdf_expand_label");
        return ret;
    }

    *out_len = md_size;
    onetls_hash(hash_id, in, in_len, hash, &ret);
    ret = onetls_hkdf_expand_label(hash_id,
                                   ctx->security.exporter_master_secret, md_size, 
                                   label, strlen(label), 
                                   hash, md_size, 
                                   out, *out_len);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_hkdf_expand_label");
        return ret;
    }    
    return ret;
}
