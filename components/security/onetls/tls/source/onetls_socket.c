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
 * @file        onetls_socket.c
 *
 * @brief       onetls_socket functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/errno.h>

#include "onetls_socket.h"
#include "onetls_lib.h"
#include "onetls_alert.h"

uint32_t onetls_sock_test(int fd, uint8_t op, uint32_t timeout)
{ 
    fd_set fds;    
    if (fd >= sizeof(fds) * 8) {    // 主要是面向物联网板子的。socket都很小，select的普适性高一点，所以没有切换到poll/epoll（不一定有这个接口）
        onetls_check_errlog(ONETLS_FAIL, "select fail! suggest using API:poll/epoll");
        return ONETLS_FAIL;
    }

    FD_ZERO(&fds);   
    FD_SET(fd, &fds);  

    struct timeval tv;
    tv.tv_sec = timeout / 1000;  
    tv.tv_usec = timeout % 1000;

    int ret = select((fd + 1), 
                     (op == ONETLS_SOCK_RD) ? &fds : NULL, 
                     (op == ONETLS_SOCK_WR) ? &fds : NULL, 
                     NULL, &tv);
    if (ret < 0) {
        onetls_check_errlog(ret, "select");
        return ONETLS_SOCKET_FAIL;
    }
    if (ret == 0) {
        return ONETLS_SOCKET_TIMEOUT;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_sock_recv(int fd, uint8_t *buf, uint32_t len, uint32_t *recv_len, uint32_t time_out)
{  
    *recv_len = 0;
    while (*recv_len < len) {
        int ret = onetls_sock_test(fd, ONETLS_SOCK_RD, time_out);
        if (ret) {
            return ret;
        }

        // 调用底层的接收接口
        ret = recv(fd, buf + *recv_len, len - *recv_len, 0);
        if (ret < 0) {
            if (onetls_sys_errno() == EINTR) {
                continue;
            }
            if (onetls_sys_errno() == EAGAIN) {
                return ONETLS_SOCKET_TRYAGAIN;
            }
            onetls_check_errlog(ret, "recv");
            return ONETLS_SOCKET_RECV_FAIL;
        }
        *recv_len += ret;
  
        if (ret == 0) { // 关闭了socket
            return ONETLS_SOCKET_CLOSED;
        }
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_sock_send(int fd, uint8_t *buf, uint32_t len, uint32_t *send_len, uint32_t time_out)
{
    *send_len = 0;
    while (*send_len < len) {
        int ret = onetls_sock_test(fd, ONETLS_SOCK_WR, time_out);
        if (ret) {
            return ret;
        }

        // 调用底层的发送接口
        ret = send(fd, buf + *send_len, len - *send_len, 0);
        if (ret < 0) {
            if (onetls_sys_errno() == EINTR) {
                continue;
            }
            if (onetls_sys_errno() == EAGAIN) {
                return ONETLS_SOCKET_TRYAGAIN;
            }
            onetls_check_errlog(ret, "send");
            return ONETLS_SOCKET_SEND_FAIL;
        }
        *send_len += ret;

        if (ret == 0) { // 关闭了socket
            return ONETLS_SOCKET_CLOSED;
        }
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_send_record_not_encrypt(onetls_ctx *ctx)
{
    uint16_t hdr_len = 0;
    uint16_t version = ONETLS_VERSION_TLS_12;
    uint8_t *hdr = ctx->send_buffer + ctx->send_buffer_len;
    uint8_t *p = hdr;

    p += onetls_put_u8(p, ctx->send_msg_type);
    p += onetls_put_u16(p, version);
    p += onetls_put_u16(p, ctx->send_msg_len);
    hdr_len = p - hdr;

    memcpy(p, ctx->send_msg, ctx->send_msg_len);
    ctx->send_buffer_len += hdr_len + ctx->send_msg_len;

    onetls_seq_num_add(ctx, 1);
    return ONETLS_SUCCESS;    
}

uint32_t onetls_send_record(onetls_ctx *ctx)
{
    uint32_t out_len = ONETLS_MAX_RECORD_PACKET_LEN - ctx->send_buffer_len;
    uint32_t hdr_len = 0;
    uint16_t cipher_len = ctx->send_msg_len + ctx->security.cipher->tag_len + 1;    
    uint8_t nonce[ONETLS_MAX_IV_LEN] = { 0 };
    uint8_t offset = 0;
    uint8_t *seq = ctx->security.wr_seq.seqence_number;
    uint8_t *hdr = ctx->send_buffer + ctx->send_buffer_len;
    uint8_t *p = hdr;

    memcpy(nonce, ctx->security.wr_iv, ctx->security.cipher->iv_len); // 静态iv
    for (offset = 0; offset < 8; offset ++) {         // 生成nonce
        nonce[offset + ctx->security.cipher->iv_len - 8] ^= seq[offset];
    }
    {
        p += onetls_put_u8(p, ONETLS_MT_APPLICATION_DATA);
        p += onetls_put_u16(p, ONETLS_VERSION_TLS_12);
        p += onetls_put_u16(p, cipher_len);
    }
    hdr_len = p - ctx->send_buffer;
    uint32_t ret = onetls_aead_encrypt(ctx, nonce, // nonce，长度由算法决定
                                       hdr, hdr_len,
                                       ctx->send_msg, ctx->send_msg_len, &ctx->send_msg_type,
                                       p, &out_len);
    if ((ret != ONETLS_SUCCESS) && (out_len != cipher_len)) {
        onetls_check_errlog(ret, "onetls_aead_encrypt");
        return ret;
    }
    onetls_seq_num_add(ctx, 1);

    ctx->send_buffer_len += hdr_len + out_len;
    return ONETLS_SUCCESS;
}

uint32_t onetls_recv_makeup_seq_aad(onetls_ctx *ctx, uint8_t *seq, uint8_t *hdr, uint32_t *hdr_len, uint32_t *cipher_len)
{
    uint32_t record_max_len = ctx->recv_buffer_len - ctx->recv_buffer_cursor;
    uint16_t cipher_len_tmp = 0;
    uint16_t hdr_len_tmp = 0;

    hdr_len_tmp = 5;
    onetls_get_u16(hdr + 3, &cipher_len_tmp);

    if ((cipher_len_tmp + hdr_len_tmp) > record_max_len) {
        return ONETLS_SOCKET_BAD_RECORD_LEN;
    }

    *hdr_len = hdr_len_tmp;
    *cipher_len = cipher_len_tmp;
    return ONETLS_SUCCESS;
}

uint32_t onetls_recv_decode(onetls_ctx *ctx, uint32_t *record_len)
{
    uint32_t hdr_len = 0;
    uint32_t cipher_len = 0;
    uint32_t plain_len = ONETLS_MAX_RECORD_PACKET_LEN;
    uint32_t ret = 0;
    uint8_t nonce[ONETLS_MAX_IV_LEN] = { 0 };
    uint8_t loop = 0;
    uint8_t offset_iv = ctx->security.cipher->iv_len - 8;
    uint8_t *hdr = ctx->recv_buffer + ctx->recv_buffer_cursor;
    onetls_record_number seq =  ctx->security.rd_seq;

    ret = onetls_recv_makeup_seq_aad(ctx, seq.seqence_number, hdr, &hdr_len, &cipher_len);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_recv_makeup_seq_aad");
        return ret;
    } 
    *record_len = hdr_len + cipher_len;

    memcpy(nonce, ctx->security.rd_iv, ctx->security.cipher->iv_len);    // 静态iv
    for (loop = 0; loop < 8; loop ++) {
        nonce[offset_iv + loop] ^= seq.seqence_number[loop];
    }

    ret = onetls_aead_decrypt(ctx, nonce, // nonce，长度由算法决定
                              hdr, hdr_len,
                              hdr + hdr_len, cipher_len, &ctx->recv_msg_type,
                              ctx->recv_msg, &plain_len);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_aead_decrypt");

        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }
    ctx->recv_msg_len = plain_len;
    ctx->security.rd_seq = seq;
    onetls_seq_num_add(ctx, 0);
    return ONETLS_SUCCESS;
}

uint32_t onetls_recv_record_from_buffer(onetls_ctx *ctx)
{
    uint32_t record_len = ctx->recv_buffer_len - ctx->recv_buffer_cursor;
    uint8_t *record = ctx->recv_buffer + ctx->recv_buffer_cursor;
    uint8_t type = record[0];
    uint16_t length = 0;
    uint16_t hdr_len = 5;
 
    if (hdr_len >= record_len) {
        return ONETLS_SYS_UNEXPECTED_MESSAGE;
    }

    if ((type & 0x20) || (type == ONETLS_MT_APPLICATION_DATA)) { // 加密报文
        uint32_t ret = 0;
        if (record_len < (ctx->security.cipher->tag_len + hdr_len + ctx->cid.server_len + 1)) {   // tag_len + hdr_len
            return ONETLS_SOCKET_BAD_RECORD_LEN;
        }

        ret = onetls_recv_decode(ctx, &record_len);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_recv_decode");
            return ONETLS_SYS_UNEXPECTED_MESSAGE;
        }
        ctx->recv_buffer_cursor += record_len;
    } else {
        uint32_t r_len = 0;
        onetls_get_u16(record + hdr_len - 2, &length);

        r_len = length + hdr_len;
        if (r_len > record_len) {
            return ONETLS_SYS_UNEXPECTED_MESSAGE;
        }

        memcpy(ctx->recv_msg, record + hdr_len, length);
        ctx->recv_msg_len = length;
        ctx->recv_msg_type = type;
        ctx->recv_buffer_cursor += r_len;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_recv_record_from_socket(onetls_ctx *ctx)
{
    uint32_t ret = 0;
    uint32_t recv_len = 0;

    uint16_t record_len = 0;
    uint8_t *buffer = ctx->recv_buffer + ctx->recv_buffer_len;

    // 收取一个header
    ret = onetls_sock_recv(ctx->recv_fd, buffer, 5, &recv_len, ctx->socket_timeout);
    ctx->recv_buffer_len += recv_len;
    if (ret != ONETLS_SUCCESS) {
        return ret;
    }

    onetls_get_u16(buffer + 3, &record_len);
    if ((record_len > ONETLS_MAX_RECORD_PACKET_LEN) || (record_len == 0)) {
        return ONETLS_SOCKET_BAD_RECORD_LEN;
    }

    ret = onetls_sock_recv(ctx->recv_fd, buffer + 5, record_len, &recv_len, ctx->socket_timeout);
    if (record_len != recv_len) {
        return ONETLS_SOCKET_BAD_RECORD_LEN;
    }    
    ctx->recv_buffer_len += recv_len;
    return ret;
}

uint32_t onetls_recv_handle_message_type(onetls_ctx *ctx)
{
    uint8_t type = ctx->recv_msg_type;
    if (type == ONETLS_MT_CCS) {
        onetls_reset_msg_buffer(ctx, 0);
        return ONETLS_SUCCESS;
    }

    if (type == ONETLS_MT_ALERT) {
        onetls_recv_alert(ctx, ctx->recv_msg[0], ctx->recv_msg[1]);
        onetls_reset_msg_buffer(ctx, 0);
        return ONETLS_SUCCESS;    
    }
return ONETLS_SUCCESS;
}


uint32_t onetls_recv_record(onetls_ctx *ctx)
{
    uint32_t ret = 0;
    onetls_reset_msg_buffer(ctx, 0);

    while ((ctx->recv_msg_len == 0) && (!ctx->shutdown)) {

        // 先查看网路缓冲区
        // 再查看flight缓冲区
        // 都没有就从网络中收取一个报文        
        if (ctx->recv_buffer_cursor < ctx->recv_buffer_len) {            
            ret = onetls_recv_record_from_buffer(ctx);
            if (ret != ONETLS_SUCCESS) {
                onetls_reset_buffer(ctx, 0);
                return ret;
            }

            ret = onetls_recv_handle_message_type(ctx);
            if (ret != ONETLS_SUCCESS) {
                onetls_reset_buffer(ctx, 0);
                return ret;
            }            
        }

        // 上面两轮过去了。还是没有合适的报文，从网络中收取
        if (ctx->recv_msg_len == 0) {
            ret = onetls_recv_record_from_socket(ctx); 
            if (ret != ONETLS_SUCCESS) {
                onetls_reset_buffer(ctx, 0);    
                return ret;
            }            
        }
    }
    return (ctx->shutdown) ? (ctx->shutdown) : ret;
}

uint32_t onetls_send_handshake_message(onetls_ctx *ctx, uint8_t flight)
{
    ctx->send_msg_type = ONETLS_MT_HANDSHAKE;
    if (flight == 0) {
        // 需要加入到hash表单中去
        if ((ctx->state < ONETLS_STATE_OK) && (ctx->handshake != NULL)) {
            psa_hash_update(&(ctx->handshake->digest.hash_ctx), ctx->send_msg, ctx->send_msg_len);
        }
    }
    if (ctx->send_msg[0] == ONETLS_CLIENT_HELLO) {
        return onetls_send_record_not_encrypt(ctx);
    }
    return onetls_send_record(ctx);
}

uint32_t onetls_recv_record_type(onetls_ctx *ctx, uint8_t type)
{
    uint32_t ret = ONETLS_SUCCESS;
    while (ret == ONETLS_SUCCESS) {
        ret = onetls_recv_record(ctx);
        if (type == ctx->recv_msg_type) {
            break;
        }
    }
    return ret;
}
