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
 * @file        onetls_crypto.c
 *
 * @brief       onetls_crypto functions.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include "onetls_crypto.h"
#include "onetls_util.h"
#include "onetls_lib.h"

const static onetls_cipher g_onetls_ciphers[] = {
#if defined(ONETLS_CIPHER_AES_128_GCM_SHA256)
    { 0x1301, PSA_KEY_TYPE_AES,         PSA_ALG_SHA_256, PSA_ALG_GCM,           16, 12, 16},       // "TLS_AES_128_GCM_SHA256"
#endif
#if defined(ONETLS_CIPHER_AES_256_GCM_SHA384)
    { 0x1302, PSA_KEY_TYPE_AES,         PSA_ALG_SHA_384, PSA_ALG_GCM,           32, 12, 16},       // "TLS_AES_256_GCM_SHA384"
#endif
#if defined(ONETLS_CIPHER_CHACHA20_POLY1305_SHA256)
    { 0x1303, PSA_KEY_TYPE_CHACHA20,    PSA_ALG_SHA_256, PSA_ALG_CHACHA20_POLY1305, 32, 12, 16},       // "TLS_CHACHA20_POLY1305_SHA256"
#endif
#if defined(ONETLS_CIPHER_AES_128_CCM_SHA256)
    { 0x1304, PSA_KEY_TYPE_AES,         PSA_ALG_SHA_256, PSA_ALG_CCM,           16, 12, 16},       // "TLS_AES_128_CCM_SHA256"
#endif
#if defined(ONETLS_CIPHER_AES_128_CCM_8_SHA256)
    { 0x1305, PSA_KEY_TYPE_AES,         PSA_ALG_SHA_256, PSA_ALG_CCM,           16, 12, 8 },       // "TLS_AES_128_CCM_8_SHA256"
#endif
};
const static uint8_t g_onetls_cipher_num = sizeof(g_onetls_ciphers) / sizeof(onetls_cipher);

// 配置支持的group
static const onetls_dh_groups g_onetls_support_groups[] = {
#if defined(ONETLS_ECP_DP_CURVE25519_ENABLED)
    { 0x001D, 256, PSA_ALG_ECDH, PSA_ECC_CURVE_CURVE25519,   onetls_min(ONETLS_DH_PB_KEY_LEN, 32)},        // "x25519"
#endif
#if defined(ONETLS_ECP_DP_SECP256R1_ENABLED)
    { 0x0017, 256, PSA_ALG_ECDH, PSA_ECC_CURVE_SECP256R1,    onetls_min(ONETLS_DH_PB_KEY_LEN, 65)},        // "secp256r1"
#endif
#if defined(ONETLS_ECP_DP_SECP384R1_ENABLED)
    { 0x0018, 384, PSA_ALG_ECDH, PSA_ECC_CURVE_SECP384R1,    onetls_min(ONETLS_DH_PB_KEY_LEN, 97)},        // "secp384r1"
#endif
#if defined(ONETLS_ECP_DP_SECP521R1_ENABLED)
    { 0x0019, 521, PSA_ALG_ECDH, PSA_ECC_CURVE_SECP521R1,    onetls_min(ONETLS_DH_PB_KEY_LEN, 133)},       // "secp521r1"
#endif
};
const static uint8_t g_onetls_dh_groups_num = sizeof(g_onetls_support_groups) / sizeof(onetls_dh_groups);

// 配置签名算法
static const onetls_sign_alg g_onetls_sign_algs[] = {
    // { 0x0401 },		// "rsa_pkcs1_sha256"
    { 0x0403 },     // "ecdsa_secp256r1_sha256"
    // { 0x0501 },     // "rsa_pkcs1_sha384"
    // { 0x0503 },     // "ecdsa_secp384r1_sha384"
    // { 0x0601 },     // "rsa_pkcs1_sha512"
    // { 0x0603 },     // "ecdsa_secp521r1_sha512"
    // { 0x0804 },     // "rsa_pss_rsae_sha256" 
    // { 0x0805 },     // "rsa_pss_rsae_sha384"
    // { 0x0806 },     // "rsa_pss_rsae_sha512"
    // { 0x0807 },     // "ed25519"    
    // { 0x0808 },     // "ed448"    
};
const static uint8_t g_onetls_sign_algs_num = sizeof(g_onetls_sign_algs) / sizeof(onetls_sign_alg);

const onetls_cipher *onetls_get_cipher_byid(uint16_t id)
{
    for (uint8_t i = 0; i < g_onetls_cipher_num; i++) {
        if (g_onetls_ciphers[i].iana_id == id) {
            return &(g_onetls_ciphers[i]);
        }
    }
    return NULL;
}

void onetls_crypto_init()
{
    // 随机数
    srand(onetls_time());

    // 初始化库
    uint32_t ret = psa_crypto_init();
    onetls_check_errlog(ret, "psa_crypto_init fail!");
}

int onetls_psa_entropy_stub(void *data, uint8_t *output, size_t len, size_t *out_len)
{ 
    *out_len = 0;
    while ((*out_len) < len) {
        // 修改psa的熵源, 用伪随机
        output[(*out_len)++] = rand() & 0xff;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_tls13_default_hkdf_hash()
{
    return PSA_ALG_SHA_256;
}

void onetls_get_rnd(uint8_t *data, uint32_t len)
{
    uint32_t ret = psa_generate_random(data, len);
    onetls_check_errlog(ret, "psa_generate_random");
}

uint32_t onetls_import_psa_key(psa_key_handle_t *handle, 
                               const uint8_t *key, 
                               uint32_t key_len, 
                               uint16_t key_type, 
                               uint32_t key_usage, 
                               uint32_t key_alg)
{
    psa_key_attributes_t attributes = psa_key_attributes_init();
    
    /* 设置Key的属性 */
    psa_set_key_type(&attributes, key_type);
    psa_set_key_usage_flags(&attributes, key_usage);
    psa_set_key_algorithm(&attributes, key_alg);

    /* 导入密钥 */
    psa_status_t status = psa_import_key(&attributes, key, key_len, handle);
    onetls_check_errlog(status, "psa_import_key");
    
    psa_reset_key_attributes(&attributes);
    return status;   
}

uint32_t onetls_hash_size(uint32_t hash_id)
{
    return PSA_HASH_SIZE(hash_id);
}

uint32_t onetls_hash(uint32_t hash_id, 
                     const uint8_t *in, 
                     uint32_t in_len, 
                     uint8_t *out, 
                     uint32_t *out_len)
{ 
    size_t len = *out_len;

    // 计算摘要
    psa_status_t ret = psa_hash_compute(hash_id, in, in_len, out, len, &len);
    onetls_check_errlog(ret, "psa_hash_compute");

    *out_len = (uint32_t)len;
    return ret;    
}

uint32_t onetls_hmac(uint32_t hash_id,
                     const uint8_t *key, 
                     uint32_t key_len,
                     const uint8_t *in, 
                     uint32_t in_len,
                     uint8_t *out, 
                     uint32_t *out_len)
{   
    psa_key_handle_t handle;
    psa_mac_operation_t operation = psa_mac_operation_init();
    size_t len = *out_len;

    /* 导入密钥 */
    psa_status_t status = onetls_import_psa_key(&handle, 
                                                key, 
                                                key_len, 
                                                PSA_KEY_TYPE_HMAC, 
                                                PSA_KEY_USAGE_SIGN, 
                                                PSA_ALG_HMAC(hash_id));
    if (status != ONETLS_SUCCESS) {
        onetls_check_errlog(status, "psa_import_key");
        psa_destroy_key(handle);
        return status;
    }

    /* 创建mac对象 */
    psa_mac_sign_setup(&operation, handle, PSA_ALG_HMAC(hash_id));
    
    /* 更新消息 */
    psa_mac_update(&operation, in, in_len);

    /* 完成消息MAC计算 */
    psa_mac_sign_finish(&operation, out, len, &len);

    *out_len = (uint32_t)len;
    psa_destroy_key(handle);
    psa_mac_abort(&operation);
    return status;
}

void onetls_del_key_share(onetls_key_share *key_share)
{
    if (key_share == NULL) {
        return;
    }
    psa_destroy_key(key_share->private_key_index);
    key_share->private_key_index = 0;
}

uint32_t onetls_new_key_share(const onetls_dh_groups *dh, onetls_key_share *key_share)
{    
    size_t out_len = 0;
    psa_key_attributes_t attributes = psa_key_attributes_init();
    
    if (dh->kex == PSA_ALG_ECDH) {
        psa_set_key_type(&attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(dh->curve));
    } else if (dh->kex == PSA_ALG_FFDH) {
        psa_set_key_type(&attributes, PSA_KEY_TYPE_DH_KEY_PAIR(dh->curve));
    }

    psa_set_key_bits(&attributes, dh->key_bits);
    psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_EXPORT | PSA_KEY_USAGE_DERIVE);
    psa_set_key_algorithm(&attributes, dh->kex);   

    /* 产生key */
    uint32_t ret = psa_generate_key(&attributes, &(key_share->private_key_index));
    psa_reset_key_attributes(&attributes);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "psa_generate_key");    
        return ret;
    }

    ret = psa_export_public_key(key_share->private_key_index, key_share->public_key, dh->public_key_len, &(out_len));
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "psa_export_public_key");
        psa_destroy_key(key_share->private_key_index);
        key_share->private_key_index = 0;        
        return ret;
    }
    return ONETLS_SUCCESS;
}

uint32_t onetls_new_keyshare(onetls_ctx *ctx)
{
    ctx->handshake->key_share.key_share_all = (onetls_key_share *)onetls_malloc(g_onetls_dh_groups_num * sizeof(onetls_key_share));
    if (ctx->handshake->key_share.key_share_all == NULL) {
        return ONETLS_NULL_PTR;
    }

    for (uint8_t loop = 0; loop < g_onetls_dh_groups_num; loop++) {
        const onetls_dh_groups *dh = &(g_onetls_support_groups[loop]);
        onetls_key_share *key_share = &(ctx->handshake->key_share.key_share_all[loop]);

        key_share->group = dh;

        uint32_t ret = onetls_new_key_share(dh, key_share);
        if (ret != ONETLS_SUCCESS) {
            onetls_free(ctx->handshake->key_share.key_share_all);
            ctx->handshake->key_share.key_share_all = NULL;
            onetls_check_errlog(ret, "onetls_new_key_share");
            return ret;
        }
    }
    ctx->handshake->key_share.key_share_num = g_onetls_dh_groups_num;
    return ONETLS_SUCCESS;
}

uint32_t onetls_gen_kex_agreement(uint32_t kex_type, 
                                  psa_key_handle_t private_key, 
                                  const uint8_t *pub_key, uint32_t pub_len, 
                                  uint8_t *out, uint32_t out_size, uint32_t *out_len)
{
    size_t out_len_tmp = 0;
    uint32_t ret = psa_raw_key_agreement(kex_type, 
                                         private_key, 
                                         pub_key, pub_len, 
                                         out, out_size, &out_len_tmp);
    onetls_check_errlog(ret, "psa_raw_key_agreement");

    *out_len = out_len_tmp;
    return ret;
}

uint32_t onetls_gen_kex_with_pub_key(onetls_ctx *ctx, onetls_key_share *key_share, const uint8_t* pub_key, uint32_t pub_len)
{
    uint32_t out_len = 0;
    uint8_t raw_shared_secret[ONETLS_DH_PB_KEY_LEN] = { 0 };

    uint32_t ret = onetls_gen_kex_agreement(key_share->group->kex, 
                                            key_share->private_key_index,
                                            pub_key, pub_len, 
                                            raw_shared_secret, sizeof(raw_shared_secret), &out_len);
    if (ret != ONETLS_SUCCESS) {
        onetls_check_errlog(ret, "psa_raw_key_agreement");
        return ret;
    }
    
    onetls_free(ctx->handshake->key_share.dh_key);
    ctx->handshake->key_share.dh_key = onetls_malloc(out_len);
    if (ctx->handshake->key_share.dh_key == NULL) {
        onetls_check_errlog(ONETLS_MALLOC_FAIL, "onetls_malloc");
        return ONETLS_MALLOC_FAIL;
    }

    ctx->handshake->key_share.dh_key_len = out_len;
    memcpy(ctx->handshake->key_share.dh_key, raw_shared_secret, out_len);    
    memset(raw_shared_secret, 0, sizeof(raw_shared_secret));
    return ONETLS_SUCCESS;
}

void onetls_construct_client_hello_attr(uint8_t type, uint8_t *packet, uint32_t *length)
{
    uint8_t *tmp = packet + 2;
    uint8_t loop = 0;

    switch (type) {
    case ONETLS_ATTR_CIPHER:
        for (loop = 0; loop < g_onetls_cipher_num; loop++) {
            tmp += onetls_put_u16(tmp, g_onetls_ciphers[loop].iana_id);
        }
        break;
    case ONETLS_ATTR_GROUPS:
        for (loop = 0; loop < g_onetls_dh_groups_num; loop++) {
            tmp += onetls_put_u16(tmp, g_onetls_support_groups[loop].iana_id);
        }
        break;
    case ONETLS_ATTR_SIGN:
        for (loop = 0; loop < g_onetls_sign_algs_num; loop++) {
            tmp += onetls_put_u16(tmp, g_onetls_sign_algs[loop].iana_id);
        }
        break;
    default:
        onetls_check_errlog(ONETLS_INNER_SYSERR, "onetls_construct_client_hello_attr");
        return;
    }

    *length = tmp - packet;
    onetls_put_u16(packet, *length - 2);
}

uint32_t onetls_aes_ecb(uint8_t en, uint8_t *key, uint32_t key_len, uint8_t *input, uint8_t *output)
{
    mbedtls_aes_context aes_ctx;
    mbedtls_aes_init(&aes_ctx);
    
    en ?
    mbedtls_aes_setkey_enc(&aes_ctx, key, key_len * 8) :
    mbedtls_aes_setkey_dec(&aes_ctx, key, key_len * 8);

    int ret = mbedtls_aes_crypt_ecb(&aes_ctx, en ? MBEDTLS_AES_ENCRYPT : MBEDTLS_AES_DECRYPT, input, output);
    onetls_check_errlog(ret, "mbedtls_aes_crypt_ecb[%d]", en);

    mbedtls_aes_free(&aes_ctx);
    return (uint32_t)ret;
}

uint32_t onetls_crypto_create_ctx(psa_key_handle_t *ctx, const onetls_cipher *cipher, uint8_t *key, uint8_t usage)
{
    // 初始化秘钥
    psa_destroy_key(*ctx);
    
    // 重新注入密钥
    uint32_t status = onetls_import_psa_key(ctx, 
                                            key, cipher->key_len, cipher->key_type, 
                                            usage ? PSA_KEY_USAGE_ENCRYPT : PSA_KEY_USAGE_DECRYPT, 
                                            PSA_ALG_AEAD_WITH_TAG_LENGTH(cipher->key_alg, cipher->tag_len));
    if (status != ONETLS_SUCCESS) {
        onetls_check_errlog(status, "psa_import_key");
        psa_destroy_key(*ctx);
        *ctx = 0;
        return status;
    }
    return ONETLS_SUCCESS;      
}

void onetls_crypto_delete_ctx(psa_key_handle_t *ctx)
{ 
    psa_destroy_key(*ctx);
    *ctx = 0;
}

uint32_t onetls_aead_decrypt(onetls_ctx *ctx, 
                             uint8_t *nonce, 
                             uint8_t *add, uint32_t add_len, 
                             uint8_t *in, uint32_t in_len, 
                             uint8_t *mt, 
                             uint8_t *out, uint32_t *out_len)
{
    size_t out_len_t = *out_len;
    uint32_t status = psa_aead_decrypt(ctx->security.de_ctx,
                                       PSA_ALG_AEAD_WITH_TAG_LENGTH(ctx->security.cipher->key_alg, ctx->security.cipher->tag_len),  
                                       nonce, ctx->security.cipher->iv_len,
                                       add, add_len,
                                       in, in_len, out, out_len_t, &out_len_t);
    if (status != ONETLS_SUCCESS) {
        onetls_check_errlog(status, "psa_aead_decrypt");
        return status;
    }

    *out_len = out_len_t - 1;
    *mt = out[*out_len];
    return ONETLS_SUCCESS;
}

uint32_t onetls_aead_encrypt(onetls_ctx *ctx, 
                             uint8_t *nonce, 
                             uint8_t *add, uint32_t add_len, 
                             uint8_t *in, uint32_t in_len, 
                             uint8_t *mt, 
                             uint8_t *out, uint32_t *out_len)
{
    size_t out_len_t = *out_len;
    uint32_t status = 0;
    in[in_len++] = *mt;

    status = psa_aead_encrypt(ctx->security.en_ctx,
                              PSA_ALG_AEAD_WITH_TAG_LENGTH(ctx->security.cipher->key_alg, ctx->security.cipher->tag_len),  
                              nonce, ctx->security.cipher->iv_len, 
                              add, add_len, 
                              in, in_len, 
                              out, out_len_t, &out_len_t);
    if (status != ONETLS_SUCCESS) {
        onetls_check_errlog(status, "psa_aead_encrypt");
        return status;
    }
    *out_len = out_len_t;
    return status;    
}
