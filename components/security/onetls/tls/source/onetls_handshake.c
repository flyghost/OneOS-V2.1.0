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
 * @file        onetls_handshake.c
 *
 * @brief       onetls_handshake functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "onetls_handshake.h"
#include "onetls_lib.h"
#include "onetls_util.h"
#include "onetls_hkdf.h"
#include "onetls_socket.h"
#include "onetls_extension.h"
#include "onetls_alert.h"

static onetls_state_func g_onetls_state_machine[ONETLS_STATE_OK] = {
    // 准备活动主要是校验、获取PSK等检查操作
    [ONETLS_STATE_INIT] =                       onetls_init_state,
    // 发送client_hello
    [ONETLS_STATE_SEND_CNT_HELLO] =             onetls_send_client_hello,
    // 发送early_data
    [ONETLS_STATE_SEND_EARLY_DATA] =            onetls_send_early_data_process,
    // 收到server_hello
    [ONETLS_STATE_RECV_SVR_HELLO] =             onetls_recv_server_hello,
    // 收到server的ee报文
    [ONETLS_STATE_RECV_SVR_EE] =                onetls_recv_server_ee,
    // 收到server的finish报文
    [ONETLS_STATE_RECV_SVR_FINISH] =            onetls_recv_server_finish,    
    // 发送client的end of early_dta
    [ONETLS_STATE_SEND_EOE] =                   onetls_send_client_eoe,   
    // 发送client报文
    [ONETLS_STATE_SEND_CNT_FINISH] =            onetls_send_client_finish,
    // 握手完成，准备清理中间信息
    [ONETLS_STATE_PRE_OK] =                     onetls_pre_ok,
};

void onetls_new_handshake(onetls_ctx *ctx)
{
    onetls_handshake *handshake = ctx->handshake;
    if (handshake != NULL) {
        return;
    }
    handshake = (onetls_handshake *)onetls_malloc(sizeof(onetls_handshake));
    if (handshake == NULL) {
        onetls_check_errlog(ONETLS_NULL_PTR, "onetls_malloc");
        return;
    }
    memset(handshake, 0, sizeof(onetls_handshake));

    psa_hash_setup(&(handshake->digest.hash_ctx), onetls_tls13_default_hkdf_hash());
    ctx->handshake = handshake;
}

void onetls_release_handshake_key_share(onetls_ctx *ctx)
{
    onetls_handshake *handshake = ctx->handshake;
    if (handshake->key_share.key_share_all != NULL) {
        uint8_t loop = 0;
        for (loop = 0; loop < handshake->key_share.key_share_num; loop++) {
            onetls_del_key_share(&((handshake->key_share.key_share_all)[loop]));
        }

        onetls_free(handshake->key_share.key_share_all);
        handshake->key_share.key_share_all = NULL;
    }

    onetls_free(handshake->key_share.server_public);
    handshake->key_share.server_public = NULL;
}

void onetls_del_handshake(onetls_ctx *ctx)
{
    onetls_handshake *handshake = ctx->handshake;
    if (handshake == NULL) {
        return;
    }

    onetls_release_handshake_key_share(ctx);
    onetls_free(handshake->key_share.dh_key);

    onetls_free(handshake->cookie);
    onetls_free(handshake->session_id);
    
    psa_hash_abort(&handshake->digest.hash_ctx);

    memset(handshake, 0, sizeof(onetls_handshake)); 
    onetls_free(handshake);
    ctx->handshake = NULL;
}

uint32_t onetls_negoatiate(onetls_ctx *ctx)
{
    uint32_t ret = ONETLS_SUCCESS;
    while (ctx->state < ONETLS_STATE_OK) {
        // 如果中间那里出错了，且不可恢复，就直接shutdown
        if (ctx->shutdown > 0) {
            onetls_shutdown_ex(ctx);
            return ctx->shutdown;
        }

        if (g_onetls_state_machine[ctx->state] == NULL) {
            onetls_check_errlog(ONETLS_INNER_SYSERR, "g_onetls_state_machine %d", ctx->state);
            return ONETLS_INNER_SYSERR;
        }

        // 进入状态机
        ret = g_onetls_state_machine[ctx->state](ctx);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_ctx_negotiate[%d]", ctx->state);
            break;
        }
    }
    return ret;
}

uint32_t onetls_get_handshake_digest(onetls_ctx *ctx, uint8_t *out)
{
    size_t hash_size = ONETLS_MAX_MD_LEN;
    uint32_t ret = 0;
    psa_hash_operation_t tmp = PSA_HASH_OPERATION_INIT;
    ret = psa_hash_clone( &(ctx->handshake->digest.hash_ctx), &tmp);
    if (ret != ONETLS_SUCCESS) {
        psa_hash_abort(&tmp);
        return ret;
    }

    ret = psa_hash_finish(&tmp, out, hash_size, &hash_size);
    if (ret != ONETLS_SUCCESS) {
        psa_hash_abort(&tmp);
        return ret;
    }
    psa_hash_abort(&tmp);
    return ret;
}

uint32_t onetls_gen_early_secret(onetls_ctx *ctx)
{
    uint32_t hash_id = 0;
    uint32_t md_size = 0;
    uint32_t ret = 0;
    uint8_t offset = 0;    
    ctx->handshake->binder_len = 0;
    ctx->handshake->identity_len = 0;
    if (ctx->resumption.ticket != NULL) {
        hash_id = ctx->resumption.cipher->hash_alg;
        md_size = onetls_hash_size(hash_id);
        ret = onetls_hkdf_extract(ctx->resumption.cipher->hash_alg, 
                                  NULL, 0, 
                                  ctx->resumption.master_key, md_size, 
                                  ctx->handshake->early_secret[offset], md_size);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_hkdf_extract");
            return ret;
        }

        onetls_log_dump("early res secret", ctx->handshake->early_secret[offset], md_size);

        ctx->handshake->label[offset] = ONETLS_LABEL_RES_BINDER;
        ctx->handshake->early_secret_hash[offset] = ctx->resumption.cipher->hash_alg;
        ctx->handshake->binder_len += md_size + 1;
        ctx->handshake->identity_len += ctx->resumption.ticket_len + 6;
        offset ++;
    }

    if ((ctx->hint != NULL) && (ctx->psk_cb != NULL)) {
        uint8_t psk_key[ONETLS_MAX_PSK_LEN] = { 0 };
        uint32_t psk_key_len = 0;
        
        ret = ctx->psk_cb(ctx->hint, ctx->hint_len, psk_key, sizeof(psk_key), &psk_key_len);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ONETLS_HANDSHAKE_GET_PSK_KEY, "get psk key failed!");
            return ret;
        }

        hash_id = onetls_tls13_default_hkdf_hash();
        md_size = onetls_hash_size(hash_id);
        ret = onetls_hkdf_extract(hash_id, 
                                  NULL, 0, 
                                  psk_key, psk_key_len, 
                                  ctx->handshake->early_secret[offset], md_size);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_hkdf_extract");
            return ret;
        }

        onetls_log_dump("early ext secret", ctx->handshake->early_secret[offset], md_size);

        memset(psk_key, 0, psk_key_len);
        ctx->handshake->label[offset] = ONETLS_LABEL_EXT_BINDER;
        ctx->handshake->early_secret_hash[offset] = hash_id;
        ctx->handshake->binder_len += md_size + 1;
        ctx->handshake->identity_len += ctx->hint_len + 6;
        offset ++;
    }
    
    if (offset == 0) {
        return ONETLS_HANDSHAKE_NEED_A_PSK;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_gen_early_traffic_secret(onetls_ctx *ctx)
{
    uint8_t key[ONETLS_MAX_MD_LEN] = { 0 };
    uint8_t *in_sec = ctx->handshake->early_secret[0];  // 必须使用第一个。
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_derive_secret_key_iv_finish_sn(hash_id, 
                                                         ONETLS_LABEL_EARLY_TRAFFIC, 
                                                         ctx->handshake->digest.ch, md_size,
                                                         in_sec, md_size,
                                                         ctx->handshake->early_traffic_secret, md_size,
                                                         key, ctx->security.cipher->key_len,
                                                         ctx->security.wr_iv, ctx->security.cipher->iv_len,
                                                         NULL, 0,
                                                         NULL, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "c e t");
        return ret;
    }

    onetls_log_dump("early traffic secret", ctx->handshake->early_traffic_secret, md_size);

    memset(ctx->security.wr_seq.seqence_number, 0, 8);
    return onetls_crypto_create_ctx(&(ctx->security.en_ctx), ctx->security.cipher, key, 1);
}

uint32_t onetls_gen_handshake_secret(onetls_ctx *ctx)
{
    uint8_t pre_secret[ONETLS_MAX_MD_LEN] = { 0 };
    uint32_t md_size = onetls_hash_size(ctx->security.cipher->hash_alg);
    uint32_t ret = 0;

    ret = onetls_hkdf_derive_secret(ctx->security.cipher->hash_alg,
                                    ONETLS_LABEL_DERIVED, strlen(ONETLS_LABEL_DERIVED),
                                    ctx->handshake->early_secret[ctx->handshake->server_selected_psk], md_size,
                                    NULL, 0,
                                    pre_secret, md_size);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_hkdf_derive_secret handshake");
        return ret;
    }   
    ret = onetls_hkdf_extract(ctx->security.cipher->hash_alg, 
                              pre_secret, md_size, 
                              ctx->handshake->key_share.dh_key, ctx->handshake->key_share.dh_key_len, 
                              ctx->handshake->handshake_secret, md_size);
    onetls_check_errlog(ret, "onetls_hkdf_extract handshake");

    onetls_log_dump("handshake secret", ctx->handshake->handshake_secret, md_size);
    return ret;
}

uint32_t onetls_gen_s_hs_traffic_secret(onetls_ctx *ctx)
{
    uint8_t key[ONETLS_MAX_MD_LEN] = { 0 };
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_derive_secret_key_iv_finish_sn(hash_id,
                                                         ONETLS_LABEL_SERVER_HS_TRAFFIC, 
                                                         ctx->handshake->digest.ch_to_sh, md_size,
                                                         ctx->handshake->handshake_secret, md_size,
                                                         ctx->handshake->s_handshake_secret, md_size,
                                                         key, ctx->security.cipher->key_len,
                                                         ctx->security.rd_iv, ctx->security.cipher->iv_len,
                                                         ctx->handshake->s_finish_key, md_size,
                                                         NULL, 0);



    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_traffic_secret_key_iv c hs");
        return ret;
    }

    onetls_log_dump("handshake s traffic secret", ctx->handshake->s_handshake_secret, md_size);
    onetls_log_dump("handshake s traffic key", key, ctx->security.cipher->key_len);
    onetls_log_dump("handshake s traffic iv", ctx->security.rd_iv, ctx->security.cipher->iv_len);

    memset(ctx->security.rd_seq.seqence_number, 0, 8);
    return onetls_crypto_create_ctx(&(ctx->security.de_ctx), ctx->security.cipher, key, 0);
}

uint32_t onetls_gen_c_hs_traffic_secret(onetls_ctx *ctx)
{
    uint8_t key[ONETLS_MAX_MD_LEN] = { 0 };
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_derive_secret_key_iv_finish_sn(hash_id,
                                                         ONETLS_LABEL_CLIENT_HS_TRAFFIC, 
                                                         ctx->handshake->digest.ch_to_sh, md_size,
                                                         ctx->handshake->handshake_secret, md_size,
                                                         ctx->handshake->c_handshake_secret, md_size,
                                                         key, ctx->security.cipher->key_len,
                                                         ctx->security.wr_iv, ctx->security.cipher->iv_len,
                                                         ctx->handshake->c_finish_key, md_size,
                                                         NULL, 0);
                                                       
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_traffic_secret_key_iv c hs");
        return ret;
    }
    memset(ctx->security.wr_seq.seqence_number, 0, 8);
    return onetls_crypto_create_ctx(&(ctx->security.en_ctx), ctx->security.cipher, key, 1);
}

uint32_t onetls_gen_master_secret(onetls_ctx *ctx)
{
    uint8_t pre_secret[ONETLS_MAX_MD_LEN] = { 0 };
    uint32_t md_size = onetls_hash_size(ctx->security.cipher->hash_alg);
    uint32_t ret = 0;

    ret = onetls_hkdf_derive_secret(ctx->security.cipher->hash_alg,
                                    ONETLS_LABEL_DERIVED, strlen(ONETLS_LABEL_DERIVED),
                                    ctx->handshake->handshake_secret, md_size,
                                    NULL, 0,
                                    pre_secret, md_size);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_hkdf_derive_secret master");
        return ret;
    }   
    ret = onetls_hkdf_extract(ctx->security.cipher->hash_alg, 
                              pre_secret, md_size, 
                              NULL, 0, 
                              ctx->security.master_secret, md_size);
    onetls_check_errlog(ret, "onetls_hkdf_extract master");
    return ret;
}

uint32_t onetls_gen_c_app_traffic_secret(onetls_ctx *ctx)
{
    uint8_t key[ONETLS_MAX_MD_LEN] = { 0 };
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_derive_secret_key_iv_finish_sn(hash_id,
                                                         ONETLS_LABEL_CLIENT_AP_TRAFFIC, 
                                                         ctx->handshake->digest.ch_to_sf, md_size,
                                                         ctx->security.master_secret, md_size,
                                                         ctx->security.c_app_secret, md_size,
                                                         key, ctx->security.cipher->key_len,
                                                         ctx->security.wr_iv, ctx->security.cipher->iv_len,
                                                         NULL, 0,
                                                         NULL, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_traffic_secret_key_iv c hs");
        return ret;
    }
    memset(ctx->security.wr_seq.seqence_number, 0, 8);
    return onetls_crypto_create_ctx(&(ctx->security.en_ctx), ctx->security.cipher, key, 1);
}

uint32_t onetls_gen_s_app_traffic_secret(onetls_ctx *ctx)
{
    uint8_t key[ONETLS_MAX_MD_LEN] = { 0 };
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_derive_secret_key_iv_finish_sn(hash_id,
                                                         ONETLS_LABEL_SERVER_AP_TRAFFIC, 
                                                         ctx->handshake->digest.ch_to_sf, md_size,
                                                         ctx->security.master_secret, md_size,
                                                         ctx->security.s_app_secret, md_size,
                                                         key, ctx->security.cipher->key_len,
                                                         ctx->security.rd_iv, ctx->security.cipher->iv_len,
                                                         NULL, 0,
                                                         NULL, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_traffic_secret_key_iv c hs");
        return ret;
    }
    memset(ctx->security.rd_seq.seqence_number, 0, 8);
    return onetls_crypto_create_ctx(&(ctx->security.de_ctx), ctx->security.cipher, key, 0);
}

uint32_t onetls_gen_res_master_secret(onetls_ctx *ctx)
{
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_derive_secret_key_iv_finish_sn(hash_id,
                                                         ONETLS_LABEL_RES_MASTER, 
                                                         ctx->handshake->digest.ch_to_cf, md_size,
                                                         ctx->security.master_secret, md_size,
                                                         ctx->security.resume_master_secret, md_size,
                                                         NULL, 0,
                                                         NULL, 0,
                                                         NULL, 0,
                                                         NULL, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_res_master_secret");
        return ret;
    }
    onetls_log_dump("resump_master_secret", ctx->security.resume_master_secret, md_size);
    return ONETLS_SUCCESS;
} 

uint32_t onetls_gen_exp_master_secret(onetls_ctx *ctx)
{
    uint32_t hash_id = ctx->security.cipher->hash_alg;    
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_derive_secret_key_iv_finish_sn(hash_id,
                                                         ONETLS_LABEL_EXP_MASTER, 
                                                         ctx->handshake->digest.ch_to_cf, md_size,
                                                         ctx->security.master_secret, md_size,
                                                         ctx->security.exporter_master_secret, md_size,
                                                         NULL, 0,
                                                         NULL, 0,
                                                         NULL, 0,
                                                         NULL, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_exp_master_secret");
        return ret;
    }
    onetls_log_dump("export_master_secret", ctx->security.exporter_master_secret, md_size);
    return ONETLS_SUCCESS;
} 

uint32_t onetls_gen_res_master_key(onetls_ctx* ctx, uint8_t *res_data, uint8_t *nonce, uint8_t nonce_len)
{
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = onetls_hkdf_expand_label(hash_id,
                                            ctx->security.resume_master_secret, md_size, 
                                            ONETLS_LABEL_RES_MASTER_KEY, strlen(ONETLS_LABEL_RES_MASTER_KEY), 
                                            nonce, nonce_len, 
                                            res_data, md_size);
    onetls_check_errlog(ret, "onetls_gen_res_master_key");
    return ret;
}

uint32_t onetls_update_traffic_key(onetls_ctx *ctx, uint8_t sending)
{
    uint32_t hash_id = ctx->security.cipher->hash_alg;
    uint32_t md_size = onetls_hash_size(hash_id);
    uint32_t ret = 0;
    uint8_t *iv = NULL;
    uint8_t *secret_n = NULL;

    onetls_record_number *seq = NULL;    
    uint8_t out[ONETLS_MAX_MD_LEN] = { 0 };
    uint8_t key[ONETLS_MAX_MD_LEN] = { 0 };
    psa_key_handle_t crypto_ctx = 0;
    
    if (sending) {
        iv = ctx->security.wr_iv;
        secret_n = ctx->security.c_app_secret;
        crypto_ctx = ctx->security.en_ctx;
        seq = &(ctx->security.wr_seq);  
    } else {
        iv = ctx->security.rd_iv;
        secret_n = ctx->security.s_app_secret;
        crypto_ctx = ctx->security.de_ctx;
        seq = &(ctx->security.rd_seq);
    }

    ret = onetls_derive_secret_key_iv_finish_sn(hash_id,
                                                ONETLS_LABEL_TRAFFIC_UPDATE,
                                                NULL, 0,
                                                secret_n, md_size,
                                                out, md_size,
                                                key, ctx->security.cipher->key_len,
                                                iv, ctx->security.cipher->iv_len,
                                                NULL, 0,
                                                NULL, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_derive_secret_key_iv_finish_sn");
        return ret;
    }
    memset(seq->seqence_number, 0, 8);
    memcpy(secret_n, out, ONETLS_MAX_MD_LEN);
    return onetls_crypto_create_ctx(&crypto_ctx, ctx->security.cipher, key, sending);
}

void onetls_client_hello_random(uint8_t *random_c)
{
    uint32_t t = onetls_time();
    
    // 高位是时间
    onetls_put_u32(random_c, t);
    onetls_get_rnd(random_c + 4, 28);
}

uint32_t onetls_init_state(onetls_ctx *ctx)
{
    uint32_t ret = 0;
    memset(&(ctx->security), 0, sizeof(ctx->security));

    ret = onetls_gen_early_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_early_secret");
        return ret;
    }
    ret = onetls_new_keyshare(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_new_keyshare");
        return ret;
    }
    memset(&(ctx->handshake->msg_tag), 0, sizeof(ctx->handshake->msg_tag));
    onetls_client_hello_random(ctx->handshake->random_c);
    onetls_reset_buffer(ctx, 1);
    onetls_reset_buffer(ctx, 0);
    ctx->state = ONETLS_STATE_SEND_CNT_HELLO;
    return ONETLS_SUCCESS;
}

uint32_t onetls_construct_client_hello_binder(onetls_ctx *ctx)
{
    uint32_t msg_len = 0;
    uint32_t binder_len = 0;
    uint8_t *binder_start = NULL;
    uint8_t  offset = 0;

    if (ctx->handshake->binder_len == 0) {
        return ONETLS_SUCCESS;  // 没有binder标识不适用psk，可能是证书
    }

    msg_len = ctx->send_msg_len - ctx->handshake->binder_len - 2;
    binder_start = ctx->send_msg + msg_len;
    binder_start += onetls_put_u16(binder_start, ctx->handshake->binder_len);

    for (offset = 0; offset < 2; offset++) {
        uint8_t binder_key[ONETLS_MAX_MD_LEN] = { 0 };
        uint8_t finish_key[ONETLS_MAX_MD_LEN] = { 0 };
        uint8_t hash[ONETLS_MAX_MD_LEN] = { 0 };
        uint32_t md_size = 0;
        uint32_t hash_id = ctx->handshake->early_secret_hash[offset];
        if (onetls_check_data_empty(ctx->handshake->early_secret[offset], ONETLS_MAX_MD_LEN)) {
            break;
        }

        md_size = onetls_hash_size(hash_id);

        onetls_hkdf_derive_secret(hash_id,
                                  ctx->handshake->label[offset], 
                                  strlen(ctx->handshake->label[offset]), 
                                  ctx->handshake->early_secret[offset], md_size, 
                                  NULL, 0, binder_key, md_size);

        onetls_hkdf_expand_label(hash_id, 
                                 binder_key, md_size, 
                                 ONETLS_LABEL_FINISHED, strlen(ONETLS_LABEL_FINISHED), 
                                 NULL, 0, finish_key, md_size);

        // 计算消息的摘要
        binder_start += onetls_put_u8(binder_start, md_size);

        binder_len = md_size;
        onetls_hash(hash_id, ctx->send_msg, msg_len, hash, &binder_len);
        onetls_hmac(hash_id, finish_key, md_size, hash, md_size, binder_start, &binder_len);
        binder_start += md_size;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_construct_client_hello(onetls_ctx *ctx)
{
    uint32_t ret = 0;
    uint32_t length = 0;
    uint16_t version = ONETLS_VERSION_TLS_12;
    uint8_t *packet = ctx->send_msg;
    uint8_t *end = ctx->send_buffer + ONETLS_MAX_RECORD_PACKET_LEN;

    packet += onetls_put_u8(packet, ONETLS_CLIENT_HELLO);
    packet += onetls_put_u24(packet, 0);    // 待反写
    

    packet += onetls_put_u16(packet, version);
    packet += onetls_put_data(packet, ctx->handshake->random_c, 32);

    // 暂时不支持session id的恢复
    packet += onetls_put_u8(packet, 0);

    onetls_construct_client_hello_attr(ONETLS_ATTR_CIPHER, packet, &length);
    packet += length;
    packet += onetls_put_u16(packet, 0x0100);

    // 添加扩展
    ret = onetls_construct_client_hello_extension(ctx, packet, end - packet, &length);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_construct_hello_ext");
        return ret;
    }
    packet += length;
    ctx->send_msg_len = packet - ctx->send_msg;
    length = ctx->send_msg_len - 4;

    onetls_put_u24(ctx->send_msg + 1, length);

    // 长度反写之后，计算binder
    ret = onetls_construct_client_hello_binder(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_construct_hello_binder");
        return ret;
    }    
    return ONETLS_SUCCESS;
}

uint32_t onetls_send_client_hello(onetls_ctx *ctx)
{
    uint32_t ret = 0;
    ctx->handshake->c_message_seq = 0;
    ctx->handshake->s_message_seq = 0;
    memset(&(ctx->security.wr_seq), 0, sizeof(ctx->security.wr_seq));
    memset(&(ctx->security.rd_seq), 0, sizeof(ctx->security.rd_seq));

    onetls_reset_msg_buffer(ctx, 1);
    ret = onetls_construct_client_hello(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_construct_client_hello");
        return ret;
    }    
    ret = onetls_send_handshake_message(ctx, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_send_handshake_message");
        return ret;
    } 

    onetls_get_handshake_digest(ctx, ctx->handshake->digest.ch);
    ctx->state = ONETLS_STATE_SEND_EARLY_DATA;
    ctx->handshake->c_message_seq ++;
    return ONETLS_SUCCESS;
}

uint32_t onetls_check_handshake_message_type(onetls_ctx *ctx, uint8_t type, uint8_t **payload, uint32_t *payload_len) 
{
    uint8_t *p = ctx->recv_msg;
    uint8_t type_tmp = 0;
    uint32_t length = 0;
    uint32_t payload_len_tmp = 0;

    p += onetls_get_u8(p, &type_tmp);
    if (type_tmp != type) {
        return ONETLS_HANDSHAKE_BAD_RECORD;
    }    

    p += onetls_get_u24(p, &length);

    payload_len_tmp = ctx->recv_msg_len - 4;

    if (payload_len_tmp != length) {
        return ONETLS_HANDSHAKE_BAD_RECORD;
    }
    *payload = p;
    *payload_len = payload_len_tmp;
    return ONETLS_SUCCESS;
}

uint32_t onetls_send_early_data_process(onetls_ctx *ctx)
{
    uint32_t ret = 0;
    if (ctx->early_data.data != NULL) {
        // 使用会话恢复的算法
        ctx->security.cipher = ctx->resumption.cipher;

        ret = onetls_gen_early_traffic_secret(ctx);
        if (ret != ONETLS_SUCCESS) {
            onetls_check_errlog(ret, "onetls_gen_early_traffic_secret");
            return ret;
        }

        ret = onetls_send_data_out(ctx, ctx->early_data.data, ctx->early_data.data_len, &(ctx->early_data.data_send_len));
        if (ret != ONETLS_SUCCESS) {
            return ret;
        }        
    }   
    onetls_fflush(ctx);
    ctx->state = ONETLS_STATE_RECV_SVR_HELLO;
    return ONETLS_SUCCESS;
}

uint32_t onetls_pasrse_server_hello_random(onetls_ctx *ctx)
{
    uint8_t hdr[4] = { 0 };
    uint8_t hello_retry_req[32] = {
        0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 
        0xBE, 0x1D, 0x8C, 0x02, 0x1E, 0x65, 0xB8, 0x91,
        0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB, 0x8C, 0x5E, 
        0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C,
    };

    // TODO: 判断retry
    if (memcmp(hello_retry_req, ctx->handshake->random_s, 32) == 0) {
        if (ctx->handshake->msg_tag.hrr == 1) {
            onetls_check_errlog(ONETLS_HANDSHAKE_BAD_RECORD, "repeat hrr!");
            onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
            return ONETLS_HANDSHAKE_BAD_RECORD;
        }
        psa_hash_abort(&(ctx->handshake->digest.hash_ctx));
        memset(&(ctx->handshake->digest.hash_ctx), 0, sizeof(ctx->handshake->digest.hash_ctx));
        psa_hash_setup(&(ctx->handshake->digest.hash_ctx), ctx->security.cipher->hash_alg);

        hdr[0] = ONETLS_MESSAGE_HASH;
        hdr[3] = onetls_hash_size(ctx->security.cipher->hash_alg);

        psa_hash_update(&(ctx->handshake->digest.hash_ctx), hdr, 4);
        psa_hash_update(&(ctx->handshake->digest.hash_ctx), ctx->handshake->digest.ch, onetls_hash_size(ctx->security.cipher->hash_alg));
        ctx->handshake->msg_tag.hrr = 1;
        return ONETLS_SUCCESS;    
    }
    ctx->handshake->msg_tag.hrr = 0;
    return ONETLS_SUCCESS;
}

uint32_t onetls_parse_server_hello_head(onetls_ctx *ctx, uint8_t *packet, uint32_t packet_len, uint32_t *len)
{
    uint8_t *p = packet;
    uint32_t length = 0;
    uint32_t ret = 0;
    uint16_t cipher_id = 0;
    uint16_t version = ONETLS_VERSION_TLS_12;
    uint16_t version_t = 0;

    ret = onetls_check_handshake_message_type(ctx, ONETLS_SERVER_HELLO, &p, &length);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_check_handshake_message_type");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ret;
    }

    p += onetls_get_u16(p, &version_t);
    if (version_t < version) {
        onetls_check_errlog(ONETLS_HANDSHAKE_BAD_RECORD, "version");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_PROTOCOL_VERSION);
        return ONETLS_HANDSHAKE_BAD_RECORD;
    }

    p += onetls_get_data(p, ctx->handshake->random_s, 32);
    p += onetls_get_u8(p, &(ctx->handshake->session_id_len));
    if (ctx->handshake->session_id_len > 0) {
        // 当前没有存放session id, 暂时不支持session id
        p += ctx->handshake->session_id_len;
    }

    p += onetls_get_u16(p, &cipher_id);
    ctx->security.cipher = onetls_get_cipher_byid(cipher_id);
    if (ctx->security.cipher == NULL) {
        onetls_check_errlog(ONETLS_HANDSHAKE_BAD_RECORD, "cipher");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ONETLS_HANDSHAKE_NO_CIPHER_MATCH;
    }
    p += 1;     // 压缩算法不支持，不解析了。

    ret = onetls_pasrse_server_hello_random(ctx);

    *len = p - packet;
    return ret;
}

uint32_t onetls_parse_server_hello(onetls_ctx *ctx)
{
    uint8_t *packet = ctx->recv_msg;
    uint32_t packet_len = ctx->recv_msg_len;
    uint32_t len = 0;
    uint32_t ret = onetls_parse_server_hello_head(ctx, packet, packet_len, &len);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_parse_server_hello_head");
        return ret;
    }

    packet += len;
    packet_len -= len;

    ret = onetls_parse_server_extension(ctx, ONETLS_SERVER_HELLO, packet, packet_len);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_parse_server_extension");
        return ret;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_recv_server_hello(onetls_ctx *ctx)
{
    uint32_t ret = onetls_recv_record_type(ctx, ONETLS_MT_HANDSHAKE);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_recv_record_type server hello");
        return ret;
    }

    ret = onetls_parse_server_hello(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_parse_server_hello");
        return ret;
    }   

    psa_hash_update(&(ctx->handshake->digest.hash_ctx), ctx->recv_msg, ctx->recv_msg_len);
    onetls_get_handshake_digest(ctx, ctx->handshake->digest.ch_to_sh);
    if (ctx->handshake->msg_tag.hrr) {
        ctx->state = ONETLS_STATE_SEND_CNT_HELLO;
        return ONETLS_SUCCESS;
    }

    ret = onetls_gen_handshake_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_handshake_traffic_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }

    ret = onetls_gen_s_hs_traffic_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_s_hs_traffic_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }

    ctx->state = ONETLS_STATE_RECV_SVR_EE;
    ctx->handshake->s_message_seq ++;
    return ret;
}

uint32_t onetls_recv_server_ee(onetls_ctx *ctx)
{
    uint8_t *p = NULL;
    uint32_t length = 0;
    uint32_t ret = onetls_recv_record_type(ctx, ONETLS_MT_HANDSHAKE);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    }

    ret = onetls_check_handshake_message_type(ctx, ONETLS_ENCRYPTED_EXTENSIONS, &p, &length);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    }

    ret = onetls_parse_server_extension(ctx, ONETLS_ENCRYPTED_EXTENSIONS, p, length);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_parse_server_extension");
        return ret;
    }

    psa_hash_update(&(ctx->handshake->digest.hash_ctx), ctx->recv_msg, ctx->recv_msg_len);

    ctx->state = ONETLS_STATE_RECV_SVR_FINISH;
    ctx->handshake->s_message_seq ++;
    return ret;
}

uint32_t onetls_recv_server_finish(onetls_ctx *ctx)
{
    uint8_t *p = NULL;
    uint32_t ret = 0;
    uint32_t length = 0;
    uint32_t md_size = onetls_hash_size(ctx->security.cipher->hash_alg);
    uint8_t ch_to_before_sf[ONETLS_MAX_MD_LEN] = { 0 };
    uint8_t hash[ONETLS_MAX_MD_LEN] = { 0 };

    ret = onetls_recv_record_type(ctx, ONETLS_MT_HANDSHAKE);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    }

    ret = onetls_check_handshake_message_type(ctx, ONETLS_FINISHED, &p, &length);
    if (ret != ONETLS_SUCCESS) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }

    if (length != md_size) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_DECODE_ERROR);
        return ONETLS_SOCKET_BAD_RECORD;
    }

    onetls_get_handshake_digest(ctx, ch_to_before_sf);
    onetls_hmac(ctx->security.cipher->hash_alg, 
                ctx->handshake->s_finish_key, md_size, ch_to_before_sf, md_size, hash, &md_size); 

    if (memcmp(p, hash, md_size) != 0) {
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ONETLS_SOCKET_BAD_RECORD;
    }

    psa_hash_update(&(ctx->handshake->digest.hash_ctx), ctx->recv_msg, ctx->recv_msg_len);
    onetls_get_handshake_digest(ctx, ctx->handshake->digest.ch_to_sf);

    ctx->state = ONETLS_STATE_SEND_EOE;
    ctx->handshake->s_message_seq ++;
    return ONETLS_SUCCESS;
}

uint32_t onetls_send_client_eoe(onetls_ctx *ctx)
{
    uint32_t ret = 0;
    if (ctx->handshake->msg_tag.early_data_ee) {
        uint32_t send_len = 0;
        uint8_t eoe[4] = { ONETLS_CLIENT_END_OF_EARLY_DATA, 0 };
        ret = onetls_send_data_out(ctx, eoe, 4, &send_len);
        if (ret != ONETLS_SUCCESS) {
            return ret;
        }
    }
    ret = onetls_gen_c_hs_traffic_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_c_hs_traffic_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }
    ctx->state = ONETLS_STATE_SEND_CNT_FINISH;
    return ONETLS_SUCCESS;
}

uint32_t onetls_send_client_finish(onetls_ctx *ctx)
{
    uint8_t *packet = ctx->send_msg;
    uint32_t ret = 0;
    uint32_t md_size = onetls_hash_size(ctx->security.cipher->hash_alg);
    uint8_t ch_to_before_cf[ONETLS_MAX_MD_LEN] = { 0 };
    uint8_t hash[ONETLS_MAX_MD_LEN] = { 0 };

    onetls_reset_msg_buffer(ctx, 1);

    onetls_get_handshake_digest(ctx, ch_to_before_cf);
    onetls_hmac(ctx->security.cipher->hash_alg, 
                ctx->handshake->c_finish_key, md_size, ch_to_before_cf, md_size, hash, &md_size); 


    packet += onetls_put_u8(packet, ONETLS_FINISHED);
    packet += onetls_put_u24(packet, md_size);

    packet += onetls_put_data(packet, hash, md_size);

    ctx->send_msg_len = packet - ctx->send_msg;

    ret = onetls_send_handshake_message(ctx, 0);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_send_handshake_message");
        return ret;
    } 

    onetls_fflush(ctx);
    onetls_get_handshake_digest(ctx, ctx->handshake->digest.ch_to_cf);
    ctx->state = ONETLS_STATE_PRE_OK;
    ctx->handshake->c_message_seq ++;
    return ONETLS_SUCCESS;
}

uint32_t onetls_pre_ok(onetls_ctx *ctx)
{
    uint32_t ret = onetls_gen_master_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_master_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }
    
    ret = onetls_gen_s_app_traffic_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_s_app_traffic_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }

    ret = onetls_gen_res_master_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_res_master_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }

    ret = onetls_gen_exp_master_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_exp_master_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }

    ret = onetls_gen_c_app_traffic_secret(ctx);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "onetls_gen_c_app_traffic_secret");
        onetls_send_alert(ctx, ONETLS_ALERT_LEVEL_FATAL, ONETLS_ALERT_DESCRIPTION_HANDSHAKE_FAILURE);
        return ret;
    }

    onetls_reset_msg_buffer(ctx, 0);
    ctx->state = ONETLS_STATE_OK;
    return ONETLS_SUCCESS;    
}

uint32_t onetls_send_key_update(onetls_ctx* ctx, uint8_t k)
{
    uint32_t ret = 0;
    uint8_t *pkt = ctx->send_msg;

    onetls_reset_msg_buffer(ctx, 1);

    pkt += onetls_put_u8(pkt, ONETLS_KEY_UPDATE);
    pkt += onetls_put_u24(pkt, 1);
    pkt += onetls_put_u8(pkt, k);
    ctx->send_msg_len = pkt - ctx->send_msg;

    ret = onetls_send_handshake_message(ctx, 0);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    } 

    ret = onetls_update_traffic_key(ctx, 1);
    onetls_check_errlog(ret, "onetls_update_traffic_key");
    onetls_fflush(ctx);
    return ret;
}

uint32_t onetls_recv_key_update(onetls_ctx* ctx, uint8_t *packet, uint32_t len)
{
    uint32_t ret = 0;
    uint8_t k = packet[0];
    if (len != 1) {
        return ONETLS_SOCKET_BAD_RECORD_LEN;
    }

    ret = onetls_update_traffic_key(ctx, 0);
    onetls_check_errlog(ret, "onetls_update_traffic_key");

    if (k) {
        ret = onetls_send_key_update(ctx, 0);
        onetls_check_errlog(ret, "onetls_update_traffic_key");
    }
    return ret;
}

uint32_t onetls_recv_nst(onetls_ctx* ctx, uint8_t *packet, uint32_t len)
{
    uint8_t *p = packet;
    onetls_ticket *ticket = NULL;
    uint32_t ret = 0;
    uint32_t lifetime = 0;
    uint32_t age_add = 0;
    uint16_t ticket_len = 0;
    uint8_t nonce_len = 0;
    uint8_t nonce[255] = { 0 };
    
    if (ctx->nst_cb == NULL) {
        return ONETLS_SUCCESS;
    }

    if (len < 9) {
        return ONETLS_SOCKET_BAD_RECORD;
    }

    p += onetls_get_u32(p, &lifetime);
    p += onetls_get_u32(p, &age_add);
    p += onetls_get_u8(p, &nonce_len);
    if (nonce_len + 5 > len) {
        return ONETLS_SOCKET_BAD_RECORD;
    }

    p += onetls_get_data(p, nonce, nonce_len);
    p += onetls_get_u16(p, &ticket_len);
    if (nonce_len + ticket_len + 5 > len) {
        return ONETLS_SOCKET_BAD_RECORD;
    }    

    ticket = onetls_malloc(sizeof(onetls_ticket) + ticket_len);
    if (ticket == NULL) {
        return ONETLS_MALLOC_FAIL;
    }

    p += onetls_get_data(p, ticket->ticket, ticket_len);
    ticket->ticket_len = ticket_len;
    ticket->life_time = lifetime;
    ticket->age_add = age_add;
    ticket->cipher_id = ctx->security.cipher->iana_id;

    ret = onetls_gen_res_master_key(ctx, ticket->master_key, nonce, nonce_len);
    if (ret != ONETLS_SUCCESS) {
        onetls_free(ticket);    
        return ret;
    }

    ret = onetls_parse_server_extension(ctx, ONETLS_SERVER_NEW_SESSION_TICKET, p, packet + len - p);
    if (ret != ONETLS_SUCCESS) {
        onetls_free(ticket);    
        return ret;
    }  
    ticket->max_early_data = ctx->resumption.max_early_data;

    ret = ctx->nst_cb(ctx, (uint8_t*)ticket, sizeof(onetls_ticket) + ticket_len);
    onetls_check_errlog(ret, "nst cb");
    onetls_free(ticket);

    return ret;
}

uint32_t onetls_deal_post_handshake(onetls_ctx *ctx)
{
    uint8_t mt = ctx->recv_msg[0];
    uint8_t *p = NULL;
    uint32_t length = 0;
    uint32_t ret = onetls_check_handshake_message_type(ctx, mt, &p, &length);
    if (ret != ONETLS_SUCCESS) {
        return ret;
    } 

    switch (mt) {
    case ONETLS_SERVER_NEW_SESSION_TICKET:   
        ret = onetls_recv_nst(ctx, p, length);
        onetls_check_errlog(ret, "onetls_recv_nst");
        break;
    case ONETLS_KEY_UPDATE:
        ret = onetls_recv_key_update(ctx, p, length);
        onetls_check_errlog(ret, "onetls_recv_key_update");
        break;
    default:    // 其他的消息都不支持
        ret = ONETLS_SYS_UNEXPECTED_MESSAGE;
        break;
    }
    return ret;
}
